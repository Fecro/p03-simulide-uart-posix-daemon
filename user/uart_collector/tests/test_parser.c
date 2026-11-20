/**
 * @file test_parser.c
 * @brief Unit tests for FSM frame synchronization, CRC error recovery, and drop
 * detection.
 */
#include "frame_parser.h"
#include <assert.h>
#include <stdio.h>

static void build_test_frame(uint16_t seq, uint16_t adc, uint8_t out[8]) {
  out[0] = PARSER_SOF_0;
  out[1] = PARSER_SOF_1;
  out[2] = (uint8_t)(seq & 0xFFU);
  out[3] = (uint8_t)(seq >> 8U);
  out[4] = (uint8_t)(adc & 0xFFU);
  out[5] = (uint8_t)(adc >> 8U);
  uint16_t crc = parser_crc16_ccitt(out, 6U);
  out[6] = (uint8_t)(crc & 0xFFU);
  out[7] = (uint8_t)(crc >> 8U);
}

int main(void) {
  ring_buffer_t rb;
  parser_stats_t stats;
  parsed_telemetry_t sample = {0};

  assert(rb_init(&rb) == RB_OK);
  assert(parser_stats_init(&stats) == PARSER_FRAME_OK);

  /* 1. Inject 4 noise bytes + first 4 bytes of Golden Test Vector (seq=1,
   * adc=512) */
  const uint8_t noise[4] = {0x00U, 0xFFU, 0xAAU, 0x12U};
  uint8_t f1[8] = {0};
  build_test_frame(1U, 512U, f1);

  assert(rb_push(&rb, noise, 4U) == RB_OK);
  assert(rb_push(&rb, &f1[0], 4U) == RB_OK);
  assert(parser_try_extract(&stats, &rb, &sample) == PARSER_NEED_MORE_DATA);

  /* 2. Push remaining 4 bytes of frame 1 */
  assert(rb_push(&rb, &f1[4], 4U) == RB_OK);
  assert(parser_try_extract(&stats, &rb, &sample) == PARSER_FRAME_OK);
  assert(sample.seq_id == 1U);
  assert(sample.adc_raw == 512U);
  assert(sample.crc16 == 0x09A9U);
  assert(sample.voltage_mv == 2502U);
  assert(stats.resync_bytes == 4U);

  /* 3. Inject corrupted frame (seq=2, flipped bit in ADC) + valid frame (seq=3,
   * adc=1023) */
  uint8_t f2_bad[8] = {0};
  uint8_t f3_ok[8] = {0};
  build_test_frame(2U, 256U, f2_bad);
  f2_bad[4] ^= 0x01U; /* Corrupt 1 bit */
  build_test_frame(3U, 1023U, f3_ok);

  assert(rb_push(&rb, f2_bad, 8U) == RB_OK);
  assert(rb_push(&rb, f3_ok, 8U) == RB_OK);

  assert(parser_try_extract(&stats, &rb, &sample) == PARSER_FRAME_OK);
  assert(sample.seq_id == 3U);
  assert(sample.adc_raw == 1023U);
  assert(sample.voltage_mv == 5000U);
  assert(stats.crc_errors == 1U);
  assert(stats.dropped_frames == 1U);
  assert(stats.valid_frames == 2U);

  printf("[OK] All frame_parser tests passed (resync, CRC recovery, drop "
         "detection).\n");
  return 0;
}
