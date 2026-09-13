# UniPlus+ networking and kernel restoration — history and status

This page records the effort, started September 2026, to restore the networking stack of UniSoft UniPlus+ System V on the Apple Lisa: what we set out to do, what we found, what was built, where things stand, and what comes next. It links to the more detailed documents in the repo instead of repeating them.

> **Status in one line (13 September 2026):** the V.1.5+ network kernel builds on a (LisaEm) Lisa and boots, and a full TCP round trip over loopback passes (`netlib/looptest`). That needed faithful ProFile and 6522 VIA emulation in LisaEm (lisaem PR #55), which replaced LisaEm's UniPlus-specific hacks. Next: the dual parallel card and the remaining Lisa OS regression tests for the emulation, and networking beyond loopback.

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
- **`tools/put_profile_files.py`:** adds or replaces files in an existing filesystem on an image, in place, allocating like the kernel and checking the filesystem before and after. This is how files reach the Lisa, since LisaEm can't paste.
- **`tools/make_profile_image.py`:** builds a ProFile image with a new filesystem from host folders, and self-verifies. Options:
  - `--base-image`, `--disk-sectors`, `--start`, `--blocks` build a larger system disk around an existing one;
  - `--prlmap LETTER` patches that kernel partition table entry;
  - `--replace-partition` regenerates the source partition.
- **LisaEm fix (committed, lisaem `00bc0f4`):** moving one serial port off Loopback now moves the other to Nothing with an alert, instead of silently reverting PseudoTTY.
- **Serial file transfer:** `uucp.md` covers UUCP over the PseudoTTY. A LisaEm constant (`SCC_MIN_CYCLES_BETWEEN_READS` in `z8530.c`) deliberately limits incoming Serial B data to about one byte per emulated MHz per second.
- **Build disk:**
  - **Tried first:** a separate 10 MB source ProFile on a dual parallel card. I/O on slot-card ProFiles hung under LisaEm (both ports, block and raw); not yet retested on the new emulation (section 8).
  - **What works:** a **20 MB system disk** (`lisa-build.md` section 0). The first 10 MB is the system disk; the second is a source filesystem in partition `e` (sector 19456, 19456 blocks), mounted from `/dev/p0e` after changing `prlmap[e]` in `/unix` from `{0,0}` to `{19456,19456}`. This boots and works. `v1.5/sys/pro.c` now has that entry too, so rebuilt kernels see partition e.
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

## 7. Why the new kernels didn't boot: LisaEm's UniPlus hacks (resolved in section 8)

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

## 8. Faithful ProFile emulation in LisaEm

Research and design: `profile-emulation-notes.md`, `profile-emulation-plan.md`. Code: lisaem branch `profile-emulation`, **PR #55** (https://github.com/arcanebyte/lisaem/pull/55). Test status there: `ProFileEmulationTesting.md`.

**Exact failure:** `prochk` asserts that /BSY is high (drive not busy) before each exchange. LisaEm's drive held /BSY low while the six command bytes arrived and after each transfer. 1.4 only survived because LisaEm patched that assertion out of 1.4's RAM.

**What changed in LisaEm:**
- **Drive (`profile.c`):** level-driven state machine following the ProFile protocol (ESProFile/Aphid):
  - reply to /CMD low with $01/$02–$04/$06 and /BSY low;
  - sample $55 when /CMD rises;
  - stay busy for a scheduled time (`clock_e`, now a real timer event in `irq.c`), then raise /BSY once;
  - /BSY high whenever the drive waits for the Lisa, no drive-side timeouts, one byte per /PSTRB.
- **VIA (`via6522.c`), parallel port VIAs only:** 6522 datasheet behaviour:
  - CA1 latched only on a BSY edge, regardless of IER; no flag side effects from IFR reads, PCR writes or IER writes;
  - register 1 clears CA1 and strobes only in CA2 handshake/pulse mode; register 15 has no handshake;
  - interrupts are taken as soon as IFR & IER is non-zero.
- **`hle.c`:** the 1.4 and `sunix` BSY-assert and timeout RAM patches, the UniPlus loader handshake patches, and the UniPlus/Xenix BSY/CA1 fakes are gone.
- **Checked:** the H boot ROM's ProFile code uses CA2 pulse mode through register 1, matching the new strobe rules.

**Results with the new emulation** (built-in port, "Hard drive acceleration" off):

| Kernel / OS | Result |
|---|---|
| V.1.5+ `unix.nonet` | Boots. `find`, `sum`, `cp`/`cmp` round trips pass. |
| V.1.5+ `unix.net` | Boots. Mounts partition e. `tcpconn 127.0.0.1 5000` gets "Connection refused" from loopback TCP. |
| UniPlus 1.4 `/unix` (now `/unix.orig` on `build2`) | Boots without the removed RAM patches |
| LOS 3.1 | Opens (its HLE read/write loops always run, see below) |

**Two kernel-side issues found once the kernel booted:**
- **Partition e:** `pro.c`'s `prlmap[]` had e = `{0, 0}`, so kernels built from source rejected every `/dev/p0e` block ("read error"). Fixed in `pro.c` (`{19456, 19456}`), and the 2 bytes patched into `/unix` on `build2` (backup: `uniplus_unix_20mb.build2.before-prlmap-patch.image`).
- **8-character struct tags:** the Lisa `cc` keeps only 8 characters of a struct tag, so netlib's `sockaddr_in` collided with `sockaddr`. Fixed with `#define sockaddr_in sock_in`, as the kernel's `net/misc.h` does.

## 9. Current state

**uniplus repo:** everything through the netlib `sock_in` fix is committed (link fixes, `unix.pad` diagnostic, `make_profile_image.py`, `lisa-build.md`, research docs, `tcpecho.c`, `pro.c` partition e). Only the disk images are untracked.

**lisaem:** branch `profile-emulation`, PR #55 (open, review required):
- `00bc0f4` Loopback serial-port fix
- `a2cd251` drive and timer
- `12dd3f0` VIA flags
- `170e18d` UniPlus patch removal
- testing doc

Test build: `~/github/lisaem/bin/LisaEm-profile.app`. `bin/LisaEm.app` is the master build.

**Disk images in `~/Documents/LisaEm Files`:**

| Image | State |
|---|---|
| `uniplus_unix_20mb.build2.image` | **Current.** `/unix` = `unix.net` with partition e patched; `/unix.orig` = 1.4. Partition e has the sources, objects, `unix.net`/`unix.nonet`/`unix.pad` (old prlmap), logs, `netlib` (on-disk `in.h` has the `sock_in` fix only if it was edited on the Lisa). |
| `uniplus_unix_20mb.build2.before-prlmap-patch.image` | Backup from just before the `/unix` partition e patch |
| `uniplus_unix_20mb.original.image` | Earlier build (before the link fixes) |
| `uniplus_unix_20mb.test*.image` | Throwaway test copies; `test2` has a damaged root inode |
| `uniplus_unix_10mb_testing.image` | Your 10 MB system disk |

## 10. Next steps

**Networking (uniplus):**
1. **Loopback TCP and UDP: done.** `netlib/looptest` (TCP, 51 bytes round trip) and `netlib/udptest` (UDP datagram with addresses) pass on `unix.net` over 127.0.0.1; `netlib/netinfo` reads the host name and the configured address (`SIOCGIADDR`).
2. **Rebuild `unix.net` from the fixed source:** partition e now has the fixed `pro.c` (written with `tools/put_profile_files.py`). Rebuild and install it so `/unix` no longer depends on the 2-byte image patch.
3. **Etherbox emulation in LisaEm:** the register-level plan, libslirp backend and a private IP in `conf.c` are in `etherbox-emulation-plan.md`.
4. **Port network tools:** try the Torch 4.1a binaries, port 2.9BSD `netstat`.

**LisaEm (PR #55)**, full list in `ProFileEmulationTesting.md`:
1. **Dual parallel card:** ProFile read/write and boot. It used to hang; not yet tried on the new emulation.
2. **LOS 3.1 on the full emulation path:** first make `apply_los31_hacks()` respect "Hard drive acceleration" (`if (!los31_hle || !hle) return;`, not done).
3. **Other OSes:** LOS 1.x/2.x, Workshop, MacWorks, Xenix, `sunix`, LisaTest; other boot ROMs; UniPlus 1.4 with HLE on.
4. **Build quirk:** `build.sh` fails in-tree while a git worktree sits under `.claude/`; build from a copy (workaround in the testing doc).
