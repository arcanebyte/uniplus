#!/usr/bin/env python3
"""
extract_profile_image.py -- copy files out of a UniPlus+ (Apple Lisa) ProFile disk image.

Read-only: the image is never modified.  Needs only Python 3, no extra modules.

USAGE
    python3 tools/extract_profile_image.py IMAGE OUTDIR
    python3 tools/extract_profile_image.py --list IMAGE

    IMAGE    raw ProFile image, e.g. uniplus_unix_10mb_ai_copy.image
    OUTDIR   new or empty directory to extract into (created if missing;
             a non-empty directory is refused so nothing gets overwritten)

    --list   only report the System V filesystems found and exit
    --base N extract the filesystem starting at ProFile sector N instead of
             the first one found (use the numbers printed by --list)

EXAMPLES
    # see what is on the disk
    python3 tools/extract_profile_image.py --list uniplus_unix_10mb_ai_copy.image

    # dump the root filesystem into ./dump
    python3 tools/extract_profile_image.py uniplus_unix_10mb_ai_copy.image dump

WHAT YOU GET
    - Regular files with their contents and modification times.
    - Hard links: every name is written as a separate copy (e.g. /bin/cp and
      /bin/mv are identical files, not links).
    - Device nodes, pipes: a text file NAME.special holding the octal mode and
      device number in hex, e.g. "mode 20666 addr0 401" = char device major 4
      minor 1.
    - Names differing only in case (the 10 MB disk has /usr/lib/Pascal and
      /usr/lib/pascal) collide on macOS/Windows; the later one is written as
      NAME~2 and a warning is printed.
    - Not preserved: owner, group, permission bits.

IMAGE FORMAT ASSUMED
    - 532-byte sectors: 20-byte ProFile tag followed by 512 data bytes.  A
      plain 512-byte-per-sector image is also detected.
    - UniPlus+ System V filesystem: 512-byte blocks (superblock s_type 1),
      superblock magic 0xfd187e20, big-endian, 64-byte inodes, 14-char names.
    - On a 10 MB disk the root filesystem starts at sector 2501 (after 101 boot
      sectors and 2400 swap sectors; see prlmap[] in v1.5/sys/pro.c).
"""
import argparse
import os
import struct
import sys

FS_MAGIC = 0xfd187e20


class Image:
    def __init__(self, raw, sector, tag, base):
        self.raw, self.sector, self.tag, self.base = raw, sector, tag, base
        sb = self.block(1)
        self.isize = struct.unpack('>H', sb[0:2])[0]
        self.fsize = struct.unpack('>I', sb[2:6])[0]
        self.fname = sb[0x1ec:0x1f2].split(b'\0')[0].decode('latin1')

    def block(self, n):
        o = (self.base + n) * self.sector + self.tag
        return self.raw[o:o + 512]

    def inode(self, ino):
        b, off = divmod(ino - 1, 8)
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
        for p in struct.unpack('>128I', self.block(bn)):
            if level == 1:
                out.append(p)
            else:
                self.indirect(p, level - 1, out)

    def data(self, ino):
        mode, size, addr, _ = self.inode(ino)
        blocks = list(addr[:10])
        for level, a in zip((1, 2, 3), addr[10:]):
            self.indirect(a, level, blocks)
        need = (size + 511) // 512
        buf = b''.join(self.block(b) if self.valid(b) else bytes(512)
                       for b in blocks[:need])
        return buf[:size]


def find_filesystems(raw):
    """Yield (sector_size, tag_size, base_sector) for every superblock found."""
    mag = struct.pack('>I', FS_MAGIC)
    i = raw.find(mag)
    while i >= 0:
        for sector, tag in ((532, 20), (512, 0)):
            if (i - tag) % sector == 504:
                base = (i - tag) // sector - 1
                if base >= 0:
                    yield sector, tag, base
        i = raw.find(mag, i + 1)


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
                if e and name not in ('.', '..'):
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
    ap = argparse.ArgumentParser(description='Extract a UniPlus+ Lisa ProFile disk image.')
    ap.add_argument('image')
    ap.add_argument('outdir', nargs='?')
    ap.add_argument('--list', action='store_true', help='only list filesystems found')
    ap.add_argument('--base', type=int, help='ProFile sector where the filesystem starts')
    args = ap.parse_args()

    with open(args.image, 'rb') as f:
        raw = f.read()

    found = []
    for sector, tag, base in find_filesystems(raw):
        img = Image(raw, sector, tag, base)
        if img.block(1)[511] == 1 and 0 < img.isize < img.fsize:
            found.append(img)

    if not found:
        sys.exit('no UniPlus System V (512-byte block) filesystem found in %s' % args.image)

    if args.list:
        for img in found:
            print('sector %d (%d-byte sectors): %d blocks, %d inode blocks, name "%s"'
                  % (img.base, img.sector, img.fsize, img.isize, img.fname))
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
