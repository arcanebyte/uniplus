# Provenance of the V.1.5+ kernel headers

This directory holds the headers the V.1.5+ kernel build (`../sys/Makefile`, `unix.net`) needs that are **not** on a stock UniPlus+ Lisa disk. The generic System V kernel headers (`sys/param.h`, `sys/user.h`, `sys/proc.h`, `sys/file.h`, …) come from the Lisa's own `/usr/include/sys`, preserved in `../../dump/usr/include/sys`, and aren't copied here.

## Provenance levels

| Level | Meaning | Marking in the file |
|---|---|---|
| **ORIGINAL** | Byte-for-byte copy of a UniSoft file | None; the file is kept identical to its source |
| **MODIFIED** | Original plus changes | Banner at the top describing the change |
| **RECONSTRUCTED** | Written in 2026 from evidence | `RECONSTRUCTED` banner; each definition says how its value was obtained |

Confidence tags inside reconstructed files:
- **CONFIRMED:** value taken from a surviving binary, the manual, or unambiguous kernel code.
- **INFERRED:** follows from kernel usage, but not proven.
- **GUESS:** no evidence survives; chosen not to collide with known values.

## Files

### ORIGINAL — Lisa V.1.5+ (Bitsavers `bits/Unisoft/V.1.5+/sys/h/sys`)

`sys/`: `bmfont.h`, `cops.h`, `cv.h`, `d_profile.h`, `kb.h`, `keyboard.h`, `l2.h`, `local.h`, `mmu.h`, `ms.h`, `pport.h`, `priam.h`, `profile.h`, `reboot.h`, `scc.h`, `sony.h`, `sunol.h`, `swapsz.h`

`sys/mmu.h` differs from the copy in the Lisa's `/usr/include/sys`. Use this one: it comes with the kernel sources.

### ORIGINAL — Torch Triple X UniPlus+ (`../../torch/portable-disk/sector-20504/include`)

The Lisa's own copies were lost. These come from the same UniSoft UniPlus+ release family, dated Oct 1984, with the same flat `net/` layout, and their revisions match the Lisa kernel sources (see "Version match" below).

| File(s) | Notes |
|---|---|
| `net/*.h` (34 files) | `af.h`, `if.h` (UniSoft 2.1 84/05/04), `if_ec.h`, `if_ether.h` (UniSoft 2.1 84/05/04), `if_imphost.h`, `if_uba.h`, `imp.h`, `in.h` 4.13, `in_pcb.h` 4.5 82/10/20, `in_systm.h` 4.13, `ip.h` 1.11 82/12/28, `ip_icmp.h`, `ip_var.h` 4.9 83/02/23, `mbuf.h` 4.13 82/06/14, `misc.h` (UniSoft v.0.1.1), `netdb.h`, `netisr.h`, `protosw.h` 4.11, `pty.h`, `pup.h`, `raw_cb.h`, `route.h` 4.8, `socket.h` 4.16 82/06/08, `socketvar.h` 4.16, `tcp.h`, `tcp_debug.h`, `tcp_fsm.h`, `tcp_seq.h`, `tcp_timer.h`, `tcp_var.h`, `tcpip.h`, `ubavar.h`, `udp.h`, `udp_var.h` |
| `sys/scat.h` | Identical on both Torch `/usr` filesystems; matches every use in `bio.c`, `malloc.c`, `sys1.c` |
| `sys/context.h` | Every field the kernel uses (`cx_forw`, `cx_back`, `cx_proc`, `cx_daddr`, `cx_dsize`, `cx_num`, `cx_phys[]`, `cx_shm[]`) is present; array sizes come from the Lisa `mmu.h`/`param.h` |
| `sys/uioctl.h` | Identical to the Lisa's `/usr/include/sys/uioctl.h` plus `UIOCSIZE (UIOC\|11)`, which `priam.c` uses and `priam(5L)` documents |

`net/misc.h` is where UniSoft put the network additions to the generic headers:
- `fd_set`, the `SSEL`/`STIMO` process flags, `FSOCKET`/`FISUSER`/`f_socket`;
- the socket ioctls `FIONBIO`, `FIOASYNC`, `TIOCPKT`, `SIOCDONE`…`SIOCCHGRT`, and UniSoft's `SIOCCIADDR` (13) and `SIOCGIADDR` (14);
- 7-character renames for the linker (e.g. `tcp_output`→`tcp_oput`).

So the Lisa's `file.h`, `proc.h`, `types.h` and `ioctl.h` need no changes. The new flag bits don't collide with the Lisa's existing ones.

### RECONSTRUCTED

| File | Contents | Evidence |
|---|---|---|
| `sys/config.h` | `PR0` 0, `SN1` 1, `CV2` 2, `PM3` 3, `NSC` 2, `NTE` 4, `CONSOLE` 0 | Block majors: `bdevsw[]` order in `conf.c` (CONFIRMED). `NSC`: two `sc_ttptr[]` entries (CONFIRMED). `NTE`: `te_cnt` = 4 in the symbol table and data of the working 1.4 `/unix` (CONFIRMED); without it `conf.c` silently omits the Tecmar tables and the link leaves `te_*` undefined. `CONSOLE`: console is major/minor 0 (INFERRED). The Torch `config.h` is for different hardware and wasn't used. |
| `sys/al_ioctl.h` | `AL_S*` 1–9, `AL_REVVIDEO` 10, `AL_G*` 17–25, `AL_EJECT` 34, `AL_GBMADDR` 26, `AL_GMOUSE` 32, `AL_SMOUSE` 33 | Names and meanings: `console(5L)`, `mouse(5L)`, `eject(1L)` in the UniPlus+ Lisa-specific manual (Bitsavers `pdf/unisoft/UniPlus-Lisa-specific.pdf`). Values 1–10, 17–25 decoded from the `ioctl()` calls in `/usr/bin/setparams`, whose call order matches its menu, the `co.c` switch order and the manual (CONFIRMED). 34 from `/usr/bin/eject` and `/bin/tar` (CONFIRMED). No surviving program or document gives `AL_GBMADDR`, `AL_GMOUSE`, `AL_SMOUSE` (GUESS). |
| `sys/speaker.h` | `struct speaker { ushort sk_wavlen, sk_duration, sk_volume; }`, `MINWLEN` 8, `MAXWLEN` 8191 | `speaker(5L)` gives the struct and the 8/8191 limits (CONFIRMED); `sk.c` uses `MAXWLEN` as a mask, and 8191 = 0x1FFF works as one. |

### Not needed by this kernel

These are included only inside `#ifdef`s that aren't set for the Lisa, so they were left out:

| Header | Guard |
|---|---|
| `sys/istk.h`, `sys/macro.h` | `#ifdef u3b` |
| `sys/page.h` | `#ifdef vax` |
| `sys/fptrap.h` | `#ifdef mc68881` |
| `nd.h` | `#if NND > 0` |
| `domain.h` | Include commented out in `raw_cb.c` |

## Version match

The Lisa network sources are Berkeley's 82/06/20 network release (the base also used by 2.9BSD), with some files updated to 82/10–83/02 revisions and `if_ether.c` from 4.2BSD. The Torch `net/` headers show the same mix: an 82/06 base, 83/02 `ip_var.h` (matching `ip_input.c` 83/02/23), and UniSoft's 84/05 `if.h`/`if_ether.h`. Berkeley's own revisions can be compared through https://github.com/robohack/ucb-csrg-bsd; for example, its `socket.h` 4.16 82/06/08 matches the Torch copy.

## Build notes and open issues

**Reconstructed source file (in `../sys`):**
- **`../sys/cxstub.c`:** empty `cxrelse()` and `cxtxfree()`. `slp.c`, `text.c` and `sys1.c` call them, but their real definitions in `context.c` are for a many-context MMU (`cxmap`, `USERCX`, `NUMUCONTX`, `context[]` aren't in any Lisa header) and aren't in the Lisa build. The first Lisa link reported both as undefined. On the Lisa no context is ever allocated, so `p_context` and `x_cxaddr` stay 0, and for those values the `context.c` versions return without doing anything. The stubs are equivalent. Added to `KERNEL` and `SKERNEL` in the Makefile.

**Build changes already made:**
- **`-Dm68000` added to `DEFS` in `../sys/Makefile`.** `net/ip.h` and `net/ip_var.h` test `m68000`, but the Lisa `cpp` predefines only `unix` and `mc68000`. Without it, `struct ip` has no `ip_v`/`ip_hl` fields.
- **Include path:** the Makefile's `INCLUDE=` is empty. On the Lisa, either copy this directory into `/usr/include`, or set `INCLUDE=-I<this directory>` so these files are found before `/usr/include`. `uioctl.h` and `mmu.h` must win over the stock copies.

**Host syntax check.** `clang -fsyntax-only -std=gnu89` on all 97 build sources, with the Makefile defines, against this directory plus `../../dump/usr/include`:
- **Result:** no undefined identifiers, unknown types or missing struct members, apart from the items below.
- **Not errors:** the remaining messages are clang being stricter than a 1984 compiler (K&R code, kernel `malloc`/`free` clashing with built-ins) and the `-D` values `name.c` gets from the Makefile.
- **Limits:** this only proves every name resolves. It doesn't check struct layouts or values on the 68000, and it can't see link-time problems. A macro that's never defined silently reads as 0 in `#if` (how the missing `NTE` slipped through), so also run the preprocessor with `-Wundef`. Only the Lisa `ld` reports undefined symbols.

**Results from building on the Lisa (Sep 2026):**
- **Compile:** all sources compile with the Lisa `cc` (`-OBPS -v`), including `net/misc.h`'s `typedef short void;` and `net/ip.h`'s unnamed union in `struct ip_timestamp`; no changes were needed.
- **Link:** the first link left `_cxrelse`, `_cxtxfree` and the `_te_*` Tecmar tables undefined. UniSoft's `ld` still writes the output file with these resolved to 0, so check `build.log` for `Undefined` after every link. Fixed by `NTE` in `sys/config.h` and `../sys/cxstub.c`.
- **Boot:** that first `unix.net` panicked mounting root (`HARD DISK ERROR dev 0: EXCESSIVE DISK DELAY`, `panic: iinit`); not yet re-tested with the link fixed.

**Other notes:**
- **`../sys/sys1.c`:** `#ifdef NONSCATLOAD` at line 700 is never closed. This matches both source copies and the 1986 printed listing, so UniSoft's `cpp` evidently accepted it.
- **`AL_GBMADDR`, `AL_GMOUSE`, `AL_SMOUSE`** are guesses. They only matter for binary compatibility with programs that use them; none survive on the known disks.
