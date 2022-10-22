/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
*/

#include <stdio.h>
#include <stdlib.h>
#include <kernel.h>
#include <iopcontrol.h>
#include <iopheap.h>
#include <debug.h>
#include <netman.h>
#include <ps2ip.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <sbv_patches.h>
#include "ps2ipcfg.h"
#include "ps2ipcfglib.h"


int ps2ip(struct oplnetconf *netconf, void(*onready)(void*), void *onreadydata)
{
    ethPrintLinkStatus();
    if (netconf == NULL || onready == NULL) goto end;
    struct ip4_addr *IP, *NM, *GW, *DNS;
    IP = (struct ip4_addr*) calloc(1, sizeof(struct ip4_addr));
    NM = (struct ip4_addr*) calloc(1, sizeof(struct ip4_addr));
    GW = (struct ip4_addr*) calloc(1, sizeof(struct ip4_addr));
    DNS = (struct ip4_addr*) calloc(1, sizeof(struct ip4_addr));
    if (IP == NULL || NM == NULL || GW == NULL || DNS == NULL) goto end;
    int EthernetLinkMode;

    // The network interface link mode/duplex can be set.
    EthernetLinkMode = NETMAN_NETIF_ETH_LINK_MODE_AUTO;

    // Attempt to apply the new link setting.
    if (ethApplyNetIFConfig(EthernetLinkMode) != 0)
    {
        scr_printf("Error: failed to set link mode.\n");
        goto end;
    }

    if (netconf->usedhcp != 0)
    {
        // Initialize IP address.
        // In this example, DHCP is enabled, hence the IP, NM, GW and DNS fields are cleared to 0..
        // ip4_addr_set_zero(IP);
        // ip4_addr_set_zero(NM);
        // ip4_addr_set_zero(GW);
        // ip4_addr_set_zero(DNS);
        IP->addr=0;
        NM->addr=0;
        GW->addr=0;
        DNS->addr=0;

        // Initialize the TCP/IP protocol stack.
        ps2ipInit(IP, NM, GW);

        // Enable DHCP
        ethApplyIPConfig(1, IP, NM, GW, DNS);

        scr_printf("Waiting for DHCP lease...\n");
        // Wait for DHCP to initialize, if DHCP is enabled.
        if (ethWaitValidDHCPState() != 0)
        {
            scr_printf("DHCP failed\n.");
            goto end;
        }
    }
    else
    {
        // Initialize IP address.
        // IP4_ADDR(IP, 192, 168, 0, 80);
        // IP4_ADDR(NM, 255, 255, 255, 0);
        // IP4_ADDR(GW, 192, 168, 0, 1);
        // DNS is not required if the DNS service is not used, but this demo will show how it is done.
        // IP4_ADDR(DNS, 192, 168, 0, 1);
        IP->addr = netconf->ps2ip.single;
        NM->addr = netconf->ps2nm.single;
        GW->addr = netconf->ps2gw.single;
        DNS->addr = netconf->ps2dns.single;

        // Initialize the TCP/IP protocol stack.
        ps2ipInit(IP, NM, GW);
        dns_setserver(0, DNS); // Set DNS server
        if (ethWaitValidNetIFLinkState() != 0)
        {
            scr_printf("Error: failed to get valid link status.\n");
            goto end;
        }
    }
    // Wait for the link to become ready.
    scr_printf("Waiting for connection...  ");
    if (ethWaitValidNetIFLinkState() != 0)
    {
        scr_printf("Error: failed to get valid link status.\n");
        goto end;
    }

    scr_printf("done!\n");
    scr_printf("Initialized:\n");
    ethPrintLinkStatus();
    ethPrintIPConfig();

    // At this point, network support has been initialized and the PS2 can be pinged.
    onready(onreadydata);

end:
    // To cleanup, just call these functions.
    ps2ipDeinit();
    NetManDeinit();

    // Deinitialize SIF services
    SifExitRpc();

    return 0;
}
