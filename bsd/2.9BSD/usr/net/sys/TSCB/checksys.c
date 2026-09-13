/*
 * checksys
 *	checks the system size and reports any limits exceeded.
 */

#include "param.h"
#include <a.out.h>
#include <stdio.h>
#include <sys/tty.h>

#define	KB	* 1024

/*
 *  Round up to a click boundary.
 */
#define cround(bytes)	((bytes + ctob(1) - 1) / ctob(1) * ctob(1));

struct exec obj;
struct ovlhdr	ovlhdr;

struct nlist nl[] =
{
	"_end", 0, 0,
#define	N_END		0
	"_remap_a", 0, 0,
#define	N_REMAPAREA	1
	"NOKA5", 0, 0,
#define	N_NOKA5		2
	"_nbuf", 0, 0,
#define	N_NBUF		3
	"_bsize", 0, 0,
#define	N_BSIZE		4
	"UCB_CLIST", 0, 0,
#define	N_CLIST		5
	"_nclist", 0, 0,
#define	N_NCLIST	6
#ifdef  UCB_NET
	"_nlsize", 0, 0,
#define N_NLSIZE        7
	"_mbsize", 0, 0,
#define N_MBSIZE        8
#endif
	0
};

char *file;
int fi;

main(argc,argv)
int argc;
char **argv;
{
	register i;
	long size, totsize;
	int errs = 0;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s unix-binary\n", argv[0]);
		exit(20);
	}
	file = argv[1];
	if ((fi=open(file,0)) < 0)
	{
		perror(file);
		exit(20);
	}
	if (read(fi,&obj,sizeof(obj)) != sizeof(obj))
	{
		printf("%s is not an object file.\n", file);
		close(fi);
		exit(20);
	}
	if (obj.a_magic == A_MAGIC5 || obj.a_magic == A_MAGIC6)
		if (read(fi,&ovlhdr,sizeof(ovlhdr)) != sizeof(ovlhdr)) {
			printf("%s is not an object file.\n", file);
			close(fi);
			exit(20);
		}
	switch(obj.a_magic)
	{

	/*
	 *	0407-- nonseparate I/D "vanilla"
	 */
	case A_MAGIC1:
		size = (long)obj.a_text + obj.a_data + obj.a_bss;
		if (size > (unsigned) 48 KB) {
			printf("Total size too large by %D bytes.\n",
				size - (unsigned) 48 KB);
			errs++;
		}
		totsize = cround(size);
		break;

	/*
	 *	0411-- separate I/D
	 */
	case A_MAGIC3:
		size = (long) obj.a_data + obj.a_bss;
		if (size > (unsigned) 48 KB) {
			printf("Data too large by %D bytes.\n",
				size - (unsigned) 48 KB);
			errs++;
		}
		totsize = obj.a_text + cround(size);
		break;

	/*
	 *	0430-- overlaid nonseparate I/D
	 */
	case A_MAGIC5:
		if (obj.a_text > 16 KB) {
			printf("Base segment too large by %u bytes.\n",
				obj.a_text - 16 KB);
			errs++;
		}
		if (obj.a_text <= 8 KB) {
			printf("Base segment too small by %u bytes.\n",
				8 KB - obj.a_text);
			errs++;
		}
		size = (long) obj.a_data + obj.a_bss;
		if (size > 24 KB) {
			printf("Data too large by %D bytes.\n", size - 24 KB);
			errs++;
		}
		/*
		 *  Base and overlay 1 occupy 16K and 8K of physical
		 *  memory, respectively, regardless of actual size.
		 */
		totsize = 24 KB + cround(size);
		/*
		 *  Subtract the first overlay, it will be added below
		 *  and it has already been included.
		 */
		totsize -= ovlhdr.ov_siz[0];
		goto checkov;
		break;

	/*
	 *	0431-- overlaid separate I/D
	 */
	case A_MAGIC6:
		if (obj.a_text > (unsigned) 56 KB) {
			printf("Base segment too large by %u bytes.\n",
				obj.a_text - (unsigned) 56 KB);
			errs++;
		}
		if (obj.a_text <= (unsigned) 48 KB) {
			printf("Base segment too small by %u bytes.\n",
				(unsigned) 48 KB - obj.a_text);
			errs++;
		}
		size = (long)obj.a_data + obj.a_bss;
		if (size > (unsigned) 48 KB) {
			printf("Data too large by %D bytes.\n",
				size - (unsigned) 48 KB);
			errs++;
		}
		totsize = (long)obj.a_text + cround(size);
checkov:
		for (i=0; i<NOVL; i++) {
			totsize += ovlhdr.ov_siz[i];
			if (ovlhdr.ov_siz[i] > 8 KB) {
			    printf("Overlay %d too large by %u bytes.\n",
				i, ovlhdr.ov_siz[i] - 8 KB);
			    errs++;
			}
		}
		break;

	default:
		printf("Magic number not recognized.\n");
		close(fi);
		exit(20);
	}

	nlist(file, nl);
	if (nl[N_NOKA5].n_type == 0) {
		fprintf(stderr, "Symbols not found in namelist\n");
		exit(20);
	}
	if (nl[N_NOKA5].n_value == 0) {
		if (nl[N_REMAPAREA].n_value >= 0120000) {
printf("The remapping area (0120000-0140000, KDSD5)\n");
printf("contains data other than the proc, text and file tables.\n");
printf("Reduce other data by %u bytes.", nl[N_REMAPAREA].n_value - 0120000);
			errs++;
		}
	} else {
		if (nl[N_END].n_value >= 0120000) {
printf("Data extends into the remapping area (0120000-0140000, KDSD5)\n");
printf("by %u bytes; undefine NOKA5 or reduce data size.\n", 
		nl[N_END].n_value - 0120000);
			errs++;
		}
	}

	totsize += cround((long) getval(N_NBUF) * (long) getval(N_BSIZE));
	if (nl[N_CLIST].n_value)
		totsize += cround((long) getval(N_NCLIST)
			* (long) sizeof(struct cblock));
	totsize += ctob(USIZE);
#ifdef  UCB_NET
	totsize += cround((long) getval(N_MBSIZE));
	totsize += getval(N_NLSIZE);
#endif
printf("System will occupy %D bytes of memory (including buffers and clists).\n",
		totsize);

	close(fi);
	if (errs)
		printf("**** SYSTEM IS NOT BOOTABLE.\n");
	exit(errs);
}

#define round(x) (ctob(stoc(ctos(btoc(x)))))
/*
 *  Get the value of an initialized variable from the object file.
 */
getval(index)
{
	int ret;
	off_t offst;

	offst = (off_t)nl[index].n_value
		+ (off_t) obj.a_text + sizeof(obj);
	if (obj.a_magic == A_MAGIC2 || obj.a_magic == A_MAGIC5)
		offst -= (off_t)round(obj.a_text);
	if (obj.a_magic == A_MAGIC5 || obj.a_magic == A_MAGIC6) {
		register i;
		offst += sizeof ovlhdr;
		if (obj.a_magic == A_MAGIC5)
			offst -= (off_t)round(ovlhdr.max_ovl);
		for(i=0; i<NOVL; i++)
			offst += (off_t)ovlhdr.ov_siz[i];
	}
	lseek(fi, offst, 0);
	read(fi, &ret, sizeof(ret));
	return(ret);
}
