#define _GNU_SOURCE

#include "container.h"
#include "rootfs.h"
#include "cgroup.h"
#include "network.h"
#include "bridge.h"
#include "nat.h"
#include "userns.h"
#include "overlay.h"

#include <errno.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>



static char child_stack[STACK_SIZE];

static int child_fn(void *arg)
{
    printf(
    "Start of Child_fn : uid=%d gid=%d\n",
    getuid(),
    getgid()
);
    ContainerConfig *config = (ContainerConfig *)arg;

    printf("Inside container\n");
    printf("PID inside namespace: %d\n", getpid());

    // add piping to stop the child process until the parent process finishes setting up the network namespace and cgroup limits .. we use pipe for synchronization between parent and child processes
    close(config->sync_pipe[1]);
    char ready;

    read(
        config->sync_pipe[0],
        &ready,
        1
    );

    // Section will be delteted:
    printf(
    "before setuid: uid=%d gid=%d\n",
    getuid(),
    getgid()
    ////////////////////////////
);

setgid(0);
setuid(0);
 // Section will be delteted:
printf(
    "after setuid: uid=%d gid=%d\n",
    getuid(),
    getgid()
);

///////////////////////////////////////////////////

    if (mount_overlay(config->name) != 0)
     {
    printf("overlay mount failed\n");
     }
    else
    {
        printf("overlay mount ok\n");
    }

    char merged[256];

    get_merged_path(
    config->name,
    merged,
    sizeof(merged)
    );


    if (setup_rootfs(merged) != 0)
    {
        return 1;
    }

    execlp("/bin/sh", "sh", NULL);

    perror("execlp");

    return 1;
}

int start_container(ContainerConfig *config)
{
    // Create cgroup and add current process to it
    if (create_cgroup(config->name, config->cpu_percent, config->memory_mb) != 0)
    {
        return -1;
    }

    create_overlay_dirs(config->name);
    // add write per to user 1000 in host -> 0 in container
    fix_overlay_permissions(config->name);

    if (pipe(config->sync_pipe) != 0)
    {
    perror("pipe");
    return -1;
    }
    

    pid_t pid = clone(
        child_fn,
        child_stack + STACK_SIZE,
        CLONE_NEWUSER |
        CLONE_NEWPID |
        CLONE_NEWNS |
        CLONE_NEWNET |
        SIGCHLD ,
        config
    );

    if (pid == -1)
    {
        printf("errno = %d\n", errno);
        perror("clone");
        return EXIT_FAILURE;
    }
    printf("Host PID = %d\n", pid);

    close(config->sync_pipe[0]);


        // Debugging: print the contents of uid_map and gid_map
    if (setup_user_namespace(pid) != 0)
{
    printf("userns failed\n");
}
else
{
    printf("userns ok\n");
}

    // Net namespace : 1) add veth pair 2) move one end to container 3) assign IP addresses 4) bring up interfaces
    // first name the interfaces in the parent and child config structures .. Ex : veth-host , veth-container
      char host_if[64];
    char container_if[64];
    snprintf(
    config->host_if,
    sizeof(config->host_if),
    "%s-host",
    config->name
);

snprintf(
    config->container_if,
    sizeof(config->container_if),
    "%s-cont",
    config->name
);


   ensure_bridge_exists();

   // check and enable ip forwarding and nat .. Ex : sysctl -w net.ipv4.ip_forward=1 , iptables -t nat -A POSTROUTING -s
   enable_ip_forwarding();

enable_nat(
    "10.0.0.0/24",
    "wlp4s0"
);
    
    // 1) add veth pair .. Ex : ip link add veth0 (host) type veth peer name veth1 (container)
//     char cmd[512];

// snprintf(
//     cmd,
//     sizeof(cmd),
//     "ip link add veth-host type veth peer name veth-container"
// );

// system(cmd);
  create_veth_pair(config->host_if, config->container_if);

  // attach host end of veth pair to bridge .. Ex : ip link set veth-host master mini0
  attach_interface_to_bridge(
    config->host_if
);

    // 2) move one end to container .. Ex : ip link set veth1 netns <pid>
    // why using pid here ? because the container is running in a new pid namespace , so we need to use the pid of the container process in the host namespace  .. Network Namespace belongs to process , so here we tell kernel to move veth1 (interface) to the network namespace of the process with pid <pid> in the host namespace
//     snprintf(
//         cmd,
//         sizeof(cmd),
//         "ip link set veth-container netns %d",
//         pid
//     );


// system(cmd);
    move_veth_to_namespace(pid, config->container_if);

// 3) assign IP addresses .. Ex : ip addr add
// now we handle layer 3 and assign IP addresses to the interfaces .. we assign IP address to veth-host in the host namespace and veth-container in the container namespace .. Ex : ip addr add
  // 1- host side
// system(
//     "ip addr add 10.0.0.1/24 dev veth-host"
// );
//    //switch the interface status from Down to Up .. Ex : ip link set veth-host up
//    system(
//     "ip link set veth-host up"
// );
configure_host_network(config->host_if);

    // Add child process to cgroup
    if (add_process_to_cgroup(config->name, pid) != 0)
    {
        return -1;
    }

    // 2- child side
   configure_container_network(pid, config->container_if , config->ip);

   // if port mapping is required , add DNAT rule to iptables .. Ex : iptables -t nat -A OUTPUT -p tcp --dport 8080 -j DNAT --to-destination
   if (
    config->host_port > 0 &&
    config->container_port > 0
)
{
    add_port_mapping(
        config->host_port,
        config->ip,
        config->container_port
    );
}

    // allow the child process to continue after the parent process finishes setting up the network namespace and cgroup limits
    write(
    config->sync_pipe[1],
    "x",
    1
);

close(config->sync_pipe[1]);


    waitpid(pid, NULL, 0);

    unmount_overlay(config->name);

    // cleanup port mapping if exists
    if (
    config->host_port > 0 &&
    config->container_port > 0
)
{
    remove_port_mapping(
        config->host_port,
        config->ip,
        config->container_port
    );
}

    // Cleanup cgroup after container exits
        delete_cgroup(config->name);

    return 0;
}