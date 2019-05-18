#ifndef __ELENGINE_H
#define __ELENGINE_H	 
#include "sys.h"
void TIM2_PWM_Init(u16 arr,u16 psc);
void Open_Elengine_1(int dir);
void Open_Elengine_2(void);
void Close_Elengine_1(void);
void Close_Elengine_2(void);
void dianjiqudong(void);
#endif
