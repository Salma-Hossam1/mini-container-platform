
#define _GNU_SOURCE
#include "network.h"

#include <errno.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>



int create_veth_pair(char *host_if, char *container_if){
        char cmd[512];

snprintf(
    cmd,
    sizeof(cmd),
    "ip link add %s type veth peer name %s",
    host_if,
    container_if
);

if (system(cmd) != 0)
{
    return -1;
}

return 0;
}

int move_veth_to_namespace(pid_t pid ,char *container_if){
        // why using pid here ? because the container is running in a new pid namespace , so we need to use the pid of the container process in the host namespace  .. Network Namespace belongs to process , so here we tell kernel to move veth1 (interface) to the network namespace of the process with pid <pid> in the host namespace
         char cmd[512];
    snprintf(
        cmd,
        sizeof(cmd),
        "ip link set %s netns %d",
        container_if,
        pid
    );


if (system(cmd) != 0)
{
    return -1;
}
return 0;
}

int configure_host_network(char *host_if){

    char cmd[512];
    // cancel this step after using bridge , as we will assign ip to the bridge interface not the host end of veth pair
//     snprintf(
//     cmd,
//     sizeof(cmd),
//     "ip addr add 10.0.0.1/24 dev %s",
//     host_if
// );
//     system(cmd);
   //switch the interface status from Down to Up .. Ex : ip link set veth-host up
   snprintf(
    cmd,
    sizeof(cmd),
    "ip link set %s up",
    host_if
);
    if (system(cmd) != 0)
{
    return -1;
}

return 0;}

int configure_container_network(pid_t pid , char *container_if , char *ip){
     // interface in child namespace , so the parent cannot set the configurations as it cannot see them
    // Enter child namespace :
    char path[256];
    char cmd[512];

snprintf(
    path,
    sizeof(path),
    "/proc/%d/ns/net",
    pid
);
// fd now is handle on container namespace 
int fd = open(path, O_RDONLY);

// now we are in host namespace , before enter the child , save the namespace of parent 
int original_ns =
    open("/proc/self/ns/net", O_RDONLY);

    // Enter namespace of child 
    // Now parent process can see lo , veth-container , not veth-host :
    setns(fd, CLONE_NEWNET);  // process itself not changed , but its view changed 

    // now set up the container ip interface , ....
    snprintf(
    cmd,
    sizeof(cmd),
    "ip addr add %s/24 dev %s",
    ip,
    container_if
);
printf("%s\n", cmd);
    system(cmd);

// system(
//     "ip link set " container_if " up"
// );
snprintf(
    cmd,
    sizeof(cmd),
    "ip link set %s up",
    container_if
);
    system(cmd);

system(
    "ip link set lo up"
);

// add gateway to allow container to access outside world .. Ex : ip route add default via  .. 10.0.0.1 -> mini0 ip address in the host
snprintf(
    cmd,
    sizeof(cmd),
    "ip route add default via 10.0.0.1"
);

system(cmd);

  // bach to host prespective
  setns(original_ns, CLONE_NEWNET);
  return 0;
}