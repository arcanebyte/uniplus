#!/usr/bin/env python3
"""
put_profile_files.py -- add or replace files in an existing UniPlus+ System V
filesystem inside a raw Apple Lisa ProFile image, in place.

Use it to get source files onto a disk you already use (for example the
sources partition of the 20 MB build disk) without rebuilding the filesystem.
LisaEm must be shut down, or at least have the image closed. Keep a backup.

USAGE
    python3 tools/put_profile_files.py IMAGE [--base SECTOR] DEST=SOURCE [...]

    IMAGE          raw ProFile image (532-byte sectors), modified in place
    --base SECTOR  sector where the filesystem starts; default: the only
                   filesystem on the disk (see extract_profile_image.py --list)
    DEST=SOURCE    copy host file SOURCE to /DEST in that filesystem. The
                   directory must already exist, unless --mkdir is given. An
                   existing regular file of that name is replaced (its
                   blocks are freed first).
    --mkdir        create missing directories on the way to each DEST
                   (mode 0755, owned by root)

    20 MB build disk, sources partition (mounted at /usr/src on the Lisa):
        python3 tools/put_profile_files.py uniplus_unix_20mb.build2.image \\
            --base 19456 netlib/looptest.c=netlib/looptest.c

Blocks and inodes are allocated the way the UniPlus kernel's alloc()/free()
and ialloc() do: from the superblock free-block cache and its chain, and the
free-inode cache (refilled by scanning the inode list). Files are owned by
root (0/0), mode 0644 (0755 if executable on the host), modification time
from the host. Files up to 10 + 128 blocks (70 KB) are supported.

The filesystem is checked before writing (the tool refuses to touch an
inconsistent one) and again afterwards, and every file written is read back
and compared. Still run  fsck  on the Lisa before mounting it read-write.
"""
import argparse
import os
import struct
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from extract_profile_image import find_filesystems, Image  # noqa: E402

SECTOR, TAG, DATA = 532, 20, 512
NICFREE, NICINOD = 50, 100
S_IFMT, S_IFDIR, S_IFREG = 0o170000, 0o040000, 0o100000
PER = DATA // 4


class FS:
    def __init__(self, raw, base):
        self.raw, self.base = raw, base
        sb = self.block(1)
        self.isize = struct.unpack('>H', sb[0:2])[0]
        self.fsize = struct.unpack('>I', sb[2:6])[0]
        self.nfree = struct.unpack('>h', sb[6:8])[0]
        self.free = list(struct.unpack('>50I', sb[8:208]))
        self.ninode = struct.unpack('>h', sb[208:210])[0]
        self.inodes = list(struct.unpack('>100H', sb[210:410]))
        self.tfree = struct.unpack('>I', sb[426:430])[0]
        self.tinode = struct.unpack('>H', sb[430:432])[0]
        if struct.unpack('>I', sb[508:512])[0] != 1:
            sys.exit('only 512-byte-block (s_type 1) filesystems are supported')
        self.ninodes = (self.isize - 2) * 8

    # ---- raw block and inode access ----
    def off(self, bn):
        return (self.base + bn) * SECTOR + TAG

    def block(self, bn):
        o = self.off(bn)
        return bytes(self.raw[o:o + DATA])

    def put_block(self, bn, data):
        assert len(data) == DATA and 2 <= bn < self.fsize or bn == 1
        o = self.off(bn)
        self.raw[o:o + DATA] = data

    def inode(self, ino):
        b, i = divmod(ino - 1, 8)
        d = self.block(2 + b)[i * 64:(i + 1) * 64]
        mode, nlink, uid, gid, size = struct.unpack('>HhHHI', d[:12])
        addr = [int.from_bytes(d[12 + 3 * k:15 + 3 * k], 'big') for k in range(13)]
        return mode, nlink, size, addr

    def put_inode(self, ino, mode, nlink, size, addr, mtime):
        b, i = divmod(ino - 1, 8)
        raw = b''.join(a.to_bytes(3, 'big') for a in addr).ljust(40, b'\0')
        d = struct.pack('>HhHHI', mode, nlink, 0, 0, size) + raw + struct.pack('>III', mtime, mtime, mtime)
        blk = bytearray(self.block(2 + b))
        blk[i * 64:(i + 1) * 64] = d
        self.put_block(2 + b, bytes(blk))

    def write_super(self):
        sb = bytearray(self.block(1))
        sb[6:8] = struct.pack('>h', self.nfree)
        sb[8:208] = struct.pack('>50I', *self.free)
        sb[208:210] = struct.pack('>h', self.ninode)
        sb[210:410] = struct.pack('>100H', *self.inodes)
        sb[414:418] = struct.pack('>I', int(time.time()))
        sb[426:430] = struct.pack('>I', self.tfree)
        sb[430:432] = struct.pack('>H', self.tinode)
        self.put_block(1, bytes(sb))

    # ---- allocation, as in the kernel ----
    def alloc(self):
        if self.nfree <= 0:
            sys.exit('filesystem full')
        self.nfree -= 1
        bn = self.free[self.nfree]
        if bn == 0:
            sys.exit('filesystem full')
        if not (self.isize <= bn < self.fsize):
            sys.exit('bad block %d in free list' % bn)
        if self.nfree <= 0:              # bn is a chain block: load the next batch
            d = self.block(bn)
            self.nfree = struct.unpack('>i', d[0:4])[0]
            self.free = list(struct.unpack('>50I', d[4:204]))
        self.tfree -= 1
        return bn

    def bfree(self, bn):
        if self.nfree <= 0:
            self.nfree = 1
            self.free[0] = 0
        if self.nfree >= NICFREE:        # cache full: it becomes the chain block
            d = struct.pack('>i', self.nfree) + struct.pack('>50I', *self.free)
            self.put_block(bn, d.ljust(DATA, b'\0'))
            self.nfree = 0
            self.free = [0] * NICFREE
        self.free[self.nfree] = bn
        self.nfree += 1
        self.tfree += 1

    def ialloc(self):
        while True:
            if self.ninode <= 0:
                found = [i for i in range(1, self.ninodes + 1) if self.inode(i)[0] == 0 and i > 1]
                if not found:
                    sys.exit('out of inodes')
                found = found[:NICINOD]
                self.inodes = found + [0] * (NICINOD - len(found))
                self.ninode = len(found)
            self.ninode -= 1
            ino = self.inodes[self.ninode]
            if ino > 1 and self.inode(ino)[0] == 0:
                self.tinode -= 1
                return ino

    # ---- files ----
    def file_blocks(self, addr, size):
        """Data blocks and indirect blocks of a file (direct and single indirect only)."""
        need = (size + DATA - 1) // DATA
        data = [a for a in addr[:10]][:need]
        meta = []
        if need > 10:
            if not addr[10]:
                sys.exit('file has a hole; not supported')
            meta.append(addr[10])
            ptrs = struct.unpack('>%dI' % PER, self.block(addr[10]))
            data += list(ptrs[:need - 10])
        if addr[11] or addr[12]:
            sys.exit('existing file uses double/triple indirect blocks; not supported')
        return data, meta

    def write_file(self, content):
        nblk = (len(content) + DATA - 1) // DATA
        if nblk > 10 + PER:
            sys.exit('file too large for this tool (%d blocks)' % nblk)
        blocks = []
        for k in range(nblk):
            bn = self.alloc()
            self.put_block(bn, content[k * DATA:(k + 1) * DATA].ljust(DATA, b'\0'))
            blocks.append(bn)
        addr = blocks[:10] + [0, 0, 0]
        if nblk > 10:
            ind = self.alloc()
            rest = blocks[10:]
            self.put_block(ind, struct.pack('>%dI' % len(rest), *rest).ljust(DATA, b'\0'))
            addr[10] = ind
        return addr

    def dir_entries(self, dino):
        mode, nlink, size, addr = self.inode(dino)
        if mode & S_IFMT != S_IFDIR:
            sys.exit('inode %d is not a directory' % dino)
        data_blocks, _ = self.file_blocks(addr, size)
        buf = b''.join(self.block(b) for b in data_blocks)[:size]
        return [(i // 16, struct.unpack('>H', buf[i:i + 2])[0], buf[i + 2:i + 16].split(b'\0')[0].decode('latin1'))
                for i in range(0, size, 16)], data_blocks, size, addr

    def lookup(self, path):
        ino = 2
        for part in [p for p in path.split('/') if p]:
            ents = self.dir_entries(ino)[0]
            match = [e for (_, e, n) in ents if e and n == part]
            if not match:
                return None
            ino = match[0]
        return ino

    def add_entry(self, dino, name, ino):
        ents, data_blocks, size, addr = self.dir_entries(dino)
        slot = next((k for (k, e, _) in ents if e == 0), None)
        rec = struct.pack('>H', ino) + name.encode('latin1').ljust(14, b'\0')
        if slot is None:
            slot = size // 16
            size += 16
            if slot // 32 >= len(data_blocks):
                if len(data_blocks) >= 10:
                    sys.exit('directory too large for this tool')
                bn = self.alloc()
                self.put_block(bn, bytes(DATA))
                data_blocks.append(bn)
                addr[len(data_blocks) - 1] = bn
            mode, nlink, _, _ = self.inode(dino)
            self.put_inode(dino, mode, nlink, size, addr, int(time.time()))
        bn = data_blocks[slot // 32]
        blk = bytearray(self.block(bn))
        o = (slot % 32) * 16
        blk[o:o + 16] = rec
        self.put_block(bn, bytes(blk))

    def mkdir(self, path, mtime):
        """Create directory path (its parent must exist); returns its inode."""
        parts = [p for p in path.split('/') if p]
        name = parts[-1]
        if len(name.encode('latin1')) > 14:
            sys.exit('name longer than 14 characters: %s' % name)
        pino = self.lookup('/'.join(parts[:-1]))
        if pino is None:
            sys.exit('directory does not exist: /%s' % '/'.join(parts[:-1]))
        ino = self.ialloc()
        bn = self.alloc()
        ents = struct.pack('>H', ino) + b'.'.ljust(14, b'\0') + struct.pack('>H', pino) + b'..'.ljust(14, b'\0')
        self.put_block(bn, ents.ljust(DATA, b'\0'))
        self.put_inode(ino, S_IFDIR | 0o755, 2, len(ents), [bn] + [0] * 12, mtime)
        self.add_entry(pino, name, ino)
        pmode, pnlink, psize, paddr = self.inode(pino)   # the new ".." links to the parent
        self.put_inode(pino, pmode, pnlink + 1, psize, paddr, mtime)
        return ino

    def put(self, dest, content, mode, mtime, mkdirs=False):
        parts = [p for p in dest.split('/') if p]
        name = parts[-1]
        if len(name.encode('latin1')) > 14:
            sys.exit('name longer than 14 characters: %s' % name)
        if mkdirs:
            for k in range(1, len(parts)):
                sub = '/'.join(parts[:k])
                ino = self.lookup(sub)
                if ino is None:
                    self.mkdir(sub, mtime)
                elif self.inode(ino)[0] & S_IFMT != S_IFDIR:
                    sys.exit('/%s exists and is not a directory' % sub)
        dino = self.lookup('/'.join(parts[:-1]))
        if dino is None:
            sys.exit('directory does not exist: /%s' % '/'.join(parts[:-1]))
        ino = self.lookup(dest)
        if ino is not None:
            omode, nlink, size, addr = self.inode(ino)
            if omode & S_IFMT != S_IFREG:
                sys.exit('/%s exists and is not a regular file' % dest)
            data_blocks, meta = self.file_blocks(addr, size)
            for bn in data_blocks + meta:
                self.bfree(bn)
            newaddr = self.write_file(content)
            self.put_inode(ino, S_IFREG | mode, nlink, len(content), newaddr, mtime)
            return 'replaced', ino
        ino = self.ialloc()
        newaddr = self.write_file(content)
        self.put_inode(ino, S_IFREG | mode, 1, len(content), newaddr, mtime)
        self.add_entry(dino, name, ino)
        return 'added', ino


def check(raw, base):
    """Every block owned once or free, free list sane, counts match. Returns list of problems."""
    fs = FS(raw, base)
    problems, owner = [], {}
    for ino in range(1, fs.ninodes + 1):
        mode, nlink, size, addr = fs.inode(ino)
        if mode == 0 or ino == 1 or mode & S_IFMT not in (S_IFREG, S_IFDIR, 0o010000):
            continue
        need = (size + DATA - 1) // DATA
        blocks = [a for a in addr[:10][:need] if a]
        for level, a in zip((1, 2, 3), addr[10:]):
            if not a:
                continue
            stack = [(a, level)]
            while stack:
                bn, lv = stack.pop()
                if not (fs.isize <= bn < fs.fsize):
                    problems.append('inode %d: bad block %d' % (ino, bn))
                    continue
                blocks.append(bn)
                for p in struct.unpack('>%dI' % PER, fs.block(bn)):
                    if p:
                        if lv == 1:
                            blocks.append(p)
                        else:
                            stack.append((p, lv - 1))
        for bn in blocks:
            if not (fs.isize <= bn < fs.fsize):
                problems.append('inode %d: bad block %d' % (ino, bn))
            elif bn in owner:
                problems.append('block %d in inodes %d and %d' % (bn, owner[bn], ino))
            else:
                owner[bn] = ino
    free, nfree, flist = set(), fs.nfree, fs.free
    while nfree > 0:
        for bn in flist[:nfree][1:]:
            free.add(bn)
        head = flist[0]
        if head == 0:
            break
        if head in free or not (fs.isize <= head < fs.fsize):
            problems.append('free chain loops or leaves the filesystem at %d' % head)
            break
        free.add(head)
        d = fs.block(head)
        nfree = struct.unpack('>i', d[0:4])[0]
        flist = list(struct.unpack('>50I', d[4:204]))
        if not 0 <= nfree <= NICFREE:
            problems.append('bad free chain count %d in block %d' % (nfree, head))
            break
    both = free & set(owner)
    if both:
        problems.append('%d blocks both free and in use, e.g. %d' % (len(both), min(both)))
    total = fs.fsize - fs.isize
    if len(owner) + len(free) != total:
        problems.append('%d blocks in use + %d free != %d' % (len(owner), len(free), total))
    if len(free) != fs.tfree:
        problems.append('superblock s_tfree %d, free list has %d' % (fs.tfree, len(free)))
    return problems


def main():
    ap = argparse.ArgumentParser(description='Add or replace files in a UniPlus+ ProFile image filesystem.')
    ap.add_argument('image')
    ap.add_argument('mappings', nargs='+', help='DEST=SOURCE')
    ap.add_argument('--base', type=int, help='sector where the filesystem starts')
    ap.add_argument('--mkdir', action='store_true', help='create missing directories on the way to each DEST')
    args = ap.parse_args()

    raw = bytearray(open(args.image, 'rb').read())
    if args.base is None:
        found = find_filesystems(bytes(raw))
        if len(found) != 1:
            sys.exit('%d filesystems on this disk; choose one with --base (%s)'
                     % (len(found), ', '.join(str(f.base) for f in found)))
        args.base = found[0].base

    problems = check(raw, args.base)
    if problems:
        sys.exit('filesystem at sector %d is not consistent, not writing:\n  %s'
                 % (args.base, '\n  '.join(problems[:10])))

    fs = FS(raw, args.base)
    results = []
    for m in args.mappings:
        if '=' not in m:
            sys.exit('expected DEST=SOURCE, got %s' % m)
        dest, src = m.split('=', 1)
        st = os.stat(src)
        content = open(src, 'rb').read()
        mode = 0o755 if st.st_mode & 0o111 else 0o644
        what, ino = fs.put(dest, content, mode, int(st.st_mtime), args.mkdir)
        results.append((dest, src, content, what, ino))
    fs.write_super()

    problems = check(raw, args.base)
    if problems:
        sys.exit('filesystem check failed after the changes, image NOT written:\n  %s'
                 % '\n  '.join(problems[:10]))
    img = Image(bytes(raw), SECTOR, TAG, args.base)
    for dest, src, content, what, ino in results:
        if img.data(ino) != content:
            sys.exit('read-back of /%s differs, image NOT written' % dest)

    with open(args.image, 'r+b') as f:
        f.write(raw)
    for dest, src, content, what, ino in results:
        print('%s /%s (%d bytes, inode %d) from %s' % (what, dest, len(content), ino, src))
    print('filesystem at sector %d checked: consistent, %d blocks free, %d inodes free'
          % (args.base, fs.tfree, fs.tinode))


if __name__ == '__main__':
    main()
