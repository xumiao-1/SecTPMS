/*
 * mydef.cpp
 *
 *  Created on: Jun 30, 2013
 *      Author: xum
 */

#include "mydef.h"
#include <cbc_mac.h>
#include <my_util.h>
#include <stdlib.h>

//uint8_t id_ecu[ID_ECU_LEN] = { 0xBE, 0xEF };
//uint8_t id_sensor[NUM_SENSORS][ID_SENSOR_LEN] = { { 0x04, 0xA9, 0xE3, 0xA1 } };
uint16_t id_ecu = 0xBEEF;
uint32_t id_sensor[NUM_SENSORS] = { 0x04A9E3A1 };

// long-lived key
uint8_t k_ll[NUM_SENSORS][KEY_LEN] = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
		0x8, 0x9 };
// authentication key
uint8_t k_a[NUM_SENSORS][KEY_LEN];
// session key, one for each sensor
uint8_t k_s[NUM_SENSORS][KEY_LEN];

// get next random #
void getNextR(uint8_t *r) {
	long rd = 0;
	uint8_t round = RAND_LEN / sizeof(long);
	uint8_t left = RAND_LEN % sizeof(long);

	for (uint8_t i = 0; i < round; i++) {
		rd = random();
		my_memcpy(r + i * sizeof(long), (uint8_t*) &rd, sizeof(long));
	}
	rd = random();
	my_memcpy(r + RAND_LEN - left, (uint8_t*) &rd, left);
}

// check if MAC correct
bool checkMAC(uint8_t *msg, uint8_t len, uint8_t *key, uint8_t *mac) {
	uint8_t my_mac[BLOCK_SIZE];
	cbc_mac(msg, len, key, my_mac);
	return my_memcmp(mac, my_mac, BLOCK_SIZE);
}

