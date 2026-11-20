/**
 * @file c23_compat.h
 * @brief ISO/IEC 9899:2024 (C23) portability layer for host and
 * cross-toolchains.
 */
#ifndef P03_C23_COMPAT_H
#define P03_C23_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if defined(__has_include) && __has_include(<stdckdint.h>)
#include <stdckdint.h>
#else
#define ckd_add(r, a, b) __builtin_add_overflow((a), (b), (r))
#define ckd_sub(r, a, b) __builtin_sub_overflow((a), (b), (r))
#define ckd_mul(r, a, b) __builtin_mul_overflow((a), (b), (r))
#endif

#if !defined(__cplusplus) && (__STDC_VERSION__ < 202311L)
#ifndef nullptr
#define nullptr ((void *)0)
#endif
#endif

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 202311L)
#define P03_NODISCARD [[nodiscard]]
#elif defined(__GNUC__) || defined(__clang__)
#define P03_NODISCARD __attribute__((warn_unused_result))
#else
#define P03_NODISCARD
#endif

#endif /* P03_C23_COMPAT_H */
