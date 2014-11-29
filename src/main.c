#include "stm32f10x_conf.h"
#include "stm32f10x.h"
#include "stm32f10x_it.h"
#include "system_stm32f10x.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hw/uart.h"
#include "util/itoa.h"

#include "dcf77.h"
#include "clock.h"
#include "systick.h"




int main(void)
{
    // Enable systick
    SysTick_init();

    // run uart
    UART_init();

    //Clock_Init();

    DFC77_init();

    // send welcome message
    const char str[] = "Welcome to STM32F103 DCF77\n\0";
    uint16_t len  = (uint16_t) strlen(str);
    UART_Send((uint8_t*) str, len);


    int i=0;

    while(1) {
//        volatile int* tmp = malloc(100*sizeof(int));
//        for(int i=0;i<100;i++){
//            if(tmp[i]>=0)
//                tmp[i]=i;
//            else
//                tmp[i]=i+1;
//        }
//        free((void*) tmp);

        // print dcf
        char str[100];

        int ss = dcf.ss;
        int mm = dcf.mm;
        int hh = dcf.hh;

        if(i++ > 1000000){

            // print time the ugly way
            itoa(hh, str);
            UART_Send((const uint8_t*) str, strlen(str));
            UART_Send((const uint8_t*) ":", 1);

            itoa(mm, str);
            UART_Send((const uint8_t*) str, strlen(str));
            UART_Send((const uint8_t*) ":", 1);

            itoa(ss, str);
            UART_Send((const uint8_t*) str, strlen(str));
            UART_Send((const uint8_t*) "\n", 1);

            i=0;
        }
    }
}
