#ifndef SAVE_FOLDERS_H_   /* Include guard */
#define SAVE_FOLDERS_H_

#define VERSION_NUM_STR "v2.013"

#define ROOT_DIR "/dev_hdd0/game/LBPCSPPHB/USRDIR/"
#define WORKING_DIR ROOT_DIR "temp_files/"
#define COLOUR_CONFIG_FILE ROOT_DIR "colours_config.txt"
#define PATCH_LUA_FILE_NAME "patch.lua"
#define PATCH_LUA_FILE ROOT_DIR PATCH_LUA_FILE_NAME

#define DEFAULT_URLS "http://lighthouse.lbpunion.com/LITTLEBIGPLANETPS3_XML\n"\
					 "http://refresh.jvyden.xyz:2095/lbp CustomServerDigest\n"\
					 "http://lnfinite.site/LITTLEBIGPLANETPS3_XML\n"\


#define OLD_SAVED_URLS_TXT ROOT_DIR "saved_urls.txt"
#define NEW_NUM_1_SAVED_URLS_TXT ROOT_DIR "saved_urls_1.txt"
#define SAVED_URLS_TXT_FIRST_HALF ROOT_DIR "saved_urls"
#define SAVED_URLS_TXT_SECOND_HALF ".txt"


#define TITLE_ID_TXT ROOT_DIR "title_id_to_patch.txt"

#endif // SAVE_FOLDERS_H_
