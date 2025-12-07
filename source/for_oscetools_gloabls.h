#ifndef FOR_OSCETOOLS_GLOABLS_H_
#define FOR_OSCETOOLS_GLOABLS_H_

#define PATCH_LUA_SIZE 256
#define PATCH_METHOD_LUA_STRING_SIZE 512

#include <ppu-types.h>

bool does_file_exist(char * filename);

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

extern struct SecondThreadArgs second_thread_args;
extern bool global_found_raps;
extern bool global_is_digital_eboot;
extern char global_content_id[128];

#endif // FOR_OSCETOOLS_GLOABLS_H_