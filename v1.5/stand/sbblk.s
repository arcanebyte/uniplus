| Apple Sony reset-booter
| Read in blocks 1 through 100.

FCADDR	= 0xFCC001		| floppy controller addr
MON	= 0x84			| rom monitor
READ	= 0x94			| rom read Sony routine offset
DSPLY	= 0x88			| display routine
USTART	= 0x60000		| load address

TRKS	= 9			| tracks to read (actually read 108 blocks)
SECS	= 12			| sectors per track
| d1 contains 0xDDSSBBTT where:
|	DD is drive - always 0x80 for Sony
|	SS is side - always 0x00 for Sony
|	BB is sector
|	TT is track
DRIVE	= 0x80000000		| drive 1

.globl	_snblk0
_snblk0:
	movl	#0xFE0000,a4	| rom addr
|	movl	#.one,a3	| DEBUG message
|	movl	#0,d5		| DEBUG message
|	movl	#0,d6		| DEBUG message
|	jsr	a4@(DSPLY)	| DEBUG message
	movl	#USTART,a1	| where we want to load booter
	movl	a1,a2		| addr to pass to rom
	subw	#20,a1		| where rom will load header

	moveq	#SECS-2,d3	| sectors per track (don't read sector 0)
	moveq	#TRKS-1,d4	| number of tracks to read
	clrl	d1		| starting track
	orl	#DRIVE,d1	| drive 1, side 0
	addl	#0x100,d1	| start with sector 1, track 0
.1$:	movl	#FCADDR,a0	| setup address of controller
	movl	#0xFFFFF,d2	| timeout count
	clrl	d0
	jsr	a4@(READ)	| rom read routine
	tstb	d0		| test results
	bne	.2$		| print err message and exit
	addl	#512,a2		| update data pointer
	addl	#0x100,d1	| next sector
	dbra	d3,.1$		| continue 'til SECS have been read
|	movl	#.two,a3	| DEBUG message
|	jsr	a4@(DSPLY)	| DEBUG message
	moveq	#SECS-1,d3	| reload sectors per track
	andl	#0xFFFF00FF,d1	| reset sector number
	addqb	#1,d1		| next track
	dbra	d4,.1$		| continue 'til TRKS have been read
|	movl	#.three,a3	| DEBUG message
|	jsr	a4@(DSPLY)	| DEBUG message
	jmp	USTART
.2$:	jmp	a4@(MON)	| rom monitor

|	.data
|.one:	.byte	061
|	.byte	0
|.two:	.byte	062
|	.byte	0
|.three:	.byte	063
|	.byte	0
