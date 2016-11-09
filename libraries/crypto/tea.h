/*
 * tea.h
 *
 *  Created on: Jun 26, 2013
 *      Author: xum
 */

#ifndef TEA_H_
#define TEA_H_

#include <stdint.h>

extern uint8_t KEY_TEA[];

// modified by Miao
// for 8-bit processor
void encrypt_tea(uint8_t* v, uint8_t* k);
void decrypt_tea(uint8_t* v, uint8_t* k);
//void encrypt_tea (uint32_t* v, uint32_t* k);
//void decrypt_tea (uint32_t* v, uint32_t* k);

#endif /* TEA_H_ */
