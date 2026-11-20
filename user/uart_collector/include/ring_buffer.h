/**
 * @file ring_buffer.h
 * @brief Fixed-capacity, zero-heap circular FIFO byte buffer (MISRA C:2025
 * aligned).
 */
#ifndef P03_RING_BUFFER_H
#define P03_RING_BUFFER_H

#include "c23_compat.h"

enum { RB_CAPACITY = 512U, RB_MASK = RB_CAPACITY - 1U };

_Static_assert((RB_CAPACITY & RB_MASK) == 0U,
               "RB_CAPACITY must be a power of two for bitmask indexing");

typedef enum {
  RB_OK = 0,
  RB_ERR_NULL_PTR = -1,
  RB_ERR_FULL = -2,
  RB_ERR_EMPTY = -3,
  RB_ERR_BOUNDS = -4
} rb_status_t;

typedef struct {
  uint8_t data[RB_CAPACITY];
  size_t head;  /* Read index  */
  size_t tail;  /* Write index */
  size_t count; /* Current number of stored bytes */
} ring_buffer_t;

P03_NODISCARD rb_status_t rb_init(ring_buffer_t *rb);
P03_NODISCARD size_t rb_count(const ring_buffer_t *rb);
P03_NODISCARD size_t rb_free_space(const ring_buffer_t *rb);
P03_NODISCARD rb_status_t rb_push(ring_buffer_t *rb, const uint8_t *src,
                                  size_t len);
P03_NODISCARD rb_status_t rb_peek(const ring_buffer_t *rb, size_t offset,
                                  uint8_t *dst, size_t len);
P03_NODISCARD rb_status_t rb_consume(ring_buffer_t *rb, size_t len);

#endif /* P03_RING_BUFFER_H */
