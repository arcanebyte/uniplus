/*
 * Convert an old style L.sys file to the new format.
 *
 * cc convLsys.c ../cfgets.o ../getargs.o
 *
 * L.M.McLoughlin.
 */
#include <stdio.h>

extern char *index();

#define DEF_WIDTH 8	/* Default data-width */

main()
{
	char buff[ 2000 ], *flds[ 100 ];

	printf( "#site dates devtype class width connectnum login.....\n" );
	printf( "# ?str expect str, :str send str, !str send only if ? failed\n"
	);
	while( cfgets( buff, sizeof( buff ), stdin ) != NULL ){
		int i;
		int fn = getargs( buff, flds );
		if( fn >= 0 )
			printf( "# How to connect to %s\n", flds[ 0 ] );
		if( fn <= 5 ){
			for( i = 0; i < fn && i < 4; i++ )
				printf( "%s ", flds[ i ] );
			for( ; i < 4; i++ )
				printf( "unused " );
			printf( " %d ", DEF_WIDTH );
			if( fn == 5 )
				printf( "%s\n", flds[ i ] );
			else	printf( "unused\n" );
			continue;
		}
		printf( "%s %s %s %s %d %s ",
		  flds[ 0 ], flds[ 1 ], flds[ 2 ], flds[ 3 ],
		  DEF_WIDTH,
		  flds[ 4 ] );
		if( fn > 5 )
			printf( "\\\n\t" );
		for( i = 5; i < fn; i++ )
			if( (i % 2 ) == 1 )
				newexp( flds[ i ] );
			else	newsend( flds[ i ] );
		printf( "\n" );
	}
}

newexp( s )
char *s;
{
	if( strcmp( s, "\"\"" ) == 0 )
		return;

	while( *s ){
		char *p = index( s, '-' );
		if( p != NULL ){
			*p = '\0';
			printf( "?%s ", s );
			s = p + 1;
		}
		else {
			printf( "?%s ", s );
			return;
		}
		if( ! *s )
			return;
		if( *s == '-' ){
			printf( "!\\r " );
			s++;
			continue;
		}
		p = index( s, '-' );
		if( p != NULL ){
			*p = '\0';
			printf( "!%s ", s );
			s = p + 1;
		}
		else {
			printf( "!%s ", s );
			return;
		}
	}
}

newsend( s )
char *s;
{
	char *p;
	int addret;

	if( sndnocr( s ) ){
		if( strncmp( "\"\"", s ) == 0 )
			printf( ":\\r " );
		else	printf( ":%s ", s );
		return;
	}

	p = s;
	addret = 1;
	while( *p ){
		if( *p == '\\' && *(p+1) == 'c' ){
			addret = 0;
			break;
		}
		p++;
	}
	printf( ":%s%s ", s, addret?"\\r":"" );
}
char *nocrs[] = {
	"\"\"", "BREAK", "PAUSE", "EOT", "P_ZERO", "P_ONE", "P_EVEN",
	"P_ODD", "LF", "CR", ""
	};
sndnocr( str )
char *str;
{
	char **p = nocrs;
	while( **p ){
		if( strncmp( str, *p, strlen( *p ) )== 0 )
			return( 1 );
		p++;
	}
	return( 0 );
}
