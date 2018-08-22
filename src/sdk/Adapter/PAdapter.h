/*
 * PAdapter.h
 *
 * �û�����ӿ�ͷ�ļ�
 * �û���Ҫʵ�ָ�ͷ�ļ�����Ľӿں���
 *
 * By Berlin 2018.08 <chenbl@yumair.cn>
 */
#ifndef PADPTER_H
#define PADPTER_H

/*
* ��ȡ�豸��CTEI��
* ����ֵ:�ն˵�CTEI��,�����ն˰�װ���ϵ�CTEI����һ��
*/
char *PGetCTEI(void);

/*
* ��ȡ�豸IP��ַ
* ����: @ip:IP��ŵ����飬�û��ɽ�IP���Ƶ��������в�����
* ����ֵ:�豸IP��ַ����ʽ:"xxx.xxx.xxx.xxx"
*/
char *PGetIpAddr(char *ip);

/*
* ��ȡ�豸MAC��ַ
* ����: @mac:MAC��ŵ����飬�û��ɽ�MAC���Ƶ��������в�����
* ����ֵ:�豸MAC��ַ����ʽ:"xx:xx:xx:xx:xx:xx"
*/
char *PGetMacAddr(char *mac);

/*
* ��������״̬
* ����ֵ: 1:���������ӣ�0:����Ͽ�
*/
unsigned char PIsLinkup(void);

/*
* UTCʱ��
* ����ֵ:utcʱ��
*/
unsigned int PUtcTime(void);

/*
* ϵͳʱ��
* ����ֵ:����ϵͳ������ĺ���ֵ
*/
unsigned int PlatformTime(void);

#endif // !PADPTER_H
