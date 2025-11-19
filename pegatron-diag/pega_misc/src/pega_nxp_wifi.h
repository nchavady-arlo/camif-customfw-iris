#ifndef _PEGA_NXP_WIFI_H
#define _PEGA_NXP_WIFI_H
//==============================================================================
//#include <netinet/ether.h>
#include <sys/ioctl.h>
//==============================================================================
/** Command buffer max length */
#define BUFFER_LENGTH (512)
#define IFNAMSIZ 16
/** IOCTL number */
#define MLAN_ETH_PRIV (SIOCDEVPRIVATE + 14)
//==============================================================================
/** NXP private command identifier string */
#define CMD_NXP "MRVL_CMD"
//==============================================================================
struct eth_priv_cmd {
	/** Command buffer */
	uint8_t *buf;
	/** Used length */
	int used_len;
	/** Total length */
	int total_len;
};
//==============================================================================
int Pega_nxp_wifi_socket_open(void);
int Pega_nxp_wifi_socket_close(void);
//==============================================================================
int Pega_nxp_wifi_country_code_get(const char *ifname);
int Pega_nxp_wifi_country_code_set(const char *ifname, const uint8_t *countryCode);
//==============================================================================

#endif //_PEGA_NXP_WIFI_H
