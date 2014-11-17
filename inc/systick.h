#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>
#include "stm32f10x.h"

#ifdef __cplusplus
 extern "C" {
#endif


extern volatile uint32_t milliseconds;

void SysTick_init();

void SysTick_Handler(void);


#ifdef __cplusplus
 }
#endif

#endif // SYSTICK_H
