/******************************************************************************
 *                                                                            *
 * A TorchDraw header file.   ICONS                                           *
 *                                                                            *
 ******************************************************************************/

/* SCCS identification "@(#) icon.h 1.1@(#)" */

#define ICONIDLEN 9

typedef struct
{
   form  data;
   form  mask;
   char  ident [ICONIDLEN];       /* Icon identifying string */
}
icon;

