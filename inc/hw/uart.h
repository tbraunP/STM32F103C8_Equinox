#ifndef UART_H
#define UART_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

void UART_init();

void UART_Send(const uint8_t* str, uint16_t len);

void UART_SendString(const char* str);

#ifdef __cplusplus
}
#endif

#endif // UART_H
