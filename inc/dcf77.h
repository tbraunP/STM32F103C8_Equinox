#ifndef DCF77_H
#define DCF77_H
#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

void DFC77_init();


// EXTI-Handler PA0 for DCF77 Module
void EXTI0_IRQHandler(void);

// DCF Timer Handler
void TIM2_IRQHandler(void);

#ifdef __cplusplus
 }
#endif

#endif // DCF77_H
