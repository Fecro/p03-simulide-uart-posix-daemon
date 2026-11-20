/**
 * @file uart.c
 * @brief Bare-metal USART0 register-level implementation for ATmega328P.
 */
#include "uart.h"
#include <avr/io.h>

#ifndef F_CPU
#error "F_CPU must be defined for UART baudrate calculation"
#endif

void uart_init(uint32_t baudrate) {
  if (baudrate == 0UL) {
    return;
  }

  /* Enable Double Speed (U2X0 = 1) to minimize baudrate clock error at 16 MHz
   */
  UCSR0A = (uint8_t)(1U << U2X0);

  /* UBRR0 = (F_CPU / (8 * baudrate)) - 1 */
  uint16_t ubrr = (uint16_t)(((F_CPU / (8UL * baudrate)) - 1UL));
  UBRR0H = (uint8_t)(ubrr >> 8U);
  UBRR0L = (uint8_t)(ubrr & 0xFFU);

  /* Enable USART0 Transmitter and Receiver */
  UCSR0B = (uint8_t)((1U << TXEN0) | (1U << RXEN0));

  /* Asynchronous mode, No Parity, 1 Stop bit, 8 Data bits (8N1) */
  UCSR0C = (uint8_t)((1U << UCSZ01) | (1U << UCSZ00));
}

void uart_send_byte(uint8_t byte) {
  /* Wait until USART Data Register Empty (UDRE0) flag is set */
  while ((UCSR0A & (uint8_t)(1U << UDRE0)) == 0U) {
    /* Busy-wait for TX buffer readiness */
  }
  UDR0 = byte;
}

void uart_send_buffer(const uint8_t *buf, size_t len) {
  if (buf == NULL) {
    return;
  }

  for (size_t i = 0U; i < len; ++i) {
    uart_send_byte(buf[i]);
  }
}
