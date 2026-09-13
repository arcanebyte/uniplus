nofunc()
{
	int	fd = open("/dev/tty", 0);

	if (fd != -1)
		write(fd, "Bad entry in jump table\n", 24);
	_exit(1);
}
