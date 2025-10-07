#ifndef _PEGA_NETWORK_IP_H_
#define _PEGA_NETWORK_IP_H_
//==============================================================================
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
#define IPV6_ADDR_GLOBAL        0x0000U
#define IPV6_ADDR_LOOPBACK      0x0010U
#define IPV6_ADDR_LINKLOCAL     0x0020U
#define IPV6_ADDR_SITELOCAL     0x0040U
#define IPV6_ADDR_COMPATv4      0x0080U 
//==============================================================================
int pega_netip_ip_get(const char *ifname, char *ip, int len);
int pega_netip_is_interface_exist(const char *ifname, int bDebug);
//==============================================================================
#ifdef __cplusplus
}
#endif
#endif //_PEGA_NETWORK_IP_H_
