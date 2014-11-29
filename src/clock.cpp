#include "clock.h"

#include "stm32f10x_conf.h"
#include "stm32f10x.h"

#include "hw/uart.h"
#include "dcf77.h"

#define     UPDATE_RATE         (25)
#define     PRESCALER           (uint16_t) (0x2C1E)
#define     COUNTERVALUE40MS    (uint16_t) (0xFF)

// local state variables
static volatile int32_t totalDuration = 0;
static volatile uint8_t loops = 0;

/**
 * @brief localTime - time of local clock
 */
static volatile struct DCF77_Time_t localTime;


// forward declarations
static void Clock_IncrementSecond();
static void updateVisualization(volatile struct DCF77_Time_t* localTime, uint8_t loops, uint8_t MAXRATE);

/**
 * @brief Clock_Init
 * Initialize local timer for clock visualization
 */
void Clock_Init(){
    // Timer configuration
    TIM_TimeBaseInitTypeDef timerConfig;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    NVIC_InitTypeDef   NVIC_InitStructure;


    // now we need a timer to decide if we received a bit or not
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    TIM_DeInit(TIM4);
    TIM_TimeBaseStructInit(&timerConfig);

    /* Time base configuration */
    timerConfig.TIM_Period = 0xFFFF;
    timerConfig.TIM_Prescaler = PRESCALER-1;
    timerConfig.TIM_ClockDivision = 0;
    timerConfig.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(TIM4, &timerConfig);
    TIM_ITConfig(TIM4, TIM_IT_CC1, ENABLE);
    TIM_ClearITPendingBit(TIM4, TIM_IT_CC1);

    /* Compare interrupt */
    TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Active;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;
    //TIM_OCInitStructure.TIM_Pulse = 0;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    //TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
    TIM_OC1Init(TIM4, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Disable);

    // set compare value
    TIM4->CCR1 = COUNTERVALUE40MS;

    // total time
    totalDuration = COUNTERVALUE40MS * UPDATE_RATE;


    // Enable Interrupt
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    // reset interrupt
    NVIC_ClearPendingIRQ(TIM4_IRQn);
    NVIC_Init(&NVIC_InitStructure);

    // now start the timer
    TIM_Cmd(TIM4, ENABLE);
}



/**
 * Compare Interrupt
 */
void TIM4_IRQHandler(void){
    // reset
    TIM4->CNT = 0;

    // clear IRQ Status
    TIM_ClearITPendingBit(TIM4, TIM_IT_CC1);
    NVIC_ClearPendingIRQ(TIM2_IRQn);

    // now increment local loop counter and modify local
    // time on second increment
    ++loops;
    if(loops == UPDATE_RATE){
        Clock_IncrementSecond();
        loops = 0;
    }

    // update visualization of clock
    updateVisualization(&localTime, loops, UPDATE_RATE);
}

/**
 * @brief Clock_IncrementSecond
 * Increment local time
 */
static void Clock_IncrementSecond(){
    localTime.ss++;//Addiere +1 zu Sekunden

    if (localTime.ss == 60)
    {
        localTime.ss = 0;
        localTime.mm++;//Addiere +1 zu Minuten
        if (localTime.mm == 60) {
            localTime.mm = 0;
            localTime.hh++;//Addiere +1 zu Stunden
            if (localTime.hh == 24) {
                localTime.hh = 0;
            }
        }
    }
}

// tobe removed
static void updateVisualization(volatile struct DCF77_Time_t* localTime, uint8_t loops, uint8_t MAXRATE){

}
