# ProFile emulation notes — making LisaEm faithful enough for UniPlus

These are research notes (September 2026) for replacing LisaEm's UniPlus-specific ProFile hacks with accurate emulation. See `RESTORATION.md` section 7 for why the hacks exist and what they patch.

Paths:
- **LisaEm:** relative to `lisaem/src/`.
- **UniPlus:** relative to this repo.
- **Line numbers:** as read in September 2026.

## References

| Source | What it offers |
|---|---|
| **ESProFile** (ESP32 ProFile emulator), https://github.com/alexthecat123/ESProFile, `sw/ESProFile/ESProFile_Emulator.ino` (v1.5, commit 7ee7ee4) | A drive-side protocol implementation proven on real Lisas and LisaFPGA, including the 2-port parallel card. `ESProFile_Diagnostic.ino` is the host-side tester. |
| **Cameo/Aphid** (Tom Stepleton), https://github.com/stepleton/cameo, `aphid/firmware/aphd_pru1_control/aphd_pru1_control.cc` 475–620 | Second independent drive-side implementation |
| **LisaFPGA**, https://github.com/alexthecat123/LisaFPGA (v1.6, commit 555dcf7) | Datasheet-accurate 6522 (`LisaFPGA.srcs/sources_1/imports/lisaStuff/via6522.v`) and the parallel-port wiring (`lisaStuff/IO_board.sv` 883–1047). Its ProFile is a real ESProFile, not HDL; the dual parallel card isn't implemented. |
| **Apple "ProFile HD Communications Protocol"** (rev 11-14-83), https://archive.org/details/AppleLisa_ProFileHDCommProtocol | Authoritative protocol; the "Profile Communication Protocol" chapter |

## The protocol

Lisa side; /CMD and /BSY are active low.

1. **Idle:** /BSY high.
2. **Host starts a command:** pulls /CMD low.
3. **Drive responds:** puts **$01** on the bus and pulls /BSY low. Aphid pulls /BSY low immediately.
4. **Host acknowledges:** reads the byte, writes **$55**, sets R/W, and raises /CMD. The drive samples the bus **when /CMD rises**; anything but $55 sets status "no 55" and returns to idle.
5. **Drive accepts the command block:** raises /BSY and takes 6 command bytes (`cmd, blkH, blkM, blkL, retry, spare-threshold`), one per /PSTRB pulse. /BSY stays high throughout.
6. **Host signals the end of the block:** pulls /CMD low again.
7. **Drive answers the command:** **$02** read, **$03** write, **$04** write/verify, with /BSY low. Host sends $55, then raises /CMD.
8. **Busy period, on the drive's own time:**
   - **Read:** the drive reads the block, puts status byte 0 on the bus and raises /BSY. The host reads 4 status + 20 tag + 512 data bytes by strobing.
   - **Write:** the host sends 20 + 512 bytes; the drive answers **$06**; host sends $55. The drive writes, then raises /BSY with status.
9. **Special blocks:** $FFFFFF is the spare table, $FFFFFE the RAM buffer.
10. **No drive-side timeouts:** the drive "waits (forever, if necessary)" for CMD. Heads park after 1.5 s idle.

**Timing that matters in practice:**
- **The 2-port parallel card needs /BSY to go low very quickly after /CMD goes low.** Alex Anderson-McLeod, LisaList2, Feb 2025: "the parallel card just expects [BSY] to be asserted much more quickly than the built-in port". UniPlus behaves the same way: it polls with a short timeout.
- **ESProFile has no deliberate delays;** its only timeouts are loop counts.

## What UniPlus's driver does

`v1.5/sys/pro.c`:
- **`proinit`** (222–234):
  - ACR=0; PCR=**$6B**: CA1 interrupt on /BSY rising, CA2 pulse output on every register-1 access, CB1 negative edge, CB2 independent positive edge.
  - DDRB bits 2–4 as outputs; T2=0; IER=**$82** (CA1 only).
- **`prochk`** (556–601):
  1. `ASSERT((d_irb&BSY)==BSY)`: /BSY must already be high.
  2. DRW to input; /CMD low.
  3. Poll up to `RSPTIME` (0xC000 iterations, `sys/d_profile.h`) for /BSY low.
  4. Read the response via register 1; DRW to output; write $55 via register 1; IER=$82; /CMD high.
  5. Return and **wait for the CA1 interrupt** from /BSY rising.
- **`prointr`** (370–537):
  - Acknowledges with `d_ifr = d_ifr`.
  - Moves the 6 command bytes, or 4+20+512 read bytes, or 20+512 write bytes, via register 1 (`a5@(9)` in hand-written assembly), so CA2 strobes on every byte.
  - Calls `prochk` for the next phase.
- **`ppintr`** (`v1.5/sys/config.c` 274–314): built-in port 0 calls `prointr` directly; slot cards scan each VIA's `d_ifr & FCA1`.

## 6522 behaviour UniPlus relies on

Per the datasheet and LisaFPGA's `via6522.v`:
- **CA1 edge:** a CA1 edge of the polarity selected by PCR bit 0 sets IFR bit 1 (`via6522.v` 115).
- **Flags vs mask:** flags latch **whatever IER holds**; IRQ = `|(IFR & IER)`, evaluated immediately (228, 150).
- **Clearing CA1/CA2:** **only a read or write of register 1** clears them (246–250, 353–356), or writing 1 bits to IFR (289–290). Register 15 (no handshake) clears nothing and doesn't pulse CA2.
- **Register writes:** writing IER only changes the mask (292–297). Writing PCR raises no flags.
- **Xenix one-shot fix:** Alex fixed Timer 2 re-firing in one-shot mode because Xenix doesn't mask T2 when checking BSY (462–468).
- **LisaFPGA's wiring** (`IO_board.sv`):
  - CA1 = /BSY, re-synchronised to the E clock.
  - CA2 = /PSTRB. PB1 = /BSY, PB2 = /ProFile_EN, PB3 = R/W, PB4 = /CMD, PB5 = parity out.
  - CB2 = latched parity error.
  - /CMD, R/W and /PSTRB are gated by /ProFile_EN.

## LisaEm deficiencies

These likely force the UniPlus hacks. All are read from source; anything marked *(inference)* hasn't been proved.

**ProFile state machine** (`storage/profile.c`):
- **A. /BSY flickers during the command block.** `GET_CMDBLK_STATE` (~1125–1136) sets `BSYLine=1` (busy) on each ORA write and releases it on other events. Each release with PCR bit 0=1 sets CA1 (`lisa/io_board/via6522.c` 296–298), giving spurious interrupts while IER=$82. Real drives keep /BSY high.
- **B. The UniPlus fake itself asserts BSY while forcing CA1** (~1116–1124). The kernel patch skipping `prochk`'s BSY assertion at 0x20f9c (`storage/hle.c` 864–869) is then needed *(inference, strong)*. On a real drive, CA1 *is* BSY rising, so BSY is always high when it fires.
- **C. No time base of its own.**
  - `P->clock_e` is never scheduled as a timer event (`lisa/cpu_board/irq.c` 178, 237, 603–604), so delays finish only when some other timer fires or the host touches the port.
  - `PARSE_CMD_STATE` doesn't advance on null events (~1315–1320).
- **D. Drive-side timeouts the real drive doesn't have.** `CHECK_PROFILE_LOOP_TIMEOUT` (~806–824) returns to idle after 0.1–0.5 s of emulated time (`ONE_SECOND` = 5,000,000 clocks, `include/vars.h` 1258). Idle reacts only to a /CMD *edge* (`last_cmd`, ~972–997), so if /CMD is already low, the drive may miss `RSPTIME`. That matches the "increase timeout" patch at 0x210b0 *(inference)*.
- **E. $55 accepted from any ORA write** (`WAIT_2nd_0x55` ~1277–1293, `WAIT_3rd_0x55` ~1450–1465) instead of being sampled when /CMD rises.
- **F. /BSY left low at idle:** after a transfer (~1601–1607) and in `FINAL_FLIP_TO_IDLE_STATE`.

**VIA** (`lisa/io_board/via6522.c`):
- **G1. Reading IFR invents CA1 from BSY level:** `if (BSYLine) IFR |= CA1` (~2511–2517). `prointr` reads IFR, and `ppintr` scans it on slot cards.
- **G2. Writing PCR sets CA1** when the polarity bit changes (~2253–2256), so `proinit`'s $6B can raise a spurious flag.
- **G3. Writing IER clears IFR** (`IFR &= IER`, ~2296–2297). The real chip keeps flags regardless of IER.
- **G4. Register 15 (`ORANH2`/`IRANH2`) behaves like register 1** (~1911, ~2412–2424): it clears flags and counts as a ProFile strobe.
- **G5. `VIA_CLEAR_IRQ_PORT_A` skips clearing CA1** when CA2 is in independent mode (~56–66). The real chip always clears CA1 on register-1 access.
- **G6. Duplicate ORA writes dropped** after an `ORANH2` write (~1923–1927), losing a strobe.
- **G7. Coarse CA1 edge detection:** only the net `BSYLine` change across one `ProfileLoop` call counts (`VIAProfileLoop`, ~282–316); BSY changes made elsewhere bypass it.

**Interrupt delivery** (`lib/libGenerator/generator/reg68k.c` ~2505–2512, `irq.c` 676–677):
- **H.** Interrupts are checked only when the CPU loop reaches `cpu68k_clocks_stop`. A CA1 flag set during a port access, or an IER write that unmasks an existing flag, waits for the next scheduled timer.

## Fix order for option 1

1. **Rewrite the ProFile drive as a level-driven state machine** keyed to /CMD and strobes, with the sequence above. /BSY high whenever waiting for the host; no drive-side timeouts; $55 sampled when /CMD rises.
2. **Give the drive its own time base:** schedule `clock_e` as a timer event so the busy period ends on time with one /BSY rising edge.
3. **Make VIA flags datasheet-accurate:** latch CA1 edges regardless of IER; clear only on register 1 or IFR writes; no IFR-read, PCR-write or IER-write side effects; register 15 without handshake. Re-evaluate IRQ immediately after any IFR/IER change.
4. **Remove the OS-specific pieces:** `apply_uniplus_hacks()` RAM patches, `check_running_lisa_os()`-gated BSY/CA1 fakes in `profile.c`, and the loader patch. Keep HLE speed-ups optional and signature-based.
5. **Regression-test** Lisa Office System, Workshop, MacWorks, Xenix, UniPlus 1.4 and `sunix`, and a rebuilt UniPlus V.1.5+ kernel, on the built-in port and the dual parallel card.
