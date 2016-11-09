/*
 * crypto.h
 *
 *  Created on: Jun 26, 2013
 *      Author: xum
 */

#ifndef CRYPTO_H_
#define CRYPTO_H_

#define KLEIN80

#if defined(PRESENT)
// PRESENT related
#include "present.h"
#elif defined(TEA)
// TEA related
#include "tea.h"
#elif defined(KLEIN80)
// KLEIN related
#include "klein_80.h"
#define BLOCK_SIZE (KLEIN80_BLOCK_SIZE)
#else
#endif

#endif /* CRYPTO_H_ */
