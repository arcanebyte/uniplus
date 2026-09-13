/*
 * Generalized argument parser for use by people who are tired of writing
 * and rewriting their own parsers.  This is general enough for my use
 * but others might find the conventions it imposes to be too restrictive.
 * 
 * Written 3-Dec-81 by Michael Toy
 */

#include	<stdio.h>
#include	"args.h"
char	_bf[52];
char	*_sf[52];
char	**_argspace, **_ap;
int	nargs = 0;

parse_args(ac, av, bopts, fopts)
int ac;
char **av, *bopts, *fopts;
{
	int argc = ac, errcnt = 0;
	char **argv = av, *pname, **ap, *cp;

	pname = argv[0];			/* Saved for error messages */
	_ap = _argspace = ap = (char **) calloc(argc, sizeof (char *));
	/*
	 * Loop once through the argument list.  Record boolean and string
	 * flags and count the number of arguments, saving each normal arg.
	 */
	while (--argc) {
	cp = *++argv;
	/*
	 * A flag string begins with a - unless it is a -- which is
	 * a dash escape.
	 */
	if (*cp == '-' && cp[1] != '-') {
		/*
		 * Process each flag in the string.  Set boolean flags and
		 * save string flags
		 */
		while(*++cp) {
		if (index(bopts, *cp) != 0)
			bools(*cp) = 1;			/* Set bool */
		else if (index(fopts, *cp))
			if (argc > 1) {
			argc--;
			strings(*cp) = newstr(*++argv);	/* Save string */
			} else {
			fprintf(stderr,
				"%s: Argument for %c flag not specified\n",
				pname, *cp);
			errcnt++;
			}
		else {
			fprintf(stderr, "%s: No such flag as %c\n", pname, *cp);
			errcnt++;
		}
		}
	} else {
		/*
		 * It is a regular argument, just save it
		 */
		*ap++ = newstr(cp);
		nargs++;
	}
	}
	*ap++ = (char *) 0;
	return errcnt;
}

char *newstr(s)
char *s;
{
	char *cp;

	cp = (char *) calloc(1, strlen(s)+1);
	strcpy(cp, s);
	return cp;
}
