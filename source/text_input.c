/*
https://github.com/bucanero/apollo-ps3/blob/master/source/dialog.c
*/
#include <sysutil/osk.h>

#include <tiny3d.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <sys/memory.h>
#include <sysutil/msg.h>
#include <sysutil/osk.h>
#include <sysutil/sysutil.h>

#define OSK_IME_DIALOG_MAX_TITLE_LENGTH  (128)
#define OSK_IME_DIALOG_MAX_TEXT_LENGTH   (512)


volatile int msg_dialog_action = 0;

static int g_ime_active;
static int osk_action = 0;
static int osk_level = 0;

static sys_mem_container_t container_mem;
static oskCallbackReturnParam OutputReturnedParam;

volatile int osk_event = 0;
volatile int osk_unloaded = 0;

static uint16_t g_ime_title[OSK_IME_DIALOG_MAX_TITLE_LENGTH];
static uint16_t g_ime_text[OSK_IME_DIALOG_MAX_TEXT_LENGTH];
static uint16_t g_ime_input[OSK_IME_DIALOG_MAX_TEXT_LENGTH + 1];




#define countof(_array) (sizeof(_array) / sizeof(_array[0]))

static void osk_exit(void)
{
    if(osk_level == 2) {
        oskAbort();
        oskUnloadAsync(&OutputReturnedParam);
        
        osk_event = 0;
        osk_action=-1;
    }

    if(osk_level >= 1) {
        sysUtilUnregisterCallback(SYSUTIL_EVENT_SLOT1);
        sysMemContainerDestroy(container_mem);
    }
}


static int convert_from_utf16(const uint16_t* utf16, char* utf8, uint32_t size)
{
    int count = 0;
    while (*utf16)
    {
        uint32_t code;
        uint16_t ch = *utf16++;
        if (ch < 0xd800 || ch >= 0xe000)
        {
            code = ch;
        }
        else // surrogate pair
        {
            uint16_t ch2 = *utf16++;
            if (ch < 0xdc00 || ch > 0xe000 || ch2 < 0xd800 || ch2 > 0xdc00)
            {
                goto utf8_end;
            }
            code = 0x10000 + ((ch & 0x03FF) << 10) + (ch2 & 0x03FF);
        }

        if (code < 0x80)
        {
            if (size < 1) goto utf8_end;
            utf8[count++] = (char)code;
            size--;
        }
        else if (code < 0x800)
        {
            if (size < 2) goto utf8_end;
            utf8[count++] = (char)(0xc0 | (code >> 6));
            utf8[count++] = (char)(0x80 | (code & 0x3f));
            size -= 2;
        }
        else if (code < 0x10000)
        {
            if (size < 3) goto utf8_end;
            utf8[count++] = (char)(0xe0 | (code >> 12));
            utf8[count++] = (char)(0x80 | ((code >> 6) & 0x3f));
            utf8[count++] = (char)(0x80 | (code & 0x3f));
            size -= 3;
        }
        else
        {
            if (size < 4) goto utf8_end;
            utf8[count++] = (char)(0xf0 | (code >> 18));
            utf8[count++] = (char)(0x80 | ((code >> 12) & 0x3f));
            utf8[count++] = (char)(0x80 | ((code >> 6) & 0x3f));
            utf8[count++] = (char)(0x80 | (code & 0x3f));
            size -= 4;
        }
    }

utf8_end:
    utf8[count]=0;
    return count;
}


static int convert_to_utf16(const char* utf8, uint16_t* utf16, uint32_t available)
{
    int count = 0;
    while (*utf8)
    {
        uint8_t ch = (uint8_t)*utf8++;
        uint32_t code;
        uint32_t extra;

        if (ch < 0x80)
        {
            code = ch;
            extra = 0;
        }
        else if ((ch & 0xe0) == 0xc0)
        {
            code = ch & 31;
            extra = 1;
        }
        else if ((ch & 0xf0) == 0xe0)
        {
            code = ch & 15;
            extra = 2;
        }
        else
        {
            // TODO: this assumes there won't be invalid utf8 codepoints
            code = ch & 7;
            extra = 3;
        }

        for (uint32_t i=0; i<extra; i++)
        {
            uint8_t next = (uint8_t)*utf8++;
            if (next == 0 || (next & 0xc0) != 0x80)
            {
                goto utf16_end;
            }
            code = (code << 6) | (next & 0x3f);
        }

        if (code < 0xd800 || code >= 0xe000)
        {
            if (available < 1) goto utf16_end;
            utf16[count++] = (uint16_t)code;
            available--;
        }
        else // surrogate pair
        {
            if (available < 2) goto utf16_end;
            code -= 0x10000;
            utf16[count++] = 0xd800 | (code >> 10);
            utf16[count++] = 0xdc00 | (code & 0x3ff);
            available -= 2;
        }
    }

utf16_end:
    utf16[count]=0;
    return count;
}


static int osk_dialog_input_update(void)
{
	
    if (!osk_unloaded)
    {
        switch(osk_event)
        {
            case SYSUTIL_OSK_INPUT_ENTERED:
                oskGetInputText(&OutputReturnedParam);
                osk_event = 0;
                break;

            case SYSUTIL_OSK_INPUT_CANCELED:
                oskAbort();
                oskUnloadAsync(&OutputReturnedParam);
                osk_event = 0;
                osk_action = -1;
                break;

            case SYSUTIL_OSK_DONE:
                if (osk_action != -1) osk_action = 1;
                oskUnloadAsync(&OutputReturnedParam);
                osk_event = 0;
                break;

            default:
                break;
        }
    }
    else
    {
        g_ime_active = 0;

        if ((OutputReturnedParam.res == OSK_OK) && (osk_action == 1))
        {
            osk_exit();
            return 1;
        }
         
        osk_exit();
        return (-1);
    }

    return 0;
}


static void osk_event_handle(u64 status, u64 param, void * userdata)
{
    switch((u32) status) 
    {
	    case SYSUTIL_OSK_INPUT_CANCELED:
		    osk_event = SYSUTIL_OSK_INPUT_CANCELED;
		    break;

        case SYSUTIL_OSK_UNLOADED:
		    osk_unloaded = 1;
		    break;

        case SYSUTIL_OSK_INPUT_ENTERED:
	    	osk_event = SYSUTIL_OSK_INPUT_ENTERED;
		    break;

	    case SYSUTIL_OSK_DONE:
	    	osk_event = SYSUTIL_OSK_DONE;
		    break;

        default:
            break;
    }
}



static int osk_dialog_input_init(const char* title, const char* text, int maxlen)
{
    oskParam DialogOskParam;
    oskInputFieldInfo inputFieldInfo;
    oskPoint pos = {0.0, 0.0};

    osk_level = 0;
    g_ime_active = 0;
    
	if(sysMemContainerCreate(&container_mem, 8*1024*1024) < 0) {
        osk_exit();
        return 0;
    }

    osk_level = 1;

    memset(g_ime_input, 0, sizeof(g_ime_input));
    convert_to_utf16(title, g_ime_title, countof(g_ime_title) - 1);
    convert_to_utf16(text, g_ime_text, countof(g_ime_text) - 1);
    
    inputFieldInfo.message =  g_ime_title;
    inputFieldInfo.startText = g_ime_text;
    inputFieldInfo.maxLength = maxlen;
       
    OutputReturnedParam.res = OSK_NO_TEXT;
    OutputReturnedParam.len = OSK_IME_DIALOG_MAX_TEXT_LENGTH;
    OutputReturnedParam.str = g_ime_input;

    DialogOskParam.controlPoint = pos;
    DialogOskParam.firstViewPanel = OSK_PANEL_TYPE_ALPHABET_FULL_WIDTH;
    DialogOskParam.allowedPanels = (OSK_PANEL_TYPE_ALPHABET | OSK_PANEL_TYPE_NUMERAL | OSK_PANEL_TYPE_ENGLISH | OSK_PANEL_TYPE_URL);
    DialogOskParam.prohibitFlags = OSK_PROHIBIT_RETURN;

    if ((oskSetKeyLayoutOption (OSK_10KEY_PANEL | OSK_FULLKEY_PANEL) < 0) ||
        (oskAddSupportLanguage (DialogOskParam.allowedPanels) < 0) ||
        (oskSetLayoutMode(OSK_LAYOUTMODE_HORIZONTAL_ALIGN_CENTER | OSK_LAYOUTMODE_VERTICAL_ALIGN_CENTER) < 0) ||
        (oskSetInitialInputDevice(OSK_DEVICE_PAD) < 0))
    {
        osk_exit();
        return 0;
    }

    sysUtilUnregisterCallback(SYSUTIL_EVENT_SLOT1);
    sysUtilRegisterCallback(SYSUTIL_EVENT_SLOT1, osk_event_handle, NULL);

    osk_action = 0;
    osk_unloaded = 0;

    if(oskLoadAsync(container_mem, &DialogOskParam, &inputFieldInfo) < 0) {
        osk_exit();
        return 0;
    }

    osk_level = 2;
    g_ime_active = 1;
    return 1;
}

int input(const char* title, char* text, uint32_t size)
{
    size = (size > OSK_IME_DIALOG_MAX_TEXT_LENGTH) ? OSK_IME_DIALOG_MAX_TEXT_LENGTH : (size-1);

    if (!osk_dialog_input_init(title, text, size))
        return 0;

    while (g_ime_active)
    {
        if (osk_dialog_input_update() < 0)
            return 0;
	tiny3d_Clear(0xff000000, TINY3D_CLEAR_ALL);
	tiny3d_AlphaTest(1, 0x10, TINY3D_ALPHA_FUNC_GEQUAL);
	tiny3d_BlendFunc(1, TINY3D_BLEND_FUNC_SRC_RGB_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_ALPHA_SRC_ALPHA,
		TINY3D_BLEND_FUNC_SRC_ALPHA_ONE_MINUS_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_RGB_ONE_MINUS_SRC_ALPHA,
		TINY3D_BLEND_RGB_FUNC_ADD | TINY3D_BLEND_ALPHA_FUNC_ADD);

	tiny3d_Project2D();
tiny3d_Flip();
        
    }

    return (convert_from_utf16(g_ime_input, text, size));
}

