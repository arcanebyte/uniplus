clear
trap : 2
echo "\n\n\tThis is the Triple X Software Listing Program."
echo
echo "\n\tDo you wish to list the files in long format (y/n) ? \c"
read answer
if [ $answer = "y" ]
then listtype="long"
else listtype="short"
fi
echo "\n\n\tFor each disc to be listed, press 'l' followed by the return key."
echo "\tTo stop listing program, press any other key followed by return key"
echo "\n\n\tPress "l" followed by return to continue \c"
read key
while [ "$key" = "l" ]
do
	listfloppy.sh $listtype
	echo "\n\n\tPress "l" followed by return to continue \c"
	read key
done
