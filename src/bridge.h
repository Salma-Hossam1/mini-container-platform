#ifndef BRIDGE_H
#define BRIDGE_H

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

int ensure_bridge_exists(void);
int attach_interface_to_bridge(const char *host_if);

#endif