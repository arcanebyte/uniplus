{
  Pascal-2 version of /usr/include/time.h
  Release version: 2.1C  Level: 1  Date: 21-Feb-1984 22:42:57
  Processor: 68000
  System: UNIX
  Flavor: UniPlus System III
}

type
  ptm_t = ^tm_t;
  tm_t = record { see ctime(3) }
    tm_sec: integer;
    tm_min: integer;
    tm_hour: integer;
    tm_mday: integer;
    tm_mon: integer;
    tm_year: integer;
    tm_wday: integer;
    tm_yday: integer;
    tm_isdst: integer;
    end;
