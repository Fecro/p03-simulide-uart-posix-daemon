/**
 * @file adc.c
 * @brief Bare-metal 10-bit ADC register-level driver for ATmega328P.
 */
#include "adc.h"
#include <avr/io.h>

void adc_init(void) {
  /* Voltage reference = AVCC (5V) with external capacitor at AREF */
  ADMUX = (uint8_t)(1U << REFS0);

  /* Enable ADC with prescaler 128 -> 16 MHz / 128 = 125 kHz ADC clock */
  ADCSRA =
      (uint8_t)((1U << ADEN) | (1U << ADPS2) | (1U << ADPS1) | (1U << ADPS0));
}

uint16_t adc_read_channel(uint8_t channel) {
  /* Select ADC channel (0..7) while preserving reference selection */
  ADMUX = (uint8_t)((ADMUX & 0xF0U) | (channel & 0x07U));

  /* Start single conversion */
  ADCSRA |= (uint8_t)(1U << ADSC);

  /* Wait until hardware clears ADSC upon conversion completion */
  while ((ADCSRA & (uint8_t)(1U << ADSC)) != 0U) {
    /* Busy-wait for ADC result */
  }

  /* Reading ADC macro reads ADCL then ADCH in the required order */
  return ADC;
}
