#!/usr/bin/env python3
"""
lisa_names.py -- check C sources for the Apple Lisa cc's name limits before
building them on the Lisa.

The UniPlus+ cc keeps 7 characters of an external name (8 with the leading
underscore), so two functions or globals that differ only after the 7th
character become one symbol, and a call to a function the Lisa's libc
doesn't have (strstr, bcopy, index) only fails at link time on the Lisa.
This compiles each file with the host clang against the Lisa headers,
lists the external names with nm, and reports:

  - names that collide once cut to 7 characters;
  - references that nothing defines: not the sources, not the extra
    objects/libraries given with -l, not /lib/libc.a from the Lisa dump;
  - names the sources define that libc also defines (a local version of a
    libc routine, or an accidental clash).

USAGE
    python3 tools/lisa_names.py [-I DIR]... [-D NAME]... [-l LIB]... FILE...

    FILE    .c files are compiled; .s files are scanned for .globl labels
    -I, -D  passed to clang (dump/usr/include is always searched last)
    -l LIB  a Lisa archive (.a) or a .c/.s file whose definitions count as
            available, e.g. -l netlib/sockcall.s

Exit status 1 if anything is reported.  Needs clang and nm (Xcode tools).
"""
import argparse
import os
import re
import struct
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LIBC = os.path.join(ROOT, 'dump', 'lib', 'libc.a')
INCLUDE = os.path.join(ROOT, 'dump', 'usr', 'include')
# emitted by the host compiler, never by the Lisa's
HOST_HELPERS = {'_memcpy', '_memset', '_memmove', '___stack_chk_fail', '___stack_chk_guard', '_bzero'}


def lisa_archive_symbols(path):
    """Defined external symbols in a UniPlus+ archive of a.out objects."""
    d = open(path, 'rb').read()
    if struct.unpack('>l', d[:4])[0] != 0o177545:
        sys.exit('%s: not a UniPlus+ archive' % path)
    out, off = set(), 4
    while off + 28 <= len(d):
        size = struct.unpack('>l', d[off + 24:off + 28])[0]
        obj = d[off + 28:off + 28 + size]
        off += 28 + size + (size & 1)
        ts, ds, bs, ss = struct.unpack('>4l', obj[4:20])
        s, i = obj[32 + ts + ds:32 + ts + ds + ss], 0
        while i < len(s):
            t = s[i]
            j = s.index(b'\0', i + 6)
            if t & 0o40 and t & 0o37:          # external and defined
                out.add(s[i + 6:j].decode('latin1'))
            i = j + 1
    return out


def asm_globals(path):
    names = set()
    for m in re.finditer(r'\.globl\s+([^\n]*)', open(path).read()):
        names |= {n.strip() for n in m.group(1).split('|')[0].split(',') if n.strip()}
    return names


def c_symbols(path, cflags):
    """(defined, undefined) external names of a C file, with the leading underscore."""
    with tempfile.TemporaryDirectory() as tmp:
        obj = os.path.join(tmp, 'x.o')
        cmd = ['clang', '-c', '-std=c89', '-w', '-O0', '-fno-builtin', '-ffreestanding', '-fno-stack-protector',
               '-fno-common', '-Wno-return-mismatch', '-Wno-int-conversion', '-Wno-incompatible-pointer-types',
               '-Wno-implicit-function-declaration', '-Wno-implicit-int',
               '-target', 'x86_64-apple-macos11', '-nostdinc', '-Dmc68000'] + cflags + \
              ['-I', INCLUDE, '-o', obj, path]
        r = subprocess.run(cmd, capture_output=True, text=True)
        if r.returncode:
            sys.exit('%s: clang failed:\n%s' % (path, r.stderr))
        nm = subprocess.run(['nm', '-g', obj], capture_output=True, text=True).stdout
    defined, undefined = set(), set()
    for line in nm.splitlines():
        parts = line.split()
        if len(parts) >= 2 and parts[-2] == 'U':
            undefined.add(parts[-1])
        elif len(parts) == 3:
            defined.add(parts[-1])
    return defined, undefined


def main():
    ap = argparse.ArgumentParser(description="Check C sources against the Lisa cc's 7-character names and libc.")
    ap.add_argument('-I', dest='incs', action='append', default=[])
    ap.add_argument('-D', dest='defs', action='append', default=[])
    ap.add_argument('-l', dest='libs', action='append', default=[])
    ap.add_argument('files', nargs='+')
    args = ap.parse_args()
    cflags = sum((['-I', i] for i in args.incs), []) + ['-D' + d for d in args.defs]

    def symbols(path):
        if path.endswith('.s'):
            return asm_globals(path), set()
        if path.endswith('.a'):
            return lisa_archive_symbols(path), set()
        return c_symbols(path, cflags)

    cut = lambda n: n[:8]
    libc = lisa_archive_symbols(LIBC)
    extra = set()
    for lib in args.libs:
        extra |= symbols(lib)[0]
    where, defined, undefined = {}, set(), set()
    for f in args.files:
        d, u = symbols(f)
        for n in d | u:
            where.setdefault(n, set()).add(os.path.basename(f))
        defined |= d
        undefined |= u
    bad = 0

    # .s files spell long names the way cc cuts them (_gethost for gethostname),
    # so an assembler label that is already the short form is the same symbol
    asm = set()
    for f in args.files + args.libs:
        if f.endswith('.s'):
            asm |= asm_globals(f)
    byname = {}
    for n in defined | undefined | extra:
        byname.setdefault(cut(n), set()).add(n)
    for short, names in sorted(byname.items()):
        if short in asm:
            names = names - {short}
        if len(names) > 1:
            bad += 1
            print('collision %s: %s' % (short, ', '.join('%s (%s)' % (n, ', '.join(sorted(where.get(n, {'-l'}))))
                                                         for n in sorted(names))))

    have = {cut(n) for n in defined | extra} | libc
    for n in sorted(undefined - HOST_HELPERS):
        if cut(n) not in have:
            bad += 1
            print('undefined %s (used in %s)' % (n, ', '.join(sorted(where[n]))))

    for n in sorted(defined):
        if cut(n) in libc:
            bad += 1
            print('also in libc %s (defined in %s)' % (n, ', '.join(sorted(where[n]))))

    if not bad:
        print('ok: %d files, %d external names' % (len(args.files), len(defined | undefined)))
    sys.exit(1 if bad else 0)


if __name__ == '__main__':
    main()
