#include "katan.h"
#include <stdio.h>

static void katan32_encrypt_rounds(const uint8_t *plain, const uint8_t *key,
		uint16_t rounds, uint8_t *cipher);
static void katan32_decrypt_rounds(const uint8_t *cipher, const uint8_t *key,
		uint16_t rounds, uint8_t *plain);
//static void katan32_encrypt_rounds(const uint32_t *plain, const uint8_t *key,
//		uint16_t rounds, uint32_t *cipher);
//static void katan32_decrypt_rounds(const uint8_t *cipher, const uint8_t *key,
//		uint16_t rounds, uint8_t *plain);

//// left shift 1-bit
//inline void shift_l(uint8_t *in_bytes, size_t in_length) {
//	uint8_t *byte;
//	uint8_t bit = 0;
//
//	for (byte = in_bytes + in_length - 1; in_length--; --byte) {
//		if (in_length) { // get the most significant bit of next byte
//			bit = (*(byte - 1)) >> 7;
//		}
//		*byte <<= 1;
//		*byte |= bit; // set the LS-bit using MS-bit of its next byte
//	}
//}

// right shift 1-bit
/*inline*/void shift_r(uint8_t *in_bytes, size_t in_length) {
	uint8_t *byte;
	uint8_t bit = 0;

	for (byte = in_bytes; in_length--; byte++) {
		if (in_length) //check for last run, to not go over
			bit = (*(byte + 1)) & 0x01; //get lsb of next byte
		*byte >>= 1; //shift current byte
		*byte |= (bit << 7); //set new msb of current byte
	}
}

// len: must be muliple times of block size
void encrypt_katan32(const uint8_t *plain, uint8_t len, const uint8_t *key,
		uint16_t rounds, uint8_t *cipher) {
	uint8_t run_time = len / KATAN32_BLOCK_SIZE;

	// sub-key for each round, only 1bit valid k[n](b0)
	//   i=0..253
	//   k[2i]: for L1
	//   k[2i+1]: for L2
	uint16_t upper = 2 * rounds > 80 ? 2 * rounds : 80;	//1 + (2 * rounds - 1) / 8; // ceiling
	uint8_t k[upper];
	for (uint16_t i = 0; i < 80; i++) {
		k[i] = key[i / 8] >> (i % 8);
	}
	for (uint16_t i = 80; i < upper; i++) {
		k[i] = k[i - 80] ^ k[i - 61] ^ k[i - 50] ^ k[i - 13];
	}

//	for (uint8_t i = 0; i < run_time; i++) {
	while (run_time--) {
		katan32_encrypt_rounds((plain), (k), rounds, (cipher));
		plain += KATAN32_BLOCK_SIZE;
		cipher += KATAN32_BLOCK_SIZE;
	}
}

void decrypt_katan32(const uint8_t *cipher, uint8_t len, const uint8_t *key,
		uint16_t rounds, uint8_t *plain) {
	uint8_t run_time = len / KATAN32_BLOCK_SIZE;

//	for (uint8_t i = 0; i < run_time; i++) {
	while (run_time--) {
		katan32_decrypt_rounds((cipher), (key), rounds, (plain));
		plain += KATAN32_BLOCK_SIZE;
		cipher += KATAN32_BLOCK_SIZE;
	}
}

//// from the paper
//const uint8_t IR[32] = { 0b01111111, 0b10101100, 0b01111010, 0b00110011,
//		0b00100101, 0b01100010, 0b00111100, 0b10000100, 0b10000010, 0b11001111,
//		0b10101111, 0b10101000, 0b00001100, 0b01110011, 0b11011111, 0b10100101,
//		0b10010110, 0b10110011, 0b11010001, 0b11101101, 0b01101001, 0b11101011,
//		0b00100100, 0b10001011, 0b00100011, 0b00101111, 0b01011100, 0b10000011,
//		0b00100110, 0b10110000, 0b00000011, 0b00010010 };
// IR constants, either 1 for all slices, are 0 for all slices
#define ONES (1)
const uint8_t IR[254] = { ONES, ONES, ONES, ONES, ONES, ONES, ONES, 0, 0,
		0, // 0-9
		ONES, ONES, 0, ONES, 0, ONES, 0, ONES, 0, ONES, ONES, ONES, ONES, 0,
		ONES, ONES, 0, 0, ONES, ONES, 0, 0, ONES, 0, ONES, 0, 0, ONES, 0, 0, 0,
		ONES, 0, 0, 0, ONES, ONES, 0, 0, 0, ONES, ONES, ONES, ONES, 0, 0, 0, 0,
		ONES, 0, 0, 0, 0, ONES, 0, ONES, 0, 0, 0,
		0, // 60-69
		0, ONES, ONES, ONES, ONES, ONES, 0, 0, ONES, ONES, ONES, ONES, ONES,
		ONES, 0, ONES, 0, ONES, 0, 0, 0, ONES, 0, ONES, 0, ONES, 0, 0, ONES,
		ONES, 0, 0, 0, 0, ONES, ONES, 0, 0, ONES, ONES, ONES, 0, ONES, ONES,
		ONES, ONES, ONES, 0, ONES, ONES, ONES, 0, ONES, 0, 0, ONES, 0, ONES, 0,
		ONES, // 120-129
		ONES, 0, ONES, 0, 0, ONES, ONES, ONES, 0, 0, ONES, ONES, 0, ONES, ONES,
		0, 0, 0, ONES, 0, ONES, ONES, ONES, 0, ONES, ONES, 0, ONES, ONES, ONES,
		ONES, 0, 0, ONES, 0, ONES, ONES, 0, ONES, ONES, 0, ONES, 0, ONES, ONES,
		ONES, 0, 0, ONES, 0, 0, ONES, 0, 0, ONES, ONES, 0, ONES, 0,
		0, // 180-189
		0, ONES, ONES, ONES, 0, 0, 0, ONES, 0, 0, ONES, ONES, ONES, ONES, 0,
		ONES, 0, 0, 0, 0, ONES, ONES, ONES, 0, ONES, 0, ONES, ONES, 0, 0, 0, 0,
		0, ONES, 0, ONES, ONES, 0, 0, ONES, 0, 0, 0, 0, 0, 0, ONES, ONES, 0,
		ONES, ONES, ONES, 0, 0, 0, 0, 0, 0, 0, ONES, // 240-249
		0, 0, ONES, 0, };

//// encryption
//void katan32_encrypt_rounds(const uint32_t *plain, const uint8_t *key,
//		uint16_t rounds, uint32_t *cipher) {
//	// L1 - 13bit valid: L1[0](b0..b7), L1[1](b0..b4)
//	uint32_t L1 = 0;
//	// L2 - 19bit valid: L2[0](b0..b7), L2[1](b0..b7), L2[2](b0..b2)
//	uint32_t L2 = 0;
//	// sub-key for each round, only 1bit valid k[n](b0)
//	//   i=0..253
//	//   k[2i]: for L1
//	//   k[2i+1]: for L2
//	uint16_t upper = 1 + (2 * rounds - 1) / 8; // ceiling
//	uint8_t k[upper];
//
//	// carrier: fa, fb, only 1bit valid
//	uint8_t fa, fb;
//
//	uint16_t i;
//
//	// load plain into L1 & L2,
//	// L2 for lower bits,
//	// L1 for higher bits
////	L2[0] = plain[0];	//0..7
////	L2[1] = plain[1];	//0..7
////	L2[2] = plain[2];	//0..2
////	L1[0] = plain[2] >> 3 | plain[3] << 5; //0..7
////	L1[1] = plain[3] >> 3; //0..4
//	L2 = *plain;
//	L1 = (*plain) >> 19;
//
//	// load sub-key
//	for (i = 0; i < 10; i++) {
//		k[i] = key[i];
//	}
//	for (i = 10; i < (int16_t) upper; i++) {
//		k[i] = k[i - 10] ^ (k[i - 8] >> 3 | k[i - 7] << 5)
//				^ (k[i - 7] >> 6 | k[i - 6] << 2)
//				^ (k[i - 2] >> 3 | k[i - 1] << 5);
//	}
//
//	// round by round
//	uint16_t byte_offset;
//	uint16_t bit_offset;
//	for (i = 0; i < rounds; i++) {
//		byte_offset = (2 * i) / 8;
//		bit_offset = (2 * i) % 8;
//
//		// L1 bit: 12, 7, 8, 5, 3
//		fa = ((L1 >> 12) ^ (L1 >> 7) ^ (L1 >> 8 & (L1 >> 5))
//				^ ((L1 >> 3) & (IR[i / 8] >> (i % 8)))
//				^ (k[byte_offset] >> bit_offset)) & 0x01;
//		// L2 bit: 18, 7, 12, 10, 8, 3
//		fb =
//				((L2 >> 18) ^ (L2 >> 7) ^ ((L2 >> 12) & (L2 >> 10))
//						^ ((L2 >> 8) & (L2 >> 3))
//						^ (k[byte_offset] >> (bit_offset + 1))) & 0x01;
//
//		// left shift
////		shift_l(L1, sizeof(L1));
//		L1 <<= 1;
////		shift_l(L2, sizeof(L2));
//		L2 <<= 1;
//
//		// carrier
////		if (fb) {
////			L1[0] |= 0x01; // set last bit to 1
////		} else {
////			L1[0] &= 0xFE; // set last bit to 0
////		}
//		L1 |= fb;
////		if (fa) {
////			L2[0] |= 0x01;
////		} else {
////			L2[0] &= 0xFE;
////		}
//		L2 |= fa;
//
//	}
//
//	// output cipher
////	cipher[0] = L2[0];
////	cipher[1] = L2[1];
////	cipher[2] = (L2[2] & 0x07) | (L1[0] << 3);
////	cipher[3] = (L1[0] >> 5) | (L1[1] << 3);
//	*cipher = (L2 & 0x0007FFFF) | L1 << 19;
//}

// encryption
inline void katan32_encrypt_rounds(const uint8_t *plain, const uint8_t *k/*ey*/,
		uint16_t rounds, uint8_t *cipher) {
	// L1 - 13bit valid: L1[0](b0..b7), L1[1](b0..b4)
	uint8_t L1[2] = { 0 };
	// L2 - 19bit valid: L2[0](b0..b7), L2[1](b0..b7), L2[2](b0..b2)
	uint8_t L2[3] = { 0 };
//	// sub-key for each round, only 1bit valid k[n](b0)
//	//   i=0..253
//	//   k[2i]: for L1
//	//   k[2i+1]: for L2
//	uint16_t upper = 2 * rounds > 80 ? 2 * rounds : 80;	//1 + (2 * rounds - 1) / 8; // ceiling
//	uint8_t k[upper];

	// carrier: fa, fb, only 1bit valid
	uint8_t fa, fb;

	// load plain into L1 & L2,
	// L2 for lower bits,
	// L1 for higher bits
	L2[0] = plain[0];	//0..7
	L2[1] = plain[1];	//0..7
	L2[2] = plain[2];	//0..2
	L1[0] = plain[2] >> 3 | plain[3] << 5; //0..7
	L1[1] = plain[3] >> 3; //0..4

	// load sub-key
//	for (i = 0; i < 10; i++) {
//		k[i] = key[i];
//	}
//	for (i = 10; i < (int16_t) upper; i++) {
//		k[i] = k[i - 10] ^ (k[i - 8] >> 3 | k[i - 7] << 5)
//				^ (k[i - 7] >> 6 | k[i - 6] << 2)
//				^ (k[i - 2] >> 3 | k[i - 1] << 5);
//	}
//	for (i = 0; i < 80; i++) {
//		k[i] = key[i / 8] >> (i % 8);
//	}
//	for (i = 80; i < upper; i++) {
//		k[i] = k[i - 80] ^ k[i - 61] ^ k[i - 50] ^ k[i - 13];
//	}

	// round by round
	uint16_t byte_offset = 0;
	uint16_t i = 0;
//	uint16_t bit_offset;
//	for (i = 0; i < rounds; i++) {
	while (rounds--) {
//		byte_offset = (2 * i) / 8;
//		bit_offset = (2 * i) % 8;
//		byte_offset = 2 * i;

		// L1 bit: 12, 7, 8, 5, 3
//		fa = ((L1[1] >> 4) ^ (L1[0] >> 7) ^ (L1[1] & (L1[0] >> 5))
//				^ ((L1[0] >> 3) & (IR[i / 8] >> (i % 8)))
//				^ (k[byte_offset] >> bit_offset)) & 0x01;
//		// L2 bit: 18, 7, 12, 10, 8, 3
//		fb = ((L2[2] >> 2) ^ (L2[0] >> 7) ^ ((L2[1] >> 4) & (L2[1] >> 2))
//				^ ((L2[1]) & (L2[0] >> 3))
//				^ (k[byte_offset] >> (bit_offset + 1))) & 0x01;
		fa = ((L1[1] >> 4) ^ (L1[0] >> 7) ^ (L1[1] & (L1[0] >> 5))
				^ ((L1[0] >> 3) & (IR[i++])) ^ (k[byte_offset++])) & 0x01;
		fb = ((L2[2] >> 2) ^ (L2[0] >> 7) ^ ((L2[1] >> 4) & (L2[1] >> 2))
				^ ((L2[1]) & (L2[0] >> 3)) ^ (k[byte_offset++])) & 0x01;

		// left shift
//		shift_l(L1, sizeof(L1));
		L1[1] <<= 1;
		L1[1] |= (L1[0] >> 7);
		L1[0] <<= 1;
//		shift_l(L2, sizeof(L2));
		L2[2] <<= 1;
		L2[2] |= (L2[1] >> 7);
		L2[1] <<= 1;
		L2[1] |= (L2[0] >> 7);
		L2[0] <<= 1;

		// carrier
//		if (fb) {
//			L1[0] |= 0x01; // set last bit to 1
//		} else {
//			L1[0] &= 0xFE; // set last bit to 0
//		}
		L1[0] |= fb;
//		if (fa) {
//			L2[0] |= 0x01;
//		} else {
//			L2[0] &= 0xFE;
//		}
		L2[0] |= fa;

	}

	// output cipher
	cipher[0] = L2[0];
	cipher[1] = L2[1];
	cipher[2] = (L2[2] & 0x07) | (L1[0] << 3);
	cipher[3] = (L1[0] >> 5) | (L1[1] << 3);
}

// decryption
/*inline*/void katan32_decrypt_rounds(const uint8_t *cipher, const uint8_t *key,
		uint16_t rounds, uint8_t *plain) {
	uint8_t L1[2];
	uint8_t L2[3];

	uint16_t upper = 1 + (2 * rounds - 1) / 8;
	uint8_t k[upper];

	uint8_t fa, fb;
	int16_t i;

// load cipher into L1 & L2,
// L2 for lower bits,
// L1 for higher bits
	L2[0] = cipher[0];	//0..7
	L2[1] = cipher[1];	//0..7
	L2[2] = cipher[2];	//0..2
	L1[0] = cipher[2] >> 3 | cipher[3] << 5; //0..7
	L1[1] = cipher[3] >> 3; //0..4

// load sub-key
	for (i = 0; i < 10; i++) {
		k[i] = key[i];
	}
	for (i = 10; i < (int16_t) upper; i++) {
		k[i] = k[i - 10] ^ (k[i - 8] >> 3 | k[i - 7] << 5)
				^ (k[i - 7] >> 6 | k[i - 6] << 2)
				^ (k[i - 2] >> 3 | k[i - 1] << 5);
	}

	uint16_t byte_offset;
	uint16_t bit_offset;
	for (i = rounds - 1; i >= 0; i--) {
		byte_offset = (2 * i) / 8;
		bit_offset = (2 * i) % 8;
		fb = L1[0];
		fa = L2[0];

		shift_r(L1, sizeof(L1));
		shift_r(L2, sizeof(L2));

		uint8_t temp = 0;

		//0, 7, 8, 5, 3
		temp = (fa ^ (L1[0] >> 7) ^ (L1[1] & (L1[0] >> 5))
				^ ((L1[0] >> 3) & (IR[i])) ^ (k[byte_offset] >> (bit_offset)))
				& 0x01;

		if (temp)
			L1[1] |= 0x10; //0001 0000 : 4th bit (12th total)
		else
			L1[1] &= 0xEF; //set 4th bit to 0

		//7, 12, 10, 8, 3
		temp = (fb ^ (L2[0] >> 7) ^ ((L2[1] >> 4) & L2[1] >> 2)
				^ (L2[1] & (L2[0] >> 3)) ^ (k[byte_offset] >> (bit_offset + 1)))
				& 0x01;

		if (temp)
			L2[2] |= 0x04; //18th bit
		else
			L2[2] &= 0xFB; //set 18th bit to zero
	}

	plain[0] = L2[0];
	plain[1] = L2[1];
	plain[2] = (L2[2] & 0x07) | (L1[0] << 3);
	plain[3] = (L1[0] >> 5) | (L1[1] << 3);
}
