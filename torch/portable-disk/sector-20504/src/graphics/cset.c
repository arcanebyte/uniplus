			/* Version Numbers - External */
static char version [] =
   "(C) Copyright 1985 TORCH Computers Ltd Version 0.10\nTriple X";

			/* Version Numbers Records */
/*
 * Version	- 	Reason for change.
 *
 *   0.10		Release for Beta Test to KRN 13/9/85
 *
 */

#include <wlib.h>

#define NORMALMODE 0
#define INVERSEMODE 1
#define BOLDMODE 2
#define ULINEMODE 3
#define ITALICSMODE 4

char usage[] = "Usage:\tcset [mode]...\n";

main(argc,argv)
int argc;
char *argv[]; {
	int i,argn;

	static struct {
		char *name;
		int func;
	} modes[] = {
		"normal",	NORMALMODE,
		"inverse",	INVERSEMODE,
		"bold",		BOLDMODE,
		"underline",	ULINEMODE,
		"italics",	ITALICSMODE,
		NULL,		NULL
	};

	if (argc < 2) {
		printf(usage);
			printf("\nAvailable modes are:-\n\n");
		for (i =0; modes[i].name; i++)
			printf("\t%s\n",modes[i].name);
		exit();
	}

	for (argn = 1;  argn < argc; argn++) {
					/* find mode code */
		for (i = 0; modes[i].name; ++i)
			if ( strcmp(modes[i].name,argv[argn]) == 0)
				break;

		if (modes[i].name)
			switch (modes[i].func){
			case NORMALMODE:  NormText(); break;

			case INVERSEMODE: Inverse(TOGGLE); break;

			case BOLDMODE:    Bold(TOGGLE); break;

			case ULINEMODE:   ULine(TOGGLE); break;

			case ITALICSMODE: Italic(TOGGLE); break;
			}
		else
			printf("%s: Mode %s unknown\n",argv[0],argv[argn]);
	}
}

