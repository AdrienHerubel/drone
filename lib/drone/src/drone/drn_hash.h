/*! Hash functions for map indexing  
 * \file drn_hash.h
 */ 
#ifndef __DRN_HASH_H__
#define __DRN_HASH_H__

#include "drn_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! Jenkins One-At-a-Time hash function
 *  http://en.wikipedia.org/wiki/Jenkins_hash_function
 */
uint32_t drn_oat_hash( void * key, uint32_t len );

#ifdef __cplusplus
}
#endif

#endif /* __DRN_HASH_H__ */
