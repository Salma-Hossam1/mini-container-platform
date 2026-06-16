#include "cli.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static void set_defaults(
    ContainerConfig *config
)
{
    strcpy(config->name, "mcrun");

    strcpy(
        config->rootfs,
        "../rootfs/alpine"
    );

    config->cpu_percent = 100;

    config->memory_mb = 512;
    strcpy(config->ip, "10.0.0.2");

    // No port mapping by default
    config->host_port = 0;
    config->container_port = 0;
}

// Define long options for getopt , and the corresponding short options , required_argument means that the option requires an argument
// --cpu only -> error , --cpu 10 -> cpu_percent = 10
static struct option options[] =
{
    {"name", required_argument, 0, 'n'},

    {"rootfs", required_argument, 0, 'r'},

    {"cpu", required_argument, 0, 'c'},

    {"memory", required_argument, 0, 'm'},

    {"ip", required_argument, 0, 'i'},

    {"publish", required_argument, 0, 'p'},

    {0,0,0,0}
};

int parse_args(
    int argc,
    char *argv[],
    ContainerConfig *config
)
{
    set_defaults(config);

    int opt;

    while ((opt = getopt_long(argc, argv, "n:r:c:m:p:", options, NULL)) != -1)
    {
        switch (opt)
        {
        case 'n':
            strncpy(config->name, optarg, sizeof(config->name) - 1);
            break;
        case 'r':
            strncpy(config->rootfs, optarg, sizeof(config->rootfs) - 1);
            break;
        case 'c':
            config->cpu_percent = atoi(optarg);
            break;
        case 'm':
            config->memory_mb = atoi(optarg);
            break;
        case 'i':
            strncpy(config->ip, optarg, sizeof(config->ip) - 1);
            break;
        case 'p':
            // Parse port mapping argument (e.g., 8080:80)
            char *colon = strchr(optarg, ':');
            if (colon)
            {
                *colon = '\0';
                config->host_port = atoi(optarg);
                config->container_port = atoi(colon + 1);
            }
            break;
        default:
            fprintf(stderr,"Usage: %s -n <name> -r <rootfs> -c <cpu_percent> -m <memory_mb> -i <ip>\n", argv[0]);
            return -1;
        }
    }

    return 0;
}