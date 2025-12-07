#include <ppu-types.h>
#include <stdio.h>

#define PATCH_LUA_SIZE 256
#define PATCH_METHOD_LUA_STRING_SIZE 512

struct SecondThreadArgs {
    bool has_finished;
	int current_state;
	bool normalise_digest;
	char join_password[4096+1];
	bool use_patch_cache;
	u64 idps[2];
	char patch_lua_name[PATCH_LUA_SIZE + 1];
    char title_id[sizeof("BCES12345")];
};

struct SecondThreadArgs second_thread_args;

bool global_found_raps = 0;
bool global_is_digital_eboot = 0;
char global_content_id[128];

bool does_file_exist(char * filename) {
	FILE *fp = fopen(filename,"rb");
	if (fp == 0) {
		return 0;
	}
	fclose(fp);
	return 1;
}