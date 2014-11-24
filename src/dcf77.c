#include <stdio.h>
#include <string.h>

#include "dcf77.h"
#include "dcf_internal.h"

#include "stm32f10x_conf.h"
#include "stm32f10x.h"
#include "hw/uart.h"



static char str[200];

volatile struct DCF77_Time_t dcf;

// Timer configuration
static TIM_TimeBaseInitTypeDef timerConfig;

//Bitzähler für RX Bit
volatile uint8_t rx_bit_counter = 0;

//64 Bit für DCF77 benötigt werden 59 Bits
volatile uint64_t dcf_rx_buffer = 0;

// DCF77 Flags
volatile static struct DCF_Flags_t flags;

//Hilfs Sekunden Counter
static volatile uint8_t h_ss = 0;

//Hilfs Variable für Stundenwechsel
static volatile uint8_t h_hh = 0;

static void EXTI0_Config(){
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
    uint16_t prescaler = (uint16_t) (SystemCoreClock / 10000) - 1;
    /* Time base configuration */
    timerConfig.TIM_Period = (11500);
    timerConfig.TIM_Prescaler = prescaler;
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
    EXTI0_Config();
}


// some helper functions
static void Add_one_Second();
static char* itoa(int i, char b[]);



// Edge recognized
void EXTI0_IRQHandler(void){

    static int counter = 0;
    static uint32_t lastEdge =0;
    ++counter;

    //rising edge
    if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)){
        lastEdge = TIM2->CNT;
        //UART_Send((const uint8_t*)"R_DCF\n\0", 6);
    }else{
        uint32_t duration = ((uint32_t) TIM2->CNT) - lastEdge;

        // reset timer and restart
        TIM_Cmd(TIM2, DISABLE);
        TIM_DeInit(TIM2);

        TIM_TimeBaseInit(TIM2, &timerConfig);
        TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        NVIC_ClearPendingIRQ(TIM2_IRQn);

        TIM_Cmd(TIM2, ENABLE);

        // no decide if 1 or 0 bit has been received
        //uint32_t duration = ((uint32_t) TIM2->CNT) - lastEdge;
        //sprintf((char*) str, "F_DCF Duration %d\n", (int)duration);
        //UART_Send(str, strlen((char*)str));

        itoa(duration, str);
        UART_Send((const uint8_t*)str, strlen(str));

        if(duration <= 1500){
            UART_Send((const uint8_t*)"R0\n\0", 3);
        }else{
            UART_Send((const uint8_t*)"R1\n\0", 3);
        }

    }

    EXTI_ClearITPendingBit(EXTI_Line0);
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
            h_hh = 1;
        }

        //Berechnung der Stunden BCD to HEX
        dcf.hh = rx_buffer->Hour-((rx_buffer->Hour/16)*6);

        if (h_hh) {dcf.hh--;h_hh = 0;};

        //Berechnung des Tages BCD to HEX
        dcf.day= rx_buffer->Day-((rx_buffer->Day/16)*6);
        //Berechnung des Monats BCD to HEX
        dcf.mon= rx_buffer->Month-((rx_buffer->Month/16)*6);
        //Berechnung des Jahres BCD to HEX
        dcf.year= 2000 + rx_buffer->Year-((rx_buffer->Year/16)*6);
        //Sekunden werden auf 0 zurückgesetzt
        dcf.ss = 59;
        flags.dcf_sync = 1;        //SyncRTC_Clock();
    } else {
        //nicht alle 59Bits empfangen bzw kein DCF77 Signal Uhr läuft
        //manuell weiter
        UART_Send((const uint8_t*)"Sync fehlgeschlagen...\n\0",23);
        Add_one_Second();
        flags.dcf_sync = 0;
    }
    //zurücksetzen des RX Bit Counters
    rx_bit_counter = 0;

    //Löschen des Rx Buffers
    dcf_rx_buffer = 0;

    // DEBUG
    UART_Send((const uint8_t*)"TIMER Second 59\n\0", 16);
}




static void Add_one_Second (){
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


static char* itoa(int i, char b[]){
    char const digit[] = "0123456789";
    char* p = b;
    if(i < 0){
        *p++ = '-';
        i *= -1;
    }
    int shifter = i;
    do { //Move to where representation ends
        ++p;
        shifter = shifter/10;
    } while(shifter);
    *p = '\0';
    do { //Move back, inserting digits as u go
        *--p = digit[i%10];
        i = i/10;
    } while(i);
    return b;
}
