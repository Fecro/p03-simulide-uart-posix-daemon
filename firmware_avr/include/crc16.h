/**
 * @file crc16.h
 * @brief CRC-16/CCITT-FALSE integrity check (Polynomial 0x1021, Init 0xFFFF).
 */
#ifndef P03_CRC16_H
#define P03_CRC16_H

#include <stddef.h>
#include <stdint.h>

uint16_t crc16_ccitt_false(const uint8_t *data, size_t len);

#endif /* P03_CRC16_H */
