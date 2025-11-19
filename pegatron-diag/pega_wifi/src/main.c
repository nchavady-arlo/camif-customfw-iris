// wifi_status_ioctl.c
// compile: gcc -o wifi_status_ioctl wifi_status_ioctl.c
// usage: sudo ./wifi_status_ioctl wlan0

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/wireless.h>
#include <linux/if.h>

int main(int argc, char **argv) 
{
    if (argc < 2) 
	{
        fprintf(stderr, "Usage: %s <ifname>\n", argv[0]);
        return 1;
    }
	
    const char *ifname = argv[1];

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    struct iwreq wrq;
    memset(&wrq, 0, sizeof(wrq));
    strncpy(wrq.ifr_name, ifname, IFNAMSIZ-1);

    // 1) Get ESSID (SSID)
    char essid[IW_ESSID_MAX_SIZE+1];
    wrq.u.essid.pointer = essid;
    wrq.u.essid.length = IW_ESSID_MAX_SIZE;
    wrq.u.essid.flags = 0;
    if (ioctl(sock, SIOCGIWESSID, &wrq) == 0) 
	{
        essid[wrq.u.essid.length] = '\0';
        printf("SSID: %s\n", essid);
    } 
	else 
	{
        printf("SSID: (not available or not associated)\n");
    }

    // 2) Get AP MAC (BSSID)
    struct iwreq bssreq;
    memset(&bssreq, 0, sizeof(bssreq));
    strncpy(bssreq.ifr_name, ifname, IFNAMSIZ-1);
    if (ioctl(sock, SIOCGIWAP, &bssreq) == 0) 
	{
        unsigned char *mac = (unsigned char *)bssreq.u.ap_addr.sa_data;
        printf("BSSID: %02x:%02x:%02x:%02x:%02x:%02x\n",
               mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    } 
	else 
	{
        printf("BSSID: (not available)\n");
    }

    // 3) Get link quality / signal level (if supported)
    struct iw_statistics stats;
    memset(&wrq, 0, sizeof(wrq));
    strncpy(wrq.ifr_name, ifname, IFNAMSIZ-1);
    wrq.u.data.pointer = &stats;
    wrq.u.data.length = sizeof(stats);
    wrq.u.data.flags = 1; // clear updated flag (implementation detail)
    if (ioctl(sock, SIOCGIWSTATS, &wrq) == 0) 
	{
        if (stats.qual.updated & IW_QUAL_QUAL_UPDATED) 
		{
            printf("Link quality: %d / %d\n", stats.qual.qual, 70); // 70 often max
        }
		
        if (stats.qual.updated & IW_QUAL_LEVEL_UPDATED) 
		{
            // note: level may be in dBm or arbitrary units
            int level = stats.qual.level;
            if (stats.qual.level > 64) 
				level -= 256; // convert signed if needed
            printf("Signal level: %d dBm (or units)\n", level);
        }
    } 
	else 
	{
        printf("Signal info: (not available)\n");
    }

    close(sock);
    return 0;
}
