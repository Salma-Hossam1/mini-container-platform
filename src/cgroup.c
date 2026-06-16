#define _GNU_SOURCE

#include "cgroup.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int create_cgroup(
    const char *name,
    int cpu_percent,
    int memory_mb
)
{
    
    // EX : path=/sys/fs/cgroup/mcrun
    char path[256];
    snprintf(
        path,
        sizeof(path),
        "/sys/fs/cgroup/%s",
        name
    );

    // Create cgroup directory .. Ex : mkdir -p  /sys/fs/cgroup/mcrun
    char cmd[512];

snprintf(
    cmd,
    sizeof(cmd),
    "mkdir -p %s",
    path
);

system(cmd);

// now set cpu limit for the cgroup .. Ex : echo 50000 > /sys/fs/cgroup/mcrun/cpu.cfs_quota_us
int period = 100000;

int quota =
    period * cpu_percent / 100;

    // open cpu.cfs_quota_us file and write quota value to it
    char cpu_file[256];

snprintf(
    cpu_file,
    sizeof(cpu_file),
    "%s/cpu.max",
    path
);

FILE *f = fopen(cpu_file, "w");

if (f == NULL)
{
    perror("fopen");
    return -1;
}

fprintf(f, "%d %d", quota, period);
fclose(f);


// now set memory limit for the cgroup .. Ex : echo 256M > /sys/fs/cgroup/mcrun/memory.max
char memory_file[256];
long memory_bytes =
    (long)memory_mb
    * 1024
    * 1024;

snprintf(
    memory_file,
    sizeof(memory_file),
    "%s/memory.max",
    path
);
FILE *f1 = fopen(memory_file, "w");
if (f1 == NULL)
{
    perror("fopen");
    return -1;
}   
fprintf(
    f1,
    "%ld",
    memory_bytes
);
fclose(f1);

return 0;
}

int add_process_to_cgroup(
    const char *name,
    pid_t pid
)
{
    // 1- build the path to cgroup.procs file .. Ex : /sys/fs/cgroup/mcrun/cgroup.procs
    char path[256];
    snprintf(
        path,
        sizeof(path),
        "/sys/fs/cgroup/%s/cgroup.procs",
        name
    );

    FILE *f = fopen(path, "w");
    if (f == NULL)
    {
        perror("fopen");
        return -1;
    }

    fprintf(f, "%d", pid);
    fclose(f);

    return 0;
}

// delete cgroup directory and all its contents .. Ex : rm -rf /sys/fs/cgroup/mcrun
int delete_cgroup(const char *name)
{
    char path[256];

    snprintf(
        path,
        sizeof(path),
        "/sys/fs/cgroup/%s",
        name
    );

    if (rmdir(path) != 0)
    {
        perror("rmdir cgroup");
        return -1;
    }

    return 0;
}