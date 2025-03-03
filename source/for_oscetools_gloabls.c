#include <ppu-types.h>
#include <stdio.h>
#include "patching_eboot_elf_code.h"

struct SecondThreadArgs {
    bool has_finished;
	bool normalise_digest;
	u64 idps[2];
	PATCH_EBOOT_FUNC_SIGNATURE(patch_func);
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