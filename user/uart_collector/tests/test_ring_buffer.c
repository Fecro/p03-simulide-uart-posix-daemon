/**
 * @file test_ring_buffer.c
 * @brief Unit tests for ring_buffer covering wrap-around, bounds, and peek
 * semantics.
 */
#include "ring_buffer.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_null_guards(void) {
  uint8_t buf[4] = {1U, 2U, 3U, 4U};
  ring_buffer_t rb;

  assert(rb_init(nullptr) == RB_ERR_NULL_PTR);
  assert(rb_init(&rb) == RB_OK);
  assert(rb_push(nullptr, buf, 4U) == RB_ERR_NULL_PTR);
  assert(rb_push(&rb, nullptr, 4U) == RB_ERR_NULL_PTR);
  assert(rb_peek(&rb, 0U, nullptr, 1U) == RB_ERR_NULL_PTR);
  assert(rb_consume(nullptr, 1U) == RB_ERR_NULL_PTR);
}

static void test_wrap_around_and_peek(void) {
  ring_buffer_t rb;
  assert(rb_init(&rb) == RB_OK);

  /* Fill 500 of 512 bytes, then consume 490 so tail wraps around on next push
   */
  uint8_t fill[500];
  for (size_t i = 0U; i < sizeof(fill); ++i) {
    fill[i] = (uint8_t)(i & 0xFFU);
  }
  assert(rb_push(&rb, fill, sizeof(fill)) == RB_OK);
  assert(rb_count(&rb) == 500U);
  assert(rb_free_space(&rb) == 12U);

  assert(rb_consume(&rb, 490U) == RB_OK);
  assert(rb_count(&rb) == 10U);
  assert(rb_free_space(&rb) == 502U);

  /* Push 8-byte Golden Test Vector across the RB_CAPACITY boundary */
  const uint8_t frame[8] = {0xAAU, 0x55U, 0x01U, 0x00U,
                            0x00U, 0x02U, 0xA9U, 0x09U};
  assert(rb_push(&rb, frame, sizeof(frame)) == RB_OK);
  assert(rb_count(&rb) == 18U);

  /* Peek at offset 10 without consuming */
  uint8_t out[8] = {0};
  assert(rb_peek(&rb, 10U, out, sizeof(out)) == RB_OK);
  assert(memcmp(out, frame, sizeof(frame)) == 0);
  assert(rb_count(&rb) == 18U);

  /* Overfill check */
  assert(rb_push(&rb, fill, 500U) == RB_ERR_FULL);

  /* Consume remaining and verify underflow guard */
  assert(rb_consume(&rb, 18U) == RB_OK);
  assert(rb_count(&rb) == 0U);
  assert(rb_consume(&rb, 1U) == RB_ERR_EMPTY);
}

int main(void) {
  test_null_guards();
  test_wrap_around_and_peek();
  printf(
      "[OK] All ring_buffer unit tests passed (wrap-around, bounds, peek).\n");
  return 0;
}
