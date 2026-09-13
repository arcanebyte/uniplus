{
  Pascal-2 version of /usr/include/sys/param.h
  Release version: 2.1C  Level: 1  Date: 21-Feb-1984 23:26:06
  Processor: 68000
  System: UNIX
  Flavor: UniPlus System III
}

const
  { fundamental variables don't change too often }

  NOFILE  = 20;                 { max open files per process }
  MAXPID  = 30000;              { max process id }
  MAXUID  = 60000;              { max user id }
  MAXLINK = 1000;               { max links }
  NPHYS   = 4;                  { max simultaneous phys() calls }
  NSCAT   = 4;                  { number of scatter swapped segments }

  USTART  = 16#0000000;         { logical start of user program }
  UEND    = 16#1000000;         { logical end of user program +1 }
  DOFFSET = 0;                  { Data offset }
  KVOFFSET= 0;                  { Kernel virtual offset from physical 0 mem }
  { MAXMEM= (btoc(v.v_uend-v.v_ustart));{ Maximum size of user program }
  SSIZE   = 1;                  { initial stack size (in clicks) }
  SINCR   = 1;                  { increment of stack (in clicks) }
  USIZE   = 1;                  { size of user block (in clicks) }
  NSWB    = 3;                  { size of swap pool }

  CANBSIZ = 256;                { max size of typewriter line   }
  HZ      = 64;                 { Ticks/second of the clock }
  CLKTICK = 15625;              { microseconds in a clock tick }
  NCARGS  = 5120;               { # characters in exec arglist }

  NBPW    = 4;                  { number of bytes in an integer }
  BSIZE   = 512;                { size of secondary block (bytes) }
  NINDIR  = 128;                { BSIZE div size(daddr_t) }
  BMASK   = 8#0777;             { BSIZE-1 }
  INOPB   = 8;                  { inodes per block }
  BSHIFT  = 9;                  { LOG2(BSIZE) }
  NMASK   = 8#0177;             { NINDIR-1 }
  NSHIFT  = 7;                  { LOG2(NINDIR) }
  NULL    = 0;
  CMASK   = 0;                  { default mask for file creation }
  CDLIMIT = 16777216;           { default max write address (2^24) }
  NODEV   = -1;                 { (dev_t)(-1) }
  ROOTINO = 2;                  { i number of all roots }
  SUPERB  = 1;                  { block number of the super block }
  DIRSIZ  = 14;                 { max characters per directory }
  NICINOD = 100;                { number of superblock inodes }
  NICFREE = 50;                 { number of superblock free blocks }

  { priorities probably should not be altered too much }

  PSWP    = 0;
  PINOD   = 10;
  PRIBIO  = 20;
  PZERO   = 25;
  NZERO   = 20;
  PPIPE   = 26;
  PWAIT   = 30;
  PSLEP   = 40;
  PUSER   = 50;
  PIDLE   = 127;


  %include mmu;


  { Some macros for units conversion }

  function ctos(x: integer): integer;
  { core clicks to segments }
  begin ctos := (x + (SEGSIZE div PAGESIZE - 1)) div (SEGSIZE div PAGESIZE) end;

  function stoc(x: integer): integer;
  { segments to core clicks }
  begin stoc := x * (SEGSIZE div PAGESIZE) end;

  function btod(x: integer): integer;
  { bytes to disk blocks }
  begin btod := (x + (BSIZE - 1)) div BSIZE end;

  function dtob(x: integer): integer;
  { disk blocks to bytes }
  begin dtob := x * BSIZE end;

  function ctod(x: integer): integer;
  { core clicks to disk blocks }
  begin ctod := x * (PAGESIZE div BSIZE) end;

  function dtoc(x: integer): integer;
  { disk blocks to core clicks }
  begin dtoc := x div (PAGESIZE div BSIZE) end;

  function itod(x: ino_t): daddr_t;
  { inumber to disk address }
  begin itod := (x + 15) div 8 end;

  function itoo(x: ino_t): integer;
  { inumber to disk offset }
  begin itoo := (x + 15) mod 8 end;

  function ctob(x: integer): integer;
  { clicks to bytes }
  begin ctob := x * PAGESIZE end;

  function btoc(x: integer): integer;
  { bytes to clicks (round up) }
  begin btoc := (x + (PAGESIZE - 1)) div PAGESIZE { AND ADDRMASK } end;

  function btoct(x: integer): integer;
  { bytes to clicks (truncate) }
  begin btoct := x div PAGESIZE { AND ADDRMASK } end;

  function ctom(x: integer): integer;
  { clicks to memory management units }
  begin ctom := x div 256 end;

  function major(x: dev_t): integer;
  { major part of a device }
  begin major := x div 256 end;

  function minor(x: dev_t): integer;
  { minor part of a device }
  begin minor := x mod 256 end;

  function makedev(x, y: integer): dev_t;
  { make a device number }
  begin makedev := (x * 256) + y end;


  function lobyte(x: uns_word): uns_byte;
  { low-order (less significant) byte of a word }
  begin lobyte := x mod 256 end;

  function hibyte(x: uns_word): uns_byte;
  { low-order (more significant) byte of a word }
  begin hibyte := x div 256 end;

  function loword(x: uns_long): uns_word;
  { low-order (less significant) word of a long }
  begin loword := x mod 65536 end;

  function hiword(x: uns_long): uns_word;
  { high-order (more significant) word of a long }
  begin hiword := x div 65536 end;
