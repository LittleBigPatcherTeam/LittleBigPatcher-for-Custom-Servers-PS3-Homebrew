#include <sysmodule/sysmodule.h>
#include <sys/thread.h>
#include <ppu-types.h>

int get_idps(u8 *idps)
{
	lv2syscall2(867, 0x19003, (u64)idps);
	return_to_user_prog(int);
}