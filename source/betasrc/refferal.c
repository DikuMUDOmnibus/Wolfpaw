/*--------------------------------------------------------------------------*
 *                         ** WolfPaw 1.0 **                                *
 *--------------------------------------------------------------------------*
 *               WolfPaw 1.0 (c) 1997,1998 by Dale Corse                    *
 *--------------------------------------------------------------------------*
 *            The WolfPaw Coding Team is headed by: Greywolf                *
 *  With the Assitance from: Callinon, Dhamon, Sentra, Wyverns, Altrag      *
 *  Scryn, Thoric, Justice, Tricops and Mask.                               *
 *--------------------------------------------------------------------------*
 *		Refferal Points Addition and Calculation Module             *
 *--------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "mud.h"

#define KEY( literal, field, value )                                    \
                                if ( !str_cmp( word, literal ) )        \
                                {                                       \
                                    field  = value;                     \
                                    fMatch = TRUE;                      \
                                    break;                              \
                                }
 

void	add_refferal args( ( char *name,char *reff, char *site, char *email ) );
void    load_refferals args( ( void ) );
void    write_refferals args( ( void ) );
void    load_reff args( ( REFFERAL_DATA *ref, FILE *fp ) );

void add_refferal( char *name,char *reff, char *site, char *email )
{
REFFERAL_DATA *ref=NULL;
char *strtime;

record_call("<add_refferal>");
/* no refferal if no name entered*/
if ( name == NULL || reff == NULL )
return;
            strtime     = ctime( &current_time );
            strtime[strlen(strtime)-1]  = '\0';
            CREATE( ref, REFFERAL_DATA, 1 );
            ref->name = STRALLOC( reff );
            ref->refferal     = STRALLOC( capitalize(name) );
            ref->site = STRALLOC( site );
            ref->date = STRALLOC( strtime );
            ref->email  = STRALLOC( email );
            LINK(ref,first_refferal,last_refferal,next,prev);
            write_refferals();
return;
}

void make_refferal_list( void )
{
record_call("<make_refferal_list>");
return;
}

void load_refferals( void )
{
REFFERAL_DATA *ref=NULL;
FILE *fp;
bool fHeader = FALSE;
record_call("<load_refferals>");

if( (fp=fopen( REFF_FILE,"r" ) )==NULL )
{
bug("load_refferals: CANNOT OPEN REFF_FILE",0);
perror(REFF_FILE);
return;
}

for ( ; ; )
{
char letter;
char *word;

letter = fread_letter( fp );
if( letter == '*' )
{
fread_to_eol( fp );
continue;
}

if ( letter != '#' )
{
bug("Load_Refferals: # not Found.",0);
break;
}

word = fread_word( fp );
if ( !str_cmp(word, "REFF" ) )
{
fHeader = TRUE;
fread_to_eol( fp );
continue;
}

else if ( !str_cmp(word, "REFFERAL" ) )
{
CREATE( ref, REFFERAL_DATA,1 );
load_reff( ref, fp );
LINK( ref, first_refferal, last_refferal, next, prev );
continue;
}
else if ( !str_cmp(word, "END" ) )
break;
else
{
bug("Load_Refferals: Bad Section.(%s - %d",word,__LINE__);
break;
}

if ( !fHeader )
log_string("Load_Refferals: #REFFDATA not found!");

}
new_fclose(fp);  
return;
}

void write_refferals( void )
{
REFFERAL_DATA *ref;
FILE *fp;
record_call("<write_refferals>");

if ( (fp=fopen( REFF_FILE,"w" ) )==NULL)
{
bug("Cannot open REFF_FILE!",0);
perror(REFF_FILE);
return;
}

fprintf( fp, "#REFF DATA\n\n" );
for ( ref = first_refferal; ref; ref = ref->next )
{
fprintf( fp,"#REFFERAL\n");
fprintf( fp,"Name       %s~\n", ref->name );
fprintf( fp,"Refferal   %s~\n", ref->refferal );
fprintf( fp,"Site       %s~\n", ref->site );
fprintf( fp,"Date       %s~\n", ref->date );
fprintf( fp,"Email      %s~\n", ref->email );
fprintf( fp,"$$\n\n");
}

fprintf( fp,"#END DATA" );
new_fclose(fp);
return;
}
void load_reff( REFFERAL_DATA *ref, FILE *fp )
{
    char *word=NULL;
    bool fMatch=FALSE;
    record_call("<load_reff>");

    for( ; ; )
    {
        word = feof( fp ) ? "$$" : fread_word( fp );
        fMatch = FALSE;

        switch ( UPPER(word[0]) )
        {
        case '*':
                fMatch = TRUE;
                fread_to_eol( fp );
                break;

        case '$':
                fMatch = TRUE;
                return;

	case 'D':
	KEY( "Date",	 ref->date,	fread_string( fp ) );
	break;

        case 'E':
        KEY( "Email",    ref->email,     fread_string( fp ) );
        break;

        case 'N':
        KEY( "Name",    ref->name,     fread_string( fp ) );
        break;

        case 'R':
        KEY( "Refferal", ref->refferal,     fread_string( fp ) );
        break;

        case 'S':
        KEY( "Site", ref->site,  fread_string( fp ) );
        break;

        }

        if ( !fMatch  )
        {
          bug( "Load_Refferal: no match for %s.", word );
        }
   }
}

void do_refferal( CHAR_DATA *ch, char *argument )
{
record_call("<do_refferal>");
send_to_char("Function not Complete\n\r",ch);
return;
}

