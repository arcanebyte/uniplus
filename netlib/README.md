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
| `include/sys/socket.h`, `include/netinet/in.h` | One-line aliases for the `net/` headers |
| `sockcall.s` | `trap #0` stubs for syscalls 70–79: `select`, `gethostname`, `sethostname`, `socket`, `accept`, `connect`, `receive`, `send`, `socketaddr`, `netreset` |
| `tcpconn.c` | Test client: `tcpconn a.b.c.d port [message]` |
| `tcpecho.c` | Test server: `tcpecho port`, echoes one connection at a time (loopback: `tcpecho 5000 &` then `tcpconn 127.0.0.1 5000 hello`) |
| `Makefile` | Builds `tcpconn` and `tcpecho` on the Lisa |

## Status

- **Confirmed:**
  - Struct layouts and syscall numbers come from the kernel source.
  - The register calling convention matches the stubs in the shipped `/lib/libc.a` (see `../dump`).
  - Network errno values (55–85) are already in the stock `<sys/errno.h>`, and `perror()` knows their text.
- **Unverified:** constants tagged `[B]` in the headers (`SOCK_STREAM`, `AF_INET`, `SO_*`) are taken from 4.1a/4.1c BSD. The ioctl numbers (`SIOC*`, `FIONBIO`) are unknown and left out.
- **Not run yet:** nothing here has been tested on a Lisa. It needs a kernel built as `unix.net`; the installed `/unix` has no networking. Building `unix.net` requires recreating the missing `net/*.h` kernel headers.

If `sockcall.s` won't assemble, libc's generic `syscall()` works instead, e.g. `syscall(73, SOCK_STREAM, 0, 0, 0)`.
