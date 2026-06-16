# Testing Guide

This document demonstrates every feature implemented in the runtime and how to verify it.

---

# Build

```bash
gcc container.c rootfs.c main.c cgroup.c cli.c network.c bridge.c nat.c userns.c -o mcrun
```

---

# Start Container

```bash
sudo ./mcrun \
    --name web \
    --ip 10.0.0.2
```

Expected:

```text
Inside container
PID inside namespace: 1
```

---

# Test 1 - PID Namespace

Inside container:

```bash
echo $$
```

Expected:

```text
1
```

or

```bash
ps
```

Expected:

```text
PID USER COMMAND
1 root sh
```

Host:

```bash
ps -ef | grep sh
```

Expected:

Process has a different PID on host.

This proves PID isolation.

---

# Test 2 - Root Filesystem Isolation

Inside container:

```bash
ls /
```

Expected:

Only Alpine rootfs files appear.

Host directories should not appear.

---

# Test 3 - User Namespace

Inside container:

```bash
id
```

Expected:

```text
uid=0(root)
gid=0(root)
```

Find host PID:

```bash
ps -ef | grep sh
```

Then:

```bash
ps -o pid,uid,gid,cmd -p <host_pid>
```

Expected:

```text
UID=1000
GID=1000
```

This proves:

Container root != Host root

---

# Test 4 - Network Namespace

Inside container:

```bash
ip addr
```

Expected:

```text
lo
web-cont
```

Host:

```bash
ip addr
```

Expected:

```text
mini0
web-host
```

Container interfaces are isolated from host interfaces.

---

# Test 5 - Container IP

Inside container:

```bash
ip addr show web-cont
```

Expected:

```text
10.0.0.2/24
```

---

# Test 6 - Default Gateway

Inside container:

```bash
ip route
```

Expected:

```text
default via 10.0.0.1
10.0.0.0/24 dev web-cont
```

Bridge acts as gateway.

---

# Test 7 - Bridge

Host:

```bash
bridge link
```

Expected:

```text
web-host master mini0
```

This proves the host side veth is attached to the bridge.

---

# Test 8 - Multiple Containers

Terminal 1:

```bash
sudo ./mcrun --name web --ip 10.0.0.2
```

Terminal 2:

```bash
sudo ./mcrun --name db --ip 10.0.0.3
```

Inside web:

```bash
ping 10.0.0.3
```

Expected:

Successful replies.

Inside db:

```bash
ping 10.0.0.2
```

Expected:

Successful replies.

This proves bridge switching works.

---

# Test 9 - NAT

Inside container:

```bash
ping 8.8.8.8
```

Expected:

Successful replies.

Host:

```bash
sudo iptables -t nat -L POSTROUTING -n -v
```

Expected:

MASQUERADE rule exists.

This proves outbound internet access through NAT.

---

# Test 10 - Port Mapping

Start container:

```bash
sudo ./mcrun \
  --name web \
  --ip 10.0.0.2 \
  --host-port 8080 \
  --container-port 80
```

Inside container:

```bash
nc -l -p 80
```

Host:

```bash
echo hello | nc <HOST_IP> 8080
```

Expected:

Inside container:

```text
hello
```

This proves DNAT port forwarding works.

---

# Test 11 - Cgroups CPU

Host:

```bash
cat /sys/fs/cgroup/web/cpu.max
```

Expected:

Configured CPU quota.

---

# Test 12 - Cgroups Memory

Host:

```bash
cat /sys/fs/cgroup/web/memory.max
```

Expected:

Configured memory limit.

---

# Cleanup

Delete bridge:

```bash
sudo ip link delete mini0
```

Remove NAT rule:

```bash
sudo iptables -t nat -F
```

Remove cgroup:

Automatically removed when container exits.

---

# Implemented Technologies

* clone()
* PID Namespace
* Mount Namespace
* User Namespace
* Network Namespace
* veth pairs
* Linux Bridge
* NAT
* DNAT Port Mapping
* CPU Cgroups
* Memory Cgroups
* chroot RootFS Isolation
* Alpine RootFS
