# UniPlus+ networking and kernel restoration — history and status

This page records the effort, started September 2026, to restore the networking stack of UniSoft UniPlus+ System V on the Apple Lisa: what we set out to do, what we found, what was built, where things stand, and what comes next. It links to the more detailed documents in the repo instead of repeating them.

> **Status in one line:** the V.1.5+ network kernel now builds cleanly on a (LisaEm) Lisa. It doesn't boot yet, because LisaEm's ProFile emulation only works with the stock UniPlus 1.4 kernel, through address-specific hacks. Next step: make those hacks address-independent (option 2), then make the ProFile emulation faithful (option 1).

## 1. Starting point

**Goal:** understand how UniPlus+ applications used the networking stack, reconstruct enough of the userspace interface to write native IPv4 programs, and eventually get a network-capable kernel running on the Lisa.

**What we had:**
- **`v1.5/sys`**, the Bitsavers UniSoft V.1.5+ kernel source, with a BSD-derived network stack (`netipc.c`, `socket.c`, `in_pcb.c`, `tcp_*.c`, `udp_usrreq.c`, `if_eb.c`, …). Its Makefile builds `unix.net` with `-DUCB_NET`.
- **No network headers at all** (`net/socket.h`, `net/in.h`, `net/mbuf.h`, …), and several generic kernel headers missing.
- **No userspace network pieces** (libc stubs, `ifconfig`, `telnet`, …) in the known sources.
- **A working 10 MB UniPlus install image** (`uniplus_unix_10mb_ai_copy.image`) running a non-network 1.4 kernel.

## 2. The network API

Details: `netlib/README.md`.

- **It's 4.1a BSD, not 4.2BSD.** `netipc.c` is Berkeley's `ipc.c 4.20 82/06/20`. There are no `bind`, `listen`, `shutdown`, `sendto` or `recvfrom` calls:
  - `socket(type, sockproto*, sockaddr* local, options)`: a local address binds, and `SO_ACCEPTCONN` makes it a listening socket;
  - `accept(fd, sockaddr*)` returns 0 once the *listening socket itself* is connected, rather than a new descriptor;
  - `read`/`write`/`close`/`ioctl` work on sockets.
- **System calls 70–79:** `select`, `gethostname`, `sethostname`, `socket`, `accept`, `connect`, `receive`, `send`, `socketaddr`, `netreset`.
- **Calling convention:** `trap #0`, D0 = call number, arguments in A0/D1/A1/D2/A2/D3, carry set on error with errno in D0.
- **Confirmed two ways:** by the shipped `/lib/libc.a` stubs (`read.o`, `ptrace.o`, `syscall.o`), and later by UniSoft's own `libnet.a` (`netsys.o`) on a Torch Triple X disk.
- **Network errno values (55–85)** are already in the stock `<sys/errno.h>`, and `perror()` knows them.
- **Ethernet driver:** `if_eb.c` drives a 3Com EDLC-based "etherbox" through a Lisa parallel port VIA, at port 5 with IP `89.0.41.8` compiled into `conf.c`. Net 89 was UC Berkeley's Computer Center Ethernet (2.9BSD `hosts` file). With no etherbox the kernel still has loopback, 127.0.0.1.

**Built:** `netlib/` (socket headers, `sockcall.s` syscall stubs, `tcpconn.c` test client).

## 3. Finding the missing sources

| Source | What it gave us | Where |
|---|---|---|
| Lisa disk image | Generic `/usr/include`, libc, ABI confirmation, `sys/errno.h` network values, `setparams`/`eject` binaries (Lisa ioctl numbers) | `dump/` |
| V.1.0 source and 1986 kernel listing PDF | Missing `mch.s`, `ivec.s`, `tcp_usrreq.c` (identical to V.1.5+) | `v1.0/`, `v1.0/README.md` |
| 2.9BSD network kit (PDP-11) | Same 82/06/20 Berkeley base; `ioctl.h` socket ioctls; 4.1a-API `telnet`/`ftp`/`netstat` sources | `bsd/2.9BSD`, `bsd/README.md` |
| 4.1c.2 BSD headers, CSRG SCCS history | Confirmed `socket.h` 4.16 82/06/08 values; exact Berkeley revisions for comparison | `bsd/4.1c.2`, https://github.com/robohack/ucb-csrg-bsd |
| **Torch Triple X UniPlus+ disks (Bitsavers)** | **The actual lost headers:** all 34 `include/net/*.h` in UniPlus layout (Oct 1984), `sys/scat.h`, `sys/context.h`, `sys/uioctl.h`; `libnet.a`; 4.1a-API `telnet`/`ftp`/`rlogind` binaries | `torch/`, `torch/README.md` |
| UniPlus Lisa-specific manual (Bitsavers) | `console(5L)`, `mouse(5L)`, `speaker(5L)` — names, `struct speaker` | referenced in `v1.5/include/PROVENANCE.md` |

## 4. Reconstructing the kernel headers

Details: `v1.5/include/PROVENANCE.md`. Every file is labelled ORIGINAL, MODIFIED or RECONSTRUCTED, and each reconstructed value is tagged CONFIRMED, INFERRED or GUESS.

- **ORIGINAL:** Torch `net/*.h`, `scat.h`, `context.h`, `uioctl.h`. UniSoft's `net/misc.h` already carries the network additions (`FSOCKET`, `SSEL`, `fd_set`, `SIOC*` including UniSoft's `SIOCCIADDR`/`SIOCGIADDR`), so no generic header needed changing.
- **RECONSTRUCTED:**
  - **`sys/config.h`:** block majors `PR0`/`SN1`/`CV2`/`PM3` = 0–3, `NSC` 2, `NTE` 4, `CONSOLE` 0.
  - **`sys/al_ioctl.h`:** 24 of 27 `AL_*` values decoded from `setparams`/`eject`; 3 guesses.
  - **`sys/speaker.h`:** straight from the manual.
  - **`v1.5/sys/cxstub.c`** (a source file, not a header): empty `cxrelse`/`cxtxfree`; the real ones in `context.c` are for another MMU and are no-ops on the Lisa.
- **Not needed** (behind `#ifdef`s that are off for the Lisa): `istk.h`, `macro.h`, `page.h`, `fptrap.h`, `nd.h`, `domain.h`.
- **Makefile:** `-Dm68000` added, because Torch `ip.h`/`ip_var.h` test `m68000` but the Lisa `cpp` only predefines `mc68000`.
- **Version match:** the Lisa network sources and the Torch headers share the same mix, an 82/06 Berkeley base plus 83/02 updates plus UniSoft 84/05 changes.
- **Host checking:** the Mac's `clang` (`-fsyntax-only`, `-Wundef`) was used only to find unresolved names. Nothing is compiled on the Mac.

## 5. Tools and LisaEm setup

- **`tools/extract_profile_image.py`:** read-only extractor for UniPlus System V images: Lisa ProFile (532-byte tagged sectors, 512-byte blocks) and Torch (512-byte sectors, 1 K blocks), including multi-filesystem disks.
- **`tools/make_profile_image.py`:** builds a ProFile image with a new filesystem from host folders, and self-verifies. Options:
  - `--base-image`, `--disk-sectors`, `--start`, `--blocks` build a larger system disk around an existing one;
  - `--prlmap LETTER` patches that kernel partition table entry;
  - `--replace-partition` regenerates the source partition.
- **LisaEm fix (committed, lisaem `00bc0f4`):** moving one serial port off Loopback now moves the other to Nothing with an alert, instead of silently reverting PseudoTTY.
- **Serial file transfer:** `uucp.md` covers UUCP over the PseudoTTY. A LisaEm constant (`SCC_MIN_CYCLES_BETWEEN_READS` in `z8530.c`) deliberately limits incoming Serial B data to about one byte per emulated MHz per second.
- **Build disk:**
  - **Tried first:** a separate 10 MB source ProFile on a dual parallel card. I/O on slot-card ProFiles hangs under LisaEm (both ports, block and raw).
  - **What works:** a **20 MB system disk** (`lisa-build.md` section 0). The first 10 MB is the system disk; the second is a source filesystem in partition `e` (sector 19456, 19456 blocks), mounted from `/dev/p0e` after changing `prlmap[e]` in `/unix` from `{0,0}` to `{19456,19456}`. This boots and works.
- **Port mapping observed:** LisaEm slot 1 *high* is UniPlus `/dev/p2h`, and *low* is `/dev/p1h`.
- **Minor issues found along the way:**
  - `vi` "Input read error" on Esc: the `vtl` termcap arrow-key timeout; workaround `set notimeout`.
  - LisaEm maps `|` to `?` in ASCII keyboard mode.
  - UniPlus has no `ifconfig`; the address is compiled into `conf.c`.

## 6. Building the kernel on the Lisa

On the 20 MB disk, as root:
```
mount /dev/p0e /usr/src; cd /usr/src/sys
make GENnet=/usr/src/sys INCLUDE=-I/usr/src/include unix.nonet > build.log 2>&1
make GENnet=/usr/src/sys INCLUDE=-I/usr/src/include unix.net  > buildnet.log 2>&1
grep -n Undefined build.log buildnet.log     # UniSoft ld writes output even with undefined symbols
```

**Results:**
- **Compile:** everything compiles with the Lisa `cc` (`-OBPS -v`), including `typedef short void` and the unnamed union in `ip.h`.
- **First link:** undefined `_cxrelse`, `_cxtxfree` and the `_te_*` Tecmar tables. Fixed by `NTE 4` and `cxstub.c`; both links are now clean.
- **Sizes:**

  | Kernel | text + data + bss |
  |---|---|
  | stock 1.4 `/unix` | 100376 + 11524 + 80622 |
  | `unix.nonet` | 100060 + 11524 + 80622 |
  | `unix.net` | 143872 + 14120 + 112358 |

- **Comparison with 1.4** (disassembly with Capstone, symbol by symbol):
  - **Same symbol set** (1.4's extra `_FAKE` is an empty stub).
  - **ProFile and parallel-port driver code** is identical apart from relocated addresses.
  - **Configuration data** (root/swap devices, `prlmap`, `pro_da`, device counts) is identical.
  - **The only code differences** are the Lisa `cc` emitting PC-relative `bsr` instead of absolute `jsr` for calls within a file, and trivial source changes.

## 7. Why the new kernels don't boot: LisaEm's UniPlus hacks

**Symptoms:**
- **`unix.net` and `unix.nonet`:** at the first root-disk command, `ASSERTION ((devp->d_irb&BSY)==BSY) FAILED IN PROC prochk`, then `EXCESSIVE DISK DELAY`, then `panic: iinit`, every time and at any throttle (5 MHz included).
- **Padded `unix.nonet`:** `kpad.s` adds 316 bytes after `unix.o` so later code lands at 1.4's addresses. No assertion, but intermittent `failed to issue cmd to disk`, then `exec error`, then `panic: no fs`. One failed boot zeroed the root inode on a test disk, the result of a failed read being written back rather than misdirected writes.

**Ruled out along the way:** compiler register assumptions, configuration data, boot-block patching, header layout, bit-14 code placement.

**Root cause (LisaEm source, read-only):** LisaEm doesn't emulate the ProFile handshake well enough for UniPlus, and instead special-cases the 1.4 kernel by address.

1. **`src/storage/hle.c` `apply_uniplus_hacks()`**, triggered in `reg68k.c` (~line 3101) when `mch.s`'s 68010 `movec` probe traps:
   - reads the byte at `0x20f9c`: `0x67` means 1.4 (`0x60` means `sunix` 1.1, patched at `0x1fe24`/`0x1ff38`);
   - patches `0x20f9c` = `0x60`, skipping the BSY assertion in `prochk`;
   - patches `0x210b0` = `0x01`, a very long timeout;
   - puts an F-line trap at `0xc188` (idle speed-up);
   - only with HLE on: F-line read/write intercepts at `0x20c64`, `0x20d1e`, `0x20d3e`, `0x20ebc`, and a putchar hook at `0x236c6`.
2. **`src/lisa/motherboard/glue.c` `check_running_lisa_os()`:** sets `LISA_UNIPLUS_RUNNING` only if interrupt vectors 25/26 (longs at `0x64`/`0x68`) are `0x1c26c`/`0x1c270`, i.e. the kernel's `_dispatc` is at `0x1c208`.
3. **`src/storage/profile.c`:** only when UniPlus is detected, fakes `BSYLine=1` and forces the VIA CA1 interrupt flag (lines ~1116, ~1538, ~1639). There's also a boot-loader patch (~911–941, `uniplus_loader_patch`).

| Kernel | Byte at `0x20f9c` | `_dispatc` | Patched | Detected | Boot |
|---|---|---|---|---|---|
| stock 1.4 | `0x67` | `0x1c208` | yes | yes | works |
| `unix.nonet` | `0x00` | `0x1c0cc` | no | no | assertion, `panic: iinit` |
| `unix.net` | `0x6d` | `0x26b80` | no | no | same |
| `unix.pad` | `0x67` | `0x1c208` | yes | yes | intermittent disk failures |

The same emulation gap most likely explains why ProFiles on the dual parallel card hang under UniPlus.

## 8. Current state

**uniplus repo commits (newest first):**
- `16e00cb` Makefile `-Dm68000`
- `33a690b` kernel header set and `PROVENANCE.md`
- `2294ee7` Torch images and filesystems
- `2c56cae` extractor 1 K blocks
- `573fd56` v1.0 additions
- `8f8bde4` `bsd/` references
- `cb967f6` `uucp.md`
- `840e48d` v1.0 snapshot
- `e17c5cf` `dump/`
- `a938190` extractor
- `558bc2f` `netlib`

**Uncommitted in the uniplus repo:**
- `v1.5/include/sys/config.h` (`NTE`) and `v1.5/include/PROVENANCE.md` (link fixes, Lisa build results);
- `v1.5/sys/Makefile` (`cxstub.o`, diagnostic `unix.pad` target) and new `v1.5/sys/cxstub.c`, `v1.5/sys/kpad.s` (diagnostic);
- `tools/make_profile_image.py`, `lisa-build.md`, this file;
- untracked disk images.

**LisaEm:** `00bc0f4` committed; no other changes.

**Disk images in `~/Documents/LisaEm Files`:**

| Image | State |
|---|---|
| `uniplus_unix_20mb.build2.image` | Known good: 1.4 `/unix`, source partition with `unix.net`, `unix.nonet`, `unix.pad`, objects and logs |
| `uniplus_unix_20mb.original.image` | Earlier build (before the link fixes) |
| `uniplus_unix_20mb.test*.image` | Throwaway test copies; `test2` has a damaged root inode |
| `uniplus_unix_10mb_testing.image` | Your 10 MB system disk |

## 9. Next steps

**Option 2 — address-independent hacks in LisaEm (start here).** Keep the existing workarounds, but locate them by content instead of fixed addresses, so any UniPlus kernel build gets them:
1. **Detect UniPlus by a signature:** e.g. the UniSoft copyright or `oemmsg` string in kernel memory, or the `ivec.s` dispatch table pattern that vectors 25/26 point into, instead of `_dispatc == 0x1c208`.
2. **Find `prochk`'s BSY-assert branch and timeout constant by scanning** for their instruction patterns (1.4 context around `0x20f9c` and `0x210b0`), then patch there.
3. **Find the idle-loop and HLE intercept points** the same way, or disable them for non-1.4 kernels.
4. **Test:** 1.4 and `sunix` must still boot, and `unix.nonet`/`unix.net` from `build2` should boot without padding.

**Option 1 — faithful ProFile emulation (the goal).** Model the real ProFile handshake: BSY timing relative to CMD and data, 6522 CA1 edge and PCR polarity, and IFR set/clear on port A access, so UniPlus's interrupt-driven driver works unmodified on the built-in port and the dual parallel card. Then remove the UniPlus-specific patches, detection and BSY/CA1 fakes. Reference implementations, the protocol, and a list of specific LisaEm deficiencies are in `profile-emulation-notes.md`.

**After booting a network kernel:**
- **Loopback TCP/UDP test** with `netlib` (needs a small echo server and UDP test).
- **Etherbox emulation in LisaEm** (register-level spec in `if_eb.c`; slirp backend), which needs the slot-card VIA interrupt path working; set a private IP in `conf.c`.
- **Port network tools:** try the Torch 4.1a binaries, port 2.9BSD `netstat`.
