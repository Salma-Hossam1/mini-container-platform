#ifndef CGROUP_H
#define CGROUP_H

#include <sys/types.h>

int create_cgroup(
    const char *name,
    int cpu_percent,
    int memory_mb
);

int add_process_to_cgroup(
    const char *name,
    pid_t pid
);

int delete_cgroup(const char *name);
#endif