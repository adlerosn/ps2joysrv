#ifndef __MYOPLNETPARSER_H__
#define __MYOPLNETPARSER_H__

#include <stdint.h>
#include <tamtypes.h>

#define ATTRPKD __attribute__((__packed__))

struct ATTRPKD quadroctets {
    u8 octet1;
    u8 octet2;
    u8 octet3;
    u8 octet4;
};

union ATTRPKD u32quadroctets {
    u32 single;
    struct quadroctets spread;
};

struct ATTRPKD oplnetconf {
    s32 usedhcp;
    union u32quadroctets ps2ip;
    union u32quadroctets ps2nm;
    union u32quadroctets ps2gw;
    union u32quadroctets ps2dns;
    union u32quadroctets pcip;
    u16 pcport;
};

struct oplnetconf *parse_opl(const char* text);
void print_opl_net_bind(struct oplnetconf * netconf);
void print_opl_net_target(struct oplnetconf * netconf);


#endif // __MYOPLNETPARSER_H__
