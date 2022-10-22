#include <stdlib.h>
#include <tamtypes.h>
#include <debug.h>
#include "myoplnetparser.h"

const char* PS2_IP = "ps2_ip_addr=";
const char* PS2_NM = "ps2_netmask=";
const char* PS2_GW = "ps2_gateway=";
const char* PS2_DNS = "ps2_dns=";
const char* PS2_DHCP = "ps2_ip_use_dhcp=";
const char* SMB_IP = "smb_ip=";
const char* SMB_PORT = "smb_port=";



s32 parse_s32(s32 *number, const char* text);
s32 parse_u16(u16 *number, const char* text);
s32 parse_u8(u8 *number, const char* text);
s32 parse_ip(struct quadroctets *ip, const char* text);
s32 starts_with(const char* longer, const char* shorter);
s32 skip_until_past_newline(const char* text);
s32 skip_until_past_char(const char* text, char c);

struct oplnetconf *parse_opl(const char* text) {
    struct oplnetconf *nc = calloc(sizeof(struct oplnetconf), 1);
    if(nc==NULL) return NULL;
    const char* p = text;
    while(p[0] != 0){
        if(starts_with(p, PS2_IP)){
            p+=skip_until_past_char(p, '=');
            p+=parse_ip(&nc->ps2ip.spread, p);
        }
        else if(starts_with(p, PS2_NM)){
            p+=skip_until_past_char(p, '=');
            p+=parse_ip(&nc->ps2nm.spread, p);
        }
        else if(starts_with(p, PS2_GW)){
            p+=skip_until_past_char(p, '=');
            p+=parse_ip(&nc->ps2gw.spread, p);
        }
        else if(starts_with(p, PS2_DNS)){
            p+=skip_until_past_char(p, '=');
            p+=parse_ip(&nc->ps2dns.spread, p);
        }
        else if(starts_with(p, SMB_IP)){
            p+=skip_until_past_char(p, '=');
            p+=parse_ip(&nc->pcip.spread, p);
        }
        else if(starts_with(p, SMB_PORT)){
            p+=skip_until_past_char(p, '=');
            u16 t = nc->pcport;
            p+=parse_u16(&t, p);
            nc->pcport = t;
        }
        else if(starts_with(p, PS2_DHCP)){
            p+=skip_until_past_char(p, '=');
            s32 t = nc->usedhcp;
            p+=parse_s32(&t, p);
            nc->usedhcp = t;
        }
        else{
            p+=skip_until_past_newline(p);
        }
    }
    return nc;
}

s32 parse_s32(s32 *number, const char* text) {
    int i;
    for(i=0; '0' <= text[i] && text[i] <= '9'; i++)
        *number = (*number)*10 + (text[i]-'0');
    return i;
}

s32 parse_u16(u16 *number, const char* text) {
    int i;
    for(i=0; '0' <= text[i] && text[i] <= '9'; i++)
        *number = (*number)*10 + (text[i]-'0');
    return i;
}

s32 parse_u8(u8 *number, const char* text) {
    int i;
    for(i=0; '0' <= text[i] && text[i] <= '9'; i++)
        *number = (*number)*10 + (text[i]-'0');
    return i;
}

s32 parse_ip(struct quadroctets *ip, const char* text) {
    const char *p = text;
    p+=parse_u8(&ip->octet1, p);
    if(p[0]=='.')
        p+=1+parse_u8(&ip->octet2, &p[1]);
    if(p[0]=='.')
        p+=1+parse_u8(&ip->octet3, &p[1]);
    if(p[0]=='.')
        p+=1+parse_u8(&ip->octet4, &p[1]);
    return p-text;
}

s32 starts_with(const char* longer, const char* shorter) {
    int i;
    for(i = 0; longer[i] == shorter[i] && longer[i] != 0 && shorter[i] != 0; i++);
    if(shorter[i] == 0) return -1;
    return 0;
}

s32 skip_until_past_newline(const char* text) {
    int i = 0;
    while(text[i] != 0 && (text[i] != '\n' && text[i] != '\r')) i++;
    while(text[i] != 0 && (text[i] == '\n' || text[i] == '\r')) i++;
    return i;
}

s32 skip_until_past_char(const char* text, char c){
    int i = 0;
    while(text[i] != 0 && text[i] != c) i++;
    while(text[i] != 0 && text[i] == c) i++;
    return i;
}

void print_opl_net_bind(struct oplnetconf * netconf) {
    if(netconf==NULL) return;
    scr_printf("DHCP:    %d\n", netconf->usedhcp);
    scr_printf("PS2 IP:  %03d.%03d.%03d.%03d\n", netconf->ps2ip.spread.octet1, netconf->ps2ip.spread.octet2, netconf->ps2ip.spread.octet3, netconf->ps2ip.spread.octet4);
    scr_printf("PS2 NM:  %03d.%03d.%03d.%03d\n", netconf->ps2nm.spread.octet1, netconf->ps2nm.spread.octet2, netconf->ps2nm.spread.octet3, netconf->ps2nm.spread.octet4);
    scr_printf("PS2 GW:  %03d.%03d.%03d.%03d\n", netconf->ps2gw.spread.octet1, netconf->ps2gw.spread.octet2, netconf->ps2gw.spread.octet3, netconf->ps2gw.spread.octet4);
    scr_printf("PS2 DNS: %03d.%03d.%03d.%03d\n", netconf->ps2dns.spread.octet1, netconf->ps2dns.spread.octet2, netconf->ps2dns.spread.octet3, netconf->ps2dns.spread.octet4);
}
void print_opl_net_target(struct oplnetconf * netconf) {
    if(netconf==NULL) return;
    scr_printf("PC IP:   %03d.%03d.%03d.%03d\n", netconf->pcip.spread.octet1, netconf->pcip.spread.octet2, netconf->pcip.spread.octet3, netconf->pcip.spread.octet4);
    scr_printf("PC PORT: %05d\n", netconf->pcport);
}