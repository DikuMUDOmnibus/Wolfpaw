#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include "mud.h"

/*
 * Local functions.
 */
FILE            *       popen           args( ( const char *command, const
char *type ) );
int                     pclose          args( ( FILE *stream ) );
char            *       fgetf           args( ( char *s, int n, register
FILE *iop ) );

void do_pipe( CHAR_DATA *ch, char *argument )
{
    char buf[5000];
    FILE *fp;

    fp = popen( argument, "r" );

    fgetf( buf, 5000, fp );

    send_to_pager( buf, ch );

    pclose( fp );

    return;
}

char *fgetf( char *s, int n, register FILE *iop )
{
    register int c;
    register char *cs;

    c = '\0';
    cs = s;
    while( --n > 0 && (c = getc(iop)) != EOF)
        if ((*cs++ = c) == '\0')
            break;
    *cs = '\0';
    return((c == EOF && cs == s) ? NULL : s);
}
