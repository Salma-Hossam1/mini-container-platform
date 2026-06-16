#if !defined(NAT_H)
#define NAT_H

#include <errno.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

int enable_ip_forwarding(void);
int enable_nat(const char *subnet, const char *out_if);

// port mapping using DNAT
int add_port_mapping(
    int host_port,
    const char *container_ip,
    int container_port
);

int remove_port_mapping(
    int host_port,
    const char *container_ip,
    int container_port
);
#endif // NAT_H

