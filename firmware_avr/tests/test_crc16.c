/**
 * @file test_crc16.c
 * @brief Host unit test verifying the Golden Test Vector for the UART frame.
 */
#include "crc16.h"
#include "frame.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
  telemetry_frame_t frame = {.sof = {FRAME_SOF_0, FRAME_SOF_1},
                             .seq_id = 1U,
                             .adc_raw = 512U,
                             .crc16 = 0U};

  frame.crc16 = crc16_ccitt_false((const uint8_t *)&frame, FRAME_CRC_OFFSET);

  const uint8_t *raw = (const uint8_t *)&frame;
  printf("Frame (8B): %02X %02X %02X %02X %02X %02X %02X %02X\n", raw[0],
         raw[1], raw[2], raw[3], raw[4], raw[5], raw[6], raw[7]);
  printf("CRC-16    : 0x%04X\n", frame.crc16);

  assert(frame.crc16 == 0x09A9U);
  assert(raw[6] == 0xA9U && raw[7] == 0x09U);
  printf("[OK] Golden Test Vector verified!\n");
  return 0;
}
