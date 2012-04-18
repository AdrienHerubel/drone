#include <stdio.h>
#include <stdlib.h>

#include "getopt.h"

#include <drone/drn_reader.h>

/* Short command line options for get_opt */
const char * SHORT_OPTIONS = "hvl";
/* Long options for getopt_long */
const struct option LONG_OPTIONS[]=
{
    {"help",no_argument,NULL,'h'}, /*  Print help */
    {"verbose",no_argument,NULL,'v'}, /* Verbose mode */
    {"list",required_argument,NULL,'l'}, /*  List mode */
    {NULL, 0, NULL, 0} /*  End of array need by getopt_long do not delete it */
};

typedef unsigned char uint8_t;

/* Handle command line options */
typedef struct cli_opts_t
{
    uint8_t verbose;
    uint8_t list;
} cli_opts_t;

/* Parse command line options and fill BruteOptions structure */
void parse_cli_opts(int argc, char **argv, cli_opts_t * opts);
/* Print usage on standard output */
void usage();


int main (int argc, char **argv)
{
    cli_opts_t opts;
    parse_cli_opts(argc, argv, &opts);

    drn_t cache;
    drn_open(&cache, argv[1], DRN_READ_MMAP);

    printf("File:%s\n", argv[argc-1]);
    printf("Version:%lu\n", drn_get_version(&cache));
    printf("Description:%s\n", drn_get_description(&cache));
    printf("Entry count:%lu\n", drn_get_chunk_count(&cache));
    printf("Tag count:%lu\n", drn_get_map_count(&cache));


    uint64_t i;
    for (i = 0; i < drn_get_map_count(&cache); ++i)
    {
        printf("hash:%s\n",drn_get_map_name(&cache, i));
    }
    for (i = 0; i < drn_get_chunk_count(&cache); ++i)
    {
        drn_desc_t desc = drn_get_desc(&cache, i); 
        printf("desc:%lu:offset:%lu:size:%lu",i, desc.offset, desc.size);
        uint64_t j;
        for (j = 0; j < drn_get_map_count(&cache); ++j)
        {
            const char * value = drn_get_desc_key_value(&cache, i, j);
            if (value)
                printf(":tag:%s/%s",drn_get_map_name(&cache, j), value);
        }
        printf("\n");
    }

    drn_close(&cache);

    return 0;
}

void parse_cli_opts(int argc, char **argv, cli_opts_t * opts)
{
    char c;
    int optIdx;
    opts->verbose = 0;
    opts->list = 0;
    while ((c = getopt_long(argc, argv, (char *) SHORT_OPTIONS, (option *) LONG_OPTIONS, &optIdx)) != -1)
    {
        switch (c)
        {
        case 'h' : /* Print usage and exit */
            usage();
            exit(1);
            break;
        case 'v' : /* Verbose mode */
            opts->verbose = 1;
            break;
        case 'l' :  /* List mode */
            opts->list = 1;
            break;
        case '?' : /* Wut? */
            fprintf(stderr, "Unknown option\n");
            usage();
            exit(1);
        default :
            break;
        }
    }
}

void usage()
{
    printf("drnread [-hvl] \n");
}
