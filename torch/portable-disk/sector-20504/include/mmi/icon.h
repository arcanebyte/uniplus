/******************************************************************************
 *                                                                            *
 * A TorchDraw header file.   ICONS                                           *
 *                                                                            *
 ******************************************************************************/

#define ICONIDLEN 9

typedef struct
{
   form  data;
   form  mask;
   char  ident [ICONIDLEN];       /* Icon identifying string */
}
icon;

