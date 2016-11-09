#ifndef MY_UTIL_H
#define MY_UTIL_H

#include <stdint.h>
#include <stddef.h>

void my_memset(uint8_t *dst, uint8_t val, size_t len);
void my_memcpy(uint8_t *dst, uint8_t *src, size_t len);
bool my_memcmp(uint8_t *dst, uint8_t *src, size_t len);
void my_print_hex(uint8_t *buf, size_t len);
//void my_fmt_print(char *fmt, ...);

#endif // MY_UTIL_H
