/*--------------------------------------------------------------------------*
 *                         ** WolfPaw 1.0 **                                *
 *--------------------------------------------------------------------------*
 *               WolfPaw 1.0 (c) 1997,1998 by Dale Corse                    *
 *--------------------------------------------------------------------------*
 *            The WolfPaw Coding Team is headed by: Greywolf                *
 *  With the Assitance from: Callinon, Dhamon, Sentra, Wyverns, Altrag      *
 *  Scryn, Thoric, Justice, Tricops and Mask.                               *
 *--------------------------------------------------------------------------*
 *			Petitions Module				    *
 *--------------------------------------------------------------------------*/
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "mud.h"

void load_petitions(void);
bool load_petition_file( char *pfile );
void fread_petition( PETITION_DATA *p, FILE *fp );
void write_petition( PETITION_DATA *p );
void write_petition_list( void );
void create_petition( char *owner, char *info );
void destroy_petition( PETITION_DATA *p );

/*
 * Load all Petition Files
 */
void load_petitions( )
{
    FILE *fpList;
    char *filename;
    char plist[256];
    char buf[MAX_STRING_LENGTH];


    first_petition  = NULL;
    last_petition   = NULL;

    sprintf( plist, "%s%s", PETITION_DIR, PETITION_LIST );
//    new_fclose( fpReserve );
    if ( ( fpList = fopen( plist, "r" ) ) == NULL )
    {
        perror( plist );
        exit( 1 );
    }

    for ( ; ; )
    {
        filename = feof( fpList ) ? "$" : fread_word( fpList );
        log_string( filename );
        if ( filename[0] == '$' )
          break;

        if ( !load_petition_file( filename ) )
        {
          sprintf( buf, "Cannot load clan file: %s", filename );
          bug( buf, 0 );
        }
    }
    new_fclose( fpList );
    log_string(" Done Petitions " );
//    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

/*
 * Load Petition File
 */
bool load_petition_file( char *pfile )
{
    char filename[256];
    PETITION_DATA *p=NULL;
    FILE *fp;

    sprintf( filename, "%s%s", PETITION_DIR, pfile );

    if ( ( fp = fopen( filename, "r" ) ) != NULL )
    {

        for ( ; ; )
        {
            char letter;
            char *word;

            letter = fread_letter( fp );
            if ( letter == '*' )

            {
                fread_to_eol( fp );
                continue;
            }

            if ( letter != '#' )
            {
                bug( "Load_petition_file: # not found.", 0 );
                break;
            }

            word = fread_word( fp );
            if ( !str_cmp( word, "PETITION" ) )
            {
                CREATE( p, PETITION_DATA, 1 );
                fread_petition( p, fp );
		LINK( p, first_petition, last_petition, next, prev );
                break;
            }
            else
            if ( !str_cmp( word, "END"  ) )
                break;
            else
            {
                char buf[MAX_STRING_LENGTH];

                sprintf( buf, "Load_petition_file: bad section: %s.", word );
                bug( buf, 0 );
                break;
            }
        }
        new_fclose( fp );
    }
return TRUE;
}


#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )                                    \
                                if ( !str_cmp( word, literal ) )        \
                                {                                       \
                                    field  = value;                     \
                                    fMatch = TRUE;                      \
                                    break;                              \
				}

/*
 * Read in a Petitions Data
 */
void fread_petition( PETITION_DATA *p, FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *word;
    bool fMatch;

    for ( ; ; )
    {
        word   = feof( fp ) ? "End" : fread_word( fp );
        fMatch = FALSE;

        switch ( UPPER(word[0]) )
        {
        case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;

        case 'I':
            KEY( "Info",        p->info,            fread_string( fp ));
            break;

	case 'O':
	    KEY( "Owner",	p->owner,	    fread_string( fp ));
	    break;

        case 'S':
	    KEY( "Signers",	p->signers,	    fread_number( fp ));
	    break;
        }

        if ( !fMatch )
        {
            sprintf( buf, "Fread_petition: no match: %s", word );
            bug( buf, 0 );
        }
    }
}

void write_petition( PETITION_DATA *p )
{
    FILE *fp;
    char file[MIL];

if ( !p )
return;

if ( !p->owner )
return;

sprintf( file, "%s%s.P", PETITION_DIR, p->owner );

if ( ( fp = fopen( file, "w" ) ) == NULL )
{
bug("Write_petition: cannot open file!",0);
perror(file);
return;
}

fprintf( fp, "#PETITION" );
fprintf( fp, "Owner		%s~\n", p->owner  );
fprintf( fp, "Info		%s~\n", p->info   );
fprintf( fp, "Signers		%d\n", p->signers );
fprintf( fp, "End\n\n" );
fprintf( fp, "#END\n"  );
new_fclose(fp);

return;
}

void create_petition( char *owner, char *info )
{
PETITION_DATA *p=NULL;

CREATE( p, PETITION_DATA, 1 );
p->owner		= STRALLOC( owner );
p->info			= STRALLOC( info );
p->signers		= 0;
LINK( p, first_petition, last_petition, next, prev );
write_petition_list();
return;
}

void write_petition_list( )
{
PETITION_DATA *temp, *temp_next;
FILE *fp;
char file[MIL];

sprintf( file, "%s%s", PETITION_DIR, PETITION_LIST );

if ( ( fp = fopen( file, "w" ) ) == NULL )
{
bug("write_petition_list: cannot open file!",0);
perror(file);
return;
}

for( temp = first_petition; temp; temp = temp_next )
{
temp_next = temp->next;

fprintf( fp, "%s.P\n", temp->owner );

}

fprintf( fp, "$" );
new_fclose(fp);
return;
}

void destroy_petition( PETITION_DATA *p )
{
if ( !p )
return;

STRFREE(p->owner);
STRFREE(p->info );
UNLINK( p, first_petition, last_petition, next, prev );
DISPOSE(p);
}


