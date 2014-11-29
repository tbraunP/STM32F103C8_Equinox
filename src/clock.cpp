#include "clock.h"

#include "stm32f10x_conf.h"
#include "stm32f10x.h"

#include "hw/uart.h"
#include "util/itoa.h"
#include "dcf77.h"

// CONSTANTS
#define     UPDATE_RATE_SEC         (25)
#define     RATE_MIN                (60*UPDATE_RATE_SEC)
#define     PRESCALER               (uint16_t) (0x2C1E)
#define     COUNTERVALUE40MS        (uint16_t) (0xFF)
#define     COUNTERVALUE40MS_2      (uint16_t) (COUNTERVALUE40MS/2)

// length of current second cycle in timer click
static volatile int64_t totalDuration = 0;
static volatile uint16_t lastCompareDuration = COUNTERVALUE40MS;
static volatile uint16_t previousCompareRegisterValue = COUNTERVALUE40MS;

// new duration of whole cycle after correction in timer clicks
static volatile int64_t newTotalDuration = 0;
static volatile int64_t immediateCorrection = 0;

// position in current cycle
static volatile int64_t currentCycleDuration = 0;

// macrotick counter
static volatile uint16_t loops = 0;


// stores round overflow occuring by switching to the next macrotick,
// using the terms of flexray
static volatile int64_t lastOverflow = 0;

/**
 * @brief localTime - time of local clock
 */
static volatile struct DCF77_Time_t localTime;


// forward declarations
static void updateVisualization(volatile struct DCF77_Time_t* localTime, uint16_t loops, uint16_t MAXRATE);

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
    lastCompareDuration = COUNTERVALUE40MS;
    TIM4->CCR1 = COUNTERVALUE40MS;

    // total time
    totalDuration = COUNTERVALUE40MS * RATE_MIN;
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
    int32_t divider = tmp / RATE_MIN;

    int32_t remainder = tmp - (divider * RATE_MIN);
    // < 0 not possible
    // = 0 worked out perfect
    if(remainder == 0){
        lastOverflow = 0;
    }else{
        // remainder > 0
        lastOverflow = remainder - RATE_MIN;
        ++divider;
    }

    // check bounds
    if(divider <= COUNTERVALUE40MS_2)
        return COUNTERVALUE40MS_2;
    if(divider >= COUNTERVALUE40MS+COUNTERVALUE40MS_2)
        return (COUNTERVALUE40MS+COUNTERVALUE40MS_2);
    return (uint16_t)divider;
}

/**
 * Compare Interrupt
 */
void TIM4_IRQHandler(void){
    if(TIM_GetITStatus(TIM4, TIM_IT_CC1) == SET){
        // clear IRQ Status
        TIM_ClearITPendingBit(TIM4, TIM_IT_CC1);
        NVIC_ClearPendingIRQ(TIM2_IRQn);

        // now increment local loop counter and modify local
        // time on second increment
        ++loops;
        if(loops == RATE_MIN){
            // reset timing information for next 1 second round
            loops = 0;
            totalDuration = newTotalDuration;
            lastOverflow = 0;
            currentCycleDuration = 0;
            immediateCorrection = 0; // only applied till next cycle/second starts, then
            // newTotalDuration contains corrected value
        }

        if(loops % UPDATE_RATE_SEC == 0){
            DCF77_incrementTime(&localTime);
            UART_SendString("S\n\0");
        }

        // update timer compare register and update duration of current cycle /second in ticks
        currentCycleDuration += lastCompareDuration;
        lastCompareDuration = Clock_calcMacrotickDuration();
        previousCompareRegisterValue = TIM4->CCR1;
        TIM4->CCR1 += lastCompareDuration;

        // update visualization of clock
        updateVisualization(&localTime, (loops % UPDATE_RATE_SEC), UPDATE_RATE_SEC);
    }
}


// tobe removed
static void updateVisualization(volatile struct DCF77_Time_t* localTime, uint16_t loops, uint16_t MAXRATE){

}

/**
 * @brief Clock_Sync
 * Perform a resynchronisation between DCF77Clock and local clock
 */
void Clock_Sync(volatile struct DCF77_Time_t* dcfTime){
    NVIC_DisableIRQ(TIM4_IRQn);
    int64_t currentPos = (TIM4->CNT - previousCompareRegisterValue) + currentCycleDuration;
    immediateCorrection = 0;

    // store correction time
    volatile struct DCF77_Time_t* correct = nullptr;
    volatile struct DCF77_Time_t lTime;

    // perform correction
    // NOTE: We assume WC, we are near the middle when the external trigger hit, therefore /2 correction rate,
    // slower convergence but more stable
    if( currentPos >= totalDuration/2 ){
        // if the local clock is to slow, we can use current pos as new length of the cycle,
        // but we must also consider the remaining ticks until the overflow occures (calling Clock_IncrementSecond)
        newTotalDuration = (totalDuration + currentPos)/2;

        // but we start right now to apply our cycle change instead of waiting for the next round to start
        immediateCorrection = -(totalDuration - currentPos)/2;

        // store corrected time
        DFC77_cloneDCF(&lTime, dcfTime);

        // now decrement time by one second, to get the right time at next increment second call
        DCF77_decrementTime(&lTime);

        // choose right correction
        correct = &lTime;
    } else {
        // no we assume the local clock is too fast, and we have to increase the cycle length
        newTotalDuration = totalDuration + (currentPos/2);
        // but we start right now to apply our cycle change instead of waiting for the next round to start
        // so we modify the immediate correction to apply our changes
        immediateCorrection = (currentPos/2);

        // choose right correction
        correct = dcfTime;
    }
    // ok we update the local time
    DFC77_cloneDCF(&localTime, correct);

    // reenable timing interrupt
    NVIC_EnableIRQ(TIM4_IRQn);

    // print correction value
    char str[100];
    itoa((int) immediateCorrection, str);
    UART_SendString(str);
    UART_SendString(":\0");
    itoa((int) totalDuration, str);
    UART_SendString(str);
    UART_SendString(":\0");
    itoa((int) newTotalDuration, str);
    UART_SendString(str);
    UART_SendString("\n\0");
}

