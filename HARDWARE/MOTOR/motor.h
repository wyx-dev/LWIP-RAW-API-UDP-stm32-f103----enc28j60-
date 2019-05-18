#ifndef __MOTOR_H
#define __MOTOR_H
#include "sys.h"
#include <stm32f10x.h>


#define PWM_LEFT  		TIM1->CCR4
#define PWM_RIGHT 		TIM1->CCR1
#define AIN1  			PAout(4) 
#define AIN2  			PAout(5)
#define BIN1  			PAout(6)
#define BIN2  			PAout(7)


void MOTOR_PWM_Init(u16 arr,u16 psc);
void MOTOR_IO_Init(void);

#endif
