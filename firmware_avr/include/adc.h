/**
 * @file adc.h
 * @brief Bare-metal 10-bit ADC driver interface for ATmega328P.
 */
#ifndef P03_ADC_H
#define P03_ADC_H

#include <stdint.h>

void adc_init(void);
uint16_t adc_read_channel(uint8_t channel);

#endif /* P03_ADC_H */
