#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "dcf77.h"
#include "dcf_internal.h"
#include "clock.h"

#include "stm32f10x_conf.h"
#include "stm32f10x.h"
#include "hw/uart.h"
#include "util/itoa.h"

/*
 * Thanks to Ulrich Radig for sharing his code on http://www.mikrocontroller.net/attachment/110549/clock.c
 * which has been adopted to the Cortex M3.
 */

//static char str[200];

// external visible struct
volatile struct DCF77_Time_t dcf;

// Timer configuration
static TIM_TimeBaseInitTypeDef timerConfig;

//Bitzähler für RX Bit
static volatile uint8_t rx_bit_counter = 0;

//64 Bit für DCF77 benötigt werden 59 Bits
static volatile uint64_t dcf_rx_buffer = 0;

// DCF77 Flags
static volatile struct DCF_Flags_t flags;

static volatile bool h_hh = false;


// some helper functions, forward declarations
static void DFC77_AddSecond();
static void DFC77_SyncRTC_Clock();


static void DFC77_EXTI0_Config(){
    // config structures
    EXTI_InitTypeDef   EXTI_InitStructure;
    GPIO_InitTypeDef   GPIO_InitStructure;
    NVIC_InitTypeDef   NVIC_InitStructure;


    /* Alternative functions */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    /* Enable GPIOA clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    /* Configure PA0 pin as input floating */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // set gpio
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);

    /* Configure EXTI0 line */
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Enable and set EXTI0 Interrupt to the lowest priority */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0E;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0E;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    // reset interrupt
    EXTI_ClearITPendingBit(EXTI_Line0);
    NVIC_ClearPendingIRQ(EXTI0_IRQn);
    NVIC_Init(&NVIC_InitStructure);

    // now we need a timer to decide if we received a bit or not
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_DeInit(TIM2);
    TIM_TimeBaseStructInit(&timerConfig);

    /* Compute the prescaler value for 10 KHz -> 10000 * 10 KHz = 1 s */
    uint16_t prescaler = (uint16_t) (SystemCoreClock / 10000);
    /* Time base configuration */
    timerConfig.TIM_Period = (11500);
    timerConfig.TIM_Prescaler = prescaler-1;
    timerConfig.TIM_ClockDivision = 0;
    timerConfig.TIM_CounterMode = TIM_CounterMode_Up;


    TIM_TimeBaseInit(TIM2, &timerConfig);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

    // Enable Interrupt
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    // reset interrupt
    NVIC_ClearPendingIRQ(TIM2_IRQn);
    NVIC_Init(&NVIC_InitStructure);

    // now start the timer
    TIM_Cmd(TIM2, ENABLE);
}


void DFC77_init(){
    DFC77_EXTI0_Config();
}


// Edge recognized
void EXTI0_IRQHandler(void){

    static uint32_t lastRisingEdge =0;
    static uint32_t timerValue = 0;

    // perform early readout
    timerValue = (uint32_t) TIM2->CNT;

    //rising edge
    if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)){
        // prevent reception errors (300ms before next rising edge)
        if((timerValue - lastRisingEdge > 300)){
            lastRisingEdge = timerValue;
            flags.dcf_rx = true;
            //UART_Send((const uint8_t*)"R_DCF\n\0", 6);

            // short delay but who cares
            DFC77_AddSecond();
        }
    } else {
        // falling edge

        // store duration
        uint32_t duration = timerValue - lastRisingEdge;
        // sanity check to prevent disturbances
        if(duration > 500 && duration < 3000){
            // reset second overflow timer and restart
            {
                TIM_Cmd(TIM2, DISABLE);
                TIM_DeInit(TIM2);

                TIM_TimeBaseInit(TIM2, &timerConfig);
                TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
                TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
                NVIC_ClearPendingIRQ(TIM2_IRQn);

                TIM_Cmd(TIM2, ENABLE);
            }
            // reset storage for last rising edge
            lastRisingEdge = 0;

            //itoa(duration, str);
            //UART_Send((const uint8_t*)str, strlen(str));

            //Parity speichern
            //beginn von Bereich P1/P2/P3
            if (rx_bit_counter ==  21 || rx_bit_counter ==  29 || rx_bit_counter ==  36) {
                flags.parity_err = 0;
            }
            //Speichern von P1
            if (rx_bit_counter ==  28) {
                flags.parity_P1 = flags.parity_err;
            }

            //Speichern von P2
            if (rx_bit_counter ==  35) {
                flags.parity_P2 = flags.parity_err;
            }

            //Speichern von P3
            if (rx_bit_counter ==  58) {
                flags.parity_P3 = flags.parity_err;
            }

            // Decode bits
            //0 = 100ms -> 1000
            //1 = 200ms -> 2000

            if(duration <= 1500){
                //UART_Send((const uint8_t*)"R0\n\0", 3);
            } else {
                //UART_Send((const uint8_t*)"R1\n\0", 3);
                //Schreiben einer 1 im dcf_rx_buffer an der Bitstelle rx_bit_counter
                dcf_rx_buffer = dcf_rx_buffer | ((uint64_t) 1 << rx_bit_counter);
                //Toggel Hilfs Parity
                flags.parity_err = flags.parity_err ^ 1;
            }
            // next bit
            ++rx_bit_counter;
        }
    }
    // clear interrupt
    EXTI_ClearITPendingBit(EXTI_Line0);
    NVIC_ClearPendingIRQ(EXTI0_IRQn);
}

/*
 * Overflow Interrupt wird ausgelöst bei 59Sekunde oder fehlenden DCF77 Signal
 */
void TIM2_IRQHandler(void){
    // clear IRQ Status
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    NVIC_ClearPendingIRQ(TIM2_IRQn);

    // Perform Update
    // DCF bit buffer
    struct DCF77_Bits_t* rx_buffer = (struct DCF77_Bits_t*)&dcf_rx_buffer;

    //wurden alle 59 Bits empfangen und sind die Paritys richtig?
    if (rx_bit_counter == 59 &&
            flags.parity_P1 == rx_buffer->P1 &&
            flags.parity_P2 == rx_buffer->P2 &&
            flags.parity_P3 == rx_buffer->P3){
        //Alle 59Bits empfangen stellen der Uhr nach DCF77 Buffer
        //Berechnung der Minuten BCD to HEX
        dcf.mm = rx_buffer->Min-((rx_buffer->Min/16)*6);

        if (dcf.mm != 0){
            dcf.mm--;
        } else {
            dcf.mm = 59;
            h_hh = true;
        }

        //Berechnung der Stunden BCD to HEX
        dcf.hh = rx_buffer->Hour-((rx_buffer->Hour/16)*6);

        // hour correction
        if (h_hh) {
            dcf.hh--;
            h_hh = false;
        }

        //Berechnung des Tages BCD to HEX
        dcf.day= rx_buffer->Day-((rx_buffer->Day/16)*6);
        //Berechnung des Monats BCD to HEX
        dcf.mon= rx_buffer->Month-((rx_buffer->Month/16)*6);
        //Berechnung des Jahres BCD to HEX
        dcf.year= 2000 + rx_buffer->Year-((rx_buffer->Year/16)*6);
        //Sekunden werden auf 0 zurückgesetzt
        dcf.ss = 59;

        flags.dcf_sync = true;
    } else {
        //nicht alle 59Bits empfangen bzw kein DCF77 Signal Uhr läuft
        //manuell weiter
        UART_SendString("Sync fehlgeschlagen...\n");
        DFC77_AddSecond();

        flags.dcf_sync = false;
        flags.dcf_rx = false;
    }
    // clock update, flags.dcf_sync indiacted sync
    DFC77_SyncRTC_Clock();

    //zurücksetzen des RX Bit Counters
    rx_bit_counter = 0;

    //Löschen des Rx Buffers
    dcf_rx_buffer = 0;

    // DEBUG
    //UART_Send((const uint8_t*)"TIMER Second 59\n\0", 16);
}


/**
 * @brief DFC77_SyncRTC_Clock
 * Perform a clock synchronisation
 */
static void DFC77_SyncRTC_Clock(){
    static bool clockStarted = false;
    static uint8_t consecSync = 0;

    if(flags.dcf_sync){
        ++consecSync;
        if(consecSync > 5){
            consecSync = 5;
            flags.dcf_sync_strong = true;
        }
    }else{
        consecSync = 0;
        flags.dcf_sync_strong = true;
    }

    // start clock
    if(flags.dcf_sync_strong && flags.dcf_sync){
        if(clockStarted){
            Clock_Sync(&dcf);
        }else{
            Clock_Init();
            UART_SendString("Starting Clock\n");
            clockStarted = true;
        }
    }
}


static void DFC77_AddSecond (){
    dcf.ss++;//Addiere +1 zu Sekunden

    if (dcf.ss == 60)
    {
        dcf.ss = 0;
        dcf.mm++;//Addiere +1 zu Minuten
        if (dcf.mm == 60) {
            dcf.mm = 0;
            dcf.hh++;//Addiere +1 zu Stunden
            if (dcf.hh == 24) {
                dcf.hh = 0;
            }
        }
    }
}


/**
 * @brief clone a time from src to dest
 * @param dest
 * @param src
 */
void DFC77_cloneDCF(volatile struct DCF77_Time_t*dest, volatile struct DCF77_Time_t* src){
    dest->day = src->day;
    dest->mon = src->mon;
    dest->year = src->year;
    dest->hh = src->hh;
    dest->mm = src->mm;
    dest->ss = src->ss;
}
