#include "ets_sys.h"
#include "driver/uart.h"
#include "osapi.h"
#include "user_interface.h"
#include "UserTypes.h"
#if defined(COMPILE_DEMO_1)
#include "demo1.h"
#else
#include "demo2.h"
#endif
#include "Wifi.h"

static bool g_init;
static os_event_t g_events[1];
static unsigned int g_utcTime = 1514736000; //2018/1/1 0:0:0

ICACHE_FLASH_ATTR uint32_t SysGetTimeMs(void)
{
    return (system_get_time() / 1000);
}

ICACHE_FLASH_ATTR void SysUtcTimeSet(unsigned int time)
{
    g_utcTime = time;
}

ICACHE_FLASH_ATTR static void utcTimeUpdate(void)
{
    static uint32_t lasttime;
    uint32_t diff = system_get_time() - lasttime;
    diff /= 1000000;
    if(diff)
    {
        lasttime = system_get_time();
        g_utcTime++;

    }
}

ICACHE_FLASH_ATTR static void task(os_event_t *event)
{
    if(!g_init)
    {
        g_init = true;
        WifiInitialize();
        WifiJoinAp("Yunho_berlin", "a123456789", WIFI_AUTH_WPA_WPA2_PSK);
        DemoInit();
    }
    else
    {
        DemoPoll();
        utcTimeUpdate();
    }

    system_os_post(0, 0, 0);
}

void user_rf_pre_init()
{


}

void user_init(void)
{
    uart_init(1, 0, 115200);
    uart_switch_debug(1);

    system_os_task(task, 0, g_events, 1);
    system_os_post(0, 0, 0);
}

