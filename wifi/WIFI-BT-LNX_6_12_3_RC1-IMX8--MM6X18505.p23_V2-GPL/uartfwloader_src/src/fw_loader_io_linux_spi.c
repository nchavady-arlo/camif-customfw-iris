/** @file   fw_loader_io_linux_spi.c
 *
 *  @brief  This file contains the functions that implement the Nxp specific
 *          Helper Protocol for Linux over spi.
 *
 *  Copyright 2022 NXP
 *
 *  NXP CONFIDENTIAL
 *  The source code contained or described herein and all documents related to
 *  the source code (Materials) are owned by NXP, its
 *  suppliers and/or its licensors. Title to the Materials remains with NXP,
 *  its suppliers and/or its licensors. The Materials contain
 *  trade secrets and proprietary and confidential information of NXP, its
 *  suppliers and/or its licensors. The Materials are protected by worldwide copyright
 *  and trade secret laws and treaty provisions. No part of the Materials may be
 *  used, copied, reproduced, modified, published, uploaded, posted,
 *  transmitted, distributed, or disclosed in any way without NXP's prior
 *  express written permission.
 *
 *  No license under any patent, copyright, trade secret or other intellectual
 *  property right is granted to or conferred upon you by disclosure or delivery
 *  of the Materials, either expressly, by implication, inducement, estoppel or
 *  otherwise. Any license under such intellectual property rights must be
 *  express and approved by NXP in writing.
 *
 */

/*===================== Include Files ============================================*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <linux/gpio.h>
#include <linux/ioctl.h>
#include <linux/spi/spidev.h>
#include "fw_loader_io_linux.h"
#include "fw_loader_frames.h"

/*===================== Macros ===================================================*/
#define SPI_CS_DELAY            20	/* µSec */
#define SPI_MAX_FRAME_SIZE      2048	/* Observed so far */
#define SPI_MIN_RX_FRAME_SIZE    5	/* One rx frame is min 10 bytes (!dataReqV3) */
#define SPI_MAX_RX_FRAME_SIZE   10	/* One rx frame is max 10 bytes (dataReqV3) */
#define SPI_MAX_RX_RING_SIZE    (5*SPI_MAX_RX_FRAME_SIZE+1)

#define MIN(a,b)                (a<b)?(a):(b)
#define MAX(a,b)                (a>b)?(a):(b)

#define TIMEOUT_FOR_READ        2000
#define MISO_DETECT_FRAME_SIZE	/* If not defined, takes any data between 0xFF <data1> ... <dataN> 0xFF */

//#define DEBUG_API               /* Debug SPI API interface */
//#define DEBUG_SPI               /* Debug SPI transfer */
//#define DEBUG_INT               /* Debug GpioInt */
//#define DEBUG_RNG               /* Debug ring buffer */
//#define DEBUG_RNG_DATA          /* Debug ring buffer data*/
//#define DUMP_SPI_DATA           /* Dump SPI Data including frame detection */

#ifdef DEBUG_API
#define PRINT_API(...)        printf(__VA_ARGS__)
#else
#define PRINT_API(...)        do {}while(0)
#endif

#ifdef DEBUG_SPI
#define PRINT_SPI(...)        printf(__VA_ARGS__)
#else
#define PRINT_SPI(...)        do {}while(0)
#endif

#ifdef DEBUG_INT
#define PRINT_INT(...)        printf(__VA_ARGS__)
#else
#define PRINT_INT(...)        do {}while(0)
#endif

#ifdef DEBUG_RNG
#define PRINT_RNG(...)        printf(__VA_ARGS__)
#ifdef DEBUG_RNG
#define PRINT_RNG_DATA(...) printf(__VA_ARGS__)
#else
#define PRINT_RNG_DATA(...) do {}while(0)
#endif
#else
#define PRINT_RNG(...)        do {}while(0)
#define PRINT_RNG_DATA(...)   do {}while(0)
#endif

/*==================== Typedefs =================================================*/

typedef struct {
	uint8 data[SPI_MAX_RX_RING_SIZE];
	uint16 inIdx;
	uint16 outIdx;
} RING_BUFFER;

/*==================== Function Prototypes ======================================*/
static int8 fw_upload_Probe_Linux_Spi(int8 * dev);
static int8 fw_upload_ComReadChar_Linux_Spi(int32 iPortID);
static int8 fw_upload_ComWriteChar_Linux_Spi(int32 iPortID, int8 iChar);
static int8 fw_upload_ComWriteChars_Linux_Spi(int32 iPortID, int8 * pChBuffer,
					      int32 uiLen);
static int32 fw_upload_ComReadChars_Linux_Spi(int32 iPortID, int8 * pChBuffer,
					      int32 uiCount);
static int32 fw_upload_init_uart_Linux_Spi(int8 * pPortName,
					   SERIAL_CONFIG * pConfig);
static int32 fw_upload_ComGetSpiInactive_Linux_Spi(int32 iPortID);
static int32 fw_upload_ComGetBufferSize_Linux_Spi(int32 iPortID);
static void fw_upload_Flush_Linux_Spi(int32 iPortID);
static void fw_upload_CloseUart_Linux_Spi(int32 iPortID);

static int fw_upload_gpioInt_Open(char *dev, uint8 line, char *label);
static uint8 fw_upload_gpioInt_IsActive(int fd);
static void fw_upload_gpioInt_Acknowledge(int fd);
static void fw_upload_gpioInt_Close(int fd);

static int fw_upload_DoSpiTransfer(int iPortID, uint16 txLen, uint16 rxLen);

static uint16 ringBuf_getCount(RING_BUFFER * pRingBuf);
static uint16 ringBuf_getRoom(RING_BUFFER * pRingBuf);
static void ringBuf_flush(RING_BUFFER * pRingBuf);
static uint16 ringBuf_writeData(RING_BUFFER * pRingBuf, uint8 * pData,
				uint16 len);
static uint16 ringBuf_readData(RING_BUFFER * pRingBuf, uint8 * pData,
			       uint16 maxLen);

/*===================== Global Vars ==============================================*/
SERIAL_INTERFACE sSerialInterfaceLinuxSpi = {
	.name = "SPI interface",
	.type = SPI_IF,
	.Probe = fw_upload_Probe_Linux_Spi,
	.Open = fw_upload_init_uart_Linux_Spi,
	.Close = fw_upload_CloseUart_Linux_Spi,
	.ComReadChar = fw_upload_ComReadChar_Linux_Spi,
	.ComWriteChar = fw_upload_ComWriteChar_Linux_Spi,
	.ComWriteChars = fw_upload_ComWriteChars_Linux_Spi,
	.ComReadChars = fw_upload_ComReadChars_Linux_Spi,
	.ComGetCTS = fw_upload_ComGetSpiInactive_Linux_Spi,
	.ComGetBufferSize = fw_upload_ComGetBufferSize_Linux_Spi,
	.Flush = fw_upload_Flush_Linux_Spi,
};

static int gpioIntFd = -1;

static uint32 spiSpeedHz;
static uint8 spiBitsPerWord;

static uint8 spiTxBuffer[SPI_MAX_FRAME_SIZE];
static uint8 spiRxBuffer[SPI_MAX_FRAME_SIZE];
static BOOLEAN spiTxIsReady = FALSE;

static RING_BUFFER rxRingBuffer = { 0 };

/*==================== Coded Procedures =========================================*/
/******************************************************************************
 *
 * Name: fw_upload_Probe_Linux_Spi
 *
 * Description:
 *   This function validates this device belongs to this driver.
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   dev - Device name.
 *
 * Return Value:
 *   Returns 1 for success.
 *   Returns 0 for failure.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static int8
fw_upload_Probe_Linux_Spi(int8 * dev)
{
	int fd, ret;
	int8 result = 0;
	uint32 speed;

	fd = open(dev, O_RDWR | O_CLOEXEC);
	if (fd < 0) {
		perror("Can't probe serial port");
		return 0;
	}

	/* Can read the speed, assume it is a spi, othercase we will get the error "Inappropriate ioctl for device" */
	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == 0) {
		result = 1;
	} else {
		PRINT_API("%s is not a SPI (%s)\n", dev, strerror(errno));
	}

	close(fd);

	return result;
}

/******************************************************************************
 *
 * Name: fw_upload_ComReadChar_Linux_Spi
 *
 * Description:
 *   Read a character from the port specified by nPortID.     
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   nPortID : Port ID.
 *
 * Return Value:
 *   Returns the character, if Successful.
 *   Returns -1 if no character available (OR TIMED-OUT)
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static int8
fw_upload_ComReadChar_Linux_Spi(int32 iPortID)
{
	int8 data = 0xFF;

	fw_upload_ComReadChars_Linux_Spi(iPortID, &data, 1);

	return data;
}

/******************************************************************************
 *
 * Name: fw_upload_ComReadChars_Linux_Spi
 *
 * Description:
 *   Read iCount characters from the port specified by nPortID.     
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   iPortID   : Port ID.
 *   pBuffer : Destination buffer for the characters read 
 *   iCount    : Number of Characters to be read.
 *
 * Return Value:
 *   Returns the number of characters read if Successful.
 *   Returns -1 if iCount characters could not be read or if Port ID is invalid.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static int32
fw_upload_ComReadChars_Linux_Spi(int32 iPortID, int8 * pBuffer, int32 uiCount)
{
	int32 iRet;
	fd_set rd;
	struct timeval tv;

	/* If there is enough data on rxRingBuffer, takes them */
	if (ringBuf_getCount(&rxRingBuffer) >= uiCount) {
		iRet = ringBuf_readData(&rxRingBuffer, (uint8 *) pBuffer,
					uiCount);
		return iRet;
	}

	FD_ZERO(&rd);

	if (fw_upload_gpioInt_IsActive(gpioIntFd)) {
		/* Data to recv from slave */
		tv.tv_sec = 0;
		tv.tv_usec = 0;
	} else {
		/* Waits for the next GpioInt */
		FD_SET(gpioIntFd, &rd);
		tv.tv_sec = TIMEOUT_FOR_READ / 1000;
		tv.tv_usec = (TIMEOUT_FOR_READ % 1000) * 1000;
	}

	iRet = select(gpioIntFd + 1, &rd, NULL, NULL, &tv);
	if (iRet >= 0) {
		/* If the event GpioInt occured, ack it */
		if (FD_ISSET(gpioIntFd, &rd)) {
			fw_upload_gpioInt_Acknowledge(gpioIntFd);
		}
		if (fw_upload_gpioInt_IsActive(gpioIntFd)) {
			/* If the caller is looking for 1 byte, it is expecting a frame header, so read a complete frame */
#if defined(SPI_SLAVE_SEND_CTRL_FRAME_10_BYTES)
			fw_upload_DoSpiTransfer(iPortID, 0,
						SPI_MAX_RX_FRAME_SIZE);
#elif defined(SPI_SLAVE_SEND_CTRL_FRAME_2XFER_1_THEN_REMAINING)
			fw_upload_DoSpiTransfer(iPortID, 0, uiCount);
#else
			fw_upload_DoSpiTransfer(iPortID, 0, uiCount);
#endif
		}
	}

	iRet = ringBuf_readData(&rxRingBuffer, (uint8 *) pBuffer, uiCount);
	return iRet;
}

/******************************************************************************
 *
 * Name: fw_upload_ComWriteChar_Linux_Spi
 *
 * Description:
 *   Write a character to the port specified by iPortID.     
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   iPortID : Port ID.
 *   iChar   : Character to be written
 *
 * Return Value:
 *   Returns TRUE, if write is Successful.
 *   Returns FALSE if write is a failure.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static int8
fw_upload_ComWriteChar_Linux_Spi(int32 iPortID, int8 iChar)
{
	return fw_upload_ComWriteChars_Linux_Spi(iPortID, &iChar, 1);
}

/******************************************************************************
 *
 * Name: fw_upload_ComWriteChars_Linux_Spi
 *
 * Description:
 *   Write iLen characters to the port specified by iPortID.     
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   iPortID : Port ID.
 *   pBuffer : Buffer where characters are available to be written to the Port.
 *   iLen    : Number of Characters to write.
 *
 * Return Value:
 *   Returns TRUE, if write is Successful.
 *   Returns FALSE if write is a failure.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static int8
fw_upload_ComWriteChars_Linux_Spi(int32 iPortID, int8 * pBuffer, int32 uiLen)
{
	int iRet;

	/* Api does not allow to return BUSY, so just waits... */
	while (spiTxIsReady)
		fw_upload_DelayInMs_Linux(1);

	spiTxIsReady = TRUE;
	memcpy(spiTxBuffer, pBuffer, uiLen);

	iRet = fw_upload_DoSpiTransfer(iPortID, uiLen, 0);
	if (iRet < 0)
		return (int8) RW_FAILURE;

	return (int8) RW_SUCCESSFUL;
}

/******************************************************************************
 *
 * Name: fw_upload_ComGetSpiInactive_Linux_Spi
 *
 * Description:
 *   Check CTS status
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   iPortID   : Port ID.
 *
 * Return Value:
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static int32
fw_upload_ComGetSpiInactive_Linux_Spi(int32 iPortID)
{
	return fw_upload_gpioInt_IsActive(gpioIntFd);
}

/******************************************************************************
 *
 * Name: fw_upload_ComGetBufferSize_Linux_Spi
 *
 * Description:
 *   Check buffer size
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   iPortID   : Port ID.
 *
 * Return Value:
 *   size in buffer
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static int32
fw_upload_ComGetBufferSize_Linux_Spi(int32 iPortID)
{
	/* If an spiRx has been requested by the chip, try to read a FRAME */
	if (fw_upload_gpioInt_IsActive(gpioIntFd))
		fw_upload_DoSpiTransfer(iPortID, 0, SPI_MAX_RX_FRAME_SIZE);

	return ringBuf_getCount(&rxRingBuffer);
}

/******************************************************************************
 *
 * Name: fw_upload_Flush_Linux_Spi
 *
 * Description:
 *   flush buffer
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   iPortID   : Port ID.
 *
 * Return Value:
 *   None.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static void
fw_upload_Flush_Linux_Spi(int32 iPortID)
{
	int8 dummy[20];		/* Protocol V3: max 10 bytes */

	ringBuf_flush(&rxRingBuffer);
	while (fw_upload_gpioInt_IsActive(gpioIntFd))
		fw_upload_ComReadChars_Linux_Spi(iPortID, dummy, sizeof(dummy));
}

// adapted from hciattach.c (bluez-util-3.7)
/******************************************************************************
 *
 * Name: fw_upload_init_uart_Linux_Spi
 *
 * Description:
 *   Initialize UART.
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   iPortID   : Port ID.
 *   pConfig   : Configuration of the port.
 *
 * Return Value:
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static int32
fw_upload_init_uart_Linux_Spi(int8 * dev, SERIAL_CONFIG * pConfig)
{
	int iRet;
	int fd;
	uint8 ucMode;		/* transfer mode.  Use the constants SPI_MODE_0..SPI_MODE_3;
				   or if you prefer you can combine SPI_CPOL (clock polarity, idle high iff this is set)
				   or SPI_CPHA (clock phase, sample on trailing edge iff this is set) flags */

	fd = open(dev, O_RDWR | O_CLOEXEC);

	if (fd < 0) {
		perror("Can't open serial port");
		return OPEN_FAILURE;
	}

	/* Configure SPI interface: Mode */
	switch (pConfig->spi.ucMode) {
	case 0:
		ucMode = SPI_MODE_0;
		break;
	case 1:
		ucMode = SPI_MODE_1;
		break;
	case 2:
		ucMode = SPI_MODE_2;
		break;
	case 3:
		ucMode = SPI_MODE_3;
		break;
	default:
		printf("Invalid mode %d\n", pConfig->spi.ucMode);
		close(fd);
		return OPEN_FAILURE;
		break;
	}
	iRet = ioctl(fd, SPI_IOC_WR_MODE, &ucMode);
	if (iRet == -1) {
		perror("Can't configure spi mode");
		close(fd);
		return OPEN_FAILURE;
	}

	/* Configure SPI interface: Speed */
	iRet = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &pConfig->uiSpeed);
	if (iRet == -1) {
		perror("Can't configure spi speed");
		close(fd);
		return OPEN_FAILURE;
	}

	/* Configure SPI interface: Bits per Word */
	iRet = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &pConfig->ucByteSize);
	if (iRet == -1) {
		perror("Can't configure spi bits per word");
		close(fd);
		return OPEN_FAILURE;
	}

#if 0
	/* Lock file descriptor */
	iRet = flock(fd, LOCK_EX | LOCK_NB);
	if (iRet == -1) {
		perror("Can't lock file descriptor");
		close(fd);
		return OPEN_FAILURE;
	}
#endif

	PRINT_API("SpiOpen(%s) Speed %d Hz, Mode %d, wordBits %d, fd=%d\n", dev,
		  pConfig->uiSpeed, pConfig->spi.ucMode, pConfig->ucByteSize,
		  fd);

	/* Open & configure Gpio "Interrupt" */
	gpioIntFd =
		fw_upload_gpioInt_Open(pConfig->spi.pGpioIntDev,
				       pConfig->spi.ucGpioIntLine,
				       "NXP_FW_Loader_SPI_INT");
	if (gpioIntFd < 0) {
		close(fd);
		return OPEN_FAILURE;
	}

	spiSpeedHz = pConfig->uiSpeed;
	spiBitsPerWord = pConfig->ucByteSize;

	return fd;
}

/******************************************************************************
 *
 * Name: fw_upload_CloseUart_Linux_Spi
 *
 * Description:
 *   Close Uart.
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   iPortID   : Port ID.
 *
 * Return Value:
 *   None.
 *
 * Notes:
 *   None.
 *
 
*****************************************************************************/
static void
fw_upload_CloseUart_Linux_Spi(int32 iPortID)
{
	fw_upload_gpioInt_Close(gpioIntFd);
	gpioIntFd = -1;

	if (iPortID > 1000000) {
		exit(-1);
	}

	close(iPortID);
}

/******************************************************************************
 *
 * Name: fw_upload_DoSpiTransfer
 *
 * Description:
 *   This function performs a SPI transfer.
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   iPortID   : Port ID.
 *   txLen     : Length of the data to transmit.
 *   rxLen     : Length of the data to receive.
 *
 * Return Value:
 *   Result    : result of the transaction.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static int
fw_upload_DoSpiTransfer(int iPortID, uint16 txLen, uint16 rxLen)
{
	int iRet;
	int spiTransferBytes = 0;
	struct spi_ioc_transfer spiTransfer[2];
#ifdef MISO_DETECT_FRAME_SIZE
	static int misoRemaining = 0;
	int begin, len;
#else
	int begin, end;
#endif

	PRINT_SPI("\n%s(%d, tx: %d, rx: %d) gpioInt %d\n", __FUNCTION__,
		  iPortID, txLen, rxLen, fw_upload_gpioInt_IsActive(gpioIntFd));

	int waitCount = 0;

	if (rxLen == 0) {
		/* Wait for the remote side to release the interrupt line */
		do {
			if (!waitCount) {
#ifdef DUMP_SPI_DATA
				printf("Spi Wait gpioInt to be inactive\n");
#endif
			}
			waitCount += 5;
#warning TODO: adjust tempo after releasing interrupt line
			fw_upload_DelayInMs_Linux(5);
		} while (fw_upload_gpioInt_IsActive(gpioIntFd));
#ifdef DUMP_SPI_DATA
		printf("Spi Waited %d ms for gpioInt to be inactive\n",
		       waitCount);
#endif
	}

	/* Prepare transfer buffer */
	memset(spiTransfer, 0, sizeof(spiTransfer));
	spiTransferBytes = MAX(txLen, rxLen);

	/* Clear Rx Buffer */
	memset(spiRxBuffer, 0xFF, SPI_MAX_FRAME_SIZE);
	/* Clear Tx Buffer (unused data) */
	memset(spiTxBuffer + txLen, 0xFF, SPI_MAX_FRAME_SIZE - txLen);

	/* Delay between CS & start clock */
	spiTransfer[0].tx_buf = 0;
	spiTransfer[0].rx_buf = 0;
	spiTransfer[0].len = 0;
	spiTransfer[0].speed_hz = spiSpeedHz;
	spiTransfer[0].delay_usecs = SPI_CS_DELAY;
	spiTransfer[0].bits_per_word = spiBitsPerWord;
	spiTransfer[0].cs_change = FALSE;
	/* Real transfer */
	spiTransfer[1].tx_buf = (__u64) spiTxBuffer;
	spiTransfer[1].rx_buf = (__u64) spiRxBuffer;
	spiTransfer[1].len = spiTransferBytes;
	spiTransfer[1].speed_hz = spiSpeedHz;
	spiTransfer[1].delay_usecs = 0;
	spiTransfer[1].bits_per_word = spiBitsPerWord;
	spiTransfer[1].cs_change = FALSE;

#ifdef DUMP_SPI_DATA
	if (txLen)
		log_buffer(spiTxBuffer, txLen, "SPI Tx %d", txLen);
#endif

	iRet = ioctl(iPortID, SPI_IOC_MESSAGE(2), spiTransfer);
	if (iRet < 0) {
		perror("Can't transfer Spi");
		return iRet;
	}

#ifdef DUMP_SPI_DATA
	log_buffer(spiRxBuffer, spiTransferBytes, "SPI Rx %d",
		   spiTransferBytes);
#endif

#ifdef MISO_DETECT_FRAME_SIZE

	/* First store remaining data of the last transfer */
	if (misoRemaining > 0) {
		if (misoRemaining <= spiTransferBytes) {
#ifdef DUMP_SPI_DATA
			printf("SPI Rx real data index [%d-%d] (full remaining data of last transfer)\n", 0, misoRemaining - 1);
#endif
			ringBuf_writeData(&rxRingBuffer, &spiRxBuffer[0],
					  misoRemaining);
			begin = misoRemaining;
			misoRemaining = 0;
		} else {
#ifdef DUMP_SPI_DATA
			printf("SPI Rx real data index [%d-%d] (partial remaining data of last transfer)\n", begin, spiTransferBytes - 1);
#endif
			ringBuf_writeData(&rxRingBuffer, &spiRxBuffer[0],
					  spiTransferBytes);
			begin = spiTransferBytes;
			misoRemaining -= spiTransferBytes;
		}
	} else {
		begin = 0;
	}

	/* Search a frame header and store frame len */
	while (begin < spiTransferBytes) {
		len = (spiRxBuffer[begin] ==
		       0xFF) ? (0)
  : (fw_upload_getFrameLen(spiRxBuffer[begin]));
		if (len > 0) {
			if (begin + len <= spiTransferBytes) {
#ifdef DUMP_SPI_DATA
				printf("SPI Rx real data index [%d-%d]\n",
				       begin, begin + len - 1);
#endif
				ringBuf_writeData(&rxRingBuffer,
						  &spiRxBuffer[begin], len);
				begin += len;
				/* frame is complete, Wait for the interrupt to be released */
				while (fw_upload_gpioInt_IsActive(gpioIntFd))
					fw_upload_DelayInMs_Linux(5);
			} else {
#ifdef DUMP_SPI_DATA
				printf("SPI Rx real data index [%d-%d] (incomplete)\n", begin, spiTransferBytes - 1);
#endif
				ringBuf_writeData(&rxRingBuffer,
						  &spiRxBuffer[begin],
						  spiTransferBytes - begin);
				begin = spiTransferBytes;
				misoRemaining =
					len - (spiTransferBytes - begin);
			}
		} else {
			begin++;
		}
	}

#else

	for (begin = 0; begin < spiTransferBytes; begin++) {
		if (spiRxBuffer[begin] != 0xFF)
			break;
	}
	for (end = spiTransferBytes; end > 0; end--) {
		if (spiRxBuffer[end - 1] != 0xFF)
			break;
	}
#ifdef DUMP_SPI_DATA
	printf("SPI Rx real data index [%d-%d]\n", begin, end);
#endif

	if (end - begin > 0) {
		ringBuf_writeData(&rxRingBuffer, &spiRxBuffer[begin],
				  end - begin);
	}

#endif

	spiTxIsReady = FALSE;

	return iRet;
}

/******************************************************************************
 *
 * Name: fw_upload_gpioInt_Open
 *
 * Description:
 *   This function opens the gpio used as interrupt (chip signaling data available).
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   dev - Device name.
 *   line - Line number
 *   label - our label
 *
 * Return Value:
 *   Returns file descriptor.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static int
fw_upload_gpioInt_Open(char *dev, uint8 line, char *label)
{
	int iRet;
	int fd;
	struct gpioevent_request gpioReq;

	/* Open & configure Gpio "Interrupt" */
	fd = open(dev, O_RDWR);
	if (fd < 0) {
		perror("Can't open GpioIntDev");
		return -1;
	}

	gpioReq.lineoffset = line;
	gpioReq.handleflags = GPIOHANDLE_REQUEST_INPUT;
	gpioReq.eventflags = GPIOEVENT_REQUEST_FALLING_EDGE;
	snprintf(gpioReq.consumer_label, sizeof(gpioReq.consumer_label), "%s",
		 label);
	iRet = ioctl(fd, GPIO_GET_LINEEVENT_IOCTL, &gpioReq);
	if (iRet == -1) {
		perror("Can't configure GpioIntDev");
		close(fd);
		return -1;
	}

	close(fd);
	PRINT_API("GpioIntOpen(%s, %d, %s): fd=%d->%d\n", dev, line, label, fd,
		  gpioReq.fd);

	return gpioReq.fd;
}

/******************************************************************************
 *
 * Name: fw_upload_gpioInt_IsActive
 *
 * Description:
 *   This function checks if the "interrupt" is active.
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   fd - gpio int file descriptor
 *
 * Return Value:
 *   Returns 0: Gpio Interrupt is inactive.
 *   Returns 1: Gpio Interrupt is active.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static uint8
fw_upload_gpioInt_IsActive(int fd)
{
	int iRet;
	struct gpiohandle_data data = { 0 };
	static int lastValue = -1;

	errno = 0;
	iRet = ioctl(fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data);
	if (iRet == -1) {
		printf("Can't get GpioIntDev(%d) value: %s\n", fd,
		       strerror(errno));
		return 0;
	}

	if (lastValue != data.values[0]) {
		PRINT_INT("GpioIntDev(%d) value: %d -> %d\n", fd, lastValue,
			  data.values[0]);
		lastValue = data.values[0];
	}

	return (data.values[0] == 0);	/* Active low */
}

/******************************************************************************
 *
 * Name: fw_upload_gpioInt_Acknowledge
 *
 * Description:
 *   This function acknowledges an raised "interrupt".
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   fd - gpio int file descriptor
 *
 * Return Value:
 *   None.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static void
fw_upload_gpioInt_Acknowledge(int fd)
{
	int iRet;
	struct gpioevent_data event;

	/* Read event data to clear interrupt */
	iRet = read(fd, &event, sizeof(event));
	if (iRet == -1) {
		perror("Can't read GpioIntDev to ack");
	}
}

/******************************************************************************
 *
 * Name: fw_upload_gpioInt_Close
 *
 * Description:
 *   This function closes the gpio used as interrupt (chip signaling data available).
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   fd - gpio int file descriptor
 *
 * Return Value:
 *   None.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static void
fw_upload_gpioInt_Close(int fd)
{
	if (fd != -1)
		close(fd);
}

/******************************************************************************
 *
 * Name: ringBuf_getCount
 *
 * Description:
 *   This function returns the number of data used in the ring buffer.
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   pRingBuffer - ring buffer structure
 *
 * Return Value:
 *   Returns the number of bytes present in the ring Buffer.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static uint16
ringBuf_getCount(RING_BUFFER * pRingBuf)
{
	uint16 count;

	if (pRingBuf->inIdx >= pRingBuf->outIdx)
		count = pRingBuf->inIdx - pRingBuf->outIdx;
	else
		count = (SPI_MAX_RX_RING_SIZE + pRingBuf->inIdx -
			 pRingBuf->outIdx);

	PRINT_RNG("%s(%p) size %d, in %d, out %d => %d\n", __FUNCTION__,
		  pRingBuf, SPI_MAX_RX_RING_SIZE, pRingBuf->inIdx,
		  pRingBuf->outIdx, count);
	return count;
}

/******************************************************************************
 *
 * Name: ringBuf_getRoom
 *
 * Description:
 *   This function returns the number of data unused in the ring buffer.
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   pRingBuffer - ring buffer structure
 *
 * Return Value:
 *   Returns the number of unused bytes in the ring Buffer.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static uint16
ringBuf_getRoom(RING_BUFFER * pRingBuf)
{
	uint16 room;

	if (pRingBuf->outIdx > pRingBuf->inIdx)
		room = pRingBuf->outIdx - pRingBuf->inIdx;
	else
		room = (SPI_MAX_RX_RING_SIZE + pRingBuf->outIdx -
			pRingBuf->inIdx);

	PRINT_RNG("%s(%p) size %d, in %d, out %d => %d\n", __FUNCTION__,
		  pRingBuf, SPI_MAX_RX_RING_SIZE, pRingBuf->inIdx,
		  pRingBuf->outIdx, room);
	return room;
}

/******************************************************************************
 *
 * Name: ringBuf_flush
 *
 * Description:
 *   This function removed used data in the ring buffer.
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   pRingBuffer - ring buffer structure
 *
 * Return Value:
 *   None.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static void
ringBuf_flush(RING_BUFFER * pRingBuf)
{
	pRingBuf->inIdx = 0;
	pRingBuf->outIdx = 0;
}

/******************************************************************************
 *
 * Name: ringBuf_writeData
 *
 * Description:
 *   This function add used data at the end of the ring buffer.
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   pRingBuffer - ring buffer structure
 *   pData       - data to add
 *   len         - number of data to add
 *
 * Return Value:
 *   Returns the number of data really added.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static uint16
ringBuf_writeData(RING_BUFFER * pRingBuf, uint8 * pData, uint16 len)
{
	int i, room;

	room = ringBuf_getRoom(pRingBuf);
	if (room < len) {
		printf("\nSPI OVERFLOW: receive %d bytes but can only store %d (ringBuf: used %d, free %d))\n", len, room, ringBuf_getCount(pRingBuf), ringBuf_getRoom(pRingBuf));
	}

	len = MIN(len, room);

	for (i = 0; i < len; i++) {
		PRINT_RNG_DATA("ringBuf Write 0x%02x at index %d\n", pData[i],
			       pRingBuf->inIdx);
		pRingBuf->data[pRingBuf->inIdx++] = pData[i];
		if (pRingBuf->inIdx == SPI_MAX_RX_RING_SIZE)
			pRingBuf->inIdx = 0;
	}

	return len;
}

/******************************************************************************
 *
 * Name: ringBuf_readData
 *
 * Description:
 *   This function get data at the beginning of the ring buffer.
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   pRingBuffer - ring buffer structure
 *   pData       - data to get
 *   len         - number of data to get
 *
 * Return Value:
 *   Returns the number of data really got.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static uint16
ringBuf_readData(RING_BUFFER * pRingBuf, uint8 * pData, uint16 maxLen)
{
	int i;

	maxLen = MIN(maxLen, ringBuf_getCount(pRingBuf));

	for (i = 0; i < maxLen; i++) {
		PRINT_RNG_DATA("ringBuf Read  0x%02x at index %d\n",
			       pRingBuf->data[pRingBuf->outIdx],
			       pRingBuf->outIdx);
		pData[i] = pRingBuf->data[pRingBuf->outIdx++];
		if (pRingBuf->outIdx == SPI_MAX_RX_RING_SIZE)
			pRingBuf->outIdx = 0;
	}
	return maxLen;
}
