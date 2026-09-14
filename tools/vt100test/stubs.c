/*
 * stubs.c -- a character-grid model of the bitmap display routines in
 * v1.5/sys/bm.c (and the kernel routines vt100.c calls), so vt100.c can be
 * tested on the Mac.  Rows and columns include the one-cell border.
 */
#include <stdio.h>
#include <string.h>
#define R 40
#define C 90
#define LB (90*9)
char grid[R][C]; char inv[R][C]; char rev[R][C];
char fake[LB*42]; char *bmscrn = fake;
char bmbck = -1, bmcolor = 0, bmnormal = -1;
char kb_altkp, kb_ckm, kb_chrbuf;
long lbolt;
char replies[256]; int nrep;
int beeps;
void reset_grid(void){ memset(grid,' ',sizeof grid); memset(inv,0,sizeof inv); memset(rev,0,sizeof rev);}
int bmputc(int r,int c,int k){ grid[r][c]=k; rev[r][c]=(bmbck!=bmnormal); return 0;}
int bminvert(int r,int c){ inv[r][c]^=1; return 0;}
int bmmvc(int dr,int dc,int sr,int sc){ grid[dr][dc]=grid[sr][sc]; rev[dr][dc]=rev[sr][sc]; inv[dr][dc]=inv[sr][sc]; return 0;}
int bmcpl(int dl,int sl){ memcpy(grid[dl],grid[sl],C); memcpy(inv[dl],inv[sl],C); memcpy(rev[dl],rev[sl],C); return 0;}
int bmblank(int dl){ memset(grid[dl],' ',C); memset(inv[dl],0,C); memset(rev[dl],0,C); return 0;}
int bmclear(void){ reset_grid(); return 0;}
int bmswitch(void){ bmcolor = bmcolor ? 0 : -1; return 0;}
int blt(char *d, char *s, int n){ int dr=(d-bmscrn)/LB, sr=(s-bmscrn)/LB, rows=n/LB, i;
  for(i=0;i<rows;i++){ if (sr+i < R) bmcpl(dr+i, sr+i); else bmblank(dr+i);} return 0;}
int beep(void){ beeps++; return 0;}
int l2undim(void){ return 0;}
int spl7(void){ return 0;} int splx(int x){ return 0;}
int cointr(int d){ replies[nrep++]=kb_chrbuf; replies[nrep]=0; return 0;}
