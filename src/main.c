#include "stm32f10x_conf.h"
#include "stm32f10x.h"
#include "stm32f10x_it.h"
#include "system_stm32f10x.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hw/uart.h"

#include "dcf77.h"
#include "systick.h"


int main(void)
{
    // Enable systick
    SysTick_init();

    // run uart
    UART_init();

    DFC77_init();

    // send welcome message
    const char str[] = "Welcome to STM32F103 DCF77\n\0";
    uint16_t len  = (uint16_t) strlen(str);
    UART_Send((uint8_t*) str, len);

    while(1)
    {

    }


}
