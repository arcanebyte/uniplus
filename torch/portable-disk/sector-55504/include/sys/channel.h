
/* Definitions for Channel Testing */

struct channel_vars {

  char  intail;
  char  inhead;
  short inaddr;
  char  outhead;
  char  outtail;
  short outaddr;
  char  inwm;
  char  outwm;
  char  busoff;
  char  rdwr;
  char  enbl;
  char  extra1;   /* these last 3 only used with DaCom and IEEE-488 */
  char  extra2;
  char  extra3;
};


