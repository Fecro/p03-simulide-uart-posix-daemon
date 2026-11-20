/**
 * @file main.c
 * @brief Bare-metal sensor node firmware for ATmega328P (SimulIDE).
 *        Samples ADC0 at 5 Hz and transmits 8-byte CRC-16 protected frames.
 */
#include <avr/io.h>
#include <util/delay.h>

#include "adc.h"
#include "crc16.h"
#include "frame.h"
#include "uart.h"

enum { UART_BAUDRATE = 115200UL, ADC_CHANNEL_0 = 0U, SAMPLE_DELAY_MS = 200U };

int main(void) {
  /* Configure PB5 (Arduino Pin 13) as output for TX activity LED */
  DDRB |= (uint8_t)(1U << DDB5);

  uart_init(UART_BAUDRATE);
  adc_init();

  uint16_t seq_id = 0U;

  for (;;) {
    PORTB |= (uint8_t)(1U << PORTB5);

    uint16_t adc_sample = adc_read_channel(ADC_CHANNEL_0);

    telemetry_frame_t frame = {.sof = {FRAME_SOF_0, FRAME_SOF_1},
                               .seq_id = seq_id,
                               .adc_raw = adc_sample,
                               .crc16 = 0U};

    frame.crc16 = crc16_ccitt_false((const uint8_t *)&frame, FRAME_CRC_OFFSET);
    uart_send_buffer((const uint8_t *)&frame, sizeof(frame));

    ++seq_id;

    _delay_ms(40);
    PORTB &= (uint8_t)~(1U << PORTB5);
    _delay_ms(SAMPLE_DELAY_MS - 40U);
  }

  return 0;
}
