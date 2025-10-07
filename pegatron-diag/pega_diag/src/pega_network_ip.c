#include <sys/types.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <stdlib.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <linux/if_link.h>
//==============================================================================
#include "pega_network_ip.h"
//==============================================================================
int pega_netip_parse_inet6(const char *ifname, char *ip, char bDebug)
{
    FILE *f;
    int ret, scope, prefix;
    unsigned char ipv6[16];
    char dname[IFNAMSIZ];
    char address[INET6_ADDRSTRLEN];
    char *scopestr;
    int rtn = -1;
	
    f = fopen("/proc/net/if_inet6", "r");
    
	if (f == NULL) 
	{
        return -1;
    }
 
    while (19 == fscanf(f,
                        " %2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx %*x %x %x %*x %s",
                        &ipv6[0],
                        &ipv6[1],
                        &ipv6[2],
                        &ipv6[3],
                        &ipv6[4],
                        &ipv6[5],
                        &ipv6[6],
                        &ipv6[7],
                        &ipv6[8],
                        &ipv6[9],
                        &ipv6[10],
                        &ipv6[11],
                        &ipv6[12],
                        &ipv6[13],
                        &ipv6[14],
                        &ipv6[15],
                        &prefix,
                        &scope,
                        dname)) {
 
        if (strcmp(ifname, dname) != 0) {
            continue;
        }
 
        if (inet_ntop(AF_INET6, ipv6, address, sizeof(address)) == NULL) {
            continue;
        }
 
        switch (scope) {
        case IPV6_ADDR_GLOBAL:
            scopestr = "Global";
            break;
        case IPV6_ADDR_LINKLOCAL:
            scopestr = "Link";
            break;
        case IPV6_ADDR_SITELOCAL:
            scopestr = "Site";
            break;
        case IPV6_ADDR_COMPATv4:
            scopestr = "Compat";
            break;
        case IPV6_ADDR_LOOPBACK:
            scopestr = "Host";
            break;
        default:
            scopestr = "Unknown";
        }
 
        rtn = 0;
		
		if (ip != NULL)
		{
			strcpy(ip, address);
		}
		
		if (bDebug > 0)
		{
			printf("IPv6 address: %s(%d), prefix: %d, scope: %s\n", address, strlen(address), prefix, scopestr);
		}
    }
 
    fclose(f);
	
	if ((rtn < 0) && (bDebug > 0))
	{
		//printf("IPv6 address: NULL\n");
	}
	
	return rtn;
}

//pega_ip ipv4 eth0
//pega_ip ipv4 wlan0 //wifi
//pega_ip ipv4 wwan0 //5G
int pega_netip_parse_inet4(const char *ifname, char *ip, char bDebug)
{
    int sock;
    struct ifreq ifr;
    struct sockaddr_in *ipaddr;
    char address[INET_ADDRSTRLEN];
    size_t ifnamelen;
    int rtn = -1;
    /* copy ifname to ifr object */
    ifnamelen = strlen(ifname);
	
    if (ifnamelen >= sizeof(ifr.ifr_name)) 
	{
        return rtn;
    }
	
    memcpy(ifr.ifr_name, ifname, ifnamelen);
    ifr.ifr_name[ifnamelen] = '\0';
 
    /* open socket */
    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) 
	{
        return rtn;
    }
 
    /* process mac */
    if (ioctl(sock, SIOCGIFHWADDR, &ifr) != -1) 
	{
		if (bDebug > 0)
		{
        printf("Mac address: %02x:%02x:%02x:%02x:%02x:%02x\n",
                (unsigned char)ifr.ifr_hwaddr.sa_data[0],
                (unsigned char)ifr.ifr_hwaddr.sa_data[1],
                (unsigned char)ifr.ifr_hwaddr.sa_data[2],
                (unsigned char)ifr.ifr_hwaddr.sa_data[3],
                (unsigned char)ifr.ifr_hwaddr.sa_data[4],
                (unsigned char)ifr.ifr_hwaddr.sa_data[5]);
		}
    }
 
    #if 0
    /* process mtu */
    if (ioctl(sock, SIOCGIFMTU, &ifr) != -1) 
	{
        printf("MTU: %d\n", ifr.ifr_mtu);
    }
	#endif
 
    /* die if cannot get address */
    if (ioctl(sock, SIOCGIFADDR, &ifr) == -1) 
	{
        close(sock);
        return rtn;
    }
 
    /* process ip */
    ipaddr = (struct sockaddr_in *)&ifr.ifr_addr;
	
    if (inet_ntop(AF_INET, &ipaddr->sin_addr, address, sizeof(address)) != NULL) 
	{
		if (ip != NULL)
		{
			strcpy(ip, address);
		}
		
		rtn = 0;
		
		if (bDebug > 0)
		{
			printf("IPv4 address: %s\n", address);
		}
    }
 
    #if 0
    /* try to get broadcast */
    if (ioctl(sock, SIOCGIFBRDADDR, &ifr) != -1) {
        ipaddr = (struct sockaddr_in *)&ifr.ifr_broadaddr;
        if (inet_ntop(AF_INET, &ipaddr->sin_addr, address, sizeof(address)) != NULL) {
            printf("Broadcast: %s\n", address);
        }
    }
	#endif
 
    #if 0
    /* try to get mask */
    if (ioctl(sock, SIOCGIFNETMASK, &ifr) != -1) {
        ipaddr = (struct sockaddr_in *)&ifr.ifr_netmask;
        if (inet_ntop(AF_INET, &ipaddr->sin_addr, address, sizeof(address)) != NULL) {
            printf("Netmask: %s\n", address);
        }
    }
	#endif
	
    close(sock);
	
	if ((rtn < 0) && (bDebug > 0))
	{
		printf("IPv4 address: NULL\n");
	}
	
	return rtn;
}

int pega_netip_ip_get(const char *ifname, char *ip, int len)
{	
    if (ifname == NULL)
	{
		printf("[ERR]ifname is NULL\n");
		return -1;
	}
	
	//printf("\n[%s]ifname=%s\n", __func__, ifname); 
	
	memset(ip, 0, len);
		
	if (pega_netip_parse_inet4(ifname, ip, 0) > -1)
	{	
		return 0;		
	}
	
	if (pega_netip_parse_inet6(ifname, ip, 0) > -1)
	{	
		return 0;		
	}
	
	return -1;
}
//pega_ip ifname eth0
//pega_ip ifname mlan0 //wifi 1
//pega_ip ifname mlan1 //wifi 2
int pega_netip_is_interface_exist(const char *ifname, int bDebug)
{
	int bRtn = -1;
	
    DIR *pDir;
    struct dirent *de;
 
    if (ifname == NULL) 
	{
        return bRtn;
    }
	
    pDir = opendir("/sys/class/net/");
	
    if (pDir == NULL) 
	{
        return bRtn;
    }
 
    while (NULL != (de = readdir(pDir))) 
	{
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) 
		{
            continue;
        }
				
		if (!strcmp(de->d_name, ifname))
		{
			if (bDebug > 0)
			{
				printf("Found: Interface %s\n", de->d_name);
			}
			bRtn = 0;
			break;
		}
    }
    
	closedir(pDir);
	
    return bRtn;
}

//pega_ip btf hci0
int pega_bt_is_interface_exist(const char *ifname, int bDebug)
{
	int bRtn = -1;
	
    DIR *pDir;
    struct dirent *de;
 
    if (ifname == NULL) 
	{
        return bRtn;
    }
	
    pDir = opendir("/sys/class/bluetooth/");
	
    if (pDir == NULL) 
	{
        return bRtn;
    }
 
    while (NULL != (de = readdir(pDir))) 
	{
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) 
		{
            continue;
        }
				
		if (!strcmp(de->d_name, ifname))
		{
			if (bDebug > 0)
			{
				printf("Found: Interface %s\n", de->d_name);
			}
			bRtn = 0;
			break;
		}
    }
    
	closedir(pDir);
	
    return bRtn;
}

