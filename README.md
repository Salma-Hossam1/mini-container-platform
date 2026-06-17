# Mini Container Runtime

A lightweight container runtime built from scratch in C using Linux kernel primitives.

The goal of this project is educational:

Instead of treating Docker as a black box, this project rebuilds the core technologies behind containers directly on top of the Linux kernel and demonstrates how process isolation, filesystem isolation, networking, resource control, and user mapping actually work.

---

# Motivation

Most developers interact with containers through:

```bash
docker run
```

without seeing what happens underneath.

This project explores the layers hidden behind that command by implementing a minimal container runtime from scratch using:

* Linux Namespaces
* Linux Cgroups
* OverlayFS
* Virtual Ethernet Devices (veth)
* Linux Bridges
* NAT
* Port Forwarding

No Docker Engine.

No container libraries.

Just Linux.

---

# What Happens When a Container Starts?

When running:

```bash
sudo ./mcrun --name web --ip 10.0.0.2
```

the runtime performs the following steps:

1. Create CPU and Memory cgroups.
2. Create OverlayFS container layers.
3. Create a new process using `clone()`.
4. Create isolated namespaces:

   * PID Namespace
   * Mount Namespace
   * Network Namespace
   * User Namespace
5. Create a veth pair.
6. Move one end into the container network namespace.
7. Attach the host side to a Linux bridge.
8. Configure container networking.
9. Configure a default gateway.
10. Enable internet access through NAT.
11. Mount the container filesystem using OverlayFS.
12. Switch into the container root filesystem.
13. Start `/bin/sh`.

Result:

A process that behaves like an independent machine while sharing the host kernel.

---

# Implemented Features

## Filesystem Isolation

Implemented using:

* `chroot()`
* Mount Namespace

Container sees its own filesystem hierarchy:

```text
/bin
/etc
/usr
/root
...
```

instead of the host filesystem.

---

## Overlay Filesystem (OverlayFS)

Implemented using Linux OverlayFS.

Each container receives four layers:

```text
Lower Layer (Read Only)
        |
        v
    Alpine RootFS
        |
        |
        +------------------+
                           |
                           v
                 Overlay Filesystem
                           |
            +--------------+--------------+
            |                             |
            v                             v
      Upper Layer                    Work Layer
   (Container Writes)           (Kernel Internal)
```

### Lower Layer

Shared Alpine root filesystem.

Read-only.

Used by all containers.

### Upper Layer

Stores container modifications.

Examples:

* New files
* Edited files
* Deleted files

### Work Layer

Internal directory required by the Linux OverlayFS implementation.

### Merged Layer

The filesystem visible inside the container.

Changes made by one container do not affect other containers.

This provides copy-on-write behavior similar to Docker's OverlayFS storage driver.

---

## PID Namespace

Implemented using:

```c
CLONE_NEWPID
```

Processes inside the container have their own PID tree.

Example:

Container:

```text
PID = 1
```

Host:

```text
PID = 2191565
```

Both represent the same process.

---

## Mount Namespace

Implemented using:

```c
CLONE_NEWNS
```

Mount operations performed inside the container do not affect the host.

Each container has an isolated mount table.

This namespace is what allows each container to mount its own OverlayFS view without affecting other containers.

---

## User Namespace

Implemented using:

```c
CLONE_NEWUSER
```

UID Mapping:

Container:

```text
uid=0 (root)
```

Host:

```text
uid=1000
```

This allows a process to appear as root inside the container while remaining an unprivileged user on the host.

Example:

Container:

```text
uid=0(root)
```

Host:

```text
uid=1000(salma)
```

Both represent the same process.

---

## CPU Limits

Implemented using:

```text
cgroups v2
```

Example:

```bash
--cpu 50
```

Limits the container to approximately 50% CPU usage.

---

## Memory Limits

Implemented using:

```text
cgroups v2
```

Example:

```bash
--memory 256
```

Limits the container to 256 MB of RAM.

---

# Networking

Networking is implemented entirely using Linux networking primitives.

---

## veth Pair

Each container receives:

Host side:

```text
web-host
```

Container side:

```text
web-cont
```

A veth pair behaves like a virtual Ethernet cable.

Anything sent through one side appears on the other.

---

## Linux Bridge

A bridge named:

```text
mini0
```

acts as a Layer-2 switch.

All container interfaces are attached to it.

Example:

```text
          mini0
        /       \
       /         \
  web-host     db-host
      |            |
  web-cont     db-cont
```

Containers can communicate directly through the bridge.

---

## Container-to-Container Communication

Example:

Container A:

```text
10.0.0.2
```

Container B:

```text
10.0.0.3
```

Both communicate directly through the bridge.

No NAT is involved.

Traffic remains inside the virtual network.

---

## Default Gateway

Bridge IP:

```text
10.0.0.1
```

Container route:

```text
default via 10.0.0.1
```

The bridge acts as the first hop for outgoing traffic.

---

## NAT

Implemented using:

```text
iptables MASQUERADE
```

Example:

Container:

```text
10.0.0.2
```

Host:

```text
192.168.1.x
```

When traffic leaves the host:

```text
10.0.0.2
```

becomes:

```text
192.168.1.x
```

Replies are translated back automatically.

This enables internet connectivity for containers.

---

## Port Mapping

Implemented using:

```text
iptables DNAT
```

Example:

Host:

```text
192.168.1.x:8080
```

Container:

```text
10.0.0.2:80
```

Incoming packets are rewritten before routing:

```text
192.168.1.x:8080
            |
            v
10.0.0.2:80
```

Conceptually similar to:

```bash
docker run -p 8080:80
```

---

# Architecture

```text
                         Internet
                              |
                              |
                         NAT Router
                              |
                    Host IP: 192.168.1.x
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

```bash
gcc \
container.c \
rootfs.c \
overlay.c \
cgroup.c \
network.c \
bridge.c \
nat.c \
userns.c \
cli.c \
main.c \
-o mcrun
```

---

# Download Alpine RootFS

This repository does not include the Alpine root filesystem.

Download it separately:

```bash
mkdir -p rootfs/alpine

wget https://dl-cdn.alpinelinux.org/alpine/latest-stable/releases/x86_64/alpine-minirootfs-*.tar.gz

tar -xzf alpine-minirootfs-*.tar.gz -C rootfs/alpine
```

---

# Run

```bash
sudo ./mcrun \
--name web \
--ip 10.0.0.2
```

---

# Run With Resource Limits

```bash
sudo ./mcrun \
--name web \
--ip 10.0.0.2 \
--cpu 50 \
--memory 256
```

---

# Run With Port Mapping

```bash
sudo ./mcrun \
--name web \
--ip 10.0.0.2 \
--host-port 8080 \
--container-port 80
```

---

# Project Structure

```text
container.c
    Container lifecycle and clone()

rootfs.c
    Filesystem isolation and chroot

overlay.c
    OverlayFS management

cgroup.c
    CPU and memory limits

network.c
    veth creation and namespace networking

bridge.c
    Linux bridge management

nat.c
    NAT and port forwarding

userns.c
    User namespace configuration

cli.c
    Argument parsing

main.c
    Program entry point
```

---

# Future Improvements

Potential future enhancements:

* Volumes
* Image Management
* Private Registry
* Container Metadata Store
* Capabilities
* Seccomp
* OCI Compatibility
* Container Lifecycle API
* Container Snapshots
* Container Scheduler
* Kubernetes-style Orchestration

---

# Educational Goal

The objective is not to replace Docker.

The objective is to understand how modern container runtimes are built using Linux kernel primitives and expose the layers that are normally hidden behind a single command.
