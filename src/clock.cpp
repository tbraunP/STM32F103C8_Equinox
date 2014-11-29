#include "clock.h"

#include "stm32f10x_conf.h"
#include "stm32f10x.h"

#include "hw/uart.h"
#include "dcf77.h"

// CONSTANTS
#define     UPDATE_RATE         (25)
#define     PRESCALER           (uint16_t) (0x2C1E)
#define     COUNTERVALUE40MS    (uint16_t) (0xFF)

// length of current second cycle in timer click
static volatile int32_t totalDuration = 0;

// new duration of whole cycle after correction in timer clicks
static volatile int32_t newTotalDuration = 0;
static volatile int32_t immediateCorrection = 0;

// position in current cycle
static volatile int32_t currentCycleDuration = 0;

// macrotick counter
static volatile uint8_t loops = 0;

// stores round overflow occuring by switching to the next macrotick,
// using the terms of flexray
static volatile int32_t lastOverflow = 0;



/**
 * @brief localTime - time of local clock
 */
static volatile struct DCF77_Time_t localTime;


// forward declarations
static void Clock_cloneDCF(volatile struct DCF77_Time_t*dest, volatile struct DCF77_Time_t* src);
static void Clock_DecrementSecond(volatile struct DCF77_Time_t* time);

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
    newTotalDuration = totalDuration;


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
 * @brief Clock_calcMacrotickDuration
 * Using the FlexRay clock synchronisation mechanismn to divide totalDuration in UPDATE_RATE
 * Macroticks applying the correction value once every second.
 * @return macrotick duration in number of timer clicks (input for timer compare register)
 */
static inline uint16_t Clock_calcMacrotickDuration(){
    int32_t tmp = totalDuration + lastOverflow + immediateCorrection;
    // assuming a division is faster in this case
    int32_t divider = tmp / UPDATE_RATE;

    int32_t remainder = tmp - (divider * UPDATE_RATE);
    // < 0 not possible
    // = 0 worked out perfect
    if(remainder == 0){
        lastOverflow = 0;
        return (uint16_t) (divider);
    }else{
        // remainder > 0
        lastOverflow = remainder - UPDATE_RATE;
        return (uint16_t) (divider+1);
    }
}

/**
 * Compare Interrupt
 */
void TIM4_IRQHandler(void){
    // reset counter
    TIM4->CNT = 0;

    // clear IRQ Status
    TIM_ClearITPendingBit(TIM4, TIM_IT_CC1);
    NVIC_ClearPendingIRQ(TIM2_IRQn);

    // now increment local loop counter and modify local
    // time on second increment
    ++loops;
    if(loops == UPDATE_RATE){
        Clock_IncrementSecond();

        // reset timing information for next 1 second round
        loops = 0;
        totalDuration = newTotalDuration;
        lastOverflow = 0;
        currentCycleDuration = 0;
        immediateCorrection = 0; // only applied till next cycle/second starts, then
        // newTotalDuration contains corrected value
    }

    // update timer compare register
    currentCycleDuration += TIM4->CCR1;
    TIM4->CCR1 = Clock_calcMacrotickDuration();

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

/**
 * @brief Clock_Sync
 * Perform a resynchronisation between DCF77Clock and local clock
 */
void Clock_Sync(volatile struct DCF77_Time_t* dcfTime){
    NVIC_DisableIRQ(TIM4_IRQn);
    int32_t currentPos = TIM4->CNT + currentCycleDuration;
    immediateCorrection = 0;

    // store correction time
    volatile struct DCF77_Time_t* correct = nullptr;
    volatile struct DCF77_Time_t lTime;

    // perform correction
    if( currentPos >= totalDuration/2 ){
        // if the local clock is to slow, we can use current pos as new length of the cycle,
        // but we must also consider the remaining ticks until the overflow occures (calling Clock_IncrementSecond)
        newTotalDuration = currentPos;
        // but we start right now to apply our cycle change instead of waiting for the next round to start
        immediateCorrection = -(totalDuration - currentPos);

        // store corrected time
        Clock_cloneDCF(&lTime, dcfTime);

        // now decrement time by one second, to get the right time at next increment second call
        Clock_DecrementSecond(&lTime);

        // choose right correction
        correct = &lTime;
    } else {
        // no we assume the local clock is too fast, and we have to increase the cycle length
        newTotalDuration = totalDuration + currentPos;
        // but we start right now to apply our cycle change instead of waiting for the next round to start
        // so we modify the immediate correction to apply our changes
        immediateCorrection = currentPos;

        // choose right correction
        correct = dcfTime;
    }

    // ok we update the local time
    Clock_cloneDCF(&localTime, correct);


    // reenable timing interrupt
    NVIC_EnableIRQ(TIM4_IRQn);
}

/**
 * @brief clone a time from src to dest
 * @param dest
 * @param src
 */
static void Clock_cloneDCF(volatile struct DCF77_Time_t*dest, volatile struct DCF77_Time_t* src){
    dest->day = src->day;
    dest->mon = src->mon;
    dest->year = src->year;
    dest->hh = src->hh;
    dest->mm = src->mm;
    dest->ss = src->ss;
}

/**
 * @brief Clock_IncrementSecond
 * decrement time
 */
static void Clock_DecrementSecond(volatile struct DCF77_Time_t* time){
    time->ss--;//Addiere -1 zu Sekunden

    // underflow causes wrap arround
    if (localTime.ss > 60)
    {
        localTime.ss = 0;
        localTime.mm--;//Addiere -1 zu Minuten
        if (localTime.mm > 60) { // underflow causes wrap arround
            localTime.mm = 0;
            localTime.hh--;//Addiere -1 zu Stunden
            if (localTime.hh > 24) { // underflow causes wrap arround
                localTime.hh = 0;
            }
        }
    }
}
