/*! Drone writer functions and structures  
 * \file drn_writer.h
 */ 
#ifndef __DRN_WRITER_H__
#define __DRN_WRITER_H__

#include "drn_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! Structure for storing a key during writing
 *  Contains associated chunk ids
 */
typedef struct {
    /*! Chunk ids associated to the key */
    drn_chunk_id_t * descriptors;
    /*! Key value */
    char * value;
} drn_writer_key_t;

/*! Structure for storing a map during writing
 *  Contains all possible keys and associated chunks
 */
typedef struct {
    /*! List of keys and associated chunk ids */
    drn_writer_key_t * keys;
    /*! Name of the map */
    char * name;
} drn_writer_map_t;

/*! Writer structure for storing chunk metadata
 * during cache writing
 */
typedef struct {
    /*! Fixed size cache description field */
    char description[256];
    /*! Map storage */
    drn_writer_map_t * maps;
    /*! Lost of chunk descriptors */
    drn_desc_t * descriptors;
    /*! Drone API version number */
    uint64_t version;
    /*! Number of stored chunks */
    uint64_t chunk_count;
    /*! Offset of the first chunk position in the file */
    uint64_t desc_start_offset;
    /*! Offset of the end of the last chunk in the file */
    uint64_t desc_end_offset;
    /*! File descriptor of the cache */
    int fd;
} drn_writer_t;

/*! Opens a cache file and prepare metadata for writing.
 *  Returns 0, or a negative number if an error has happened.
 */
int32_t drn_open_writer(drn_writer_t * cache, const char * filename, const char * description);

/*! Closes the cache file, consolidate and store meta data
 *  Returns 0, or a negative number if an error has happened.
 */
int32_t drn_close_writer(drn_writer_t *cache);

/*! Creates a map with name "map_name"
 *  Returns the id of the created map
 */
drn_map_id_t drn_writer_create_map(drn_writer_t *cache, const char * map_name);

/*! Retrieve the map id of the map named  "map_name"
 *  Returns the map id.
 */
drn_map_id_t drn_writer_get_map_id(drn_writer_t *cache, const char * map_name);

/*! Add a "size" bytes of a chunk "data" in the cache. Prepare chunk metadata
 *  Returns 0, or a negative number if an error has happened.
 */
int32_t drn_writer_add_chunk(drn_writer_t * cache, const void * data, uint64_t size);

/*! Updates the data of the chunk referenced by id "chunk_id". 
 * "size" has to match exactly the size of existing chunk
 *  Returns 0, or a negative number if an error has happened.
 */
int32_t drn_writer_update_chunk(drn_writer_t * cache, drn_chunk_id_t chunk_id, const void * data, uint64_t size);

/*! Map a chunk with multiple map/key pairs
 *  "map_count" is the number of map/key pairs
 *  "map_ids" is the list of map ids
 *  "key_values" is the list of keys
 *  Returns 0, or a negative number if an error has happened.
 */
int32_t drn_writer_map_chunk(drn_writer_t * cache,
                             drn_chunk_id_t chunk_id,
                             uint64_t map_count,
                             const drn_map_id_t * map_ids,
                             const char ** key_values);

/*! Returns the last chunk id written in the cache
 */
drn_chunk_id_t drn_writer_get_last_chunk_id(drn_writer_t * cache);

#ifdef __cplusplus
}
#endif

#endif /* __DRN_WRITER_H__ */

