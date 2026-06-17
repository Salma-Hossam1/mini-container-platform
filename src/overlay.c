#define _GNU_SOURCE

#include "overlay.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>


int create_overlay_dirs(
    const char *container_name
)
{
    char cmd[512];

    snprintf(
        cmd,
        sizeof(cmd),
        "mkdir -p "
        "../rootfs/containers/%s/upper "
        "../rootfs/containers/%s/work "
        "../rootfs/containers/%s/merged",
        container_name,
        container_name,
        container_name
    );

    return system(cmd);
}

int fix_overlay_permissions(
    const char *container_name
)
{
    char path[256];

    snprintf(
        path,
        sizeof(path),
        "../rootfs/containers/%s/upper",
        container_name
    );

    if (chown(path, 1000, 1000) != 0)
    {
        perror("upper");
        return -1;
    }

    snprintf(
        path,
        sizeof(path),
        "../rootfs/containers/%s/work",
        container_name
    );

    if (chown(path, 1000, 1000) != 0)
    {
        perror("work");
        return -1;
    }

    snprintf(
        path,
        sizeof(path),
        "../rootfs/containers/%s/merged",
        container_name
    );

    if (chown(path, 1000, 1000) != 0)
    {
        perror("merged");
        return -1;
    }

    return 0;
}

void get_merged_path(
    const char *container_name,
    char *buffer,
    int size
)
{
    snprintf(
        buffer,
        size,
        "../rootfs/containers/%s/merged",
        container_name
    );
}

int mount_overlay(
    const char *container_name
)
{
    char cmd[1024];

    snprintf(
        cmd,
        sizeof(cmd),

        "mount -t overlay overlay "
        "-o lowerdir=../rootfs/alpine,"
        "upperdir=../rootfs/containers/%s/upper,"
        "workdir=../rootfs/containers/%s/work "
        "../rootfs/containers/%s/merged",

        container_name,
        container_name,
        container_name
    );
    

    printf("%s\n", cmd);
    return system(cmd);
}

int unmount_overlay(
    const char *container_name
)
{
    char cmd[512];

    snprintf(
        cmd,
        sizeof(cmd),
        "umount ../rootfs/containers/%s/merged",
        container_name
    );

    return system(cmd);
}