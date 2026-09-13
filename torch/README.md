# torch

Hard disk images from the **Torch Triple X**, a 1980s UK 68000 workstation that ran UniSoft UniPlus+ System V. It's a sibling of UniPlus+ on the Apple Lisa, and the source of the Lisa kernel's missing network headers.

- **Source:** https://bitsavers.trailing-edge.com/bits/Torch/, downloaded 2026-09-12.
- **Original images:** `*.img.gz`, unmodified.
- **Extracted filesystems:** made with `../tools/extract_profile_image.py`, one folder per filesystem, named by the sector where it starts. For example:
  ```
  python3 tools/extract_profile_image.py --list  torch/Torch_Triple-X_SysV_Portable_Disk.img
  python3 tools/extract_profile_image.py --base 20504 torch/Torch_Triple-X_SysV_Portable_Disk.img out
  ```
  Both images are plain 512-byte sectors holding big-endian System V filesystems with 1024-byte blocks. Hard links are separate copies, device nodes are `NAME.special` text files, and owners/permissions aren't preserved (see the tool's header comment).

## `portable-disk/` — `Torch_Triple-X_SysV_Portable_Disk.img.gz`

UniPlus+ from 1983–86. The system files carry UniSoft copyright lines for V.1.0 and V.1.1, the same generation as the Lisa V.1.0/V.1.5+ kernels.

| Folder | Size | What it appears to be |
|---|---|---|
| `sector-00004` | 0.2 MB | Boot area: boot icons and masks |
| `sector-00408` | 2.6 MB | Utility/scratch filesystem with its own `unix` (Mar 1987) |
| `sector-09504` | 5.6 MB | **Root filesystem with networking**: `unix` (Mar 1986), `bin/ftp`, `bin/telnet`, `etc/telnetd`, `etc/ftpd`, `etc/rlogind` |
| `sector-20504` | 13.3 MB | **`/usr`**: `include/net/` (the 4.1a-era network headers), `lib/libnet.a`, `bin/rlogin`, `include/sys/` |
| `sector-46504` | 4.6 MB | Second root filesystem, `unix` (Dec 1986) |
| `sector-55504` | 14.3 MB | Second `/usr` (`distrib1.3`) |

Why `sector-20504` matters for the Lisa:
- **`include/net/`** has all 34 network headers in the same flat `net/` layout the Lisa kernel expects. That includes the ones lost from the Lisa sources: `misc.h` (UniSoft's own), `mbuf.h` (with the PDP-11-port `MSGET`/`MAPSAVE` macros), `socket.h` 4.16 82/06/08, `socketvar.h`, `protosw.h`, `ubavar.h`, `if_ether.h`, `netisr.h`, `netdb.h`. The versions match the Lisa kernel's own mix of revisions: an 82/06 base, 83/02 updates (`ip_var.h` 83/02/23), and UniSoft `if.h`/`if_ether.h` 2.1 84/05/04. The files are dated Oct 1984.
- **`include/sys/`** has `scat.h` and `context.h`, also missing from the Lisa sources. Its `config.h` is for Torch (VMEbus) hardware, not the Lisa.
- **`lib/libnet.a`** is UniSoft's network library (`netsys.o`, `rhost.o`, `raddr.o`). Its stubs use syscalls 70–79 with the same register convention as `../netlib/sockcall.s`.

## `mfm-disk/` — `Torch_MFM_D5146_615c8hd.img.gz`

A later UniPlus V.2.1.x (1987). By then the network headers had moved to the 4.2BSD layout (`include/sys/socket.h`, `include/netinet/`), so this disk is mainly useful for comparison.

| Folder | Size | What it appears to be |
|---|---|---|
| `sector-00004` | 0.2 MB | Boot area |
| `sector-00408` | 10.2 MB | Root filesystem |
| `sector-30408` | 26.6 MB | `/usr` (`distrib2.0`) |
