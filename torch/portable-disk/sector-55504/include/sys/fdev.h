
/*
 * this is the header file for the SCSI control driver ioctl.
 * ioctl(scsifd, F_DEV_CONV, &fdevinst)
 * int scsifd;
 * struct fdevstruct fdevinst;
 */
#define	F_IOCTL		('f' << 8)
#define F_DEV_CONV	(F_IOCTL | 1)
#define F_DEV_COMM	(F_IOCTL | 2)
#define F_DEV_UNPROT	(F_IOCTL | 3)


struct fdevstruct {
	ushort f_mode;
	dev_t f_dev;
	char f_dest;
	char f_lun;
	char f_type;
};

