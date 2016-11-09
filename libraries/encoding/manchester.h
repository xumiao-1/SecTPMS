/**
 Copyright (C) 2012 - Frank de Brabander

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <stddef.h>

void set_bit(uint8_t *bytes, uint32_t position, uint8_t bit_value);

/* Decode a byte with differential Manchester encoding, this byte
 * contains only 4 actual data bits
 *
 * @param in_bytes      Byte array data encoded with differential Manchester.
 * @param out_bytes     Output buffer, must be at least half the size of in_bytes.
 * @param in_length     Number of input bytes.
 *
 * returns              Zero on success, one if decoding detected problems.
 */
uint8_t decode_dmanchester(uint8_t *in_bytes, uint8_t *out_bytes,
		size_t in_length, uint8_t previous);

///* Decode a byte with Biphase Mark Code (BMC), this byte contains
// * only 4 actual data bits.
// *
// * @param byte      Byte encoded with BMC.
// * @returns         Decoded value, will always be the lower 4 bits
// *                  only. This means a number in the range 0x0 - 0xF
// */
//uint8_t decode_bmc(uint8_t byte);

//uint8_t encode_dmanchester(uint8_t *in_bytes, uint8_t *out_bytes, size_t in_length);
uint8_t encode_dmanchester(uint8_t *in_bytes, uint8_t *out_bytes,
		size_t in_length, uint8_t previous);

/* Manchester decoding.
 * in_length -> in_length/2
 *
 */
uint8_t decode_manchester(uint8_t *in_bytes, uint8_t *out_bytes,
		size_t in_length, uint8_t inverted);

/* Manchester encoding.
 * in_length -> 2 * in_length
 *
 */
uint8_t encode_manchester(uint8_t *in_bytes, uint8_t *out_bytes,
		size_t in_length, uint8_t inverted);

/* left shift */
void shift_l(uint8_t *in_bytes, uint8_t shift_num, size_t in_length);
/* right shift */
void shift_r(uint8_t *in_bytes, uint8_t shift_num, size_t in_length);

