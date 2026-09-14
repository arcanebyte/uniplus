# The Lisa console under UniPlus+

The Lisa's own screen and keyboard are the `console` device (`v1.5/sys/co.c`). Output goes through a VT100 emulator (`vt100.c`) that draws 88×38 characters of 8×9 pixels on the 720×364 bitmap (`bm.c`), inside a one-character border. The keyboard driver (`kb.c`) sends VT100 codes for the arrow keys and keypad.

## What was wrong

- **No tab stops until something set them**, so a tab went to the right edge.
- **Unknown escape sequences left characters on the screen.** `ESC [ ? 25 l` and `ESC ( B` both printed their last character.
- **Wrapping was immediate.** Writing column 88 moved to the next line at once.
- **Backspace stopped at column 0**, so erasing input that had wrapped onto a second line left the first line's tail behind.
- **`ESC M` at the top of the screen scrolled down without blanking the new top line.**
- **No scrolling regions, insert mode, saved cursor, cursor visibility or replies to terminal queries.**

Two things are the shell and tty driver, not the emulator:
- **Arrow keys:** `/bin/sh` has no history, so up-arrow's `ESC [ A` is echoed by the tty driver and moves the cursor. Use csh's history (`!!`, `!vi`, `^old^new`) instead.
- **The empty line:** System V's `ECHOE` echoed backspace-space-backspace for every erase, even with nothing left to erase, so it wiped the prompt (and, once the console wrapped backwards, the text above). `tt1.c` now checks the line first; see below.

## The reworked emulator (September 2026)

`vt100.c` is now one state machine. It keeps UniSoft's drawing code and adds more VT100 behaviour, with some of xterm's:

| Area | Now |
|---|---|
| Tabs | Stops every 8 columns at power-on (and after `reinit`), as on a VT100; `ESC H`, `ESC [ g`, `ESC [ 3 g` as before |
| Wrapping | Deferred: the cursor stays on column 88 until the next printable character (termcap `xn`); `ESC [ ? 7 l` turns wrapping off |
| Backspace | Reverse wrap: at column 0 it goes to the end of the line above, and just after a deferred wrap it stays put, so erasing wrapped input works (termcap `bw`) |
| Scrolling | Regions with `ESC [ top ; bottom r`, origin mode `ESC [ ? 6 h`; line feed, `ESC D`, `ESC M`, insert and delete line, `ESC [ S` / `T` all respect the region; `ESC M` blanks the new top line |
| Editing | `ESC [ @` insert characters, `ESC [ X` erase characters, insert mode `ESC [ 4 h` / `l`, plus the existing `ESC [ P`, `L`, `M`, `J`, `K` |
| Cursor | `ESC 7` / `ESC 8` and `ESC [ s` / `ESC [ u` save and restore (with attributes); `ESC E` next line; `ESC [ E` / `F` next/previous line; `ESC [ G` or `` ` `` column; `ESC [ d` row; `ESC [ a` / `e` relative; `ESC [ ? 25 l` / `h` hide and show |
| Attributes | `ESC [ 7 m` (and `1`, bold) reverse, `4` underline, `0` off as before; now also `22`/`27` reverse off and `24` underline off |
| Keyboard | `ESC [ ? 1 h` makes the arrow keys send `ESC O A`…; `ESC =` keypad mode as before |
| Replies | `ESC [ c` and `ESC Z` answer `ESC [ ? 1 ; 2 c` (VT100 with advanced video); `ESC [ 5 n` answers `ESC [ 0 n`; `ESC [ 6 n` reports the cursor position. Replies arrive as keyboard input |
| Parsing | Any number of parameters, private markers and intermediate characters are read to the end of a sequence and ignored if unknown; `ESC (`, `ESC )`, `ESC *`, `ESC +`, `ESC #` swallow their next character; CAN (^X) and SUB (^Z) abandon a sequence; `ESC c` resets the terminal |

Not emulated: 132 columns, double-width lines, the line-drawing character set (the Lisa font has none), blink, reverse screen (`ESC [ ? 5 h`), and separate bold (bold shows as reverse, as before).

**Erase on an empty line (`tt1.c`, and `tt0.c`):** with `echoe`, an erase is only echoed when the input line has a character left to remove, as Berkeley's tty driver did. The driver echoes when a character arrives but erases when the line is read, so `ttrubok()` replays the raw input queue (newline, EOF, EOL and the kill character start a new line; an erase removes one character; a character after `\` is literal). This works on every terminal, not just the console.

Files changed: `vt100.c`; `kb.c` and `s_kb.c` (`kb_ckm`, application cursor keys); `reinit.c` (default tabs instead of none); `tt1.c` (erase echo; the kernel Makefile sets `TTY = 1`, so `tt1.c` is the one built) and the same change in `tt0.c`.

**Testing:** `tools/vt100test/run.sh` builds `vt100.c` on the Mac against a character-grid model of the `bm.c` routines and checks wrapping, reverse wrap and erase, tabs, unknown sequences, cursor visibility, scrolling regions, insert and delete line and character, insert mode, saved cursor, attributes, the replies, origin mode and reset. Run it after changing `vt100.c`. On the Lisa it needs a kernel rebuild (`rm -f net.o unix.net` first).

## termcap

Replace the `vtl` entry in `/etc/termcap` so programs use the new features:

```
# Partial VT100 emulation used on bitmap display under UniPlus+ on the Apple Lisa.
dx|vtl|vtlisa|Lisa bitmap display:\
	:co#88:li#38:cl=\E[H\E[J:bs:cm=\E[%i%2;%2H:nd=\E[C:up=\E[A:\
	:ce=\E[K:cd=\E[J:so=\E[7m:se=\E[m:us=\E[4m:ue=\E[m:\
	:al=\E[L:dl=\E[M:dc=\E[P:pt:sr=\EM:am:xn:bw:ms:\
	:cs=\E[%i%d;%dr:sc=\E7:rc=\E8:ho=\E[H:do=^J:sf=^J:ta=^I:\
	:im=\E[4h:ei=\E[4l:mi:vi=\E[?25l:ve=\E[?25h:\
	:is=\E>\E[r\E[?7h\E[m\E[H\E[J:rs=\Ec:ks=\E=:ke=\E>:\
	:kb=^H:ku=\E[A:kd=\E[B:kr=\E[C:kl=\E[D:kh=\E[H:k1=\EOP:k2=\EOQ:k3=\EOR:
```

The old entry's `if=/usr/lib/tabset/vt100` is gone: the tab stops are already set. Keep the reverse-video `vtlrev` entry below it, which inherits with `tc=vtl`. With the old kernel, use the old entry: `xn`, `bw`, `cs`, `sc`/`rc`, `im`/`ei` and `vi`/`ve` need the new emulator.

## Setting up a login on the console

`/etc/profile` (Bourne shell), after `readonly LOGNAME`:
```sh
if [ "`tty`" = /dev/console ]
then
	TERM=vtl
	export TERM
	tset -e^H vtl
fi
```

`/.cshrc` or `~/.cshrc` (csh):
```csh
set history=40
if (`tty` == /dev/console) then
	setenv TERM vtl
	tset -e^H vtl
endif
```

`/bin/csh` is installed, and `/etc/passwd` has a `rootcsh` account (uid 0) whose shell is csh. There is no `chsh`; edit `/etc/passwd` to change a user's shell.
