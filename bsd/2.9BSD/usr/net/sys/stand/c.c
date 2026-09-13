#include <sys/param.h>
#include <sys/inode.h>
#include <saio.h>

devread(io)
register struct iob *io;
{

	return( (*devsw[io->i_ino.i_dev].dv_strategy)(io,READ) );
}

devwrite(io)
register struct iob *io;
{
	return( (*devsw[io->i_ino.i_dev].dv_strategy)(io, WRITE) );
}

devopen(io)
register struct iob *io;
{
	(*devsw[io->i_ino.i_dev].dv_open)(io);
}

devclose(io)
register struct iob *io;
{
	(*devsw[io->i_ino.i_dev].dv_close)(io);
}

nullsys()
{ ; }

int	xpstrategy();
int	rpstrategy();
int	rkstrategy();
int	hkstrategy();
int	rlstrategy();
int	nullsys();
int	tmstrategy(), tmrew(), tmopen();
int	htstrategy(), htopen(), htclose();
int	tsstrategy(), tsopen(), tsclose();
struct devsw devsw[] {
	"xp",	xpstrategy,	nullsys,	nullsys,
	"rp",	rpstrategy,	nullsys,	nullsys,
	/* "hp" and "rm" are synonyms for "xp" */
	"hp",	xpstrategy,	nullsys,	nullsys,
	"rm",	xpstrategy,	nullsys,	nullsys,
	"rk",	rkstrategy,	nullsys,	nullsys,
	"hk",	hkstrategy,	nullsys,	nullsys,
	"rl",	rlstrategy,	nullsys,	nullsys,
	"tm",	tmstrategy,	tmopen,		tmrew,
	"ht",	htstrategy,	htopen,		htclose,
	"ts",	tsstrategy,	tsopen,		tsclose,
	0,0,0,0
};
