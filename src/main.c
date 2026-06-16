

#include "container.h"
#include "cli.h"


int main(int argc, char *argv[])
{
    ContainerConfig config;

    parse_args(argc, argv, &config);

    printf(
    "%s %s %d %d %s\n",
    config.name,
    config.rootfs,
    config.cpu_percent,
    config.memory_mb,
    config.ip
);

    return start_container(&config);
}