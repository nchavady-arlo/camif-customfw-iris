/*
  c_iflag flag constants:

       IGNBRK Ignore BREAK condition on input.

       BRKINT If IGNBRK is set, a BREAK is ignored.  If it is not set but
              BRKINT is set, then a BREAK causes the input and output queues
              to be flushed, and if the terminal is the controlling terminal
              of a foreground process group, it will cause a SIGINT to be
              sent to this foreground process group.  When neither IGNBRK
              nor BRKINT are set, a BREAK reads as a null byte ('\0'),
              except when PARMRK is set, in which case it reads as the
              sequence \377 \0 \0.

       IGNPAR Ignore framing errors and parity errors.

       PARMRK If this bit is set, input bytes with parity or framing errors
              are marked when passed to the program.  This bit is meaningful
              only when INPCK is set and IGNPAR is not set.  The way
              erroneous bytes are marked is with two preceding bytes, \377
              and \0.  Thus, the program actually reads three bytes for one
              erroneous byte received from the terminal.  If a valid byte
              has the value \377, and ISTRIP (see below) is not set, the
              program might confuse it with the prefix that marks a parity
              error.  Therefore, a valid byte \377 is passed to the program
              as two bytes, \377 \377, in this case.

              If neither IGNPAR nor PARMRK is set, read a character with a
              parity error or framing error as \0.

       INPCK  Enable input parity checking.

       ISTRIP Strip off eighth bit.

       INLCR  Translate NL to CR on input.

       IGNCR  Ignore carriage return on input.

       ICRNL  Translate carriage return to newline on input (unless IGNCR is
              set).

       IUCLC  (not in POSIX) Map uppercase characters to lowercase on input.

       IXON   Enable XON/XOFF flow control on output.

       IXANY  (XSI) Typing any character will restart stopped output.  (The
              default is to allow just the START character to restart
              output.)

       IXOFF  Enable XON/XOFF flow control on input.

       IMAXBEL
              (not in POSIX) Ring bell when input queue is full.  Linux does
              not implement this bit, and acts as if it is always set.

       IUTF8 (since Linux 2.6.4)
              (not in POSIX) Input is UTF8; this allows character-erase to
              be correctly performed in cooked mode.

       c_oflag flag constants:

       OPOST  Enable implementation-defined output processing.

       OLCUC  (not in POSIX) Map lowercase characters to uppercase on
              output.

       ONLCR  (XSI) Map NL to CR-NL on output.

       OCRNL  Map CR to NL on output.

       ONOCR  Don't output CR at column 0.

       ONLRET Don't output CR.

       OFILL  Send fill characters for a delay, rather than using a timed
              delay.

       OFDEL  Fill character is ASCII DEL (0177).  If unset, fill character
              is ASCII NUL ('\0').  (Not implemented on Linux.)

       NLDLY  Newline delay mask.  Values are NL0 and NL1.  [requires
              _BSD_SOURCE or _SVID_SOURCE or _XOPEN_SOURCE]

       CRDLY  Carriage return delay mask.  Values are CR0, CR1, CR2, or CR3.
              [requires _BSD_SOURCE or _SVID_SOURCE or _XOPEN_SOURCE]

       TABDLY Horizontal tab delay mask.  Values are TAB0, TAB1, TAB2, TAB3
              (or XTABS).  A value of TAB3, that is, XTABS, expands tabs to
              spaces (with tab stops every eight columns).  [requires
              _BSD_SOURCE or _SVID_SOURCE or _XOPEN_SOURCE]

       BSDLY  Backspace delay mask.  Values are BS0 or BS1.  (Has never been
              implemented.)  [requires _BSD_SOURCE or _SVID_SOURCE or
              _XOPEN_SOURCE]

       VTDLY  Vertical tab delay mask.  Values are VT0 or VT1.

       FFDLY  Form feed delay mask.  Values are FF0 or FF1.  [requires
              _BSD_SOURCE or _SVID_SOURCE or _XOPEN_SOURCE]

       c_cflag flag constants:

       CBAUD  (not in POSIX) Baud speed mask (4+1 bits).  [requires
              _BSD_SOURCE or _SVID_SOURCE]

       CBAUDEX
              (not in POSIX) Extra baud speed mask (1 bit), included in
              CBAUD.  [requires _BSD_SOURCE or _SVID_SOURCE]

              (POSIX says that the baud speed is stored in the termios
              structure without specifying where precisely, and provides
              cfgetispeed() and cfsetispeed() for getting at it.  Some
              systems use bits selected by CBAUD in c_cflag, other systems
              use separate fields, for example, sg_ispeed and sg_ospeed.)

       CSIZE  Character size mask.  Values are CS5, CS6, CS7, or CS8.

       CSTOPB Set two stop bits, rather than one.

       CREAD  Enable receiver.

       PARENB Enable parity generation on output and parity checking for
              input.

       PARODD If set, then parity for input and output is odd; otherwise
              even parity is used.

       HUPCL  Lower modem control lines after last process closes the device
              (hang up).

       CLOCAL Ignore modem control lines.

       LOBLK  (not in POSIX) Block output from a noncurrent shell layer.
              For use by shl (shell layers).  (Not implemented on Linux.)

       CIBAUD (not in POSIX) Mask for input speeds.  The values for the
              CIBAUD bits are the same as the values for the CBAUD bits,
              shifted left IBSHIFT bits.  [requires _BSD_SOURCE or
              _SVID_SOURCE] (Not implemented on Linux.)

       CMSPAR (not in POSIX) Use "stick" (mark/space) parity (supported on
              certain serial devices): if PARODD is set, the parity bit is
              always 1; if PARODD is not set, then the parity bit is always
              0.  [requires _BSD_SOURCE or _SVID_SOURCE]

       CRTSCTS
              (not in POSIX) Enable RTS/CTS (hardware) flow control.
              [requires _BSD_SOURCE or _SVID_SOURCE]

*/

// Pi3 disable console command must do
//sudo systemctl stop serial-getty@ttyS0.service
//sudo systemctl disable serial-getty@ttyS0.service

#include "stdint.h"
#include "stdbool.h"

#include "stdio.h"
#include "unistd.h"			//Used for UART
#include "fcntl.h"			//Used for UART
#include "termios.h"		//Used for UART
#include "sys/signal.h"
#include <string.h>


#include "pega_uart.h"


//------------------------------------------------------------------------------------------
static int m_intUartFilesSream;// GPIO Linux device handle
static char m_u8Buffer[UART_BUFFER_MAX_LEN];
static int	m_intLength = 0;
static int m_intCount = 0;

static char m_u8ReceiveDataBuffer[UART_BUFFER_MAX_LEN] = {0};
static int	m_intReceiveDataLength = 0;
static bool m_bDataReceive = false;

//------------------------------------------------------------------------------------------
void signal_handler_IO (int status);



//------------------------------------------------------------------------------------------
static bool Uart_CheckReceiveDone(void)
{
	char *pReceiveStr = m_u8ReceiveDataBuffer;
	char *pFoundStr = strstr(m_u8ReceiveDataBuffer, RECEIVE_DONE_STRING);
    
	if (pFoundStr)
		{
		if ((pFoundStr-pReceiveStr)< m_intReceiveDataLength)
			return true;
		}
	return false;
}

//------------------------------------------------------------------------------------------
bool Uart_DeviceOpen(char *u8DeviceName)
{
	// O_NOCTTY：告訴Linux這個程式不想控制TTY介面，如果不設定這個旗標，有些輸入(例如鍵盤的abort)信號可能影響程式。
	// O_NDELAY：告訴Linux這個程式不介意RS-232的DCD信號的狀態。如果不設定這個旗標，程式會處於speep狀態，直到RS-232有DCD信號進來。
	// O_RDWR   : enable read write 
//	if ((m_intUartFilesSream = open(u8DeviceName, O_RDWR | O_NOCTTY | O_NDELAY) ) < 0) 
//	if ((m_intUartFilesSream = open(u8DeviceName, O_RDWR) ) < 0) 
//	if ((m_intUartFilesSream = open(u8DeviceName, O_RDWR | O_NOCTTY|O_NONBLOCK) ) < 0) 
	if ((m_intUartFilesSream = open(u8DeviceName, O_RDWR | O_NOCTTY|O_NONBLOCK| O_NDELAY) ) < 0) 
	{
		perror("Uart_DeviceOpen");
		return false;
	}

	return true;
}

//------------------------------------------------------------------------------------------
void Uart_DeviceClose(void)
{
	 close(m_intUartFilesSream);
}
//------------------------------------------------------------------------------------------
//	NFC reader default baud rate 57600
void Uart_InitialParameter(unsigned int u32BaudRate, int bRTSCTS_En)
{	
	struct termios options;
	struct sigaction saio;   
    
	//----------------------------------------
	//install the serial handler before making the device asynchronous
	saio.sa_handler = signal_handler_IO;
	sigemptyset(&saio.sa_mask);   //saio.sa_mask = 0;
	saio.sa_flags = 0;
	saio.sa_restorer = NULL;
	sigaction(SIGIO,&saio,NULL);

	//----------------------------------------
	// allow the process to receive SIGIO
	fcntl(m_intUartFilesSream, F_SETOWN, getpid());
	// Make the file descriptor asynchronous (the manual page says only O_APPEND and O_NONBLOCK, will work with F_SETFL...)
	fcntl(m_intUartFilesSream, F_SETFL, FASYNC);
	//fcntl(m_intUartFilesSream, F_SETFL, FNDELAY);

	//tcgetattr(m_intUartFilesSream, &options);
	
	//----------------------------------------
	//B115200	: baud rate
	//CS8     	: 8n1 (8bit,no parity,1 stopbit)
	//CLOCAL  	: local connection, no modem control
	//CREAD   	: enable receiving characters
	switch(u32BaudRate)
	{
		case 9600:
		     options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
			 break;
		
		case 19200:
		     options.c_cflag = B19200 | CS8 | CLOCAL | CREAD;
			 break;
		
		case 38400:
		     options.c_cflag = B38400 | CS8 | CLOCAL | CREAD;
			 break;
		
        case 57600:
		     options.c_cflag = B57600 | CS8 | CLOCAL | CREAD;
			 break;
		
        case 230400:
		     options.c_cflag = B230400 | CS8 | CLOCAL | CREAD;
			 break;
		
        case 460800:
		     options.c_cflag = B460800 | CS8 | CLOCAL | CREAD;
			 break;
		
        case 921600:
		     options.c_cflag = B921600 | CS8 | CLOCAL | CREAD;
			 break;
		
        case 1000000:
		     options.c_cflag = B1000000 | CS8 | CLOCAL | CREAD;
			 break;
		
        case 2000000:
		     options.c_cflag = B2000000 | CS8 | CLOCAL | CREAD;
			 break;
			 
        case 3000000:
		     options.c_cflag = B3000000 | CS8 | CLOCAL | CREAD;
			 break;
          			 
		default:
		     options.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
			 break;
	}
	
    if (bRTSCTS_En > 0)
	{		
		options.c_cflag |= CRTSCTS;	//B115200 / B57600	
	}
    //options.c_cflag = B115200 | CS8 | CLOCAL | CREAD | CRTSCTS;	//B921600	
	//IGNPAR  : ignore bytes with parity errors
	//ICRNL   : map CR to NL (otherwise a CR input on the other computer will not terminate input)	
	options.c_iflag = IGNPAR;
	//options.c_iflag = IGNPAR | ICRNL;
	options.c_oflag = 0;
	options.c_lflag = 0;

	// One input byte is enough to return from read()
	// Inter-character timer off
	options.c_cc[VMIN]=0;
	options.c_cc[VTIME]=1;

	tcflush(m_intUartFilesSream, TCIFLUSH);
	tcsetattr(m_intUartFilesSream, TCSANOW, &options);
	
	printf("[%s]u32BaudRate:%d bRTSCTS_En:%d\n", __func__, u32BaudRate, bRTSCTS_En); 
}



//------------------------------------------------------------------------------------------
//brief  Send a byte array over UART
//param  pData  : buffer to send
//param  length : number of bytes to send
//retval None
//------------------------------------------------------------------------------------------
int UART_SendBuffer(const char *pCommand, unsigned int length) 
{
#if 1//for debug
	printf("UART_SendBuffer , length =(%d)\r\n",length);
	for (unsigned int i=0; i<length; i++)
		printf("0x%02x ",pCommand[i]);

	printf("\r\n");
#endif

	if (m_intUartFilesSream != -1)
		{
		int count = write(m_intUartFilesSream, pCommand, length);		//Filestream, bytes to write, number of bytes to write
		if (count < 0)
			{
			printf("UART TX error\n");
			return 1;
			}
		else
			{
			#if 0//for debug
			printf("count %d \n",count);
			#endif
			return 0;
			}
			
		}
	return 0;
}

//------------------------------------------------------------------------------------------
//brief  Send a byte array over UART
//param  pResponse : pointer on the buffer response
//param  length 	 : number of bytes to read
//retval None
//------------------------------------------------------------------------------------------
int UART_ReceiveBuffer(char *pResponse, unsigned int length) 
{
	int rx_length;

	if (m_intUartFilesSream != -1)
		{
		rx_length = read(m_intUartFilesSream, pResponse, length);		//Filestream, buffer to store in, number of bytes to read (max)
	
		return rx_length;
		}
	return -2;
}



//------------------------------------------------------------------------------------------
//brief  Send one byte over UART
//param  data 	: data to send
//retval None
//------------------------------------------------------------------------------------------
void UART_SendByte(char data) 
{	
	UART_SendBuffer(&data, 1);
}

//------------------------------------------------------------------------------------------
//brief  Receive one byte over UART
//param  None
//retval Return received data
//------------------------------------------------------------------------------------------
uint8_t UART_ReceiveByte( void ) 
{	
  uint8_t data = 0;
  
  //HAL_UART_Receive(&huart, &data, 1, UARTx_TIMEOUT_MAX);
  
  return data;
}

//------------------------------------------------------------------------------------------
//brief  Receive one byte over UART
//param  None
//retval Return received data
//------------------------------------------------------------------------------------------
void UART_ReceiveHandler(void) 
{	
	if (m_bDataReceive == true)
		{
		m_bDataReceive = false;
		m_intLength = UART_ReceiveBuffer(m_u8Buffer, UART_BUFFER_MAX_LEN);
		if (m_intLength < 0)
			{
			//An error occured (will occur if there are no bytes)
			return;
			}
		else if (m_intLength == 0)
			{
			//No data waiting
			return;
			}
		else
			{
			int i;

			for (i=0; i<m_intLength ; i++)
				{
				#if 0//for debug
				//printf(" 0x%02x \r\n",m_u8Buffer[i]);
				printf(" 0x%02x ",m_u8Buffer[i]);
				#endif
			
				m_u8ReceiveDataBuffer[m_intReceiveDataLength+i] = m_u8Buffer[i];
				}
				#if 0//for debug
				printf("\r\n");
				#endif
			m_intReceiveDataLength = m_intReceiveDataLength + m_intLength;
			}
		}
}



//------------------------------------------------------------------------------------------
int UART_ReceiveDataLengthGet( void ) 
{
	return m_intReceiveDataLength;
}


//------------------------------------------------------------------------------------------
void UART_ReceiveDataLengthSet( int intNewLength ) 
{
	m_intReceiveDataLength = intNewLength;
}

//------------------------------------------------------------------------------------------
void UART_ReceiveDataBytesGet(char *pResponse, unsigned int u8Length)
{
	unsigned int i;

	for (i=0; i<u8Length; i++)
		{
		pResponse[i]=m_u8ReceiveDataBuffer[i];
		}
}



//------------------------------------------------------------------------------------------
void signal_handler_IO (int status)
{
	status = status;
	m_bDataReceive = true;
	m_intCount++;
	//printf("\r\n m_intCount=(%d)",m_intCount);
}


//------------------------------------------------------------------------------------------
bool Uart_TestDone(void)
{
	if (m_intCount>10)
		return true;

	return false;
	
}


//------------------------------------------------------------------------------------------
int Uart_InterruptCount(void)
{
	return m_intCount;
}

//------------------------------------------------------------------------------------------
void Uart_WaitAndSaveDataToBuffer(int intTimeout_ms)
{
	for (int i=0; i<intTimeout_ms/10; i++)
	{
		UART_ReceiveHandler();
		
		if (Uart_CheckReceiveDone())
		{
			goto Fun_end;//return;
		}
		
		usleep(10*1000); 	
	}

Fun_end:
	
	if (m_intLength > 0)
		{
			printf(" Rcv:%s \n ", m_u8ReceiveDataBuffer);
		}
}

