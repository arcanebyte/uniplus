/* Used when the system halts to drop back into the rom debugger
 */

typedef int (*pfri)();
#define ROMADDR ((pfri)(0xFE0084))

/* ARGSUSED */
rom_mon(icon, msg, errcode)
	long *icon;
	char *msg;
	short errcode;
{
	register char *a5, *a4, *a3;
	register long *a2;
	register short d7;
	a2 = 0; /* icon; */
	a3 = msg;
	d7 = errcode;
	asm(" movw d7,d0 ");
	asm(" movl 0x1000,sp ");
	ROMADDR();
}
