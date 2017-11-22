/*--------------------------------------------------------------------------*
 *                         ** WolfPaw 3.0 ** 				    *
 *--------------------------------------------------------------------------*
 *               WolfPaw 3.0 (c) 1997 - 1999 by Dale Corse		    *
 *--------------------------------------------------------------------------*
 *            The WolfPaw Coding Team is headed by: Greywolf                *
 *  With the Assitance from: Callinon, Dhamon, Sentra, Wyverns, Altrag      *
 *                           Scryn, Thoric, Justice, Tricops and Brogar.    *
 *--------------------------------------------------------------------------*
 *  Based on SMAUG 1.2a. Copyright 1994 - 1996 by Derek Snider		    *
 *  SMAUG Coding Team: Thoric, Altrag, Blodkai, Narn, Haus, Scryn, Rennard, *
 *		       Swordbearer, Gorog, Grishnakh and Tricops.           *
 *--------------------------------------------------------------------------*
 *  Merc 2.1 Diku MUD Improvments (C) 1992 - 1993 by Michael Chastain,      *
 *  Michael Quan, and Michael Tse. 					    *
 *  Original Diku MUD (C) 1990 - 1991 by Sebastian Hammer, Michael Seifert, *
 *  Hans Hendrik Strfeldt, Tom Madsen, and Katja Nyboe. 		    *
 *--------------------------------------------------------------------------*
 *			     Councils Module			            *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "mud.h"

/* local routines */
void	fread_clan	args( ( CLAN_DATA *clan, FILE *fp ) );
bool	load_clan_file	args( ( char *clanfile ) );
void	write_clan_list	args( ( void ) );

void	fread_council	args( ( COUNCIL_DATA *council, FILE *fp ) );
bool	load_council_file	args( ( char *councilfile ) );
void	write_council_list	args( ( void ) );

COUNCIL_DATA *get_council( char *name )
{
    COUNCIL_DATA *council;
    
    for ( council = first_council; council; council = council->next )
       if ( !str_cmp( name, council->name ) )
         return council;
    return NULL;
}

void write_council_list( )
{
    COUNCIL_DATA *tcouncil;
    FILE *fpout;
    char filename[256];

    sprintf( filename, "%s%s", COUNCIL_DIR, COUNCIL_LIST );
    fpout = fopen( filename, "w" );
    if ( !fpout )
    {
	bug( "FATAL: cannot open council.lst for writing!\n\r", 0 );
 	return;
    }	  
    for ( tcouncil = first_council; tcouncil; tcouncil = tcouncil->next )
	fprintf( fpout, "%s\n", tcouncil->filename );
    fprintf( fpout, "$\n" );
    new_fclose( fpout );
}

/*
 * Save a council's data to its data file
 */
void save_council( COUNCIL_DATA *council )
{
    FILE *fp;
    char filename[256];
    char buf[MAX_STRING_LENGTH];

    if ( !council )
    {
	bug( "save_council: null council pointer!", 0 );
	return;
    }
        
    if ( !council->filename || council->filename[0] == '\0' )
    {
	sprintf( buf, "save_council: %s has no filename", council->name );
	bug( buf, 0 );
	return;
    }
 
    sprintf( filename, "%s%s", COUNCIL_DIR, council->filename );
    
//    new_fclose( fpReserve );
    if ( ( fp = fopen( filename, "w" ) ) == NULL )
    {
    	bug( "save_council: fopen", 0 );
    	perror( filename );
    }
    else
    {
	fprintf( fp, "#COUNCIL\n" );
	fprintf( fp, "Name         %s~\n",	council->name		);
	fprintf( fp, "Filename     %s~\n",	council->filename	);
	fprintf( fp, "Description  %s~\n",	council->description	);
	fprintf( fp, "Head         %s~\n",	council->head		);
	fprintf( fp, "Members      %d\n",	council->members	);
	fprintf( fp, "Board        %d\n",	council->board		);
	fprintf( fp, "Meeting      %d\n",	council->meeting	);
	fprintf( fp, "Powers       %s~\n",	council->powers		);
	fprintf( fp, "End\n\n"						);
	fprintf( fp, "#END\n"						);
    }
    new_fclose( fp );
//    fpReserve = fopen( NULL_FILE, "r" );
    return;
}


/*
 * Read in actual council data.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

void fread_council( COUNCIL_DATA *council, FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *word=NULL;
    bool fMatch=FALSE;

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

	case 'B':
	    KEY( "Board",	council->board,		fread_number( fp ) );
	    break;

	case 'D':
	    KEY( "Description",	council->description,	fread_string( fp ) );
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
		if (!council->name)
		  council->name		= STRALLOC( "" );
		if (!council->description)
		  council->description 	= STRALLOC( "" );
		if (!council->powers)
		  council->powers	= STRALLOC( "" );
		return;
	    }
	    break;
	    
	case 'F':
	    KEY( "Filename",	council->filename,	fread_string_nohash( fp ) );
  	    break;

	case 'H':
	    KEY( "Head", 	council->head, 		fread_string( fp ) );
	    break;

	case 'M':
	    KEY( "Members",	council->members,	fread_number( fp ) );
	    KEY( "Meeting",   	council->meeting, 	fread_number( fp ) );
	    break;
 
	case 'N':
	    KEY( "Name",	council->name,		fread_string( fp ) );
	    break;

	case 'P':
	    KEY( "Powers",	council->powers,	fread_string( fp ) );
	    break;
	}
	
	if ( !fMatch )
	{
	    sprintf( buf, "Fread_council: no match: %s", word );
	    bug( buf, 0 );
	}
    }
}

/*
 * Load a council file
 */

bool load_council_file( char *councilfile )
{
    char filename[256];
    COUNCIL_DATA *council=NULL;
    FILE *fp;
    bool found;

    CREATE( council, COUNCIL_DATA, 1 );

    found = FALSE;
    sprintf( filename, "%s%s", COUNCIL_DIR, councilfile );

    if ( ( fp = fopen( filename, "r" ) ) != NULL )
    {

	found = TRUE;
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
		bug( "Load_council_file: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
	    if ( !str_cmp( word, "COUNCIL"	) )
	    {
	    	fread_council( council, fp );
	    	break;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
	        break;
	    else
	    {
		bug( "Load_council_file: bad section.", 0 );
		break;
	    }
	}
	new_fclose( fp );
    }

    if ( found )
      LINK( council, first_council, last_council, next, prev );

    else
      DISPOSE( council );

    return found;
}

/*
 * Load in all the council files.
 */
void load_councils( )
{
    FILE *fpList;
    char *filename;
    char councillist[256];
    char buf[MAX_STRING_LENGTH];
    
    
    first_council	= NULL;
    last_council	= NULL;

    log_string( "Loading councils..." );

    sprintf( councillist, "%s%s", COUNCIL_DIR, COUNCIL_LIST );
//    new_fclose( fpReserve );
    if ( ( fpList = fopen( councillist, "r" ) ) == NULL )
    {
	perror( councillist );
	exit( 1 );
    }

    for ( ; ; )
    {
	filename = feof( fpList ) ? "$" : fread_word( fpList );
	log_string( filename );
	if ( filename[0] == '$' )
	  break;

	if ( !load_council_file( filename ) )
	{
	  sprintf( buf, "Cannot load council file: %s", filename );
	  bug( buf, 0 );
	}
    }
    new_fclose( fpList );
    log_string(" Done councils " );
//    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

void do_council_induct( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    COUNCIL_DATA *council;

    if ( IS_NPC( ch ) || !ch->pcdata->council )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    council = ch->pcdata->council;
    
    if ( str_cmp( ch->name, council->head) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Induct whom into your council?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

/*    if ( victim->level < 51 )
    {
	send_to_char( "This player is not worthy of joining any council yet.\n\r", ch );
	return;
    }
*/
    if ( victim->pcdata->council )
    {
	send_to_char( "This player already belongs to a council!\n\r", ch );
	return;
    }

    council->members++;
    victim->pcdata->council = council;
    STRFREE(victim->pcdata->council_name);
    victim->pcdata->council_name = QUICKLINK( council->name );
    act( AT_MAGIC, "You induct $N into $t", ch, council->name, victim, TO_CHAR );
    act( AT_MAGIC, "$n inducts $N into $t", ch, council->name, victim, TO_ROOM );
    act( AT_MAGIC, "$n inducts you into $t", ch, council->name, victim, TO_VICT );
    save_char_obj( victim );
    save_council( council );
    return;
}

 void do_council_outcast( CHAR_DATA *ch,char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    COUNCIL_DATA *council;

    if ( IS_NPC( ch ) || !ch->pcdata->council )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    council = ch->pcdata->council;

    if ( str_cmp( ch->name, council->head ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Outcast whom from your council?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Kick yourself out of your own council?\n\r", ch );
	return;
    }
 
    if ( victim->pcdata->council != ch->pcdata->council )
    {
	send_to_char( "This player does not belong to your council!\n\r", ch );
	return;
    }

    --council->members;
    victim->pcdata->council = NULL;
    STRFREE(victim->pcdata->council_name);
    victim->pcdata->council_name = STRALLOC( "" );
    act( AT_MAGIC, "You outcast $N from $t", ch, council->name, victim, TO_CHAR );
    act( AT_MAGIC, "$n outcasts $N from $t", ch, council->name, victim, TO_ROOM );
    act( AT_MAGIC, "$n outcasts you from $t", ch, council->name, victim, TO_VICT );
    save_char_obj( victim );
    save_council( council );
    return;
}


void do_setcouncil( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    COUNCIL_DATA *council;

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Usage: setcouncil <council> <field> <deity|leader> <player>\n\r", ch );
	send_to_char( "\n\rField being one of:\n\r", ch );
	send_to_char( " head members board meeting\n\r", ch ); 
	if ( get_trust( ch ) >= LEVEL_GOD )
	{
	  send_to_char( " name filename desc\n\r", ch );
	  send_to_char( " powers\n\r", ch);
	}
	return;
    }

    council = get_council( arg1 );
    if ( !council )
    {
	send_to_char( "No such council.\n\r", ch );
	return;
    }

    if ( !strcmp( arg2, "head" ) )
    {
	STRFREE( council->head );
	council->head = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_council( council );
	return;
    }

    if ( !strcmp( arg2, "board" ) )
    {
	council->board = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_council( council );
	return;
    }

    if ( !strcmp( arg2, "members" ) )
    {
	council->members = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_council( council );
	return;
    }

    if ( !strcmp( arg2, "meeting" ) )
    {
	council->meeting = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_council( council );
	return;
    }

    if ( get_trust( ch ) < LEVEL_GOD )
    {
	do_setcouncil( ch, "" );
	return;
    }
    
    if ( !strcmp( arg2, "name" ) )
    {
	STRFREE( council->name );
	council->name = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_council( council );
	return;
    }


    if ( !strcmp( arg2, "filename" ) )
    {
	DISPOSE( council->filename );
	council->filename = str_dup( argument );
	send_to_char( "Done.\n\r", ch );
	save_council( council );
	write_council_list( );
	return;
    }

    if ( !strcmp( arg2, "desc" ) )
    {
	STRFREE( council->description );
	council->description = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_council( council );
	return;
    }

    if ( get_trust( ch ) < LEVEL_GOD )
    {
	do_setcouncil( ch, "" );
	return;
    }

    if ( !strcmp( arg2, "powers" ) )
    {
	STRFREE( council->powers );
	council->powers = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_council( council );
	return;
    }
    
    do_setcouncil( ch, "" );
    return;
}

void do_showcouncil( CHAR_DATA *ch, char *argument )
{
    COUNCIL_DATA *council;

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Usage: showcouncil <council>\n\r", ch );
	return;
    }

    council = get_council( argument );
    if ( !council )
    {
	send_to_char( "No such council.\n\r", ch );
	return;
    }

    ch_printf( ch, "Council    : %s\n\rFilename: %s\n\r",
    			council->name,
    			council->filename );
    ch_printf( ch, "Description: %s\n\rHead: %s\n\rMembers: %3d\n\r",
    			council->description,
    			council->head,
    			council->members );
    ch_printf( ch, "Board: %5d\n\rMeeting: %5d\n\rPowers: %s\n\r",
    			council->board,
    			council->meeting,
			council->powers );
    return;
}

void do_makecouncil( CHAR_DATA *ch, char *argument )
{
    char filename[256];
    COUNCIL_DATA *council=NULL;
    bool found;

    if ( !argument || argument[0] == '\0' )
    {
	send_to_char( "Usage: makecouncil <council name>\n\r", ch );
	return;
    }

    found = FALSE;
    sprintf( filename, "%s%s", COUNCIL_DIR, strlower(argument) );

    CREATE( council, COUNCIL_DATA, 1 );
    LINK( council, first_council, last_council, next, prev );
    council->name		= STRALLOC( argument );
    council->head		= STRALLOC( "" );
    council->powers		= STRALLOC( "" );
}

void do_councils( CHAR_DATA *ch, char *argument)
{
    COUNCIL_DATA *council;

    if ( !first_council )
    {
	send_to_char( "There are no councils currently formed.\n\r", ch );
	return;
    }

    set_char_color( AT_NOTE, ch );
    send_to_char( "Name                  Head\n\r", ch);
    for ( council = first_council; council; council = council->next )
	ch_printf( ch, "%-21s %-14s\n\r", council->name, council->head);
}                                                                           
