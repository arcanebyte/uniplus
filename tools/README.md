# tools

Host-side utilities (run on a modern machine, not the Lisa).

## extract_profile_image.py

Copies files out of a raw UniPlus+ ProFile disk image without modifying the image. Requires only Python 3.

```
python3 tools/extract_profile_image.py --list IMAGE      # show filesystems found
python3 tools/extract_profile_image.py IMAGE OUTDIR      # extract into a new/empty dir
python3 tools/extract_profile_image.py --base N IMAGE OUTDIR   # pick a filesystem by sector
```

- **Image format:** 532-byte ProFile sectors (20-byte tag + 512 data bytes) or plain 512-byte sectors, holding a System V filesystem with 512-byte blocks.
- **Filesystem detection:** found automatically by its superblock. On the 10 MB disk the root filesystem is at sector 2501.
- **Regular files:** extracted with their modification times. Hard links become separate copies.
- **Device nodes:** written as `NAME.special` text files containing the mode and device number.
- **Case-only name clashes:** on macOS/Windows, the second name is written as `NAME~2` with a warning.
- **Not preserved:** owner, group, permissions.

See the comment at the top of the script for full details.
