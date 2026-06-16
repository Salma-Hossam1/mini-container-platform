#ifndef CONTAINER_H
#define CONTAINER_H

#define STACK_SIZE (1024 * 1024)

typedef struct
{
    char name[64];

    char rootfs[256];

    int cpu_percent;

    int memory_mb;

    int sync_pipe[2];

      char host_if[64];
    char container_if[64];
    char ip[32];

    // for DNAT
    int host_port;
    int container_port;
} ContainerConfig;

int start_container(ContainerConfig *config);

#endif