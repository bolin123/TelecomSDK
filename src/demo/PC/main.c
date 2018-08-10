#include "UserTypes.h"
//#include "../PlatformUser.h"
#include "../miniGW.h"

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
	miniGWInit();

	while (1)
	{
	    PSocketPoll();
        PSocketBackPoll();
        //PlatformUserPoll();
        miniGWPoll();
		SLEEP_MS();
	}

	return 0;
}
