/**
 * @file serial_port.c
 * @brief Non-blocking POSIX termios raw-mode serial port initializer.
 */
#define _DEFAULT_SOURCE

#include "serial_port.h"

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

serial_status_t serial_open_raw(const char *dev_path, int *out_fd) {
  if (dev_path == nullptr || out_fd == nullptr) {
    return SERIAL_ERR_NULL_PTR;
  }

  int fd = open(dev_path, O_RDONLY | O_NOCTTY | O_NONBLOCK);
  if (fd < 0) {
    return SERIAL_ERR_OPEN;
  }

  struct termios tty = {0};
  if (tcgetattr(fd, &tty) != 0) {
    (void)close(fd);
    return SERIAL_ERR_TERMIOS;
  }

  /* Disable canonical line buffering, echo, signals, and XON/XOFF translation
   */
  cfmakeraw(&tty);

  /* Configure 115200 bps, 8N1, local line, enable receiver */
  (void)cfsetispeed(&tty, B115200);
  (void)cfsetospeed(&tty, B115200);

  tty.c_cflag &= ~(tcflag_t)(CSIZE | PARENB | CSTOPB);
  tty.c_cflag |= (tcflag_t)(CS8 | CLOCAL | CREAD);

  /* Non-blocking reads return immediately with available bytes */
  tty.c_cc[VMIN] = 0U;
  tty.c_cc[VTIME] = 0U;

  if (tcsetattr(fd, TCSANOW, &tty) != 0) {
    (void)close(fd);
    return SERIAL_ERR_TERMIOS;
  }

  (void)tcflush(fd, TCIFLUSH);
  *out_fd = fd;
  return SERIAL_OK;
}
