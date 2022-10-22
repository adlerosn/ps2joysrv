#ifndef __PS2IPCGF_H__
#define __PS2IPCFG_H__
#include "myoplnetparser.h"

int ps2ip(struct oplnetconf *netconf, void(*onready)(void*), void *onreadydata);

#endif // __PS2IPCFG_H__