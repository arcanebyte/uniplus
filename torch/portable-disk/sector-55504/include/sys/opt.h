/* @(#)opt.h	1.4 */
#if MESG==0
msgsys(){nosys();}
msginit(){return(0);}
#endif

#if SEMA==0
semsys(){nosys();}
seminit(){}
semexit(){}
#endif

#if SHMEM==0
shmsys(){nosys();}
shmexec(){}
shmexit(){}
shmfork(){}
shmreset(){}
#endif
