#ifndef PADPTER_H
#define PADPTER_H

char *PGetIpAddr(char *ip);
char *PGetMacAddr(char *mac);
unsigned char PIsLinkup(void);
unsigned int PUtcTime(void);
unsigned int PlatformTime(void);

#endif // !PADPTER_H
