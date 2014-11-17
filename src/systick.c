#include <stdint.h>
#include "systick.h"


volatile uint32_t milliseconds;

// SysTick alle 10 ms
void SysTick_init(){
    // NVIC will be enabled by SysTick_Config
    SysTick_Config(SystemCoreClock / 100);
}


void SysTick_Handler(void){
    milliseconds+=10;
}
