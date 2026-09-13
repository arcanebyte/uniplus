# Faithful ProFile + VIA emulation in LisaEm: implementation plan

**Status (13 September 2026):** implemented as lisaem PR #55 (https://github.com/arcanebyte/lisaem/pull/55). UniPlus 1.4, V.1.5+ `unix.nonet` and `unix.net`, and LOS 3.1 boot on the built-in port; remaining tests are listed in the lisaem repo's `ProFileEmulationTesting.md`.

Branch `profile-emulation` in `~/github/lisaem`. Background and references are in `profile-emulation-notes.md`; the deficiency letters (A–H, G1–G7) below refer to that file.

## What the study found

Signal conventions in LisaEm stay as they are: `CMDLine=1` means /CMD is asserted (low), and `BSYLine=1` means /BSY is asserted (low, drive busy). Viewed from the host, the CA1 interrupt UniPlus waits for (PCR bit 0 = 1) is `BSYLine` going **1 → 0**.

UniPlus `pro.c` drives every phase like this:

| Host step | Access | Drive must |
|---|---|---|
| `prochk`: assert BSY high, /CMD low | IRB read, ORB write | already have /BSY high; answer $01/$02/$03/$06 with /BSY low |
| poll up to `RSPTIME` for /BSY low, read reply | IRB reads, reg-1 read | assert /BSY promptly |
| write $55 (reg 1), IER=$82, /CMD high, return | reg-1 write, IER, ORB | sample $55 **at /CMD rising**, then later raise /BSY once (CA1 edge) |
| `prointr`: `d_ifr = d_ifr`, move bytes via reg 1 | IFR read/write, reg-1 strobes | keep /BSY high while bytes move |

Why the current code fails an unpatched kernel:
- In `GET_CMDBLK_STATE`, every ORA write sets `BSYLine=1`. The UniPlus-only branch forces `BSYLine=1` as well. So when `prochk` runs after the six command bytes, /BSY is low and `ASSERT((d_irb&BSY)==BSY)` fails. That is the ASSERTION BSY our rebuilt kernels hit; 1.4 survives only because of the RAM patch at `0x20f9c`.
- When a transfer finishes, `SEND_DATA_AND_TAGS_STATE` sets `BSYLine=1` and returns to idle.
- The busy period only ends when something calls `ProfileLoop`, so the CA1 edge waits for an unrelated timer event.

## Design

### 1. Level-driven drive (`src/storage/profile.c`)

`ProfileLoop()` is replaced by a state machine that follows the ESProFile/Aphid sequence. It reacts to line levels and strobes, and **has no drive-side timeouts**.

| State | /BSY | Leaves on | Action |
|---|---|---|---|
| `IDLE` 0 | high | /CMD low (level) | bus=$01, /BSY low → `HANDSHAKE` |
| `HANDSHAKE` 2 | low | /CMD high | bus==$55 ? schedule "ready" : status no-55 → `IDLE` |
| `GET_CMDBLK` 4 | high | /CMD low | strobes store bytes 4..9; on /CMD low bus=cmd+2 ($02/$03/$04), /BSY low → `HANDSHAKE` |
| `BUSY` 6/9 | low | timer (`clock_e`) | read: `do_profile_read`, `indexread=0` → `SEND_DATA`; write: `do_profile_write` → `SEND_STATUS`; ready-for-data → `ACCEPT_WRITE`; ready-for-cmd → `GET_CMDBLK`; then /BSY high (one CA1 edge) |
| `ACCEPT_WRITE` 7 | high | /CMD low | strobes store bytes 10..541; on /CMD low bus=$06, /BSY low → `HANDSHAKE` |
| `SEND_DATA` 10 / `SEND_STATUS` 12 | high | /CMD low | read strobes return `DataBlock[indexread++]`; /CMD low starts the next command |

- `HANDSHAKE` records which reply it sent ($01, cmd+2, or $06). That tells the scheduled "ready" event where to go next.
- State numbers 10 and 12 and the `DataBlock`/`indexread`/`indexwrite` layout are unchanged. The existing HLE intercepts (`hle.c`: LOS 3.1 and UniPlus 1.4) keep working.
- Delays are named constants: `PROFILE_HANDSHAKE_DELAY` (≈100 µs) and `PROFILE_RW_DELAY` (≈1 ms). They are long enough that the host finishes its return path before the edge, and short enough to keep disk I/O fast.
- Removed from `profile.c`:
  - `CHECK_PROFILE_LOOP_TIMEOUT` and the `last_cmd` edge logic;
  - the UniPlus/Xenix `BSYLine` and `IFR` fakes (~1106, ~1116, ~1538, ~1639);
  - the loader handshake patches at `0x6281c`/`0x62a52`.
- Kept: the BLU serial-number loader patch, which is unrelated to disk timing.

### 2. Drive time base (`src/lisa/cpu_board/irq.c`)

- `get_next_timer_event()` considers `via[i].ProFile->clock_e` (when > now) as a candidate, with ID `CYCLE_TIMER_VIAn_CA1(i)` = i+32. That ID is already defined and unused.
- `check_current_timer_irq()` dispatches that ID to `VIAProfileLoop(i, P, PROLOOP_EV_NUL)`, then reschedules.
- A helper in `profile.c` sets `clock_e` and pulls `cpu68k_clocks_stop` forward when the new event is earlier, like `FIX_CLKSTOP_VIA_T1`.
- `ProfileLoop` also completes an overdue busy period on any event, as a safety net.

### 3. Datasheet 6522 flags for the parallel VIAs (`src/lisa/io_board/via6522.c`)

Applies to both the motherboard `via[2]` handlers and the slot-card `lisa_*_ext_2par_via`:
- **G7:** CA1 is latched only in `VIAProfileLoop`, on a `BSYLine` edge matching PCR bit 0, regardless of IER. That is already the one place `BSYLine` changes once the fakes are gone.
- **G1:** IFR read no longer invents CA1 from `BSYLine`.
- **G2:** PCR write no longer sets CA1.
- **G3:** IER write no longer clears IFR.
- **G4/G6:** register 15 moves data without handshake: no flag clearing, no strobe. Register 1 clears CA1 always and CA2 unless independent. With CA2 in handshake/pulse mode it strobes the drive. The "duplicate ORA after ORANH" drop goes.
- **G5:** `VIA_CLEAR_IRQ_PORT_A` always clears CA1. `cops.c` keeps its own copy.
- **Manual CA2:** if an OS strobes /PSTRB by writing PCR (CA2 low→high in manual mode), that counts as a strobe. Accesses that would not strobe a real drive during a data phase are logged once (`ALERT_LOG`), so a regression shows up in the log.
- **H:** when CA1 is latched or IER is written and `IFR & IER` becomes non-zero, the CPU loop is made to stop after the current instruction (`next_expired_timer=0`, `cpu68k_clocks_stop=cpu68k_clocks`). Pending interrupts are then evaluated immediately; `get_next_timer_event()` recomputes the real next timer.

### 4. Remove OS-specific ProFile patches (`src/storage/hle.c`)

- `apply_uniplus_hacks()` loses the `0x20f9c`/`0x210b0` (1.4) and `0x1fe24`/`0x1ff38` (sunix) handshake patches.
- It keeps detection, the optional `0xc188` idle speed-up and the HLE-only intercepts.
- This is a separate change so it can be reverted on its own if 1.4 needs it during testing.

### 5. Build and test

- Build with `./build.sh` on the branch. The master build is the A/B reference.
- Test matrix, run by the user (built-in port, then dual parallel card, both ports where the OS supports it):
  1. Boot ROM → boot from ProFile (LOS 3.1 image)
  2. LOS 3.1 with HLE off, and with HLE on
  3. Workshop
  4. MacWorks XL
  5. Xenix
  6. UniPlus 1.4 `/unix`, and the 20 MB `/dev/p2h` mount
  7. sunix 1.1
  8. Rebuilt V.1.5+ `unix.nonet`, then `unix.net`
  9. LisaTest ProFile tests, if available
- Signs to look for: BSY assertions, "EXCESSIVE DISK DELAY", boot hangs, `ALERT_LOG` strobe warnings in the log.
