#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// Generally you would define your own explicit list of lwIP options
// (see https://www.nongnu.org/lwip/2_1_x/group__lwip__opts.html)
//
// This example uses a common include to avoid repetition
#include "lwipopts_examples_common.h"

#define LWIP_DNS                    1
#define DNS_TABLE_SIZE              2
#define DNS_MAX_NAME_LENGTH         256
#define LWIP_DHCP                   1
#define LWIP_ICMP 1

// Required memory settings for DNS resolution
#define MEMP_NUM_SYS_TIMEOUT        (MEMP_NUM_TCP_PCB + MEMP_NUM_UDP_PCB + MEMP_NUM_RAW_PCB + MEMP_NUM_UDP_PCB + 3)

#endif
