/*
 * crypto.h
 *
 *  Created on: Jun 26, 2013
 *      Author: xum
 */

#ifndef CRYPTO_H_
#define CRYPTO_H_

#define KLEIN80
//#define KATAN32

#if defined(PRESENT) /* PRESENT */
#include "present.h"


#elif defined(TEA) /* TEA */
#include "tea.h"


#elif defined(KLEIN80) /* KLEIN */
#include "klein_80.h"
#define BLOCK_SIZE (KLEIN80_BLOCK_SIZE)


#elif defined(KATAN32) /* KATAN32 */
#include "katan.h"
#define BLOCK_SIZE (KATAN32_BLOCK_SIZE)

#else
#error "No block cipher defined!"

#endif

#endif /* CRYPTO_H_ */
