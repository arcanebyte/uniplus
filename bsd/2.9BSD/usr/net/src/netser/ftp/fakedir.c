mkdir (what, mode)
char *what;
int mode;
{
	return (dircall (1, "/bin/mkdir", "mkdir", what, 0, 0));
}

rmdir (what)
char *what;
{
	return (dircall (1, "/bin/rmdir", "rmdir", what, 0, 0));
}

rename (from, to)
char *from, *to;
{
	return (dircall (1, "/bin/mv", "mv", from, to, 0));
}

static
dircall (setids, path, a, b, c, d)
int setids;
char *path, *a, *b, *c, *d;
{
	extern int errno;
	int statbuf;
	int pid;

	errno = 0;
	if ((pid = fork ()) == -1)
		return (-1);
	if (pid == 0)
	{
		register int euid, egid;

		if (setids)
		{
			euid = geteuid ();
			seteuid (0);
			setruid (euid);
			egid = getegid ();
			setegid (0);
			setrgid (egid);
		}
		close (0);
		close (1);
		execl (path, a, b, c, d);
		exit (-1);
	}
	if (wait (&statbuf) != pid || statbuf != 0)
		return (-1);
	return (0);
}
