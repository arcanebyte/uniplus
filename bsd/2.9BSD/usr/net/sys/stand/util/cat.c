char module[] = "cat";		/* module name -- used by trap */
main()
{
	int c, i;
	char buf[50];

	printf("%s\n",module);
	do {
		printf("File: ");
		gets(buf);
		i = open(buf, 0);
	} while (i <= 0);

	while ((c = getc(i)) > 0)
		putchar(c);
	exit(0);
}
