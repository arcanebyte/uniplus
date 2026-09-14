# tools

Host-side utilities (run on a modern machine, not the Lisa).

## extract_profile_image.py

Copies files out of a raw UniPlus+ System V disk image without modifying the image: Apple Lisa ProFile images, and other UniPlus+ machines such as the Torch Triple X. Requires only Python 3.

```
python3 tools/extract_profile_image.py --list IMAGE            # show filesystems found
python3 tools/extract_profile_image.py IMAGE OUTDIR            # extract the first one
python3 tools/extract_profile_image.py --base N IMAGE OUTDIR   # extract the one at sector N
```

- **Image format:** 532-byte Lisa ProFile sectors (20-byte tag + 512 data bytes) or plain 512-byte sectors, detected automatically.
- **Filesystem types:** 512-byte blocks (Lisa) or 1024-byte blocks (Torch), read from the superblock.
- **Filesystem detection:** every filesystem on the disk is found by its superblock and checked by reading its root directory. On the 10 MB Lisa disk the root filesystem is at sector 2501.
- **Regular files:** extracted with their modification times. Hard links become separate copies.
- **Device nodes:** written as `NAME.special` text files containing the mode and device number.
- **Case-only name clashes:** on macOS/Windows, the second name is written as `NAME~2` with a warning.
- **Not preserved:** owner, group, permissions.

See the comment at the top of the script for full details.

## lisa_names.py

Checks C sources for the Lisa `cc`'s limits before they go onto the disk. `cc` keeps 7 characters of an external name, and a function missing from the Lisa's libc only shows up when linking on the Lisa. The script compiles each file with the host `clang` against `dump/usr/include`, lists external names with `nm`, and reports names that collide when cut to 7 characters, references nothing defines (sources, `-l` extras, `dump/lib/libc.a`), and definitions that libc also has. Requires Xcode's command-line tools.

```
python3 tools/lisa_names.py -I netlib/include -l netlib/sockcall.s netlib/ping.c
python3 tools/lisa_names.py -I netlib/include -l netlib/sockcall.s netlib/netdb/*.c netlib/telnet/telnet.c
```
