
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

extern int errno;

main(argc, argv)
char **argv; {
	int fd;

	if (argc != 2) {
		fprintf(stderr, "Usage: lock name\n");
		exit(1);
	}
	while ((fd = open(argv[1], O_RDWR|O_CREAT|O_EXCL, 0666)) == -1) {
		if (errno != EEXIST)
			break;
		sleep(10);
	}
	if (fd == -1) {
		perror(argv[1]);
		exit(1);
	}
	close(fd);
	exit(0);
}

