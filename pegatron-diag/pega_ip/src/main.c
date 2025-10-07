#include <sys/socket.h>
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
#include <linux/if_link.h>
//==============================================================================
#define IPV6_ADDR_GLOBAL        0x0000U
#define IPV6_ADDR_LOOPBACK      0x0010U
#define IPV6_ADDR_LINKLOCAL     0x0020U
#define IPV6_ADDR_SITELOCAL     0x0040U
#define IPV6_ADDR_COMPATv4      0x0080U 
//==============================================================================
struct ifaddrs 
{
   struct ifaddrs  *ifa_next;    /* Next item in list */
   char            *ifa_name;    /* Name of interface */
   unsigned int     ifa_flags;   /* Flags from SIOCGIFFLAGS */
   struct sockaddr *ifa_addr;    /* Address of interface */
   struct sockaddr *ifa_netmask; /* Netmask of interface */
   union 
   {
       struct sockaddr *ifu_broadaddr;  /* Broadcast address of interface */
       struct sockaddr *ifu_dstaddr;   /* Point-to-point destination address */
   } ifa_ifu;
#define              ifa_broadaddr ifa_ifu.ifu_broadaddr
#define              ifa_dstaddr   ifa_ifu.ifu_dstaddr
   void            *ifa_data;    /* Address-specific data */
};
//==============================================================================
//get_local_ipv6("wlan0", NULL);
int get_local_ipv6(const char *eth_inf, char *ip)
{
   struct ifaddrs *ifaddr;
   int family, s;
   char host[NI_MAXHOST];
   int rtn = -1;
   
   if (getifaddrs(&ifaddr) == -1) 
   {  //通?getifaddrs?得ifaddrs ?构体
       //perror("getifaddrs");
	   printf("getifaddrs error\n");
       return rtn;
   }
   
   /* Walk through linked list, maintaining head pointer so we can free list later */
   for (struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) 
   {
       if (ifa->ifa_addr == NULL)
	   {
           continue;
	   }
 
       family = ifa->ifa_addr->sa_family;  //通?family?确定包的?型
 
       /* Display interface name and family (including symbolic form of the latter for the common families) */
 
       #if 0
       printf("%-8s %s (%d)\n",
              ifa->ifa_name,
              (family == AF_PACKET) ? "AF_PACKET" :
              (family == AF_INET) ? "AF_INET" :
              (family == AF_INET6) ? "AF_INET6" : "???",
              family);
       #endif
       /* For an AF_INET* interface address, display the address */
 
       if ((family == AF_INET6) && (!strcmp(ifa->ifa_name, eth_inf)))  
	   {
           s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in6), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
           
		   if (s != 0) 
		   {
               printf("getnameinfo() failed: %s\n", gai_strerror(s));
               //exit(EXIT_FAILURE);
			   break;
           }
		   
		   rtn = 0;//success.
           printf("\t\taddress: <%s>\n", host); 
       } 	   
   }
 
   freeifaddrs(ifaddr);
   
   return rtn;
}
//==============================================================================
//pega_ip ipv6 eth0
//pega_ip ipv6 mlan0 //wifi 1
//pega_ip ipv6 mlan1 //wifi 2
int parse_inet6(const char *ifname, char *ip, char bDebug)
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
		printf("IPv6 address: NULL\n");
	}
	
	return rtn;
}
//pega_ip ipv4 eth0
//pega_ip ipv4 mlan0 //wifi 1
//pega_ip ipv4 mlan1 //wifi 2
int parse_inet4(const char *ifname, char *ip, char bDebug)
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
 
int parse_ioctl(const char *ifname)
{
    int sock;
    struct ifreq ifr;
    struct sockaddr_in *ipaddr;
    char address[INET_ADDRSTRLEN];
    size_t ifnamelen;
 
    /* copy ifname to ifr object */
    ifnamelen = strlen(ifname);
    if (ifnamelen >= sizeof(ifr.ifr_name)) 
	{
        return -1;
    }
    memcpy(ifr.ifr_name, ifname, ifnamelen);
    ifr.ifr_name[ifnamelen] = '\0';
 
    /* open socket */
    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) 
	{
        return -1;
    }
 
    /* process mac */
    if (ioctl(sock, SIOCGIFHWADDR, &ifr) != -1) {
        printf("Mac address: %02x:%02x:%02x:%02x:%02x:%02x\n",
                (unsigned char)ifr.ifr_hwaddr.sa_data[0],
                (unsigned char)ifr.ifr_hwaddr.sa_data[1],
                (unsigned char)ifr.ifr_hwaddr.sa_data[2],
                (unsigned char)ifr.ifr_hwaddr.sa_data[3],
                (unsigned char)ifr.ifr_hwaddr.sa_data[4],
                (unsigned char)ifr.ifr_hwaddr.sa_data[5]);
    }
 
    /* process mtu */
    if (ioctl(sock, SIOCGIFMTU, &ifr) != -1) {
        printf("MTU: %d\n", ifr.ifr_mtu);
    }
 
    /* die if cannot get address */
    if (ioctl(sock, SIOCGIFADDR, &ifr) == -1) {
        close(sock);
        return -1;
    }
 
    /* process ip */
    ipaddr = (struct sockaddr_in *)&ifr.ifr_addr;
    if (inet_ntop(AF_INET, &ipaddr->sin_addr, address, sizeof(address)) != NULL) {
        printf("Ip address: %s\n", address);
    }
 
    /* try to get broadcast */
    if (ioctl(sock, SIOCGIFBRDADDR, &ifr) != -1) {
        ipaddr = (struct sockaddr_in *)&ifr.ifr_broadaddr;
        if (inet_ntop(AF_INET, &ipaddr->sin_addr, address, sizeof(address)) != NULL) {
            printf("Broadcast: %s\n", address);
        }
    }
 
    /* try to get mask */
    if (ioctl(sock, SIOCGIFNETMASK, &ifr) != -1) {
        ipaddr = (struct sockaddr_in *)&ifr.ifr_netmask;
        if (inet_ntop(AF_INET, &ipaddr->sin_addr, address, sizeof(address)) != NULL) {
            printf("Netmask: %s\n", address);
        }
    }
	
    close(sock);
	
	return 0;
}
 
static int local_ip_show(void)
{
    DIR *pDir;
    struct dirent *de;
 
    pDir = opendir("/sys/class/net/");
	
    if (pDir == NULL) 
	{
        return -1;
    }
 
    while (NULL != (de = readdir(pDir))) 
	{
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) 
		{
            continue;
        }
        printf("Interface %s\n", de->d_name);
 
        parse_ioctl(de->d_name);
 
        parse_inet6(de->d_name, NULL, 1);
 
        printf("\n");
    }
    
	closedir(pDir);
	
    return 0;
}
//pega_ip ifname eth0
//pega_ip ifname mlan0 //wifi 1
//pega_ip ifname mlan1 //wifi 2
static int is_interface_exist(const char *ifname, int bDebug)
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

int main(int argc, char *argv[])
{
	if (!strcmp(argv[1],"show"))
	{
		local_ip_show();
	}
	else if (!strcmp(argv[1],"ifname"))
	{
		if (argv[2] != NULL)
		{
			is_interface_exist(argv[2], 1);
		}
	}
	else if (!strcmp(argv[1], "ipv4"))//"wlan0"
	{
		if (argv[2] != NULL)
		{
			parse_inet4(argv[2], NULL, 1);
		}
	}
	else if (!strcmp(argv[1], "ipv6"))//"wlan0"
	{
		if (argv[2] != NULL)
		{
			parse_inet6(argv[2], NULL, 1);
		}
	}
	
	return 0;
}