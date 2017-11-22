/*--------------------------------------------------------------------------*
 *                         ** WolfPaw 1.0 **                                *
 *--------------------------------------------------------------------------*
 *               WolfPaw 1.0 (c) 1997,1998 by Dale Corse                    *
 *--------------------------------------------------------------------------*
 *            The WolfPaw Coding Team is headed by: Greywolf                *
 *  With the Assitance from: Callinon, Dhamon, Sentra, Wyverns, Altrag      *
 *  Scryn, Thoric, Justice, Tricops and Mask.                               *
 *--------------------------------------------------------------------------*
 *  	      Room Saving Module For NON-Clan Storeroom Saving		    *
 *--------------------------------------------------------------------------*/
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include "mud.h"

void load_save_rooms( void );
void load_save_room( char *filename );
void save_save_room( CHAR_DATA *ch );
void clean_save_room( ROOM_INDEX_DATA *room );

static  OBJ_DATA *      rgObjNest       [MAX_NEST];

void load_save_rooms( void )
{
    FILE *fpList;
    char *filename;
    char clanlist[256];

    sprintf( clanlist, "%s%s", SAVE_ROOM_DIR, SAVE_ROOM_LIST );
    if ( ( fpList = fopen( clanlist, "r" ) ) == NULL )
    {
        perror( clanlist );
        exit( 1 );
    }

    for ( ; ; )
    {
        filename = feof( fpList ) ? "$" : fread_word( fpList );
        if ( filename[0] == '$' )
          break;

	if ( filename[0] != '\0' )
        load_save_room( filename );

    }
    new_fclose( fpList );
    log_string("Done SaveRooms" );
    return;
}

void load_save_room( char *filename )
{
    FILE *fp;
    ROOM_INDEX_DATA *save_room;
    int vnum, zone;
    char buf[MSL], log_string[MSL];
    char *temp=NULL;
    int objloaded=0;
    
    strcpy( temp, &filename[0]);
    zone = atoi(temp);
    strcpy(temp,filename);
    filename++;
    filename++;
    vnum = atoi(filename);
    strcpy(filename,temp);

	sprintf( buf, "%s%s", SAVE_ROOM_DIR, filename );
        if ( ( fp = fopen( buf, "r" ) ) != NULL )
        {
            int iNest;
            bool found;
            OBJ_DATA *tobj, *tobj_next;
		
	    sprintf( buf, "Loading SAVE-ROOM: Z:%d R:%d",zone,vnum);
            log_string( buf );

	    if ( ( save_room = get_room_index( vnum, zone ) )==NULL)
	    {
		bug("Load_Save_Room: Bad room index! (Z:%d R:%d)",zone,vnum);
		return;
	    }

            rset_supermob(save_room);
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
                    bug( "Load_Save_Room: # not found.", 0 );
                    bug( "Z:%d R:%d", zone,vnum );
                    break;
                }

                word = fread_word( fp );
                if ( !str_cmp( word, "OBJECT" ) )       /* Objects      */
	        {
                  fread_obj  ( supermob, fp, OS_CARRY );
	          objloaded++;
	        }
                else
                if ( !str_cmp( word, "END"    ) )       /* Done         */
	        {
	          sprintf(log_string,"Loaded %d Objects.",objloaded);
	          log_string(log_string);
                  break;
	        }
                else
                {
                    bug( "Load_Save_Room: bad section.", 0 );
                    bug( "Z:%d R:%d", zone,vnum );
                    break;
                }
            }
            new_fclose( fp );
            for ( tobj = supermob->first_carrying; tobj; tobj = tobj_next )
            {
                tobj_next = tobj->next_content;
                obj_from_char( tobj );
                obj_to_room( tobj, save_room );
            }
            release_supermob();
	    if ( IS_SET( save_room->room_flags, ROOM_DONATION ) )
	    clean_save_room(save_room);

        }
	else
	{
	bug("Load_Save_Room: Cannot Open File (%s)!",filename);
	return;
	}

return;
}

void save_save_room( CHAR_DATA *ch )
{
    FILE *fp;
    char filename[256];
    sh_int templvl;
    OBJ_DATA *contents;
    char buf[256];

    if ( !ch )
    {
        bug ("Save_Save_Room: Null ch pointer!", 0);
        return;
    }
    
    sprintf( buf, "%d-%d",ch->in_room->area->zone->number,
		ch->in_room->vnum );
    sprintf( filename, "%s%s.save-room", SAVE_ROOM_DIR, buf );
    if ( ( fp = fopen( filename, "w" ) ) == NULL )
    {
        bug( "Save_Save_Room: fopen", 0 );
        perror( filename );
    }
    else
    {
        templvl = ch->level;
        ch->level = LEVEL_AVATAR;               /* make sure EQ doesn't get lost */

        contents = ch->in_room->last_content;
        if (contents)
          fwrite_obj(ch, contents, fp, 0, OS_SAVEROOM, FALSE );
        fprintf( fp, "#END\n" );
        ch->level = templvl;
        new_fclose( fp );
    }
    return;
}

/*
 * Donation Room Cleaner --GW 
 */
void clean_save_room( ROOM_INDEX_DATA *room )
{
char log_str[MSL];
OBJ_DATA *pit, *tobj, *tobj_next, *tobj2, *tobj_next2;

sprintf(log_str,"Cleaning Save-Room: %d",room->vnum);
log_string(log_str);

/* Bringin the Super-Mob */
rset_supermob(room);

/* Find the Pit */
if ( !(pit = get_obj_here( supermob, "pit" ) ) )
{
bug("Clean_save_room: NO PIT!",0);
return;
}

/* Get all the stuff on the floor */
for ( tobj = supermob->in_room->first_content; tobj; tobj = tobj_next)
{
                tobj_next = tobj->next_content;
		if ( IS_SET(tobj->wear_flags, ITEM_TAKE ) )
	        {
                  obj_from_room( tobj );
                  obj_to_char( tobj, supermob );
}		}

/* Put it all in the Pit */
for ( tobj2 = supermob->first_carrying; tobj2; tobj2 = tobj_next2)
{
                tobj_next2 = tobj2->next_content;

		if ( tobj2 != pit )
		{
                  obj_from_char( tobj2 );
                  obj_to_obj( tobj2, pit );
		}
}

/* drop the pit */
for ( tobj = supermob->first_carrying; tobj; tobj = tobj_next)
{
                tobj_next = tobj->next_content;
                obj_from_char( pit );
                obj_to_room( pit, supermob->in_room );
}

/* Release Supermob */
release_supermob();
log_string("Done");
return;
}

