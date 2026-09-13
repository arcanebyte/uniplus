/******************************************************************************
 *                                                                            *
 * TITLE     : XXX - Long Integer Library Definitions.                        *
 *                                                                            *
 * COPYRIGHT : (c) 1985 TORCH Computers Limited                               *
 *                                                                            *
 * AUTHOR    : D.M. Gilday                                                    *
 * CREATED   : 28th August 1985                                               *
 *                                                                            *
 * UPDATE ON | CHANGES MADE                                       | BY WHOM   *
 * ----------|----------------------------------------------------|-----------*
 * 16-Sep-85 | LIGEZero() added as a macro.                       |  DMG      *
 *           |                                                    |           *
 *                                                                            *
 ******************************************************************************/

extern LIRShift();
extern LIAdd();
extern LISub();
extern LIUMul();
extern unsigned int LISqrt();
extern LIInc();
extern LINeg();
extern LIGTZero();

#define LIGEZero(i)	((i)->h >= 0)	/* i.e. if sign positive */

/*                                                            *****************
                                                              *               *
                                                              * LINT          *
                                                              *               *
                                                              *****************
-------------------------------------------------------------------------------
| Type definition of a 64 bit signed integer.                                 |
-------------------------------------------------------------------------------
*/

typedef struct
{
   int h;
   unsigned int l;
} lint;

