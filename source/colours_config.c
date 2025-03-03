#include <string.h>
#include <stdio.h>
#include <ppu-types.h>

#include "save_folders.h"

u32 TITLE_FONT_COLOUR = 0xffffffff; // white
u32 TITLE_BG_COLOUR = 0x00000000; // invisible
u32 SELECTABLE_NORMAL_FONT_COLOUR = 0xffffffff; // white
u32 TURNED_ON_FONT_COLOUR = 0xFFFF00ff; // yellow
u32 BACKGROUND_COLOUR = 0x002b26ff; // very dark turquiose
u32 SELECTED_FONT_BG_COLOUR = 0x575757ff; // dark grey
u32 UNSELECTED_FONT_BG_COLOUR = 0x00000000; // invisible
u32 ERROR_MESSAGE_COLOUR = 0xFF0000FF; // red
u32 ERROR_MESSAGE_BG_COLOUR = 0x00000000; // invisible
u32 SUCCESS_MESSAGE_COLOUR = 0x00FF00FF; // green
u32 SUCCESS_MESSAGE_BG_COLOUR = 0x00000000; // invisible

void load_config() {
	char * hex_colour_code;
	bool hex_colour_code_started_with_a_hashtag = 0;
	
	u8 hex_colour_code_offset_from_line;
	u8 hex_colour_code_len;
	char * line = NULL;
    size_t len = 0;
    ssize_t len_of_line;
	
	FILE *fp = fopen(COLOUR_CONFIG_FILE, "ab+"); // not checking if it fails to open, just let it segfault, cause theres bigger problems if it doesnt works
    rewind(fp);
	while ((len_of_line = __getline(&line, &len, fp)) > 0) {
        // TODO perhaps remove all leading and trailing whitespace from the line, similar to python str.strip methond
		if (len_of_line > 1000) {
			continue;
		}
		line[strcspn(line, "\r\n")] = 0;
		len_of_line = strlen(line);
		if (!line[0]) {
			continue;
		}

		hex_colour_code_offset_from_line = strcspn(line, " ");
		hex_colour_code_len = len_of_line - hex_colour_code_offset_from_line;
		
		if (hex_colour_code_len == 0) {
			continue;
		}
		hex_colour_code_len--; // for space charr

		hex_colour_code = (char*)malloc(len_of_line);

		memcpy(hex_colour_code,line+hex_colour_code_offset_from_line+1,hex_colour_code_len);
		
		if (hex_colour_code[0] == '#' && strlen(hex_colour_code) > 1) {
			hex_colour_code++;
			hex_colour_code_started_with_a_hashtag = 1;
		}
		
		line[hex_colour_code_offset_from_line] = 0;
		len_of_line -= hex_colour_code_len;
		len_of_line--; // for the space char
		
		if (strcmp("TITLE_FONT_COLOUR", line) == 0) {
			TITLE_FONT_COLOUR = (u32)strtol(hex_colour_code, NULL, 16);
		}
		if (strcmp("TITLE_BG_COLOUR", line) == 0) {
			TITLE_BG_COLOUR = (u32)strtol(hex_colour_code, NULL, 16);
		}
		else if (strcmp("SELECTABLE_NORMAL_FONT_COLOUR", line) == 0) {
			SELECTABLE_NORMAL_FONT_COLOUR = (u32)strtol(hex_colour_code, NULL, 16);
		}
		else if (strcmp("TURNED_ON_FONT_COLOUR", line) == 0) {
			TURNED_ON_FONT_COLOUR = (u32)strtol(hex_colour_code, NULL, 16);
		}
		else if (strcmp("BACKGROUND_COLOUR", line) == 0) {
			BACKGROUND_COLOUR = (u32)strtol(hex_colour_code, NULL, 16);
		}
		else if (strcmp("SELECTED_FONT_BG_COLOUR", line) == 0) {
			SELECTED_FONT_BG_COLOUR = (u32)strtol(hex_colour_code, NULL, 16);
		}
		else if (strcmp("UNSELECTED_FONT_BG_COLOUR", line) == 0) {
			UNSELECTED_FONT_BG_COLOUR = (u32)strtol(hex_colour_code, NULL, 16);
		}
		else if (strcmp("ERROR_MESSAGE_COLOUR", line) == 0) {
			ERROR_MESSAGE_COLOUR = (u32)strtol(hex_colour_code, NULL, 16);
		}
		else if (strcmp("ERROR_MESSAGE_BG_COLOUR", line) == 0) {
			ERROR_MESSAGE_BG_COLOUR = (u32)strtol(hex_colour_code, NULL, 16);
		}
		else if (strcmp("SUCCESS_MESSAGE_COLOUR", line) == 0) {
			SUCCESS_MESSAGE_COLOUR = (u32)strtol(hex_colour_code, NULL, 16);
		}
		else if (strcmp("SUCCESS_MESSAGE_BG_COLOUR", line) == 0) {
			SUCCESS_MESSAGE_BG_COLOUR = (u32)strtol(hex_colour_code, NULL, 16);
		}
		if (hex_colour_code_started_with_a_hashtag) {
			hex_colour_code--;
		}
		free(hex_colour_code);
	}
	
	fclose(fp);
	free(line);
}