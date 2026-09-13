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

main(){
int i;

	openpl();

	for(i =0; i < 32; i++){
		SetFPat(i);
		printf("colour %d\r\n",i);
	}

	closepl();
}

