/*! Drone reader functions an structures 
 * \file  drn_reader.h
 */ 
#ifndef __DRN_READER_H__
#define __DRN_READER_H__

#include "drn_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! Cache loading modes
 * DRN_READ_LOAD   : load everything in memory using a single read*
 * Can not use read_desc in this mode
 */
#define DRN_READ_LOAD 0
/*! Cache loading modes
 * DRN_READ_MMAP   : map the whole file using "mmap" (unix) or "MapViewOfFile" (windows)
 * Can not use read_desc in this mode
 */
#define DRN_READ_MMAP 1
/*! Cache loading modes
 * DRN_READ_NOLOAD : load only header and chunk metadata
 * Can not use get_desc in this mode
 */
#define DRN_READ_NOLOAD 2

/*! A map associates chunks to string keys
 *  A map has a name, a list of possible values,
 *  and a list of chunks referenced by the key
 *  Quick matching is done using a hash-map
 */
typedef struct {
    /*! Hash map */
    const drn_hash_cell_t * hash;
    /*! Map name */
    const char * name;
    /*! Number of chunks referenced by the key */
    uint64_t chunk_count;
    /*! Array of chunk descriptors referenced by the map */
    const drn_hash_desc_t * descriptors;
    /*! Array of possible key values separated by \0 char */
    const char * value_strings;
} drn_map_t;

/*! Cache struture for reading
 */
typedef struct {
    /*! Bootstrap header data */
    drn_header_container_t * header;
    /*! First chunk */
    void * data;
    /*! Array of chunk descriptors */
    drn_desc_t * descriptors;
    /*! Number of keys */
    uint64_t map_count;
    /*! Array of keys */
    drn_map_t * maps;
    /*! Size of the mapped region */
    uint64_t mmap_size;
    /*! Start pointer for the mapped file */
    void * mmap_start;
#ifdef _MSC_VER
	/*! win32 file handle for mmap mode */
	void * w_fhandle;
	/*! win32 memory map handle */
	void * w_mhandle;
#endif
    /*! Read mode */
    int32_t mode;
    /*! File descriptor */
    int32_t fd;
} drn_t;

/*! Open a cache file
 *  Load and prepare data/metadata according to the load policy
 *  Returns 0, or a negative number if an error has happened.
 */
int32_t      drn_open(drn_t * cache, const char * filename, int mode);
/*! Close a cache file
 *  Free data and metadata according to load policy
 *  Returns 0, or a negative number if an error has happened.
 */
int32_t      drn_close(drn_t *cache);
/*! Read drn API version */
uint64_t     drn_get_version(drn_t * cache);
/*! Read cache fixed size description  */
const char * drn_get_description(drn_t * cache);
/*! Retrieve the map id of the map named  "map_name"
 *  Returns the map id.
 */
drn_map_id_t  drn_get_map_id(drn_t *cache, const char * map_name);
/*! Get the number of keys in the cache */
uint64_t      drn_get_map_count(drn_t * cache);
/*! Get the name of the key associated to "map_id" */
const char *  drn_get_map_name(drn_t * cache, drn_map_id_t map_id);
/*! Get the number of stored chunks */
uint64_t      drn_get_chunk_count(drn_t * cache);
/*! Get the descriptor of the chunk referenced by "chunk_id" */
drn_desc_t    drn_get_desc(drn_t * cache, drn_chunk_id_t chunk_id);
/*! Get a pointer on the chunk referenced by "chunk_id"  MMAP and LOAD mode only*/
const void *  drn_get_chunk(drn_t * cache, drn_chunk_id_t chunk_id);
/*! Read the chunk referenced by "chunk_id" from the disk NOLOAD mode only
 *  The chunk is read directly into "data" no allocations are made
 *  Returns 0, or a negative number if an error has happened.
 */
int32_t       drn_read_chunk(drn_t * cache, drn_chunk_id_t chunk_id, void * data);
/*! Get the value of the key associated to the chunk in the map  "map_id"
 *  Returns associated value, or 0 if the chunk is not associated to the key
 */
const char * drn_get_desc_key_value(drn_t * cache, drn_chunk_id_t chunk_id, drn_map_id_t map_id);
/*! Count the number of chunk referenced by the given map/key pair
 *  Returns the number of matching chunks
 */
uint64_t     drn_count_matching_chunks(drn_t * cache,
                                       drn_map_id_t map_id, const char * key_value);
/*! Retrieve chunk ids referenced by the given map/key pair
 *  "max_chunk_count" is the maximum number of matching chunk returned
 *  Chunk ids are stored into "matching_chunks", no allocation is performed
 *  Returns the number of matching chunks
 */
uint64_t     drn_get_matching_chunks(drn_t * cache,
                                     drn_map_id_t map_id, const char * key_value,
                                     uint64_t max_chunk_count, drn_chunk_id_t * matching_chunks);
/*! Count number of chunk referenced by the union the given map/key pair list
 *  "map_count" is the number of maps in the union
 *  "map_ids" is the list of map ids in the union
 *  "key_values" is the list of key values in the union
 *  Returns the number of matching chunks
 */
uint64_t     drn_count_matching_chunks_union(drn_t * cache,
                                             uint64_t map_count, const drn_map_id_t * map_ids, const char ** key_values);
/*! Retrieve chunk ids referenced by the  key list union
 *  "map_count" is the number of maps in the union
 *  "map_ids" is the list of map ids in the union
 *  "key_values" is the list of key values in the union
 *  "max_chunk_count" is the maximum number of matching chunk returned
 *  Chunk ids are stored into "matching_chunks", no allocation is performed
 *  Returns the number of matching chunks
 */
uint64_t    drn_get_matching_chunks_union(drn_t * cache,
                                          uint64_t map_count, const drn_map_id_t * map_ids, const char ** key_values,
                                          uint64_t max_chunk_count, drn_chunk_id_t * matching_chunks);

#ifdef __cplusplus
}
#endif

#endif /* __DRN_READER_H__ */

