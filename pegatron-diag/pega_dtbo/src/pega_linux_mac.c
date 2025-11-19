/*******************************************************************************
* File Name: pega_linux_mac.c
*
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/if_arp.h>
//==============================================================================
#include "pega_linux_mac.h"
//==============================================================================
// 將 MAC 字串 "AA:BB:CC:DD:EE:FF" 轉成 6 bytes
static int parse_mac(const char *mac_str, unsigned char *mac) 
{
    if (sscanf(mac_str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
               &mac[0], &mac[1], &mac[2],
               &mac[3], &mac[4], &mac[5]) != 6) {
        return -1;
    }
    return 0;
}

int Pega_linux_mac_address_set(const char *ifname, const char *mac_str)
{
    int sockfd;
    struct ifreq ifr;
    unsigned char mac[6];

    if (parse_mac(mac_str, mac) != 0) {
        fprintf(stderr, "Invalid MAC format: %s\n", mac_str);
        return -1;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);

    ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
    memcpy(ifr.ifr_hwaddr.sa_data, mac, 6);

    // 設定 MAC 位址
    if (ioctl(sockfd, SIOCSIFHWADDR, &ifr) < 0) {
        perror("ioctl(SIOCSIFHWADDR)");
        close(sockfd);
        return -1;
    }

    close(sockfd);
    return 0;
}