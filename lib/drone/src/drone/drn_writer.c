#ifndef _MSC_VER /* Force 64 bits lseek offsets */
#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64
#endif

#include <stdlib.h>
#include <string.h>
#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif /*_MSC_VER */
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <assert.h>

#include "drn_writer.h"
#include "drn_hash.h"

#define ALLOCATE(SIZE) malloc(SIZE)
#define FREE(PTR) free(PTR)
#define RESTRICT __restrict__

#ifdef _MSC_VER
#pragma warning(disable : 4996) /* Disable deprecated warning on posix functions */
#define CREAT_MODE _S_IWRITE
#define OPEN_MODE O_WRONLY | O_CREAT | O_TRUNC | _O_BINARY
#define LSEEK _lseeki64
#define CLOSE _close
#define OPEN _open
#define READ _read
#define WRITE _write
#define WRITE_COUNT_CAST (int)
#else
#define CREAT_MODE S_IRWXU
#define OPEN_MODE O_WRONLY | O_CREAT | O_TRUNC
#define LSEEK lseek64
#define CLOSE close
#define OPEN open
#define READ read
#define WRITE write
#define WRITE_COUNT_CAST (size_t)
#endif /*_MSC_VER */

/* stretchy buffer  init: NULL  free: sbfree()  push_back: sbpush()  size: sbcount() */
#define sbfree(a)         ((a) ? FREE(stb__sbraw(a)),0 : 0)
#define sbpush(a,v)       (stb__sbmaybegrow(a,1), (a)[stb__sbn(a)++] = (v))
#define sbcount(a)        ((a) ? stb__sbn(a) : 0)
#define sbadd(a,n)        (stb__sbmaybegrow(a,n), stb__sbn(a)+=(n), &(a)[stb__sbn(a)-(n)])
#define sblast(a)         ((a)[stb__sbn(a)-1])
#define stb__sbraw(a) ((int *) (a) - 2)
#define stb__sbm(a)   stb__sbraw(a)[0]
#define stb__sbn(a)   stb__sbraw(a)[1]
#define stb__sbneedgrow(a,n)  ((a)==0 || stb__sbn(a)+n >= stb__sbm(a))
#define stb__sbmaybegrow(a,n) (stb__sbneedgrow(a,(n)) ? stb__sbgrow(a,n), 0 : 0)
#define stb__sbgrow(a,n)  stb__sbgrowf((void **) &(a), (n), sizeof(*(a)))
static void stb__sbgrowf(void **arr, int increment, int itemsize)
{
    int m = *arr ? 2*stb__sbm(*arr)+increment : increment+1;
    void *p = realloc(*arr ? stb__sbraw(*arr) : 0, itemsize * m + sizeof(int)*2);
    assert(p);
    if (p) {
        if (!*arr) ((int *) p)[1] = 0;
        *arr = (void *) ((int *) p + 2);
        stb__sbm(*arr) = m;
    }
}

int32_t find_map(const drn_writer_map_t * array, const char * name)
{
    int32_t idx;
    for (idx = 0; idx < sbcount(array); ++idx)
        if (!strcmp(array[idx].name, name))
            return idx;
    return -1;
}

int32_t find_key(const drn_writer_key_t * array, const char * value)
{
    int32_t idx;
    for (idx = 0; idx < sbcount(array); ++idx)
        if (!strcmp(array[idx].value, value))
            return idx;
    return -1;
}

int32_t drn_val_hash_cmp(const void * valA, const void * valB)
{
    drn_writer_key_t * v1 = (drn_writer_key_t * ) valA;
    drn_writer_key_t * v2 = (drn_writer_key_t * ) valB;
    uint32_t h1 = drn_oat_hash(v1->value, (uint32_t) strlen(v1->value))%DRN_HASH_MAP_SIZE;
    uint32_t h2 = drn_oat_hash(v2->value, (uint32_t) strlen(v2->value))%DRN_HASH_MAP_SIZE;
    if (h1 > h2) return 1;
    if (h1 < h2) return -1;
    return 0;
}

int32_t drn_uint64_cmp(const void * valA, const void * valB)
{
    uint64_t v1 = *(uint64_t * ) valA;
    uint64_t v2 = *(uint64_t * ) valB;
    if (v1 > v2) return 1;
    if (v1 < v2) return -1;
    return 0;
}

int32_t drn_open_writer(drn_writer_t * cache, const char * filename, const char * description)
{
    size_t length;

    length = strlen(description);
    if (length > 256)
        return -2;
    cache->fd = OPEN(filename, OPEN_MODE, CREAT_MODE);
    if (cache->fd <= 0)
        return -1;
    memset(cache->description, 0, 256);
    memcpy(cache->description, description, length);
    cache->maps = 0;
    cache->descriptors = 0;
    cache->version = DRN_WRITER_VERSION;
    cache->chunk_count = 0;
    cache->desc_start_offset = sizeof(drn_header_container_t);
    cache->desc_end_offset = 0;
    return 0;
}

int32_t drn_close_writer(drn_writer_t *cache)
{
    drn_header_container_t header;
    drn_map_container_t * maps = (drn_map_container_t *) ALLOCATE(sbcount(cache->maps) * sizeof(drn_map_container_t));
    uint64_t m_idx;
    uint64_t i;
    size_t bytes;

    for (m_idx=0; m_idx < sbcount(cache->maps); ++m_idx)
    {
        drn_writer_map_t * map = cache->maps + m_idx;
        drn_hash_cell_t * hash_map = (drn_hash_cell_t *) ALLOCATE(DRN_HASH_MAP_SIZE * sizeof (drn_hash_cell_t));
        uint64_t hc_idx;
        uint64_t chunk_count = 0;
        uint64_t values_size = 0;
        drn_writer_key_t * key_array = map->keys;
        uint64_t k_idx;
        drn_hash_desc_t * descriptors;
        char * values_string_array;
        uint32_t desc_offset_cntr;
        uint32_t value_strings_offset_cntr;

        for (hc_idx = 0; hc_idx < DRN_HASH_MAP_SIZE; ++hc_idx)
        {
            hash_map[hc_idx].offset = 0;
            hash_map[hc_idx].count = 0;
        }
        /* count Es + sum VV lengths */
        for (k_idx = 0; k_idx < sbcount(map->keys); ++k_idx)
        {
            drn_writer_key_t * value = map->keys + k_idx;
            chunk_count += sbcount(value->descriptors);
            values_size += strlen(value->value)+1;
        }
        /* Allocate Es */
        descriptors = (drn_hash_desc_t*) ALLOCATE((size_t) chunk_count * sizeof(drn_hash_desc_t));
        /* Allocate Vv */
        values_string_array = (char *) ALLOCATE((size_t) values_size * sizeof(char));
        /* Sort Vs */
        qsort(key_array, sbcount(map->keys), sizeof(drn_writer_key_t), drn_val_hash_cmp);
        /* For each V */
        desc_offset_cntr = 0;
        value_strings_offset_cntr = 0;
        for (k_idx = 0; k_idx < sbcount(map->keys); ++k_idx)
        {
            uint32_t hash = drn_oat_hash(key_array[k_idx].value, (uint32_t) strlen(key_array[k_idx].value))%DRN_HASH_MAP_SIZE;
            uint64_t * chunk_ids = key_array[k_idx].descriptors;
            uint64_t d_idx;

            /* Fill H */
            hash_map[hash].offset = desc_offset_cntr;
            hash_map[hash].count = sbcount(key_array[k_idx].descriptors);
            /* Fill Vv */
            memcpy(values_string_array + value_strings_offset_cntr, key_array[k_idx].value, strlen(key_array[k_idx].value)+1);
            /*  fill E */
            qsort(chunk_ids, sbcount(key_array[k_idx].descriptors), sizeof(uint64_t), drn_uint64_cmp);
            for (d_idx = 0; d_idx < sbcount(key_array[k_idx].descriptors); ++d_idx)
            {
                descriptors[desc_offset_cntr].chunk_id = chunk_ids[d_idx];
                descriptors[desc_offset_cntr].key_value_offset = value_strings_offset_cntr;
                ++desc_offset_cntr;
            }
            /* Increment offsets */
            value_strings_offset_cntr += (uint32_t) strlen(key_array[k_idx].value)+1;
        }
        /*  Write hash array as desc */
        drn_writer_add_chunk(cache, hash_map, sizeof(drn_hash_cell_t) * DRN_HASH_MAP_SIZE);
        maps[m_idx].hash_chunk_id = drn_writer_get_last_chunk_id(cache);
        /*  Write hash name as desc */
        drn_writer_add_chunk(cache, map->name, strlen(map->name) + 1);
        maps[m_idx].name_chunk_id = drn_writer_get_last_chunk_id(cache);
        /*  Write value string array as desc */
        drn_writer_add_chunk(cache, descriptors, sizeof(drn_hash_desc_t) * chunk_count);
        maps[m_idx].descriptors_chunk_id = drn_writer_get_last_chunk_id(cache);
        /*  Write desc array as desc */
        drn_writer_add_chunk(cache, values_string_array, values_size);
        maps[m_idx].value_strings_chunk_id = drn_writer_get_last_chunk_id(cache);
        FREE(hash_map);
        FREE(descriptors);
        FREE(values_string_array);
    }
    drn_writer_add_chunk(cache, maps, sizeof(drn_map_container_t) * sbcount(cache->maps));
    FREE(maps);
    header.version = cache->version;
    memcpy(header.description, cache->description, 256);
    header.index_offset = cache->desc_start_offset + cache->desc_end_offset;
    header.chunk_count = cache->chunk_count;
    header.maps_chunk_id = drn_writer_get_last_chunk_id(cache);
    LSEEK(cache->fd, 0L, SEEK_SET);
    bytes = WRITE(cache->fd, &header, sizeof(drn_header_container_t));
    if (bytes !=  sizeof(drn_header_container_t))
        return -1;
    LSEEK(cache->fd, cache->desc_start_offset + cache->desc_end_offset, SEEK_SET);
    bytes = WRITE(cache->fd, cache->descriptors, WRITE_COUNT_CAST (sizeof(drn_desc_t) * cache->chunk_count));
    if (bytes !=  sizeof(drn_desc_t) * cache->chunk_count)
        return -1;
    if (CLOSE(cache->fd) == -1)
        return -1;
    for (i = 0; i < sbcount(cache->maps); ++i)
    {
        drn_writer_map_t * map = cache->maps + i;
        uint64_t j;
        for (j = 0; j < sbcount(map->keys); ++j)
        {
            drn_writer_key_t * value = map->keys + j;
            sbfree(value->descriptors);
            FREE(value->value);
        }
        sbfree(map->keys);
        FREE(map->name);
    }
    sbfree(cache->maps);
    sbfree(cache->descriptors);
    return 0;
}

drn_map_id_t drn_writer_create_map(drn_writer_t *cache, const char * mapname)
{
    drn_writer_map_t * map;
    int32_t match = find_map(cache->maps, (void*) mapname);

    if (match != -1)
        return (drn_map_id_t) match;
    map = sbadd(cache->maps, 1);
    map->name = (char*) ALLOCATE(strlen(mapname)*sizeof(char)+1);
    map->keys = 0;
    strcpy(map->name, mapname);
    return sbcount(cache->maps) - 1;
}

drn_map_id_t drn_writer_get_map_id(drn_writer_t *cache, const char * map)
{
    drn_map_id_t match = find_map(cache->maps, (void*) map);
    return match;
}

drn_chunk_id_t  drn_writer_get_last_chunk_id(drn_writer_t * cache)
{
    return cache->chunk_count -1;
}

int32_t  drn_writer_add_chunk(drn_writer_t * cache,
                              const void * data, uint64_t size)
{
    size_t bytes;
    drn_desc_t * desc = sbadd(cache->descriptors, 1);
    LSEEK(cache->fd, cache->desc_start_offset + cache->desc_end_offset, SEEK_SET);
    bytes = WRITE(cache->fd, data, WRITE_COUNT_CAST size);
    if (bytes != size)
        return -1;
    desc->size = size;
    desc->offset = cache->desc_end_offset;
    cache->desc_end_offset += size;
    ++cache->chunk_count;
    return 0;
}

int32_t drn_writer_update_chunk(drn_writer_t * cache,
                                 drn_chunk_id_t chunk_id, const void * data, uint64_t size)
{
    size_t bytes;
    size_t seeked = LSEEK(cache->fd, 0L, SEEK_CUR);
    drn_desc_t d;

    if (chunk_id >= cache->chunk_count)
        return -3;
    d = cache->descriptors[chunk_id];
    if (d.size != size)
        return -2;
    LSEEK(cache->fd, cache->desc_start_offset + d.offset, SEEK_SET);
    bytes = WRITE(cache->fd, data, WRITE_COUNT_CAST size);
    if (bytes != size)
        return -1;
    LSEEK(cache->fd, seeked, SEEK_SET);
    return 0;
}

int32_t drn_writer_map_chunk(drn_writer_t * cache,
                             drn_chunk_id_t desc,
                             uint64_t map_count,
                             const drn_map_id_t * map_ids,
                             const char ** key_values)
{
    uint64_t i;

    for (i = 0; i < map_count; ++i)
    {
        drn_writer_map_t * map;
        int32_t match;
        drn_writer_key_t * key = 0;

        if (sbcount(cache->maps) < map_count)
            return -1;
        map = cache->maps + map_ids[i];
        match = find_key(map->keys, (void*) key_values[i]);
        if (match == -1)
        {
            key = sbadd(map->keys, 1);
            key->value = (char*) ALLOCATE(strlen(key_values[i])*sizeof(char)+1);
            key->descriptors = 0;
            strcpy(key->value, key_values[i]);
        }
        else
        {
            key = map->keys +  match;
        }
        * (sbadd(key->descriptors, 1)) = desc;
    }
    return 0;
}
