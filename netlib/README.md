# netlib

Minimal userspace networking kit for UniPlus+ V.1.5+ on the Apple Lisa, reconstructed from the surviving kernel source in `../v1.5/sys`.

The kernel's network stack is **4.1a BSD**, not 4.2BSD. There is no `bind`, `listen`, `shutdown`, `sendto` or `recvfrom`:

- **bind:** pass a local address as the 3rd argument of `socket()`.
- **listen:** pass `SO_ACCEPTCONN` as the 4th argument of `socket()`.
- **accept:** returns 0 once the listening socket *itself* is connected. No new descriptor is created.
- **send/receive data:** use `read`/`write`, or `send`/`receive(fd, addr, buf, len)`.

## Files

| File | Description |
|---|---|
| `include/net/socket.h` | `sockaddr`, `sockproto`, socket types, option flags, syscall declarations |
| `include/net/in.h` | `in_addr`, `sockaddr_in`, protocol numbers, `htons`/`htonl` (no-ops on the 68000) |
| `include/net/route.h` | `struct rtentry` and `RTF_` flags for `SIOCADDRT`/`SIOCDELRT` |
| `include/sys/socket.h`, `include/netinet/in.h` | One-line aliases for the `net/` headers |
| `sockcall.s` | `trap #0` stubs for syscalls 70–79: `select`, `gethostname`, `sethostname`, `socket`, `accept`, `connect`, `receive`, `send`, `socketaddr`, `netreset` |
| `tcpconn.c` | Test client: `tcpconn a.b.c.d port [message]` |
| `tcpecho.c` | Test server: `tcpecho port`, echoes one connection at a time (loopback: `tcpecho 5000 &` then `tcpconn 127.0.0.1 5000 hello`) |
| `looptest.c` | One-shot loopback test: forks an echo server, connects to 127.0.0.1, checks the echo and prints PASS or FAIL (`looptest [port]`) |
| `udptest.c` | One-shot UDP loopback test: forks a datagram echo server, sends to 127.0.0.1 with `send()`, checks the reply from `receive()` and prints PASS or FAIL (`udptest [port]`) |
| `netinfo.c` | Shows the host name (`gethostname`) and the Internet address compiled into the kernel (`SIOCGIADDR`) |
| `ping.c` | ICMP echo on a raw socket: `ping [-d] [-w seconds] a.b.c.d [count]`, run as root. `-w` is how long to wait for each reply (default 2); late replies still count and duplicates are marked `(DUP!)`. `-d` turns on the kernel's ICMP console messages and shows the raw input queue. Needs a kernel with the raw socket fixes (see below). |
| `route.c` | Routing table: `route [show]`, `route add dest gateway`, `route delete dest gateway`; `dest` is an address or `default`. `show` reads the kernel's `rthost`/`rtnet` tables through `/unix` and `/dev/kmem`; add and delete need root. A default route needs the `rtalloc()` fallback in `../v1.5/sys/route.c`. |
| `nc.c` | Netcat: `nc [-v] [-c] [-q] [-w secs] a.b.c.d port`, or `-l port` to listen; `-u` for UDP |
| `httpd.c`, `www/index.html` | Web server: `httpd [-p port] [docroot]`, `.html`/`.htm`/`.txt` from `/usr/www` |
| `ifconfig.c` | `ifconfig [interface]` lists interfaces from the kernel; `ifconfig eb0 a.b.c.d` sets the address (root) |
| `Makefile` | Builds `tcpconn`, `tcpecho`, `looptest`, `udptest`, `netinfo`, `ping`, `route`, `nc`, `httpd` and `ifconfig` on the Lisa |

## Routes

The kernel adds a network route for each interface (10.0.0.0 on `eb0`, 127.0.0.0 on `lo0`). To reach anything else through LisaEm's slirp network, add a default route to slirp's host address:
```
route add default 10.0.2.2
```
Routes are not kept across reboots; put that line in `/etc/rc` to make it permanent.

`route` follows 4.1a BSD: a destination whose host part is non-zero for its address class is a host route, anything else a network route, and the route goes through a gateway unless the gateway is one of the Lisa's own addresses. The 4.1a kernel only matched host and network routes; the default route is a later addition to `rtalloc()` (`../RESTORATION.md` section 9).

## Raw sockets and ping

A raw ICMP socket needs the fixes in `../v1.5/sys` `proto.c`, `raw_cb.c`, `raw_usrreq.c`, `raw_ip.c` and `ip_icmp.c` (`../RESTORATION.md` section 9). On a kernel without them, `socket()` for raw ICMP calls a null pointer and crashes the system. `ping` therefore reads the running kernel's protocol table through `/unix` and `/dev/kmem` first and refuses to run if the raw ICMP entry is missing. `/unix` must be the kernel that is running.

In a received echo reply the kernel has stripped the IP header and zeroed the ICMP checksum. `icmp_input()` treats the first data byte as the header length of a quoted IP packet and drops replies too short for it, so `ping` sends a 0 there.

## Status

- **Confirmed:**
  - Struct layouts and syscall numbers come from the kernel source.
  - The register calling convention matches the stubs in the shipped `/lib/libc.a` (see `../dump`).
  - Network errno values (55–85) are already in the stock `<sys/errno.h>`, and `perror()` knows their text.
- **Unverified:** constants tagged `[B]` in the headers (`SOCK_STREAM`, `AF_INET`, `SO_*`) are taken from 4.1a/4.1c BSD. The socket ioctls `FIONBIO`, `SIOCGIADDR` and `SIOCCIADDR` come from UniSoft's `net/misc.h` (Torch headers) and match `soioctl()` in the kernel.
- **Tested on a Lisa (13 September 2026):** on `unix.net`, `tcpconn 127.0.0.1 5000 hello` builds and gets "Connection refused" from the loopback TCP stack, which exercises the headers, `sockcall.s`, `socket()` and `connect()`. `looptest` passes: a full TCP round trip over 127.0.0.1 (fork, listening socket with `SO_ACCEPTCONN`, `accept`, `connect`, `read`/`write` both ways, `close`). `udptest` passes: a datagram sent with `send()` from an ephemeral port (1025) and echoed back with `receive()` returning the sender's address. `netinfo` prints `hostnameunknown` and 89.0.41.8 through `gethostname()` and `SIOCGIADDR` (10.0.2.15 on kernels built with the current `conf.c`).
- **Tested over Ethernet (LisaEm EtherBox with slirp):** `tcpconn` from the Lisa to a listener on the Mac, `tcpecho` on the Lisa reached from the Mac through a port forward, and `ping` to 127.0.0.1 and 10.0.2.2. With `route add default 10.0.2.2`, `ping 8.8.8.8` gets 4 of 4 replies from the internet at 5 MHz. At much higher emulated speeds the Lisa's clock runs ahead of real time and replies come back after ping's wait.
- **Lisa `cc` limit:** struct tags are unique only to 8 characters, so `in.h` maps `sockaddr_in` to `sock_in`, as the kernel's `net/misc.h` does. Keep new identifiers unique within 8 characters, and external names within 7. `cc` truncates external names to 7 characters plus `_`, but `as` keeps labels whole, so `sockcall.s` spells the long calls as `cc` emits them (`_gethost`, `_sethost`, `_socketa`, `_netrese`).

If `sockcall.s` won't assemble, libc's generic `syscall()` works instead, e.g. `syscall(73, SOCK_STREAM, 0, 0, 0)`.
