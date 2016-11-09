/*
 * cbc_mac.cpp
 *
 *  Created on: Jul 1, 2013
 *      Author: xum
 */

#include "cbc_mac.h"
#include "crypto.h"


//#ifdef KLEIN80
//#define BLOCK_SIZE (KLEIN80_BLOCK_SIZE)
//#endif

void cbc_mac(uint8_t *msg, uint8_t len, uint8_t *key, uint8_t *mac) {

	uint8_t blocks = len / BLOCK_SIZE;

	for (uint8_t i = 0; i < BLOCK_SIZE; i++) // IV
		mac[i] = 0;

	for (uint8_t i = 0; i < blocks; i++) {
		uint8_t input[BLOCK_SIZE] = { 0 };

		// xor
		for (uint8_t j = 0; j < BLOCK_SIZE; j++) {
			input[j] = mac[j] ^ (*msg);
			msg++;
		}

		klein80_encrypt_rounds(input, key, 1, mac);
	}
}

