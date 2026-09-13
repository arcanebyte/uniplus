# bsd

Reference sources from Berkeley Unix of the same era as the UniPlus+ network stack, used to reconstruct its missing headers and userland. Nothing here builds as-is on the Lisa.

The UniPlus+ V.1.5+ network code (`../v1.5/sys`) is 4.1a BSD derived (`ipc.c 4.20 82/06/20`, `tcp_usrreq.c 1.59 82/06/20`). It uses the pre-4.2 socket API: `socket(type, sockproto*, sockaddr*, options)`.

## Contents

### `2.9BSD/` — the closest match (PDP-11)

- **Source:** `https://www.tuhs.org/Archive/Distributions/UCB/2BSD/2.9BSD/usr.tar.gz`, downloaded 2026-09-12.
- **`usr.tar.gz`:** the original archive, unmodified.
- **`usr/net/`, `usr/include/`:** extracted from that archive for browsing.

The 2.9BSD network kit is the same 4.1a code base, ported to the PDP-11 by UT Austin/BBN/Berkeley, with the same flat `net/` layout:

| Path | What it holds |
|---|---|
| `usr/net/sys/net/*.h` | 33 of the kernel headers UniPlus is missing (`in.h`, `if.h`, `route.h`, `in_pcb.h`, `ip*.h`, `tcp*.h`, `udp*.h`, `af.h`, `raw_cb.h`, …). Not present: `socket.h`, `socketvar.h`, `protosw.h`, `mbuf.h`, the network `misc.h`, `ubavar.h`. |
| `usr/net/sys/sys/` | `ipc.c`, `socket.c`, `socketsubr.c`, `mbuf.c`, `proto.c`, `sysent.c`, `syslocal.c` |
| `usr/include/sys/ioctl.h` | Socket ioctls under `#ifdef UCB_NET`: `FIONBIO`, `FIOASYNC`, `SIOCDONE` … `SIOCCHGRT`, in the `('s'<<8)\|n` encoding |
| `usr/net/man/man2/socket.2X` | `socket(2X)` man page, 3/17/82, "4.1a Provisional" |
| `usr/net/src/net/` | `netsys.s` (PDP-11 socket syscall stubs), `in.h`, `netdb.h`, host lookup library |
| `usr/net/src/netser/` | 4.1a-API `telnet`/`telnetd`, `ftp`, `rlogin`, `rsh`, `tftp`, `rwho`, `netstat`, `routed`, `implog` |

(`usr/include/misc.h` is an unrelated PDP-11 header, not the network `misc.h`.)

### `4.1c.2/` — the socket-layer headers 2.9BSD lacks (VAX)

- **Source:** `https://github.com/dspinellis/unix-history-repo`, branch `BSD-4_1c_2`, fetched 2026-09-12.

| File | Version |
|---|---|
| `a/sys/h/socket.h` | 4.25 83/01/22 — still the old `sockproto` API |
| `a/sys/h/socketvar.h` | 4.19 82/07/24 |
| `a/sys/h/protosw.h` | 4.14 82/11/13 |
| `a/sys/h/mbuf.h` | 4.17 83/01/17 |
| `a/sys/h/ioctl.h` | Newer `_IOW()` encoding; **not** the UniPlus-era numbering (use 2.9BSD's) |
| `a/sys/netinet/in.h` | 4.20 83/01/17 |

These are a few months newer than the UniPlus code, so compare them against what the kernel actually uses. For example, 4.1c defines `SO_DONTLINGER` as `~SO_LINGER`, but the UniPlus kernel uses it as a single flag bit.

### `4.1aBSD/` — 4.1a userland (VAX)

- **Source:** `https://archive.org/download/4.1aBSD/4.1aBSD.tar.gz`, downloaded 2026-09-12. Original archive, unmodified.
- **What's in it:** `doc`, `games`, `ingres`, `lib`, and `src/cmd` (berknet, fed, learn, …).
- **What isn't:** no kernel sources and no network headers. It's mainly useful as a source of K&R-era programs.

## Other known locations (not downloaded)

- **4.1cBSD, browsable:** https://www.tuhs.org/cgi-bin/utree.pl?file=4.1cBSD
- **2.9BSD complete simh image,** which may contain `socket.h`: https://www.tuhs.org/Archive/Distributions/UCB/2BSD/2.9BSD_MSCP/
- **UniSoft kernel listing:** https://bitsavers.org/bits/Unisoft/V.1.0/Unisoft_V.1.0_kernel_listing.pdf (a copy is in `../v1.0/`)
- **Motorola SYSTEM V/68 R1V2.8** floppy images (a UniSoft port): https://bitsavers.org/bits/Motorola/VME_10/SysV128/
