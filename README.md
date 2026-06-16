# Mini Container Runtime

A lightweight container runtime built from scratch in C using Linux kernel primitives.

The goal of this project is educational:

Instead of using Docker as a black box, rebuild the core technologies behind containers and understand how Linux provides process isolation, filesystem isolation, networking, resource control, and user mapping.

---

# What Happens When a Container Starts?

When running:

sudo ./mcrun --name web --ip 10.0.0.2

the runtime performs the following steps:

1. Create CPU and Memory cgroups.
2. Create a new process using clone().
3. Create isolated namespaces:

   * PID Namespace
   * Mount Namespace
   * Network Namespace
   * User Namespace
4. Create a veth pair.
5. Move one end into the container network namespace.
6. Attach the host side to a Linux bridge.
7. Configure container networking.
8. Configure default gateway.
9. Enable internet access through NAT.
10. Apply resource limits.
11. Switch to the container root filesystem.
12. Start /bin/sh inside the container.

Result:

A process that behaves like a separate machine while sharing the host kernel.

---

# Implemented Features

## Filesystem Isolation

Implemented using:

* chroot()
* custom Alpine root filesystem

Container sees:

/bin
/etc
/usr
...

instead of the host filesystem.

---

## PID Namespace

Implemented using:

CLONE_NEWPID

Processes inside the container have their own PID tree.

Example:

Inside container:

PID = 1

Host:

PID = 2191565

Both represent the same process.

---

## Mount Namespace

Implemented using:

CLONE_NEWNS

Mount operations performed inside the container do not affect the host.

---

## User Namespace

Implemented using:

CLONE_NEWUSER

UID Mapping:

Container:

root (uid=0)

Host:

uid=1000

This allows the container to appear as root internally without granting root privileges on the host.

Example:

Container:

uid=0(root)

Host:

uid=1000(salma)

---

## CPU Limits

Implemented using:

cgroups v2

Example:

--cpu 50

Limits the container to approximately 50% CPU.

---

## Memory Limits

Implemented using:

cgroups v2

Example:

--memory 256

Limits the container to 256 MB.

---

# Networking

Networking was implemented entirely using Linux networking primitives.

---

## veth Pair

Each container receives:

Host side:

web-host

Container side:

web-cont

Think of a veth pair as a virtual Ethernet cable.

Anything sent on one side appears on the other.

---

## Linux Bridge

A bridge named:

mini0

acts like a Layer-2 switch.

All container interfaces are connected to it.

Example:

```
        mini0
      /       \
     /         \
```

web-host         db-host
|                |
web-cont         db-cont

Containers can communicate directly through the bridge.

---

## Container Communication

Example:

Container A:

10.0.0.2

Container B:

10.0.0.3

A can ping B directly.

No NAT is involved.

Traffic stays inside the bridge.

---

## Default Gateway

Bridge IP:

10.0.0.1

Container route:

default via 10.0.0.1

The bridge acts as the first hop for outgoing traffic.

---

## NAT

Implemented using:

iptables MASQUERADE

Example:

Container:

10.0.0.2

Host WiFi:

192.168.1.6

When container sends traffic:

Source:

10.0.0.2

becomes:

192.168.1.6

before leaving the host.

Reply packets are translated back automatically.

This allows containers to access the internet.

---

## Port Mapping

Implemented using:

iptables DNAT

Example:

Host:

192.168.1.6:8080

Container:

10.0.0.2:80

Incoming packets:

192.168.1.6:8080

are rewritten to:

10.0.0.2:80

before routing.

This is conceptually similar to:

docker run -p 8080:80

---

# Architecture

```
                      Internet
                           |
                           |
                      NAT Router
                           |
                 Host IP: 192.168.1.6
                           |
                           |
                   +----------------+
                   |     mini0      |
                   | Linux Bridge   |
                   +----------------+
                      /          \
                     /            \
                    /              \
              web-host         db-host
                  |                |
              web-cont         db-cont
              10.0.0.2        10.0.0.3
```

---

# Build

gcc container.c rootfs.c main.c cgroup.c cli.c network.c bridge.c nat.c userns.c -o mcrun

---

# Run

sudo ./mcrun 
--name web 
--ip 10.0.0.2

---

# Run With Limits

sudo ./mcrun 
--name web 
--ip 10.0.0.2 
--cpu 50 
--memory 256

---

# Run With Port Mapping

sudo ./mcrun 
--name web 
--ip 10.0.0.2 
--host-port 8080 
--container-port 80

---

# Project Structure

container.c

Container lifecycle and clone().

rootfs.c

Filesystem isolation.

cgroup.c

CPU and memory limits.

network.c

veth creation and namespace networking.

bridge.c

Linux bridge management.

nat.c

NAT and port forwarding.

userns.c

User namespace configuration.

cli.c

Argument parsing.

main.c

Program entry point.

---

# What Is Missing?

This runtime intentionally focuses on core container technologies.

Potential future improvements:

* Volumes
* Image management
* Private registry
* Overlay filesystems
* Capabilities
* Seccomp
* OCI compatibility
* Container metadata store
* Container lifecycle API
* Container orchestration

---

# Educational Goal

The objective is not to replace Docker.

The objective is to understand how Docker-like runtimes are built using Linux kernel primitives and to expose the layers that are normally hidden behind a single command.
