/*
 * tea.cpp
 *
 *  Created on: Jul 1, 2013
 *      Author: xum
 */

#include "tea.h"

/*********** TEA ****************/
// 128-bit key
uint8_t KEY_TEA[16] = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA,
		0xB, 0xC, 0xD, 0xE, 0xF };
void encrypt_tea(uint32_t* v, uint32_t* k) {
	uint32_t v0 = v[0], v1 = v[1], sum = 0, i; /* set up */
	uint32_t delta = 0x9e3779b9; /* a key schedule constant */
	uint32_t k0 = k[0], k1 = k[1], k2 = k[2], k3 = k[3]; /* cache key */
	for (i = 0; i < 32; i++) { /* basic cycle start */
		sum += delta;
		v0 += ((v1 << 4) + k0) ^ (v1 + sum) ^ ((v1 >> 5) + k1);
		v1 += ((v0 << 4) + k2) ^ (v0 + sum) ^ ((v0 >> 5) + k3);
	} /* end cycle */
	v[0] = v0;
	v[1] = v1;
}

// add by Miao
// convert parameter from uint8_t to uint32_t
void encrypt_tea(uint8_t *v, uint8_t *k) {
	uint32_t vv[2], kk[4];

	// value
	vv[0] = ((uint32_t) v[3] << 24) + ((uint32_t) v[2] << 16)
			+ ((uint32_t) v[1] << 8) + v[0];
	vv[1] = ((uint32_t) v[7] << 24) + ((uint32_t) v[6] << 16)
			+ ((uint32_t) v[5] << 8) + v[4];

	// key
	kk[0] = ((uint32_t) k[3] << 24) + ((uint32_t) k[2] << 16)
			+ ((uint32_t) k[1] << 8) + k[0];
	kk[1] = ((uint32_t) k[7] << 24) + ((uint32_t) k[6] << 16)
			+ ((uint32_t) k[5] << 8) + k[4];
	kk[2] = ((uint32_t) k[11] << 24) + ((uint32_t) k[10] << 16)
			+ ((uint32_t) k[9] << 8) + k[8];
	kk[3] = ((uint32_t) k[15] << 24) + ((uint32_t) k[14] << 16)
			+ ((uint32_t) k[13] << 8) + k[12];

	encrypt_tea(vv, kk);
	v[0] = (uint8_t) (vv[0]);
	v[1] = (uint8_t) (vv[0] >> 8);
	v[2] = (uint8_t) (vv[0] >> 16);
	v[3] = (uint8_t) (vv[0] >> 24);
	v[4] = (uint8_t) (vv[1]);
	v[5] = (uint8_t) (vv[1] >> 8);
	v[6] = (uint8_t) (vv[1] >> 16);
	v[7] = (uint8_t) (vv[1] >> 24);
	return;
}

void decrypt_tea(uint32_t* v, uint32_t* k) {
	uint32_t v0 = v[0], v1 = v[1], sum = 0xC6EF3720, i; /* set up */
	uint32_t delta = 0x9e3779b9; /* a key schedule constant */
	uint32_t k0 = k[0], k1 = k[1], k2 = k[2], k3 = k[3]; /* cache key */
	for (i = 0; i < 32; i++) { /* basic cycle start */
		v1 -= ((v0 << 4) + k2) ^ (v0 + sum) ^ ((v0 >> 5) + k3);
		v0 -= ((v1 << 4) + k0) ^ (v1 + sum) ^ ((v1 >> 5) + k1);
		sum -= delta;
	} /* end cycle */
	v[0] = v0;
	v[1] = v1;
}

// add by Miao
// convert parameter from uint8_t to uint32_t
void decrypt_tea(uint8_t *v, uint8_t *k) {
	uint32_t vv[2], kk[4];

	// value
	vv[0] = ((uint32_t) v[3] << 24) + ((uint32_t) v[2] << 16)
			+ ((uint32_t) v[1] << 8) + v[0];
	vv[1] = ((uint32_t) v[7] << 24) + ((uint32_t) v[6] << 16)
			+ ((uint32_t) v[5] << 8) + v[4];

	// key
	kk[0] = ((uint32_t) k[3] << 24) + ((uint32_t) k[2] << 16)
			+ ((uint32_t) k[1] << 8) + k[0];
	kk[1] = ((uint32_t) k[7] << 24) + ((uint32_t) k[6] << 16)
			+ ((uint32_t) k[5] << 8) + k[4];
	kk[2] = ((uint32_t) k[11] << 24) + ((uint32_t) k[10] << 16)
			+ ((uint32_t) k[9] << 8) + k[8];
	kk[3] = ((uint32_t) k[15] << 24) + ((uint32_t) k[14] << 16)
			+ ((uint32_t) k[13] << 8) + k[12];

	decrypt_tea(vv, kk);
	v[0] = (uint8_t) (vv[0]);
	v[1] = (uint8_t) (vv[0] >> 8);
	v[2] = (uint8_t) (vv[0] >> 16);
	v[3] = (uint8_t) (vv[0] >> 24);
	v[4] = (uint8_t) (vv[1]);
	v[5] = (uint8_t) (vv[1] >> 8);
	v[6] = (uint8_t) (vv[1] >> 16);
	v[7] = (uint8_t) (vv[1] >> 24);
	return;
}
