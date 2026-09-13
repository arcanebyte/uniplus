# Etherbox emulation in LisaEm: plan for a new session

**Goal:** real networking for the UniPlus+ V.1.5+ `unix.net` kernel under LisaEm. Emulate the 3Com "etherbox" that the kernel's `if_eb.c` drives through a Lisa parallel port, and bridge its Ethernet frames to the Mac. Success means `tcpconn` on the Lisa reaches a service on the Mac, and a Mac client reaches `tcpecho` on the Lisa.

This document is written for an agent or person starting fresh. Read the context section first.

> **Progress (13 September 2026):** steps 1–5 are done and the goal is met.
> - **Step 1:** unit 5 is LisaEm's slot 2 upper port (`via[5]`).
> - **Steps 2–5:** the box model, register trace, pcap dump, responder and libslirp backends are in lisaem PR #57.
> - **Kernel:** `conf.c` is 10.0.2.15.
> - **Results:** `tcpconn` from the Lisa reaches the Mac, the Mac reaches `tcpecho` through a port forward, and `netlib/ping` gets replies from 10.0.2.2. Ping needed kernel fixes to raw sockets and ICMP input.
>
> Remaining:
> - step 6: robustness under load;
> - step 7: a Preferences UI (settings are environment variables for now);
> - step 8: a `route` tool and telnet/ftp.
>
> What was found and changed is in `RESTORATION.md` section 9. Some details below were settled differently during implementation:
> - the transmit buffer and the bus buffer pointer are shared;
> - frames are capped at 1500 bytes because of `ebrbuf`;
> - the box only interrupts once the CPU accepts the slot's interrupt level.
>
> The code comments in `etherbox.c` explain each.

## 1. Context and where things stand (13 September 2026)

**Repos:**
- **`~/github/uniplus`** (`arcanebyte/uniplus`, PR #4 on branch `lisa-network-kernel`):
  - `v1.5/sys`: kernel source.
  - `v1.5/include`: reconstructed and Torch headers; see `v1.5/include/PROVENANCE.md`.
  - `netlib/`: userspace socket headers, `sockcall.s` syscall stubs, and the test programs `tcpconn`, `tcpecho`, `looptest`, `udptest`, `netinfo`.
  - `tools/`: `extract_profile_image.py`, `make_profile_image.py`, `put_profile_files.py`.
  - History: `RESTORATION.md`. Build procedure: `lisa-build.md`.
- **`~/github/lisaem`** (`arcanebyte/lisaem`, PR #55 on branch `profile-emulation`): faithful ProFile drive and 6522 VIA emulation. Test status: `ProFileEmulationTesting.md`.

**What works:**
- `unix.net` builds on the (emulated) Lisa and boots on LisaEm from PR #55.
- `netlib/looptest` passes a TCP round trip over 127.0.0.1.
- `netlib/udptest` passes a UDP round trip over loopback, and `netlib/netinfo` shows the host name and configured address.
- The kernel image is on `~/Documents/LisaEm Files/uniplus_unix_20mb.build2.image`, with sources in partition e (`/dev/p0e`, mounted at `/usr/src`).

**Working rules the user has set:**
- No Claude/AI attribution in commits or PRs.
- Commit or push only when asked.
- LisaEm must be shut down before any disk image is read or written from the Mac. Back up an image before changing it.
- LisaEm can't paste into the Lisa. Files go onto the disk with `tools/put_profile_files.py`, then `fsck` on the Lisa.
- No drive-by refactors in LisaEm; mention adjacent issues instead of fixing them.

**LisaEm build notes:**
- **Nested worktree:** `build.sh` adds every `include` directory under the tree. A git worktree under `.claude/worktrees/` breaks the build; build from an rsync copy that excludes `.claude` (see `ProFileEmulationTesting.md`).
- **Header changes:** the build doesn't track header dependencies. After changing `src/include/vars.h`, delete `obj/*.o` in the build copy.
- **Test apps:** `bin/LisaEm-profile.app` is the current test build. It includes uncommitted trace code (`profile_trace()` in `profile.c`, calls in `via6522.c`) that writes `~/lisaem-profile-trace.log`. Keep it out of commits.
- **Bisect builds:** `bin/LisaEm-eb9c325.app`, `-2.0.0`, `-3620e87`, `-5195244` are kept for bisecting.

## 2. What the kernel expects (from `v1.5/sys/if_eb.c`)

### 2.1 Configuration
- **`conf.c`:** `ubdinit[] = { &ebdriver, 0, (caddr_t)5, 0x59002908 }`.
  - The address field **5** is the parallel "unit".
  - The flags field is the IP address, **89.0.41.8** (net 89 was UC Berkeley's Computer Center Ethernet).
- **Unit 5** is `pro_da[5]` = `STDIO+0x6800` in `config.c`: port 1 of a dual parallel card in expansion **slot 2**.
  - `appleinit()` requires `PPOK(5)` and `slot[PPSLOT(5)] == PR0` (slot 2 must hold a parallel card).
  - It installs `ebintr` as that port's interrupt handler with `setppint`.
  - Other valid units: 1, 2 (slot 1), 4, 5 (slot 2), 7, 8 (slot 3).
- **Which LisaEm port:** verify how LisaEm's slot/port numbering maps to units. Earlier testing found LisaEm slot 1 "high" answered as `/dev/p2h` and "low" as `/dev/p1h`, which doesn't obviously match `config.c`'s order.
- **Address change:** it is fixed at boot. `SIOCCIADDR` only rewrites `ubdinit` flags after `ebattach` has already used them, so change the address in `conf.c` and rebuild.

### 2.2 Wire protocol over the 6522
The etherbox is driven entirely through one VIA. Command/state codes go out on port B, and register numbers and data move on port A, one byte per strobe.

- **Setup (`ebreset`):**
  - DDRA=0 and ORB=`$18`;
  - DDRB=`$BC` (outputs: bits 2, 3, 4, 5, 7);
  - ORB=`$A8` and PCR=`$0B`: CA2 in pulse mode, so every register-1 access strobes; CA1 on the rising edge;
  - IER=`$82` (CA1 enabled).
- **Port B codes:**

  | Value | Name | Meaning |
  |---|---|---|
  | `$A8` | RC | idle / read command |
  | `$A0` | WC | write command: the next port-A byte is a register number |
  | `$B0` | WD | write data: port-A bytes go to the selected register |
  | `$B8` | RD | read data: port-A reads come from the selected register |

  Bit 3 (`$08`) looks like the read/write direction and bit 4 (`$10`) like command/data. The kernel only ever uses these four values.
- **Write a register (`ebwr_reg`):** ORB=WC, DDRA=`$FF`, ORA=regno (strobe), ORB=WD, ORA=byte (strobe).
- **Read a register (`ebrd_reg`):** ORB=WC, DDRA=`$FF`, ORA=regno (strobe), ORB=RD, DDRA=0, read IRA (strobe).
- **Bulk data:** `ebwr_setup`/`ebwr_data` and `ebrd_setup`/`ebrd_data` select a buffer register once, then move many bytes, one strobe each. The box's buffer pointer advances each time.
- **Back to idle (`ebportreset`):** ORB=RC, DDRA=0.
- **All data moves through register 1** (`d_ira`, offset 9), so the strobes follow LisaEm's new pulse-mode handling (`VIA_CA2_STROBES`).

### 2.3 Controller register model
It matches a 3Com EDLC-based interface; compare `v1.5/sys/if_ec.c` (4.xBSD 3Com driver).

| Reg | Name | Behaviour the driver relies on |
|---|---|---|
| 0–5 | ACTADDR0–5 | Station address RAM (written from the PROM at init) |
| 6 | RCVCMD | Receive filter; driver writes `EB_RCVNORM` = station + broadcast plus error detection |
| 7 | XCSR | W: transmit interrupt enables (`EIEOF`, `EI16COLL`). R: status (`XREADY` `$08`, `COLL16` `$04`, `COLL` `$02`) |
| 8, 9 | XBP_HI, XBP_LO | Transmit buffer pointer (11 bits) |
| 10 | BBPCLEAR (W) / PROM (R) | W: reset the bus-buffer pointer to 0. R: station address PROM, read sequentially (6 bytes) |
| 11 | AUXCSR | See below |
| 12 | COLLCNTR | Collision counter; written 0 to clear |
| 13 | XMTBUF | Transmit buffer (2048 bytes) at the bus-buffer pointer |
| 14, 15 | RCVBUFA, RCVBUFB | Two receive buffers (2048 bytes each) read at the bus-buffer pointer |

- **AUXCSR bits:**
  - W: `EDLCRES` `$80` reset; `SYSEI` `$40` enable interrupts; `RBBSW` `$20` / `RBASW` `$10` hand receive buffer B/A to the controller; `XBUFSW` `$08` hand the transmit buffer to the controller (starts transmit); `POWINTR` `$01` clear power-on interrupt.
  - R: `XCVRUP` `$80`; the `$70` mask (`EB_IMASK`); `BBASW` `$04`, set when buffer B filled before A.
  - While a buffer is owned by the controller its switch bit reads 1. When a frame lands in it, or a transmit completes, the controller clears that bit and interrupts if `SYSEI`.
- **Transmit (`ebput`/`ebstart`):**
  1. Compute `off = 2048 - framelen`, clamped to at most 1536 (60-byte minimum) and made even.
  2. Write XBP=off, select XMTBUF, and write the frame so it ends at offset 2047.
  3. Write XBP=off again, set XCSR interrupt enables, then write AUXCSR with `XBUFSW|SYSEI` (preserving the `$70` bits).
  4. **Emulation:** when `XBUFSW` is set, send bytes off..2047 as one Ethernet frame (no CRC). Clear `XBUFSW`, set XCSR `XREADY`, raise the interrupt.
- **Receive (`ebintr`/`ebread`):**
  1. AUXCSR tells which buffers hold frames: `RBASW`/`RBBSW` cleared, `BBASW` for the order.
  2. For each buffer: BBPCLEAR, then read byte1 and byte2 from the buffer register.
     - `byte1 & $F8` = error bits (`STALE` `$80`, `SHORT` `$40`, `DRIBBLE` `$20`, `CRC` `$10`, `OVFL` `$08`).
     - `((byte1 & 7) << 8 | byte2)` = end offset of the frame.
  3. The frame data starts at buffer offset 2; `len = endoff - 2`. The driver reads `len` bytes with another setup on the same buffer, continuing from the pointer.
  4. Then it rewrites AUXCSR with `RBASW|RBBSW|SYSEI` to re-arm the buffers.
  5. **Emulation:** put an incoming frame into a free, armed buffer as `[status, endoff lo]` plus frame, set `BBASW` as needed, clear that buffer's switch bit, and raise CA1.
- **Interrupt:** the box pulls the parallel port's CA1 line. `ppintr()` in `config.c` scans slot VIAs for `IFR & CA1` and calls `ebintr`. Emulate it as a /BSY-style edge on that VIA's CA1 (use the edge/latch helpers from PR #55).
- **Periodic reset:** `ebpoll()` (`polleb = 1`) fully resets and re-initializes the box every 10 seconds (`timeout(ebpoll, pport, v_hz*10)`). The emulation must survive this without losing configuration. A dropped frame here and there is acceptable. Optionally patch `polleb` to 0 in a later kernel build.
- **The 3Com can't hear itself:** `eboutput` copies broadcasts to the loopback interface. The emulation must not echo transmitted frames back.
- **Trailer encapsulation** (ether types `$1000`–`$100F`) is decoded on receive. Senders on the Mac side won't use it.

### 2.4 Ethernet and IP layer
- **ARP:** `if_ether.c` has `arpinput`, `arpresolve` and `arpwhohas` (sent at init).
- **Addressing:** class-based, no netmasks. The interface's network is derived from the address class. A route is needed for anything off that network; `route.c` exists, and `SIOCADDRT` is `('s'<<8)|10`, with no userspace `route` tool yet.

## 3. Design

### 3.1 LisaEm device
- **A new attachable device** alongside ProFile and ImageWriter: "EtherBox".
  - Selectable per parallel port in Preferences, including dual parallel card ports.
  - Its own struct (`EtherBoxType`), stored in `viatype` like `ProFile`/`ADMP`.
- **Hook points in `src/lisa/io_board/via6522.c`:** `viaX_ora`, `viaX_ira`, `viaX_orb`, `viaX_irb` (slot VIAs, `lisa_rb_ext_2par_via`/`lisa_wb_ext_2par_via`), and the `via2_*` equivalents if the built-in port should work too.
  - Strobed register-1 reads and writes arrive as `PROLOOP_EV_ORA`/`PROLOOP_EV_IRA`-style events.
  - Port B writes carry the command code.
- **Box state:** selected register; bus-buffer pointer; XBP; the 2048-byte transmit buffer and two receive buffers; AUXCSR, XCSR, RCVCMD and station address; PROM bytes.
  - Suggested MAC address: `02:60:8C:xx:xx:xx`, 3Com's OUI with the locally administered bit.
- **Interrupts:** CA1 edges through the same `VIAProfileLoop`-style edge/latch path as the ProFile, so IFR, IER and immediate IRQ behave correctly.
- **Timer:** incoming frames arrive asynchronously. Poll the backend from the existing timer loop (`get_next_timer_event`/`check_current_timer_irq` in `src/lisa/cpu_board/irq.c`), with a small periodic timer while the box is enabled. Deliver a frame only when a receive buffer is armed.

### 3.2 Host backend
Options, recommended first:
1. **libslirp** (user-mode NAT, as QEMU uses). No root, no entitlements, works on macOS and Linux.
   - It gives the guest a private network (default `10.0.2.0/24`): gateway/host `10.0.2.2`, DNS `10.0.2.3`, guest typically `10.0.2.15`.
   - It forwards guest TCP/UDP to the outside and supports host→guest port forwards, e.g. Mac `localhost:2323` → Lisa port 23.
   - It handles ARP for the gateway. Available through Homebrew (`libslirp`) or vendored.
2. **macOS vmnet.framework:** bridged or shared mode; needs root or a signed entitlement.
3. **TAP/utun device or pcap bridge:** needs root and extra drivers.

Start with libslirp behind a small backend interface (`open`, `send_frame`, `poll`/callback for received frames, `close`), so another backend can be added later.

### 3.3 Kernel configuration for slirp
- **Address:** in `conf.c` set ubdinit flags to `10.0.2.15` = `0x0a00020f`, and the unit to the parallel port the etherbox is attached to. Rebuild `unix.net` on the Lisa (`lisa-build.md`) and install as `/unix`. Keep a fallback kernel.
- **Network class:** class-based addressing makes `10.0.2.15` class A, net 10, so `10.0.2.2` is on the same network: no route needed to reach the Mac through slirp.
- **Internet:** reaching the internet needs a default route to `10.0.2.2`. That means a small `route` program using `SIOCADDRT` with the 4.1a `struct rtentry` from `v1.5/include/net/route.h`, or a kernel patch.

## 4. Step-by-step plan

1. **Verify the port mapping.** Read LisaEm's dual parallel card mapping (`src/lisa/cpu_board/memory.c`: `lisa_rb_ext_2par_via` dispatch, `via[3..8]` and their base addresses) against `pro_da[]` in `v1.5/sys/config.c`, and record which LisaEm slot/port is unit 5. If needed, change `conf.c` to a unit that's convenient.
2. **Box model without a network.** Implement the register model and protocol in LisaEm, using a local loopback backend that just drops or records transmitted frames.
   - Boot `unix.net` with the etherbox attached. Expect `Ethernet address = 02608C…` on the console, and no "Can't find port for etherbox".
   - Log every register access to a trace file, like the ProFile trace, and compare the sequence with `ebinit`/`ebpoll`.
3. **Transmit path.** Confirm the ARP request from `arpwhohas()` at init leaves as a well-formed frame: broadcast destination, ether type `$0806`, the kernel's IP. Dump frames to a pcap file so Wireshark can check them.
4. **Receive path.** Inject a crafted ARP reply, then an ICMP echo request to the kernel's address. The 4.1a kernel answers ICMP echo in `ip_icmp.c`, so an echo reply should be transmitted.
5. **libslirp backend.** Wire frames to libslirp, and set `conf.c` to `10.0.2.15`. Test:
   - Lisa → Mac: run `nc -l 7000` on the Mac, then `tcpconn 10.0.2.2 7000 hello` on the Lisa.
   - Mac → Lisa: port-forward Mac `localhost:5000` to Lisa 5000, run `tcpecho 5000 &` on the Lisa, then `nc localhost 5000` on the Mac.
   - UDP: a variant of `udptest` with a remote address.
6. **Robustness.**
   - Survive the 10-second `ebpoll` reset.
   - Handle back-pressure: both receive buffers full → drop.
   - Handle frames under 60 or over 1514 bytes.
   - Keep interrupts correct with the ProFile on another port of the same card.
7. **User interface and persistence.** Preferences choice per port; backend options (port forwards) in the config file.
8. **Userspace.** A `route` tool (`SIOCADDRT`) for a default route. Try the Torch 4.1a `telnet`/`ftp` binaries (`torch/`), or port 2.9BSD `telnet`/`ftp` from `bsd/`, then telnet from the Lisa to a host.
9. **Documentation and PRs.**
   - lisaem: a new branch off `profile-emulation` (it needs PR #55's VIA changes) and a testing doc.
   - uniplus: the `conf.c` change, `route` tool and results in `RESTORATION.md`/`netlib/README.md`.

## 5. Open questions and risks
- **Exact box behaviour** beyond what the driver exercises is unknown: for example what `XCSR` reads while idle, how `COLLCNTR` behaves, and `RCVCMD` filtering details. Implement only what `if_eb.c` needs. Compare `if_ec.c` and 3Com 3C400/3C300 documentation (search Bitsavers) for the EDLC register set.
- **The kernel ignores most error conditions**, so the emulation can report "no errors, no collisions" always.
- **Port B line meanings** (which bits are strobe, direction, command/data) are inferred from four constant values. The box may also drive port B inputs (for example /BSY for flow control); the driver never reads IRB for the etherbox, so it probably doesn't matter.
- **Interrupt storms:** `ebintr` re-reads AUXCSR in a loop until it stops changing. A frame arriving mid-loop must not wedge it.
- **Performance:** each byte is several VIA accesses. At emulated Lisa speed a full frame is a few thousand register accesses, acceptable at 12 MHz but worth measuring.
- **Licensing:** libslirp is BSD-3-Clause, compatible with LisaEm's GPL.

## 6. References
- **Kernel source:**
  - `v1.5/sys/if_eb.c`: the driver.
  - `v1.5/sys/config.c`: `pro_da`, `setppint`, `ppintr`.
  - `v1.5/sys/conf.c`: `ubdinit`.
  - `v1.5/sys/if_ether.c`: ARP.
  - `v1.5/sys/if_ec.c`: the 3Com EDLC register semantics.
  - `v1.5/sys/socket.c` `soioctl`: `SIOCGIADDR`/`SIOCCIADDR`.
  - `v1.5/sys/route.c` and `v1.5/include/net/route.h`: routing.
- **LisaEm:**
  - `src/lisa/io_board/via6522.c`: VIA, strobes, `VIAProfileLoop` edge latching.
  - `src/lisa/cpu_board/irq.c`: timers and IRQ delivery.
  - `src/storage/profile.c`: a model device on a VIA.
  - `src/printer/imagewriter/`: the other parallel device.
- **UniPlus repo history:** `RESTORATION.md`, `profile-emulation-notes.md`, `profile-emulation-plan.md`.
- **libslirp:** https://gitlab.freedesktop.org/slirp/libslirp
