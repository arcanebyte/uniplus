/******************************************************************************
 *                                                                            *
 * This program is a calculator of a similar sort to that found on other      *
 * windowed systems.						              *
 * It is intended to be a test vehicle for the window device       	      *
 * driver.                                                                    *
 *                        Commenced on 29th January 1985                      *
 *                                                                            *
 ******************************************************************************/

#include <math.h>
#include <termio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <wlib.h>

#define EventHandled()    ioctl(fd, 'J' << 8 | EEVNTH, 0)

#define MAX_AREAS 26        /* Largest number of rectangles */

extern errno;   /* system error code */
extern int fd;
extern FILE *fs;

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
static int selected, calc_ok;

   /* An array of interesting rectangles to point at */

static rectangle boxes [MAX_AREAS] =
{
   40,  184, 80, 80,
   40,  312, 80, 80,
   168, 312, 80, 80,
   296, 312, 80, 80,
   424, 312, 80, 80, 
   552, 312, 80, 80, 
   40,  440, 80, 80, 
   168, 440, 80, 80, 
   296, 440, 80, 80, 
   424, 440, 80, 80, 
   552, 440, 80, 80, 
   40,  568, 80, 80, 
   168, 568, 80, 80, 
   296, 568, 80, 80, 
   424, 568, 80, 80, 
   552, 568, 80, 80, 
   40,  696, 80, 80, 
   168, 696, 80, 80, 
   296, 696, 80, 80, 
   424, 696, 80, 80, 
   552, 696, 80, 80,
   40,  824, 80, 80, 
   168, 824, 80, 80, 
   296, 824, 80, 80, 
   424, 824, 80, 80, 
   552, 824, 80, 80 
};

static rectangle display [] =
{
   40, 56, 592, 80
};

#define MEMORY    1
#define PCENT     3
#define EXP       4
#define DIV       5
#define TIMES     10
#define MINUS     15
#define PLUS      20
#define CANCEL    21
#define POINT     23
#define NEG       24
#define EQUALS    25

static char lbox [MAX_AREAS] =
{
   0xb2,
   'm',  0,   '%', 'e', 0xf6,
   0,    '7', '8', '9', 'x',
   0,    '4', '5', '6', '-',
   0,    '1', '2', '3', '+',
   'c',  '0', '.', 0xf1,'=',
};

static char colours [MAX_AREAS] =
{
   NULLBITS,
   BITS_9, BITS_9, BITS_9, BITS_9, BITS_9,
   BITS_9, NULLBITS, NULLBITS, NULLBITS, BITS_9,
   BITS_9, NULLBITS, NULLBITS, NULLBITS, BITS_9,
   BITS_9, NULLBITS, NULLBITS, NULLBITS, BITS_9,
   BITS_5, NULLBITS, NULLBITS, NULLBITS, BITS_9
};

static int lookup [MAX_AREAS] =
{
   0,
   0, 0, 0, 0, 0,
   0, 7, 8, 9, 0,
   0, 4, 5, 6, 0,
   0, 1, 2, 3, 0,
   0, 0, 0, 0, 0
};

static rectangle winrect =
{
   160,   /* Top left corner of window */
   64,    /*  "  right  "     "   "    */
   168,   /* Width of window           */
   120    /* Height of window          */
};

static ostruct  opargs;   /* Structure passed in ioctl call                 */
static emask;             /* Event Mask */

static char inchar;       /* Character got from keyboard */
static double num1, num2, memory;
static int real, operator, lastop;
static int act, minflag;
static inexp, exponent;
static char nums [18];
static point zpt =
{
   0, 0
};

int MLDAct (), MLUAct (), UEAct ();

static int (*actprocs  [16]) () =
{
   0, 0,
   MLDAct, 0,
   MLUAct, 0,
   0, 0, 0,
   UEAct,
   0, 0, 0, 0, 0, 0
};


/*                                                                *************
                                                                  *           *
                                                                  *   MAIN    *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| The entry point                                                             |
-------------------------------------------------------------------------------
*/

main (argno, argv)
int argno;
char *argv[];

{
   struct termio termargs;

      /* Set up size and position of window in global coords */
      /* This gives a Local rectangle of 0, 0, 336, 832      */

      /* First open the window in delayed mode. Open completed by ioctl */

   selected = 1;
   calc_ok  = 0;

   if (WOpen (0, 1, &winrect, VANILLA, &zpt, actprocs, 0) < 0)
   {
      printf ("calc: failed to open\n");
      exit (1);
   }

      /* Events could happen from here on in */

   ioctl (fd, TCGETA, &termargs);   /* Read the tty controls */
   termargs.c_oflag &= ~OPOST;      /* Switch off post processing */
   termargs.c_lflag &= ~ECHO;       /* Stop character reflection */
   termargs.c_lflag &= ~ICANON;     /* No canonical input */
   termargs.c_cc [VMIN] = 1;        /* Minimum of 1 character */
   termargs.c_cc [VTIME] = 255;     /* Maximum time of 255 secs */
   ioctl (fd, TCSETA, &termargs);   /* Tell the driver */

   HideCursor ();

   DrawCalc ();                 /* Draw the calculator */

   calc_ok=1;

   do
      if (read (fd, &inchar, 1) == 1)   /* Sleep on read */
         ProcessKey (inchar);
   while (1);
}

/*                                                                *************
                                                                  *           *
                                                                  *   CLOSE   *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|                                                                             |
-------------------------------------------------------------------------------
*/

Close ()
{
   EventHandled ();
   fclose (fs);       /* And close everything up */
   close (fd);
   exit (0);
}

/*                                                                *************
                                                                  *           *
                                                                  * DRAWCALC  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Draw the calculator face                                                    |
-------------------------------------------------------------------------------
*/

DrawCalc ()
{
   register x;
   rectangle temprect;

   winrect.width  += winrect.x;
   winrect.height += winrect.y;
   GToL (&winrect);
   GToL (&winrect.width);
   winrect.width  -= winrect.x;
   winrect.height -= winrect.y;

   SetFPat (BITS_12_0);
   FillRect (&winrect);
   SetFPat (NULLBITS);

      /* frame all the active areas */

   for (x=0; x < MAX_AREAS; x++)
   {
      OFrameRect ( &boxes[x] );
      blt (&temprect, &boxes[x], sizeof (rectangle));
      temprect.x += 8;
      temprect.y += 8;
      OFrameRect ( &temprect );

      ClrRect (&boxes[x]);

      SetFPat (colours[x]);
/*
      if (colours[x])
         Bold (TOGGLE);
*/

      if (x)
         MoveCursor ( ((x-1) % 5) *4 + 2, (x-1)/5*2 + 5);
      else
         MoveCursor (2, 3);
      putc (lbox [x], fs);
/*
      if (colours[x])
         Bold (TOGGLE);
*/

      SetFPat (NULLBITS);
        
   }

      /* Frame the display */

   FillRect (display);
   if (!selected)
   {
      TextFace (NORMAL);
      InvRect (display);
   }
   else
      TextFace (INVERSE);
   SetFPat (BITS_15_3);
   OFrameRect (display);
   SetFPat (NULLBITS);

   fflush (fs);                    /* Flush the output buffer */
   Display ((double) 0);
}

ProcessKey (pchar)
char pchar;

{
   register x;

   if (pchar == '\n')
      pchar = '=';

   if (pchar == '*')
      pchar = 'x';

   if (pchar == '/')
      pchar = 0xf6;

   if (pchar == 's')
      pchar = 0xf1;

   for (x=1; x<MAX_AREAS; x++)
      if (lbox [x] == pchar)
      {
         InvRect (&boxes[x]);
         fflush (fs);
         DoIt (x);
         InvRect (&boxes[x]);
         fflush (fs);
         break;
      }
}

int tmp;

UEAct (eventptr)
EventRec *eventptr;

{
   if (calc_ok)
   {
      InvRect (display);
      Inverse (TOGGLE);
      fflush (fs);
   }
   selected ^= 1;
   return (1);
}

MLDAct (eventptr)
EventRec *eventptr;

{
   point temp;

   temp.x = eventptr->where.x;
   temp.y = eventptr->where.y;

   GToL (&temp);  /* convert to local coordinate */

       /* In close box? */

   if (PtInRect (&temp, &boxes[0]))
   {
      InvRect (&boxes[0]);
      fflush (fs);
      return;
   }

       /* event in any other rectangle? */

   if (tmp=CheckRest (&temp))
   {
      InvRect (&boxes[tmp]);
      fflush (fs);
   }

      /* Now check the standard areas */

   else
      DoDrag ();

   return;
}

MLUAct (eventptr)
EventRec *eventptr;

{
   point temp;

   temp.x = eventptr->where.x;
   temp.y = eventptr->where.y;

   GToL (&temp);  /* convert to local coordinate */

       /* In close box? */

   if (!tmp)
   {
      InvRect (&boxes[0]);
      if (PtInRect (&temp, &boxes[0]))
         Close ();     /* Never comes back */
   }

       /* event in any other rectangle? */

   else
   {
      if (tmp == CheckRest (&temp))
         DoIt (tmp);
      InvRect (&boxes[tmp]);
   }
   fflush (fs);

   tmp = 0;
}

/*                                                                *************
                                                                  *           *
                                                                  *   DOIT    *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Take appropriate action                                                     |
-------------------------------------------------------------------------------
*/

DoIt (boxno)
int boxno;

{
   register char *sp1, *sp2;
   register x;

   switch (boxno)
   {
      case EXP :
            if (!inexp)
            {
               sprintf (nums, "% 17.10g",num2);
               for (x=0; nums[x+12] != 'e' && x<5; x++) {}
               sp1 = &nums[0];
               sp2 = &nums[x];
               do
                   *sp1++ = *sp2++;
               while (*sp2 && *sp2 != 'e');
               *sp1 = 0;
               exponent = 0;
            }
            inexp = 1;
            break;

      case POINT :
            inexp = 0;
            if (!real)
               real = 10;
            break;

      case CANCEL :
            inexp = num2 = real = minflag = 0;
            if (!operator)   /* If no pending operation */
               act = 1;
            Display ((double) 0);
            break;

      case MEMORY :
             if (!act || lastop == CANCEL)
                num2 = memory;
             else
                memory = num2;
             act = 1;
             Display (num2);
             break;

      /* Are we trying to stack an operation? */

      case PLUS :
      case MINUS :
      case TIMES :
      case DIV :
            inexp = 0;
            if (act)   /* If last operation was not an operator */
            {
/*             if (operator)   /* If an old operation is pending */
                  Operate();   /* Perform the operation */
               num1 = num2;    /* rotate stack */
               num2 = 0;       /* zero num2 */
               operator = boxno;  /* This is the new operator */
               minflag = real = act = 0; /* An operation has been carried out */
            }
            break;

      case EQUALS :
            inexp = 0;
            if (act)   /* If an old operation is pending */
               Operate();   /* Perform the operation */
            num1 = num2;    /* rotate stack */
            operator = boxno;   /* No operations pending */
            minflag = real = 0;
            act = 1;        /* There is a number to use */
            break;

      case NEG :
            minflag = 1;     /* Negate the number */

      /* Still accumulating a number */

      default :
            if (lbox [boxno] == 0)
               break;

               /* No operator pending... */
            if (operator == EQUALS || operator == MEMORY)
               num2 = operator = minflag = 0;    /* So zero num2 */
         
            Accum (&num2, boxno);
            act = 1;    /* have a bit of a number to use */
            break;

   } /* End Switch */

   lastop = boxno;
}

/*                                                                *************
                                                                  *           *
                                                                  *  OPERATE  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|                                                                             |
-------------------------------------------------------------------------------
*/

Operate ()
{
   switch (operator)
   {
      case PLUS :
         num2 = num1 + num2;
         break;

      case MINUS :
         num2 = num1 - num2;
         break;

      case TIMES :
         num2 = num1 * num2;
         break;

      case DIV :
         if (num2 != 0)
            num2 = num1 / num2;
         break;

      default :
         break;
   }
   Display (num2);
   num1 = num2;
}

/*                                                                *************
                                                                  *           *
                                                                  *   ACCUM   *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|                                                                             |
-------------------------------------------------------------------------------
*/

Accum (numptr, boxno)
double *numptr;
int boxno;

{
   int oldexp;

   if (inexp)
   {
      oldexp = exponent;

      if (boxno != NEG)
      {
         if (exponent >= 0)
            exponent = (exponent * 10 + lookup[boxno]) % 100;
         else
            exponent = (exponent * 10 - lookup[boxno]) % 100;
      }
         
      if (minflag && exponent != 0)
      {
         exponent = -exponent;
         minflag = 0;
      }
      *numptr = *numptr * pow ((double) 10, (double) (exponent - oldexp));
      goto disp;
   }

   if (boxno == NEG)
      goto negate;

   if (real)
   {
      if (*numptr >= 0)
         *numptr = *numptr + (double) lookup [boxno] / real;
      else
         *numptr = *numptr - (double) lookup [boxno] / real;
      real = real * 10;
   }
   else
   {
      if (*numptr >= 0)
         *numptr = *numptr * 10 + lookup [boxno];
      else
         *numptr = *numptr * 10 - lookup [boxno];
   }

negate:
   if (minflag && *numptr != 0)
   {
      *numptr = -(*numptr);
      minflag = 0;
   }

disp:
   Display (*numptr);
}

/*                                                                *************
                                                                  *           *
                                                                  *  DISPLAY  *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
|                                                                             |
-------------------------------------------------------------------------------
*/

Display (number)
double number;

{
   MoveCursor (2,1);
   if (inexp)
   {
      fprintf (fs, "%s", nums);
      putc ('e', fs);
      fprintf (fs, "%+-4.1d",exponent);
   }
   else
      fprintf (fs,"% 17.10g",number);
   fflush (fs);                    /* Flush the output buffer */
}


/*                                                                *************
                                                                  *           *
                                                                  * CHECKREST *
                                                                  *           *
                                                                  *************
-------------------------------------------------------------------------------
| Work through other rectangles, take appropriate action                      |
-------------------------------------------------------------------------------
*/

CheckRest (pptr)
point *pptr;

{
   register x;

   for (x=1; x<MAX_AREAS; x++)
      if (PtInRect (pptr, &boxes[x]))
         return (x);

   return (0);
}

