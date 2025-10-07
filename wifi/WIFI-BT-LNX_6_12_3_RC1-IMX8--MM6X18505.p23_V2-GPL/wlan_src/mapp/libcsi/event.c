
/** @file  event.c
 *
 * @brief This file handles event for 11mc/11az Wifi location services
 * application
 *
 *  Usage:
 *
 *
 * Copyright 2024 NXP
 *
 * NXP CONFIDENTIAL
 * The source code contained or described herein and all documents related to
 * the source code (Materials) are owned by NXP, its
 * suppliers and/or its licensors. Title to the Materials remains with NXP,
 * its suppliers and/or its licensors. The Materials contain
 * trade secrets and proprietary and confidential information of NXP, its
 * suppliers and/or its licensors. The Materials are protected by worldwide
 * copyright and trade secret laws and treaty provisions. No part of the
 * Materials may be used, copied, reproduced, modified, published, uploaded,
 * posted, transmitted, distributed, or disclosed in any way without NXP's prior
 * express written permission.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery
 * of the Materials, either expressly, by implication, inducement, estoppel or
 * otherwise. Any license under such intellectual property rights must be
 * express and approved by NXP in writing.
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/wireless.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/ethernet.h>
#include "../mlanutl/mlanutl.h"
// #include    "mlanwls.h"
#include "wls_api.h"
#include "wls_param_defines.h"
#include "range_kalman.h"
#include "event.h"

#define HostCmd_CMD_DEBUG (0x8B)

/** Command buffer size */
#define MRVDRV_SIZE_OF_CMD_BUFFER (2 * 1024)

/** NXP private command identifier */
#define CMD_NXP "MRVL_CMD"

/** Flag: is initialized */
hal_wls_packet_params_t packetparams;
int setRef = 0;
float referenceBuffer[2 * (MAX_RX * MAX_TX) * MAX_IFFT_SIZE_CSI];

/** CSI record data structure */
typedef struct _csi_record_ds {
	/** Length in DWORDS, including header */
	t_u16 Len;
	/** CSI signature. 0xABCD fixed */
	t_u16 CSI_Sign;
	/** User defined HeaderID  */
	t_u32 CSI_HeaderID;
	/** Packet info field */
	t_u16 PKT_info;
	/** Frame control field for the received packet*/
	t_u16 FCF;
	/** Timestamp when packet received */
	t_u64 TSF;
	/** Received Packet Destination MAC Address */
	t_u8 Dst_MAC[6];
	/** Received Packet Source MAC Address */
	t_u8 Src_MAC[6];
	/** RSSI for antenna A */
	t_u8 Rx_RSSI_A;
	/** RSSI for antenna B */
	t_u8 Rx_RSSI_B;
	/** Noise floor for antenna A */
	t_u8 Rx_NF_A;
	/** Noise floor for antenna A */
	t_u8 Rx_NF_B;
	/** Rx signal strength above noise floor */
	t_u8 Rx_SINR;
	/** Channel */
	t_u8 channel;
	/** user defined Chip ID */
	t_u16 chip_id;
	/** Reserved */
	t_u32 rsvd;
	/** CSI data length in DWORDs */
	t_u32 CSI_Data_Length;
	/** Start of CSI data */
	t_u8 CSI_Data[0];
	/** At the end of CSI raw data, user defined TailID of 4 bytes*/
} __ATTRIB_PACK__ csi_record_ds;

/** Event body : MLAN_CSI */
typedef struct _event_mlan_csi {
	/** Event string: EVENT=MLAN_CSI */
	char event_str[14];
	/** Event sequence # */
	t_u16 sequence;
	csi_record_ds csi_record;
} __ATTRIB_PACK__ event_mlan_csi;

typedef struct _csi_ack_cmd {
	/** HostCmd_DS_GEN */
	HostCmd_DS_GEN cmd_hdr;
	/** Command Body */
	t_u16 action;
	t_u16 sub_id;
	t_u32 ack;
	t_u32 phase_roll;
	t_u32 firstpath_delay;
	t_u32 fft_size_pointer;
	t_u32 csi_tsf;
} __ATTRIB_PACK__ hostcmd_csi_ack;

extern wls_csi_cfg_t gwls_csi_cfg;
static int mlanwls_prepare_buffer(t_u8 *buffer, char *cmd, t_u32 num,
				  char *args[]);

#define RANGE_DRIVE_VAR 4e-5f // in meter/(s^2)
#define RANGE_MEASUREMENT_VAR 4e-2f // in meter^2
#define RANGE_RATE_INIT 1e-3f // in (meter/s)^2

range_kalman_state range_input_str = {0};

/**
 *  @brief Writes the Distance value to a file /tmp/mwu.log to display in
 * GuiTracker app
 *
 *  @param distance distance value obtained from FTM complete event
 *
 *  @return  error
 */
/*
static int mlanwls_update_distance_to_gui(int distance, unsigned int tsf)
{
#ifdef PRINT_DISTANCE_FOR_GUI
       FILE* logfile;
       char buf[50];
       int distance_m, distance_cm;
#endif
       unsigned int delta_time_ms = 0, time_ms = tsf/1000;
       float distance_flt = 1.0f*distance/(1<<8); // in meters
       float distance_kalman;

       if (range_input_str.time == 0)
       {
	       range_kalman_init(&range_input_str, distance_flt, time_ms,
RANGE_DRIVE_VAR, RANGE_MEASUREMENT_VAR, RANGE_RATE_INIT); range_input_str.time =
1;
       }
       else
       {
	       range_input_str.range_measurement = distance_flt;
	       range_input_str.time = time_ms;
	       delta_time_ms = time_ms - (unsigned
int)range_input_str.last_time; range_kalman(&range_input_str);
       }
       distance_kalman =  range_input_str.last_range;
       printf("Measured Distance: %f m; Kalman Distance: %f m [%d ms]\n",
distance_flt, distance_kalman, delta_time_ms);

#ifdef PRINT_DISTANCE_FOR_GUI
       logfile = fopen("/var/www/html/mwu.log", "w");
       if (logfile==NULL)
       {
       printf("Could not open log file\n");
       return 2;
       }
       distance_m = distance>>8;
       distance_cm = ((distance&0xff)*100)>>8;

       sprintf(buf, "%d.%02d\n",distance_m, distance_cm);
       fwrite(buf, 1, strlen(buf), logfile);
       fclose(logfile);
#endif
       return 0;
}
*/
void set_csi_filter(unsigned int *headerBuffer,
		    hal_wls_packet_params_t *packetparams)
{
	hal_csirxinfo_t *csirxinfo = (hal_csirxinfo_t *)headerBuffer;
	hal_pktinfo_t *pktinfo;
	unsigned int tempVec[2] = {0, 0};

	tempVec[0] = (unsigned int)csirxinfo->pktinfo;
	pktinfo = (hal_pktinfo_t *)tempVec;

	// set sig format and BW
	packetparams->ftmSignalBW = pktinfo->sigBw;
	packetparams->ftmPacketType = pktinfo->packetType;
	// set MAC address
	packetparams->peerMacAddress_lo = csirxinfo->addr2_lo;
	packetparams->peerMacAddress_hi = csirxinfo->addr2_hi;

	printf("CSI filter set MAC: %x.%x.%x.%x.%x.%x, sig BW/format %d|%d\n",
	       csirxinfo->addr2_lo & 0xff, (csirxinfo->addr2_lo >> 8) & 0xff,
	       csirxinfo->addr2_hi & 0xff, (csirxinfo->addr2_hi >> 8) & 0xff,
	       (csirxinfo->addr2_hi >> 16) & 0xff,
	       (csirxinfo->addr2_hi >> 24) & 0xff, pktinfo->sigBw,
	       pktinfo->packetType);
}

int check_csi_filter(unsigned int *headerBuffer,
		     hal_wls_packet_params_t *packetparams)
{
	hal_csirxinfo_t *csirxinfo = (hal_csirxinfo_t *)headerBuffer;
	hal_pktinfo_t *pktinfo;
	unsigned int tempVec[2] = {0, 0};

	tempVec[0] = (unsigned int)csirxinfo->pktinfo;
	pktinfo = (hal_pktinfo_t *)tempVec;

	// check sig format and BW
	if ((packetparams->ftmSignalBW != pktinfo->sigBw) ||
	    (packetparams->ftmPacketType != pktinfo->packetType))
		return 0;

	// set MAC address
	if ((packetparams->peerMacAddress_lo != csirxinfo->addr2_lo) ||
	    (packetparams->peerMacAddress_hi != csirxinfo->addr2_hi))
		return 0;

	return 1;
}

void proc_csi_event(event_header *event, unsigned int *resArray)
{
	unsigned char *rdPtr =
		(unsigned char *)&(((event_mlan_csi *)event)->csi_record);
	unsigned int *csiBuffer;
	unsigned int *fftInBuffer, *scratchBuffer1;
	unsigned short csi_len;
	int firstPathDelay;
	float perturbVal_dB = 0.0f;
	unsigned int headerBuffer[HEADER_LEN];
	unsigned int totalpower[MAX_RX * MAX_TX + 1];

	memcpy(headerBuffer, rdPtr, HEADER_LEN * sizeof(unsigned int));

	csi_len = headerBuffer[0] & 0x1fff; // 13 LSBs
	csiBuffer = (unsigned int *)malloc(sizeof(unsigned int) * csi_len);

	memcpy(csiBuffer, rdPtr, sizeof(unsigned int) * csi_len);

	fftInBuffer = (unsigned int *)malloc(sizeof(unsigned int) *
					     (MAX_RX * MAX_TX + NUM_PROC_BUF) *
					     MAX_IFFT_SIZE_CSI);

	scratchBuffer1 = (unsigned int *)malloc(
		sizeof(unsigned int) * (MAX_RX * MAX_TX + NUM_PROC_BUF) *
		MAX_IFFT_SIZE_CSI);

	packetparams.chNum = gwls_csi_cfg.channel;

	wls_unpack_csi(csiBuffer, fftInBuffer, &packetparams,
		       &gwls_csi_cfg.wls_processing_input, totalpower);

	firstPathDelay =
		wls_calculate_toa(headerBuffer, fftInBuffer, scratchBuffer1,
				  totalpower, &packetparams,
				  &gwls_csi_cfg.wls_processing_input);

	if (setRef == 0) { // initialize
		setRef = 1;
		wls_intialize_reference(headerBuffer, fftInBuffer,
					referenceBuffer);

		set_csi_filter(headerBuffer, &packetparams);
	} else if (check_csi_filter(headerBuffer, &packetparams)) {
		perturbVal_dB =
			wls_update_cross_corr_pi_calc(headerBuffer, fftInBuffer,
						      referenceBuffer,
						      scratchBuffer1);

		{
			hal_pktinfo_t *pktinfo =
				(hal_pktinfo_t *)&(headerBuffer[2]);
			char myStr[4] = {'V', 'H', 'T', '\0'};
			int BW = 20 << pktinfo->sigBw;
			// record TSF
			UINT64 TSF = (((UINT64)headerBuffer[4]) << 32) +
				     headerBuffer[3];
			// calculate ToA in ns
			float toa_ns = 1.e3f * firstPathDelay / (1 << 16);
			if (pktinfo->packetType == 0) {
				myStr[0] = 'l';
				myStr[1] = 'e';
				myStr[2] = 'g';
			} else if ((pktinfo->packetType == 1) ||
				   (pktinfo->packetType == 4)) {
				myStr[0] = 'H';
				myStr[1] =
					(pktinfo->packetType == 4) ? 'E' : 'T';
				myStr[2] = '\0';
			}
			printf("CSI Processing results: %s(%d), %0.2f\tTSF %llx, PI %0.1f \n",
			       myStr, BW, toa_ns, TSF, perturbVal_dB);
		}
	}

	free(fftInBuffer);
	free(scratchBuffer1);
}

void proc_csi_event_wls(event_header *event, unsigned int *resArray)
{
	unsigned char *rdPtr =
		(unsigned char *)&(((event_mlan_csi *)event)->csi_record);
	unsigned int headerBuffer;
	unsigned int *csiBuffer;
	unsigned int *fftInBuffer;
	hal_wls_packet_params_t packetparams;
	unsigned short csi_len;

	memcpy(&headerBuffer, rdPtr, sizeof(unsigned int));

	csi_len = headerBuffer & 0x1fff; // 13 LSBs
	csiBuffer = (unsigned int *)malloc(sizeof(unsigned int) * csi_len);

	memcpy(csiBuffer, rdPtr, sizeof(unsigned int) * csi_len);

	fftInBuffer = (unsigned int *)malloc(sizeof(unsigned int) *
					     (MAX_RX * MAX_TX + NUM_PROC_BUF) *
					     MAX_IFFT_SIZE_CSI);

	packetparams.chNum = gwls_csi_cfg.channel;

	resArray[0] = 0xffffffff;
	resArray[1] = 0xffffffff;
	resArray[2] = 0xffffffff;
	resArray[3] = 0xffffffff;

	wls_process_csi(csiBuffer, fftInBuffer, &packetparams,
			&gwls_csi_cfg.wls_processing_input, resArray);

	// record TSF
	resArray[3] = csiBuffer[3];

	{
		hal_pktinfo_t *pktinfo = (hal_pktinfo_t *)&(resArray[2]);
		char myStr[4] = {'V', 'H', 'T', '\0'};
		int BW = 20 << pktinfo->sigBw;
		if (pktinfo->packetType == 0) {
			myStr[0] = 'l';
			myStr[1] = 'e';
			myStr[2] = 'g';
		} else if ((pktinfo->packetType == 1) ||
			   (pktinfo->packetType == 4)) {
			myStr[0] = 'H';
			myStr[1] = (pktinfo->packetType == 4) ? 'E' : 'T';
			myStr[2] = '\0';
		}
		printf("EVENT: MLAN_CSI Processing results: format %s(%d), %d | %d (%x|%x), TSF %x \n",
		       myStr, BW, resArray[0], resArray[1], resArray[2],
		       *((unsigned int *)&gwls_csi_cfg.wls_processing_input),
		       resArray[3]);
	}
	free(fftInBuffer);
}

/**
 *  @brief Prepare command buffer
 *  @param buffer   Command buffer to be filled
 *  @param cmd      Command id
 *  @param num      Number of arguments
 *  @param args     Arguments list
 *  @return         MLAN_STATUS_SUCCESS
 */
static int mlanwls_prepare_buffer(t_u8 *buffer, char *cmd, t_u32 num,
				  char *args[])
{
	t_u8 *pos = NULL;
	unsigned int i = 0;

	memset(buffer, 0, MRVDRV_SIZE_OF_CMD_BUFFER);

	/* Flag it for our use */
	pos = buffer;
	memcpy((char *)pos, CMD_NXP, strlen(CMD_NXP));
	pos += (strlen(CMD_NXP));

	/* Insert command */
	strncpy((char *)pos, (char *)cmd, strlen(cmd));
	pos += (strlen(cmd));

	/* Insert arguments */
	for (i = 0; i < num; i++) {
		strncpy((char *)pos, args[i], strlen(args[i]));
		pos += strlen(args[i]);
		if (i < (num - 1)) {
			memcpy((char *)pos, " ", strlen(" "));
			pos += 1;
		}
	}

	return MLAN_STATUS_SUCCESS;
}

#ifdef PRINT_CSI_TO_FILE
static void print_csi_event(event_header *event, t_u16 size, char *if_name)
{
	struct stat fbuf;
	int fstat;
	FILE *csi_fp = NULL;
	char filename[64];
	/* Mask out Bit15,14,13 for RAW CSI */
	t_u16 csi_length = 0x1fff & ((event_mlan_csi *)event)->csi_record.Len;
	t_u16 seq = ((event_mlan_csi *)event)->sequence;
	static t_u16 last_seq = 0;
	static t_u16 total_lost_count = 0;

	/* Print CSI EVENT data */
	printf("EVENT: MLAN_CSI\n");
	printf("Sequence: %d\n", ((event_mlan_csi *)event)->sequence);
	printf("Length: %d DWORDs\n", csi_length);
	// printf("CSI record:\n");
	// hexdump(NULL, (void *)&(((event_mlan_csi *)event)->csi_record),
	//     4*csi_length, ' ');

	/* Save CSI data to file */

	snprintf(filename, sizeof(filename), "csidump_%s.txt", if_name);
	fstat = stat(filename, &fbuf);

	if ((fstat == 0) && (fbuf.st_size >= CSI_DUMP_FILE_MAX)) {
		printf("File %s reached maximum size. Not saving CSI records.\n",
		       filename);
		printf("ERR: Lost csi event as %s reached maximum size for %d times\n",
		       filename, seq - last_seq);
	} else {
		t_u16 fpos;
		csi_fp = fopen(filename, "a+");

		if (seq == 0 || last_seq == 0 || seq == (last_seq + 1))
			last_seq = seq;
		else {
			total_lost_count += seq - last_seq;
			printf("ERR: Lost csi event for %d times, total_lost_count = %d\n",
			       seq - last_seq, total_lost_count);
			last_seq = seq;
		}

		if (csi_fp) {
			for (fpos = 0; fpos < csi_length; fpos++) {
				fprintf(csi_fp, "%08x ",
					*((t_u32 *)&(((event_mlan_csi *)event)
							     ->csi_record) +
					  fpos));
			}
			fclose(csi_fp);
		}
		chmod(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
					S_IROTH | S_IWOTH);
	}
}
#endif

/**
 *  @brief Send hostcmd IOCTL to driver
 *  @param cmd_buf  pointer to Host Command buffer
 *
 *  @return     MLAN_STATUS_SUCCESS--success, otherwise--fail
 */

static int mlanwls_send_ioctl(t_u8 *cmd_buf)
{
	struct ifreq ifr;
	struct eth_priv_cmd *cmd = NULL;
	int ret = MLAN_STATUS_SUCCESS;

	/*hexdump((void *) cmd_buf, MRVDRV_SIZE_OF_CMD_BUFFER, ' ');*/

	if (!cmd_buf) {
		printf("ERR:IOCTL Failed due to null cmd buffer!\n");
		ret = MLAN_STATUS_FAILURE;
		goto done;
	}

	cmd = (struct eth_priv_cmd *)malloc(sizeof(struct eth_priv_cmd));
	if (!cmd) {
		printf("ERR:Cannot allocate buffer for command!\n");
		ret = MLAN_STATUS_FAILURE;
		goto done;
	}

	/* Fill up buffer */
#ifdef USERSPACE_32BIT_OVER_KERNEL_64BIT
	memset(cmd, 0, sizeof(struct eth_priv_cmd));
	memcpy(&cmd->buf, cmd_buf, sizeof(cmd_buf));
#else
	cmd->buf = cmd_buf;
#endif
	cmd->used_len = 0;
	cmd->total_len = MRVDRV_SIZE_OF_CMD_BUFFER;

	/* Perform IOCTL */
	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
	ifr.ifr_ifru.ifru_data = (void *)cmd;

	if (ioctl(sockfd, MLAN_ETH_PRIV, &ifr)) {
		perror("mlanwls");
		fprintf(stderr, "IOCTL fail\n");
		ret = MLAN_STATUS_FAILURE;
		goto done;
	}

done:
	if (cmd)
		free(cmd);
	return ret;
}

static int mlanwls_csi_ack_resp(char *cmd_name, t_u8 *buf)
{
	t_u32 hostcmd_size = 0;
	HostCmd_DS_GEN *hostcmd = NULL;
	int ret = MLAN_STATUS_SUCCESS;
	hostcmd_csi_ack *phostcmd = NULL;

	buf += strlen(CMD_NXP) + strlen(cmd_name);
	memcpy((t_u8 *)&hostcmd_size, buf, sizeof(t_u32));
	buf += sizeof(t_u32);

	hostcmd = (HostCmd_DS_GEN *)buf;
	hostcmd->command = le16_to_cpu(hostcmd->command);
	hostcmd->size = le16_to_cpu(hostcmd->size);

	hostcmd->command &= ~HostCmd_RET_BIT;

	switch (hostcmd->command) {
	case HostCmd_CMD_DEBUG:
		phostcmd = (hostcmd_csi_ack *)buf;
		if (!le16_to_cpu(phostcmd->cmd_hdr.result)) {
			printf("SUCCESS\n");
		} else {
			printf("FAIL\n");
		}
		break;

	default:
		break;
	}

	return ret;
}

void send_csi_ack(unsigned int *resArray)
{
	t_u8 *buffer = NULL;
	hostcmd_csi_ack *phostcmd = NULL;

	/* Initialize buffer */
	buffer = (t_u8 *)malloc(BUFFER_LENGTH);
	if (!buffer) {
		printf("ERR:Cannot allocate buffer for command!\n");
		return;
	}

	mlanwls_prepare_buffer(buffer, "hostcmd", 0, NULL);
	phostcmd = (hostcmd_csi_ack *)(buffer +
				       (strlen(CMD_NXP) + strlen("hostcmd") +
					sizeof(t_u32)));

	/*Parse the arguments*/
	phostcmd->cmd_hdr.command = cpu_to_le16(HostCmd_CMD_DEBUG);
	phostcmd->cmd_hdr.size = S_DS_GEN + sizeof(hostcmd_csi_ack);
	phostcmd->action = 0;
	phostcmd->sub_id = 0x333;
	phostcmd->ack = 1;

	phostcmd->phase_roll = resArray[0];
	phostcmd->firstpath_delay = resArray[1];
	phostcmd->fft_size_pointer = resArray[2];
	phostcmd->csi_tsf = resArray[3];

	phostcmd->cmd_hdr.size += 6 * sizeof(t_u32);
	phostcmd->cmd_hdr.size = cpu_to_le16(phostcmd->cmd_hdr.size);

	/* send command */
	mlanwls_send_ioctl(buffer);

	/* handle command response */
	mlanwls_csi_ack_resp("hostcmd", buffer);
}
