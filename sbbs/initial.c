
 /* Initial "shell" */
main()
{

  /* Fake shell name */

  execl ("/bin/sh", "sbbs", "-c", "/u/robertd/lib/sbbs/programs/login-sig", (char *) 0);
}
