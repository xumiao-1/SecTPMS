/*
 * mydef.h
 *
 *  Created on: Jun 30, 2013
 *      Author: xum
 */

#ifndef MYDEF_H_
#define MYDEF_H_

#include <stdint.h>
#include <crypto.h>

#define CRYPTO

// add by Miao
// macro
#define NUM_SENSORS (1)
#define ID_ECU_LEN (2) // assume 16bit long
#define ID_SENSOR_LEN (4) // 28bit in real, but use 32 for simplicity
#define KEY_LEN (10) // 80bit
#define RAND_LEN (8) // 64bit
#define MAX_MSG_LEN (32) // 256bit
//extern uint8_t id_ecu[ID_ECU_LEN];
//extern uint8_t id_sensor[NUM_SENSORS][ID_SENSOR_LEN];
extern uint16_t id_ecu;
extern uint32_t id_sensor[NUM_SENSORS];

extern uint8_t k_ll[NUM_SENSORS][KEY_LEN]; // long-lived key
extern uint8_t k_a[NUM_SENSORS][KEY_LEN]; // authentication key
extern uint8_t k_s[NUM_SENSORS][KEY_LEN]; // session key, one for each sensor

struct {
	// 24-byte data part
	uint8_t P[RAND_LEN]; // for padding purpose
	uint8_t R[RAND_LEN];
	uint16_t id_ecu;
	uint16_t reserved;
	uint32_t id_sensor;
	// 8-byte mac part
	uint8_t mac[BLOCK_SIZE]; // cipher block size: 8
} MSG1;

struct {
	uint8_t S[RAND_LEN]; // for padding purpose
	uint8_t R[RAND_LEN];
	uint16_t id_ecu;
	uint16_t reserved;
	uint32_t id_sensor;

	uint8_t mac[BLOCK_SIZE]; // cipher block size: 8
} MSG2;

struct {
	uint8_t R[RAND_LEN];
	uint8_t S[RAND_LEN];
	uint32_t id_sensor;
	uint16_t reserved;
	uint16_t id_ecu;

	uint8_t mac[BLOCK_SIZE]; // cipher block size: 8
} MSG3;

// function prototype
// they are used by both ECU and sensors
void getNextR(uint8_t *r);
bool checkMAC(uint8_t *msg, uint8_t len, uint8_t *key, uint8_t *mac);

#endif /* MYDEF_H_ */
