# Mini Container Runtime

A lightweight container runtime built from scratch in C using Linux kernel primitives.

               Internet
                   |
                NAT
                   |
              +---------+
              |  mini0  |
              +---------+
                /     \
               /       \
      web-host         db-host
          |               |
      web-cont        db-cont
      10.0.0.2        10.0.0.3

## Features

### Isolation

* PID Namespace
* Mount Namespace
* Network Namespace
* User Namespace

### Resource Limits

* CPU Cgroups
* Memory Cgroups

### Networking

* veth pairs
* Linux Bridge
* NAT
* Port Mapping

### Filesystem

* Custom RootFS (Alpine)

---

## Architecture

Host
|
+-- Bridge (mini0)
|       |
|       +-- container A
|       +-- container B
|
+-- NAT
|
+-- Internet

Each container runs inside isolated Linux namespaces while sharing the host kernel.

---

## Build

```bash
gcc container.c rootfs.c main.c cgroup.c cli.c network.c bridge.c nat.c userns.c -o mcrun
```

Run

```bash
sudo ./mcrun \
  --name web \
  --ip 10.0.0.2
```

Run With Port Mapping

```bash
sudo ./mcrun \
  --name web \
  --ip 10.0.0.2 \
  --host-port 8080 \
  --container-port 80
```

Example Tests

1. PID Namespace

Inside container:

```bash
ps
```

Expected:

```bash
PID 1
```

2. RootFS Isolation

```bash
ls /
```

Expected:
Only Alpine filesystem appears.

3. User Namespace

Inside:

```bash
id
```

Expected:

```bash
uid=0(root)
```

Host:

```bash
ps -o pid,uid,gid,cmd -p <host_pid>
```

Expected:

```bash
uid=1000
```

4. Network Namespace

Inside:

```bash
ip addr
```

Expected:

```bash
lo
web-cont
```

Host:

```bash
ip addr
```

Expected:

```bash
mini0
web-host
```

5. Container-to-Container Communication

Container A:

```bash
ping 10.0.0.3
```

Expected:
Successful replies.

6. Internet Access

Inside:

```bash
ping 8.8.8.8
```

Expected:
Successful replies through NAT.

7. Port Mapping

Inside container:

```bash
nc -l -p 80
```

Host:

```bash
echo hello | nc <host-ip> 8080
```

Expected:
hello appears inside container.

Future Work

* Volumes
* Capabilities
* Seccomp
* Image Management
* Registry
* OCI Compatibility
* Container Metadata Store
* Container Lifecycle Management

Educational Goal

Understand how Docker works internally by rebuilding core container technologies from Linux primitives.
