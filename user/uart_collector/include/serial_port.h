/**
 * @file serial_port.h
 * @brief POSIX termios raw-mode UART configuration (115200 8N1, non-blocking).
 */
#ifndef P03_SERIAL_PORT_H
#define P03_SERIAL_PORT_H

#include "c23_compat.h"

typedef enum {
  SERIAL_OK = 0,
  SERIAL_ERR_NULL_PTR = -1,
  SERIAL_ERR_OPEN = -2,
  SERIAL_ERR_TERMIOS = -3
} serial_status_t;

P03_NODISCARD serial_status_t serial_open_raw(const char *dev_path,
                                              int *out_fd);

#endif /* P03_SERIAL_PORT_H */
