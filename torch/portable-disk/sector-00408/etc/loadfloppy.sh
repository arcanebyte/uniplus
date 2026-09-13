if yorn "LOADFLOPPY" "Put disc in drive. Ready ?"
then
while :
do
	# ensure no /toberun files accidently left on disc
	rm -f /toberun
	/etc/dtype /dev/rfloppy
	fstype="$?"
	case $fstype in
		0)	# bad format or floppy not loaded
			if yorn "LOADFLOPPY" "Please check that the floppy to be read has been correctly inserted into the floppy disc drive. Try Again ?"
			then
				continue
			else
				exit
			fi
			;;

		1)	# non formatted
			if yorn "LOADFLOPPY" "Please check that the floppy to be read has been formatted. Try Again ?"
			then
				continue
			else
				exit
			fi
			;;
		2)	# Empty disc
			if yorn "LOADFLOPPY" "Please check that the floppy to be read has information on it. Try Again ?"
			then
				continue
			else
				exit
			fi
			;;

		3)	# cpio  format
			echo "CPIO format disc"
			cpio -iBdcuv </dev/rfloppy 
			if [ -f /toberun ]
			then 
				# file to be executed then removed
				echo "Special Installation"
				echo "===================="
				echo
				/toberun
				rm /toberun
			fi
			break
			;;

		4)	# UNIX fs  format
			echo "SYSTEM V Filesystem format"
			if mount /dev/floppy /mnt -r >/dev/null 2>&1
			then

			if [ -f /mnt/toberun ]
			then 
				# file to be executed only
				echo "Special Installation"
				echo "===================="
				echo
				/mnt/toberun
			else
				cd /mnt
				find . -print|cpio -pdluv /
				cd 
			fi

			else
				echo "\tCannot mount the UNIX filesystem on the floppy. Please check."
			fi
			umount /dev/floppy >/dev/null 2>&1
			break
			;;

		5)	# assumed tar  format
			echo "TAR format assumed"
			tar xvf /dev/rfloppy
			if [ -f /toberun ]
			then 
				# file to be executed then removed
				echo "Special Installation"
				echo "===================="
				echo
				/toberun
				rm /toberun
			fi
			break
			;;

		*)	# Unexpected type
			echo unexpected type of disc ??
			;;
	esac
	if yorn "LOAD" "Another Disc ?"
	then
		continue
	else
		break
	fi
done
fi
