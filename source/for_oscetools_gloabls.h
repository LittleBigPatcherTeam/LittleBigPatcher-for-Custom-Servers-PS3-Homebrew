#ifndef FOR_OSCETOOLS_GLOABLS_H_
#define FOR_OSCETOOLS_GLOABLS_H_

#include <ppu-types.h>
#include "patching_eboot_elf_code.h"

bool does_file_exist(char * filename);

struct SecondThreadArgs {
    bool has_finished;
	int current_state;
	bool normalise_digest;
	u64 idps[2];
	PATCH_EBOOT_FUNC_SIGNATURE(patch_func);
    char title_id[sizeof("BCES12345")];
};

extern struct SecondThreadArgs second_thread_args;
extern bool global_found_raps;
extern bool global_is_digital_eboot;
extern char global_content_id[128];

#endif // FOR_OSCETOOLS_GLOABLS_H_