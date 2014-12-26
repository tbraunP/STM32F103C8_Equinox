#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

void Animator_Init();



void TIM4_IRQHandler(void);



#ifdef __cplusplus
}
#endif

#endif // ANIMATOR_H
