#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
//==============================================================================
#include "pega_debug_msgq_cmd.h"
//==============================================================================

//==============================================================================
static void Pega_misc_dbg_help_print(void)
{	   	   
	   printf("\n Usage :");	   
	   printf("\n Pega_Gpio help info [options]");
	   printf("\n options:");
	   
	   printf("\n");		 
}
//==============================================================================
int main(int argc, char* argv[])
{	
    stDebugCmdType stDebugMsg;
		
    printf("[%s] argc=%d \n", __func__, argc);  
	
	if (argc > 1)
    {
    	printf("[%s]%s,%s \n", __func__, argv[0],argv[1]);    			 	    	 	
    } 
    else
    {
      	Pega_misc_dbg_help_print();
     	goto Cmd_End;
    }  
        
    if (!strcmp(argv[1],"info"))
  	  {
		stDebugMsg.eCmdId = Debug_CmdId_Debug_Print;
		
  		if (argv[2] != NULL)
		  {
		    stDebugMsg.value1 = strtol(argv[2],NULL,10); //16 
		  }		
        		
        Pega_debug_msgq_send_to_misc(stDebugMsg);
  	  }
	else if (!strcmp(argv[1],"debug"))
  	  {
		stDebugMsg.eCmdId = Debug_CmdId_Debug_Msg_Control;
		
  		if (argv[2] != NULL)
		  {
		    stDebugMsg.value1 = strtol(argv[2],NULL,10); //16 
		  }		
        		
        Pega_debug_msgq_send_to_misc(stDebugMsg);
  	  }
    else if (!strcmp(argv[1],"set"))
  	  {
		stDebugMsg.eCmdId = Debug_CmdId_Debug_Set;		
  		
        if (argv[2] != NULL)
		  {
		    stDebugMsg.value1 = strtol(argv[2],NULL,10); //16 
		  }		
		
		if (argv[3] != NULL)
		  {
		    stDebugMsg.value2 = strtol(argv[3],NULL,10); //16 
		  }		
		
        if (argv[4] != NULL)
		  {
		    stDebugMsg.value3 = strtol(argv[4],NULL,10); //16 
		  }	
		 
        if (argv[5] != NULL)
		  {
		    stDebugMsg.value4 = strtol(argv[5],NULL,10); //16 
		  }			 
		  
        Pega_debug_msgq_send_to_misc(stDebugMsg);
  	  }
	else if (!strcmp(argv[1],"get"))
  	  {
		stDebugMsg.eCmdId = Debug_CmdId_Debug_Get;
		
  		if (argv[2] != NULL)
		  {
		    stDebugMsg.value1 = strtol(argv[2],NULL,10); //16 
		  }		
        		
        Pega_debug_msgq_send_to_misc(stDebugMsg);
  	  }
	else if (!strcmp(argv[1],"motor"))
  	  {
		stDebugMsg.eCmdId = Debug_CmdId_Debug_Motor;		
  		
        if (argv[2] != NULL)
		  {
		    stDebugMsg.value1 = strtol(argv[2],NULL,10); //16 
		  }		
		
		if (argv[3] != NULL)
		  {
		    stDebugMsg.value2 = strtol(argv[3],NULL,10); //16 
		  }		
		
        if (argv[4] != NULL)
		  {
		    stDebugMsg.value3 = strtol(argv[4],NULL,10); //16 
		  }	
		 
        if (argv[5] != NULL)
		  {
		    stDebugMsg.value4 = strtol(argv[5],NULL,10); //16 
		  }			 
		  
        Pega_debug_msgq_send_to_misc(stDebugMsg);
  	  }  
	else if (!strcmp(argv[1],"sch"))
  	  {
		stDebugMsg.eCmdId = Debug_CmdId_Schedule_Event;
		
  		if (argv[2] != NULL)
		  {
		    stDebugMsg.value1 = strtol(argv[2],NULL,10); //16 
		  }		
        		
        Pega_debug_msgq_send_to_misc(stDebugMsg);
  	  }      
  	else	
  	  {
  		  Pega_misc_dbg_help_print();	 	
  	  }
  	      
Cmd_End:
    	  
    //printf("\n[%s]exit....\n", __func__);
    
    return 0;
}