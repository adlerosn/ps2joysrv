#ifndef __PS2IPCFGLIB_H__
#define __PS2IPCFGLIB_H__

#include <tamtypes.h>
#include <ps2ip.h>

int ethApplyNetIFConfig(int mode);
void EthStatusCheckCb(s32 alarm_id, u16 time, void *common);
int WaitValidNetState(int (*checkingFunction)(void));
int ethGetNetIFLinkStatus(void);
int ethWaitValidNetIFLinkState(void);
int ethGetDHCPStatus(void);
int ethWaitValidDHCPState(void);
int ethApplyIPConfig(int use_dhcp, const struct ip4_addr *ip, const struct ip4_addr *netmask, const struct ip4_addr *gateway, const struct ip4_addr *dns);
void ethPrintIPConfig(void);
void ethPrintLinkStatus(void);

#endif  // __PS2IPCFGLIB_H__