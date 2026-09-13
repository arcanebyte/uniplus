# Building the UniPlus+ V.1.5+ kernel on the Lisa (LisaEm)

**Status:** untested. This is the planned procedure; update it as it gets tried.

The kernel sources and headers go on a second 10 MB ProFile (a "build disk") mounted at `/usr/src`. The system disk stays untouched, and objects, intermediate links and the new kernel don't fill the root filesystem, which has only ~2.7 MB free.

## 0. Recommended: 20 MB system disk (no expansion card)

ProFiles on the dual parallel card currently hang under LisaEm (see the known problem in section 3). Until that's fixed, use a single larger drive on the built-in parallel port instead:

- **Build it on the Mac** (LisaEm must be shut down; the base image is only read):
  ```
  python3 tools/make_profile_image.py uniplus_unix_20mb.image \
      --base-image uniplus_unix_10mb_testing.image \
      --disk-sectors 38912 --start 19456 --blocks 19456 --prlmap e \
      sys=v1.5/sys include=v1.5/include netlib=netlib README=lisa-build.md
  ```
- **Layout:**
  - The first 10 MB is the base system disk, copied unchanged, except partition `e` of the kernel's partition table in `/unix`, set from `{0, 0}` to `{19456, 19456}` (2 bytes).
  - The second 10 MB (sectors 19456–38911) is a new filesystem with the sources.
- **In LisaEm:** point the built-in parallel port ProFile at `uniplus_unix_20mb.image` and boot as usual. The dual parallel card isn't needed.
- **On UniPlus, as root** (the device nodes are needed once):
  ```
  mknod /dev/p0e b 0 4
  mknod /dev/rp0e c 5 4
  fsck /dev/p0e
  mkdir /usr/src
  mount /dev/p0e /usr/src
  ```
- **Then continue with section 4.**
- **Untested:** whether the Lisa boot ROM and LisaEm accept a 20 MB drive on the built-in port. If it doesn't boot, point LisaEm back at the 10 MB image.

## 1. Build disk contents (separate 10 MB disk)

`uniplus_src_10mb.image` is created on the Mac with `tools/make_profile_image.py`:

```
python3 tools/make_profile_image.py uniplus_src_10mb.image \
    sys=v1.5/sys include=v1.5/include netlib=netlib README=lisa-build.md
```

| Path on the disk | Contents |
|---|---|
| `/sys` | V.1.5+ kernel sources and `Makefile` |
| `/include` | Kernel headers not on a stock Lisa disk: `net/*.h` and the Lisa `sys/*.h` additions (see `include/PROVENANCE.md`) |
| `/netlib` | Socket headers, syscall stubs and `tcpconn.c` for testing the network kernel |
| `/README` | This file |
| `/lost+found` | Pre-sized, for `fsck` |

It's one System V filesystem in partition `h` (sector 101, 19,355 blocks), the whole-disk partition of a 10 MB ProFile.

## 2. LisaEm setup

1. **Keep a backup of your system disk image.** Copy it before experimenting with new kernels.
2. **File → Preferences → Slots:** set **Slot 1** to **Dual Parallel**. No ROM file is needed; LisaEm emulates the card's ID ROM (card ID `0xE002`).
3. **Slot 1 high port:** ProFile, pointing at `uniplus_src_10mb.image`. Observed on LisaEm: with the image on slot 1 *high* and *low* set to Nothing, `/dev/p1h` gives "cannot open" and `/dev/p2h` finds the drive. So LisaEm's slot 1 high port is UniPlus unit 2 (`/dev/p2h`), and low is unit 1 (`/dev/p1h`).
4. Apply, then restart LisaEm and boot UniPlus from the built-in ProFile as usual. At boot the kernel should print `Expansion slot 1: two port card`.

## 3. Mount the build disk (as root)

```
mkdir /usr/src          # first time only
fsck /dev/p2h           # check the new filesystem the first time
mount /dev/p2h /usr/src
df /usr/src
```

`/dev/p1h` (block 0x17) is the other port on the same card. "cannot open" on a port means no drive answered there.

**Known problem (Sep 2026):** I/O on `/dev/p2h` hangs under LisaEm, even though the drive is detected. The UniPlus ProFile driver is interrupt-driven: it waits for the drive's BSY signal to raise a CA1 interrupt on the card's VIA, which `ppintr()` in `sys/config.c` services for both slot 1 ports. LisaEm's handling of that interrupt for expansion-slot ProFiles is the prime suspect.

Unmount before stopping LisaEm or changing the image on the Mac:
```
sync; umount /dev/p2h
```

## 4. Build

The Makefile already expects sources in `/usr/src/sys`. It still points `GENnet` at UniSoft's old path and has an empty `INCLUDE`, so set both on the command line:

```
cd /usr/src/sys
make GENnet=/usr/src/sys INCLUDE=-I/usr/src/include config.o
```

Start with that single file. It exercises the reconstructed `sys/config.h`, the compiler and the include path. Then build the whole network kernel:

```
make GENnet=/usr/src/sys INCLUDE=-I/usr/src/include unix.net
```

Other targets: `unix.nonet` (no networking), `unix.su` (single user).

**Do not run `make install`.** It moves `/unix` to `/ounix` and copies the new kernel over `/unix`.

Things likely to need attention, from `include/PROVENANCE.md`:
- **`include/net/misc.h` has `typedef short void;`.** If the Lisa compiler rejects it, remove that line (and note the change in `PROVENANCE.md`).
- **`include/net/ip.h`, `struct ip_timestamp`:** the union member is unnamed and lacks a `;`.
- **Compiler flags:** `-OBPS` and `-v` come from UniSoft's cross-build. If `cc` rejects them, run `make OOPT= V= ...`.
- **`/tmp` space:** the compiler writes temporary files to `/tmp` on the root filesystem; check `df` if compiles fail oddly.

## 5. Trying the new kernel

Only try this on a copy of your system disk image.

1. Copy the kernel to the root filesystem under a different name: `cp unix.net /unix.net`.
2. Boot it on the copy. How to select a kernel other than `/unix` at boot isn't documented here yet; the simplest route is to replace `/unix` on the *copy* of the system image and keep the original image untouched.
3. With the network kernel running, test loopback (no Ethernet hardware needed):
   ```
   cd /usr/src/netlib
   make
   ./tcpconn 127.0.0.1 <port>
   ```
   There's no TCP server yet, so expect "Connection refused" (errno 81) at first. That still proves the syscalls work.
