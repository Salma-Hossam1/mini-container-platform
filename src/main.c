// #define _GNU_SOURCE
// #include <errno.h>
// #include <sched.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <sys/wait.h>
// #include <unistd.h>
// #include <sys/mount.h>
// #include <sys/mount.h>
// #include <unistd.h>


// #define STACK_SIZE (1024 * 1024)

// static char child_stack[STACK_SIZE];

// int child_fn(void *arg)
// {
//     printf("Inside container\n");

//     printf("PID inside namespace: %d\n", getpid());

//     // Mount namespace , private proc , no shared mounts between container and host
//     // Docker & runc make mounts private
//     mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL);

//     if (chroot("../rootfs/alpine") != 0)
// {
//     perror("chroot");
//     return 1;
// }

// if (chdir("/") != 0)
// {
//     perror("chdir");
//     return 1;
// }
// mount("proc", "/proc", "proc", 0, NULL);
//     // test that mounting only in container not host after mounting namespace
// /*    system("mkdir -p /tmp/testmount");
//     mount(
//     "tmpfs",
//     "/tmp/testmount",
//     "tmpfs",
//     0,
//     NULL
// );*/
//    // system("mount | grep testmount");

//    // mount proc to make container see the processes using ps 
//    mount("proc", "/proc", "proc", 0, NULL);
//     execlp("/bin/sh", "sh", NULL);

//     perror("execlp");
//     return 1;
// }

// int main()
// {
//     pid_t pid = clone(
//         child_fn,
//         child_stack + STACK_SIZE,
//         CLONE_NEWPID | CLONE_NEWNS | SIGCHLD,
//         NULL
//     );

//     if (pid == -1)
//     {
// 	printf("errno = %d\n", errno);
//         perror("clone");
//         exit(EXIT_FAILURE);
//     }

//     waitpid(pid, NULL, 0);

//     return 0;
// }

#include "container.h"
#include "cli.h"

// int main()
// {
//   ContainerConfig config =
// {
//     .name = "mcrun",

//     .rootfs = "../rootfs/alpine",

//     .cpu_percent = 10,

//     .memory_mb = 256
// };

//     return start_container(&config);
// }

int main(int argc, char *argv[])
{
    ContainerConfig config;

    parse_args(argc, argv, &config);

    printf(
    "%s %s %d %d %s\n",
    config.name,
    config.rootfs,
    config.cpu_percent,
    config.memory_mb,
    config.ip
);

    return start_container(&config);
}