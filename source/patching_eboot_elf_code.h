#include <ppu-types.h>

#ifndef PATCHING_EBOOT_ELF_CODE_H_   /* Include guard */
#define PATCHING_EBOOT_ELF_CODE_H_

#define MAX_URL_LEN_INCL_NULL 72
#define BIGGEST_POSSIBLE_URL_IN_EBOOT_INCL_NULL 80
#define MAX_DIGEST_LEN_INCL_NULL 20
#define BIGGEST_POSSIBLE_DIGEST_IN_EBOOT_INCL_NULL 24

#define PATCH_METHOD_MAIN_SERIES "LittleBigPlanet Main Series"
#define PATCH_METHOD_KARTING "LittleBigPlanet Karting"
#define BIGGEST_POSSIBLE_URL_IN_LBPK_EBOOT_INCL_NULL 30


#define PATCH_ERR_EBOOT_ELF_NO_EXISTS 1
#define PATCH_ERR_NO_URLS_FOUND 2
#define PATCH_ERR_NO_DIGESTS_FOUND 3
#define PATCH_ERR_THIS_SHOULD_NEVER_HAPPEN 4

// int patch_eboot_elf_main_series(const char *eboot_elf_path,const char *url, const char *digest, bool normalise_digest);
#define PATCH_EBOOT_FUNC_SIGNATURE(name) int (*name)(const char *, const char *, const char *, bool)

int patch_eboot_elf_main_series(const char *eboot_elf_path,const char *url, const char *digest, bool normalise_digest);
int patch_eboot_elf_karting(const char *eboot_elf_path, const char *url, const char *digest, bool normalise_digest);

#endif // PATCHING_EBOOT_ELF_CODE_H_