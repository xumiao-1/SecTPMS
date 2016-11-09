#include "my_util.h"
#include <SPI.h>
#include <stdarg.h>

void my_memset(uint8_t *dst, uint8_t val, size_t len) {
	for (uint8_t i = 0; i < len; i++) {
		dst[i] = val;
	}
}

void my_memcpy(uint8_t *dst, uint8_t *src, size_t len) {
	for (uint8_t i = 0; i < len; i++) {
		dst[i] = src[i];
	}
}

bool my_memcmp(uint8_t *dst, uint8_t *src, size_t len) {
	for (uint8_t i = 0; i < len; i++) {
		if (dst[i] != src[i])
			return false;
	}

	return true;
}

void my_print_hex(uint8_t *buf, size_t len) {
	for (uint8_t i = 0; i < len; i++) {
		if (buf[i] < 0x10) {
			Serial.print("0");
		}
		Serial.print(buf[i], HEX);
	}
//	Serial.println();
}

//void my_fmt_print(char *fmt, ... ){
//        char tmp[128]; // resulting string limited to 128 chars
//        va_list args;
//        va_start (args, fmt );
//        vsnprintf(tmp, 128, fmt, args);
//        va_end (args);
//        Serial.print(tmp);
//}
