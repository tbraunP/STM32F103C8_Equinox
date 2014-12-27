#include "stm32f10x_conf.h"
#include "stm32f10x.h"
#include "stm32f10x_it.h"
#include "system_stm32f10x.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hw/uart.h"
#include "ws2812/ws2812.h"
#include "ledClock/animator.h"
#include "util/itoa.h"

#include "dcf77/dcf77.h"
#include "ledClock/clock.h"
#include "systick.h"


void printTime(int hh, int mm, int ss){
    // print time the ugly way
    static char str[100];
    itoa(hh, str);
    UART_SendString(str);
    UART_SendString(":\0");

    itoa(mm, str);
    UART_SendString(str);
    UART_SendString(":\0");

    itoa(ss, str);
    UART_SendString(str);
    UART_SendString("\n\0");
}


int main(void)
{
    // run uart for debug output
    UART_init();

    // Enable systick
    SysTick_init();

    // start WS2812 and disable display
    WS2812_Init();
    WS2812_clear();

    // DCF77 activate
    DFC77_init();

    // send welcome message
    UART_SendString("Welcome to STM32F103 Equinox\n\0");

    static int i = 0;
    static int oldss1 = -1;
    static int oldss2 = -1;

    while(1) {
        if(i++ > 50000){
            // only print if changed
            if(oldss1 != clockTime.ss){
                UART_SendString("EQ: ");
                printTime(clockTime.hh, clockTime.mm, clockTime.ss);
                oldss1 = clockTime.ss;
            }

            if(oldss2 != dcf.ss){
                UART_SendString("DCF77: ");
                printTime(dcf.hh, dcf.mm, dcf.ss);
                oldss2 = dcf.ss;
            }

            i=0;
        }
    }
}
