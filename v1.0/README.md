# v1.0

UniSoft UniPlus+ V.1.0 kernel source for the Apple Lisa, from Bitsavers (`bits/Unisoft/V.1.0/`).

- **`sys/`:** kernel sources. Filenames are cut to 8 characters (e.g. `socketsu.c` = `socketsubr.c`, `tcp_inpu.c` = `tcp_input.c`) and the original files use DOS (CRLF) line endings, from an earlier 8.3-style copy.
- **`Unisoft_V.1.0_kernel_listing.pdf`:** a 340-page printout of this `sys/` directory, dated Fri Sep 5 1986. It has an OCR text layer but doesn't include the generic or network (`net/*.h`) headers.

## Files added from V.1.5+

Apart from the three files below, V.1.0 `sys/` has the same content as `../v1.5/sys`, ignoring line endings. The Bitsavers copy was missing two of those files and had one empty. They were added from `../v1.5/sys` on 2026-09-12:

| File in `sys/` | Copied from | Why | Listing pages checked |
|---|---|---|---|
| `mch.s` | `v1.5/sys/mch.s` | Missing from the V.1.0 copy | PDF pp. 3–5 |
| `ivec.s` | `v1.5/sys/ivec.s` | Missing from the V.1.0 copy | PDF pp. 1–2 |
| `tcp_usrr.c` | `v1.5/sys/tcp_usrreq.c` | Present but empty (0 bytes) | PDF pp. 293–295 |

Each file was checked line by line against its pages in the kernel listing. Every printed line matched the v1.5 file: code, comments, labels, constants, commented-out lines, and `tcp_usrreq.c`'s SCCS id `1.59 82/06/20`. So V.1.0 and V.1.5+ have the same content for all three.

Notes:
- A printout can't show tabs versus spaces or trailing whitespace, so whitespace follows the v1.5 copies.
- The added files keep v1.5's Unix (LF) line endings, unlike the CRLF files around them.
- `tcp_usrr.c` keeps the directory's 8-character naming.

## Other notes

- `sys/robots.txt` is an empty file left over from the web download, not part of the kernel.
- The original, unmodified makefile is `sys/makefile` (`REL=5.0`, `/v/sys/...` paths). The copy in `../v1.5/sys/Makefile` has local edits for building on a Lisa.
