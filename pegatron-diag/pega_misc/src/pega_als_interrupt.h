#ifndef _PEGA_ALS_INT_H_
#define _PEGA_ALS_INT_H_
//==============================================================================
#include "pega_defines.h"
//==============================================================================
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================

//==============================================================================
void Pega_ALS_interrupt_handler_Start(void);
void Pega_ALS_interrupt_handler_Stop(void);
//==============================================================================
int  Pega_ALS_interrupt_trigger_set(int sGpioNum, unsigned int eInterrupt) ;
//==============================================================================
#ifdef __cplusplus
}
#endif
#endif //_PEGA_ALS_INT_H_
