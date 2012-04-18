/*! Common Drone types 
 *  \file  drn_types.h 
 */ 
#ifndef __DRN_TYPES_H__
#define __DRN_TYPES_H__

#define DRN_HASH_MAP_SIZE 65000
#define DRN_WRITER_VERSION 1

#ifdef _MSC_VER
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*! Id of a chunk in the chunk index */
typedef uint64_t drn_chunk_id_t;
/*! Id of a map in the map list */
typedef uint64_t drn_map_id_t;

/*! Descriptor of a chunk
 *  Contains the offset in the file and the size of a chunk
 */
typedef struct {
    uint64_t offset;
    uint64_t size;
} drn_desc_t;

/*! Hash map chunk descriptor
 *  Contains the id of the referenced chunk and
 *  the offset in the key value array
 */
typedef struct {
    drn_chunk_id_t chunk_id;
    uint64_t key_value_offset;
} drn_hash_desc_t;

/*! Hash map cell
 *  Contains the number of chunks referenced
 *  and the offset in the descriptor array associated
 *  to the map
 */
typedef struct {
    uint32_t offset;
    uint32_t count;
} drn_hash_cell_t;

/*! Container for map data
 *  Contains chunk ids of the arrays
 *  used by the map
 */
typedef struct {
    /*! Array of drn_hash_cell_t */
    drn_chunk_id_t hash_chunk_id;
    /*! Name of the map */
    drn_chunk_id_t name_chunk_id;
    /*! Array of drn_hash_desc_t */
    drn_chunk_id_t descriptors_chunk_id;
    /*! String containing all key values separated by \0 */
    drn_chunk_id_t value_strings_chunk_id;
} drn_map_container_t;

/*! Container for header data
 */
typedef struct {
    char description[256];
    uint64_t version;
    uint64_t index_offset;
    uint64_t chunk_count;
    /*! Array of drn_map_container_t */
    drn_chunk_id_t maps_chunk_id;
} drn_header_container_t;

#ifdef __cplusplus
}
#endif

#endif /* __DRN_TYPES_H__ */
