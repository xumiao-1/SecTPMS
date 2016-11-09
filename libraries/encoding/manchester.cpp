#include "manchester.h"
#include <stdio.h>

/**
 * Set a bit in buffer to one or zero
 *
 * @param bytes         Buffer to change bit state in
 * @param position      Position in buffer of bit, starting at 1.
 * @param bit_value     Value of one or zero that indicates new bit state
 */
void set_bit(uint8_t *bytes, uint32_t position, uint8_t bit_value) {
	uint32_t byte_offset = 0;
	uint32_t bit_offset = 0;

	position--;
	byte_offset = position / 8;
	bit_offset = 7 - (position % 8);
	bytes += byte_offset;

	if (bit_value) {
		*bytes |= (1 << bit_offset);
	} else {
		*bytes &= ~(1 << bit_offset);
	}
}

/**
 * Decode a single bit value
 * @param bit       Bit value (only 1 or 0 allowed)
 * @param previous  Previous bit (only 1 or 0 allowed)
 * @returns         One or zero, indication actual data bit value
 */
uint8_t decode_bit(uint8_t bit, uint8_t previous) {
	if (bit == previous) {
		return 1;
	} else {
		return 0;
	}
}

/**
 * Encode a bit sequence
 * if in_bytes is n length, then
 * out_bytes is 2n length.
 */
uint8_t encode_dmanchester(uint8_t *in_bytes, uint8_t *out_bytes,
		size_t in_length, uint8_t previous) {
	uint8_t *byte = in_bytes;
	uint8_t data_bit;
	int32_t bit_position;
	uint8_t half1, half2;
//	uint8_t previous = 1;
	uint32_t out_bit_position = 1;

	for (; byte < (in_bytes + in_length); byte++) { // each byte
		for (bit_position = 7; bit_position >= 0; bit_position--) {
			data_bit = (*byte & (1 << bit_position)) ? 1 : 0; // 0 or 1
			half1 = (data_bit == 1) ? previous : !previous;
			half2 = !half1;

			set_bit(out_bytes, out_bit_position++, half1);
			set_bit(out_bytes, out_bit_position++, half2);

			previous = half2;
		}
	}

	return 0;
}

uint8_t decode_dmanchester(uint8_t *in_bytes, uint8_t *out_bytes,
		size_t in_length, uint8_t previous) {
	uint8_t *byte = in_bytes;
//	uint8_t previous = 1; // Kind of hack
	uint8_t data_bit;
	uint8_t decoded_bit;
	int32_t bit_position;
	uint32_t out_bit_position = 1;

	for (; byte < (in_bytes + in_length); byte++) {
		for (bit_position = 7; bit_position >= 1; bit_position -= 2) {
			data_bit = (*byte & (1 << bit_position)) ? 1 : 0;
			decoded_bit = decode_bit(data_bit, previous);

			set_bit(out_bytes, out_bit_position++, decoded_bit);

			previous = (*byte & (1 << (bit_position - 1))) ? 1 : 0;

			if (data_bit != !previous) { // Ensure clock bit is correct
				return 1;
			}
		}
	}

	return 0;
}

//uint8_t decode_bmc(uint8_t byte) {
//    uint8_t mask = 0x3;
//    uint8_t bit = 0;
//    uint8_t decoded = 0;
//    int i = 0;
//
//    for (i = 0; i <= 3; ++i) {
//        bit = byte & mask;
//        bit >>= i * 2;
//
//        if (bit == 1 || bit == 2) {
//            decoded |= (1 << i);
//        }
//
//        mask <<= 2;
//    }
//
//    return decoded;
//}

/**
 * non-inverted: rising edge: 0
 * inverted: rising edge: 1
 */
uint8_t decode_manchester(uint8_t *in_bytes, uint8_t *out_bytes,
		size_t in_length, uint8_t inverted) {
	uint8_t *bytes = in_bytes;
	uint32_t out_bit_position = 1; // start at 1

	for (; bytes < (in_bytes + in_length); bytes++) { // each byte
		for (int8_t bit_position = 7; bit_position > 0; bit_position -= 2) { // each bit
			uint8_t half1 = (*bytes & (1 << bit_position)) ? 1 : 0;
			uint8_t half2 = (*bytes & (1 << (bit_position - 1))) ? 1 : 0;

			if (half1 == half2) {
				return 1;
			}

			uint8_t val = half1;
			if (inverted)
				val = !val;

			set_bit(out_bytes, out_bit_position++, val);
		}
	}

	return 0;
}

/**
 * non-inverted: rising edge: 0
 * inverted: rising edge: 1
 */
uint8_t encode_manchester(uint8_t *in_bytes, uint8_t *out_bytes,
		size_t in_length, uint8_t inverted) {
	uint8_t *byte = in_bytes;
	uint8_t data_bit;
	int32_t bit_position;
	uint8_t half1, half2;
	uint32_t out_bit_position = 1;

	for (; byte < (in_bytes + in_length); byte++) { // each byte
		for (bit_position = 7; bit_position >= 0; bit_position--) {
			data_bit = (*byte & (1 << bit_position)) ? 1 : 0; // 0 or 1

			half1 = data_bit;
			half2 = !half1;

			if (inverted) {
				half1 = !half1;
				half2 = !half2;
			}

			set_bit(out_bytes, out_bit_position++, half1);
			set_bit(out_bytes, out_bit_position++, half2);
		}
	}

	return 0;
}

/**
 * left shift
 */
void shift_l(uint8_t *in_bytes, uint8_t shift_num, size_t in_length) {
	uint8_t *byte;
	for (byte = in_bytes; in_length--; ++byte) {
		uint8_t bit = 0;
		if (in_length) {
			bit = (byte[1] & (0x01 << 7)) ? 1 : 0;
		}
		*byte <<= 1;
		*byte |= bit;
	}
}

/**
 * right shift
 */
void shift_r(uint8_t *in_bytes, uint8_t shift_num, size_t in_length) {
	uint8_t *byte;
	for (byte = in_bytes + in_length - 1; in_length--; --byte) {
		uint8_t bit = 0;
		if (in_length) {
			bit = ((*(byte - 1)) & 0x01) ? 1 : 0;
		}
		*byte >>= 1;
		*byte |= bit << 7;
	}
}
