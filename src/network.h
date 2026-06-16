#ifndef NETWORK_H
#define NETWORK_H

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>


int create_veth_pair(char *host_if, char *container_if);

int move_veth_to_namespace(pid_t pid ,char *container_if);

int configure_host_network(char *host_if);

int configure_container_network(pid_t pid , char *container_if , char *ip);

#endif