#include "stm32f10x_conf.h"
#include "stm32f10x.h"
#include "stm32f10x_it.h"
#include "system_stm32f10x.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hw/uart.h"
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
    const char str[] = "Welcome to STM32F103 DCF77\n\0";
    uint16_t len  = (uint16_t) strlen(str);
    UART_Send((uint8_t*) str, len);

    static int i = 0;
    static int oldss = -1;

    while(1) {


        // print dcf
        volatile struct DCF77_Time_t* time = &clockTime;

        int ss = time->ss;
        int mm = time->mm;
        int hh = time->hh;

        if(i++ > 50000){
            // only print if changed
            if(oldss == ss)
                continue;

            printTime(hh, mm, ss);
            oldss = ss;

            i=0;
        }
    }
}
