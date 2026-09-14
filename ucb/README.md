# ucb

Berkeley versions of commands, installed beside UniPlus+ System V's own under a `7` suffix. The root `.cshrc` shipped on the Lisa disk (July 1984) already has `alias ls ls7 -F`, so these fill in programs that UniSoft's own systems evidently had.

| Program | From | What it adds over System V |
|---|---|---|
| `ls7` | 2.9BSD `net/src/cmd/ls.c` (4.2BSD `ls.c` 4.5 82/03/31, with Tektronix's 1983 user-and-group `-l`) | Columns by default on a terminal (`-C`, `-1`), `-F` (`/` for directories, `*` for executables), `-A`, `-R`, `-q`, `-s` in kilobytes; owner and group in `-l` |
| `rm7` | 2.9BSD `src/cmd/rm.c` (4.3 Berkeley 1/4/81) | Options may be separate arguments (`rm7 -i -f`); with both, `-i` still asks |

Changes for the Lisa:
- **`ndir.c`/`ndir.h`:** 4.2BSD's `opendir`/`readdir`/`closedir` over System V's 16-byte directory entries, returning null-terminated names. Include `<sys/types.h>` before `ndir.h`.
- **`ls7`:** no symbolic links, so `lstat` is `stat`, `-L` does nothing and `-F` never shows `@`; the tab check uses termio (`TAB3`) instead of sgtty (`XTABS`).
- **`rm7`:** `access()` mode 2 spelled out (no `<sys/file.h>`); no symbolic links.

```
cd /usr/src/ucb
make
make install        # /usr/bin/ls7, /usr/bin/rm7
```

Checked on the Mac: `ls7` builds and lists a directory in columns, with `-F`, `-l`, `-a` and `-R` (using the host's `opendir`); `ndir.c` reads a file laid out as a System V directory, skipping deleted entries and ending 14-character names; `rm7 -i -f` asks, `-f` is quiet about missing files. `tools/lisa_names.py` passes. Not yet built on the Lisa.
