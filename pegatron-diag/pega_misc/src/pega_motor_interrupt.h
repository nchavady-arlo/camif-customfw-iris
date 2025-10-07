#ifndef _PEGA_MOTOR_INT_H_
#define _PEGA_MOTOR_INT_H_
//==============================================================================
#include "pega_defines.h"
//==============================================================================
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================

//==============================================================================
void Pega_Motor_interrupt_handler_Start(void);
void Pega_Motor_interrupt_handler_Stop(void);
void Pega_Motor_interrupt_handler_Exit(void);
//==============================================================================
int Pega_Motor_interrupt_trigger_set(int sGpioNum, unsigned int eInterrupt);
//==============================================================================
#ifdef __cplusplus
}
#endif
#endif //_PEGA_MOTOR_INT_H_
