/**
 * @file main.c
 * @brief Asynchronous epoll UART telemetry collector daemon in ISO C23.
 */
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "frame_parser.h"
#include "ring_buffer.h"
#include "serial_port.h"

enum { RX_CHUNK_SIZE = 128U, EPOLL_MAX_EVENTS = 1, EPOLL_TIMEOUT_MS = 1000 };

static volatile sig_atomic_t g_running = 1;

static void on_signal(int sig) {
  (void)sig;
  g_running = 0;
}

static void print_summary(const parser_stats_t *stats) {
  printf("\n--- Telemetry Session Summary ---\n");
  printf("Valid frames  : %" PRIu64 "\n", stats->valid_frames);
  printf("CRC errors    : %" PRIu64 "\n", stats->crc_errors);
  printf("Dropped frames: %" PRIu64 "\n", stats->dropped_frames);
  printf("Resync bytes  : %" PRIu64 "\n", stats->resync_bytes);
  (void)fflush(stdout);
}

int main(int argc, char *argv[]) {
  const char *dev_path = "/dev/ttyAMA1";
  if (argc >= 2 && argv[1] != nullptr && argv[1][0] != '\0') {
    dev_path = argv[1];
  }

  struct sigaction sa = {0};
  sa.sa_handler = on_signal;
  sigemptyset(&sa.sa_mask);
  (void)sigaction(SIGINT, &sa, nullptr);
  (void)sigaction(SIGTERM, &sa, nullptr);

  int serial_fd = -1;
  serial_status_t sst = serial_open_raw(dev_path, &serial_fd);
  if (sst != SERIAL_OK) {
    fprintf(stderr, "[ERROR] Failed to open serial port '%s' (err=%d: %s)\n",
            dev_path, (int)sst, strerror(errno));
    return EXIT_FAILURE;
  }

  int epfd = epoll_create1(EPOLL_CLOEXEC);
  if (epfd < 0) {
    fprintf(stderr, "[ERROR] epoll_create1 failed: %s\n", strerror(errno));
    (void)close(serial_fd);
    return EXIT_FAILURE;
  }

  struct epoll_event ev = {.events = EPOLLIN, .data.fd = serial_fd};
  if (epoll_ctl(epfd, EPOLL_CTL_ADD, serial_fd, &ev) != 0) {
    fprintf(stderr, "[ERROR] epoll_ctl ADD failed: %s\n", strerror(errno));
    (void)close(epfd);
    (void)close(serial_fd);
    return EXIT_FAILURE;
  }

  ring_buffer_t rb;
  parser_stats_t stats;
  if (rb_init(&rb) != RB_OK || parser_stats_init(&stats) != PARSER_FRAME_OK) {
    fprintf(stderr, "[ERROR] Internal buffer initialization failed.\n");
    (void)close(epfd);
    (void)close(serial_fd);
    return EXIT_FAILURE;
  }

  printf("[INFO] uart_collector_d listening on %s (115200 8N1 raw, epoll)...\n",
         dev_path);
  (void)fflush(stdout);

  struct epoll_event events[EPOLL_MAX_EVENTS];

  while (g_running != 0) {
    int nfds = epoll_wait(epfd, events, EPOLL_MAX_EVENTS, EPOLL_TIMEOUT_MS);
    if (nfds < 0) {
      if (errno == EINTR) {
        continue;
      }
      fprintf(stderr, "[ERROR] epoll_wait failed: %s\n", strerror(errno));
      break;
    }
    if (nfds == 0) {
      continue;
    }

    if ((events[0].events & (EPOLLERR | EPOLLHUP)) != 0U) {
      fprintf(stderr, "[WARN] Serial link hangup/error event received.\n");
      break;
    }

    if ((events[0].events & EPOLLIN) != 0U) {
      uint8_t rx_buf[RX_CHUNK_SIZE];
      ssize_t nread = read(serial_fd, rx_buf, sizeof(rx_buf));

      if (nread < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
          continue;
        }
        fprintf(stderr, "[ERROR] read() failed: %s\n", strerror(errno));
        break;
      }
      if (nread == 0) {
        continue;
      }

      if (rb_push(&rb, rx_buf, (size_t)nread) != RB_OK) {
        fprintf(stderr, "[WARN] Ring buffer full, resetting stream.\n");
        if (rb_init(&rb) != RB_OK) {
          break;
        }
        continue;
      }

      parsed_telemetry_t sample = {0};
      while (parser_try_extract(&stats, &rb, &sample) == PARSER_FRAME_OK) {
        printf("[RX #%05u] ADC: %4u / 1023 | Voltage: %u.%03u V | CRC: 0x%04X "
               "[OK]\n",
               sample.seq_id, sample.adc_raw, sample.voltage_mv / 1000U,
               sample.voltage_mv % 1000U, sample.crc16);
        (void)fflush(stdout);
      }
    }
  }

  print_summary(&stats);
  (void)close(epfd);
  (void)close(serial_fd);
  return EXIT_SUCCESS;
}
