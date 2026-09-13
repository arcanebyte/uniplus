if [ ! -r version ]; then echo 0 > version; fi
touch version
awk '	{	version = $1 + 1; }\
END	{	printf "char version[] = \"Berkeley UNIX (Rev. 2.9.%d) ", version > "vers.c";\
		printf "%d\n", version > "tmpvers"; }' < version
mv tmpvers version
echo `date`'\n";' >> vers.c
