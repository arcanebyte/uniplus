#
# _Version_ = (C) Copyright 1987 UniSoft Corp. Version V.2.1.6
# _Origin_ = Sun Microsystems  NFSSRC 2.1 86/04/16
# sccsid = @(#)make.script	UniPlus V.2.1.6 (Sun 2.1)
#
DIR =/etc
DOM = `domainname`
NOPUSH = ""
ALIASES = /usr/lib/aliases
SHELL=/bin/sh

all: passwd group hosts ethers networks rpc services protocols \
	netgroup aliases

passwd.time: $(DIR)/passwd
	-@PATH=.:$$PATH ; \
	if [ -f $(DIR)/passwd ]; then \
		awk 'BEGIN { FS=":"; OFS="\t"; } /^[a-zA-Z0-9_]/ { print $$1, $$0 }' \
		  $(DIR)/passwd  | makedbm - /etc/yp/$(DOM)/`yplocalname passwd.byname`; \
		awk 'BEGIN { FS=":"; OFS="\t"; } /^[a-zA-Z0-9_]/ { print $$3, $$0 }' \
		  $(DIR)/passwd | makedbm - /etc/yp/$(DOM)/`yplocalname passwd.byuid`; \
		touch passwd.time; \
		echo "updated passwd"; \
		if [ ! $(NOPUSH) ]; then \
			/etc/yp/yppush `yplocalname passwd.byname`; \
			/etc/yp/yppush `yplocalname passwd.byuid`; \
			echo "pushed passwd"; \
		else \
			: ; \
		fi \
	else \
		echo "couldn't find $(DIR)/passwd"; \
	fi

group.time: $(DIR)/group
	-@PATH=.:$$PATH ; \
	if [ -f $(DIR)/group ]; then \
		awk 'BEGIN { FS=":"; OFS="\t"; } { print $$1, $$0 }' \
		    $(DIR)/group | makedbm - /etc/yp/$(DOM)/`yplocalname group.byname`; \
		awk 'BEGIN { FS=":"; OFS="\t"; } { print $$3, $$0 }' \
		    $(DIR)/group | makedbm - /etc/yp/$(DOM)/`yplocalname group.bygid`; \
		touch group.time; \
		echo "updated group"; \
		if [ ! $(NOPUSH) ]; then \
			/etc/yp/yppush `yplocalname group.byname`; \
			/etc/yp/yppush `yplocalname group.bygid`; \
			echo "pushed group"; \
		else \
			: ; \
		fi \
	else \
		echo "couldn't find $(DIR)/group"; \
	fi

hosts.time: $(DIR)/hosts
	-@PATH=.:$$PATH ; \
	if [ -f $(DIR)/hosts ]; then \
		sed -e "/^#/d" -e s/#.*$$// $(DIR)/hosts | /etc/yp/stdhosts \
		    | awk '{for (i = 2; i <= NF; i++) print $$i, $$0}' \
		    | makedbm - /etc/yp/$(DOM)/`yplocalname hosts.byname`; \
		/etc/yp/stdhosts $(DIR)/hosts | \
		    awk 'BEGIN { OFS="\t"; } $$1 !~ /^#/ { print $$1, $$0 }' \
		    | makedbm - /etc/yp/$(DOM)/`yplocalname hosts.byaddr`; \
		touch hosts.time; \
		echo "updated hosts"; \
		if [ ! $(NOPUSH) ]; then \
			/etc/yp/yppush `yplocalname hosts.byname`; \
			/etc/yp/yppush `yplocalname hosts.byaddr`; \
			echo "pushed hosts"; \
		else \
			: ; \
		fi \
	else \
		echo "couldn't find $(DIR)/hosts"; \
	fi

ethers.time: $(DIR)/ethers
	-@PATH=.:$$PATH ; \
	if [ -f $(DIR)/ethers ]; then \
		sed -e "/^#/d" -e s/#.*$$// $(DIR)/ethers \
		| awk 'BEGIN { X["A"]="a"; X["B"]="b"; X["C"]="c"; X["D"]="d";X["E"]="e"; X["F"]="f"; } \
		    { for (i = 1; i <= length($$1); i++) \
		      if (substr($$1, i, 1) >= "A" && substr($$1, i, 1) <= "Z") \
			printf("%s", X[substr($$1, i, 1)]) ; \
		      else \
			printf("%s", substr($$1, i, 1)); printf(" "); \
		    print $$2}' \
		|awk '{print $$1, $$0; for (i = 3;i <= NF;i++) print $$i,$$0}' \
		| makedbm - /etc/yp/$(DOM)/`yplocalname ethers.byaddr`; \
		awk 'BEGIN { OFS="\t"; } $$1 !~ /^#/ { print $$2, $$0 }' \
		   $(DIR)/ethers | \
		makedbm - /etc/yp/$(DOM)/`yplocalname ethers.byname`;\
		touch ethers.time; \
		echo "updated ethers"; \
		if [ ! $(NOPUSH) ]; then \
			/etc/yp/yppush `yplocalname ethers.byname`; \
			/etc/yp/yppush `yplocalname ethers.byaddr`; \
			echo "pushed ethers"; \
		else \
			: ; \
		fi \
	else \
		echo "couldn't find $(DIR)/ethers"; \
	fi

networks.time: $(DIR)/networks
	-@PATH=.:$$PATH ; \
	if [ -f $(DIR)/networks ]; then \
		sed -e "/^#/d" -e s/#.*$$// $(DIR)/networks | awk \
		    '{print $$1, $$0; for (i = 3;i <= NF;i++) print $$i,$$0}' \
		    | makedbm - /etc/yp/$(DOM)/`yplocalname networks.byname`; \
		awk 'BEGIN { OFS="\t"; } $$1 !~ /^#/ { print $$2, $$0 }' \
		   $(DIR)/networks | makedbm - /etc/yp/$(DOM)/`yplocalname networks.byaddr`;\
		touch networks.time; \
		echo "updated networks"; \
		if [ ! $(NOPUSH) ]; then \
			/etc/yp/yppush `yplocalname networks.byname`; \
			/etc/yp/yppush `yplocalname networks.byaddr`; \
			echo "pushed networks"; \
		else \
			: ; \
		fi \
	else \
		echo "couldn't find $(DIR)/networks"; \
	fi

services.time: $(DIR)/services
	-@PATH=.:$$PATH ; \
	if [ -f $(DIR)/services ]; then \
		awk 'BEGIN { OFS="\t"; } $$1 !~ /^#/ { print $$2, $$0 }' \
		    $(DIR)/services| makedbm - /etc/yp/$(DOM)/`yplocalname services.byname`;\
		touch services.time; \
		echo "updated services"; \
		if [ ! $(NOPUSH) ]; then \
			/etc/yp/yppush `yplocalname services.byname`; \
			echo "pushed services"; \
		else \
			: ; \
		fi \
	else \
		echo "couldn't find $(DIR)/services"; \
	fi

rpc.time: $(DIR)/rpc
	-@PATH=.:$$PATH ; \
	if [ -f $(DIR)/rpc ]; then \
		awk 'BEGIN { OFS="\t"; } $$1 !~ /^#/ { print $$2, $$0 }' \
		    $(DIR)/rpc| makedbm - /etc/yp/$(DOM)/`yplocalname rpc.bynumber`;\
		touch rpc.time; \
		echo "updated rpc"; \
		if [ ! $(NOPUSH) ]; then \
			/etc/yp/yppush `yplocalname rpc.bynumber`; \
			echo "pushed rpc"; \
		else \
			: ; \
		fi \
	else \
		echo "couldn't find $(DIR)/rpc"; \
	fi

protocols.time: $(DIR)/protocols
	-@PATH=.:$$PATH ; \
	if [ -f $(DIR)/protocols ]; then \
		awk 'BEGIN { OFS="\t"; } $$1 !~ /^#/ { print $$2, $$0 }' \
		    $(DIR)/protocols | makedbm - \
		    /etc/yp/$(DOM)/`yplocalname protocols.bynumber`; \
		sed -e "/^#/d" -e s/#.*$$// $(DIR)/protocols | awk \
		    '{print $$1,$$0; for (i = 3;i <= NF;i++) print $$i, $$0}' \
		    | makedbm - /etc/yp/$(DOM)/`yplocalname protocols.byname`; \
		touch protocols.time; \
		echo "updated protocols"; \
		if [ ! $(NOPUSH) ]; then \
			/etc/yp/yppush `yplocalname protocols.byname`; \
			/etc/yp/yppush `yplocalname protocols.bynumber`; \
			echo "pushed protocols"; \
			else \
			: ; \
		fi \
	else \
		echo "couldn't find $(DIR)/protocols"; \
	fi

netgroup.time: $(DIR)/netgroup
	-@PATH=.:$$PATH ; \
	if [ -f $(DIR)/netgroup ]; then \
		DOM=$(DOM); \
		makedbm $(DIR)/netgroup /etc/yp/$$DOM/`yplocalname netgroup`; \
		revnetgroup -u | makedbm - /etc/yp/$$DOM/`yplocalname netgroup.byuser`; \
		revnetgroup -h | makedbm - /etc/yp/$$DOM/`yplocalname netgroup.byhost`; \
		touch netgroup.time; \
		echo "updated netgroup"; \
		if [ ! $(NOPUSH) ]; then \
			/etc/yp/yppush `yplocalname netgroup`; \
			/etc/yp/yppush `yplocalname netgroup.byuser`; \
			/etc/yp/yppush `yplocalname netgroup.byhost`; \
			echo "pushed netgroup"; \
		else \
			: ; \
		fi \
	else \
		echo "couldn't find $(DIR)/netgroup"; \
	fi

aliases.time: $(ALIASES)
	-@PATH=.:$$PATH ; \
	if [ -f $(ALIASES) ]; then \
		if [ -f /usr/lib/sendmail ]; then \
			cp $(ALIASES) /etc/yp/$(DOM)/`yplocalname mail.aliases`; \
			/usr/lib/sendmail -bi -oA/etc/yp/$(DOM)/`yplocalname mail.aliases`; \
			rm /etc/yp/$(DOM)/`yplocalname mail.aliases`; \
			touch aliases.time; \
			echo "updated aliases"; \
			if [ ! $(NOPUSH) ]; then \
				/etc/yp/yppush `yplocalname mail.aliases`; \
				echo "pushed aliases"; \
			else \
				: ; \
			fi \
		else \
			echo "couldn't find /usr/lib/sendmail"; \
		fi \
	else \
		echo "couldn't find $(ALIASES)"; \
	fi

passwd: passwd.time
group: group.time
hosts: hosts.time
ethers: ethers.time
networks: networks.time
rpc: rpc.time
services: services.time
protocols: protocols.time
netgroup: netgroup.time
aliases: aliases.time
$(DIR)/passwd:
$(DIR)/group:
$(DIR)/hosts:
$(DIR)/ethers:
$(DIR)/networks:
$(DIR)/rpc:
$(DIR)/services:
$(DIR)/protocols:
$(DIR)/netgroup:
$(ALIASES):
