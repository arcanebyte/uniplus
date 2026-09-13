{
  Pascal-2 version of /usr/include/sys/stat.h
  Release version: 2.1C  Level: 1  Date: 21-Feb-1984 23:08:49
  Processor: 68000
  System: UNIX
  Flavor: UniPlus System III
}

const
  S_IFMT  = 8#170000;           { type of file }
  S_IFDIR = 8#040000;           {   directory }
  S_IFCHR = 8#020000;           {   character special }
  S_IFBLK = 8#060000;           {   block special }
  S_IFREG = 8#100000;           {   regular }
  S_IFCTG = 8#110000;           {   contigous file }
  S_IFIFO = 8#010000;           {   fifo }
  S_ISUID = 8#4000;             { set user id on execution }
  S_ISGID = 8#2000;             { set group id on execution }
  S_ISVTX = 8#1000;             { save swapped text even after use }
  S_IREAD = 8#0400;             { read permission, owner }
  S_IWRITE= 8#0200;             { write permission, owner }
  S_IEXEC = 8#0100;             { execute/search permission, owner }

type  { Structure of the result of stat }
  stat_t = record
    st_dev: dev_t;
    st_ino: ino_t;
    st_mode: uns_word;
    st_nlink: word;
    st_uid: word;
    st_gid: word;
    st_rdev: dev_t;
    st_size: off_t;
    st_atime: time_t;
    st_mtime: time_t;
    st_ctime: time_t;
  end;
