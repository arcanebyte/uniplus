main (argc, argv)
{
  	printf ("%d; %d\n", getuid (), geteuid());
	seteuid (1);
  	printf ("%d; %d\n", getuid (), geteuid());
	seteuid (0);
  	printf ("%d; %d\n", getuid (), geteuid());
	setruid (1);
  	printf ("%d; %d\n", getuid (), geteuid());
	setruid (0);
  	printf ("%d; %d\n", getuid (), geteuid());
}
