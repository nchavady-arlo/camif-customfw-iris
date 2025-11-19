/*******************************************************************************
* File Name: pega_voice_cmd.c
*
*******************************************************************************/
#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/un.h>
#include <signal.h>
#include <stddef.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/msg.h>
//==============================================================================
#include "pega_voice_cmd.h"
//==============================================================================
void Pega_matter_device_contrl(MatterDevEnumType eDev, int bIsPowerOn) 
{
	  switch(eDev)
	 {
		  //==============================================================================
	      case MATTER_Dev_Light:
		  {
		       if (bIsPowerOn > 0)
			   {
				   printf("Pega: Turn on kitchen lights.\n");
			   }
			   else
			   {
				   printf("Pega: Turn off kitchen lights.\n");
			   }
               break;          
		  }
		  //==============================================================================
	      case MATTER_Dev_Plug:
		  {
		       if (bIsPowerOn > 0)
			   {
				   printf("Pega: Turn on Plug.\n");
			   }
			   else
			   {
				   printf("Pega: Turn off Plug.\n");
			   }
               break;          
		  }
		  //==============================================================================
	      case MATTER_Dev_All:
		  {
		       if (bIsPowerOn > 0)
			   {
				   printf("Pega: Turn on all.\n");
			   }
			   else
			   {
				   printf("Pega: Turn off all.\n");
			   }
               break;          
		  }
		  //==============================================================================
          default:
		       printf("Pega:Device type error.(%d).\n", eDev);
               break;		 
	 }	
}

void Pega_matter_light_contrl(LightColorEnumType eState) 
{
	 switch(eState)
	 {
		 case LIGHT_Color_Off:
		      printf("Pega:Turn off light.\n");
              break;
		 case LIGHT_Color_Red:
		      printf("Pega:Set light to Red.\n");
              break;
         case LIGHT_Color_Orange:
		      printf("Pega:Set light to Orange.\n");
              break;
         case LIGHT_Color_Yellow:
		      printf("Pega:Set light to Yellow.\n");
              break;
         case LIGHT_Color_Green:
		      printf("Pega:Set light to Green.\n");
              break;
         case LIGHT_Color_Blue:
		      printf("Pega:Set light to Blue.\n");
              break;
 		 case LIGHT_Color_Purple:
		      printf("Pega:Set light to Purple.\n");
              break;
         case LIGHT_Color_Pink:
		      printf("Pega:Set light to Pink.\n");
              break;
         case LIGHT_Color_White:
		      printf("Pega:Set light to White.\n");
              break;
			  
		 default:
              printf("Pega:Light control error.(%d).\n", eState);
              break;		 
	 }
}

void Pega_matter_light_brightness_contrl(LightBrightnessEnumType eState, int u8Brightness) 
{
	 switch(eState)
	 {
		 case LIGHT_Brightness_Up:
		      printf("Pega:Light brightness up.\n");
              break;
		 case LIGHT_Brightness_Down:
		      printf("Pega:Light brightness down.\n");
              break;
         case LIGHT_Brightness_Set:
		      printf("Pega:Light brightness set %d.\n", u8Brightness);
              break;
         			  
		 default:
              printf("Pega:Light brightness error.(%d).\n", eState);
              break;		 
	 }
}
//==============================================================================