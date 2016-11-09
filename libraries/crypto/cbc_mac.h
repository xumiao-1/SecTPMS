/*
 * cbc_mac.h
 *
 *  Created on: Jul 1, 2013
 *      Author: xum
 */

#ifndef CBC_MAC_H_
#define CBC_MAC_H_

#include <stdint.h>

//typedef enum {
//	NOCIPHER = 0, KATAN64, KLEIN80, TEA, PRESENT
//} BlockCipherChoice;

// len must be multiple times of block size
void cbc_mac(uint8_t *msg, uint8_t len, uint8_t *key, uint8_t *mac);

#endif /* CBC_MAC_H_ */
