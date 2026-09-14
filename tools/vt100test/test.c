/*
 * test.c -- checks for the console VT100 emulator, v1.5/sys/vt100.c.
 * Run with tools/vt100test/run.sh.
 */
#include <stdio.h>
#include <string.h>
extern char grid[40][90], inv[40][90], rev[40][90], replies[]; extern int nrep;
extern short vt_row, vt_col; extern char vt_tabset[]; extern char bmbck, bmnormal, bmcolor, kb_ckm;
int vt_putc(char); void reset_grid(void);
int fails;
void out(const char *s){ while(*s) vt_putc(*s++ & 0x7f); }
void outn(const char *s, int n){ while(n--) vt_putc(*s++ & 0x7f); }
/* screen cell (0-based text row, col) */
char cell(int r,int c){ return grid[r+1][c+1]; }
void check(const char *what, int ok){ printf("%s %s\n", ok?"PASS":"FAIL", what); if(!ok) fails++; }
int cursor_is(int r,int c){ return vt_row==r && vt_col==c && inv[r+1][c+1]; }
void clear(void){ out("\033[H\033[2J"); nrep=0; replies[0]=0; }
int rowis(int r, const char *s){ return strncmp(&grid[r+1][1], s, strlen(s))==0; }
int main(void){
  int i; char buf[200];
  reset_grid();
  vt_putc('x'); out("\033[H\033[2J");
  out("\t"); check("tab at power-on to column 8", vt_col==8);
  clear(); for(i=0;i<88;i++) vt_putc('a');
  check("88 chars: cursor stays on last column (deferred wrap)", cursor_is(0,87) && cell(0,87)=='a');
  vt_putc('b'); check("next char wraps to row 1", cell(1,0)=='b' && cursor_is(1,1));
  out("\b \b"); check("erase at row 1 col 0", cell(1,0)==' ' && cursor_is(1,0));
  out("\b \b"); check("erase reverse-wraps to last column of row 0", cell(0,87)==' ' && cursor_is(0,87));
  out("\b \b"); check("next erase is column 86", cell(0,86)==' ' && cell(0,85)=='a' && cursor_is(0,86));
  clear(); out("ab\033[?25lcd\033(Bef\033[38;5;1mgh\033[?1049hij\033#8kl"); 
  check("unknown sequences leave nothing on screen", rowis(0,"abcdefghijkl"));
  check("?25l hides the cursor", !inv[1][13]);
  out("\033[?25h"); check("?25h shows it", inv[1][13]);
  clear(); out("\033[5;30H"); check("CUP 5;30", vt_row==4 && vt_col==29);
  out("\033[6n"); check("DSR 6 reply", strcmp(replies,"\033[5;30R")==0);
  nrep=0; out("\033[c"); check("DA reply", strcmp(replies,"\033[?1;2c")==0);
  nrep=0; out("\033[5n"); check("DSR 5 reply", strcmp(replies,"\033[0n")==0);
  clear(); for(i=0;i<12;i++){ sprintf(buf,"line%02d",i); out(buf); if(i<11) out("\r\n"); }
  out("\033[3;8r"); check("DECSTBM homes the cursor", vt_row==0 && vt_col==0);
  out("\033[8;1H\n"); check("LF at region bottom scrolls region", rowis(2,"line03") && rowis(6,"line07") && rowis(7,"        ") && rowis(8,"line08") && rowis(1,"line01"));
  out("\033[3;1H\033M"); check("RI at region top scrolls down and blanks top", rowis(2,"      ") && rowis(3,"line03") && rowis(8,"line08"));
  out("\033[5;1H\033[2L"); check("IL inside region", rowis(4,"      ") && rowis(5,"      ") && rowis(6,"line04") && rowis(7,"line05") && rowis(8,"line08"));
  out("\033[2M"); check("DL inside region", rowis(4,"line04") && rowis(5,"line05") && rowis(6,"      ") && rowis(7,"      ") && rowis(8,"line08"));
  out("\033[r"); check("reset region", vt_row==0);
  clear(); out("hello world\033[1;6H\033[3@"); check("ICH", rowis(0,"hello    world"));
  out("\033[2P"); check("DCH", rowis(0,"hello  world"));
  out("\033[1;1H\033[4X"); check("ECH", rowis(0,"    o  world"));
  out("\033[1;1H\033[4hab\033[4l"); check("insert mode", rowis(0,"ab    o  world"));
  clear(); out("\033[10;20H\0337\033[1;1Hzz\0338Y"); check("save/restore cursor", cell(9,19)=='Y');
  out("\033[s\033[2;2H\033[uQ"); check("CSI s/u", cell(9,20)=='Q');
  clear(); out("\033[7mR\033[27mN\033[4mU\033[24m\033[0m"); check("SGR 7/27", rev[1][1]==1 && rev[1][2]==0 && bmbck==bmnormal && bmcolor==bmbck);
  clear(); out("\033[12GX\033[3dY"); check("CHA and VPA", cell(0,11)=='X' && cell(2,12)=='Y');
  clear(); out("ab\033[1\030cd"); check("CAN abandons a sequence", rowis(0,"abcd"));
  out("\033[?1h"); check("?1h app cursor keys", kb_ckm==1); out("\033[?1l"); check("?1l", kb_ckm==0);
  clear(); for(i=0;i<40;i++){ sprintf(buf,"L%02d",i); out(buf); out("\r\n"); }
  check("full-screen scroll", rowis(0,"L03") && rowis(36,"L39") && rowis(37,"   "));
  clear(); out("\033[?7l"); for(i=0;i<90;i++) vt_putc('w'); check("?7l: no wrap, last column overwritten", vt_row==0 && cell(1,0)==' ');
  out("\033[?7h");
  clear(); out("\033c"); check("ESC c reset", vt_row==0 && vt_tabset[8]==1);
  clear(); out("a\tb\033[3g\r\tc"); check("TBC 3 clears tabs", cell(0,87)=='c');
  out("\033c");
  clear(); out("\033[?6h\033[5;10r\033[1;1HO"); check("origin mode CUP relative to region", cell(4,0)=='O');
  out("\033[?6l\033[r");
  printf("%s (%d failures)\n", fails?"FAIL":"PASS", fails);
  return fails != 0;
}
