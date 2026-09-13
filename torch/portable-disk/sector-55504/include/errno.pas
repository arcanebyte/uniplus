{
  Pascal-2 version of /usr/include/errno.h
  Release version: 2.1C  Level: 1  Date: 21-Feb-1984 22:40:58
  Processor: 68000
  System: UNIX
  Flavor: UniPlus System III
}

const
  { Error codes }
  EPERM   = 1;                  { Not super-user }
  ENOENT  = 2;                  { No such file or directory }
  ESRCH   = 3;                  { No such process }
  EINTR   = 4;                  { interrupted system call }
  EIO     = 5;                  { I/O error }
  ENXIO   = 6;                  { No such device or address }
  E2BIG   = 7;                  { Arg list too long }
  ENOEXEC = 8;                  { Exec format error }
  EBADF   = 9;                  { Bad file number }
  ECHILD  = 10;                 { No children }
  EAGAIN  = 11;                 { No more processes }
  ENOMEM  = 12;                 { Not enough core }
  EACCES  = 13;                 { Permission denied }
  EFAULT  = 14;                 { Bad address }
  ENOTBLK = 15;                 { Block device required }
  EBUSY   = 16;                 { Mount device busy }
  EEXIST  = 17;                 { File exists }
  EXDEV   = 18;                 { Cross-device link }
  ENODEV  = 19;                 { No such device }
  ENOTDIR = 20;                 { Not a directory }
  EISDIR  = 21;                 { Is a directory }
  EINVAL  = 22;                 { Invalid argument }
  ENFILE  = 23;                 { File table overflow }
  EMFILE  = 24;                 { Too many open files }
  ENOTTY  = 25;                 { Not a typewriter }
  ETXTBSY = 26;                 { Text file busy }
  EFBIG   = 27;                 { File too large }
  ENOSPC  = 28;                 { No space left on device }
  ESPIPE  = 29;                 { Illegal seek }
  EROFS   = 30;                 { Read only file system }
  EMLINK  = 31;                 { Too many links }
  EPIPE   = 32;                 { Broken pipe }

  { math software }
  EDOM    = 33;                 { Math arg out of domain of func }
  ERANGE  = 34;                 { Math result not representable }
