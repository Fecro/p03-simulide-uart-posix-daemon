/**
 * @file frame_parser.h
 * @brief Binary UART frame FSM parser, CRC-16 validator, and telemetry scaler.
 */
#ifndef P03_FRAME_PARSER_H
#define P03_FRAME_PARSER_H

#include "c23_compat.h"
#include "ring_buffer.h"

enum {
  PARSER_SOF_0 = 0xAAU,
  PARSER_SOF_1 = 0x55U,
  PARSER_FRAME_SIZE = 8U,
  PARSER_CRC_OFFSET = 6U,
  PARSER_VREF_MV = 5000U,
  PARSER_ADC_MAX = 1023U
};

typedef enum {
  PARSER_FRAME_OK = 0,
  PARSER_NEED_MORE_DATA = 1,
  PARSER_ERR_NULL_PTR = -1,
  PARSER_ERR_OVERFLOW = -2
} parser_status_t;

typedef struct {
  uint16_t seq_id;
  uint16_t adc_raw;
  uint16_t crc16;
  uint32_t voltage_mv;
} parsed_telemetry_t;

typedef struct {
  uint64_t valid_frames;
  uint64_t crc_errors;
  uint64_t dropped_frames;
  uint64_t resync_bytes;
  uint16_t last_seq_id;
  bool has_last_seq;
} parser_stats_t;

P03_NODISCARD uint16_t parser_crc16_ccitt(const uint8_t *data, size_t len);
P03_NODISCARD parser_status_t parser_stats_init(parser_stats_t *stats);
P03_NODISCARD parser_status_t parser_try_extract(
    parser_stats_t *stats, ring_buffer_t *rb, parsed_telemetry_t *out_sample);

#endif /* P03_FRAME_PARSER_H */
