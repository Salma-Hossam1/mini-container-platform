#define _GNU_SOURCE

#include "rootfs.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/mount.h>

int setup_rootfs(const char *rootfs)
{
    mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL);

    if (chroot(rootfs) != 0)
    {
        perror("chroot");
        return -1;
    }

    if (chdir("/") != 0)
    {
        perror("chdir");
        return -1;
    }

    if (mount("proc", "/proc", "proc", 0, NULL) != 0)
    {
        perror("mount proc");
        return -1;
    }

    return 0;
}