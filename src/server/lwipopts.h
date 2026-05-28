#pragma once

#define NO_SYS                      1
#define LWIP_SOCKET                 0
#define MEM_LIBC_MALLOC             0
#define MEM_ALIGNMENT               4
#define MEM_SIZE                    4000
#define MEMP_NUM_TCP_SEG            32
#define MEMP_NUM_ARP_QUEUE          10
#define PBUF_POOL_SIZE              24

#define LWIP_ARP                    1
#define LWIP_ETHERNET               1
#define LWIP_ICMP                   1
#define LWIP_RAW                    1

#define TCP_WND                     (8 * TCP_MSS)
#define TCP_MSS                     1460
#define TCP_SND_BUF                 (8 * TCP_MSS)
#define TCP_SND_QUEUELEN            ((4 * (TCP_SND_BUF) + (TCP_MSS - 1)) / (TCP_MSS))

#define LWIP_NETIF_STATUS_CALLBACK  1
#define LWIP_NETIF_LINK_CALLBACK    1
#define LWIP_NETIF_HOSTNAME         1
#define LWIP_NETCONN                0

#define LWIP_HTTPD                  1  // Porneste robotul HTTPD
#define LWIP_HTTPD_SSI              1  // Activeaza SSI-ul (trimitere date)
#define LWIP_HTTPD_CGI              1  // Activeaza CGI-ul (butoanele)
#define LWIP_HTTPD_SSI_INCLUDE_TAG  0  // Setare curatare HTML

#define HTTPD_FSDATA_FILE "htmldata.c"
