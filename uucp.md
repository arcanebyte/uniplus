# UUCP between macOS and UniPlus+ (LisaEm)

Copy files in both directions between a Mac and UniPlus+ running in LisaEm. It uses UUCP over LisaEm's PseudoTTY serial port, so no copy/paste is involved.

> **Status: untested.** This setup was worked out from the files on the 10 MB UniPlus disk (see `dump/`) and the Taylor UUCP that ships with macOS. It has not yet been run end to end. Please update this page once it has.

## How it works

- **macOS** still ships **Taylor UUCP 1.07** (`/usr/sbin/uucico`, `/usr/bin/uucp`, `uux`, `uustat`). It speaks the old `g` protocol used by 1980s System V UUCP. The `-I` option loads a private config file, so no `sudo` or edits to `/etc/uucp` are needed.
- **UniPlus+** has System V UUCP in `/usr/lib/uucp` (`uucico`, `uuxqt`, `L.sys`, `L-devices`, `USERFILE`). It includes a **`uucp` login with no password** whose shell (`/usr/lib/uucp/uushell`) starts `uucico`. `USERFILE` (`uucp, /`) allows access anywhere, subject to normal file permissions.
- The **Mac calls the Lisa**: the Mac's `uucico` opens the PseudoTTY, the Lisa's Serial B login (`/dev/tty1`) answers, and the Mac logs in as `uucp`. Files queued on either side are exchanged during that one call.

## Requirements

1. **LisaEm Serial B set to PseudoTTY.** File → Preferences → Ports:
   - Serial Port A: `Nothing`. Loopback on either port forces both ports to Loopback.
   - Serial Port B: `PseudoTTY`, xon/xoff **unchecked**, alias `/tmp/lisaem-b`.
   - Restart LisaEm, then check that the alias is fresh (`ls -la /tmp/lisaem-b`) and that LisaEm holds a PTY (`lsof -c lisaem | grep ptmx`). The alias link isn't removed when LisaEm exits, so an old link can point to an unrelated terminal.
2. **UniPlus in multi-user mode.** It boots single-user, so run `init 2`. `/etc/inittab` already runs `getty` on `tty1` at 9600 baud. To go multi-user on every boot, set `is:2:initdefault:`.
3. **Nothing else using the PTY.** Quit picocom or screen before calling, because only one program can use `/tmp/lisaem-b` at a time.
4. **Throttle at 512 MHz** (LisaEm Throttle menu) while transferring. LisaEm deliberately slows incoming data on Serial B to roughly one byte per emulated MHz per second (`SCC_MIN_CYCLES_BETWEEN_READS` in `src/lisa/io_board/z8530.c`). At lower settings `g` protocol packets may time out.

## UniPlus side

1. Find the node name; the Mac must use exactly this name. It may be `upm5` (from `/etc/sys_id`), but check. Old UUCP only compares the first 7 characters.
   ```
   uname -n
   ```
2. Add the Mac to `/usr/lib/uucp/L.sys` as a system that is never called, so jobs addressed to it are queued until the Mac calls:
   ```
   mac Never
   ```

## macOS side

Create a private UUCP tree:
```
mkdir -p ~/uucp/spool ~/uucp/public
```

`~/uucp/config`
```
nodename  mac
spool     /Users/jdenton/uucp/spool
pubdir    /Users/jdenton/uucp/public
logfile   /Users/jdenton/uucp/Log
statfile  /Users/jdenton/uucp/Stats
debugfile /Users/jdenton/uucp/Debug
sysfile   /Users/jdenton/uucp/sys
portfile  /Users/jdenton/uucp/port
```

`~/uucp/port`
```
port lisa
type direct
device /tmp/lisaem-b
speed 9600
carrier false
hardflow false
```

`~/uucp/sys` (replace `upm5` with the Lisa's `uname -n`)
```
system upm5
time any
port lisa
chat "" \r\c ogin:-\r\c-ogin: uucp
protocol g
local-send /
local-receive /Users/jdenton/uucp/public
remote-send /
remote-receive /Users/jdenton/uucp/public
```

## Transferring files

**Mac → Lisa.** `~/` means the Lisa's `/usr/spool/uucppublic`:
```
uucp -I ~/uucp/config -r hello.c 'upm5!~/'
uucico -I ~/uucp/config -S upm5
```

**Lisa → Mac.** Queue on the Lisa; the file is delivered to `~/uucp/public` the next time the Mac calls:
```
uucp file.c 'mac!~/'
```
Then on the Mac:
```
uucico -I ~/uucp/config -S upm5
```

**Status and logs (Mac):**
```
uustat -I ~/uucp/config -a
tail ~/uucp/Log
```

## Troubleshooting

- **Detailed trace:** add `-x 9` to `uucico` and read `~/uucp/Debug`.
- **"Called wrong system":** the `system` name in `~/uucp/sys` doesn't match the Lisa's `uname -n`.
- **Login chat times out:**
  - Check that UniPlus is multi-user and a `login:` prompt appears on Serial B. Test with `picocom /tmp/lisaem-b`, then quit it.
  - Check that `/tmp/lisaem-b` points at LisaEm's current PTY.
- **Transfer stalls or times out:** raise the LisaEm throttle.
- **Permission denied on the Lisa:** files arrive owned by `uucp`. Send to `/usr/spool/uucppublic` (`~/`) or to a directory the `uucp` user can write.
- **LisaEm freezes:** LisaEm blocks when the Lisa sends serial output and nothing on the Mac reads it. Keep a reader attached when you aren't in a UUCP call.

## Security note

The UniPlus `uucp` account has no password, and `USERFILE` gives it access to the whole filesystem. That's fine on an emulator connected only to your Mac, but on a real line set a password with `passwd uucp` and send it with `\P` in the chat script (plus a `call-password`).
