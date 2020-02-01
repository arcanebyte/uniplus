# Kermit

C-Kermit is the most portable and long-lived of all Kermit programs, and is
provided here for Apple Lisa users running the UniSoft UniPlus+ UNIX
distribution.

To compile, you will need to copy these files to the Lisa while running UNIX.
You will likely be limited to using UUCP over serial. The UniPlus+ UNIX
distribution included with the Sapient Technologies Quad-Port Serial Expansion
Board includes a working UUCP configuration. A future blog post at
[arcanebyte.com](https://www.arcanebyte.com) will highlight the steps needed
to get things working.

When compiling, use the following command:

	make sys3nid

After 10-15 minutes, you'll find the `wermit` application. Running `wermit`
should invoke C-Kermit:

	C-Kermit, 4E(072) 24 Jan 89, AT&T System III/System V
	Type ? for help
	C-Kermit>

