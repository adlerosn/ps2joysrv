#include <dirent.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <libmc.h>
#include <kernel.h>
#include <iopcontrol.h>
#include <iopheap.h>
#include <debug.h>
#include <netman.h>
#include <ps2ip.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <sbv_patches.h>
#include <libpad.h>
#include "ps2ipcfg.h"
#include "moduleloader.h"
#include "myoplnetparser.h"
#include "pad.h"

// void when_interneted_tcp(struct oplnetconf *netconf2, struct padButtonStatus* padsButtons){
//     print_opl_net_target(netconf2);
//     struct sockaddr_in servaddr;
//     int sockfd;
//     scr_printf(" > lwip_socket... ");
//     if ((sockfd = lwip_socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
//         scr_printf("Socket creation failed\n"); 
//         return;
//     }
//     bzero(&servaddr, sizeof(servaddr)); 
//     servaddr.sin_family = AF_INET; 
//     servaddr.sin_port = htons(netconf2->pcport); 
//     servaddr.sin_addr.s_addr = netconf2->pcip.single;
//     scr_printf(" lwip_connect... ");
//     int err;
//     if((err = lwip_connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)))!=0){
//         scr_printf("Socket connection with PC failed with err=%d\n", err); 
//         return;
//     }
//     scr_printf(" Let's write!\n");
//     char *hello = "Hello from client\n";
//     int ctr = 0;
//     for(;;){
//         err = lwip_write(sockfd, (const char *)hello, strlen(hello));
//         if (err < 0) {
//             scr_printf("TCP socket data out to PC failed ret=%d\n", err); 
//             return;
//         }
//         scr_printf("%12d", ++ctr);
//         scr_setXY(0, scr_getY());
//         usleep(20000);
//     }
// }

const char* HEX_NIBBLE = "0123456789ABCDEF";

int open_opl_conf()
{
    mcGetInfo(0, 0, NULL, NULL, NULL);
    mcSync(0, NULL, NULL);
    mcGetInfo(1, 0, NULL, NULL, NULL);
    mcSync(0, NULL, NULL);
    for (int mcID = 0; mcID < 2; mcID++)
    {
        char *path = "mcX:OPL/conf_network.cfg";
        path[2] = mcID ? '1' : '0';
        scr_printf(" > Retrieving OPL configuration from %s\n", path);
        int fd = open(path, O_RDONLY);
        if (fd > 0)
            return fd;
        else
            scr_printf(" > Configuration not found in MC%d\n", mcID);
    }
    return -1;
}

u8 lower_nibble_hex(u8 c){
    return HEX_NIBBLE[c & 0x0F];
}

u8 upper_nibble_hex(u8 c){
    return lower_nibble_hex(c >> 4);
}

void when_interneted_udp(struct oplnetconf *netconf2, u8** padBufs, struct padButtonStatus* padsButtons);

void when_interneted(struct oplnetconf *netconf){
    if(netconf == NULL) return;
    if(netconf->pcport == 0) return;
    struct oplnetconf *netconf2 = calloc(sizeof(struct oplnetconf), 1);
    if(netconf2 == NULL) return;
    memcpy(netconf2, netconf, sizeof(struct oplnetconf));
    netconf2->pcport += 1024;
    char padBuf1[256] __attribute__((aligned(64)));
    char padBuf2[256] __attribute__((aligned(64)));
    bzero(padBuf1, sizeof(padBuf1));
    bzero(padBuf2, sizeof(padBuf2));
    char padBufs[2][256] = {padBuf1, padBuf2};
    scr_printf("Initializing driver for DualShock...\n");
    ps2padInitModules();
    for (int port = 0; port<2; port++){
        scr_printf("Pad %d: ", port);
        scr_printf("opening... ");
        int ret;
        if((ret = padPortOpen(port, 0, padBufs[port])) == 0) {
            scr_printf("port %d: padOpenPort failed: %d\n", port, ret);
            return;
        }
        padSetMainMode(port, 0, PAD_MMODE_DIGITAL, PAD_MMODE_UNLOCK);
        // scr_printf("initializing... ");
        // if(!initializePad(port, 0)) {
        //     scr_printf("port %d: pad initalization failed!\n", port);
        //     return;
        // }
        scr_printf("done\n");
    }
    **padBufs = **padBufs;
    struct padButtonStatus* padsButtons = calloc(sizeof(struct padButtonStatus), 2);
    if (padsButtons==NULL) return;
    when_interneted_udp(netconf2, padBufs, padsButtons);
}

void when_interneted_udp(struct oplnetconf *netconf2, u8** padBufs, struct padButtonStatus* padsButtons){
    print_opl_net_target(netconf2);
    struct sockaddr_in servaddr;
    int sockfd;
    scr_printf(" > lwip_socket... ");
    if ((sockfd = lwip_socket(AF_INET, SOCK_DGRAM, 0)) < 0) { 
        scr_printf("Socket creation failed\n"); 
        return;
    }
    bzero(&servaddr, sizeof(servaddr)); 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(netconf2->pcport); 
    servaddr.sin_addr.s_addr = netconf2->pcip.single;
    scr_printf(" Let's write!\n");
    char padReady[2] = {0, 0};
    int ctrlStrSize = 2*(sizeof(*padsButtons)+3*sizeof(u8))+sizeof(u8);
    int outstrExpectedSize = 4*(sizeof(*padsButtons)+3*sizeof(u8))+3*sizeof(u8);
    char *outstr = calloc(1, outstrExpectedSize);
    if(outstr == NULL) return;
    memset(outstr, ' ', sizeof(outstr)-1);
    outstr[ctrlStrSize-sizeof(u8)] = '\n';
    outstr[outstrExpectedSize-2] = '\n';
    outstr[outstrExpectedSize-1] = '\0';
    int err;
    u64 ctr;
    for(ctr = 1; 1; ctr++){
        for(int port=0; port<2; port++){
            char pad_state = padGetState(port, 0);
            char pad_read = 0;
            if (pad_state == PAD_STATE_STABLE || pad_state == PAD_STATE_FINDCTP1){
                if(padReady[port] == 0) {
                    initializePad(port, 0);
                    padReady[port] = 1;
                }
                pad_read = padRead(port, 0, &padsButtons[port]);
            }else{
                padReady[port] = 0;
                bzero(&padsButtons[port], sizeof(*padsButtons));
            }
            char *soc = &outstr[port+port*(2*(sizeof(*padsButtons)+3*sizeof(u8)))];
            soc[0] = '1'+port;
            soc[1] = '0'; // pre-reserved the "slot" for whoever owns a multitap ;D
            soc[2] = upper_nibble_hex(pad_state);
            soc[3] = lower_nibble_hex(pad_state);
            soc[4] = upper_nibble_hex(pad_read);
            soc[5] = lower_nibble_hex(pad_read);
            u8 *thispad = &padsButtons[port];
            for(int byteNo = 0; byteNo < sizeof(*padsButtons); byteNo++){
                soc[2*byteNo+6] = upper_nibble_hex(thispad[byteNo]);
                soc[2*byteNo+7] = lower_nibble_hex(thispad[byteNo]);
            }
            err = lwip_sendto(sockfd, (const char *)soc, ctrlStrSize, 0, (const struct sockaddr*) &servaddr, sizeof(servaddr));
            if (err < 0) {
                scr_printf("UDP socket data out to PC failed ret=%d\n", err); 
                return;
            }
        }
        // err = lwip_sendto(sockfd, (const char *)soc, strlen(outstr), 0, (const struct sockaddr*) &servaddr, sizeof(servaddr));
        // if (err < 0) {
        //     scr_printf("UDP socket data out to PC failed ret=%d\n", err); 
        //     return;
        // }
        if (ctr%75 == 0){
            scr_printf("%20llu cycles; size=%d\n", ctr, ctrlStrSize);
            scr_printf("%s", outstr);
            scr_setXY(0, scr_getY()-3);
        }
        usleep(1000000/480);
        //waitVSync();
        EE_SYNC();
    }
}

int main(int argc, char **argv)
{
    ps2IOPinit();
    init_scr();
    scr_printf("Initializing driver for Memory Card...\n");
    ps2mcInitModules();
    scr_printf("Reading OPL network config from MC...\n");
    int oplnetfd = open_opl_conf();
    scr_printf(" < fd=%d\n", oplnetfd);
    if (oplnetfd < 0)
        goto failure;
    int size = lseek(oplnetfd, 0, SEEK_END);
    if (size < 0)
        goto failure;
    lseek(oplnetfd, 0, SEEK_SET);
    char *netcnf = (char *)calloc(size + 1, sizeof(char));
    if (netcnf == NULL)
        goto failure;
    int bytes_read = 0;
    int bytes_read_pass = -1;
    do
    {
        bytes_read_pass = read(oplnetfd, &netcnf[bytes_read], size - bytes_read);
        if (bytes_read_pass > 0)
            bytes_read += bytes_read_pass;
    } while (bytes_read_pass > 0 && bytes_read - size > 0);
    if (bytes_read_pass <= 0)
        goto failure;
    ps2IOPinit();
    init_scr();
    struct oplnetconf* netobj = parse_opl(netcnf);
    if (netobj==NULL){
        scr_printf("Could not parse OPL's network configuration\n");
        goto failure;
    }
    scr_printf("Initializing driver for Network Interface...\n");
    ps2ipInitModules();
    print_opl_net_bind(netobj);
    scr_printf("Configuring network stack...\n");
    ps2ip(netobj, &when_interneted, netobj);
failure:
    scr_printf("Failure on main()\n");
    SleepThread();
    return 0;
}