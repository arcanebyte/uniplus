if [ -d /usr/start ]
then
	HOME=/usr/start
	export HOME
	cd
else
	echo	"/start/.profile: unable to find /usr/start"
	echo	"\t\tusing HOME=/start instead"
fi
if chwdown 2> /dev/null
then
	# we are logged in to a window device
	controlpanel -l
	if [ -x /bin/dm -a -x /bin/spawner ]
	then
		# if do want the machine to switch off when the Exit option
		# of the desktop is used, remove exec from the line below
		# N.B. /etc/powerfail needs super user permission to execute
		exec /bin/dm < /dev/null > /dev/null 2>&1
		exec /etc/powerfail
	fi
	echo "/start/.profile: unable to find /bin/dm or /bin/spawner" 1>&2
fi
# we are either not logged into a window device or dm is not executable
# drop out into a normal shell
