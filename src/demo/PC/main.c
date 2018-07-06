#include "SysTypes.h"
#include "../PlatformUser.h"

#ifdef __WIN__
#include <Windows.h>
#define SLEEP_MS() Sleep(1)
#else
#include <unistd.h>
#define SLEEP_MS() usleep(1000)
#endif

int main()
{
	PlatformUserInit();

	while (1)
	{
        PlatformUserPoll();
        PlatformUserTimePass(1);
		SLEEP_MS();
	}

	return 0;
}