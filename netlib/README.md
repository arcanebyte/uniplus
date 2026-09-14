# netlib

Minimal userspace networking kit for UniPlus+ V.1.5+ on the Apple Lisa, reconstructed from the surviving kernel source in `../v1.5/sys`.

The kernel's network stack is **4.1a BSD**, not 4.2BSD. There is no `bind`, `listen`, `shutdown`, `sendto` or `recvfrom`:

- **bind:** pass a local address as the 3rd argument of `socket()`.
- **listen:** pass `SO_ACCEPTCONN` as the 4th argument of `socket()`.
- **accept:** returns 0 once the listening socket *itself* is connected. No new descriptor is created.
- **send/receive data:** use `read`/`write`, or `send`/`receive(fd, addr, buf, len)`.

## Files

| File | Description |
|---|---|
| `include/net/socket.h` | `sockaddr`, `sockproto`, socket types, option flags, syscall declarations |
| `include/net/in.h` | `in_addr`, `sockaddr_in`, protocol numbers, `htons`/`htonl` (no-ops on the 68000) |
| `include/net/route.h` | `struct rtentry` and `RTF_` flags for `SIOCADDRT`/`SIOCDELRT` |
| `include/sys/socket.h`, `include/netinet/in.h` | One-line aliases for the `net/` headers |
| `sockcall.s` | `trap #0` stubs for syscalls 70–79: `select`, `gethostname`, `sethostname`, `socket`, `accept`, `connect`, `receive`, `send`, `socketaddr`, `netreset` |
| `tcpconn.c` | Test client: `tcpconn a.b.c.d port [message]` |
| `tcpecho.c` | Test server: `tcpecho port`, echoes one connection at a time (loopback: `tcpecho 5000 &` then `tcpconn 127.0.0.1 5000 hello`) |
| `looptest.c` | One-shot loopback test: forks an echo server, connects to 127.0.0.1, checks the echo and prints PASS or FAIL (`looptest [port]`) |
| `udptest.c` | One-shot UDP loopback test: forks a datagram echo server, sends to 127.0.0.1 with `send()`, checks the reply from `receive()` and prints PASS or FAIL (`udptest [port]`) |
| `netinfo.c` | Shows the host name (`gethostname`) and the Internet address compiled into the kernel (`SIOCGIADDR`) |
| `ping.c` | ICMP echo on a raw socket: `ping [-d] [-w seconds] a.b.c.d [count]`, run as root. `-w` is how long to wait for each reply (default 2); late replies still count and duplicates are marked `(DUP!)`. `-d` turns on the kernel's ICMP console messages and shows the raw input queue. Needs a kernel with the raw socket fixes (see below). |
| `route.c` | Routing table: `route [show]`, `route add dest gateway`, `route delete dest gateway`; `dest` is an address or `default`. `show` reads the kernel's `rthost`/`rtnet` tables through `/unix` and `/dev/kmem`; add and delete need root. A default route needs the `rtalloc()` fallback in `../v1.5/sys/route.c`. |
| `nc.c` | Netcat: `nc [-v] [-c] [-q] [-w secs] a.b.c.d port`, or `-l port` to listen; `-u` for UDP. A modern test and convenience tool, outside the period (netcat is from 1995) |
| `httpd.c`, `www/index.html` | Web server: `httpd [-p port] [docroot]`, `.html`/`.htm`/`.txt` from `/usr/www`. A modern test and convenience tool, outside the period (the web is from the early 1990s) |
| `ifconfig.c` | `ifconfig [interface]` lists interfaces from the kernel; `ifconfig eb0 a.b.c.d` sets the address (root) |
| `include/netdb.h`, `include/bsd.h` | Network data base declarations, and the BSD types and `bcopy`/`bzero`/`index` names mapped onto System V |
| `include/arpa/nameser.h`, `include/resolv.h` | 4.3BSD name server packet format and resolver state |
| `netdb/` | `libnetdb.a`: `gethostbyname` (name server, then `/etc/hosts`), `getservbyname` and the rest, plus `inet_addr`/`inet_ntoa`; `hosttest` checks it |
| `etc/` | Sample `/etc/hosts`, `networks`, `protocols`, `services`, `resolv.conf` |
| `telnet/` | `telnet [host [port]]`, from 2.9BSD (4.1c): escape `^]` for `close`, `quit`, `status`, `options`, `escape` |
| `tftp/` | `tftp [host [port]]`, from 2.9BSD (4.1c): `connect`, `mode binary`, `get`, `put`, `trace`, `status` |
| `ftp/` | `ftp [-v] [-d] [-i] [-n] [-p] [host [port]]`, from 2.9BSD (4.1c): `open`, `user`, `binary`, `get`, `put`, `ls`, `dir`, `cd`, `pwd`, `mkdir`, and the modern `passive` |
| `telnetd/` | `telnetd [-d] [port]`, from 2.9BSD (4.1c): logins over telnet on the kernel's pseudo-terminals; `mkptys` makes `/dev/ptyp?` and `/dev/ttyp?` |
| `netstat/` | `netstat [-Aaimnrst] [interval]`, from 2.9BSD (4.1c): TCP/UDP connections, interfaces (`-i`), routes (`-r`), mbufs (`-m`), protocol statistics (`-s`) |
| `Makefile` | Builds `tcpconn`, `tcpecho`, `looptest`, `udptest`, `netinfo`, `ping`, `route`, `nc`, `httpd` and `ifconfig` on the Lisa (`netdb/` has its own) |

## Host names and services: libnetdb.a

`netdb/` is the 4.1c BSD network data base library as back-ported in 2.9BSD (`../bsd/2.9BSD/usr/net/src/net`), for programs ported from Berkeley, with the host routines replaced by 4.3BSD's name server versions (below):
- `gethostbyname`, `gethostbyaddr`; `getnetbyname`, `getnetbyaddr`; `getservbyname`, `getservbyport`; `getprotobyname`, `getprotobynumber`; the `set`/`end` routines;
- `inet_addr`, `inet_network`, `inet_netof`, `inet_lnaof`, `inet_makeaddr`, and 4.2BSD's `inet_ntoa`.

Changes for the Lisa:
- **Data bases in `/etc`:** `/etc/hosts`, `/etc/networks`, `/etc/protocols`, `/etc/services` (2.9BSD used `/usr/lib`; UniSoft's own programs on the Torch disk use `/etc/hosts`).
- **Short names:** `cc` keeps 7 characters of an external name, so `netdb.h` renames the routines apart (`gethostbyname` is `gethbyname`, `sethostent` is `sethent`, and so on). Always include `<netdb.h>` rather than declaring them yourself.
- **Byte order:** `inet_netof`, `inet_lnaof` and `inet_makeaddr` are rewritten for the 68000 using the class masks from UniSoft's `net/in.h`, now in `include/net/in.h`.
- **`bsd.h`** (included by `netdb.h`) supplies `u_char`/`u_short`/`u_int`/`u_long` and maps `bcopy`, `bzero`, `bcmp`, `index`, `rindex` to libc's `memcpy`, `memset`, `memcmp`, `strchr`, `strrchr`.
- `struct in_addr inet_makeaddr();` has to be declared after `net/in.h` by the caller.
- Not ported: `rcmd`, `rexec`, `rhost`, `raddr` (for `rsh`/`rlogin`, not yet needed). `ruserpass` is in `ftp/`.

### Name server (resolver)

`gethostbyname` and `gethostbyaddr` are 4.3BSD's (`named/gethostnamadr.c` 6.12, `sethostent.c`, `res_comp.c`, `res_init.c`, `res_mkquery.c`, `res_send.c`, `arpa/nameser.h`, `resolv.h`, from the 4.3BSD tape: https://www.tuhs.org/cgi-bin/utree.pl?file=4.3BSD/usr/src/lib/libc/net). They send a query to each name server in `/etc/resolv.conf` (`nameserver a.b.c.d`, up to 3; `domain name` sets the default domain added to names without a dot) and read the answer, including CNAMEs and several addresses. `struct hostent` now has 4.3BSD's `h_addr_list`, with `h_addr` as the first entry, and failures set `h_errno` (`HOST_NOT_FOUND`, `TRY_AGAIN`, `NO_RECOVERY`, `NO_ADDRESS`). `res_debug.c` (the `RES_DEBUG` printout) is not included.

Changes for the Lisa:
- **4.1a sockets:** queries go by datagram with `send()` and `receive()`, and `select()` with a timeout in milliseconds waits for the answer. `select()` is given one descriptor more than it should need: the kernel's `selscan()` (`../v1.5/sys/syslocal.c`) numbers descriptors from 1 and stops at `nfds`, so before that was fixed (branch `lisa-select`) it never checked descriptor `nfds-1`; the extra one keeps older kernels working. 4.3BSD's TCP path (`RES_USEVC`, and retrying a truncated answer over TCP) is left out: a truncated answer is used as it is, and `sethostent(1)` only keeps the datagram socket open.
- **`/etc/hosts` after the name server:** when the name server gives no answer, for any reason, `/etc/hosts` is searched. 4.3BSD did that only when no name server was running, because a site with `named` served its own host names; the servers the Lisa can reach don't know `lisa` or `gateway`.
- **No `nameserver` line, no name server:** 4.3BSD then asked `named` on the local host. The Lisa has none, so it goes straight to `/etc/hosts`. Remove `/etc/resolv.conf` to work without a network.
- **Bug fixes:** `res_mkquery` used System V's `sprintf()` return value as a string (BSD's returns the buffer), and it never cleared the header's unused bits, so a query carried whatever was on the stack there (servers now read those bits as the DNSSEC AD and CD flags); a PTR answer left the alias list of the previous lookup.
- **Names:** `gethostnamadr.c` is `gethnamadr.c` (14 characters). `_gethtbyname` and `_gethtbyaddr` are `_ghtbyname` and `_ghtbyaddr` (7 characters), through `netdb.h`. `sethostfile()` (which did nothing, and clashes with `sethostname()`) is left out, and so is `gethostent()`, which 4.3BSD's name server library didn't have either.
- **Bit fields:** `nameser.h` uses the "bit zero on left" layout of `HEADER` for `mc68000`; the kernel's own `net/ip.h` and `net/tcp.h` depend on the Lisa `cc` assigning bit fields that way. `hosttest` checks it.

Timeouts are 4.3BSD's: 4, 8, 16 and 32 seconds per try, divided among the name servers. If slirp's DNS at 10.0.2.3 doesn't answer (the Mac is offline), a lookup takes about a minute before it falls back to `/etc/hosts`.

Programs that use the library must now also link `../sockcall.o` (the resolver calls `socket`, `send`, `receive`, `select` and `gethostname`); `telnet`, `tftp`, `ftp` and `telnetd` already did, and `netstat` now does (its global variable `socket` in `inet.c` became `static` to keep it apart from the `socket()` stub). Rebuild them to get name server lookups. `nc`, `ping` and `httpd` take only numeric addresses and don't use the library.

On the Lisa:
```
cd /usr/src/netlib/netdb
make                # libnetdb.a and hosttest
make install         # copies ../etc/hosts, networks, protocols, services, resolv.conf to /etc (root)
./hosttest           # PASS/FAIL for the resolver's packets and each lookup against the sample files
./hosttest lisa gateway 10.0.2.3
./hosttest www.tuhs.org 8.8.8.8     # through slirp's name server at 10.0.2.3
```
Link programs with `../sockcall.o ../netdb/libnetdb.a` (after their own objects). `make install` overwrites `/etc/hosts` and `/etc/resolv.conf`; merge by hand if you already have them.

Checked on the Mac so far (September 2026): the library builds and `hosttest` passes against `etc/` when compiled natively, and `tools/lisa_names.py` finds no 7-character clashes or missing libc routines. Not yet built on the Lisa.

The resolver (September 2026) is checked on the Mac only: compiled natively with shims for the 4.1a socket calls, against a scripted name server (A, CNAME with two addresses, PTR, NXDOMAIN falling back to `/etc/hosts`, an answer with a stale ID, a truncated answer, the default domain, a timeout with two tries, no name server) and against 1.1.1.1. `tools/lisa_names.py` passes for the library, `hosttest`, `telnet`, `tftp`, `telnetd` and `netstat`, and for `ftp` apart from `environ`, as before. Tested on the Lisa (September 2026), on a kernel with the `select()` fix: `hosttest` passes (including the packet layout and `HEADER` bit field checks), `restest www.tuhs.org` gets slirp's answer at once and `gethostbyname` returns `oldminnie.tuhs.org`, alias `www.tuhs.org`, 50.116.15.146, and `ftp` connects by host name. `telnet`, `tftp`, `ftp`, `netstat` and `telnetd` build against the new library, and `telnetd` works. Not yet tried on the Lisa: the `s + 2` workaround on an unfixed kernel, and `tftp`, `telnet` and `netstat` with host names.

## telnet and tftp

Both come from 2.9BSD's network kit (`../bsd/2.9BSD/usr/net/src/netser`) and link with `sockcall.o` and `netdb/libnetdb.a`. Build the library first, then `make` in `telnet/` and `tftp/`.

- **telnet:** the kernel's `select()` can't wait on a terminal, so a connection runs as two processes: the parent reads the network and writes the screen, a child reads the keyboard. The escape character (`^]`) gives the `telnet>` prompt; an empty line goes back to the connection. In LisaEm on macOS, `^]` could not be typed: Option-], Control-] and Command-] gave `]` or nothing, and Option-C didn't interrupt (September 2026), so LisaEm isn't sending the Lisa's Apple key, which UniPlus uses as Control, so set another escape character before connecting: run `telnet` with no host, then `escape`, then `` ` ``, then `open host`. That works on the Lisa. Terminal modes use termio. There is no `z` (no job control), and `close` just closes (no `shutdown()`). End of line is sent as 4.3BSD sends it: a newline as CR LF, a carriage return as CR NUL, and Return stays a carriage return while the remote side echoes (4.1c sent a bare newline, which some servers, such as telehack.com, ignore).
- **tftp:** fixed for current servers: it follows the server's transfer port instead of sending everything to port 69, and binary mode sends `octet` (4.1c sent `octect`). Use `mode binary` for anything but text; `ascii` mode does no CR/LF conversion.

```
telnet 10.0.2.2 2323        # a telnet server on the Mac, through slirp
tftp gateway                # tftp> mode binary, get file, put file, quit
```

Checked on the Mac so far, with host shims for termio and the 4.1a socket calls: telnet against a scripted server (option negotiation, typing, the escape commands, remote close) and tftp get/put against an RFC 1350 server, including a dropped packet. Not yet built on the Lisa.

## ftp

The 4.1c BSD ftp client, as 2.9BSD converted it for the 4.1a socket calls (`compat.c` supplies 4.2BSD-style `accept`, `connect` and friends). For the Lisa:
- **Passive mode** (a modern convenience, outside the period: the PASV command is in RFC 765 from 1980, but BSD's ftp client only gained a `passive` command years later): `ftp -p` or the `passive` command makes ftp connect to the port the server gives in its PASV reply, instead of listening for the server (PORT). Use it through slirp's NAT; LisaEm's slirp can also rewrite PORT for servers on port 21.
- **Binary transfers fixed:** 4.1c kept each byte in a `char`, so a 0377 byte ended a transfer as if it were EOF.
- **Login:** 4.1c's `ruserpass()` (from 2.9BSD's `net/common`): a `MACHhost=login,password` environment variable with the password encrypted by its own DES code, keyed to your terminal's `/etc/utmp` entry; then `machine`/`login`/`password` entries in `$HOME/.netrc` (which must not be readable by others if it holds a password); then it asks. For System V it takes only the `USER_PROCESS` utmp entry for the terminal. Nothing on the Lisa makes the encrypted variables (4.1c's `mkpwunclear()` is in the file but no program calls it), so in practice use `.netrc` or type the password.
- `pwd`, `mkdir` and `rmdir` send 4.1c's `XPWD`, `XMKD` and `XRMD` (the experimental commands of RFC 775, the names of the time). Servers still accept them; RFC 959's `PWD`, `MKD` and `RMD` came in 1985.
- Transfer times are in whole seconds (no `gettimeofday()`). Names that clash in 7 characters are renamed in `varpat.h`, as 2.9BSD did for the PDP-11.
- The shell escape (`!`) is unimplemented, as in 4.1c, and there is no `mget`/`mput`.

```
ftp -p 10.0.2.2           # an FTP server on the Mac, through slirp
ftp> binary
ftp> get file
```

Checked on the Mac with host shims for the 4.1a socket calls, against pyftpdlib: login from `.netrc`, by prompting and from an encrypted environment variable, active and passive get and put of binary files (byte for byte), ascii get, `ls`, `dir`, `pwd`, `mkdir`, `rmdir`. Not yet built on the Lisa.

## telnetd

The 4.1c BSD telnet server, so you can telnet into the Lisa. For the Lisa:
- **Pseudo-terminals:** the kernel's `pty.c` (16 pairs). `sh mkptys` as root makes `/dev/ptyp0`-`f` (controlling side, major 20) and `/dev/ttyp0`-`f` (terminal side, major 21).
- **System V login:** `/bin/login` refuses to run without a utmp entry for its process ("No utmp entry"), which getty normally leaves, so telnetd writes a `LOGIN_PROCESS` entry for the pseudo-terminal first. At logout it marks the entry `DEAD_PROCESS` and adds a wtmp record.
- **Controlling terminal:** the login process calls `setpgrp()` before opening its pseudo-terminal, so ^C and hangup reach the session (System V has no `TIOCNOTTY`).
- **Modes:** termio instead of sgtty, set with `TCSETA` (in `pty.c`, `TCSETAW` from the controlling side throws away pending output).
- **End of session:** `SIGCLD` from the login process; the controlling side of a pty doesn't read end of file here.

On the Lisa, as root:
```
cd /usr/src/netlib/telnetd
make install                  # /etc/telnetd and the pty device files
/etc/telnetd                  # detaches; add it to /etc/rc to start at boot
```
From the Mac, with LisaEm started with `LISAEM_ETHERBOX_HOSTFWD=tcp:2323:23`: `telnet 127.0.0.1 2323`.

Not yet built or run: telnetd needs the Lisa's pseudo-terminals and login, so it has only been through `tools/lisa_names.py`.

## netstat

The 4.1c BSD netstat, reading the kernel's tables through `/unix` and `/dev/kmem` (so `/unix` must be the running kernel):
- `netstat` / `netstat -a`: TCP and UDP connections (with `-a`, listening sockets too), with receive and send queues and TCP state.
- `netstat -i [interval]`: interfaces with packet and error counts; with an interval, a running display.
- `netstat -r`: the host and network routing tables (a default route shows as `default`).
- `netstat -s`: IP, TCP and UDP error counts, in 4.2BSD's wording, from the kernel's `ipstat`, `tcpstat` and `udpstat`. This kernel keeps only a few counters (bad checksums, short or malformed headers and lengths, unacknowledged TCP packets) and no ICMP statistics, so that is all `-s` shows.
- `netstat -m`: mbuf counts. `-n` shows numbers instead of names from `/etc/hosts` and `/etc/services`.

For the Lisa it is built against the kernel's own headers, so the structures it reads match the kernel without hand-written offsets: `make KINC=/usr/src/include` (the default), where the kernel was built from. Also: `nlist` from `<a.out.h>`, the IMP host table (`-h`) and VAX crash-dump paging removed, addresses printed with bytes masked because the kernel's `u_char` is signed, and `-s` added from 4.2BSD.

Only checked with `tools/lisa_names.py` so far (against a copy of the kernel headers with two lines clang rejects fixed); not yet built on the Lisa.

## Routes

The kernel adds a network route for each interface (10.0.0.0 on `eb0`, 127.0.0.0 on `lo0`). To reach anything else through LisaEm's slirp network, add a default route to slirp's host address:
```
route add default 10.0.2.2
```
Routes are not kept across reboots; put that line in `/etc/rc` to make it permanent.

`route` follows 4.1a BSD: a destination whose host part is non-zero for its address class is a host route, anything else a network route, and the route goes through a gateway unless the gateway is one of the Lisa's own addresses. The 4.1a kernel only matched host and network routes; the default route is a later addition to `rtalloc()` (`../RESTORATION.md` section 9).

## Raw sockets and ping

A raw ICMP socket needs the fixes in `../v1.5/sys` `proto.c`, `raw_cb.c`, `raw_usrreq.c`, `raw_ip.c` and `ip_icmp.c` (`../RESTORATION.md` section 9). On a kernel without them, `socket()` for raw ICMP calls a null pointer and crashes the system. `ping` therefore reads the running kernel's protocol table through `/unix` and `/dev/kmem` first and refuses to run if the raw ICMP entry is missing. `/unix` must be the kernel that is running.

In a received echo reply the kernel has stripped the IP header and zeroed the ICMP checksum. `icmp_input()` treats the first data byte as the header length of a quoted IP packet and drops replies too short for it, so `ping` sends a 0 there.

## Status

- **Confirmed:**
  - Struct layouts and syscall numbers come from the kernel source.
  - The register calling convention matches the stubs in the shipped `/lib/libc.a` (see `../dump`).
  - Network errno values (55–85) are already in the stock `<sys/errno.h>`, and `perror()` knows their text.
- **Unverified:** constants tagged `[B]` in the headers (`SOCK_STREAM`, `AF_INET`, `SO_*`) are taken from 4.1a/4.1c BSD. The socket ioctls `FIONBIO`, `SIOCGIADDR` and `SIOCCIADDR` come from UniSoft's `net/misc.h` (Torch headers) and match `soioctl()` in the kernel.
- **Tested on a Lisa (13 September 2026):** on `unix.net`, `tcpconn 127.0.0.1 5000 hello` builds and gets "Connection refused" from the loopback TCP stack, which exercises the headers, `sockcall.s`, `socket()` and `connect()`. `looptest` passes: a full TCP round trip over 127.0.0.1 (fork, listening socket with `SO_ACCEPTCONN`, `accept`, `connect`, `read`/`write` both ways, `close`). `udptest` passes: a datagram sent with `send()` from an ephemeral port (1025) and echoed back with `receive()` returning the sender's address. `netinfo` prints `hostnameunknown` and 89.0.41.8 through `gethostname()` and `SIOCGIADDR` (10.0.2.15 on kernels built with the current `conf.c`).
- **Tested over Ethernet (LisaEm EtherBox with slirp):** `tcpconn` from the Lisa to a listener on the Mac, `tcpecho` on the Lisa reached from the Mac through a port forward, and `ping` to 127.0.0.1 and 10.0.2.2. With `route add default 10.0.2.2`, `ping 8.8.8.8` gets 4 of 4 replies from the internet at 5 MHz. At much higher emulated speeds the Lisa's clock runs ahead of real time and replies come back after ping's wait.
- **Lisa `cc` limit:** struct tags are unique only to 8 characters, so `in.h` maps `sockaddr_in` to `sock_in`, as the kernel's `net/misc.h` does. Keep new identifiers unique within 8 characters, and external names within 7. `cc` truncates external names to 7 characters plus `_`, but `as` keeps labels whole, so `sockcall.s` spells the long calls as `cc` emits them (`_gethost`, `_sethost`, `_socketa`, `_netrese`).

If `sockcall.s` won't assemble, libc's generic `syscall()` works instead, e.g. `syscall(73, SOCK_STREAM, 0, 0, 0)`.
