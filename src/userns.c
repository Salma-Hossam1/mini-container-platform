//**
//  function to write in :
//  /proc/<pid>/setgroups
//  /proc/<pid>/uid_map
//  /proc/<pid>/gid_map
//  */


#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

static int write_file(
    const char *path,
    const char *data
)
{
    int fd = open(path, O_WRONLY);

    if (fd == -1)
    {
        perror(path);
        return -1;
    }

if (write(fd, data, strlen(data)) < 0)
{
    printf("Failed writing to %s\n", path);
    perror("write");

    close(fd);
    return -1;
}

    close(fd);
    

    return 0;
}

int setup_user_namespace(pid_t pid)
{
    char path[256];
    char map[256];

    snprintf(
        path,
        sizeof(path),
        "/proc/%d/setgroups",
        pid
    );

    write_file(
        path,
        "deny"
    );

    // container root   ->   host uid 1000
        snprintf(
        path,
        sizeof(path),
        "/proc/%d/uid_map",
        pid
    );

    snprintf(
        map,
        sizeof(map),
        "0 1000 1"
    );

    write_file(path, map);

    // gid_map
        snprintf(
        path,
        sizeof(path),
        "/proc/%d/gid_map",
        pid
    );

    snprintf(
        map,
        sizeof(map),
        "0 1000 1"
    );

    write_file(path, map);

    // debugging: print the contents of uid_map and gid_map
    char cmd[256];

snprintf(
    cmd,
    sizeof(cmd),
    "cat /proc/%d/uid_map",
    pid
);
system(cmd);

snprintf(
    cmd,
    sizeof(cmd),
    "cat /proc/%d/gid_map",
    pid
);
system(cmd);

    return 0;
}