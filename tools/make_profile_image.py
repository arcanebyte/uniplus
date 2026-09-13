#!/usr/bin/env python3
"""
make_profile_image.py -- build a raw Apple Lisa ProFile image containing a new
UniPlus+ System V filesystem populated from host directories and files.

Intended for a *data* disk attached as a second ProFile (e.g. on a dual
parallel card in LisaEm) and mounted under UniPlus; it has no boot blocks,
kernel or swap.  Needs only Python 3, no extra modules.

USAGE
    python3 tools/make_profile_image.py OUTPUT.image DEST=SOURCE [DEST=SOURCE ...]

    OUTPUT.image   new file to create (refuses to overwrite an existing file)
    DEST=SOURCE    copy host file or directory SOURCE to /DEST on the new
                   filesystem; directories are copied recursively

    --fname NAME   filesystem name stored in the superblock (<= 6 chars)
    --fpack NAME   pack name stored in the superblock (<= 6 chars)

    Enlarging an existing system disk instead of making a separate one:
    --base-image FILE  copy FILE (e.g. a 10 MB UniPlus system disk) into the
                       start of the new image; it is only read
    --disk-sectors N   total size of the new image (19456 = 10 MB, 38912 = 20 MB)
    --start N          sector where the new filesystem starts (default 101)
    --blocks N         size of the new filesystem in blocks (default 19355)
    --replace-partition  discard what the base image holds from --start on
                       (rebuild the source partition of an existing 20 MB
                       disk; omit --prlmap, the table is already patched)
    --prlmap LETTER    set that entry of the kernel partition table found on
                       the base image (prlmap[] in v1.5/sys/pro.c) to
                       {--start, --blocks}; the stock table must occur once

    20 MB system disk with the sources in partition e:
        python3 tools/make_profile_image.py uniplus_unix_20mb.image \\
            --base-image uniplus_unix_10mb.image --disk-sectors 38912 \\
            --start 19456 --blocks 19456 --prlmap e \\
            sys=v1.5/sys include=v1.5/include netlib=netlib README=lisa-build.md
        on UniPlus:  mknod /dev/p0e b 0 4 ; mknod /dev/rp0e c 5 4
                     mkdir /usr/src ; mount /dev/p0e /usr/src

EXAMPLE
    python3 tools/make_profile_image.py uniplus_src_10mb.image \\
        sys=v1.5/sys include=v1.5/include netlib=netlib README=lisa-build.md

    Then in LisaEm put the image on a dual parallel card ProFile (LisaEm
    slot 1 high = /dev/p2h, slot 1 low = /dev/p1h) and on UniPlus:
        mkdir /usr/src ; mount /dev/p2h /usr/src

LAYOUT (matches the UniPlus+ 10 MB ProFile conventions, see v1.5/sys/pro.c)
    - 19456 sectors of 532 bytes (20-byte tag, all zero, + 512 data bytes)
    - filesystem in partition h: sector 101, 19355 blocks of 512 bytes,
      so it is used through the whole-disk partition /dev/pNh
    - System V (s_type 1): 64-byte inodes, 14-character names, 13 block
      addresses (10 direct, single, double, triple indirect), free-block
      chain as written by mkfs; inode 1 reserved for bad blocks, inode 2
      root, a pre-sized lost+found for fsck
    - all files owned by root (uid 0, gid 0); directories 0755, files 0644
      (0755 if executable on the host); modification times from the host

After writing, the image is read back and verified: every block is used once
or free, counts match, and every file's contents read back identically.
Still run  fsck /dev/p2h  (or p1h, per the port used) on the Lisa before first use.
"""
import argparse
import os
import stat
import struct
import sys
import time

SECTOR, TAG, DATA = 532, 20, 512
DISK_SECTORS = 19456            # 10 MB ProFile
PART_START, FSIZE = 101, 19355  # prlmap[] partition h
ISIZE = 606                     # blocks 0..605: boot, super, 604 inode blocks
NINODES = (ISIZE - 2) * 8
NICFREE, NICINOD = 50, 100
FS_MAGIC, FS1B = 0xfd187e20, 1
LOSTFOUND_BLOCKS = 10
SKIP = {'.DS_Store'}
BASE_IMAGE = None               # set by --base-image

S_IFDIR, S_IFREG = 0o040000, 0o100000


class Node:
    def __init__(self, name, is_dir, data=b'', mtime=0, mode=0o644):
        self.name, self.is_dir, self.data, self.mtime, self.mode = name, is_dir, data, mtime, mode
        self.children = []
        self.ino = 0


def load(src, name):
    st = os.stat(src)
    if len(name.encode('latin1')) > 14:
        sys.exit('name longer than 14 characters: %s (%s)' % (name, src))
    if stat.S_ISDIR(st.st_mode):
        n = Node(name, True, mtime=int(st.st_mtime), mode=0o755)
        for entry in sorted(os.listdir(src)):
            if entry in SKIP:
                continue
            n.children.append(load(os.path.join(src, entry), entry))
        return n
    mode = 0o755 if st.st_mode & 0o111 else 0o644
    with open(src, 'rb') as f:
        return Node(name, False, f.read(), int(st.st_mtime), mode)


class FS:
    def __init__(self):
        self.blocks = {}            # block number -> 512 bytes
        self.inodes = {}            # ino -> 64 bytes
        self.next_block = ISIZE
        self.next_ino = 2

    def alloc_block(self, data=b''):
        b = self.next_block
        if b >= FSIZE:
            sys.exit('filesystem full')
        self.next_block += 1
        self.blocks[b] = data.ljust(DATA, b'\0')
        return b

    def write_data(self, data):
        """Store data, return the 13 inode block addresses."""
        nblk = (len(data) + DATA - 1) // DATA
        blocks = [self.alloc_block(data[i * DATA:(i + 1) * DATA]) for i in range(nblk)]
        addr = blocks[:10] + [0, 0, 0]
        rest = blocks[10:]
        per = DATA // 4
        if rest:
            chunk, rest = rest[:per], rest[per:]
            addr[10] = self.alloc_block(struct.pack('>%dI' % len(chunk), *chunk))
        if rest:
            singles = []
            while rest:
                chunk, rest = rest[:per], rest[per:]
                singles.append(self.alloc_block(struct.pack('>%dI' % len(chunk), *chunk)))
            if len(singles) > per:
                sys.exit('file too large for this tool (needs triple indirect)')
            addr[11] = self.alloc_block(struct.pack('>%dI' % len(singles), *singles))
        return addr

    def put_inode(self, ino, mode, nlink, size, addr, mtime):
        raw = b''.join(a.to_bytes(3, 'big') for a in addr).ljust(40, b'\0')
        self.inodes[ino] = struct.pack('>HhHHI', mode, nlink, 0, 0, size) + raw + \
            struct.pack('>III', mtime, mtime, mtime)


def build(root, now):
    fs = FS()
    # inode 1: bad block list, as mkfs leaves it
    fs.put_inode(1, S_IFREG, 0, 0, [0] * 13, now)

    def number(node):
        node.ino = fs.next_ino
        fs.next_ino += 1
        if fs.next_ino > NINODES + 1:
            sys.exit('out of inodes')
        for c in node.children:
            number(c)
    number(root)
    lost = Node('lost+found', True, mtime=now, mode=0o777)
    lost.ino = fs.next_ino
    fs.next_ino += 1
    root.children.append(lost)

    def write(node, parent_ino):
        if node.is_dir:
            ents = [(node.ino, '.'), (parent_ino, '..')] + [(c.ino, c.name) for c in node.children]
            data = b''.join(struct.pack('>H', i) + n.encode('latin1').ljust(14, b'\0') for i, n in ents)
            if node is lost:
                data = data.ljust(LOSTFOUND_BLOCKS * DATA, b'\0')
            addr = fs.write_data(data)
            nlink = 2 + sum(1 for c in node.children if c.is_dir)
            fs.put_inode(node.ino, S_IFDIR | node.mode, nlink, len(data), addr, node.mtime)
            for c in node.children:
                write(c, node.ino)
        else:
            addr = fs.write_data(node.data)
            fs.put_inode(node.ino, S_IFREG | node.mode, 1, len(node.data), addr, node.mtime)
    write(root, root.ino)
    return fs


def image_bytes(fs, now, fname, fpack):
    used_inodes = set(fs.inodes)
    free_inodes = [i for i in range(1, NINODES + 1) if i not in used_inodes]

    # Free-block chain, built the way the kernel's free() would: freeing in
    # descending order so allocation starts from low block numbers.
    s_free = [0]
    chain = {}
    for b in range(FSIZE - 1, fs.next_block - 1, -1):
        if len(s_free) >= NICFREE:
            chain[b] = struct.pack('>i', len(s_free)) + struct.pack('>%dI' % len(s_free), *s_free)
            s_free = []
        s_free.append(b)
    for b, data in chain.items():
        fs.blocks[b] = data.ljust(DATA, b'\0')
    tfree = FSIZE - fs.next_block

    ilist = free_inodes[:NICINOD]
    sb = struct.pack('>HIh', ISIZE, FSIZE, len(s_free))
    sb += struct.pack('>50I', *(s_free + [0] * (NICFREE - len(s_free))))
    sb += struct.pack('>h', len(ilist)) + struct.pack('>100H', *(ilist + [0] * (NICINOD - len(ilist))))
    sb += bytes(4)                                   # flock, ilock, fmod, ronly
    sb += struct.pack('>I', now)
    sb += struct.pack('>4h', 7, 400, 0, 0)           # s_dinfo: gap, blocks/cyl (as the Lisa mkfs)
    sb += struct.pack('>IH', tfree, len(free_inodes))
    sb += fname.encode('latin1')[:6].ljust(6, b'\0') + fpack.encode('latin1')[:6].ljust(6, b'\0')
    sb += bytes(56)                                  # s_fill[14]
    sb += struct.pack('>HHII', 0, 0, FS_MAGIC, FS1B)  # s_lasti, s_nbehind, magic, type
    assert len(sb) == 512

    img = bytearray(DISK_SECTORS * SECTOR)
    if BASE_IMAGE is not None:
        if len(BASE_IMAGE) > len(img):
            sys.exit('base image is larger than the new disk')
        if BASE_IMAGE[PART_START * SECTOR:].strip(b'\0'):
            sys.exit('base image has data at or beyond sector %d; choose a partition past it' % PART_START)
        img[:len(BASE_IMAGE)] = BASE_IMAGE

    def put(block, data):
        o = (PART_START + block) * SECTOR + TAG
        img[o:o + DATA] = data

    put(1, sb)
    for ino, raw in fs.inodes.items():
        b, off = divmod(ino - 1, 8)
        blk = bytearray(img[(PART_START + 2 + b) * SECTOR + TAG:(PART_START + 2 + b) * SECTOR + TAG + DATA])
        blk[off * 64:off * 64 + 64] = raw
        put(2 + b, bytes(blk))
    for b, data in fs.blocks.items():
        put(b, data)
    return bytes(img)


def verify(path, root):
    here = os.path.dirname(os.path.abspath(__file__))
    sys.path.insert(0, here)
    import extract_profile_image as ex
    raw = open(path, 'rb').read()
    found = [f for f in ex.find_filesystems(raw) if f.base == PART_START]
    assert found, 'filesystem not found by the extractor'
    img = found[0]
    owner = {}

    def claim(b, who):
        assert ISIZE <= b < FSIZE, 'block %d out of range (%s)' % (b, who)
        assert b not in owner, 'block %d used twice (%s, %s)' % (b, owner.get(b), who)
        owner[b] = who

    def blocks_of(ino):
        mode, size, addr, _ = img.inode(ino)
        n = (size + DATA - 1) // DATA
        for b in addr[:10]:
            if b:
                claim(b, ino)
        if addr[10]:
            claim(addr[10], ino)
            for p in struct.unpack('>128I', img.block(addr[10])):
                if p:
                    claim(p, ino)
        if addr[11]:
            claim(addr[11], ino)
            for s in struct.unpack('>128I', img.block(addr[11])):
                if s:
                    claim(s, ino)
                    for p in struct.unpack('>128I', img.block(s)):
                        if p:
                            claim(p, ino)

    def walk(node, ino):
        blocks_of(ino)
        if node.is_dir:
            d = img.data(ino)
            ents = {d[i + 2:i + 16].split(b'\0')[0].decode('latin1'): struct.unpack('>H', d[i:i + 2])[0]
                    for i in range(0, len(d), 16) if struct.unpack('>H', d[i:i + 2])[0]}
            for c in node.children:
                assert c.name in ents, 'missing entry %s' % c.name
                walk(c, ents[c.name])
        else:
            assert img.data(ino) == node.data, 'content mismatch for %s' % node.name
    walk(root, 2)

    # follow the free chain
    sb = img.sectordata(PART_START + 1)
    nfree = struct.unpack('>h', sb[6:8])[0]
    free = list(struct.unpack('>50I', sb[8:208]))[:nfree]
    tfree = struct.unpack('>I', sb[426:430])[0]
    count = 0
    while True:
        if not free:
            break
        head = free[0]
        for b in free[1:]:
            claim(b, 'free')
            count += 1
        if head == 0:
            break
        claim(head, 'free')
        count += 1
        blk = img.block(head)
        n = struct.unpack('>i', blk[0:4])[0]
        free = list(struct.unpack('>50I', blk[4:204]))[:n]
    assert count == tfree, 'free count %d != s_tfree %d' % (count, tfree)
    assert len(owner) == FSIZE - ISIZE, 'unaccounted blocks: %d' % (FSIZE - ISIZE - len(owner))
    return count


PRLMAP_10MB = [(2501, 16955), (101, 2400), (2501, 7227), (9728, 9728),
               (0, 0), (0, 7168), (7168, 2496), (101, 19355)]


def patch_prlmap(img, letter, start, length):
    """Change one entry of the UniPlus ProFile partition table (prlmap[] in
    v1.5/sys/pro.c) inside the kernel stored on the image.  The stock 10 MB
    table must occur exactly once; returns the byte offset patched."""
    table = b''.join(struct.pack('>II', a, b) for a, b in PRLMAP_10MB)
    at = img.find(table)
    if at < 0 or img.find(table, at + 1) >= 0:
        sys.exit('stock prlmap[] table not found exactly once in the base image')
    idx = 'abcdefgh'.index(letter)
    off = at + 8 * idx
    img[off:off + 8] = struct.pack('>II', start, length)
    return off


def main():
    global DISK_SECTORS, PART_START, FSIZE, BASE_IMAGE
    ap = argparse.ArgumentParser(description='Build a UniPlus+ ProFile data disk image.')
    ap.add_argument('output')
    ap.add_argument('mappings', nargs='+', help='DEST=SOURCE')
    ap.add_argument('--fname', default='src')
    ap.add_argument('--fpack', default='lisa')
    ap.add_argument('--base-image', help='existing ProFile image to copy into the start of the new disk')
    ap.add_argument('--disk-sectors', type=int, default=DISK_SECTORS, help='total sectors (19456 = 10 MB)')
    ap.add_argument('--start', type=int, default=PART_START, help='sector where the new filesystem starts')
    ap.add_argument('--blocks', type=int, default=FSIZE, help='size of the new filesystem in 512-byte blocks')
    ap.add_argument('--replace-partition', action='store_true',
                    help='discard whatever the base image holds from --start onward (e.g. an older source partition)')
    ap.add_argument('--prlmap', metavar='LETTER',
                    help='also set partition LETTER of the kernel partition table on the base image to --start/--blocks')
    args = ap.parse_args()

    if os.path.exists(args.output):
        sys.exit('%s exists; refusing to overwrite' % args.output)
    DISK_SECTORS, PART_START, FSIZE = args.disk_sectors, args.start, args.blocks
    if PART_START + FSIZE > DISK_SECTORS:
        sys.exit('filesystem (sector %d + %d blocks) does not fit on a %d-sector disk'
                 % (PART_START, FSIZE, DISK_SECTORS))
    if args.base_image:
        BASE_IMAGE = bytearray(open(args.base_image, 'rb').read())
        if args.replace_partition:
            del BASE_IMAGE[PART_START * SECTOR:]
        if args.prlmap:
            off = patch_prlmap(BASE_IMAGE, args.prlmap, PART_START, FSIZE)
            print('patched prlmap[%s] = {%d, %d} at byte %d of the base image'
                  % (args.prlmap, PART_START, FSIZE, off))
    elif args.prlmap:
        sys.exit('--prlmap needs --base-image')

    now = int(time.time())
    root = Node('', True, mtime=now, mode=0o755)
    for m in args.mappings:
        if '=' not in m:
            sys.exit('mapping must be DEST=SOURCE: %s' % m)
        dest, src = m.split('=', 1)
        if '/' in dest or not dest:
            sys.exit('DEST must be a single top-level name: %s' % dest)
        root.children.append(load(src, dest))

    fs = build(root, now)
    data = image_bytes(fs, now, args.fname, args.fpack)
    with open(args.output, 'wb') as f:
        f.write(data)
    free = verify(args.output, root)
    used = fs.next_block - ISIZE
    print('wrote %s: %d files/dirs, %d inodes used of %d, %d data blocks used (%.2f MB), %d free (%.2f MB); verified'
          % (args.output, fs.next_ino - 2, len(fs.inodes), NINODES, used, used * DATA / 1e6,
             free, free * DATA / 1e6))


if __name__ == '__main__':
    main()
