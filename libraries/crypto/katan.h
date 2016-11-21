/**
 * author: miao
 * file: katan.h
 * header file for katan
 * start with katan32
 */

#ifndef _KATAN_H_
#define _KATAN_H_

#include <stdint.h>

typedef uint64_t u64;

// 32bit block
#define KATAN32_BLOCK_SIZE (4)

/*
 * parameters:
 *   plain: must be 32bit (i.e., 4byte)
 *   cipher: same with plain
 *   key: must be 80bit (i.e., 10byte)
 *   rounds: 0 ~ 254
 *     (254 rounds by default)
 */
//#define encrypt_katan32(plain, len, key, cipher) katan32_encrypt((plain), (key), 254, (cipher))
//#define decrypt_katan32(cipher, len, key, plain) katan32_decrypt((cipher), (key), 254, (plain))
void encrypt_katan32(const uint8_t *plain, uint8_t len, const uint8_t *key,
		uint16_t rounds, uint8_t *cipher);
void decrypt_katan32(const uint8_t *cipher, uint8_t len, const uint8_t *key,
		uint16_t rounds, uint8_t *plain);

//static void katan32_encrypt_rounds(const uint8_t *plain, const uint8_t *key,
//		uint16_t rounds, uint8_t *cipher);
//static void katan32_decrypt_rounds(const uint8_t *cipher, const uint8_t *key,
//		uint16_t rounds, uint8_t *plain);

#endif //_KATAN_H_
