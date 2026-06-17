# Testing Guide

This document demonstrates every feature implemented in the runtime and how to verify it.

---

# Build

```bash
gcc \
container.c \
rootfs.c \
overlay.c \
main.c \
cgroup.c \
cli.c \
network.c \
bridge.c \
nat.c \
userns.c \
-o mcrun
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

or:

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

Process has a different PID on the host.

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

This proves filesystem isolation through chroot() and mount namespaces.

---

# Test 3 - OverlayFS

Inside container:

```bash
touch hello.txt

echo "overlay works" > hello.txt

cat hello.txt
```

Expected:

```text
overlay works
```

Host:

```bash
ls rootfs/containers/web/upper
```

Expected:

```text
hello.txt
```

Verify the file does NOT exist in the base Alpine image:

```bash
ls rootfs/alpine/hello.txt
```

Expected:

```text
No such file or directory
```

This proves:

* Writes go to the Upper Layer.
* Base image remains unchanged.
* Copy-on-write behavior works correctly.

---

# Test 4 - User Namespace

Inside container:

```bash
id
```

Expected:

```text
uid=0(root)
gid=0(root)
```

(or similar depending on group mappings)

Find host PID:

```bash
ps -ef | grep sh
```

Host:

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

# Test 5 - User Mapping

Inside container:

```bash
id
```

Expected:

```text
uid=0(root)
```

Host:

```bash
ps -ef | grep sh
```

Find the shell process PID.

Then:

```bash
ps -o pid,uid,gid,cmd -p <pid>
```

Expected:

```text
UID=1000
```

Same process.

Different identities.

This demonstrates Linux User Namespace mapping.

---

# Test 6 - Network Namespace

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

# Test 7 - Container IP

Inside container:

```bash
ip addr show web-cont
```

Expected:

```text
10.0.0.2/24
```

---

# Test 8 - Default Gateway

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

# Test 9 - Linux Bridge

Host:

```bash
bridge link
```

Expected:

```text
web-host master mini0
```

This proves the host-side veth is attached to the bridge.

---

# Test 10 - Multiple Containers

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

This proves Layer-2 communication through the bridge.

---

# Test 11 - Internet Access

Inside container:

```bash
ping 8.8.8.8
```

Expected:

Successful replies.

This proves:

* veth pair works
* bridge works
* routing works
* NAT works

---

# Test 12 - NAT

Host:

```bash
sudo iptables -t nat -L POSTROUTING -n -v
```

Expected:

A MASQUERADE rule exists for:

```text
10.0.0.0/24
```

This proves outbound internet traffic is translated through the host interface.

---

# Test 13 - Port Mapping

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
busybox httpd -f -p 80
```

Host:

```bash
curl http://<HOST_IP>:8080
```

Expected:

HTML output or a directory listing.

This proves DNAT port forwarding works.

Equivalent concept:

```bash
docker run -p 8080:80
```

---

# Test 14 - CPU Cgroup

Run container:

```bash
sudo ./mcrun \
  --name web \
  --ip 10.0.0.2 \
  --cpu 50
```

Host:

```bash
cat /sys/fs/cgroup/web/cpu.max
```

Expected:

A configured CPU quota.

This proves CPU resource limits are applied.

---

# Test 15 - Memory Cgroup

Run container:

```bash
sudo ./mcrun \
  --name web \
  --ip 10.0.0.2 \
  --memory 256
```

Host:

```bash
cat /sys/fs/cgroup/web/memory.max
```

Expected:

```text
268435456
```

(or equivalent value)

This proves memory limits are applied.

---

# Cleanup

Delete bridge:

```bash
sudo ip link delete mini0
```

Flush NAT rules:

```bash
sudo iptables -t nat -F
```

Remove OverlayFS directories (optional):

```bash
sudo rm -rf rootfs/containers/web
sudo rm -rf rootfs/containers/db
```

Cgroups are automatically removed when containers exit.

---

# Implemented Technologies

* clone()
* PID Namespace
* Mount Namespace
* User Namespace
* Network Namespace
* chroot()
* Alpine RootFS
* OverlayFS (Copy-on-Write Filesystem)
* veth pairs
* Linux Bridge
* NAT (MASQUERADE)
* DNAT Port Mapping
* CPU Cgroups v2
* Memory Cgroups v2

---

# Expected Learning Outcomes

After completing all tests, you will have verified:

* Process isolation
* Filesystem isolation
* Copy-on-write filesystems
* User identity mapping
* Network isolation
* Virtual Ethernet devices
* Linux bridge switching
* Internet connectivity through NAT
* Host-to-container port forwarding
* CPU resource control
* Memory resource control

These are the same Linux kernel primitives used by modern container runtimes such as Docker, containerd, and CRI-O.
