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

#include "drawdefs.h"
#define NULL 0

char usage[] = "Usage:\tf colour\n\tb colour\n";

main(argc,argv)
int argc;
char *argv[]; {
	int i;
	static struct {
		char *name,code;
	} colours[] = {
	"darkblue",	DARKBLUE,
	"white",	WHITE,
	"greengrey",	GREEN_GREY,
	"redgreen",	RED_GREEN,
	"darkred",	DARKRED,
	"grey",		GREY,
	"brokengrey",	BROKENGREY,
	"vlightgrey",	VLIGHTGREY,
	"speclegrey",	SPECLEGREY,
	"darkgreen",	DARKGREEN,
	"pink",		PINK,
	"cherryred",	CHERRYRED,
	"muddyyellow",	MUDDYYELLOW,
	"green",	GREEN,
	"halfgrey",	HALFGREY,
	"limegreen",	LIMEGREEN,
	"salmon",	SALMON,
	"beige",	BEIGE,
	"purple",	PURPLE,
	NULL,		NULL
	};

	if (argc < 2) {
		printf(usage);
			printf("\nAvailable colours are:-\n\n");
		for (i =0; colours[i].name; i++)
			printf("\t%s\n",colours[i].name);
		exit();
	}

					/* find colour code */
	for (i = 0; colours[i].name; ++i)
		if ( strcmp(colours[i].name,argv[1]) == 0)
			break;

	if (colours[i].name)
		switch (argv[0][0]) {
			case 'f': SetFPat(colours[i].code);
				  break;
			case 'b': SetBPat(colours[i].code);
				  break;
			default:
				printf(usage);
				exit();
		}
	else
		printf("%s: Colour(s) %s unknown\n",argv[0],argv[1]);
}

