clear
trap : 2
echo "\n\n\tThis is the Triple X Software Loader Program."
echo
echo "\n\tDo you wish to overwrite any newer files with same name(s) ? (y/n) \c"
read answer
if [ $answer = "y" ]
then overwr="overwr"
else overwr="confirm"
fi
echo "\n\tDo you wish to list files as they are loaded (y/n) ? \c"
read answer
if [ $answer = "y" ]
then list="long"
else list="none"
fi
echo "\n\n\tFor each disc to be loaded, press 'l' followed by the return key."
echo "\tTo stop loading, press any other key followed by return key"
echo "\n\n\tPress "l" followed by return to continue \c"
read key
while [ "$key" = "l" ]
do
	loadfloppy.sh "$list" "$overwr"
	echo "\n\n\tPress "l" followed by return to continue \c"
	read key
done
