{
  Pascal-2 version of /usr/include/sys/types.h
  Release version: 2.1C  Level: 1  Date: 21-Feb-1984 23:17:11
  Processor: 68000
  System: UNIX
  Flavor: UniPlus System III
}

type
  byte = -128..127;
  word = -32768..32767;
  long = -2147483648..2147482647;

  uns_byte = 0..255;
  uns_word = 0..65535;
  uns_long = 0..4294967295;
  uns_int = 0..65535;

  address = 0..4294967295; {2^32-1}

  daddr_t = long;
  caddr_t = ^char;
  ino_t = uns_word;
  cnt_t = word;
  mem_t = long;
  time_t = long;
  label_t = packed array [0..12] of integer;
  dev_t = uns_word;
  off_t = long;
  paddr_t = long;
