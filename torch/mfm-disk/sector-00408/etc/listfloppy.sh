#
#	5.2 version only
#
if yorn "LISTFLOPPY" "Put disc in drive. Ready ?"
then
while :
do
	/etc/dtype /dev/rfloppy
	fstype="$?"
	case $fstype in
		0)	# bad format or floppy not loaded
			if yorn "LISTFLOPPY" "Please check that the floppy to be read has been correctly inserted into the floppy disc drive. Try Again ?"
			then
				continue
			else
				exit
			fi
			;;

		1)	# non formatted
			if yorn "LISTFLOPPY" "Please check that the floppy to be read has been formatted. Try Again ?"
			then
				continue
			else
				exit
			fi
			;;

		2)	# Empty disc
			if yorn "LISTFLOPPY" "Please check that the floppy to be read has information on it. Try Again ?"
			then
				continue
			else
				exit
			fi
			;;

		3)	# cpio  format
			echo "CPIO format disc\n"
			cpio -iBctv </dev/rfloppy
			break
			;;

		4)	# UNIX fs  format
			echo "SYSTEM V Filesystem format disc\n"
			if mount -r /dev/floppy /mnt >/dev/null 2>&1
			then
				cd /mnt
				ls7 -Rl 
				cd 
			else
				echo "\tCannot mount the UNIX filesystem on the floppy. Please check."
			fi
			umount /dev/floppy >/dev/null 2>&1
			break
			;;

		5)	# assumed tar  format
			echo "TAR format disc ?"
			tar tvf /dev/rfloppy
			break
			;;

		*)	# Unexpected type
			echo unexpected type of disc ??
			;;
	esac
	if yorn "LIST" "Another Disc ?"
	then
		continue
	else
		break
	fi
done
fi
