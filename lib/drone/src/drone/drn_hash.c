#include "drn_hash.h"

uint32_t drn_oat_hash( void * key, uint32_t len )
{
    unsigned char *p = (unsigned char*) key;
    uint32_t h = 0;
    uint32_t i;
    for ( i = 0; i < len; i++ )
    {
        h += p[i];
        h += ( h << 10 );
        h ^= ( h >> 6 );
    }
    h += ( h << 3 );
    h ^= ( h >> 11 );
    h += ( h << 15 );
    return h;
}
