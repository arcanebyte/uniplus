/*
/ loopback.c
/
/ This application can be used to determine functioning
/ serial ports on an Apple Lisa running UniPlus+ UNIX.
/
/ Author: James Denton (james.denton@outlook.com)
/ Twitter: @jimmdenton, @arcanebyte
*/

#include <stdio.h>
#include <fcntl.h>
#include <termio.h>
#include <string.h>

main(argc,argv)
int    argc;
char   *argv[];
{
  /* Initialize struct for serial interfaces */
  struct termio tio;
  struct termio stdio;

  /* Initialize struct for existing settings */
  struct termio oldstdio;
  struct termio oldtio;

  int tty_fd;
  int flags;

  unsigned char c='D';

  printf("Syntax: %s /dev/tty20 (for example)\n",argv[0]);
  printf("Type on the keyboard. If you see characters, the loopback is working.\n");
  printf("If you do not see characters, the loopback is malfunctioning.\n\n");
  printf("Type 'q' to quit\n\n");


  /* Grab existing settings for console/tty */
  memset(&oldstdio,0,sizeof(oldstdio));
  ioctl(1,TCGETA,&oldstdio);

  /* Apply new settings */
  memset(&stdio,0,sizeof(stdio));
  ioctl(1,TCSETA,&stdio);
  ioctl(1,TCFLSH,&stdio);

  flags = fcntl(0,F_GETFL,0); /* Grab settings to restore */
  fcntl(0, F_SETFL, O_NDELAY); /* Apply change to stdin */



  memset(&tio,0,sizeof(tio));
  tio.c_cflag=CS8|CREAD|CLOCAL;
  tio.c_cc[VMIN]=1;
  tio.c_cc[VTIME]=5;

  tty_fd=open(argv[1], O_RDWR | O_NDELAY);

  /* Get existing settings for tty */
  memset(&oldtio,0,sizeof(&oldtio));
  ioctl(tty_fd,TCGETA,&oldtio);


  /* Apply new settings and test loopback functions */
  ioctl(tty_fd,TCSETA,&tio);
  while (c!='q')
  {
    if (read(tty_fd,&c,1)>0)  write(1,&c,1); /* Read from serial */
    if (read(0,&c,1)>0)       write(tty_fd,&c,1); /* Write to serial */
  }


  /* Restore settings */
  ioctl(1,TCSETA,&oldstdio);
  ioctl(tty_fd,TCSETA,&oldtio);
  fcntl(0,F_SETFL,flags);
  close(argv[1]);
  return 0;
}
