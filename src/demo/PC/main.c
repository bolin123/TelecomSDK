#include "UserTypes.h"
#if defined(COMPILE_DEMO_1)
#include "../demo1.h"
#else
#include "../demo2.h"
#endif

#ifdef __WIN__
#include <Windows.h>
#define SLEEP_MS() Sleep(1)
#else
#include <unistd.h>
#define SLEEP_MS() usleep(1000)
#endif

void PSocketInitialize(void);
void PSocketPoll(void);
void PSocketBackPoll(void);


int main()
{
#ifdef __WIN__
        WORD wVersionRequested = MAKEWORD(1, 1);
        WSADATA wsaData;

        int err = WSAStartup(wVersionRequested, &wsaData);
        if(0 != err)
        {
        }
#endif//socket≥ı ºªØ

    PSocketInitialize();
	//PlatformUserInit();
	DemoInit();

	while (1)
	{
	    PSocketPoll();
        PSocketBackPoll();
        //PlatformUserPoll();
        DemoPoll();
		SLEEP_MS();
	}

	return 0;
}
