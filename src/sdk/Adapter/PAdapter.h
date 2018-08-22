/*
 * PAdapter.h
 *
 * 用户适配接口头文件
 * 用户需要实现该头文件定义的接口函数
 *
 * By Berlin 2018.08 <chenbl@yumair.cn>
 */
#ifndef PADPTER_H
#define PADPTER_H

/*
* 获取设备的CTEI码
* 返回值:终端的CTEI码,须与终端包装盒上的CTEI保持一致
*/
char *PGetCTEI(void);

/*
* 获取设备IP地址
* 参数: @ip:IP存放的数组，用户可将IP复制到该数组中并返回
* 返回值:设备IP地址，格式:"xxx.xxx.xxx.xxx"
*/
char *PGetIpAddr(char *ip);

/*
* 获取设备MAC地址
* 参数: @mac:MAC存放的数组，用户可将MAC复制到该数组中并返回
* 返回值:设备MAC地址，格式:"xx:xx:xx:xx:xx:xx"
*/
char *PGetMacAddr(char *mac);

/*
* 网络链接状态
* 返回值: 1:网络已连接，0:网络断开
*/
unsigned char PIsLinkup(void);

/*
* UTC时钟
* 返回值:utc时间
*/
unsigned int PUtcTime(void);

/*
* 系统时间
* 返回值:返回系统启动后的毫秒值
*/
unsigned int PlatformTime(void);

#endif // !PADPTER_H
