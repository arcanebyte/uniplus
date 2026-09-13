#include <termio.h>
#include <fcntl.h>
#include <stdio.h>
#include <mmi.h>
#include <event.h>
#include <edit.h>
#include <zoom.h>
#include <menu.h>
#include <window.h>
#include <eioctl.h>

#define OFORM   'J' << 8 | EOFORM
#define GTOL    'J' << 8 | EGTOL

			/* Version Numbers - External */
static char version [] =
   "(C) Copyright 1985 TORCH Computers Ltd Version 1.00\nTriple X";

			/* Version Numbers Records */
/*
 * Version	- 	Reason for change.
 *
 *   0.10		Release for Beta Test to KRN 13/9/85
 *
 */
point pnt1 = {0,0};
point pnt2 = {1,1};

word chufbits [] =           /* A bitmap */
{
   0x0000, 0x0000, 0x0000,
   0x0003, 0x8003, 0xE000,
   0x0007, 0xE007, 0xF800,
   0x0007, 0xF00F, 0xFC00,
   0x001F, 0xFC0F, 0xF800,
   0x007F, 0xF80F, 0xF000,
   0x00FF, 0xF803, 0xC000,
   0x01FF, 0xC000, 0x0000,
   0x03FF, 0x0000, 0x0000,
   0x03F8, 0x000F, 0x8000,
   0x03E0, 0x000C, 0x0000,
   0x0382, 0x010C, 0x0000,
   0x0387, 0x038C, 0x0000,
   0x038F, 0x838C, 0x0000,
   0x0FFF, 0xFFFC, 0x3C00,
   0x1FFF, 0xFFFE, 0x1800,
   0x1FFF, 0xFFFF, 0x1800,
   0x5FFF, 0xFFFF, 0x1800,
   0x7FFF, 0xFFFF, 0x1800,
   0x5FFF, 0xFFFF, 0xFA00,
   0x0FFF, 0xFFFF, 0xFE00,
   0x00F3, 0xC1E7, 0x8200,
   0x01FF, 0xE3FF, 0xC000,
   0x00F3, 0xC1E7, 0x8000,
   0x1FFF, 0xFFFF, 0xF800,
   0x0000, 0x0000, 0x0000
};

addrptr chufp = chufbits;   /* Pointer to a bitmap */

form chuffer =
{
   &chufp,         /* Pointer to a pointer to a bitmap    */
   0, 0, 40, 26,   /* Boundary rectangle of form          */
   3,              /* Number of words across in ONE plane */
   0,              /* Internal format                     */
   1,              /* Number of planes                    */
   0               /* Must be zero                        */
};

int xscale, yscale;

main ()
{
   struct {
             int  result;
             form *fptr;
          } iostr;
   struct termio termargs;

   iostr.fptr = &chuffer;          /* Set up structure  */
   ioctl (1, OFORM, &iostr);       /* Open the pen form */
   if (iostr.result < 0)
   {
      printf ("Bad result\n");
      exit (0);
   }

   ioctl (1, TCGETA, &termargs);      /* Read the tty controls */
   termargs.c_oflag &= ~OPOST;         /* Switch off post processing */
   ioctl (1, TCSETA, &termargs);      /* Tell the driver */

   SetScale ();                       /* Calculate x & y scaling factors */
   SetPenSize (40*xscale, 26*yscale); /* Change pen size */
   ChPenTo (iostr.result);                        /* Set pen to this form */

   putchar (DROPPEN);                      /* Drop pen */
   do
   {
      int fred;

      putchar (27);
      putchar (FPAT);
      fred = (rand () & 0xf) + 5;
      putchar (fred >> 8);
      putchar (fred);
      Plot (rand () >> 4, rand () >> 4);  /* Plot form at random */
   }
   while (1);
}

Plot (x, y)
{
   putchar (27);      /* Move to */
   putchar (PM);
   putchar (x >> 8);
   putchar (x);
   putchar (y >> 8);
   putchar (y);
   putchar (PLOT);      /* Plot */
}

ChPenTo (penhandle)
{
   putchar (27);
   putchar (PCHANGE);
   putchar (penhandle >> 8);
   putchar (penhandle);
}

SetPenSize (x, y)
{
   putchar (27);
   putchar (PENSIZE);
   putchar (x >> 8);
   putchar (x);
   putchar (y >> 8);
   putchar (y);
}

SetScale ()
{
   ioctl (1, GTOL, &pnt1);
   ioctl (1, GTOL, &pnt2);

   xscale = pnt2.x - pnt1.x;
   yscale = pnt2.y - pnt1.y;
}
   
