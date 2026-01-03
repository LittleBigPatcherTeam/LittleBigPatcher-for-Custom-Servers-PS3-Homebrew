#include <tiny3d.h>
#include <libfont.h>
#include <sysmodule/sysmodule.h>
#include <pngdec/pngdec.h>

#include <assert.h>

#include <io/pad.h>
#include <io/kb.h>
#include <sysutil/sysutil.h>

#include <sys/thread.h>
#include <ppu-types.h>

#include <stdio.h>
#include <sys/time.h>
#include <dbglogger.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "http.h"
#include "colours_config.h"
#include "font_stuff.h"
#include "text_input.h"
#include "save_folders.h"
#include "oscetool_main.h"
#include "read_sfo.h"
#include "copyfile_thing.h"
#include "for_oscetools_gloabls.h"
#include "get_idps.h"
#include "fail0verflow_PS3_tools/unself.h"

#include "lua-5.4.7/src/lua.h"
#include "lua-5.4.7/src/lauxlib.h"
#include "lua-5.4.7/src/lualib.h"

#define MAX_URL_LEN_INCL_NULL 72
#define MAX_DIGEST_LEN_INCL_NULL 20


#define BTN_LEFT       32768
#define BTN_DOWN       16384
#define BTN_RIGHT      8192
#define BTN_UP         4096
#define BTN_START      2048
#define BTN_R3         1024
#define BTN_L3         512
#define BTN_SELECT     256
#define BTN_SQUARE     128

// globals instead of macros because we need to swap them in init if the user has circle for enter
int BTN_CROSS;
int BTN_CIRCLE;

#define BTN_TRIANGLE   16
#define BTN_R1         8
#define BTN_L1         4
#define BTN_R2         2
#define BTN_L2         1

#define DRAW_ICON_0_MAIN_PNG_X 524
#define DRAW_ICON_0_MAIN_PNG_Y 57

#define START_X_FOR_PRESS_TO_REFRESH_THINGS_TEXT 793
#define START_X_FOR_CONTROLS_TIP 100
#define MAX_CAPITIAL_W_CHARACTERS_PER_LINE 30
#define NEW_LINES_AMNT_PER_DIGIT_OF_X_INCREASE 8 // seems to be good
#define MAX_LINES (11-1-1) // minus 1 for title, minus 1 for bottom controls tip
#define MAX_LINE_LEN_OF_URL_ENTRY MAX_URL_LEN_INCL_NULL - 1 + MAX_DIGEST_LEN_INCL_NULL - 1 + strlen(" ") + PATCH_LUA_SIZE + strlen(" ")

#define CHARACTER_HEIGHT 45
#define NORMAL_TEXT_X 18 // this has to be even
#define NORMAL_TEXT_Y 32

#define SECOND_THREAD_NAME "second_thread"
#define SECOND_THREAD_PRIORITY 1500 // 0 means highest, 1500 just gotten from the sample
#define SECOND_THREAD_STACK_SIZE 0x500000 // 5mb

#define THREAD_RET_EBOOT_REVERTED 3
#define THREAD_RET_EBOOT_BAK_NO_EXIST 4
#define THREAD_RET_EBOOT_DECRYPT_FAILED 1
#define THREAD_RET_EBOOT_PATCH_FAILED 5
#define THREAD_RET_EBOOT_PATCHED 6
#define THREAD_RET_EBOOT_BACKUP_FAILED 7

#define THREAD_RET_COULD_NOT_DOWNLOAD_LATEST_PKG 1

#define THREAD_CURRENT_STATE_CLEANING_WORKSPACE 1
#define THREAD_CURRENT_STATE_RESTORING_EBOOT_BIN_BAK 2
#define THREAD_CURRENT_STATE_DECRYPTING_EBOOT_BIN 3
#define THREAD_CURRENT_STATE_UNSELF_EBOOT_BIN 4
#define THREAD_CURRENT_STATE_START_PATCHING 5
#define THREAD_CURRENT_STATE_DONE_PATCHING 6
#define THREAD_CURRENT_STATE_MAKING_EBOOT_BIN_BAK 7
#define THREAD_CURRENT_ENCRYPTING_FOR_DIGITAL 8
#define THREAD_CURRENT_ENCRYPTING_FOR_DISC 9

#define MENU_MAIN 0
#define MENU_MAIN_ARROW 5-1

#define MENU_SELECT_URLS 1
#define MENU_SELECT_URLS_ARROW (saved_urls_count-1)+1
#define MENU_URL_EDITOR_ARROW 2+(method_count-1)
#define MINUS_MENU_ARROW_AMNT_URL_EDITOR_TO_GET_PATCH_LUA_INDEX 2

#define MENU_EDIT_URLS 2
#define MENU_EDIT_URLS_ARROW (saved_urls_count >= MAX_SAVED_URLS_AMNT) ? (saved_urls_count-1) : (saved_urls_count-1)+1

#define MENU_PATCH_GAMES_ARROW 6-1
#define MENU_URL_EDITOR 3
#define MENU_PATCH_GAMES 4

#define MENU_BROWSE_GAMES 5


#define YES_NO_POPUP_ARROW 2-1
#define YES_NO_GAME_POPUP_REVERT_EBOOT 1
#define YES_NO_GAME_POPUP_PATCH_GAME 2
#define YES_NO_GAME_POPUP_DELETE_URL 3
#define YES_NO_GAME_POPUP_RESET_URLS 4

#define CURRENTLY_CHECKING_FOR_UPDATES 3

// globals instead of macros because we need to swap them in init if the user has circle for enter
char MY_CUSTOM_EDIT_OF_NOTO_SANS_FONT_CROSS_BTN[3];
char MY_CUSTOM_EDIT_OF_NOTO_SANS_FONT_CIRCLE_BTN[3];

#define MY_CUSTOM_EDIT_OF_NOTO_SANS_FONT_SQUARE_BTN "\x87"
#define MY_CUSTOM_EDIT_OF_NOTO_SANS_FONT_TRIANGLE_BTN "\x89"
#define MY_CUSTOM_EDIT_OF_NOTO_SANS_FONT_DPAD_LEFT_RIGHT "\xB3"

#define DEFAULT_TITLE_ID "BCES00000"

#define CAUSE_A_PS3_FREEZE *(int*)0x69 = 0

#define DONE_A_SWITCH has_done_a_switch = 1; load_global_title_id(); load_user_join_pwd(second_thread_args.join_password)

char global_newest_version_tag[sizeof(VERSION_NUM_STR)];

padInfo padinfo;
padData paddata;

KbInfo kbinfo;
KbConfig kbconfig;
KbData kbdata;

lua_State *L;

struct TitleIdAndGameName {
	char title_id[sizeof(DEFAULT_TITLE_ID)];
	char game_name[128];
};

struct LuaPatchDetails {
	char patch_name[PATCH_LUA_SIZE + 1];
	char patch_method[PATCH_METHOD_LUA_STRING_SIZE];
};

typedef struct UrlToPatchTo {
	char url[MAX_URL_LEN_INCL_NULL];
	char patch_name[PATCH_LUA_SIZE + 1];
	char digest[MAX_DIGEST_LEN_INCL_NULL];
};

struct UrlToPatchTo saved_urls[MAX_LINES];
#define MAX_SAVED_URLS_AMNT sizeof(saved_urls) / sizeof(saved_urls[0])
#define RESET_SELECTED_URL_INDEX sizeof(saved_urls) / sizeof(saved_urls[0]) + 1
s8 selected_url_index = RESET_SELECTED_URL_INDEX;
s8 saved_urls_count = 0;
char global_title_id[sizeof(DEFAULT_TITLE_ID)] = DEFAULT_TITLE_ID;

void exiting()
{
    sysModuleUnload(SYSMODULE_PNGDEC);
	http_end();
}

unsigned get_button_pressed(int enter_button)
{
	// no clue what these are, utf-16 codes?
	#define KB_KEY_RIGHT_ARROW 0x804F
	#define KB_KEY_LEFT_ARROW 0x8050
	#define KB_KEY_DOWN_ARROW 0x8051
	#define KB_KEY_UP_ARROW 0x8052

	#define KB_KEY_ESC 0x8029

	#define KB_KEY_F5 0x803e

	// these i know are ascii codes
	#define KB_KEY_ENTER 0xA
	#define KB_KEY_BACKSPACE 0x8

	#define KB_KEY_UPPER_W 0x57
	#define KB_KEY_LOWER_W 0x77
	#define KB_KEY_UPPER_A 0x41
	#define KB_KEY_LOWER_A 0x61
	#define KB_KEY_UPPER_S 0x53
	#define KB_KEY_LOWER_S 0x73
	#define KB_KEY_UPPER_D 0x44
	#define KB_KEY_LOWER_D 0x64
	#define KB_KEY_DOWN_SPACE 0x20
	#define KB_KEY_UPPER_R 0x52
	#define KB_KEY_LOWER_R 0x72

	int i_for_pad_num;
	int i_for_kb_num;
	int j_for_kb_num;
	unsigned result = 0;

	ioPadGetInfo(&padinfo);
	ioKbGetInfo(&kbinfo);

	for(i_for_kb_num = 0; i_for_kb_num < MAX_KEYBOARDS; i_for_kb_num++){
		if(kbinfo.status[i_for_kb_num]){
			ioKbRead(i_for_kb_num, &kbdata);
			for(j_for_kb_num = 0; j_for_kb_num < kbdata.nb_keycode; j_for_kb_num++) {
				switch (kbdata.keycode[j_for_kb_num]) {
					case KB_KEY_UP_ARROW:
					case KB_KEY_UPPER_W:
					case KB_KEY_LOWER_W:
						result |= BTN_UP;
						break;

					case KB_KEY_DOWN_ARROW:
					case KB_KEY_UPPER_S:
					case KB_KEY_LOWER_S:
						result |= BTN_DOWN;
						break;

					case KB_KEY_LEFT_ARROW:
					case KB_KEY_UPPER_A:
					case KB_KEY_LOWER_A:
						result |= BTN_LEFT;
						break;

					case KB_KEY_RIGHT_ARROW:
					case KB_KEY_UPPER_D:
					case KB_KEY_LOWER_D:
						result |= BTN_RIGHT;
						break;

					case KB_KEY_ENTER:
					case KB_KEY_DOWN_SPACE:
						if (enter_button == 1) {
							result |= BTN_CROSS;
						}
						else {
							result |= BTN_CIRCLE;
						}
						break;

					case KB_KEY_BACKSPACE:
					case KB_KEY_ESC:
						if (enter_button == 1) {
							result |= BTN_CIRCLE;
						}
						else {
							result |= BTN_CROSS;
						}
						break;

					case KB_KEY_UPPER_R:
					case KB_KEY_LOWER_R:
					case KB_KEY_F5:
						result |= BTN_TRIANGLE;
						break;
				}
			}

			break;
		}
	}

	for(i_for_pad_num = 0; i_for_pad_num < MAX_PADS; i_for_pad_num++){

		if(padinfo.status[i_for_pad_num]){
			ioPadGetData(i_for_pad_num, &paddata);
			result |= (paddata.button[2] << 8) | (paddata.button[3] & 0xff);

			// accept left analog stick inputs as dpad inputs
			if (paddata.ANA_L_V < 16)
				result |= BTN_UP;

			if (paddata.ANA_L_V > 224)
				result |= BTN_DOWN;

			if (paddata.ANA_L_H < 16)
				result |= BTN_LEFT;

			if (paddata.ANA_L_H > 224)
				result |= BTN_RIGHT;

			break;
		}

	}
	return result;
}

/*
returns -1 if something went wrong, otherwise returns the texture offset in tiny3d
but the functions handles -1 buy just not doing anything
*/
void load_png_from_filename_to_memory(pngData *texture_output, int * img_index, char * filename) {
	int png_buffer_size;
	char * png_buffer;

	FILE *fp_png = fopen(filename,"rb");
	if (fp_png == 0) {
		*img_index = -1;
		return;
	}
	fseek(fp_png, 0, SEEK_END);
	png_buffer_size = ftell(fp_png);
	if (png_buffer_size > 10000000) {
		fclose(fp_png);
		*img_index = -1;
		return;
	}
	rewind(fp_png);
	png_buffer = malloc(png_buffer_size);
	fread(png_buffer,1,10000000,fp_png);
	fclose(fp_png);

	pngLoadFromBuffer(png_buffer, png_buffer_size, texture_output);
	free(png_buffer);

	if (!texture_output->bmp_out) {
		*img_index = -1;
		return;
	}

	RSX_MEMCPY(texture_pointer, texture_output->bmp_out, texture_output->pitch * texture_output->height);
	free(texture_output->bmp_out);
	*img_index = tiny3d_TextureOffset(texture_pointer);
	texture_pointer += ((texture_output->pitch * texture_output->height + 15) & ~15) / 4;
}

void free_png_from_memory(pngData *texture_input, int * img_index) {
	if (*img_index == -1) {
		return;
	}
	texture_pointer -= ((texture_input->pitch * texture_input->height + 15) & ~15) / 4;
	*img_index = -1;
}

void draw_png(pngData *texture_input, int img_index, int x_coord, int y_coord) {
	if (img_index == -1) {
		return;
	}
	tiny3d_SetTexture(0, img_index, texture_input->width,
		texture_input->height, texture_input->pitch,
		TINY3D_TEX_FORMAT_A8R8G8B8, TEXTURE_LINEAR);

	DrawSprites2D(x_coord, y_coord, 1, texture_input->width, texture_input->height, 0xffffffff);
}

#define U32_SWAP_FOR_COLOUR(num) (u32)num

u32 internal_rainbow_colour_state = 0;
int internal_rainbow_colour_cur_index = 0;
/*
smooth transition for

#FF0000
#00FF00
#0000FF
#00FFFF
#FF00FF

100 colours total, for end loop back to red at top
*/
const u32 rainbow_colours[] = {
	U32_SWAP_FOR_COLOUR(0xff0000ff),
	U32_SWAP_FOR_COLOUR(0xf20d00ff),
	U32_SWAP_FOR_COLOUR(0xe41b00ff),
	U32_SWAP_FOR_COLOUR(0xd72800ff),
	U32_SWAP_FOR_COLOUR(0xc93600ff),
	U32_SWAP_FOR_COLOUR(0xbc4300ff),
	U32_SWAP_FOR_COLOUR(0xae5100ff),
	U32_SWAP_FOR_COLOUR(0xa15e00ff),
	U32_SWAP_FOR_COLOUR(0x946b00ff),
	U32_SWAP_FOR_COLOUR(0x867900ff),
	U32_SWAP_FOR_COLOUR(0x798600ff),
	U32_SWAP_FOR_COLOUR(0x6b9400ff),
	U32_SWAP_FOR_COLOUR(0x5ea100ff),
	U32_SWAP_FOR_COLOUR(0x51ae00ff),
	U32_SWAP_FOR_COLOUR(0x43bc00ff),
	U32_SWAP_FOR_COLOUR(0x36c900ff),
	U32_SWAP_FOR_COLOUR(0x28d700ff),
	U32_SWAP_FOR_COLOUR(0x1be400ff),
	U32_SWAP_FOR_COLOUR(0x0df200ff),
	U32_SWAP_FOR_COLOUR(0x00ff00ff),
	U32_SWAP_FOR_COLOUR(0x00ff00ff),
	U32_SWAP_FOR_COLOUR(0x00f20dff),
	U32_SWAP_FOR_COLOUR(0x00e41bff),
	U32_SWAP_FOR_COLOUR(0x00d728ff),
	U32_SWAP_FOR_COLOUR(0x00c936ff),
	U32_SWAP_FOR_COLOUR(0x00bc43ff),
	U32_SWAP_FOR_COLOUR(0x00ae51ff),
	U32_SWAP_FOR_COLOUR(0x00a15eff),
	U32_SWAP_FOR_COLOUR(0x00946bff),
	U32_SWAP_FOR_COLOUR(0x008679ff),
	U32_SWAP_FOR_COLOUR(0x007986ff),
	U32_SWAP_FOR_COLOUR(0x006b94ff),
	U32_SWAP_FOR_COLOUR(0x005ea1ff),
	U32_SWAP_FOR_COLOUR(0x0051aeff),
	U32_SWAP_FOR_COLOUR(0x0043bcff),
	U32_SWAP_FOR_COLOUR(0x0036c9ff),
	U32_SWAP_FOR_COLOUR(0x0028d7ff),
	U32_SWAP_FOR_COLOUR(0x001be4ff),
	U32_SWAP_FOR_COLOUR(0x000df2ff),
	U32_SWAP_FOR_COLOUR(0x0000ffff),
	U32_SWAP_FOR_COLOUR(0x0000ffff),
	U32_SWAP_FOR_COLOUR(0x000dffff),
	U32_SWAP_FOR_COLOUR(0x001bffff),
	U32_SWAP_FOR_COLOUR(0x0028ffff),
	U32_SWAP_FOR_COLOUR(0x0036ffff),
	U32_SWAP_FOR_COLOUR(0x0043ffff),
	U32_SWAP_FOR_COLOUR(0x0051ffff),
	U32_SWAP_FOR_COLOUR(0x005effff),
	U32_SWAP_FOR_COLOUR(0x006bffff),
	U32_SWAP_FOR_COLOUR(0x0079ffff),
	U32_SWAP_FOR_COLOUR(0x0086ffff),
	U32_SWAP_FOR_COLOUR(0x0094ffff),
	U32_SWAP_FOR_COLOUR(0x00a1ffff),
	U32_SWAP_FOR_COLOUR(0x00aeffff),
	U32_SWAP_FOR_COLOUR(0x00bcffff),
	U32_SWAP_FOR_COLOUR(0x00c9ffff),
	U32_SWAP_FOR_COLOUR(0x00d7ffff),
	U32_SWAP_FOR_COLOUR(0x00e4ffff),
	U32_SWAP_FOR_COLOUR(0x00f2ffff),
	U32_SWAP_FOR_COLOUR(0x00ffffff),
	U32_SWAP_FOR_COLOUR(0x00ffffff),
	U32_SWAP_FOR_COLOUR(0x0df2ffff),
	U32_SWAP_FOR_COLOUR(0x1be4ffff),
	U32_SWAP_FOR_COLOUR(0x28d7ffff),
	U32_SWAP_FOR_COLOUR(0x36c9ffff),
	U32_SWAP_FOR_COLOUR(0x43bcffff),
	U32_SWAP_FOR_COLOUR(0x51aeffff),
	U32_SWAP_FOR_COLOUR(0x5ea1ffff),
	U32_SWAP_FOR_COLOUR(0x6b94ffff),
	U32_SWAP_FOR_COLOUR(0x7986ffff),
	U32_SWAP_FOR_COLOUR(0x8679ffff),
	U32_SWAP_FOR_COLOUR(0x946bffff),
	U32_SWAP_FOR_COLOUR(0xa15effff),
	U32_SWAP_FOR_COLOUR(0xae51ffff),
	U32_SWAP_FOR_COLOUR(0xbc43ffff),
	U32_SWAP_FOR_COLOUR(0xc936ffff),
	U32_SWAP_FOR_COLOUR(0xd728ffff),
	U32_SWAP_FOR_COLOUR(0xe41bffff),
	U32_SWAP_FOR_COLOUR(0xf20dffff),
	U32_SWAP_FOR_COLOUR(0xff00ffff),
	U32_SWAP_FOR_COLOUR(0xff00ffff),
	U32_SWAP_FOR_COLOUR(0xff00f2ff),
	U32_SWAP_FOR_COLOUR(0xff00e4ff),
	U32_SWAP_FOR_COLOUR(0xff00d7ff),
	U32_SWAP_FOR_COLOUR(0xff00c9ff),
	U32_SWAP_FOR_COLOUR(0xff00bcff),
	U32_SWAP_FOR_COLOUR(0xff00aeff),
	U32_SWAP_FOR_COLOUR(0xff00a1ff),
	U32_SWAP_FOR_COLOUR(0xff0094ff),
	U32_SWAP_FOR_COLOUR(0xff0086ff),
	U32_SWAP_FOR_COLOUR(0xff0079ff),
	U32_SWAP_FOR_COLOUR(0xff006bff),
	U32_SWAP_FOR_COLOUR(0xff005eff),
	U32_SWAP_FOR_COLOUR(0xff0051ff),
	U32_SWAP_FOR_COLOUR(0xff0043ff),
	U32_SWAP_FOR_COLOUR(0xff0036ff),
	U32_SWAP_FOR_COLOUR(0xff0028ff),
	U32_SWAP_FOR_COLOUR(0xff001bff),
	U32_SWAP_FOR_COLOUR(0xff000dff),
};
#define RAINBOW_LAST_INDEX (sizeof(rainbow_colours) / sizeof(rainbow_colours[0]))-1

u32 get_next_rainbow_colour() {
	struct timeval current_time;
	gettimeofday(&current_time,NULL);
	if (((u32)current_time.tv_usec - internal_rainbow_colour_state) >= (1000*10)) {
		internal_rainbow_colour_state = (u32)current_time.tv_usec;
		internal_rainbow_colour_cur_index++;
	}
	if (internal_rainbow_colour_cur_index > RAINBOW_LAST_INDEX) {
		internal_rainbow_colour_cur_index = 0;
	}
	return rainbow_colours[internal_rainbow_colour_cur_index];
}


#define SetFontColor(font_colour_in, bg_colour_in) SetFontColor((font_colour_in == 522001152) ? rainbow_colour : font_colour_in, (bg_colour_in == 522001152) ? rainbow_colour : bg_colour_in)

void drawScene(u8 current_menu,int menu_arrow, u8 error_yet_to_press_ok, char* error_msg, int yes_no_game_popup, int started_a_thread, int thread_current_state,
pngData *texture_input, int * img_index, u8 saved_urls_txt_num, bool normalise_digest_checked,bool use_patch_cache_checked,struct TitleIdAndGameName browse_games_buffer[], u32 browse_games_buffer_size, u32 browse_games_buffer_start,char * global_title_id,
int method_count, struct LuaPatchDetails patch_lua_names[],
char * join_password, bool allow_triangle_bypass_exit_after_done
)
{
	u32 rainbow_colour;
	float x, y;
	int bg_colour;
	int font_colour;

	rainbow_colour = get_next_rainbow_colour();

    tiny3d_Project2D(); // change to 2D context (remember you it works with 848 x 512 as virtual coordinates)
    if (BACKGROUND_COLOUR == 522001152) {
		DrawBackground2D(rainbow_colour);
	}
	else {
		DrawBackground2D(BACKGROUND_COLOUR);
	}

    SetFontSize(NORMAL_TEXT_X, NORMAL_TEXT_Y);
    SetFontColor(TITLE_FONT_COLOUR,TITLE_BG_COLOUR);
	x= 0.0; y = 0.0;

	DrawFormatString(START_X_FOR_PRESS_TO_REFRESH_THINGS_TEXT,y,VERSION_NUM_STR);



	if (error_yet_to_press_ok != 0) {
		y += CHARACTER_HEIGHT;
		if (error_yet_to_press_ok == 1) {
			SetFontColor(ERROR_MESSAGE_COLOUR,ERROR_MESSAGE_BG_COLOUR);
		}
		else if (error_yet_to_press_ok == 2) {
			SetFontColor(SUCCESS_MESSAGE_COLOUR,SUCCESS_MESSAGE_BG_COLOUR);
		}
		DrawFormatString(x,y,error_msg);
		y += CHARACTER_HEIGHT*8; // give a bunch of space for title
		SetFontColor(SELECTABLE_NORMAL_FONT_COLOUR, SELECTED_FONT_BG_COLOUR);
		DrawFormatString(x,y,"Press %s to continue",MY_CUSTOM_EDIT_OF_NOTO_SANS_FONT_CROSS_BTN);
		if (allow_triangle_bypass_exit_after_done) {
			y += CHARACTER_HEIGHT;
			DrawFormatString(x,y,"Press "MY_CUSTOM_EDIT_OF_NOTO_SANS_FONT_TRIANGLE_BTN" to skip update");
		}
		draw_png(texture_input,*img_index,DRAW_ICON_0_MAIN_PNG_X,DRAW_ICON_0_MAIN_PNG_Y);
		return;
	}

	else if (started_a_thread != 0) {
		y += CHARACTER_HEIGHT;
		switch (started_a_thread) {
			case CURRENTLY_CHECKING_FOR_UPDATES:
				DrawFormatString(x,y,"Checking for LittleBigPatcherTeam updates. Please wait...");
				break;
			case YES_NO_GAME_POPUP_REVERT_EBOOT:
				DrawFormatString(x,y,"Reverting patches on your game. Please wait...");
				break;
			case YES_NO_GAME_POPUP_PATCH_GAME:
				DrawFormatString(x,y,"Applying patches to your game. Please wait...");
				y += CHARACTER_HEIGHT;
				bg_colour = TITLE_BG_COLOUR;
				SetFontColor(SELECTABLE_NORMAL_FONT_COLOUR, bg_colour);

				bg_colour = (thread_current_state == THREAD_CURRENT_STATE_CLEANING_WORKSPACE) ? SELECTED_FONT_BG_COLOUR : TITLE_BG_COLOUR;
				SetFontColor(SELECTABLE_NORMAL_FONT_COLOUR, bg_colour);
				DrawString(x, y, "Cleaning workspace");
				y += CHARACTER_HEIGHT;

				bg_colour = (thread_current_state == THREAD_CURRENT_STATE_RESTORING_EBOOT_BIN_BAK) ? SELECTED_FONT_BG_COLOUR : TITLE_BG_COLOUR;
				SetFontColor(SELECTABLE_NORMAL_FONT_COLOUR, bg_colour);
				DrawString(x, y, "Restoring EBOOT.BIN.BAK file");
				y += CHARACTER_HEIGHT;

				bg_colour = (thread_current_state == THREAD_CURRENT_STATE_DECRYPTING_EBOOT_BIN) ? SELECTED_FONT_BG_COLOUR : TITLE_BG_COLOUR;
				SetFontColor(SELECTABLE_NORMAL_FONT_COLOUR, bg_colour);
				DrawString(x, y, "Decrypting EBOOT.BIN file to workspace");
				y += CHARACTER_HEIGHT;

				bg_colour = (thread_current_state == THREAD_CURRENT_STATE_UNSELF_EBOOT_BIN) ? SELECTED_FONT_BG_COLOUR : TITLE_BG_COLOUR;
				SetFontColor(SELECTABLE_NORMAL_FONT_COLOUR, bg_colour);
				DrawString(x, y, "Decrypt failed, unself EBOOT.BIN file to workspace");
				y += CHARACTER_HEIGHT;

				bg_colour = (thread_current_state == THREAD_CURRENT_STATE_START_PATCHING) ? SELECTED_FONT_BG_COLOUR : TITLE_BG_COLOUR;
				SetFontColor(SELECTABLE_NORMAL_FONT_COLOUR, bg_colour);
				DrawString(x, y, "Start patching EBOOT.BIN.ELF");
				y += CHARACTER_HEIGHT;

				bg_colour = (thread_current_state == THREAD_CURRENT_STATE_DONE_PATCHING) ? SELECTED_FONT_BG_COLOUR : TITLE_BG_COLOUR;
				SetFontColor(SELECTABLE_NORMAL_FONT_COLOUR, bg_colour);
				DrawString(x, y, "Done patching");
				y += CHARACTER_HEIGHT;

				bg_colour = (thread_current_state == THREAD_CURRENT_STATE_MAKING_EBOOT_BIN_BAK) ? SELECTED_FONT_BG_COLOUR : TITLE_BG_COLOUR;
				SetFontColor(SELECTABLE_NORMAL_FONT_COLOUR, bg_colour);
				DrawString(x, y, "Making a backup of EBOOT.BIN since no EBOOT.BIN.BAK was found");
				y += CHARACTER_HEIGHT;

				bg_colour = (thread_current_state == THREAD_CURRENT_ENCRYPTING_FOR_DIGITAL) ? SELECTED_FONT_BG_COLOUR : TITLE_BG_COLOUR;
				SetFontColor(SELECTABLE_NORMAL_FONT_COLOUR, bg_colour);
				DrawString(x, y, "Encrypting EBOOT.BIN as Digital");
				y += CHARACTER_HEIGHT;

				bg_colour = (thread_current_state == THREAD_CURRENT_ENCRYPTING_FOR_DISC) ? SELECTED_FONT_BG_COLOUR : TITLE_BG_COLOUR;
				SetFontColor(SELECTABLE_NORMAL_FONT_COLOUR, bg_colour);
				DrawString(x, y, "Encrypting EBOOT.BIN as Disc update");
				y += CHARACTER_HEIGHT;


				break;
		}
		return;
	}

	else if (yes_no_game_popup != 0) {
		y += CHARACTER_HEIGHT;
		DrawFormatString(x,y,error_msg);
		y += CHARACTER_HEIGHT*8; // give a bunch of space for title

		bg_colour = (menu_arrow == 0) ? SELECTED_FONT_BG_COLOUR : UNSELECTED_FONT_BG_COLOUR;
		SetFontColor(SELECTABLE_NORMAL_FONT_COLOUR, bg_colour);
		DrawString(x,y,"Yes");
		y += CHARACTER_HEIGHT;

		bg_colour = (menu_arrow == 1) ? SELECTED_FONT_BG_COLOUR : UNSELECTED_FONT_BG_COLOUR;
		SetFontColor(SELECTABLE_NORMAL_FONT_COLOUR, bg_colour);
		DrawString(x,y,"No");
		y += CHARACTER_HEIGHT;
		draw_png(texture_input,*img_index,DRAW_ICON_0_MAIN_PNG_X,DRAW_ICON_0_MAIN_PNG_Y);

		return;
	}

	DrawFormatString(START_X_FOR_CONTROLS_TIP,(CHARACTER_HEIGHT*(MAX_LINES+1)),"Select - %s | Go Back - %s | Change Pages - %s | Delete URL - %s | Refresh Page - %s",
		MY_CUSTOM_EDIT_OF_NOTO_SANS_FONT_CROSS_BTN,
		MY_CUSTOM_EDIT_OF_NOTO_SANS_FONT_CIRCLE_BTN,
		MY_CUSTOM_EDIT_OF_NOTO_SANS_FONT_DPAD_LEFT_RIGHT,
		MY_CUSTOM_EDIT_OF_NOTO_SANS_FONT_SQUARE_BTN,
		MY_CUSTOM_EDIT_OF_NOTO_SANS_FONT_TRIANGLE_BTN
	);

    switch (current_menu) {
		case MENU_MAIN:

			DrawFormatString(x,y,"Main Menu");
			y += CHARACTER_HEIGHT;

			bg_colour = (menu_arrow == 0) ? SELECTED_FONT_BG_COLOUR : UNSELECTED_FONT_BG_COLOUR;
			SetFontColor(SELECTABLE_NORMAL_FONT_COLOUR, bg_colour);
			DrawString(x,y,"Select URL");
			y += CHARACTER_HEIGHT;

			bg_colour = (menu_arrow == 1) ? SELECTED_FONT_BG_COLOUR : UNSELECTED_FONT_BG_COLOUR;
			SetFontColor(SELECTABLE_NORMAL_FONT_COLOUR, bg_colour);
			DrawString(x,y,"Edit URLs");
			y += CHARACTER_HEIGHT;


			bg_colour = (menu_arrow == 2) ? SELECTED_FONT_BG_COLOUR : UNSELECTED_FONT_BG_COLOUR;
			SetFontColor(SELECTABLE_NORMAL_FONT_COLOUR, bg_colour);
			DrawString(x,y,"Patch Games");
			y += CHARACTER_HEIGHT;

			bg_colour = (menu_arrow == 3) ? SELECTED_FONT_BG_COLOUR : UNSELECTED_FONT_BG_COLOUR;
			font_colour = (use_patch_cache_checked) ? TURNED_ON_FONT_COLOUR : SELECTABLE_NORMAL_FONT_COLOUR;
			SetFontColor(font_colour, bg_colour);
			if (use_patch_cache_checked) {
				DrawString(x,y,"Patch Cache: ON (turn OFF to clear cache)");
			}
			else {
				DrawString(x,y,"Patch Cache: OFF (useful if you switch URLS often when ON)");
			}

			y += CHARACTER_HEIGHT;

			bg_colour = (menu_arrow == 4) ? SELECTED_FONT_BG_COLOUR : UNSELECTED_FONT_BG_COLOUR;
			SetFontColor(SELECTABLE_NORMAL_FONT_COLOUR, bg_colour);
			DrawString(x,y,"Exit");

			y += CHARACTER_HEIGHT;

			y += CHARACTER_HEIGHT;
			SetFontColor(TURNED_ON_FONT_COLOUR,0);
			DrawString(x,y,"Things will have this font colour if it is selected");
			y += CHARACTER_HEIGHT;
			SetFontColor(TITLE_FONT_COLOUR,TITLE_BG_COLOUR);
			DrawString(x,y,"Check out https://littlebigpatcherteam.github.io/2025/03/03/LBPCSPPHB.html");
			y += CHARACTER_HEIGHT;
			DrawString(x,y,"As per GPL-3.0 licence you MUST be provided the source code of this app!, refer to above for more info");
			break;
		case MENU_PATCH_GAMES:
			DrawFormatString(x,y,"Patch Games");
			y += CHARACTER_HEIGHT;

			bg_colour = (menu_arrow == 0) ? SELECTED_FONT_BG_COLOUR : UNSELECTED_FONT_BG_COLOUR;
			SetFontColor(SELECTABLE_NORMAL_FONT_COLOUR, bg_colour);
			DrawFormatString(x,y,"Edit Title id: ");
			DrawFormatString(GetFontX(),y,global_title_id);
			y += CHARACTER_HEIGHT;

			bg_colour = (menu_arrow == 1) ? SELECTED_FONT_BG_COLOUR : UNSELECTED_FONT_BG_COLOUR;
			font_colour = (normalise_digest_checked) ? TURNED_ON_FONT_COLOUR : SELECTABLE_NORMAL_FONT_COLOUR;
			SetFontColor(font_colour, bg_colour);
			DrawFormatString(x,y,"Normalise digest (select if debug build or previously patched by Refresher)");
			y += CHARACTER_HEIGHT;

			bg_colour = (menu_arrow == 2) ? SELECTED_FONT_BG_COLOUR : UNSELECTED_FONT_BG_COLOUR;
			SetFontColor(SELECTABLE_NORMAL_FONT_COLOUR, bg_colour);
			DrawFormatString(x,y,"Browse games for Title id");
			y += CHARACTER_HEIGHT;

			bg_colour = (menu_arrow == 3) ? SELECTED_FONT_BG_COLOUR : UNSELECTED_FONT_BG_COLOUR;
			SetFontColor(SELECTABLE_NORMAL_FONT_COLOUR, bg_colour);
			DrawFormatString(x,y,"Revert patches");
			y += CHARACTER_HEIGHT;

			bg_colour = (menu_arrow == 4) ? SELECTED_FONT_BG_COLOUR : UNSELECTED_FONT_BG_COLOUR;
			SetFontColor(SELECTABLE_NORMAL_FONT_COLOUR, bg_colour);
			DrawFormatString(x,y,"Join key: ");
			if (join_password[0] == 0) {
				DrawFormatString(GetFontX(),y,"Randomised (no one can join you)");
			}
			else {
				DrawFormatString(GetFontX(),y,join_password);
			}
			y += CHARACTER_HEIGHT;

			bg_colour = (menu_arrow == 5) ? SELECTED_FONT_BG_COLOUR : UNSELECTED_FONT_BG_COLOUR;
			SetFontColor(SELECTABLE_NORMAL_FONT_COLOUR, bg_colour);
			DrawFormatString(x,y,"Patch!");
			y += CHARACTER_HEIGHT;

			break;
		case MENU_SELECT_URLS:
		case MENU_EDIT_URLS:
			switch (current_menu) {
				case MENU_SELECT_URLS:
					DrawFormatString(x,y,"Select URL (Page %d/99)",saved_urls_txt_num);
					break;
				case MENU_EDIT_URLS:
					DrawFormatString(x,y,"Edit URLs (Page %d/99)",saved_urls_txt_num);
					break;
			}

			y += CHARACTER_HEIGHT;

			int i = 0;
			int current_url_entry_index;
			struct UrlToPatchTo url_entry;
			int full_text_len;
			int new_max_capitial_w_characters_per_line;
			int temp_new_x_len;
			//int temp_new_y_len;
			int i_stop = saved_urls_count;


			while (i < i_stop) {
				current_url_entry_index = i;
				url_entry = saved_urls[current_url_entry_index];



				bg_colour = (menu_arrow == i) ? SELECTED_FONT_BG_COLOUR : UNSELECTED_FONT_BG_COLOUR;
				font_colour = (current_menu == MENU_SELECT_URLS && selected_url_index == i) ? TURNED_ON_FONT_COLOUR : SELECTABLE_NORMAL_FONT_COLOUR;
				SetFontColor(font_colour, bg_colour);


				full_text_len = strlen(url_entry.url) + 1 + strlen(url_entry.digest);
				if (full_text_len > MAX_CAPITIAL_W_CHARACTERS_PER_LINE) {
					new_max_capitial_w_characters_per_line = MAX_CAPITIAL_W_CHARACTERS_PER_LINE;
					temp_new_x_len = NORMAL_TEXT_X;
					//temp_new_y_len = NORMAL_TEXT_Y;
					while (full_text_len > new_max_capitial_w_characters_per_line) {
						new_max_capitial_w_characters_per_line += NEW_LINES_AMNT_PER_DIGIT_OF_X_INCREASE;
						temp_new_x_len -= 1;
						//temp_new_y_len -= 1;
					}
					SetFontSize(temp_new_x_len, NORMAL_TEXT_Y);
				}


				DrawFormatString(x,y,"%s",url_entry.url);
				y += CHARACTER_HEIGHT;


				SetFontSize(NORMAL_TEXT_X,NORMAL_TEXT_Y);
				i++;
			}

			if (current_menu == MENU_EDIT_URLS) {
				bool max_urls_reached = saved_urls_count >= MAX_SAVED_URLS_AMNT;
				bg_colour = (menu_arrow == i) ? SELECTED_FONT_BG_COLOUR : UNSELECTED_FONT_BG_COLOUR;
				font_colour = SELECTABLE_NORMAL_FONT_COLOUR;
				if (max_urls_reached) {
					font_colour = TITLE_FONT_COLOUR;
					bg_colour = TITLE_BG_COLOUR;
				}
				SetFontColor(font_colour, bg_colour);
				if (max_urls_reached) {

				}
				else {
					DrawFormatString(x,y,"Add new URL");
				}
			}

			if (current_menu == MENU_SELECT_URLS) {
				bg_colour = (menu_arrow == i) ? SELECTED_FONT_BG_COLOUR : UNSELECTED_FONT_BG_COLOUR;
				font_colour = SELECTABLE_NORMAL_FONT_COLOUR;
				SetFontColor(font_colour, bg_colour);
				DrawFormatString(x,y,"Reset URLs");
			}

			break;
		case MENU_URL_EDITOR:
			DrawFormatString(x,y,"Server URL Editor");
			y += CHARACTER_HEIGHT;
			bg_colour = (menu_arrow == 0) ? SELECTED_FONT_BG_COLOUR : UNSELECTED_FONT_BG_COLOUR;
			SetFontColor(SELECTABLE_NORMAL_FONT_COLOUR, bg_colour);
			DrawFormatString(x,y,"URL: %s",saved_urls[selected_url_index].url);
			y += CHARACTER_HEIGHT;
			bg_colour = (menu_arrow == 1) ? SELECTED_FONT_BG_COLOUR : UNSELECTED_FONT_BG_COLOUR;
			SetFontColor(SELECTABLE_NORMAL_FONT_COLOUR, bg_colour);
			DrawFormatString(x,y,"Digest (put CustomServerDigest if Refresh based server, otherwise leave empty): %s",saved_urls[selected_url_index].digest);

			y += CHARACTER_HEIGHT;

			for (int i = 0; i < method_count; i++) {
				font_colour = (strcmp(saved_urls[selected_url_index].patch_name,patch_lua_names[i].patch_name) == 0) ? TURNED_ON_FONT_COLOUR : SELECTABLE_NORMAL_FONT_COLOUR;
				bg_colour = (menu_arrow-MINUS_MENU_ARROW_AMNT_URL_EDITOR_TO_GET_PATCH_LUA_INDEX == i) ? SELECTED_FONT_BG_COLOUR : UNSELECTED_FONT_BG_COLOUR;
				SetFontColor(font_colour, bg_colour);
				SetFontColor(font_colour, bg_colour);

				DrawFormatString(x,y,"Patch method: %s",patch_lua_names[i].patch_method);
				y += CHARACTER_HEIGHT;
			}

			break;
		case MENU_BROWSE_GAMES:
			DrawFormatString(x,y,"Browse games! Title id: %s",global_title_id);
			y += CHARACTER_HEIGHT;

			for (int i = 0; i < browse_games_buffer_size; i++) {
				bg_colour = (menu_arrow == (i + browse_games_buffer_start)) ? SELECTED_FONT_BG_COLOUR : UNSELECTED_FONT_BG_COLOUR;
				font_colour = (strcmp(global_title_id,browse_games_buffer[i].title_id) == 0) ? TURNED_ON_FONT_COLOUR : SELECTABLE_NORMAL_FONT_COLOUR;
				SetFontColor(font_colour, bg_colour);
				DrawFormatString(x,y,"%s %s",browse_games_buffer[i].title_id,browse_games_buffer[i].game_name);
				y += CHARACTER_HEIGHT;
			}

			break;
	}

}

// Source - https://stackoverflow.com/questions/15821123/removing-elements-from-an-array-in-c
// Posted by Ben, modified by community. See post 'Timeline' for change history
// Retrieved 2025-12-13, License - CC BY-SA 4.0

void remove_element(struct UrlToPatchTo *array, int index, int array_length)
{
   int i;
   for(i = index; i < array_length - 1; i++) array[i] = array[i + 1];
}


// Source - https://stackoverflow.com/questions/1726302/remove-spaces-from-a-string-in-c
// Posted by Aaron, modified by community. See post 'Timeline' for change history
// Retrieved 2025-12-08, License - CC BY-SA 4.0

void remove_spaces(char* s) {
    char* d = s;
    do {
        while (*d == ' ') {
            ++d;
        }
    } while ((*s++ = *d++));
}


bool is_valid_title_id(char* title_id) // assumes its uppercase
{
	if(strlen(title_id) != 9) {
		return 0;
	}
	for (int i = 0; i < 4; i++) {
		if(!isalpha(title_id[i])) {
			return 0;
		}
	}
	for (int i = 5; i < 9; i++) {
		if(!(title_id[i] >= '0' && title_id[i] <= '9')) {
			return 0;
		}
	}

	return 1;
}

int save_user_join_pwd(const char * pretty_user_input_join_password) {
	FILE *fp = fopen(JOIN_PASSWORD_TXT, "wb");
	if (fp == 0) {
		return -1;
	}

	if (pretty_user_input_join_password[0] == 0) {
		fclose(fp);
		return 0;
	}

	fputs(pretty_user_input_join_password, fp);
	fclose(fp);
	return 0;
}

void load_user_join_pwd(char * pretty_user_input_join_password) {
	memset(pretty_user_input_join_password,0,4096+1);
	FILE *fp = fopen(JOIN_PASSWORD_TXT, "rb");
	if (fp == 0) {
		return;
	}

	fread(pretty_user_input_join_password,1,4096,fp);

	if (strlen(pretty_user_input_join_password) < 8) {
		memset(pretty_user_input_join_password,0,4096+1);
		return;
	}

	fclose(fp);
}

int save_global_title_id_to_disk() {
	FILE *fp = fopen(TITLE_ID_TXT, "wb");
	if (fp == 0) {
		return -1;
	}

	fwrite(global_title_id,1,sizeof(global_title_id)-1,fp);
	fclose(fp);
	return 0;
}

void load_global_title_id() {
	memset(global_title_id,0,sizeof(global_title_id));
	FILE *fp = fopen(TITLE_ID_TXT, "rb");
	if (fp == 0) {
		goto fail_to_load_title_id;
	}
	fseek(fp, 0, SEEK_END);
	if (ftell(fp) > 9) {
		char trailing_char[1];
		fseek(fp, 9, SEEK_SET);
		fread(trailing_char,1,sizeof(trailing_char),fp);
		if (!(isspace(trailing_char[0]))) {
			fclose(fp);
			goto fail_to_load_title_id;
		}
	}
	rewind(fp);

	fread(global_title_id,1,sizeof(global_title_id)-1,fp);

	for (int i = 0; global_title_id[i] != '\0'; i++) {
		global_title_id[i] = toupper(global_title_id[i]);
	}

	if (!is_valid_title_id(global_title_id)) {
		fclose(fp);
		goto fail_to_load_title_id;
	}

	fclose(fp);
	return;

	fail_to_load_title_id:
	strcpy(global_title_id,DEFAULT_TITLE_ID);
	save_global_title_id_to_disk();
	return;

}

bool is_a_url_selected() {
	if (selected_url_index < 0 ) {
		return 0;
	}
	if (selected_url_index > saved_urls_count-1 ) {
		return 0;
	}

	return 1;
}

void write_saved_urls(u8 saved_urls_txt_num) {
	struct UrlToPatchTo url_entry;
	char write_buffer[sizeof(url_entry.url) + 1 + sizeof(url_entry.digest) + 1 + sizeof(url_entry.patch_name) + 1 + 1];


	char filename[sizeof(SAVED_URLS_TXT_FIRST_HALF) + (sizeof("_ff")-1) + sizeof(SAVED_URLS_TXT_SECOND_HALF)];
	sprintf(filename,"%s_%d%s",SAVED_URLS_TXT_FIRST_HALF,saved_urls_txt_num,SAVED_URLS_TXT_SECOND_HALF);

	FILE *fp = fopen(filename, "wb+"); // not checking if it fails to open, just let it segfault, cause theres bigger problems if it doesnt works
	for (int i = 0; i < saved_urls_count; i++) {
		url_entry = saved_urls[i];
		if (url_entry.digest[0] != 0) {
			sprintf(write_buffer,"%s %s %s\n",url_entry.url,url_entry.patch_name,url_entry.digest);
		}
		else {
			sprintf(write_buffer,"%s %s\n",url_entry.url,url_entry.patch_name);
		}

		fprintf(fp,write_buffer);
	}
	fclose(fp);

}

void load_saved_urls(u8 saved_urls_txt_num) {
    char * line = NULL;
    size_t len = 0;
    ssize_t len_of_line;

	char filename[sizeof(SAVED_URLS_TXT_FIRST_HALF) + (sizeof("_ff")-1) + sizeof(SAVED_URLS_TXT_SECOND_HALF)];
	sprintf(filename,"%s_%d%s",SAVED_URLS_TXT_FIRST_HALF,saved_urls_txt_num,SAVED_URLS_TXT_SECOND_HALF);

	FILE *fp = fopen(filename, "ab+"); // not checking if it fails to open, just let it segfault, cause theres bigger problems if it doesnt works
    rewind(fp);
	char *current_line_part;
	int current_index_to_copy_to;
	int ready_url_i = 0;
	saved_urls_count = 0;
	while ((len_of_line = __getline(&line, &len, fp)) > 0) {
        // TODO perhaps remove all leading and trailing whitespace from the line, similar to python str.strip methond
		line[strcspn(line, "\r\n")] = 0;
		len_of_line = strlen(line);
		if (!line[0]) {
			continue;
		}

		// remove any extra chars
		if (len_of_line > MAX_LINE_LEN_OF_URL_ENTRY) {
			line[MAX_LINE_LEN_OF_URL_ENTRY] = 0;
			len_of_line = MAX_LINE_LEN_OF_URL_ENTRY;
		}

		char temp_url[MAX_LINE_LEN_OF_URL_ENTRY + 1];
		char temp_digest[MAX_LINE_LEN_OF_URL_ENTRY + 1];
		char temp_patch_method[MAX_LINE_LEN_OF_URL_ENTRY + 1];
		current_line_part = strtok(line, " ");
		current_index_to_copy_to = 0;
		while (current_line_part != NULL) {
			current_index_to_copy_to++;
			if (current_index_to_copy_to > 3) {
				break;
			}
			if (current_index_to_copy_to == 1) {
				strcpy(temp_url,current_line_part);
			}
			else if (current_index_to_copy_to == 2) {
				strcpy(temp_patch_method,current_line_part);
			}
			else if (current_index_to_copy_to == 3) {
				strcpy(temp_digest,current_line_part);
			}

			current_line_part = strtok(NULL, " ");
		}

		if (current_index_to_copy_to == 2) {
			strcpy(temp_digest,"");
		}
		else if (current_index_to_copy_to != 3) {
			continue;
		}


		if (strlen(temp_url) > sizeof(saved_urls[0].url)-1) {
			temp_url[sizeof(saved_urls[0].url)-1] = 0;
		}
		if (strlen(temp_digest) > sizeof(saved_urls[0].digest)-1) {
			temp_digest[sizeof(saved_urls[0].digest)-1] = 0;
		}
		if (strlen(temp_patch_method) > sizeof(saved_urls[0].patch_name)-1) {
			temp_patch_method[sizeof(saved_urls[0].patch_name)-1] = 0;
		}

		if (strcmp(temp_url,"ENTER_A_URL_HERE") == 0) {
			continue;
		}

		strcpy(saved_urls[ready_url_i].url,temp_url);
		strcpy(saved_urls[ready_url_i].digest,temp_digest);
		strcpy(saved_urls[ready_url_i].patch_name,temp_patch_method);
		saved_urls_count++;


		ready_url_i++;
		if (ready_url_i >= MAX_SAVED_URLS_AMNT) {
			break;
		}

    }



	fclose(fp);
	free(line);

}

int set_arrow(int menu_arrow,unsigned btn_pressed, int max_arrow)
{
	int new_arrow = menu_arrow;

	if (btn_pressed & BTN_UP) {
		new_arrow--;
	}
	else if (btn_pressed & BTN_DOWN) {
		new_arrow++;
	}

	if (new_arrow < 0) {
		new_arrow = max_arrow;
	}
	else if (new_arrow > max_arrow) {
		new_arrow = 0;
	}
	return new_arrow;
}

bool title_id_exists(char * title_id)
{
	char fname[sizeof("/dev_hdd0/game/ABCD12345/USRDIR/EBOOT.BIN")];
	sprintf(fname,"/dev_hdd0/game/%s/USRDIR/EBOOT.BIN",title_id); // assumes that title_id is of lenght 9

	return does_file_exist(fname);
}

u32 total_count_of_patchable_games(u32 start_offset, u32 end_length)
{
	assert(end_length > 0);
	u32 total_count = 0;
	u32 start_counting = 0;
	DIR *game_dir = opendir("/dev_hdd0/game");
	struct dirent* reader;
	if (game_dir != NULL) {
		while ((reader = readdir(game_dir)) != NULL) {
			if (strcmp(reader->d_name,".") == 0 || strcmp(reader->d_name,"..") == 0) {
				continue;
			}
			if (!is_valid_title_id(reader->d_name)) {
				continue;
			}
			if (!title_id_exists(reader->d_name)) {
				continue;
			}

			if (start_counting >= start_offset) {
				total_count++;
				if (total_count >= end_length) {
					break;
				}
			}
			else {
				start_counting++;
			}

		}
		closedir(game_dir);
	}
	return total_count;
}

u32 load_patchable_games(struct TitleIdAndGameName buffer[], u32 start_offset, u32 end_length)
{
	assert(end_length > 0);
	u32 total_count = 0;
	u32 start_counting = 0;
	DIR *game_dir = opendir("/dev_hdd0/game");
	struct dirent* reader;
	char * game_name;
	char param_sfo_path[sizeof("/dev_hdd0/game/ABCD12345/PARAM.SFO")];
	if (game_dir != NULL) {
		while ((reader = readdir(game_dir)) != NULL) {
			if (strcmp(reader->d_name,".") == 0 || strcmp(reader->d_name,"..") == 0) {
				continue;
			}
			if (!is_valid_title_id(reader->d_name)) {
				continue;
			}
			if (!title_id_exists(reader->d_name)) {
				continue;
			}

			if (start_counting >= start_offset) {

				strcpy(buffer[total_count].title_id,reader->d_name);
				sprintf(param_sfo_path,"/dev_hdd0/game/%s/PARAM.SFO",reader->d_name); // ignore the warning on this line, we already ensured that the folder name is 9 chars long
				game_name = get_title_id_from_param(param_sfo_path);
				if (game_name == 0) {
					strcpy(buffer[total_count].game_name,"Unknown??");
				}
				else {
					strcpy(buffer[total_count].game_name,game_name);
					free(game_name);
				}

				total_count++;
				if (total_count >= end_length) {
					break;
				}
			}
			else {
				start_counting++;
			}

		}
		closedir(game_dir);
	}
	return total_count;
}

void reset_all_urls() {
	DIR *dir_with_saved_urls = opendir(ROOT_DIR);
	struct dirent* reader;
	char full_path_src[1024 + strlen(ROOT_DIR)];
	if (dir_with_saved_urls != NULL) {
		while ((reader = readdir(dir_with_saved_urls)) != NULL) {
			if (strcmp(reader->d_name,".") == 0 || strcmp(reader->d_name,"..") == 0) {
				continue;
			}
			if (strncmp(reader->d_name, SAVED_URLS_TXT_STRING_STARTER, strlen(SAVED_URLS_TXT_STRING_STARTER)) == 0) {
				snprintf(full_path_src, sizeof(full_path_src), "%s%s", ROOT_DIR, reader->d_name);
				remove(full_path_src);
			}

		}
		closedir(dir_with_saved_urls);
	}
	FILE *fp = fopen(NEW_NUM_1_SAVED_URLS_TXT,"wb");
	if (fp == 0) {
		return;
	}
	fwrite(DEFAULT_URLS,1,sizeof(DEFAULT_URLS)-1,fp);
	fclose(fp);
}

int save_patch_cache_bool(bool use_patch_cache) {
	if (use_patch_cache) {
		FILE *fp = fopen(PATCH_CACHE_FILE_EXISTS_THEN_TRUE, "wb");
		if (fp == 0) {
			return -1;
		}
		fprintf(fp,"1");
		fclose(fp);
		return 0;
	}
	else {
		DIR *cache_dir_fp = opendir(CACHE_DIR);
		struct dirent* reader;
		char full_path[1024];
		if (cache_dir_fp == NULL) {
			return -1;
		}
		while ((reader = readdir(cache_dir_fp)) != NULL) {
			if (strcmp(reader->d_name,".") == 0 || strcmp(reader->d_name,"..") == 0) {
				continue;
			}
			snprintf(full_path, sizeof(full_path), "%s%s", CACHE_DIR, reader->d_name);
			remove(full_path);
		}
		closedir(cache_dir_fp);
		return 0;
	}
}

int revert_eboot(char * title_id)
{
	char eboot_bin_bak_file[sizeof("/dev_hdd0/game/ABCD12345/USRDIR/EBOOT.BIN.BAK")];
	char eboot_bin_orig_file[sizeof("/dev_hdd0/game/ABCD12345/USRDIR/EBOOT.BIN.ORIG")];
	char output_eboot_bin_file[sizeof("/dev_hdd0/game/ABCD12345/USRDIR/EBOOT.BIN")];

	sprintf(eboot_bin_bak_file,"/dev_hdd0/game/%s/USRDIR/EBOOT.BIN.BAK",title_id);
	sprintf(output_eboot_bin_file,"/dev_hdd0/game/%s/USRDIR/EBOOT.BIN",title_id);
	sprintf(eboot_bin_orig_file,"/dev_hdd0/game/%s/USRDIR/EBOOT.BIN.ORIG",title_id);

	// try copying from EBOOT.BIN.ORIG file
	if (does_file_exist(eboot_bin_orig_file)) {
		return copy_file(output_eboot_bin_file,eboot_bin_orig_file);
	}
	// then just copy from EBOOT.BIN.BAK file if EBOOT.BIN.ORIG does not exist
	else {
		return copy_file(output_eboot_bin_file,eboot_bin_bak_file);
	}
}

void revert_eboot_thread(void *arg)
{
	int revert_eboot_copy_file_res;
	struct SecondThreadArgs *args = arg;

	revert_eboot_copy_file_res = revert_eboot(args->title_id);

	args->has_finished = 1;
	if (revert_eboot_copy_file_res == -1) {
		sysThreadExit(THREAD_RET_EBOOT_BAK_NO_EXIST);
	}
	else {
		sysThreadExit(THREAD_RET_EBOOT_REVERTED);
	}
}

void patch_eboot_thread(void *arg)
{
	struct SecondThreadArgs *args = arg;
	struct UrlToPatchTo my_url = saved_urls[selected_url_index];

	args->current_state = THREAD_CURRENT_STATE_CLEANING_WORKSPACE;
	mkdir(WORKING_DIR, 0777);
	remove(WORKING_DIR "EBOOT.ELF");

	char dst_file[sizeof("/dev_hdd0/game/ABCD12345/USRDIR/EBOOT.BIN.BAK")];
	char src_file[sizeof("/dev_hdd0/game/ABCD12345/USRDIR/EBOOT.BIN")];
	char dst_file_refresh_orig[sizeof("/dev_hdd0/game/ABCD12345/USRDIR/EBOOT.BIN.ORIG")];
	int copy_file_res;

	char out_bin[sizeof("/dev_hdd0/game/ABCD12345/USRDIR/EBOOT.BIN")];
	sprintf(out_bin,"/dev_hdd0/game/%s/USRDIR/EBOOT.BIN",args->title_id);

	char lua_func_name[PATCH_LUA_SIZE + 1 + sizeof("patch_")];

	// variables only use if use_patch_cache
	int line_num_of_caches = 0;
	char * current_cache_line;
	FILE *fp;
	//
	if (args->use_patch_cache) {
		int current_cache_line_len = strlen(my_url.url) + strlen(my_url.digest) + strlen(args->title_id) + strlen(args->patch_lua_name) + strlen(args->join_password);
		// TODO memory leak here, but this can only happen once so it does not really matter
		current_cache_line = malloc(current_cache_line_len + 1);
		if (!current_cache_line) {
			args->has_finished = 1;
			sysThreadExit(THREAD_RET_EBOOT_BACKUP_FAILED);
		}
		snprintf(current_cache_line,current_cache_line_len + 1,"%s%s%s%s%s",my_url.url,my_url.digest,args->title_id,args->patch_lua_name,args->join_password);
		dbglogger_log("Checking if theres a cache for %s",current_cache_line);

		fp = fopen(CACHE_TXT_FILE, "ab+");
		if (fp == 0) {
			args->has_finished = 1;
			sysThreadExit(THREAD_RET_EBOOT_BACKUP_FAILED);
		}
		rewind(fp);

		char * line = NULL;
		size_t len = 0;
		ssize_t len_of_line;

		while ((len_of_line = __getline(&line, &len, fp)) > 0) {
			if (len_of_line > 1000) {
				goto continue_and_incr_line_num;
			}
			line[strcspn(line, "\r\n")] = 0;
			len_of_line = strlen(line);
			if (!line[0]) {
				goto continue_and_incr_line_num;
			}
			if (len_of_line != current_cache_line_len) {
				goto continue_and_incr_line_num;
			}
			if (memcmp(current_cache_line,line,current_cache_line_len) == 0) {
				fclose(fp);
				args->current_state = THREAD_CURRENT_STATE_RESTORING_EBOOT_BIN_BAK;
				dbglogger_log("Cache hit, restoring eboot from cache");
				char cache_eboot_bin_file[sizeof(CACHE_DIR "eboot_bin_ps3_num_9999.bin")];
				sprintf(cache_eboot_bin_file,CACHE_DIR "eboot_bin_ps3_num_%d.bin",line_num_of_caches);
				copy_file_res = copy_file(out_bin,cache_eboot_bin_file);
				if (copy_file_res == -1) {
					args->has_finished = 1;
					sysThreadExit(THREAD_RET_EBOOT_BACKUP_FAILED);
				}
				args->has_finished = 1;
				sysThreadExit(THREAD_RET_EBOOT_PATCHED);
				// never will reach here
			}
			continue_and_incr_line_num:
			line_num_of_caches++;
		}
		fclose(fp);
	}


	sprintf(dst_file,"/dev_hdd0/game/%s/USRDIR/EBOOT.BIN.BAK",args->title_id);
	sprintf(src_file,"/dev_hdd0/game/%s/USRDIR/EBOOT.BIN",args->title_id);
	sprintf(dst_file_refresh_orig,"/dev_hdd0/game/%s/USRDIR/EBOOT.BIN.ORIG",args->title_id);

	// only backup eboot if eboot.bin.bak dont exist
	if ((!does_file_exist(dst_file)) && (!does_file_exist(dst_file_refresh_orig))) {
		// backup EBOOT.BIN later
	}
	else {
		args->current_state = THREAD_CURRENT_STATE_RESTORING_EBOOT_BIN_BAK;
		dbglogger_log("Restoring from EBOOT.BIN.BAK file");
		copy_file_res = revert_eboot(args->title_id);
		if (copy_file_res == -1) {
			args->has_finished = 1;
			sysThreadExit(THREAD_RET_EBOOT_BACKUP_FAILED);
		}
	}

	char out_and_in_elf[] = WORKING_DIR "EBOOT.ELF";

	char out_and_in_bin[sizeof("/dev_hdd0/game/ABCD12345/USRDIR/EBOOT.BIN")];
	sprintf(out_and_in_bin,"/dev_hdd0/game/%s/USRDIR/EBOOT.BIN",args->title_id);

	/* for whatever reason, oscetool shits itself when you use long args,
	probably just me being stupid and not knoinwg how to make an agrv but for now im using short ones*/
	char tmp_arg1_for_dec[] = "L";
	char tmp_arg2_for_dec[] = "-v";
	char tmp_arg3_for_dec[] = "-d";

	char *argv_for_dec[] = { tmp_arg1_for_dec, tmp_arg2_for_dec, tmp_arg3_for_dec, out_and_in_bin, out_and_in_elf, NULL };

	args->current_state = THREAD_CURRENT_STATE_DECRYPTING_EBOOT_BIN;
	run_scetool(5,argv_for_dec);

	if (!does_file_exist(WORKING_DIR "EBOOT.ELF")) {
		dbglogger_log("oscetool decrypt failed, trying unself");
		// dont give up just yet!
		char *argv_for_unself[] = {"L",out_and_in_bin,WORKING_DIR "EBOOT.ELF", NULL};
		args->current_state = THREAD_CURRENT_STATE_UNSELF_EBOOT_BIN;
		unself_main(3,argv_for_unself);


		if (!does_file_exist(WORKING_DIR "EBOOT.ELF")) {
			args->has_finished = 1;
			sysThreadExit(THREAD_RET_EBOOT_DECRYPT_FAILED);
		}
		global_is_digital_eboot = 1;
	}
	args->current_state = THREAD_CURRENT_STATE_START_PATCHING;
	dbglogger_log("start patching");
	sprintf(lua_func_name,"patch_%s",args->patch_lua_name);

	// theese lua things might be unpure, something about me needing to pop the values after, but since it will always end after this idrc to do it
    lua_getglobal(L, lua_func_name);

    if (!lua_isfunction(L, -1)) {
        args->has_finished = 1;
		sysThreadExit(THREAD_RET_EBOOT_PATCH_FAILED);
    }

	lua_pushstring(L,WORKING_DIR "EBOOT.ELF");
	lua_pushstring(L,my_url.url);
	lua_pushstring(L,my_url.digest);
	lua_pushboolean(L,args->normalise_digest);
	lua_pushstring(L,WORKING_DIR);
	lua_pushstring(L,args->join_password);

    if (lua_pcall(L, 6, 1, 0) != LUA_OK) {
        // gonna pop the error later
		// dbglogger_log("Error calling function: %s", lua_tostring(L, -1));
		args->has_finished = 1;
		sysThreadExit(THREAD_RET_EBOOT_PATCH_FAILED);
    }
	// its now assumed that if patching went wrong, the lua code will throw an error

	args->current_state = THREAD_CURRENT_STATE_DONE_PATCHING;
	dbglogger_log("done patching");
	char input_elf_to_be_enc[] = WORKING_DIR "EBOOT.ELF";

	// only backup eboot if eboot.bin.bak dont exist
	if ((!does_file_exist(dst_file)) && (!does_file_exist(dst_file_refresh_orig))) {
		args->current_state = THREAD_CURRENT_STATE_MAKING_EBOOT_BIN_BAK;
		dbglogger_log("No EBOOT.BIN.BAK was found, so making one");
		copy_file_res = copy_file(dst_file,src_file);

		if (copy_file_res == -1) {
			args->has_finished = 1;
			sysThreadExit(THREAD_RET_EBOOT_BACKUP_FAILED);
		}
	}

	remove(out_bin); // remove the old EBOOT.BIN to ensure that it actually encrypted the file
	// some of the args in argv is not getting loaded, refer to oscetool_main.c for more info
	if (global_is_digital_eboot) {
		args->current_state = THREAD_CURRENT_ENCRYPTING_FOR_DIGITAL;
		dbglogger_log("Encrypting for digital");
		char tmp_arg1_for_enc[] = "L";
		char tmp_arg2_for_enc[] = "-v";
		char tmp_arg3_for_enc[] = "-0=SELF";
		char tmp_arg4_for_enc[] = "-s=FALSE";
		char tmp_arg5_for_enc[] = "-7=TRUE";
		char tmp_arg6_for_enc[] = "-1=TRUE";
		char tmp_arg7_for_enc[] = "-2=0A";
		char tmp_arg8_for_enc[] = "-A=0001000000000000";
		char tmp_arg9_for_enc[] = "-3=1010000001000003";
		char tmp_arg10_for_enc[] = "-4=01000002";
		char tmp_arg11_for_enc[] = "-8=0000000000000000000000000000000000000000000000000000000000000000";
		char tmp_arg12_for_enc[] = "-9=00000000000000000000000000000000000000000000003B0000000100040000";
		char tmp_arg13_for_enc[] = "-5=NPDRM";
		char tmp_arg14_for_enc[] = "-6=0003005500000000";
		char tmp_arg15_for_enc[] = "-b=FREE";
		char tmp_arg16_for_enc[] = "-c=SPRX";

		char tmp_arg18_for_enc[] = "-g=EBOOT.BIN";
		char tmp_arg19_for_enc[] = "-e";

		char tmp_arg17_for_enc[sizeof("-f=") + 128];
		sprintf(tmp_arg17_for_enc,"-f=%s",global_content_id);
		char *argv_for_enc[] = {
			tmp_arg1_for_enc,
			tmp_arg2_for_enc,
			tmp_arg3_for_enc,
			tmp_arg4_for_enc,
			tmp_arg5_for_enc,
			tmp_arg6_for_enc,
			tmp_arg7_for_enc,
			tmp_arg8_for_enc,
			tmp_arg9_for_enc,
			tmp_arg10_for_enc,
			tmp_arg11_for_enc,
			tmp_arg12_for_enc,
			tmp_arg13_for_enc,
			tmp_arg14_for_enc,
			tmp_arg15_for_enc,
			tmp_arg16_for_enc,
			tmp_arg17_for_enc,
			tmp_arg18_for_enc,
			tmp_arg19_for_enc,
			input_elf_to_be_enc,
			out_bin,
			NULL
		};

		run_scetool(19+2,argv_for_enc);
	}
	else {
		args->current_state = THREAD_CURRENT_ENCRYPTING_FOR_DISC;
		dbglogger_log("Encrypting for disc");
		char tmp_arg1_for_disc_enc[] = "L";
		char tmp_arg2_for_disc_enc[] = "-v";
		char tmp_arg3_for_disc_enc[] = "-0=SELF";
		char tmp_arg4_for_disc_enc[] = "-s=FALSE";
		char tmp_arg5_for_disc_enc[] = "-2=0A";
		char tmp_arg6_for_disc_enc[] = "-A=0001000000000000";
		char tmp_arg7_for_disc_enc[] = "-3=1010000001000003";
		char tmp_arg8_for_disc_enc[] = "-4=01000002";
		char tmp_arg9_for_disc_enc[] = "-8=0000000000000000000000000000000000000000000000000000000000000000";
		char tmp_arg10_for_disc_enc[] = "-9=00000000000000000000000000000000000000000000003B0000000100040000";
		char tmp_arg11_for_disc_enc[] = "-5=APP";
		char tmp_arg12_for_disc_enc[] = "-6=0003005500000000";
		char tmp_arg13_for_disc_enc[] = "-1=TRUE";
		char tmp_arg14_for_disc_enc[] = "-e";

		char *argv_for_disc_enc[] = {
			tmp_arg1_for_disc_enc,
			tmp_arg2_for_disc_enc,
			tmp_arg3_for_disc_enc,
			tmp_arg4_for_disc_enc,
			tmp_arg5_for_disc_enc,
			tmp_arg6_for_disc_enc,
			tmp_arg7_for_disc_enc,
			tmp_arg8_for_disc_enc,
			tmp_arg9_for_disc_enc,
			tmp_arg10_for_disc_enc,
			tmp_arg11_for_disc_enc,
			tmp_arg12_for_disc_enc,
			tmp_arg13_for_disc_enc,
			tmp_arg14_for_disc_enc,
			input_elf_to_be_enc,
			out_bin,
			NULL
		};

		run_scetool(14+2,argv_for_disc_enc);
	}

	if (!does_file_exist(out_bin)) {
		revert_eboot(args->title_id);
		args->has_finished = 1;
		sysThreadExit(THREAD_RET_EBOOT_DECRYPT_FAILED);
	}

	if (args->use_patch_cache) {
		// it is assumed at this point there is not cache for this eboot as thats checked for eariler
		char cache_eboot_bin_file[sizeof(CACHE_DIR "eboot_bin_ps3_num_9999.bin")];
		sprintf(cache_eboot_bin_file,CACHE_DIR "eboot_bin_ps3_num_%d.bin",line_num_of_caches);

		dbglogger_log("Making cache copy of %s to %s",current_cache_line,cache_eboot_bin_file);

		copy_file_res = copy_file(cache_eboot_bin_file,out_bin);
		if (copy_file_res == -1) {
			args->has_finished = 1;
			sysThreadExit(THREAD_RET_EBOOT_BACKUP_FAILED);
		}
		fp = fopen(CACHE_TXT_FILE, "ab+");
		if (fp == 0) {
			args->has_finished = 1;
			sysThreadExit(THREAD_RET_EBOOT_BACKUP_FAILED);
		}
		fprintf(fp,current_cache_line); fprintf(fp,"\n");

		fclose(fp);
	}

	args->has_finished = 1;
	sysThreadExit(THREAD_RET_EBOOT_PATCHED);
}

void check_for_updates_thread(void *arg) {
	struct SecondThreadArgs *args = arg;

	int http_init_ret = http_init();
	if (http_init_ret != HTTP_SUCCESS) {
		dbglogger_log("http_init failed with %d",http_init_ret);
		args->has_finished = 1;
		sysThreadExit(0);
	}

	// quick way of not needing to parse json
	// always assuming that it will start with [{"name":"v0.000
	char newest_version_tag_buffer[sizeof("[{\"name\":\"v0.000")] = {0};


	http_init_ret = http_download_to_buffer(
		"https://api.github.com/repos/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/tags",
		"",
		newest_version_tag_buffer,
		sizeof(newest_version_tag_buffer)-1,
		HTTP_ALLOW_SMALLER_BUFFER_SIZE
	);

	if (http_init_ret == HTTP_SUCCESS) {
		strcpy(global_newest_version_tag,newest_version_tag_buffer + strlen("[{\"name\":\""));
		dbglogger_log("global_newest_version_tag = %s",global_newest_version_tag);
	}
	else {
		dbglogger_log("Failed to get newest tag from https github 0x%x",http_init_ret);
		args->has_finished = 1;
		sysThreadExit(0);
	}

	// TODO double check if version is newest or not
	if (strcmp(VERSION_NUM_STR,global_newest_version_tag) == 0) {
		args->has_finished = 1;
		sysThreadExit(0);
	}

	http_init_ret = http_download(UPDATE_DOWNLOAD_LINK,"",UPDATE_LOCATION);
	if (http_init_ret == HTTP_SUCCESS) {
		args->has_finished = 1;
		sysThreadExit(0);
	}
	else {
		args->has_finished = 1;
		sysThreadExit(THREAD_RET_COULD_NOT_DOWNLOAD_LATEST_PKG);
	}
}

s32 main(s32 argc, const char* argv[])
{
	//dbglogger_init();

	// init the global second_thread_args
	second_thread_args.has_finished = 0;
	second_thread_args.current_state = 0;
	second_thread_args.normalise_digest = 1;
	memset(second_thread_args.patch_lua_name,0,sizeof(second_thread_args.patch_lua_name));
	second_thread_args.title_id[0] = 0;
	memset(second_thread_args.join_password,0,4096+1);
	get_idps((u8*)second_thread_args.idps);

	struct LuaPatchDetails patch_lua_names[MAX_LINES];
	int method_count = 0;
	int method_index = 0;

	FILE *fp_to_write_placeholder;
	bool file_no_exist_or_is_empty = 0;

	fp_to_write_placeholder = fopen(NEW_NUM_1_SAVED_URLS_TXT,"rb");
	if (fp_to_write_placeholder == 0) {
		file_no_exist_or_is_empty = 1;
	}
	else {
		fseek(fp_to_write_placeholder, 0, SEEK_END);
		file_no_exist_or_is_empty = ftell(fp_to_write_placeholder) == 0;
		fclose(fp_to_write_placeholder);
	}

	load_config();
	if (file_no_exist_or_is_empty) {
		fp_to_write_placeholder = fopen(NEW_NUM_1_SAVED_URLS_TXT,"wb");
		if (fp_to_write_placeholder == 0) {
			return 1;
		}
		fwrite(DEFAULT_URLS,1,sizeof(DEFAULT_URLS)-1,fp_to_write_placeholder);
		fclose(fp_to_write_placeholder);
	}

	mkdir(ARCHIVE_DIR, 0777);

	DIR *dir_with_old_saved_urls = opendir(ROOT_DIR);
	struct dirent* reader;
	char full_path_src[1024 + strlen(ROOT_DIR)];
	char full_path_dst[1024 + strlen(ARCHIVE_DIR)];
	if (dir_with_old_saved_urls != NULL) {
		while ((reader = readdir(dir_with_old_saved_urls)) != NULL) {
			if (strcmp(reader->d_name,".") == 0 || strcmp(reader->d_name,"..") == 0) {
				continue;
			}
			if (strncmp(reader->d_name, OLD_VERSION_2_SAVED_URLS_STRING_STARTER, strlen(OLD_VERSION_2_SAVED_URLS_STRING_STARTER)) == 0) {
				snprintf(full_path_src, sizeof(full_path_src), "%s%s", ROOT_DIR, reader->d_name);
				snprintf(full_path_dst, sizeof(full_path_dst), "%s%s", ARCHIVE_DIR, reader->d_name);
				rename(full_path_src,full_path_dst);
			}

		}
		closedir(dir_with_old_saved_urls);
	}

	fp_to_write_placeholder = fopen(COLOUR_CONFIG_FILE,"rb");
	if (fp_to_write_placeholder == 0) {
		file_no_exist_or_is_empty = 1;
	}
	else {
		fseek(fp_to_write_placeholder, 0, SEEK_END);
		file_no_exist_or_is_empty = ftell(fp_to_write_placeholder) == 0;
		fclose(fp_to_write_placeholder);
	}

	if (file_no_exist_or_is_empty) {
		fp_to_write_placeholder = fopen(COLOUR_CONFIG_FILE,"wb");
		if (fp_to_write_placeholder == 0) {
			return 1;
		}
		fwrite(DEFAULT_COLOUR_CONFIG,1,sizeof(DEFAULT_COLOUR_CONFIG)-1,fp_to_write_placeholder);
		fclose(fp_to_write_placeholder);
	}

	mkdir(CACHE_DIR, 0777);

	fp_to_write_placeholder = fopen(PATCH_CACHE_FILE_EXISTS_THEN_TRUE,"rb");
	if (fp_to_write_placeholder != 0) {
		fclose(fp_to_write_placeholder);
	}

	second_thread_args.use_patch_cache = fp_to_write_placeholder != 0;

	tiny3d_Init(1024*1024);

	ioPadInit(MAX_PORT_NUM);
	ioKbInit(MAX_KB_PORT_NUM);

	// Load texture

    LoadTexture();

    sysModuleLoad(SYSMODULE_PNGDEC);

    atexit(exiting); // Tiny3D register the event 3 and do exit() call when you exit to the menu

	sys_ppu_thread_t second_thread_id;
	u64 second_thread_retval;

	int lua_do_file_res;

	int started_a_thread = 0;

	struct UrlToPatchTo temp_editing_url;
	char editing_url_text_buffer[72];


	int yes_no_game_popup = 0;
	char * game_title;
	char * game_category;
	char param_sfo_path[sizeof("/dev_hdd0/game/ABCD12345/PARAM.SFO")];

	u8 error_yet_to_press_ok = 0;
	bool exit_after_done = 0;
	bool allow_triangle_bypass_exit_after_done = 0;
	char error_msg[1000];
	char patch_method[sizeof(patch_lua_names[0].patch_method)];

	char pretty_showey[500];
	char icon_0_main_path[sizeof("/dev_hdd0/game/ABCD12345/ICON0.PNG")];
	pngData icon_0_main;
	int icon_0_main_index = -1;

	bool has_done_a_switch = 1;
	unsigned my_btn;
	unsigned old_btn = 0;
	int enter_button = 1;
	u8 current_menu = MENU_MAIN;
	int menu_arrow = 0;
	u8 saved_urls_txt_num = 1;

	u32 browse_games_arrow = 0;

	u32 browse_games_buffer_start = 0;
	u32 browse_games_buffer_max_size = MAX_LINES;
	u32 browse_games_buffer_size = 0;
	struct TitleIdAndGameName browse_games_buffer[browse_games_buffer_max_size];

	bool first_time = 1;
	bool checked_for_updates_yet = 0;

	sysUtilGetSystemParamInt(SYSUTIL_SYSTEMPARAM_ID_ENTER_BUTTON_ASSIGN, &enter_button);
	if (enter_button == 1) {
		BTN_CIRCLE = 32;
		BTN_CROSS = 64;
		strcpy(MY_CUSTOM_EDIT_OF_NOTO_SANS_FONT_CROSS_BTN,"\x86");
		strcpy(MY_CUSTOM_EDIT_OF_NOTO_SANS_FONT_CIRCLE_BTN,"\xb9");//"\x88"
	}
	else {
		BTN_CIRCLE = 64;
		BTN_CROSS = 32;
		strcpy(MY_CUSTOM_EDIT_OF_NOTO_SANS_FONT_CIRCLE_BTN,"\x86");
		strcpy(MY_CUSTOM_EDIT_OF_NOTO_SANS_FONT_CROSS_BTN,"\xb9");//"\x88"
	}


	// lua setup
	L = luaL_newstate();
	luaL_openlibs(L);

	lua_do_file_res = luaL_dofile(L, PATCH_LUA_FILE);
	if (lua_do_file_res) {
		dbglogger_log("Error: %s\n", lua_tostring(L, -1));
		return 1;
	}

	// checking for patch functions
	char * func_name;
	int fun_name_len;

	int full_method_count = 0;
	char method_name_temp[sizeof(patch_lua_names[0].patch_name)+sizeof("patch_method_")];
	char * method_name_string_value_temp;

	lua_pushglobaltable(L);
	lua_pushnil(L);
	while (lua_next(L, -2)) {
		if (!lua_isfunction(L, -1)) {
			goto lua_pop_continue;
		}
		func_name = lua_tostring(L, -2);
		if (strncmp(func_name,"patch_",strlen("patch_")) != 0) {
			goto lua_pop_continue;
		}
		if (strncmp(func_name,"patch_vita",strlen("patch_vita")) == 0) {
			goto lua_pop_continue;
		}
		// dont accept `patch_` named things
		fun_name_len = strlen(func_name);
		if (fun_name_len == strlen("patch_")) {
			goto lua_pop_continue;
		}
		if (fun_name_len > PATCH_LUA_SIZE-strlen("patch_")) {
			goto lua_pop_continue;
		}
		if (method_count > MAX_LINES) {
			goto lua_pop_continue;
		}
		method_count++;
		strcpy(patch_lua_names[method_index].patch_name,func_name+strlen("patch_"));
		method_index++;
		lua_pop_continue:
		lua_pop(L, 1);
	}
	lua_pop(L, 1);

	for (method_index = 0; method_index < method_count; method_index++) {
		sprintf(method_name_temp,"patch_method_%s",patch_lua_names[method_index].patch_name);
		lua_getglobal(L, method_name_temp);
		if (lua_isstring(L, -1)) {
			method_name_string_value_temp = lua_tostring(L, -1);
			if (strlen(method_name_string_value_temp) > sizeof(patch_lua_names[method_index].patch_method)-1) {
				continue;
			}
			strcpy(patch_lua_names[method_index].patch_method,method_name_string_value_temp);
			full_method_count++;
		}
		lua_pop(L, 1);
	}
	if (full_method_count != method_count) {
		dbglogger_log("found some functions but they had no patch_method_ string for it");
		return 1;
	}
	lua_pushboolean(L, 1);
	lua_setglobal(L, "IS_BIG_ENDIAN");

	method_index = 0;

	// Ok, everything is setup. Now for the main loop.
	while(1) {
		// menu control logic
		my_btn = get_button_pressed(enter_button);
		if (first_time) {
			if (!checked_for_updates_yet) {
				if (!started_a_thread) {
					sysThreadCreate(&second_thread_id,check_for_updates_thread,(void *)&second_thread_args,SECOND_THREAD_PRIORITY,SECOND_THREAD_STACK_SIZE,THREAD_JOINABLE,SECOND_THREAD_NAME);
					started_a_thread = CURRENTLY_CHECKING_FOR_UPDATES;
				}
				if (second_thread_args.has_finished) {
					sysThreadJoin(second_thread_id,&second_thread_retval);
					second_thread_args.has_finished = 0;
					started_a_thread = 0;

					exit_after_done = 1;
					error_yet_to_press_ok = 1;
					allow_triangle_bypass_exit_after_done = 1;

					if (global_newest_version_tag[0] == 0) {
						sprintf(error_msg,"Could not get newest version\nPlease check your internet connection then open this again");
					}
					else if (strcmp(VERSION_NUM_STR,global_newest_version_tag) != 0) {
						if (second_thread_retval == THREAD_RET_COULD_NOT_DOWNLOAD_LATEST_PKG) {
							sprintf(error_msg,"New version available %s.\nBut could not download it, likely because you are on RPCS3\nDownload at\nhttps://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/releases\nthen install it",global_newest_version_tag);
						}
						else {
							sprintf(error_msg,"New version available %s.\nPlease install it from\nPackage Manager -> PS3 System Storage",global_newest_version_tag);
						}
					}
					else {
						error_yet_to_press_ok = 0;
						exit_after_done = 0;
						allow_triangle_bypass_exit_after_done = 0;
					}

					checked_for_updates_yet = 1;
				}
				goto draw_scene_direct;
			}
			done_with_first_time_checking:
			first_time = 0;
			goto draw_scene_direct;
		}

		if (!(my_btn & old_btn)) {
			// special menus, popups
			if (error_yet_to_press_ok) {
				if (my_btn & BTN_CROSS) {
					if (exit_after_done) {
						return 0; // oscetool does not clean up after its done, i managed to make it sort of clean up for decrypt then encrypt but not after that
					}
					error_yet_to_press_ok = 0;
				}

				if (my_btn & BTN_TRIANGLE) {
					if (allow_triangle_bypass_exit_after_done) {
						exit_after_done = 0;
						error_yet_to_press_ok = 0;
						allow_triangle_bypass_exit_after_done = 0;
					}
				}
				goto draw_scene_direct;
			}

			else if (started_a_thread) {
				if (second_thread_args.has_finished) {
					sysThreadJoin(second_thread_id,&second_thread_retval);
					second_thread_args.has_finished = 0;
					started_a_thread = 0;

					switch (second_thread_retval) {
						case THREAD_RET_EBOOT_REVERTED:
							error_yet_to_press_ok = 2;
							sprintf(error_msg,"Succesfully reverted patches on %s",global_title_id);
							current_menu = MENU_PATCH_GAMES;
							menu_arrow = 0;
							goto draw_scene_direct;
							break;

						case THREAD_RET_EBOOT_BAK_NO_EXIST:
							error_yet_to_press_ok = 1;
							sprintf(error_msg,"EBOOT.BIN.BAK not on %s\nmost likley you never patched this game before",global_title_id);
							current_menu = MENU_PATCH_GAMES;
							menu_arrow = 0;
							goto draw_scene_direct;
							break;

						case THREAD_RET_EBOOT_PATCHED:
							error_yet_to_press_ok = 2;
							sprintf(error_msg,"Succesfully patched (%s)%s",patch_method,pretty_showey);
							current_menu = MENU_PATCH_GAMES;
							menu_arrow = 0;
							exit_after_done = 1;
							goto draw_scene_direct;
							break;
						case THREAD_RET_EBOOT_BACKUP_FAILED:
							error_yet_to_press_ok = 1;
							sprintf(error_msg,"Some reason, we could not backup your EBOOT.BIN on %s",pretty_showey);
							current_menu = MENU_PATCH_GAMES;
							menu_arrow = 0;
							exit_after_done = 1;
							goto draw_scene_direct;
							break;
						case THREAD_RET_EBOOT_DECRYPT_FAILED:
							error_yet_to_press_ok = 1;
							if ((global_is_digital_eboot) && !global_found_raps) {
								sprintf(error_msg,"Could not decrypt EBOOT.BIN on%s\nConsider Export all licenses as .RAPs on Apollo\nOr putting .rap file in /dev_hdd0/exdata",pretty_showey);
							}
							else {
								sprintf(error_msg,"Could not decrypt EBOOT.BIN on%s",pretty_showey);
							}

							current_menu = MENU_PATCH_GAMES;
							menu_arrow = 0;
							exit_after_done = 1;
							goto draw_scene_direct;
							break;
						case THREAD_RET_EBOOT_PATCH_FAILED:
							error_yet_to_press_ok = 1;
							sprintf(error_msg,"Could not patch (%s) EBOOT.BIN on%s\n%s\nplease report your game",patch_method,pretty_showey,lua_tostring(L, -1)+(strlen(PATCH_LUA_FILE)-strlen(PATCH_LUA_FILE_NAME)));
							current_menu = MENU_PATCH_GAMES;
							menu_arrow = 0;
							exit_after_done = 1;
							goto draw_scene_direct;
							break;
						default:
							assert(0);
					}

				}
				goto draw_scene_direct;
			}

			else if (yes_no_game_popup != 0) {
				if (my_btn & BTN_CROSS) {
					if (menu_arrow == 1) {

					}
					else {
						switch (yes_no_game_popup) {
							case YES_NO_GAME_POPUP_REVERT_EBOOT:
								strcpy(second_thread_args.title_id,global_title_id);
								sysThreadCreate(&second_thread_id,revert_eboot_thread,(void *)&second_thread_args,SECOND_THREAD_PRIORITY,SECOND_THREAD_STACK_SIZE,THREAD_JOINABLE,SECOND_THREAD_NAME);
								started_a_thread = YES_NO_GAME_POPUP_REVERT_EBOOT;
								break;
							case YES_NO_GAME_POPUP_PATCH_GAME:
								strcpy(second_thread_args.title_id,global_title_id);
								sysThreadCreate(&second_thread_id,patch_eboot_thread,(void *)&second_thread_args,SECOND_THREAD_PRIORITY,SECOND_THREAD_STACK_SIZE,THREAD_JOINABLE,SECOND_THREAD_NAME);
								started_a_thread = YES_NO_GAME_POPUP_PATCH_GAME;
								break;
							case YES_NO_GAME_POPUP_DELETE_URL:
								remove_element(&saved_urls,selected_url_index,saved_urls_count);
								saved_urls_count--;
								write_saved_urls(saved_urls_txt_num);
								selected_url_index = RESET_SELECTED_URL_INDEX;
								break;
							case YES_NO_GAME_POPUP_RESET_URLS:
								reset_all_urls();
								load_saved_urls(saved_urls_txt_num);
								selected_url_index = RESET_SELECTED_URL_INDEX;
								break;
							default:
								assert(0);
						}
					}
					yes_no_game_popup = 0;
					menu_arrow = 0;
				}
				menu_arrow = set_arrow(menu_arrow,my_btn,YES_NO_POPUP_ARROW);
				goto draw_scene_direct;
			}
			// refresh for global_title_id anywhere, can also put other things that refresh should refresh everywhere
			if (my_btn & BTN_TRIANGLE) {
				DONE_A_SWITCH;
				load_config();
				menu_arrow = 0;
			}

			// This might be differnt depdning on what menu but for now itll suffice
			if (my_btn & BTN_CIRCLE) {
				DONE_A_SWITCH;
				menu_arrow = 0;
				if (current_menu == MENU_BROWSE_GAMES) {
					current_menu = MENU_PATCH_GAMES;
				}
				else if (current_menu == MENU_URL_EDITOR) {
					current_menu = MENU_EDIT_URLS;
				}
				else {
					current_menu = MENU_MAIN;
				}
			}
			free_png_from_memory(&icon_0_main,&icon_0_main_index);
			// normal menus
			if (my_btn & BTN_RIGHT || my_btn & BTN_LEFT) {
				switch (current_menu) {
					case MENU_SELECT_URLS:
					case MENU_EDIT_URLS:
						DONE_A_SWITCH;
						selected_url_index = RESET_SELECTED_URL_INDEX;
						if (my_btn & BTN_RIGHT) {
							if (saved_urls_txt_num >= 99) {
								saved_urls_txt_num = 1;
							}
							else {
								saved_urls_txt_num++;
							}
						}
						else if (my_btn & BTN_LEFT) {
							if (saved_urls_txt_num <= 1) {
								saved_urls_txt_num = 99;
							}
							else {
								saved_urls_txt_num--;
							}
						}
						break;
				}
			}
			if (my_btn & BTN_SQUARE) {
				DONE_A_SWITCH;
				switch (current_menu) {
					case MENU_EDIT_URLS:
						if (saved_urls_count <= 0) {
							break;
						}
						if ((menu_arrow+1) > saved_urls_count) {
							break;
						}
						selected_url_index = menu_arrow;
						sprintf(error_msg,"Are you sure you want to delete URL\n%s",saved_urls[selected_url_index].url);
						yes_no_game_popup = YES_NO_GAME_POPUP_DELETE_URL;
						menu_arrow = 1;
						break;
				}
			}
			if (my_btn & BTN_CROSS) {
				DONE_A_SWITCH;
				switch (current_menu) {
					case MENU_MAIN:
						switch (menu_arrow) {
							case 0:
								current_menu = MENU_SELECT_URLS;
								break;
							case 1:
								current_menu = MENU_EDIT_URLS;
								break;
							case 2:
								current_menu = MENU_PATCH_GAMES;
								break;
							case 3:
								second_thread_args.use_patch_cache = !second_thread_args.use_patch_cache;
								save_patch_cache_bool(second_thread_args.use_patch_cache);
								break;
							case 4:
								return 0; // exit
								break;
						}
						break;
					case MENU_PATCH_GAMES:
						switch (menu_arrow) {
							case 0:
								while (1) {
									input("Enter in a title id (example BCES00141)",global_title_id,sizeof(global_title_id));
									for (int i = 0; global_title_id[i] != '\0'; i++) {
										global_title_id[i] = toupper(global_title_id[i]);
									}
									if (is_valid_title_id(global_title_id)) {
										save_global_title_id_to_disk();
										break;
									}

								}
								break;
							case 1:
								second_thread_args.normalise_digest = !second_thread_args.normalise_digest;
								break;
							case 2:
								current_menu = MENU_BROWSE_GAMES;
								menu_arrow = 0;
								break;
							case 4:
								input("Enter in join key (leave empty if only play alone) (follow best password practices!)",second_thread_args.join_password,sizeof(second_thread_args.join_password));
								while (strlen(second_thread_args.join_password) > 0 && strlen(second_thread_args.join_password) < 8) {
									input("ENTER AT LEAST 8 CHARACTERS",second_thread_args.join_password,sizeof(second_thread_args.join_password));
								}
								save_user_join_pwd(second_thread_args.join_password);
								break;
							default:
								if (!title_id_exists(global_title_id)) {
									error_yet_to_press_ok = 1;
									sprintf(error_msg,"We could not find %s, is the title id correct? also make sure the game is updated",global_title_id);
									current_menu = MENU_PATCH_GAMES;
									menu_arrow = 0;
									goto draw_scene_direct;
								}
								struct UrlToPatchTo temp_show = saved_urls[selected_url_index];
								sprintf(icon_0_main_path,"/dev_hdd0/game/%s/ICON0.PNG",global_title_id);
								load_png_from_filename_to_memory(&icon_0_main,&icon_0_main_index,icon_0_main_path);
								sprintf(param_sfo_path,"/dev_hdd0/game/%s/PARAM.SFO",global_title_id);
								game_category = get_category_from_param(param_sfo_path);
								// default to digital eboot if cant read param.sfo
								global_is_digital_eboot = 1;
								if (game_category != 0) {
									// GD is Game Data (update data), DG is Disc Game, usually HG Harddrive Game will be digital
									if ((strcmp(game_category,"GD") == 0) || (strcmp(game_category,"DG") == 0)) {
										global_is_digital_eboot = 0;
									}
									free(game_category);
								}

								game_title = get_title_id_from_param(param_sfo_path);
								if (game_title == 0) {
									game_title = malloc(sizeof("Unknown??"));
									strcpy(game_title,"Unknown??");
								}
								if (temp_show.digest[0]) {
									sprintf(pretty_showey,"\n%s\nTitle id: %s\nwith the url\n%s\nand with digest key\n%s",game_title,global_title_id,temp_show.url,temp_show.digest);
								}
								else {
									sprintf(pretty_showey,"\n%s\nTitle id: %s\nwith the url\n%s",game_title,global_title_id,temp_show.url);
								}

								if (menu_arrow == 3) {
									yes_no_game_popup = YES_NO_GAME_POPUP_REVERT_EBOOT;
									sprintf(error_msg,"Do you want to revert patches on\n%s\nTitle id: %s",game_title,global_title_id);
								}

								else {
									// if theres an invalid patch_name in the saved_url, just use the first one
									method_index = 0;
									for (int i = 0; i < method_count; i++) {
										if (strcmp(saved_urls[selected_url_index].patch_name,patch_lua_names[i].patch_name) == 0) {
											method_index = i;
											break;
										}
									}
									strcpy(patch_method,patch_lua_names[method_index].patch_method);
									strcpy(second_thread_args.patch_lua_name,patch_lua_names[method_index].patch_name);
									yes_no_game_popup = YES_NO_GAME_POPUP_PATCH_GAME;
									sprintf(error_msg,"Do you want to patch (%s)%s",patch_method,pretty_showey);
								}
								free(game_title);

								current_menu = MENU_PATCH_GAMES;
								menu_arrow = 1;
								goto draw_scene_direct;
								break;
						}
						break;
					case MENU_SELECT_URLS:
						// on Reset URLs button
						if (menu_arrow == (saved_urls_count-1)+1) {
							strcpy(error_msg,"Are you sure you want to reset all URLs?\nAny URLs you saved will be lost!");
							yes_no_game_popup = YES_NO_GAME_POPUP_RESET_URLS;
							menu_arrow = 1;
						}
						else {
							selected_url_index = (menu_arrow == selected_url_index) ? RESET_SELECTED_URL_INDEX : menu_arrow;
						}

						break;
					case MENU_EDIT_URLS:
						if ((menu_arrow+1) > saved_urls_count) {
							if (saved_urls_count >= MAX_SAVED_URLS_AMNT) {
								// safety check, but should never happen
								break;
							}
							int new_saved_url_index = (saved_urls_count-1)+1;
							memset(&saved_urls[new_saved_url_index],0,sizeof(saved_urls[new_saved_url_index]));

							strcpy(saved_urls[new_saved_url_index].url,"ENTER_A_URL_HERE");
							strcpy(saved_urls[new_saved_url_index].patch_name,"lbp_main");
							saved_urls_count++;

							selected_url_index = new_saved_url_index;
							current_menu = MENU_URL_EDITOR;
						}
						else {
							selected_url_index = menu_arrow;
							current_menu = MENU_URL_EDITOR;
						}
						break;
					case MENU_URL_EDITOR:
						switch (menu_arrow) {
							case 0:
								strcpy(editing_url_text_buffer,saved_urls[selected_url_index].url);
								edit_url_input:
								input("Enter in a URL",editing_url_text_buffer,sizeof(saved_urls[selected_url_index].url));
								remove_spaces(editing_url_text_buffer);
								if (editing_url_text_buffer[0] == 0) {
									goto edit_url_input;
								}
								strcpy(saved_urls[selected_url_index].url,editing_url_text_buffer);
								break;
							case 1:
								strcpy(editing_url_text_buffer,saved_urls[selected_url_index].digest);
								input("Enter in a digest key, put in CustomServerDigest if this is a Refresh server otherwise leave empty",editing_url_text_buffer,sizeof(saved_urls[selected_url_index].digest));
								remove_spaces(editing_url_text_buffer);

								strcpy(saved_urls[selected_url_index].digest,editing_url_text_buffer);
								break;
							default:
								method_index = menu_arrow - MINUS_MENU_ARROW_AMNT_URL_EDITOR_TO_GET_PATCH_LUA_INDEX;
								strcpy(saved_urls[selected_url_index].patch_name,patch_lua_names[method_index].patch_name);
								break;
						}
						write_saved_urls(saved_urls_txt_num);
						break;
					case MENU_BROWSE_GAMES:
						strcpy(global_title_id,browse_games_buffer[menu_arrow - browse_games_buffer_start].title_id);
						save_global_title_id_to_disk();
						break;
				}
				// put code here if you dont want the menu arrow to reset
				if (current_menu == MENU_BROWSE_GAMES || yes_no_game_popup == YES_NO_GAME_POPUP_RESET_URLS) {

				}
				else {
					menu_arrow = 0;
				}
			}
			switch (current_menu) {
				case MENU_MAIN:
					if (has_done_a_switch) {
						// do first time code here
						has_done_a_switch = 0;
					}

					menu_arrow = set_arrow(menu_arrow,my_btn,MENU_MAIN_ARROW);

					break;

				case MENU_PATCH_GAMES:
					if (has_done_a_switch) {
						// do first time code here
						if (!is_a_url_selected()) {
							error_yet_to_press_ok = 1;
							strcpy(error_msg,"Please select a URL in Select URL menu");
							current_menu = MENU_MAIN;
							menu_arrow = 0;
							goto draw_scene_direct;
						}

						has_done_a_switch = 0;
					}

					menu_arrow = set_arrow(menu_arrow,my_btn,MENU_PATCH_GAMES_ARROW);

					break;
				case MENU_URL_EDITOR:
					menu_arrow = set_arrow(menu_arrow,my_btn,MENU_URL_EDITOR_ARROW);
					break;
				case MENU_EDIT_URLS:
				case MENU_SELECT_URLS:
					if (has_done_a_switch) {
						load_saved_urls(saved_urls_txt_num);

						has_done_a_switch = 0;
					}
					if (current_menu == MENU_EDIT_URLS) {
						menu_arrow = set_arrow(menu_arrow,my_btn,MENU_EDIT_URLS_ARROW);
					}
					else {
						menu_arrow = set_arrow(menu_arrow,my_btn,MENU_SELECT_URLS_ARROW);
					}
					break;

				case MENU_BROWSE_GAMES:
					if (has_done_a_switch) {
						browse_games_arrow = total_count_of_patchable_games(0,0xFFFFFFFF) - 1;
						if (menu_arrow == 0) {
							browse_games_buffer_start = 0;
							browse_games_buffer_size = load_patchable_games(browse_games_buffer,browse_games_buffer_start,browse_games_buffer_max_size);
						}
						has_done_a_switch = 0;
					}
					menu_arrow = set_arrow(menu_arrow,my_btn,browse_games_arrow);
					if (menu_arrow < browse_games_buffer_start) {
						browse_games_buffer_start -= browse_games_buffer_max_size;
						browse_games_buffer_size = load_patchable_games(browse_games_buffer,browse_games_buffer_start,browse_games_buffer_max_size);
					}
					else if (menu_arrow > (browse_games_buffer_start + browse_games_buffer_max_size)-1) {
						browse_games_buffer_start += browse_games_buffer_max_size;
						browse_games_buffer_size = load_patchable_games(browse_games_buffer,browse_games_buffer_start,browse_games_buffer_max_size);
					}

					break;

			}

		}
		draw_scene_direct:
		old_btn = my_btn;
        /* DRAWING STARTS HERE */
        // clear the screen, buffer Z and initializes environment to 2D
        tiny3d_Clear(0xff000000, TINY3D_CLEAR_ALL);
        // Enable alpha Test
        tiny3d_AlphaTest(1, 0x10, TINY3D_ALPHA_FUNC_GEQUAL);
        // Enable alpha blending.
        tiny3d_BlendFunc(1, TINY3D_BLEND_FUNC_SRC_RGB_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_ALPHA_SRC_ALPHA,
            TINY3D_BLEND_FUNC_DST_RGB_ONE_MINUS_SRC_ALPHA | TINY3D_BLEND_FUNC_DST_ALPHA_ZERO,
            TINY3D_BLEND_RGB_FUNC_ADD | TINY3D_BLEND_ALPHA_FUNC_ADD);

        drawScene(current_menu,menu_arrow,error_yet_to_press_ok,error_msg,yes_no_game_popup,
		started_a_thread,second_thread_args.current_state,&icon_0_main,&icon_0_main_index,saved_urls_txt_num,second_thread_args.normalise_digest,second_thread_args.use_patch_cache,
		browse_games_buffer,browse_games_buffer_size,browse_games_buffer_start,global_title_id,
		method_count,patch_lua_names, second_thread_args.join_password,allow_triangle_bypass_exit_after_done); // Draw

        /* DRAWING FINISH HERE */

        tiny3d_Flip();

	}

	return 0;
}

