#include <stdio.h>
#include <stdlib.h>

#include "nat.h"


int enable_ip_forwarding(void)
{
    return system(
        "sysctl -w net.ipv4.ip_forward=1 > /dev/null"
    );
}


int enable_nat(
    const char *subnet,
    const char *out_if
)
{
    char cmd[512];

    snprintf(
        cmd,
        sizeof(cmd),
        "iptables -t nat -C POSTROUTING "
        "-s %s -o %s -j MASQUERADE "
        "> /dev/null 2>&1",
        subnet,
        out_if
    );

    if (system(cmd) == 0)
    {
        return 0; // already exists
    }

    snprintf(
        cmd,
        sizeof(cmd),
        "iptables -t nat -A POSTROUTING "
        "-s %s -o %s -j MASQUERADE",
        subnet,
        out_if
    );

    return system(cmd);
}

int add_port_mapping(
    int host_port,
    const char *container_ip,
    int container_port
)
{
    char cmd[512];

    snprintf(
        cmd,
        sizeof(cmd),
        "iptables -t nat "
        "-A OUTPUT "
        "-p tcp "
        "--dport %d "
        "-j DNAT "
        "--to-destination %s:%d",
        host_port,
        container_ip,
        container_port
    );

    return system(cmd);
}

int remove_port_mapping(
    int host_port,
    const char *container_ip,
    int container_port
)
{
    char cmd[512];

    snprintf(
        cmd,
        sizeof(cmd),
        "iptables -t nat "
        "-D OUTPUT "
        "-p tcp "
        "--dport %d "
        "-j DNAT "
        "--to-destination %s:%d",
        host_port,
        container_ip,
        container_port
    );

    return system(cmd);
}