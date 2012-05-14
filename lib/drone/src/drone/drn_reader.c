#ifndef _MSC_VER /* Force 64 bits lseek offsets */
#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64
#endif

#include <stdlib.h>
#include <string.h>
#ifdef _MSC_VER
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#include <sys/mman.h>
#endif /*_MSC_VER */
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <assert.h>

#include "drn_reader.h"
#include "drn_hash.h"

#define ALLOCATE(SIZE) malloc((size_t) SIZE)
#define FREE(PTR) free(PTR)
#define RESTRICT __restrict__

#ifdef _MSC_VER
#pragma warning(disable : 4996) /* Disable deprecated warning on posix functions */
#define OPEN_MODE O_RDONLY | _O_BINARY
#define READ_COUNT_CAST (int)
#define LSEEK _lseeki64
#define CLOSE _close
#define OPEN _open
#define READ _read
#define WRITE _write
#else
#define LSEEK lseek64
#define CLOSE close
#define OPEN open
#define READ read
#define WRITE write
#define OPEN_MODE O_RDONLY
#define READ_COUNT_CAST (size_t)
#endif /*_MSC_VER */

static uint64_t drn_fsize(int fd)
{
    int64_t curr = LSEEK(fd, 0L, SEEK_CUR);
    uint64_t size = (uint64_t) LSEEK(fd, 0L, SEEK_END);
    LSEEK(fd, curr, SEEK_SET);
    return size;
}

drn_map_id_t  drn_get_map_id(drn_t *cache, const char * name)
{
    return 0;
}

int32_t drn_open(drn_t * cache, const char * filename, int mode)
{
    int fd = OPEN(filename, OPEN_MODE);
    if (fd <=0)
    {
        return -1;
    }
    cache->fd = fd;
    cache->mmap_size = drn_fsize(fd);
    cache->mode = mode;
    if (mode == DRN_READ_NOLOAD)
    {
        uint64_t i;
        size_t bytes;
        drn_desc_t hashes_desc;
        drn_map_container_t * map_containers;

        cache->mmap_start = 0;
        cache->data = 0;
        cache->header = ALLOCATE(sizeof(drn_header_container_t));
        bytes = READ(fd, cache->header, sizeof(drn_header_container_t));
        assert(bytes == sizeof(drn_header_container_t));
        if (drn_get_version(cache) != DRN_WRITER_VERSION)
            return 1;
        cache->descriptors = ALLOCATE(cache->header->chunk_count * sizeof(drn_desc_t));
        LSEEK(fd, cache->header->index_offset, SEEK_SET);
        bytes = READ(fd, cache->descriptors, READ_COUNT_CAST ((size_t) cache->header->chunk_count) * sizeof(drn_desc_t));
        assert(bytes == cache->header->chunk_count * sizeof(drn_desc_t));

        hashes_desc =  drn_get_desc(cache, cache->header->maps_chunk_id);
        cache->map_count = hashes_desc.size / sizeof(drn_map_container_t);
        cache->maps = ALLOCATE(sizeof(drn_map_t) * (size_t) cache->map_count);
        map_containers = ALLOCATE(hashes_desc.size);
        drn_read_chunk(cache, cache->header->maps_chunk_id, map_containers);
        for (i = 0; i < cache->map_count; ++i)
        {
            drn_desc_t d;
            const drn_map_container_t * c = map_containers + i;
            drn_map_t * h = cache->maps + i;

            h->hash = ALLOCATE(drn_get_desc(cache, c->hash_chunk_id).size);
            drn_read_chunk(cache, c->hash_chunk_id, (void *) h->hash);
            h->name = ALLOCATE(drn_get_desc(cache, c->name_chunk_id).size);
            drn_read_chunk(cache, c->name_chunk_id, (void *) h->name);
            d = drn_get_desc(cache, c->descriptors_chunk_id);
            h->chunk_count = d.size / sizeof(drn_hash_desc_t);
            h->descriptors = ALLOCATE(d.size);
            drn_read_chunk(cache, c->descriptors_chunk_id, (void *) h->descriptors);
            h->value_strings = ALLOCATE(drn_get_desc(cache, c->value_strings_chunk_id).size);
            drn_read_chunk(cache, c->value_strings_chunk_id, (void *)h->value_strings);
        }
        FREE(map_containers);
    }
    else
    {
        uint64_t i;
        drn_desc_t maps_desc;
        drn_map_container_t * map_containers;

        if (mode == DRN_READ_MMAP)
        {
#ifdef _MSC_VER
			wchar_t * w_filename;
			size_t bytes;

			CLOSE(fd);
			w_filename = ALLOCATE((strlen(filename)+1) * sizeof(wchar_t));
			bytes = mbstowcs(w_filename, filename, strlen(filename)+1);
			//cache->w_fhandle = CreateFile(L"test.drn", GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
			cache->w_fhandle = CreateFile(w_filename, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
			FREE(w_filename);
			if ((int) cache->w_fhandle  == HFILE_ERROR)
			{
				DWORD err = GetLastError();
				printf("ERROR %Ld\n", err);
				return -1;
			}
			cache->w_mhandle = CreateFileMapping(cache->w_fhandle, NULL, PAGE_READONLY, 0, 0, NULL);
			if (cache->w_mhandle  == NULL)
			{
				DWORD err = GetLastError();
				return -1;
			}
			cache->mmap_start = MapViewOfFile(cache->w_mhandle, FILE_MAP_READ, 0, 0, (size_t) cache->mmap_size);
			if (cache->mmap_start  == NULL)
			{
				DWORD err = GetLastError();
				return -1;
			}
#else
            cache->mmap_start = mmap(0, cache->mmap_size, PROT_READ, MAP_PRIVATE, fd, 0);
			CLOSE(fd);
#endif
        }
        else if (mode == DRN_READ_LOAD)
        {
            size_t bytes;

            cache->mmap_start = ALLOCATE(cache->mmap_size);
            bytes = READ(fd, cache->mmap_start, READ_COUNT_CAST (size_t) cache->mmap_size);
            assert(bytes == cache->mmap_size);
			CLOSE(fd);
        }
        cache->data = (char *) cache->mmap_start + sizeof(drn_header_container_t);
        cache->header = (drn_header_container_t *) cache->mmap_start;
        if (drn_get_version(cache) != DRN_WRITER_VERSION)
            return 1;
        cache->descriptors = (drn_desc_t *) ((char *)cache->mmap_start + cache->header->index_offset);
        maps_desc =  drn_get_desc(cache, cache->header->maps_chunk_id);
        cache->map_count = maps_desc.size / sizeof(drn_map_container_t);
        map_containers = (drn_map_container_t *) drn_get_chunk(cache, cache->header->maps_chunk_id);
        cache->maps = ALLOCATE(sizeof(drn_map_t) * (size_t) cache->map_count);
        for (i = 0; i < cache->map_count; ++i)
        {
            drn_desc_t d;
            const drn_map_container_t * c = map_containers + i;
            drn_map_t * h = cache->maps + i;

            h->hash =  drn_get_chunk(cache, c->hash_chunk_id);
            h->name =  drn_get_chunk(cache, c->name_chunk_id);
            d = drn_get_desc(cache, c->descriptors_chunk_id);
            h->chunk_count = d.size / sizeof(drn_hash_desc_t);
            h->descriptors =  drn_get_chunk(cache, c->descriptors_chunk_id);
            h->value_strings =  drn_get_chunk(cache, c->value_strings_chunk_id);
        }
    }
    return 0;
}

int32_t drn_close(drn_t * cache)
{
    if (cache->mode == DRN_READ_MMAP)
    {
        FREE(cache->maps);
#ifdef _MSC_VER
		UnmapViewOfFile(cache->mmap_start);
		CloseHandle(cache->w_mhandle);
		CloseHandle(cache->w_fhandle);
#else
		munmap(cache->mmap_start, cache->mmap_size);
#endif
    }
    else if (cache->mode == DRN_READ_LOAD)
    {
        FREE(cache->maps);
        FREE(cache->mmap_start);
    }
    else /* DRN_READ_NOLOAD */
    {
        int i;
        for (i = 0; i < cache->map_count; ++i)
        {
            drn_map_t * h = cache->maps + i;
            FREE((void*) h->hash);
            FREE((void*) h->name);
            FREE((void*) h->descriptors);
            FREE((void*) h->value_strings);
        }
        FREE(cache->maps);
        FREE(cache->header);
        FREE(cache->descriptors);
        CLOSE(cache->fd);
    }
    return 0;
}

uint64_t drn_get_version(drn_t * cache)
{
    assert(cache);
    return cache->header->version;
}

const char * drn_get_description(drn_t * cache)
{
    assert(cache);
    return cache->header->description;
}

uint64_t drn_get_map_count(drn_t * cache)
{
    assert(cache);
    return cache->map_count;
}

const char * drn_get_map_name(drn_t * cache, uint64_t map_id)
{
    assert(cache);
    assert(cache->map_count > map_id);
    return cache->maps[map_id].name;
}

uint64_t drn_get_chunk_count(drn_t *cache)
{
    assert(cache);
    return cache->header->chunk_count;
}

drn_desc_t drn_get_desc(drn_t * cache, drn_chunk_id_t chunk_id)
{
    assert(cache->header->chunk_count > chunk_id);
    return * (cache->descriptors + chunk_id);
}

const void * drn_get_chunk(drn_t * cache, drn_chunk_id_t chunk_id)
{
    drn_desc_t * desc;

    assert(cache);
    assert(cache->header->chunk_count > chunk_id);
    assert(cache->mode != DRN_READ_NOLOAD);
    desc = (cache->descriptors + chunk_id);
    return (char *) cache->data + desc->offset;
}

int32_t drn_read_chunk(drn_t * cache, drn_chunk_id_t chunk_id, void * data)
{
    size_t bytes;
    drn_desc_t * desc = (cache->descriptors + chunk_id);

    assert(cache->mode == DRN_READ_NOLOAD);
    assert(cache->header->chunk_count > chunk_id);
    LSEEK(cache->fd, desc->offset + sizeof(drn_header_container_t), SEEK_SET);
    bytes = READ(cache->fd, data, READ_COUNT_CAST (size_t) desc->size);
    assert(bytes == desc->size);
    return 0;
}

const char * drn_get_desc_key_value(drn_t * cache, drn_chunk_id_t chunk_id, drn_map_id_t map_id)
{
    const drn_map_t * h = cache->maps + map_id;
    uint64_t hash_chunk_count = h->chunk_count;
    const drn_hash_desc_t * hash_descriptors = h->descriptors;
    const char * value_strings = h->value_strings;
    uint64_t i = 0;

    while (i < hash_chunk_count)
    {
        if (hash_descriptors[i].chunk_id == chunk_id)
            return value_strings + hash_descriptors[i].key_value_offset;
        ++i;
    }
    return 0;
}

/*! Look for chunk_id in chunk_ids. Returns 1 if found, else returns 0 */
int32_t drn_is_in(uint64_t count, drn_chunk_id_t * chunk_ids, drn_chunk_id_t chunk_id)
{
    uint64_t d_idx = 0;
    while (d_idx < count)
    {
        if (chunk_ids[d_idx] == chunk_id)
            return 1;
        ++d_idx;
    }
    return 0;
}

uint64_t drn_count_matching_chunks_union(drn_t * cache,
                                         uint64_t map_count, const drn_map_id_t * map_ids, const char ** key_values)
{
    uint64_t map_idx;
    uint64_t d_idx;
    uint64_t matching_chunk_count = 0;
    /* Identify map with minimum amount of descriptors associated to corresponding value */
    uint64_t min_desc_by_val = -1;
    uint64_t * matching_chunk_counts = (uint64_t *) ALLOCATE(map_count * sizeof(uint64_t));
    drn_chunk_id_t ** matching_chunk_ids = (drn_chunk_id_t **) ALLOCATE(map_count * sizeof(drn_chunk_id_t **));
    uint64_t min_desc_by_val_map_id = 0;

    for (map_idx = 0; map_idx < map_count; ++map_idx)
    {
        drn_map_id_t map_id = map_ids[map_idx];
        uint64_t count = drn_count_matching_chunks(cache, map_id, key_values[map_idx]);

        /* Get matching ids of each map */
        matching_chunk_counts[map_idx] = count;
        matching_chunk_ids[map_idx] = (drn_chunk_id_t *) ALLOCATE(count * sizeof(drn_chunk_id_t));
        drn_get_matching_chunks(cache, map_id, key_values[map_idx], count, matching_chunk_ids[map_id]);
        if (count > 0 && count < min_desc_by_val)
        {
            min_desc_by_val = count;
            min_desc_by_val_map_id = map_idx;
        }
    }
    /* Foreach desc in map with minimum matching descriptors */
    for (d_idx = 0 ; d_idx < matching_chunk_counts[min_desc_by_val_map_id]; ++d_idx)
    {
        int32_t match = 1;
        drn_chunk_id_t matching_desc = matching_chunk_ids[min_desc_by_val_map_id][d_idx];

        /* Look for the desc in other matching desc lists */
        map_idx = 0;
        while (map_idx < map_count && match)
        {
            if (map_idx != min_desc_by_val_map_id)
            {
                match = drn_is_in(matching_chunk_counts[map_idx], matching_chunk_ids[map_idx], matching_desc);
            }
            ++map_idx;
        }
        if (match)
            ++matching_chunk_count;
    }

    FREE(matching_chunk_counts);
    for (map_idx = 0; map_idx < map_count; ++map_idx)
        FREE(matching_chunk_ids[map_idx]);
    FREE(matching_chunk_ids);
    return matching_chunk_count;
}

uint64_t    drn_get_matching_chunks_union(drn_t * cache,
                                          uint64_t map_count, const drn_map_id_t * map_ids, const char ** key_values,
                                          uint64_t max_chunk_count, drn_chunk_id_t * matching_chunks)
{
    uint64_t map_idx;
    uint64_t matching_chunk_count = 0;
    uint64_t d_idx;
    /* Identify map with minimum amount of descriptors associated to corresponding value */
    uint64_t * matching_chunk_counts = (uint64_t *) ALLOCATE(map_count * sizeof(uint64_t));
    drn_chunk_id_t ** matching_chunk_ids = (drn_chunk_id_t **) ALLOCATE(map_count * sizeof(drn_chunk_id_t **));
    uint64_t min_desc_by_val_map_id = 0;

    for (map_idx = 0; map_idx < map_count; ++map_idx)
    {
        drn_map_id_t map_id = map_ids[map_idx];
        uint64_t count = drn_count_matching_chunks(cache, map_id, key_values[map_idx]);

        /* Get matching ids of each map */
        matching_chunk_counts[map_idx] = count;
        matching_chunk_ids[map_idx] = (drn_chunk_id_t *) ALLOCATE(count * sizeof(drn_chunk_id_t));
        drn_get_matching_chunks(cache, map_id, key_values[map_idx], count, matching_chunk_ids[map_idx]);
        if (count > 0 && count < min_desc_by_val_map_id)
        {
            min_desc_by_val_map_id = map_idx;
        }
    }
    /* Foreach desc in map with minimum matching descriptors */
    d_idx =0;
    while (d_idx < matching_chunk_counts[min_desc_by_val_map_id] && matching_chunk_count < max_chunk_count)
    {
        int32_t match = 1;
        drn_chunk_id_t matching_desc = matching_chunk_ids[min_desc_by_val_map_id][d_idx];

        /* Look for the desc in other matching desc lists */
        map_idx = 0;
        while (map_idx < map_count && match)
        {
            if (map_idx != min_desc_by_val_map_id)
            {
                match = drn_is_in(matching_chunk_counts[map_idx], matching_chunk_ids[map_idx], matching_desc);
            }
            ++map_idx;
        }
        if (match)
        {
            matching_chunks[matching_chunk_count] = matching_desc;
            ++matching_chunk_count;
        }
        ++d_idx;
    }

    FREE(matching_chunk_counts);
    for (map_idx = 0; map_idx < map_count; ++map_idx)
        FREE(matching_chunk_ids[map_idx]);
    FREE(matching_chunk_ids);
    return matching_chunk_count;
}

uint64_t drn_count_matching_chunks(drn_t * cache,
                                   drn_map_id_t map_id, const char * key_value)
{
    const drn_map_t * h = cache->maps + map_id;
    const drn_hash_cell_t * hash_map = h->hash;
    const drn_hash_desc_t * hash_descriptors = h->descriptors;
    const char * hash_values = h->value_strings;
    uint32_t cell_idx = drn_oat_hash((void * )key_value, (uint32_t) strlen(key_value))%DRN_HASH_MAP_SIZE;
    uint64_t count = 0;
    uint64_t d_idx;
    for (d_idx = 0; d_idx < hash_map[cell_idx].count; ++d_idx)
    {
        if (!strcmp(hash_values + hash_descriptors[hash_map[cell_idx].offset + d_idx].key_value_offset, key_value))
            ++count;
    }
    return count;
}

uint64_t  drn_get_matching_chunks(drn_t * cache,
                                  drn_map_id_t map_id, const char * key_value,
                                  uint64_t max_chunk_count, drn_chunk_id_t * matching_chunks)
{
    const drn_map_t * h = cache->maps + map_id;
    const drn_hash_cell_t * hash_map = h->hash;
    const drn_hash_desc_t * hash_descriptors = h->descriptors;
    const char * hash_values = h->value_strings;
    uint32_t cell_idx = drn_oat_hash((void * )key_value, (uint32_t) strlen(key_value))%DRN_HASH_MAP_SIZE;
    uint64_t count = 0;
    uint64_t d_idx = 0;
    while( count < max_chunk_count && d_idx < hash_map[cell_idx].count )
    {
        if (!strcmp(hash_values + hash_descriptors[hash_map[cell_idx].offset + d_idx].key_value_offset, key_value))
        {
            matching_chunks[count] = hash_descriptors[hash_map[cell_idx].offset + d_idx].chunk_id;
            ++count;
        }
        ++d_idx;
    }
    return count;
}
