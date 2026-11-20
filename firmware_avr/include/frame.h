/**
 * @file frame.h
 * @brief 8-byte UART binary protocol definition (Little-Endian).
 */
#ifndef P03_FRAME_H
#define P03_FRAME_H

#include <stdint.h>

enum {
  FRAME_SOF_0 = 0xAAU,
  FRAME_SOF_1 = 0x55U,
  FRAME_SIZE_BYTES = 8U,
  FRAME_CRC_OFFSET = 6U
};

typedef struct __attribute__((packed)) {
  uint8_t sof[2];   /* 0xAA, 0x55 */
  uint16_t seq_id;  /* Little-Endian: incremental counter */
  uint16_t adc_raw; /* Little-Endian: 10-bit ADC reading (0..1023) */
  uint16_t crc16;   /* Little-Endian: CRC-16/CCITT-FALSE over bytes 0..5 */
} telemetry_frame_t;

_Static_assert(sizeof(telemetry_frame_t) == FRAME_SIZE_BYTES,
               "telemetry_frame_t must be exactly 8 bytes");
#endif /* P03_FRAME_H */
