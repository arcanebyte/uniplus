#define BLKSIZE 1024
char module[] = "bcopy";		/* module name -- used by trap */
char buffer[BLKSIZE];
main()
{
	int c, o, i;
	char buf[50];

	printf("%s\n",module);
	do {
		printf("Infile: ");
		gets(buf);
		i = open(buf, 0);
	} while (i <= 0);
	do {
		printf("Outfile: ");
		gets(buf);
		o = open(buf, 1);
	} while (i <= 0);

	while ((c = read(i, buffer, BLKSIZE)) > 0)
		write(o,buffer, c);
	exit(0);
}
