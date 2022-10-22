#include "moduleloader.h"
#include <libmc.h>
#include <libpad.h>
#include <kernel.h>
#include <debug.h>
#include <netman.h>
#include <loadfile.h>
#include <iopheap.h>
#include <sifrpc.h>

extern unsigned char ps2dev9_irx[];
extern unsigned int size_ps2dev9_irx;

extern unsigned char smap_irx[];
extern unsigned int size_smap_irx;

extern unsigned char netman_irx[];
extern unsigned int size_netman_irx;

void ps2IOPinit()
{
    // Reboot IOP
    SifInitRpc(0);
    while (!SifIopReset("", 0))
    {
    };
    while (!SifIopSync())
    {
    };

    // Initialize SIF services
    SifInitRpc(0);
    SifLoadFileInit();
    SifInitIopHeap();
    sbv_patch_enable_lmb();
}


void ps2mcInitModules()
{
    int ret;
    scr_printf(" >");
    scr_printf(" SIO2MAN.IRX");
    ret = SifLoadModule("rom0:SIO2MAN", 0, NULL);
    if (ret < 0)
    {
        scr_printf(" #\n");
        scr_printf("sifLoadModule sio failed: %d\n", ret);
        SleepThread();
    }
    scr_printf(" MCMAN.IRX");
    ret = SifLoadModule("rom0:MCMAN", 0, NULL);
    if (ret < 0)
    {
        scr_printf(" #\n");
        scr_printf("sifLoadModule mcman failed: %d\n", ret);
        SleepThread();
    }
    scr_printf(" MCSERV.IRX");
    ret = SifLoadModule("rom0:MCSERV", 0, NULL);
    if (ret < 0)
    {
        scr_printf(" #\n");
        scr_printf("sifLoadModule mcserv failed: %d\n", ret);
        SleepThread();
    }
    scr_printf(" mcInit(MC_TYPE_MC);");
    if (mcInit(MC_TYPE_MC) >= 0)
        scr_printf(" <\n");
    else
    {
        scr_printf(" #\n");
        scr_printf("Failed to initialise memcard server!\n");
        SleepThread();
    }
}

void ps2padInitModules()
{
    int ret;
    scr_printf(" >");
    scr_printf(" SIO2MAN.IRX");
    ret = SifLoadModule("rom0:SIO2MAN", 0, NULL);
    if (ret < 0)
    {
        scr_printf(" #\n");
        scr_printf("sifLoadModule sio failed: %d\n", ret);
        SleepThread();
    }
    scr_printf(" PADMAN.IRX");
    ret = SifLoadModule("rom0:PADMAN", 0, NULL);
    if (ret < 0)
    {
        scr_printf(" #\n");
        scr_printf("sifLoadModule pad failed: %d\n", ret);
        SleepThread();
    }
    scr_printf(" padInit(0);");
    if (padInit(0) == 1)
        scr_printf(" <\n");
    else
    {
        scr_printf(" #\n");
        scr_printf("Failed to initialise gamepad server!\n");
        SleepThread();
    }
}

void ps2ipInitModules()
{
    int ret;
    scr_printf(" >");
    scr_printf(" DEV9.IRX");
    ret = SifExecModuleBuffer(ps2dev9_irx, size_ps2dev9_irx, 0, NULL, NULL);
    if (ret < 0)
    {
        scr_printf(" #\n");
        scr_printf("sifLoadModule dev9 failed: %d\n", ret);
        SleepThread();
    }
    scr_printf(" NETMAN.IRX");
    ret = SifExecModuleBuffer(netman_irx, size_netman_irx, 0, NULL, NULL);
    if (ret < 0)
    {
        scr_printf(" #\n");
        scr_printf("sifLoadModule netman failed: %d\n", ret);
        SleepThread();
    }
    scr_printf(" SMAP.IRX");
    ret = SifExecModuleBuffer(smap_irx, size_smap_irx, 0, NULL, NULL);
    if (ret < 0)
    {
        scr_printf(" #\n");
        scr_printf("sifLoadModule smap failed: %d\n", ret);
        SleepThread();
    }
    scr_printf(" NetManInit();");
    if (NetManInit() >= 0)
        scr_printf(" <\n");
    else
    {
        scr_printf(" #\n");
        scr_printf("Failed to initialise network manager!\n");
        SleepThread();
    }
}