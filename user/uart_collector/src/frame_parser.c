/**
 * @file frame_parser.c
 * @brief Alignment-safe Little-Endian UART frame decoder with CRC-16
 * verification.
 */
#include "frame_parser.h"

static uint16_t read_u16_le(const uint8_t *buf) {
  return (uint16_t)((uint16_t)buf[0] | (uint16_t)((uint32_t)buf[1] << 8U));
}

uint16_t parser_crc16_ccitt(const uint8_t *data, size_t len) {
  uint16_t crc = 0xFFFFU;

  if (data == nullptr) {
    return 0U;
  }

  for (size_t i = 0U; i < len; ++i) {
    crc ^= (uint16_t)((uint32_t)data[i] << 8U);
    for (uint8_t bit = 0U; bit < 8U; ++bit) {
      if ((crc & 0x8000U) != 0U) {
        crc = (uint16_t)(((uint32_t)crc << 1U) ^ 0x1021U);
      } else {
        crc = (uint16_t)((uint32_t)crc << 1U);
      }
    }
  }

  return crc;
}

parser_status_t parser_stats_init(parser_stats_t *stats) {
  if (stats == nullptr) {
    return PARSER_ERR_NULL_PTR;
  }

  stats->valid_frames = 0U;
  stats->crc_errors = 0U;
  stats->dropped_frames = 0U;
  stats->resync_bytes = 0U;
  stats->last_seq_id = 0U;
  stats->has_last_seq = false;
  return PARSER_FRAME_OK;
}

parser_status_t parser_try_extract(parser_stats_t *stats, ring_buffer_t *rb,
                                   parsed_telemetry_t *out_sample) {
  if (stats == nullptr || rb == nullptr || out_sample == nullptr) {
    return PARSER_ERR_NULL_PTR;
  }

  while (rb_count(rb) >= PARSER_FRAME_SIZE) {
    uint8_t b0 = 0U;
    if (rb_peek(rb, 0U, &b0, 1U) != RB_OK) {
      return PARSER_NEED_MORE_DATA;
    }
    if (b0 != PARSER_SOF_0) {
      if (rb_consume(rb, 1U) != RB_OK) {
        return PARSER_NEED_MORE_DATA;
      }
      ++stats->resync_bytes;
      continue;
    }

    uint8_t b1 = 0U;
    if (rb_peek(rb, 1U, &b1, 1U) != RB_OK) {
      return PARSER_NEED_MORE_DATA;
    }
    if (b1 != PARSER_SOF_1) {
      if (rb_consume(rb, 1U) != RB_OK) {
        return PARSER_NEED_MORE_DATA;
      }
      ++stats->resync_bytes;
      continue;
    }

    uint8_t raw[PARSER_FRAME_SIZE] = {0};
    if (rb_peek(rb, 0U, raw, sizeof(raw)) != RB_OK) {
      return PARSER_NEED_MORE_DATA;
    }

    uint16_t calc_crc = parser_crc16_ccitt(raw, PARSER_CRC_OFFSET);
    uint16_t rx_crc = read_u16_le(&raw[6]);

    if (calc_crc != rx_crc) {
      ++stats->crc_errors;
      if (rb_consume(rb, 1U) != RB_OK) {
        return PARSER_NEED_MORE_DATA;
      }
      ++stats->resync_bytes;
      continue;
    }

    /* Valid frame confirmed: consume all 8 bytes */
    if (rb_consume(rb, PARSER_FRAME_SIZE) != RB_OK) {
      return PARSER_NEED_MORE_DATA;
    }

    uint16_t seq_id = read_u16_le(&raw[2]);
    uint16_t adc_raw = read_u16_le(&raw[4]);

    if (stats->has_last_seq) {
      uint16_t expected = (uint16_t)(stats->last_seq_id + 1U);
      if (seq_id != expected) {
        uint16_t delta = (uint16_t)(seq_id - expected);
        stats->dropped_frames += (uint64_t)delta;
      }
    }
    stats->last_seq_id = seq_id;
    stats->has_last_seq = true;
    ++stats->valid_frames;

    uint32_t scaled_mv = 0U;
    if (ckd_mul(&scaled_mv, (uint32_t)adc_raw, (uint32_t)PARSER_VREF_MV)) {
      return PARSER_ERR_OVERFLOW;
    }

    out_sample->seq_id = seq_id;
    out_sample->adc_raw = adc_raw;
    out_sample->crc16 = rx_crc;
    out_sample->voltage_mv = scaled_mv / (uint32_t)PARSER_ADC_MAX;
    return PARSER_FRAME_OK;
  }

  return PARSER_NEED_MORE_DATA;
}
