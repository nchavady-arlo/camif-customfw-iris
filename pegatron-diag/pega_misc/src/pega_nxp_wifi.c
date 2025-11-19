/*******************************************************************************
* File Name: pega_nxp_wifi.c
*
*******************************************************************************/
//==============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <getopt.h>

#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/if.h>
#include <sys/stat.h>
#include <net/ethernet.h>
//==============================================================================
#include "pega_nxp_wifi.h"
//==============================================================================
/** Socket */
static int m_SocketFd = -1;
//==============================================================================
static int Pega_nxp_wifi_CountrycodeCheck(char *country_code)
{
	int i=0;
	static char country_code_table[16][3] = 
	{
	{"WW"}, /* World       */
	{"US"}, /* US FCC      */
	{"CA"}, /* IC Canada   */
	{"SG"}, /* Singapore   */
	{"EU"}, /* ETSI        */
	{"AU"}, /* Australia   */
	{"KR"}, /* Republic Of Korea */
	{"JP"}, /* Japan       */
	{"CN"}, /* China       */
	{"TW"}, /* TW support  */
	{"BR"}, /* Brazil      */
	{"RU"}, /* Russia      */
	{"IN"}, /* India       */
	{"MY"}, /* Malaysia    */
	{"NZ"}, /* New Zeland  */
	{"MX"}, /* Mexico */
	};
	
	if ((country_code == NULL) || (strlen(country_code) != 2))
	{
		return -1;
	}	
	
	//printf("%s(%d)\n",__FUNCTION__, strlen(country_code)); 
	
	for (i=0;i<16;i++)
	{
		//printf("country_code[%d]=%s\n", i, country_code_table[i]);
		
		if (!strncmp(country_code_table[i], country_code, strlen(country_code)))
		{			
			return i;
		}
	}

	return -1;
}
//==============================================================================
int Pega_nxp_wifi_socket_open(void)
{
	m_SocketFd = socket(AF_INET, SOCK_STREAM, 0);
	
	if (m_SocketFd < 0) 
	{
		fprintf(stderr, "mlanutl: Cannot open socket.\n");
		return -1;
	}
	
	return 0;
}

int Pega_nxp_wifi_socket_close(void)
{		
	if (m_SocketFd != -1) 
	{
		close(m_SocketFd);		
	}
	
	return 0;
}
//==============================================================================
static int Pega_nxp_wifi_country_code_update(const char *ifname, const uint8_t *countryCode)
{
	uint8_t *buffer = NULL;
	struct eth_priv_cmd *cmd = NULL;	
	struct ifreq ifr;
	int length = 0;
	
	/* Initialize buffer */
	buffer = (uint8_t *)malloc(BUFFER_LENGTH);
	
	if (!buffer) 
	{
		printf("ERR:Cannot allocate buffer for command!\n");
		return -1;
	}
	
	if (Pega_nxp_wifi_CountrycodeCheck(countryCode) < 0)
	{
		return -1;
	}
	
	snprintf(buffer, sizeof("MRVL_CMDcountrycode")+2, "MRVL_CMDcountrycode%s", countryCode);
	
	//printf("buffer(%s)(%d)\n", buffer, strlen(buffer)); 
	
	cmd = (struct eth_priv_cmd *)malloc(sizeof(struct eth_priv_cmd));
	
	if (!cmd) 
	{
		printf("ERR:Cannot allocate buffer for command!\n");
		free(buffer);
		return -1;
	}
	
	cmd->buf = buffer;
	cmd->used_len = 0;
	cmd->total_len = BUFFER_LENGTH;
	
	memset(&ifr, 0, sizeof(struct ifreq));
	
	strncpy(ifr.ifr_ifrn.ifrn_name, ifname, strlen("ifname"));
	
	ifr.ifr_ifru.ifru_data = (void *)cmd;

	if (ioctl(m_SocketFd, MLAN_ETH_PRIV, &ifr)) 
	{
		perror("mlanutl");
		fprintf(stderr, "mlanutl: countrycode fail\n");
		if (cmd)
			free(cmd);
		if (buffer)
			free(buffer);
		return -1;
	}
	
	//printf("buffer(%s)(%d)\n", buffer, strlen(buffer)); 
	
	
	return 0;
}
//==============================================================================
int Pega_nxp_wifi_country_code_get(const char *ifname)
{
	uint8_t *buffer = NULL;
	struct eth_priv_cmd *cmd = NULL;	
	struct ifreq ifr;
	int length = 0;
	
	/* Initialize buffer */
	buffer = (uint8_t *)malloc(BUFFER_LENGTH);
	
	if (!buffer) 
	{
		printf("ERR:Cannot allocate buffer for command!\n");
		return -1;
	}
	
	memcpy(&buffer[0], "MRVL_CMDcountrycode", sizeof("MRVL_CMDcountrycode"));
	
	//printf("buffer(%s)(%d)\n", buffer, strlen(buffer)); 
	
	cmd = (struct eth_priv_cmd *)malloc(sizeof(struct eth_priv_cmd));
	
	if (!cmd) {
		printf("ERR:Cannot allocate buffer for command!\n");
		free(buffer);
		return -1;
	}
	
	cmd->buf = buffer;
	cmd->used_len = 0;
	cmd->total_len = BUFFER_LENGTH;
	
	memset(&ifr, 0, sizeof(struct ifreq));
	
	strncpy(ifr.ifr_ifrn.ifrn_name, ifname, strlen(ifname));
	
	ifr.ifr_ifru.ifru_data = (void *)cmd;

	if (ioctl(m_SocketFd, MLAN_ETH_PRIV, &ifr)) 
	{
		perror("mlanutl");
		fprintf(stderr, "mlanutl: countrycode fail\n");
		if (cmd)
			free(cmd);
		if (buffer)
			free(buffer);
		return -1;
	}
	
	//printf("buffer(%s)(%d)\n", buffer, strlen(buffer)); 	
	
	return 0;
}

int Pega_nxp_wifi_country_code_set(const char *ifname, const uint8_t *countryCode)
{
	Pega_nxp_wifi_socket_open();
	if (Pega_nxp_wifi_country_code_update(ifname, countryCode) == 0)
	{
		fprintf(stdout, "[%s]countryCode=%s\n", ifname, countryCode);
	}
	Pega_nxp_wifi_socket_close();
}
//==============================================================================