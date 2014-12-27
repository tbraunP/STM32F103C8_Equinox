#ifndef NVICPRIORITIES_H
#define NVICPRIORITIES_H

#ifdef __cplusplus
 extern "C" {
#endif

// TIM4 Counter for Visualization Clock
#define NVIC_TIM4_PreemptionPriority    (0xD)
#define NVIC_TIM4_SubPriority           (0xD)

// DCF77 Timer for Minute GAP
#define NVIC_TIM2_PreemptionPriority    (0xC)
#define NVIC_TIM2_SubPriority           (0xC)

// DCF77 Edge Interrupt
#define NVIC_EXTI_PreemptionPriority    (0xB)
#define NVIC_EXTI_SubPriority           (0xB)

// WS2812 DMA finished Interrupt
#define NVIC_DMA_PreemptionPriority     (0xE)
#define NVIC_DMA_SubPriority            (0xE)

#ifdef __cplusplus
 }
#endif


#endif // NVICPRIORITIES_H

