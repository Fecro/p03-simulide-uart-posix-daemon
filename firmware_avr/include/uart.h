/**
 * @file uart.h
 * @brief Bare-metal USART0 driver interface for ATmega328P (115200 8N1).
 */
#ifndef P02_UART_H
#define P02_UART_H

#include <stddef.h>
#include <stdint.h>

void uart_init(uint32_t baudrate);
void uart_send_byte(uint8_t byte);
void uart_send_buffer(const uint8_t *buf, size_t len);

#endif /* P02_UART_H */
