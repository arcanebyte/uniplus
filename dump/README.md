# dump

Root filesystem extracted from `../uniplus_unix_10mb_ai_copy.image`, a working UniPlus+ System V install for the Apple Lisa (10 MB ProFile). It was produced with:

```
python3 tools/extract_profile_image.py uniplus_unix_10mb_ai_copy.image dump
```

This `README.md` is the only file not from the disk. The extractor refuses a non-empty directory, so to regenerate, delete `dump/` first.

## Contents

| Path | Description |
|---|---|
| `unix` | Kernel. Non-networking build (`fnet.c` stubs only). |
| `bin`, `usr/bin`, `etc`, `usr/lib` | 68000 a.out binaries and system configuration, dated mostly 1984–85 |
| `lib` | C compiler passes, `crt0.o`, `libc.a` and other libraries |
| `usr/include` | System headers, including `sys/`. No network (`net/`) headers; `sys/errno.h` defines the network errnos. |
| `usr/unix.info`, `READ.THIS` | Notes shipped for Lisa users |
| `dev` | Device nodes as `NAME.special` text files (mode + device number in hex) |

## Notes

- Hard links are separate copies (e.g. `bin/cp` and `bin/mv`).
- `usr/lib/fortran~2` and `usr/lib/pascal~2` are the disk's `/usr/lib/fortran` and `/usr/lib/pascal`. They were renamed because they clash with `Fortran`/`Pascal` on a case-insensitive filesystem.
- Owners, groups and permission bits are not preserved.
