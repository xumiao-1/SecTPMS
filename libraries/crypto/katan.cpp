#include "katan.h"
#include <stdio.h>
#include <SPI.h>


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

/**
 * for efficiency, store key ahead
 */
static uint8_t k[508];

void katan32_encrypt_rounds(const uint8_t *plain, const uint8_t *key,
		uint16_t rounds, uint8_t *cipher);
void katan32_decrypt_rounds(const uint8_t *cipher, const uint8_t *key,
		uint16_t rounds, uint8_t *plain);


void initKey(uint8_t* key, uint8_t rounds)
{
    
    //uint16_t upper = 2 * rounds > 80 ? 2 * rounds : 80;	//1 + (2 * rounds - 1) / 8; // ceiling
	//uint8_t k[upper];
	for (uint16_t i = 0; i < 80; i++) {
		k[i] = key[i / 8] >> (i % 8);
	}
	for (uint16_t i = 80; i < 2*rounds; i++) {
		k[i] = k[i - 80] ^ k[i - 61] ^ k[i - 50] ^ k[i - 13];
	}

        for(int i = 0; i < 508; i++)
        Serial.print(k[i], HEX);
    Serial.println("\n");
}


// len: must be muliple times of block size
void encrypt_katan32(const uint8_t *plain, uint8_t len, uint16_t rounds, uint8_t *cipher)
{
	uint8_t run_time = len / KATAN32_BLOCK_SIZE;

	// sub-key for each round, only 1bit valid k[n](b0)
	//   i=0..253
	//   k[2i]: for L1
	//   k[2i+1]: for L2

//	for (uint8_t i = 0; i < run_time; i++) {
	while (run_time--) {
		katan32_encrypt_rounds((plain), (k), rounds, (cipher));
		plain += KATAN32_BLOCK_SIZE;
		cipher += KATAN32_BLOCK_SIZE;
	}
}

void decrypt_katan32(const uint8_t *cipher, uint8_t len, uint16_t rounds, uint8_t *plain)
{
        
    uint8_t run_time = len / KATAN32_BLOCK_SIZE;

    //	for (uint8_t i = 0; i < run_time; i++) {
    while (run_time--)
    {
        katan32_decrypt_rounds((cipher), (k), rounds, (plain));
        plain += KATAN32_BLOCK_SIZE;
        cipher += KATAN32_BLOCK_SIZE;
    }
}

// encryption
inline void katan32_encrypt_rounds(const uint8_t *plain, const uint8_t *k,
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
		L1[1] <<= 1; //mv high 1
		L1[1] |= (L1[0] >> 7); //get top bit from next down & add
		L1[0] <<= 1; //shift next down
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

void katan32_decrypt_rounds(const uint8_t *cipher, const uint8_t *k,
uint16_t rounds, uint8_t *plain)
{
    uint8_t L1[2];
    uint8_t L2[3];

    uint8_t fa, fb;

    // load cipher into L1 & L2,
    // L2 for lower bits,
    // L1 for higher bits
    L2[0] = cipher[0];	//0..7
    L2[1] = cipher[1];	//0..7
    L2[2] = cipher[2];	//0..2
    L1[0] = cipher[2] >> 3 | cipher[3] << 5; //0..7
    L1[1] = cipher[3] >> 3; //0..4

    uint16_t byte_offset = 2 * rounds - 2;

    while(rounds--) //Note: will run correct amount of times, or correct +- 1?
    {
        Serial.println(rounds, DEC);
        Serial.println(byte_offset, DEC);
        //pick up MSB's?
        
        fb = L1[0]; //Note: & 1?
        fa = L2[0];

        //shift L1 right
        L1[0] >>= 1; //mv low 1
        L1[0] |= (L1[1] << 7); //get bottom bit from higher block & add
        L1[1] >>= 1; //shift next block

        //shift L2 right
        L2[0] >>= 1;
        L2[0] |= (L2[1] << 7);
        L2[1] >>= 1;
        L2[1] |= (L2[2] << 7);
        L2[2] >>= 1;


        uint8_t temp = 0;

        //0, 7, 8, 5, 3
        temp = (fa ^ (L1[0] >> 7) ^ (L1[1] & (L1[0] >> 5)) ^ ((L1[0] >> 3) & (IR[rounds])) ^ 
            (k[byte_offset])) & 0x01;
            
        temp <<= 4;
        L1[1] |= temp;

        //7, 12, 10, 8, 3
        temp = (fb ^ (L2[0] >> 7) ^ ((L2[1] >> 4) & L2[1] >> 2) ^ (L2[1] & (L2[0] >> 3)) ^ 
            (k[byte_offset + 1])) & 0x01;
            
        temp <<= 2;
        L2[2] |= temp;
        
        byte_offset -= 2;
    }

    plain[0] = L2[0];
    plain[1] = L2[1];
    plain[2] = (L2[2] & 0x07) | (L1[0] << 3);
    plain[3] = (L1[0] >> 5) | (L1[1] << 3);
}
