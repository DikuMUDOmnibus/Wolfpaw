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
 *           Revised Clan/Guild/Order Loading/Saving Module                 *
 *--------------------------------------------------------------------------*/
 /* NOTE: Functions in this file based upon and some parts written by the
  *       SMAUG coding team. --GW
  */
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "mud.h"

static  OBJ_DATA *      rgObjNest       [MAX_NEST];

void write_clan_list( )
{
    CLAN_DATA *tclan;
    FILE *fpout;
    char filename[256];

    sprintf( filename, "%s%s", CLAN_DIR, CLAN_LIST );
    fpout = fopen( filename, "w" );
    if ( !fpout )
    {
        bug( "FATAL: cannot open clan.lst for writing!\n\r", 0 );
         return;
    }
    for ( tclan = first_clan; tclan; tclan = tclan->next )
        fprintf( fpout, "%s\n", tclan->filename );
    fprintf( fpout, "$\n" );
    new_fclose( fpout );
}

/*
 * Save a clan's data to its data file
 */
void save_clan( CLAN_DATA *clan )
{
    FILE *fp;
    char filename[256];
    char buf[MAX_STRING_LENGTH];

    if ( !clan )
    {
        bug( "save_clan: null clan pointer!", 0 );
        return;
    }

    if ( !clan->filename || clan->filename[0] == '\0' )
    {
        sprintf( buf, "save_clan: %s has no filename", clan->name );
        bug( buf, 0 );
        return;
    }

    sprintf( filename, "%s%s", CLAN_DIR, clan->filename );

//    new_fclose( fpReserve );
    if ( ( fp = fopen( filename, "w" ) ) == NULL )
    {
        bug( "save_clan: fopen", 0 );
        perror( filename );
    }
    else
    {
        fprintf( fp, "#CLAN\n" );
        fprintf( fp, "Name         %s~\n",      clan->name              );
        fprintf( fp, "WhoName      %s~\n",      clan->whoname           );
        fprintf( fp, "Filename     %s~\n",      clan->filename          );
        fprintf( fp, "Fullname     %s~\n",      clan->fullname          );
        fprintf( fp, "Motto        %s~\n",      clan->motto             );
        fprintf( fp, "Description  %s~\n",      clan->description       );
        fprintf( fp, "Deity        %s~\n",      clan->deity             );
        fprintf( fp, "Leader       %s~\n",      clan->leader            );
        fprintf( fp, "Associate    %s~\n",      clan->associate         );
        fprintf( fp, "PKills       %d\n",       clan->pkills            );
        fprintf( fp, "PDeaths      %d\n",       clan->pdeaths           );
        fprintf( fp, "MKills       %d\n",       clan->mkills            );
        fprintf( fp, "MDeaths      %d\n",       clan->mdeaths           );
        fprintf( fp, "IllegalPK    %d\n",       clan->illegal_pk        );
        fprintf( fp, "Score        %d\n",       clan->score             );
        fprintf( fp, "Type         %d\n",       clan->clan_type         );
        fprintf( fp, "Class        %d\n",       clan->class             );
        fprintf( fp, "Favour       %d\n",       clan->favour            );
        fprintf( fp, "Strikes      %d\n",       clan->strikes           );
        fprintf( fp, "Members      %d\n",       clan->members           );
        fprintf( fp, "Alignment    %d\n",       clan->alignment         );
        fprintf( fp, "Board        %d\n",       clan->board             );
        fprintf( fp, "ClanObjOne   %d\n",       clan->clanobj1          );
        fprintf( fp, "ClanObjTwo   %d\n",       clan->clanobj2          );
        fprintf( fp, "ClanObjThree %d\n",       clan->clanobj3          );
        fprintf( fp, "Recall       %d\n",       clan->recall            );
        fprintf( fp, "Storeroom    %d\n",       clan->storeroom         );
        fprintf( fp, "GuardOne     %d\n",       clan->guard1            );
        fprintf( fp, "GuardTwo     %d\n",       clan->guard2            );
        fprintf( fp, "Morgue       %d\n",       clan->morgue            );
        fprintf( fp, "End\n\n"                                          );
        fwrite_pkill_data(fp,clan);
        fprintf( fp, "#END\n"                                           );
        new_fclose(fp);
    }
//    fpReserve = fopen( NULL_FILE, "r" );
    return;
}


/*
 * Read in actual clan data.
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

void fread_clan( CLAN_DATA *clan, FILE *fp )
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

        case 'A':
            KEY( "Associate",   clan->associate,        fread_string( fp ) );
            KEY( "Alignment",   clan->alignment,        fread_number( fp ) );
            break;

        case 'B':
            KEY( "Board",       clan->board,            fread_number( fp ) );
            break;

        case 'C':
            KEY( "ClanObjOne",  clan->clanobj1,         fread_number( fp ) );
            KEY( "ClanObjTwo",  clan->clanobj2,         fread_number( fp ) );
            KEY( "ClanObjThree",clan->clanobj3,         fread_number( fp ) );
            KEY( "Class",       clan->class,            fread_number( fp ) );
            break;

        case 'D':
            KEY( "Deity",       clan->deity,            fread_string( fp ) );
            KEY( "Description", clan->description,      fread_string( fp ) );
            break;

        case 'E':
            if ( !str_cmp( word, "End" ) )
            {
                if (!clan->name)
                  clan->name            = STRALLOC( "" );
                if (!clan->leader)
                  clan->leader          = STRALLOC( "" );
                if (!clan->description)
                  clan->description     = STRALLOC( "" );
                if (!clan->motto)
                  clan->motto           = STRALLOC( "" );
                if (!clan->deity)
                  clan->deity           = STRALLOC( "" );
                return;
            }
            break;

        case 'F':
            KEY( "Favour",      clan->favour,           fread_number( fp ) );
            KEY( "Filename",    clan->filename,         fread_string_nohash( fp ) );
            KEY( "Fullname",    clan->fullname,         fread_string( fp ) );

        case 'G':
            KEY( "GuardOne",    clan->guard1,           fread_number( fp ) );
            KEY( "GuardTwo",    clan->guard2,           fread_number( fp ) );
            break;

        case 'I':
            KEY( "IllegalPK",   clan->illegal_pk,       fread_number( fp ) );
            break;

        case 'L':
            KEY( "Leader",      clan->leader,           fread_string( fp ) );
            break;

        case 'M':
            KEY( "MDeaths",     clan->mdeaths,          fread_number( fp ) );
            KEY( "Members",     clan->members,          fread_number( fp ) );
            KEY( "MKills",      clan->mkills,           fread_number( fp ) );
            KEY( "Motto",       clan->motto,            fread_string( fp ) );
            KEY( "Morgue",      clan->morgue,           fread_number( fp ) );
            break;

        case 'N':
            KEY( "Name",        clan->name,             fread_string( fp ) );
            break;

        case 'P':
            KEY( "PDeaths",     clan->pdeaths,          fread_number( fp ) );
            KEY( "PKills",      clan->pkills,           fread_number( fp ) );
            break;

        case 'R':
            KEY( "Recall",      clan->recall,           fread_number( fp ) );
            break;

        case 'S':
            KEY( "Score",       clan->score,            fread_number( fp ) );
            KEY( "Strikes",     clan->strikes,          fread_number( fp ) );
            KEY( "Storeroom",   clan->storeroom,        fread_number( fp ) );
            break;
        case 'T':
            KEY( "Type",        clan->clan_type,        fread_number( fp ) );
            break;

        case 'W':
            KEY( "WhoName",     clan->whoname,          fread_string( fp ) );
            break;
        }

        if ( !fMatch )
        {
            sprintf( buf, "Fread_clan: no match: %s", word );
            bug( buf, 0 );
        }
    }
}

/*
 * Load a clan file
 */

bool load_clan_file( char *clanfile )
{
    char filename[256];
    CLAN_DATA *clan=NULL;
    FILE *fp;
    bool found;
    int loadedobj=0;
    char log_str[MSL];

    CREATE( clan, CLAN_DATA, 1 );

    found = FALSE;
    sprintf( filename, "%s%s", CLAN_DIR, clanfile );

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
                bug( "Load_clan_file: # not found.", 0 );
                break;
            }

            word = fread_word( fp );
            if ( !str_cmp( word, "CLAN" ) )
            {
                fread_clan( clan, fp );
            }
	    else
	    if ( !str_cmp( word, "PKDATA" ) )
	    {
		fread_pkill_data(fp,clan);
	    }		
            else
            if ( !str_cmp( word, "END"  ) )
                break;
            else
            {
                char buf[MAX_STRING_LENGTH];

                sprintf( buf, "Load_clan_file: bad section: %s.", word );
                bug( buf, 0 );
                break;
            }
        }
        new_fclose( fp );
    }

    if ( !clan->associate )
      clan->associate = STRALLOC( "" );

    if ( found )
    {
        ROOM_INDEX_DATA *storeroom;

        LINK( clan, first_clan, last_clan, next, prev );

        if ( clan->storeroom == 0
        || (storeroom = get_room_index( clan->storeroom,1 )) == NULL )
        {
            log_string( "Storeroom not found" );
            return found;
        }

        sprintf( filename, "%s%s.vault", CLAN_DIR, clan->filename );
        if ( ( fp = fopen( filename, "r" ) ) != NULL )
        {
            int iNest;
            bool found;
            OBJ_DATA *tobj, *tobj_next;

            log_string( "Loading clan storage room" );
            rset_supermob(storeroom);
            for ( iNest = 0; iNest < MAX_NEST; iNest++ )
                rgObjNest[iNest] = NULL;

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
                    bug( "Load_clan_vault: # not found.", 0 );
                    bug( clan->name, 0 );
                    break;
                }

                word = fread_word( fp );
                if ( !str_cmp( word, "OBJECT" ) )       /* Objects      */
		{
                  fread_obj  ( supermob, fp, OS_CARRY );
		  loadedobj++;
		}
                else if ( !str_cmp( word, "END"    ) )       /* Done*/
                  break;
                else
                {
                    bug( "Load_clan_vault: bad section.", 0 );
                    bug( clan->name, 0 );
                    break;
                }
            }
            new_fclose( fp );
            for ( tobj = supermob->first_carrying; tobj; tobj = tobj_next )
            {
                tobj_next = tobj->next_content;
                obj_from_char( tobj );
                obj_to_room( tobj, storeroom );
		/* Count Donation Pits in the Limits */
		if ( IS_LIMITED(tobj) && tobj->count < 2 )
		   adjust_limits( tobj->pIndexData, 1, LOADED_ADJUST_UP );
		else if ( IS_LIMITED(tobj) && tobj->count >= 2 )
		   adjust_limits( tobj->pIndexData, tobj->count, LOADED_ADJUST_UP );
            }
            release_supermob();
	    sprintf(log_str,"Loaded %d Objects.",loadedobj);
	    log_string(log_str);
	    clean_save_room(storeroom);
        }
        else
            log_string( "Cannot open clan vault" );
    }
    else
      DISPOSE( clan );

    return found;
}

/*
 * Load in all the clan files.
 */
void load_clans( )
{
    FILE *fpList;
    char *filename;
    char clanlist[256];
    char buf[MAX_STRING_LENGTH];


    first_clan  = NULL;
    last_clan   = NULL;

    sprintf( clanlist, "%s%s", CLAN_DIR, CLAN_LIST );
//    new_fclose( fpReserve );
    if ( ( fpList = fopen( clanlist, "r" ) ) == NULL )
    {
        perror( clanlist );
        exit( 1 );
    }

    for ( ; ; )
    {
        filename = feof( fpList ) ? "$" : fread_word( fpList );
        log_string( filename );
        if ( filename[0] == '$' )
          break;

        if ( !load_clan_file( filename ) )
        {
          sprintf( buf, "Cannot load clan file: %s", filename );
          bug( buf, 0 );
        }
    }
    new_fclose( fpList );
    log_string(" Done clans " );
//    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

