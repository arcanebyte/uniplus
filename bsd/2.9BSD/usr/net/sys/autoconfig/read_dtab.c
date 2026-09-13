/*
 * Read the device table into internal structures
 */

#include	<stdio.h>
#include	<ctype.h>
#include	<sys/autoconfig.h>
#include	"dtab.h"
#include	"uprobe.h"

static int	line;		/* Line number in dtab file */
FILE		*dtab_fp;	/* File pointer to dtab file */
int		guess_ndev = 0;	/* Guess as to size of nlist table */

otoi(cp)
char *cp;
{
	int res;

	sscanf(cp, "%o", &res);
	return res;
}

int	last_ch;	/* last character read by getword */

#define read_while(expr) while ((ch = getc(dtab_fp)) != EOF && (expr))
char *getword()
{
	static char buf[80];
	register int ch;
	register char *cp;

	if (feof(dtab_fp))
		return NULL;
	/* First skip any white space */
skip:
	last_ch = EOF;
	read_while(isspace(ch))
		;
	if (ch == EOF)
		return NULL;
	
	/* If its a comment, skip it too */
	if (ch == '#') {
		read_while(ch != '\n')
			;
		if (ch == EOF)
			return NULL;
		goto skip;
	}
	cp = buf;
	do {
		*cp++ = ch;
		*cp = '\0';
		if ((ch = getc(dtab_fp)) == EOF)
			return buf;
	} while (!isspace(ch));
	last_ch = ch;
	return buf;
}

char *nextword()
{
	register char *cp;

	if ((cp = getword()) == NULL) {
		fprintf(stderr, "Syntax error, not enough data on line %d\n", line);
		exit(AC_SINGLE);
	}
	return cp;
}

/*
 * Format of lines in the device table are:
 *	DNAME NUM ADDR VEC BR (HANDLER ...) SEMICOLON COMMENT
 * From a '#' to end of line is also considered a comment
 */

read_dtab()
{
	char *cp;
	register struct dtab_s *dp, *cdp;
	struct handler_s *sp;
	struct uprobe *up;
	int nhandlers;

	line = 0;
	devs = NULL;
	while ((cp = getword()) != NULL) {
		line++;
		dp = malloc(sizeof *dp);
		dp->dt_name = strsave(cp);
		if (*(cp = nextword()) == '?')
			dp->dt_unit = -1;
		else
			dp->dt_unit = atoi(cp);
		dp->dt_addr = otoi(nextword());
		dp->dt_vector = otoi(nextword());
		dp->dt_br = otoi(nextword());
		dp->dt_probe = dp->dt_attach = 0;
		dp->dt_handlers = NULL;
		nhandlers = 0;
		while (strcmp((cp = nextword()), ";")) {
			if (++nhandlers == 4)
				fprintf(stderr, "Warning, more than three handlers for device %s on line %d.\n", dp->dt_name, line);
			addent(&dp->dt_handlers, strsave(cp));
			guess_ndev++;
		}
		guess_ndev += 2;
		for (up = uprobe; up->up_name; up++) {
			if (!strcmp(dp->dt_name, up->up_name)) {
				dp->dt_uprobe = up->up_func;
				break;
			}
		}
		/*
		 * Skip the rest of the line (comment field).
		 */
		while (last_ch != '\n' && last_ch != EOF)
			last_ch = getc(dtab_fp);
		dp->dt_next = NULL;
		if (devs == NULL)
			devs = cdp = dp;
		else {
			cdp->dt_next = dp;
			cdp = dp;
		}
	}
	fclose(dtab_fp);
}

addent(listp, cp)
struct handler_s **listp;
char *cp;
{
	struct handler_s *el;
	struct handler_s *sp;

	el = malloc(sizeof *el);
	el->s_str = cp;
	el->s_next = NULL;
	if (*listp == NULL)
		*listp = el;
	else {
		for (sp = *listp; sp->s_next != NULL; sp = sp->s_next)
			;
		sp->s_next = el;
	}
}

inlist(list, str)
register struct handler_s *list;
register char *str;
{
	for (; list != NULL; list = list->s_next)
		if (strcmp(list->s_str, str) == 0)
			return 1;
	return 0;
}
