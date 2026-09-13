/*
 * nic-hosts (hnames)
 *	This program queries the NIC host database for host information
 *	The host server protocol is described in RFC-811
 */

#include <stdio.h>
#include <ctype.h>
#ifndef NCP
#include <sys/types.h>
#include <sys/socket.h>
#include <net/in.h>
#ifndef IPPORT_NIC
#define IPPORT_NIC 101
#endif
struct sockaddr_in hisctladdr = { AF_INET, IPPORT_NIC };
struct sockaddr_in myctladdr;
#else
#include <sys/netopen.h>
struct nopenparams openparams;

#define ICPSOCK 101	/* host server is socket 101 on NIC */
#endif NCP

char *hostname = "sri-nic";

/*	command definitions	*/

#define HNAME	1
#define HALL	2
#define HADDR	3

char *format[] = {
	"\n",
	"HNAME %s\r\n",
	"ALL\r\n",
	"HADDR %s\r\n",
};


FILE *netbuf;
FILE *netin;
char ttbuffer[BUFSIZ];
int com = NULL;

main(argc, argv)
int argc;
char *argv[];
{
#ifndef NCP
	long hnum, rhost ();
#else
	long int hnum, gethost();	/* number of foreign host */
#endif
	register int netfd;
	char c;                /* Character */


	if(argc < 2)
	  {	printf("Usage: %s [-a][-x][-n] keyword\n", argv[0]);
		exit(1);
	  }

	/* determine any special options */

	if (argc > 1 && *argv[1] == '-'){
		switch(argv[1][1]){
		case 'a':		/* This is possibly a host address */
			com = HADDR;
			break;
		case 'n':		/* This is possibly a host name    */
			com = HNAME;
			break;
		case 'x':		/* Want all hosts and related info */
			com = HALL;
			break;
		default:
			fprintf(stderr, "Unknown option: %s\n", argv[1]);
			exit(1);
		}
		argv++;
	}

	/*
	 * The above arguments are obsolete, but are maintained for completeness
	 * The following 2 lines determine what is really wanted
	 */
	if (com == NULL) com = decide(argv[1]);		/* command type */
	if (com == HADDR) argv[1] = fixhostnum(argv[1]);/* correct host number */

	setbuf(stdout,ttbuffer);	/* buffer all output */

#ifndef NCP
	hnum = rhost (&hostname);
#else
	hnum = gethost(hostname);
#endif
	if (hnum == -1L) {		/* in the event NIC drops from table */
		fprintf(stderr, "Unknown host:%s\n", hostname);
		exit(2);
	}

#ifndef NCP
	hisctladdr.sin_addr.s_addr = hnum;
#if vax || pdp11
	hisctladdr.sin_port = htons(hisctladdr.sin_port);
#endif
	netfd = socket(SOCK_STREAM, 0, (struct sockaddr *)0, 0);
	if (netfd < 0) {
		perror("socket");
		exit(1);
	}
	if (connect(netfd, &hisctladdr)) {
		perror("connect");
		exit(1);
	}
	if (socketaddr(netfd, &myctladdr) < 0) {
		perror("socketaddr");
		exit(1);
	}
#else NCP
	openparam.o_fskt = ICPSOCK;
	openparam.o_host = hnum;
	openparam.o_timeo = 30;
	if (  (netfd = open("/dev/network", &openparam)) == -1)
	  {	fprintf(stderr, "Can't connect\n");
		exit(1);
	  }
#endif NCP
	fprintf(stderr, "NIC Host Server:\n");
	netbuf = fdopen(netfd, "w");
	netin = fopen("/dev/null", "r");
	close(fileno(netin));
	fileno(netin) = fileno(netbuf);
	fflush(stdout);

	fprintf(netbuf,format[com], argv[1]);
	fflush(netbuf);

	while ((c=getc(netin)) != EOF)
		if (c != '\r')
			putchar(c);
	putchar('\n');
	fflush(stdout);
	if (ferror (netin))
	{
		perror ("net input error");
		exit (-1);
	}
	if (ferror (stdout))
	{
		perror ("output error");
		exit (-1);
	}
	exit (0);
}

/*
 * decide parses an argument to determine which command is most appropriate
 * to use for the query:
 *	anything starting with a digit is a host number
 *	"*" or the word "all" requests all hosts
 *	any other case must be a host name
 */
decide(str)
char *str;
{
  int type = HNAME;	/* default type */

  if (strcmp(str, "*") == 0 || strcmp(str, "all") == 0) type = HALL;
  else if (isdigit(*str))    type = HADDR;
  return type;
}

/*
 * fixhostnum will put old host numbers in proper format
 * a "/" indicates old format
 */
fixhostnum(str)
char *str;
{
  char *newstr, *start, *end, *net;
  static buffer[32];
  int host, imp;

  newstr = str;

  if (end=index(str, '/')){	/* old style */
	start = str;
	*end++ = '\0';
	if ((net = index(end,','))==NULL) net = "10";
	else *net++ = '\0';
	host = atoi(start);
	imp = atoi(end);
	sprintf(buffer, "%s.%d.0.%d", net, host, imp);
	newstr = buffer;
  }
  return newstr;
}
