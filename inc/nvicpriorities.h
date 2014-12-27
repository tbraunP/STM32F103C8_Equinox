#ifndef NVICPRIORITIES_H
#define NVICPRIORITIES_H

#ifdef __cplusplus
 extern "C" {
#endif

// TIM4 Counter for Visualization Clock
#define NVIC_TIM4_PreemptionPriority    (0xF)
#define NVIC_TIM4_SubPriority           (0xF)

// DCF77 Timer for Minute GAP
#define NVIC_TIM2_PreemptionPriority    (0xF)
#define NVIC_TIM2_SubPriority           (0xF)

// DCF77 Edge Interrupt
#define NVIC_EXTI_PreemptionPriority    (0xE)
#define NVIC_EXTI_SubPriority           (0xE)

// WS2812 DMA finished Interrupt
#define NVIC_DMA_PreemptionPriority     (0xF)
#define NVIC_DMA_SubPriority            (0xF)

#ifdef __cplusplus
 }
#endif


#endif // NVICPRIORITIES_H

