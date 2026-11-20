/**
 * @file ring_buffer.c
 * @brief Deterministic circular FIFO buffer implementation using bitmask
 * wrap-around.
 */
#include "ring_buffer.h"
#include <string.h>

rb_status_t rb_init(ring_buffer_t *rb) {
  if (rb == nullptr) {
    return RB_ERR_NULL_PTR;
  }

  (void)memset(rb->data, 0, sizeof(rb->data));
  rb->head = 0U;
  rb->tail = 0U;
  rb->count = 0U;
  return RB_OK;
}

size_t rb_count(const ring_buffer_t *rb) {
  if (rb == nullptr) {
    return 0U;
  }
  return rb->count;
}

size_t rb_free_space(const ring_buffer_t *rb) {
  if (rb == nullptr) {
    return 0U;
  }
  return RB_CAPACITY - rb->count;
}

rb_status_t rb_push(ring_buffer_t *rb, const uint8_t *src, size_t len) {
  if (rb == nullptr || src == nullptr) {
    return RB_ERR_NULL_PTR;
  }
  if (len == 0U) {
    return RB_OK;
  }
  if (len > rb_free_space(rb)) {
    return RB_ERR_FULL;
  }

  for (size_t i = 0U; i < len; ++i) {
    rb->data[rb->tail] = src[i];
    rb->tail = (rb->tail + 1U) & RB_MASK;
  }

  size_t next_count = 0U;
  if (ckd_add(&next_count, rb->count, len)) {
    return RB_ERR_BOUNDS;
  }
  rb->count = next_count;
  return RB_OK;
}

rb_status_t rb_peek(const ring_buffer_t *rb, size_t offset, uint8_t *dst,
                    size_t len) {
  if (rb == nullptr || dst == nullptr) {
    return RB_ERR_NULL_PTR;
  }
  if (len == 0U) {
    return RB_OK;
  }

  size_t required = 0U;
  if (ckd_add(&required, offset, len) || required > rb->count) {
    return RB_ERR_EMPTY;
  }

  size_t idx = (rb->head + offset) & RB_MASK;
  for (size_t i = 0U; i < len; ++i) {
    dst[i] = rb->data[idx];
    idx = (idx + 1U) & RB_MASK;
  }

  return RB_OK;
}

rb_status_t rb_consume(ring_buffer_t *rb, size_t len) {
  if (rb == nullptr) {
    return RB_ERR_NULL_PTR;
  }
  if (len == 0U) {
    return RB_OK;
  }
  if (len > rb->count) {
    return RB_ERR_EMPTY;
  }

  rb->head = (rb->head + len) & RB_MASK;
  rb->count -= len;
  return RB_OK;
}
