#!/usr/bin/env python3
"""
extract_profile_image.py -- copy files out of a UniPlus+ (UniSoft System V, 68000) disk image.

Works on Apple Lisa ProFile images and on other UniPlus+ hard disk images such
as the Torch Triple X disks on Bitsavers.

Read-only: the image is never modified.  Needs only Python 3, no extra modules.

USAGE
    python3 tools/extract_profile_image.py IMAGE OUTDIR
    python3 tools/extract_profile_image.py --list IMAGE
    python3 tools/extract_profile_image.py --base N IMAGE OUTDIR

    IMAGE    raw disk image, e.g. uniplus_unix_10mb_ai_copy.image
    OUTDIR   new or empty directory to extract into (created if missing;
             a non-empty directory is refused so nothing gets overwritten)

    --list   only report the System V filesystems found and exit
    --base N extract the filesystem starting at sector N instead of the first
             one found (use the sector numbers printed by --list); needed when
             a disk holds several filesystems

EXAMPLES
    # see what is on the disk
    python3 tools/extract_profile_image.py --list uniplus_unix_10mb_ai_copy.image

    # dump the Lisa root filesystem into ./dump
    python3 tools/extract_profile_image.py uniplus_unix_10mb_ai_copy.image dump

    # a multi-filesystem disk: list, then extract each one
    python3 tools/extract_profile_image.py --list Torch_Triple-X_SysV_Portable_Disk.img
    python3 tools/extract_profile_image.py --base 20504 Torch_Triple-X_SysV_Portable_Disk.img out

WHAT YOU GET
    - Regular files with their contents and modification times.
    - Hard links: every name is written as a separate copy (e.g. /bin/cp and
      /bin/mv are identical files, not links).
    - Device nodes, pipes: a text file NAME.special holding the octal mode and
      device number in hex, e.g. "mode 20666 addr0 401" = char device major 4
      minor 1.
    - Names differing only in case (the 10 MB Lisa disk has /usr/lib/Pascal
      and /usr/lib/pascal) collide on macOS/Windows; the later one is written
      as NAME~2 and a warning is printed.
    - Not preserved: owner, group, permission bits.

IMAGE FORMATS
    - Sectors: 532 bytes (20-byte Lisa ProFile tag followed by 512 data bytes)
      or plain 512 bytes; detected automatically.
    - Filesystem: UniPlus+ System V, big-endian, superblock magic 0xfd187e20,
      64-byte inodes, 14-character names.  Block size from the superblock
      s_type: 1 = 512-byte blocks (Lisa), 2 = 1024-byte blocks (Torch).
    - The superblock is the second 512 bytes of the filesystem, so --list
      reports the filesystem start as the sector before it.
    - On a 10 MB Lisa ProFile the root filesystem starts at sector 2501 (after
      101 boot sectors and 2400 swap sectors; see prlmap[] in v1.5/sys/pro.c).
"""
import argparse
import os
import struct
import sys

FS_MAGIC = 0xfd187e20
BLOCK_SIZES = {1: 512, 2: 1024}


class Image:
    def __init__(self, raw, sector, tag, base):
        self.raw, self.sector, self.tag, self.base = raw, sector, tag, base
        sb = self.sectordata(base + 1)
        self.fstype = struct.unpack('>I', sb[508:512])[0]
        self.bsize = BLOCK_SIZES.get(self.fstype, 0)
        self.isize = struct.unpack('>H', sb[0:2])[0]
        self.fsize = struct.unpack('>I', sb[2:6])[0]
        self.fname = sb[0x1ec:0x1f2].split(b'\0')[0].decode('latin1')

    def sectordata(self, n):
        o = n * self.sector + self.tag
        return self.raw[o:o + 512]

    def block(self, n):
        per = self.bsize // 512
        first = self.base + n * per
        return b''.join(self.sectordata(first + i) for i in range(per))

    def inode(self, ino):
        per_block = self.bsize // 64
        b, off = divmod(ino - 1, per_block)
        d = self.block(2 + b)[off * 64:(off + 1) * 64]
        mode, nlink, uid, gid, size = struct.unpack('>HHHHI', d[:12])
        addr = [int.from_bytes(d[12 + 3 * i:15 + 3 * i], 'big') for i in range(13)]
        mtime = struct.unpack('>I', d[56:60])[0]
        return mode, size, addr, mtime

    def valid(self, bn):
        return self.isize <= bn < self.fsize

    def indirect(self, bn, level, out):
        if not self.valid(bn):
            return
        nptr = self.bsize // 4
        for p in struct.unpack('>%dI' % nptr, self.block(bn)):
            if level == 1:
                out.append(p)
            else:
                self.indirect(p, level - 1, out)

    def data(self, ino):
        mode, size, addr, _ = self.inode(ino)
        blocks = list(addr[:10])
        for level, a in zip((1, 2, 3), addr[10:]):
            self.indirect(a, level, blocks)
        need = (size + self.bsize - 1) // self.bsize
        buf = b''.join(self.block(b) if self.valid(b) else bytes(self.bsize)
                       for b in blocks[:need])
        return buf[:size]

    def plausible(self):
        """A real filesystem, not a stray magic number: its root directory
        (inode 2) must be a directory whose '.' and '..' both point to 2."""
        if not self.bsize or not (0 < self.isize < self.fsize):
            return False
        if self.base * self.sector + self.fsize * self.bsize > len(self.raw) + self.bsize:
            return False
        mode, size, addr, _ = self.inode(2)
        if mode & 0o170000 != 0o040000 or size < 32:
            return False
        d = self.data(2)
        return (struct.unpack('>H', d[0:2])[0] == 2 and d[2:4] == b'.\0' and
                struct.unpack('>H', d[16:18])[0] == 2 and d[18:21] == b'..\0')


def find_filesystems(raw):
    """Return an Image for every plausible filesystem, in disk order."""
    mag = struct.pack('>I', FS_MAGIC)
    found = []
    i = raw.find(mag)
    while i >= 0:
        for sector, tag in ((532, 20), (512, 0)):
            if (i - tag) % sector == 504:
                base = (i - tag) // sector - 1
                if base >= 0:
                    img = Image(raw, sector, tag, base)
                    if img.plausible():
                        found.append(img)
        i = raw.find(mag, i + 1)
    return found


def unclobbered(path):
    """Avoid overwriting on case-insensitive hosts (macOS, Windows), where
    e.g. /usr/lib/Pascal and /usr/lib/pascal are the same host file."""
    if not os.path.exists(path):
        return path
    n = 2
    while os.path.exists('%s~%d' % (path, n)):
        n += 1
    new = '%s~%d' % (path, n)
    print('warning: %s already exists (case-insensitive host?); writing %s'
          % (path, new), file=sys.stderr)
    return new


def extract(img, outdir):
    counts = {'files': 0, 'dirs': 0, 'special': 0}
    done_dirs = set()
    max_ino = img.isize * (img.bsize // 64)

    def walk(ino, path):
        mode, size, addr, mtime = img.inode(ino)
        fmt = mode & 0o170000
        if fmt == 0o040000:
            if ino in done_dirs:
                return
            done_dirs.add(ino)
            os.makedirs(path, exist_ok=True)
            counts['dirs'] += 1
            d = img.data(ino)
            for i in range(0, len(d) - 15, 16):
                e = struct.unpack('>H', d[i:i + 2])[0]
                name = d[i + 2:i + 16].split(b'\0')[0].decode('latin1')
                if e and name not in ('.', '..') and e <= max_ino:
                    walk(e, os.path.join(path, name.replace('/', '_')))
        elif fmt == 0o100000:
            path = unclobbered(path)
            with open(path, 'wb') as f:
                f.write(img.data(ino))
            os.utime(path, (mtime, mtime))
            counts['files'] += 1
        else:
            path = unclobbered(path + '.special')
            with open(path, 'w') as f:
                f.write('mode %o addr0 %x\n' % (mode, addr[0]))
            counts['special'] += 1

    walk(2, outdir)
    return counts


def main():
    ap = argparse.ArgumentParser(description='Extract a UniPlus+ System V disk image.')
    ap.add_argument('image')
    ap.add_argument('outdir', nargs='?')
    ap.add_argument('--list', action='store_true', help='only list filesystems found')
    ap.add_argument('--base', type=int, help='sector where the filesystem starts')
    args = ap.parse_args()

    with open(args.image, 'rb') as f:
        raw = f.read()

    found = find_filesystems(raw)
    if not found:
        sys.exit('no UniPlus System V filesystem found in %s' % args.image)

    if args.list:
        for img in found:
            print('sector %d (%d-byte sectors): %d blocks of %d bytes (%.1f MB), %d inode blocks'
                  % (img.base, img.sector, img.fsize, img.bsize,
                     img.fsize * img.bsize / 1e6, img.isize))
        return

    if not args.outdir:
        ap.error('OUTDIR is required unless --list is given')

    if args.base is None:
        img = found[0]
    else:
        match = [i for i in found if i.base == args.base]
        if not match:
            sys.exit('no filesystem at sector %d; try --list' % args.base)
        img = match[0]

    if os.path.isdir(args.outdir) and os.listdir(args.outdir):
        sys.exit('%s exists and is not empty; choose a new directory' % args.outdir)

    c = extract(img, args.outdir)
    print('extracted filesystem at sector %d into %s: %d files, %d directories, %d special'
          % (img.base, args.outdir, c['files'], c['dirs'], c['special']))


if __name__ == '__main__':
    main()
