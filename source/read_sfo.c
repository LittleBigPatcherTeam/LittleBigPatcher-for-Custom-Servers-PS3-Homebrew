#include <stdio.h>
#include <string.h>
#include <ppu-types.h>

#if defined(__BYTE_ORDER__)&&(__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define READ_SFO_ES16(_val) \
	((uint16_t)(((((uint16_t)_val) & 0xff00) >> 8) | \
		   ((((uint16_t)_val) & 0x00ff) << 8)))

#define READ_SFO_ES32(_val) \
	((uint32_t)(((((uint32_t)_val) & 0xff000000) >> 24) | \
		   ((((uint32_t)_val) & 0x00ff0000) >> 8 ) | \
		   ((((uint32_t)_val) & 0x0000ff00) << 8 ) | \
		   ((((uint32_t)_val) & 0x000000ff) << 24)))

#else
#define READ_SFO_ES16(_val) (uint16_t)_val
#define READ_SFO_ES32(_val) (uint32_t)_val

#endif

#define PARAM_SFO_HEADER 0x46535000
#define PARAM_SFO_TYPE_UTF8 0x0204
#define GAME_NAME_KEY "TITLE"

char * get_title_id_from_param(char * param_sfo_file_name)
{
	FILE *sfo;
	
	sfo = fopen(param_sfo_file_name, "rb+"); 
	if(sfo == 0) {
		
		return 0;
	}
/* 	char * delete_me_just_beacuse_this_func_is_ass;
	fclose(sfo);
	delete_me_just_beacuse_this_func_is_ass = malloc(sizeof("temp thinhg lmao kill me pls"));
	strcpy(delete_me_just_beacuse_this_func_is_ass,"w");
	return delete_me_just_beacuse_this_func_is_ass; */
	
	u32 param_sfo_header;
	u32 param_sfo_version;
	u32 key_table_start;
	u32 data_table_start;
	u32 tables_entries;
	
	fread(&param_sfo_header,sizeof(param_sfo_header),1,sfo);
	if (READ_SFO_ES32(param_sfo_header) != PARAM_SFO_HEADER) {
		fclose(sfo);
		return 0;
	}
	
	fread(&param_sfo_version,sizeof(param_sfo_version),1,sfo);
	param_sfo_version = READ_SFO_ES32(param_sfo_version);

	fread(&key_table_start,sizeof(key_table_start),1,sfo);
	key_table_start = READ_SFO_ES32(key_table_start);

	fread(&data_table_start,sizeof(data_table_start),1,sfo);
	data_table_start = READ_SFO_ES32(data_table_start);

	fread(&tables_entries,sizeof(tables_entries),1,sfo);
	tables_entries = READ_SFO_ES32(tables_entries);
	
	// we should now be at the tables_entries 
	
	char * game_name;

	int i;
	int j;
	bool found_a_null_char_in_game_name = 0;
	
	int go_back_here;
	char key_buffer[sizeof(GAME_NAME_KEY)];
	
	u16 key_offset; /*** param_key offset (relative to start offset of key_table) */
	u16 data_fmt; /***** param_data data type */
	u32 data_len; /***** param_data used bytes */
	u32 data_max_len; /* param_data total bytes */
	u32 data_offset; /** param_data offset (relative to start offset of data_table) */
	
	for (i = 0; i < tables_entries; i++) {
		fread(&key_offset,sizeof(key_offset),1,sfo);
		key_offset = READ_SFO_ES16(key_offset);

		fread(&data_fmt,sizeof(data_fmt),1,sfo);
		data_fmt = READ_SFO_ES16(data_fmt);
		
		fread(&data_len,sizeof(data_len),1,sfo);
		data_len = READ_SFO_ES32(data_len);

		fread(&data_max_len,sizeof(data_max_len),1,sfo);
		data_max_len = READ_SFO_ES32(data_max_len);

		fread(&data_offset,sizeof(data_offset),1,sfo);
		data_offset = READ_SFO_ES32(data_offset);
		
		if (data_fmt != PARAM_SFO_TYPE_UTF8) {
			continue;
		}
		
		go_back_here = ftell(sfo);
		
		fseek(sfo,key_table_start+key_offset,SEEK_SET);
		fread(&key_buffer,1,sizeof(key_buffer),sfo);
		if (memcmp(key_buffer,GAME_NAME_KEY,sizeof(GAME_NAME_KEY)) != 0) {
			fseek(sfo,go_back_here,SEEK_SET);
			continue;
		}
		
		fseek(sfo,data_table_start+data_offset,SEEK_SET);
		
		// we do data_max_len because some param.sfo editors do not edit the data_len, and the ps3 still reads this full title
		game_name = (char*)malloc(data_max_len);
		
		fread(game_name,1,data_max_len,sfo);
		
		fclose(sfo);
		
		for (j = 0; j < data_max_len; j++) {
			if (game_name[j] == 0) {
				found_a_null_char_in_game_name = 1;
				break;
			}
		}
		
		if (found_a_null_char_in_game_name) {
			return game_name;
		}
		free(game_name);
		return 0;
	}
	
	fclose(sfo);
	return 0;
}
