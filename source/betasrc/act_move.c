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
 *			   Player movement module			    *
 ****************************************************************************/


#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "mud.h"

void remove_char        args( ( CHAR_DATA *ch ) );
bool area_level_check( CHAR_DATA *ch, AREA_DATA *area );

const	sh_int	movement_loss	[SECT_MAX]	=
{
    1, 2, 2, 3, 4, 6, 4, 1, 6, 10, 6, 5, 7, 4
};

char *	const	dir_name	[]		=
{
    "north", "east", "south", "west", "up", "down",
    "northeast", "northwest", "southeast", "southwest", "somewhere"
};

const	int	trap_door	[]		=
{
    TRAP_N, TRAP_E, TRAP_S, TRAP_W, TRAP_U, TRAP_D,
    TRAP_NE, TRAP_NW, TRAP_SE, TRAP_SW
};


const	sh_int	rev_dir		[]		=
{
    2, 3, 0, 1, 5, 4, 9, 8, 7, 6, 10
};


ROOM_INDEX_DATA * vroom_hash [64];


/*
 * Local functions.
 */
bool	has_key		args( ( CHAR_DATA *ch, int key ) );


char *	const		sect_names[SECT_MAX][2] =
{
    { "In a room","inside"	},	{ "In a city",	"cities"	},
    { "In a field","fields"	},	{ "In a forest","forests"	},
    { "hill",	"hills"		},	{ "On a mountain","mountains"	},
    { "In the water","waters"	},	{ "In rough water","waters"	},
    { "Underwater", "underwaters" },	{ "In the air",	"air"		},
    { "In a desert","deserts"	},	{ "Somewhere",	"unknown"	},
    { "ocean floor", "ocean floor" },	{ "underground", "underground"	}
};


const	int		sent_total[SECT_MAX]		=
{
    3, 5, 4, 4, 1, 1, 1, 1, 1, 2, 2, 25, 1, 1
};

char *	const		room_sents[SECT_MAX][25]	=
{
    {
	"rough hewn walls of granite with the occasional spider crawling around",
	"signs of a recent battle from the bloodstains on the floor",
	"a damp musty odour not unlike rotting vegetation"
    },

    {
	"the occasional stray digging through some garbage",
	"merchants trying to lure customers to their tents",
	"some street people putting on an interesting display of talent",
	"an argument between a customer and a merchant about the price of an item",
	"several shady figures talking down a dark alleyway"
    },

    {
	"sparce patches of brush and shrubs",
	"a small cluster of trees far off in the distance",
	"grassy fields as far as the eye can see",
	"a wide variety of weeds and wildflowers"
    },

    {
	"tall, dark evergreens prevent you from seeing very far",
	"many huge oak trees that look several hundred years old",
	"a solitary lonely weeping willow",
	"a patch of bright white birch trees slender and tall"
    },

    {
	"rolling hills lightly speckled with violet wildflowers"
    },

    {
	"the rocky mountain pass offers many hiding places"
    },

    {
	"the water is smooth as glass"
    },

    {
	"rough waves splash about angrily"
    },

    {
	"a small school of fish"
    },

    {
	"the land far below",
	"a misty haze of clouds"
    },

    {
	"sand as far as the eye can see",
	"an oasis far in the distance"
    },

    {
	"nothing unusual",	"nothing unusual",	"nothing unusual",
	"nothing unusual",	"nothing unusual",	"nothing unusual",
	"nothing unusual",	"nothing unusual",	"nothing unusual",
	"nothing unusual",	"nothing unusual",	"nothing unusual",
	"nothing unusual",	"nothing unusual",	"nothing unusual",
	"nothing unusual",	"nothing unusual",	"nothing unusual",
	"nothing unusual",	"nothing unusual",	"nothing unusual",
	"nothing unusual",	"nothing unusual",	"nothing unusual",
	"nothing unusual",
    },

    { 
        "rocks and coral which litter the ocean floor."
    },

    {
	"a lengthy tunnel of rock."
    }

};

char *grab_word( char *argument, char *arg_first )
{
    char cEnd;
    sh_int count;

    count = 0;

    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' || ++count >= 255 )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*arg_first++ = *argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
	argument++;

    return argument;
}

char *wordwrap( char *txt, sh_int wrap )
{
    static char buf[MAX_STRING_LENGTH];
    char *bufp;

    buf[0] = '\0';
    bufp = buf;
    if ( txt != NULL )
    {
        char line[MAX_STRING_LENGTH];
        char temp[MAX_STRING_LENGTH];
        char *ptr, *p;
        int ln, x;

	++bufp;
        line[0] = '\0';
        ptr = txt;
        while ( *ptr )
        {
	  ptr = grab_word( ptr, temp );
          ln = strlen( line );  x = strlen( temp );
          if ( (ln + x + 1) < wrap )
          {
	    if ( line[ln-1] == '.' )
              strcat( line, "  " );
	    else
              strcat( line, " " );
            strcat( line, temp );
            p = strchr( line, '\n' );
            if ( !p )
              p = strchr( line, '\r' );
            if ( p )
            {
                strcat( buf, line );
                line[0] = '\0';
            }
          }
          else
          {
            strcat( line, "\r\n" );
            strcat( buf, line );
            strcpy( line, temp );
          }
        }
        if ( line[0] != '\0' )
            strcat( buf, line );
    }
    return bufp;
}

void decorate_room( ROOM_INDEX_DATA *room )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int nRand;
    int iRand, len;
    int previous[8];
    int sector = room->sector_type;
    char *pre=NULL, *post=NULL;

    if ( room->name )
      STRFREE( room->name );
    if ( room->description )
      STRFREE( room->description );

/*
    room->name	  = STRALLOC( "In a virtual room" );
    room->description = STRALLOC( "You're on a pathway.\n\r" );
*/

    room->name	= STRALLOC( sect_names[sector][0] );
    buf[0] = '\0';
    nRand = number_range( 1, UMIN(8,sent_total[sector]) );

    for ( iRand = 0; iRand < nRand; iRand++ )
	previous[iRand] = -1;

    for ( iRand = 0; iRand < nRand; iRand++ )
    {
	while ( previous[iRand] == -1 )
	{
	    int x, z;

	    x = number_range( 0, sent_total[sector]-1 );

	    for ( z = 0; z < iRand; z++ )
		if ( previous[z] == x )
		  break;

	    if ( z < iRand )
		  continue;

	    previous[iRand] = x;

	    len = strlen(buf);
	    if ( len == 0 )
	    {
	      switch( number_range(1, 2 * (iRand == nRand -1) ? 1 : 2) )
	      {
		case 1:	pre = "You notice ";	post = ".";	 break;
		case 2: pre = "You see ";	post = ".";	 break;
		case 3: pre = "You see ";	post = ", and "; break;
		case 4: pre = "You notice ";	post = ", and "; break;
	      }
	      sprintf( buf2, "%s%s%s", pre, room_sents[sector][x], post );
	    }
	    else
	    if ( iRand != nRand -1 )
	    {
	      if ( buf[len-1] == '.' )
	      switch( number_range(0, 3) )
	      {
		case 0:	pre = "you notice ";	post = ".";	 break;
		case 1: pre = "you see ";	post = ", and "; break;
		case 2: pre = "you see ";	post = ".";	 break;
		case 3: pre = "over yonder ";	post = ", and "; break;
	      }
	      else
	      switch( number_range(0, 3) )
	      {
		case 0:	pre = ""; post = ".";			break;
		case 1:	pre = ""; post = " not too far away.";	break;
		case 2: pre = ""; post = ", and ";		break;
		case 3: pre = ""; post = " nearby.";		break;
	      }
	      sprintf( buf2, "%s%s%s", pre, room_sents[sector][x], post );
	    }
	    else
		sprintf( buf2, "%s.", room_sents[sector][x] );
	    if ( len > 5 && buf[len-1] == '.' )
	    {
		strcat( buf, "  " );
		buf2[0] = UPPER(buf2[0] );
	    }
	    else
	    if ( len == 0 )
	        buf2[0] = UPPER(buf2[0] );
	    strcat( buf, buf2 );
	}
    }
    sprintf( buf2, "%s\n\r", wordwrap(buf, 78) );
    room->description = STRALLOC( buf2 );
}

/*
 * Remove any unused virtual rooms				-Thoric
 */
void clear_vrooms( )
{
    int hash;
    ROOM_INDEX_DATA *room, *room_next, *prev;

    for ( hash = 0; hash < 64; hash++ )
    {
	while ( vroom_hash[hash]
	&&     !vroom_hash[hash]->first_person
	&&     !vroom_hash[hash]->first_content )
	{
	    room = vroom_hash[hash];
	    vroom_hash[hash] = room->next;
	    clean_room( room );
	    DISPOSE( room );
	    --top_vroom;
	}
	prev = NULL;
	for ( room = vroom_hash[hash]; room; room = room_next )
	{
	    room_next = room->next;
	    if ( !room->first_person && !room->first_content )
	    {
		if ( prev )
		  prev->next = room_next;
		clean_room( room );
		DISPOSE( room );
		--top_vroom;
	    }
	    if ( room )
	      prev = room;
	}
    }
}

/*
 * Function to get the equivelant exit of DIR 0-MAXDIR out of linked list.
 * Made to allow old-style diku-merc exit functions to work.	-Thoric
 */
EXIT_DATA *get_exit( ROOM_INDEX_DATA *room, sh_int dir )
{
    EXIT_DATA *xit;

    if ( !room )
    {
	bug( "Get_exit: NULL room", 0 );
	return NULL;
    }

    for (xit = room->first_exit; xit; xit = xit->next )
       if ( xit->vdir == dir )
         return xit;
    return NULL;
}

/*
 * Function to get an exit, leading the the specified room
 */
EXIT_DATA *get_exit_to( ROOM_INDEX_DATA *room, sh_int dir, int vnum )
{
    EXIT_DATA *xit;

    if ( !room )
    {
	bug( "Get_exit: NULL room", 0 );
	return NULL;
    }

    for (xit = room->first_exit; xit; xit = xit->next )
       if ( xit->vdir == dir && xit->vnum == vnum )
         return xit;
    return NULL;
}

/*
 * Function to get the nth exit of a room			-Thoric
 */
EXIT_DATA *get_exit_num( ROOM_INDEX_DATA *room, sh_int count )
{
    EXIT_DATA *xit;
    int cnt;

    if ( !room )
    {
	bug( "Get_exit: NULL room", 0 );
	return NULL;
    }

    for (cnt = 0, xit = room->first_exit; xit; xit = xit->next )
       if ( ++cnt == count )
         return xit;
    return NULL;
}


/*
 * Modify movement due to encumbrance				-Thoric
 */
sh_int encumbrance( CHAR_DATA *ch, sh_int move )
{
    int cur, max;

    max = can_carry_w(ch);
    cur = ch->carry_weight;
    if ( cur >= max )
      return move * 4;
    else
    if ( cur >= max * 0.95 )
      return move * 3.5;
    else
    if ( cur >= max * 0.90 )
      return move * 3;
    else
    if ( cur >= max * 0.85 )
      return move * 2.5;
    else
    if ( cur >= max * 0.80 )
      return move * 2;
    else
    if ( cur >= max * 0.75 )
      return move * 1.5;
    else
      return move;
}


/*
 * Check to see if a character can fall down, checks for looping   -Thoric
 */
bool will_fall( CHAR_DATA *ch, int fall )
{
if ( ! ch )
return FALSE;

if ( !ch->in_room || !ch->in_room->room_flags )
return FALSE;

    if ( IS_SET( ch->in_room->room_flags, ROOM_NOFLOOR )
    &&   CAN_GO(ch, DIR_DOWN)
    && (!IS_AFFECTED( ch, AFF_FLYING )
    || ( ch->mount && !IS_AFFECTED( ch->mount, AFF_FLYING ) ) ) )
    {
	if ( fall > 80 )
	{
	   bug( "Falling (in a loop?) more than 80 rooms: vnum %d", ch->in_room->vnum );
	   char_from_room( ch );
	   char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE, 1 ) );
	   fall = 0;
	   return TRUE;
	}
	set_char_color( AT_FALLING, ch );
	send_to_char( "You're falling down...\n\r", ch );
	move_char( ch, get_exit(ch->in_room, DIR_DOWN), ++fall, FALSE );
	return TRUE;
    }
    return FALSE;
}


/*
 * create a 'virtual' room					-Thoric
 */
ROOM_INDEX_DATA *generate_exit( ROOM_INDEX_DATA *in_room, EXIT_DATA **pexit )
{
    EXIT_DATA *xit, *bxit;
    EXIT_DATA *orig_exit = (EXIT_DATA *) *pexit;
    ROOM_INDEX_DATA *room, *backroom;
    int brvnum;
    int serial;
    int roomnum;
    int distance = -1;
    int vdir = orig_exit->vdir;
    sh_int hash;
    bool found = FALSE;

    if ( in_room->vnum > 32767 )	/* room is virtual */
    {
	serial = in_room->vnum;
	roomnum = in_room->tele_vnum;
	if ( (serial & 65535) == orig_exit->vnum )
	{
	  brvnum = serial >> 16;
	  --roomnum;
	  distance = roomnum;
	}
	else
	{
	  brvnum = serial & 65535;
	  ++roomnum;
	  distance = orig_exit->distance - 1;
	}
	backroom = get_room_index( brvnum,(int)in_room->area->zone->number);
    }
    else
    {
	int r1 = in_room->vnum;
	int r2 = orig_exit->vnum;

	brvnum = r1;
	backroom = in_room;
	serial = (UMAX( r1, r2 ) << 16) | UMIN( r1, r2 );
	distance = orig_exit->distance - 1;
	roomnum = r1 < r2 ? 1 : distance;
    }
    hash = serial % 64;
    
    for ( room = vroom_hash[hash]; room; room = room->next )
	if ( room->vnum == serial && room->tele_vnum == roomnum )
	{
	    found = TRUE;
	    break;
	}
    if ( !found )
    {
	CREATE( room, ROOM_INDEX_DATA, 1 );
	room->area	  = in_room->area;
	room->vnum	  = serial;
	room->tele_vnum	  = roomnum;
	room->sector_type = in_room->sector_type;
	room->room_flags  = in_room->room_flags;
	decorate_room( room );
	room->next	  = vroom_hash[hash];
	vroom_hash[hash]  = room;
	++top_vroom;
    }
    if ( !found || (xit=get_exit(room, vdir))==NULL )
    {
	xit = make_exit(room, orig_exit->to_room, vdir);
	xit->keyword		= STRALLOC( "" );
	xit->description	= STRALLOC( "" );
	xit->key		= -1;
	xit->distance = distance;
    }
    if ( !found )
    {
	bxit = make_exit(room, backroom, rev_dir[vdir]);
	bxit->keyword		= STRALLOC( "" );
	bxit->description	= STRALLOC( "" );
	bxit->key		= -1;
	if ( (serial & 65535) != orig_exit->vnum )
	  bxit->distance = roomnum;
	else
	{
	  EXIT_DATA *tmp = get_exit( backroom, vdir );
	  int fulldist = tmp->distance;
	  
	  bxit->distance = fulldist - distance;
	}
    }
    (EXIT_DATA *) pexit = xit;
    return room;
}

ch_ret move_char( CHAR_DATA *ch, EXIT_DATA *pexit, int fall, bool follower_loop )
{
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *to_room;
    ROOM_INDEX_DATA *from_room;
    char buf[MAX_STRING_LENGTH];
    char *txt;
    char *dtxt;
    ch_ret retcode;
    sh_int door, distance;
    bool drunk = FALSE;
    bool brief = FALSE;
    bool fMatch = FALSE;
    int dir = 0;
    CHAR_DATA *mob;
    CHAR_DATA *mob_next;

    if ( !ch )
    {
        bug("Move_Char: NULL ch",0);
	return rNONE;
    }

if ( !IS_NPC(ch) )
{
    if(IS_SET(ch->pcdata->flags,PCFLAG_SEVERED))
    {
        send_to_char("Move without legs? How?\n\r",ch);
        return rNONE;
    }
}

if ( IS_NPC(ch) && IS_SET(ch->acttwo,ACT2_SEVERED) )
return rNONE;

    if ( !IS_NPC( ch ) )
      if ( IS_DRUNK( ch, 2 ) && ( ch->position != POS_SHOVE )
	&& ( ch->position != POS_DRAG ) )
	drunk = TRUE;

    if ( drunk && !fall )
    {
      door = number_door();
      pexit = get_exit( ch->in_room, door );
    }

#ifdef DEBUG
    if ( pexit )
    {
	fprintf( stderr, "move_char: %s to door %d", ch->name, pexit->vdir);
	log_string( buf );
    }
#endif

    retcode = rNONE;
    txt = NULL;

    if ( IS_NPC(ch) && IS_SET( ch->act, ACT_MOUNTED ) )
      return retcode;

    in_room = ch->in_room;
    from_room = in_room;
    if ( !pexit || (to_room = pexit->to_room) == NULL )
    {
	if ( drunk )
	  send_to_char( "You hit a wall in your drunken state.\n\r", ch );
	 else
	  send_to_char( "Alas, you cannot go that way.\n\r", ch );
	return rNONE;
    }

    door = pexit->vdir;
    distance = pexit->distance;

    /*
     * Exit is only a "window", there is no way to travel in that direction
     * unless it's a door with a window in it		-Thoric
     */
    if ( IS_SET( pexit->exit_info, EX_WINDOW )
    &&  !IS_SET( pexit->exit_info, EX_ISDOOR ) )
    {
	send_to_char( "Alas, you cannot go that way.\n\r", ch );
	return rNONE;
    }

    if (  IS_SET(pexit->exit_info, EX_PORTAL) 
       && IS_NPC(ch) )
    {
        act( AT_PLAIN, "Mobs can't use portals.", ch, NULL, NULL, TO_CHAR );
	return rNONE;
    }

    if ( IS_SET(pexit->exit_info, EX_NOMOB)
	&& IS_NPC(ch) )
    {
	act( AT_PLAIN, "Mobs can't enter there.", ch, NULL, NULL, TO_CHAR );
	return rNONE;
    }

    if ( IS_SET(pexit->exit_info, EX_CLOSED)
    && (!IS_AFFECTED(ch, AFF_PASS_DOOR)
    ||   IS_SET(pexit->exit_info, EX_NOPASSDOOR)) )
    {
	if ( !IS_SET( pexit->exit_info, EX_SECRET )
	&&   !IS_SET( pexit->exit_info, EX_DIG ) )
	{
	  if ( drunk )
	  {
	    act( AT_PLAIN, "$n runs into the $d in $s drunken state.", ch,
		NULL, pexit->keyword, TO_ROOM );
	    act( AT_PLAIN, "You run into the $d in your drunken state.", ch,
		NULL, pexit->keyword, TO_CHAR ); 
	  }
	 else
	  act( AT_PLAIN, "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
	}
       else
	{
	  if ( drunk )
	    send_to_char( "You hit a wall in your drunken state.\n\r", ch );
	   else
	    send_to_char( "Alas, you cannot go that way.\n\r", ch );
	}

	return rNONE;
    }

    
    /*
     * Crazy virtual room idea, created upon demand.		-Thoric
     */
    if ( distance > 1 )
	if ( (to_room=generate_exit(in_room, &pexit)) == NULL )
	    send_to_char( "Alas, you cannot go that way.\n\r", ch );

    if ( !fall
    &&   IS_AFFECTED(ch, AFF_CHARM)
    &&   ch->master
    &&   in_room == ch->master->in_room )
    {
	send_to_char( "What?  And leave your beloved master?\n\r", ch );
	return rNONE;
    }

    if ( room_is_private( to_room ) )
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return rNONE;
    }

	/* closed for renovations ... GW*/
	if (IS_SET( to_room->area->flags, AFLAG_CLOSED) &&
	!IS_IMMORTAL(ch) )
	{
	send_to_char("This Zone is closed for renovations.\n\r",ch);
	return rNONE;
	}

        /* Room Levels --GW */
    if ( to_room->level > ch->level )
      {
       send_to_char("Your not allowed in there!!\n\r",ch);
       return rNONE;
      }
    
      area_reset_check(to_room);

	/* Warrent Checks! --GW */
	  arrest(ch);
	 if( check_warrents(ch)==TRUE )
	   return rNONE;

     /* Blocking Mobs Cannot be fled from! --GW */
	fMatch = FALSE;
	dir = door;
	if ( !IS_NPC(ch) )
        {
	for ( mob = ch->in_room->first_person; mob; mob = mob_next )
	{
	   mob_next = mob->next_in_room;

	   if ( !IS_NPC(mob) || (mob == ch) )
	     continue;

	   if ( IS_SET( mob->acttwo, ACT2_BNORTH ) && ( dir == DIR_NORTH ) )
	   {
	     do_say(mob,"Haha! You'll have to do more then that to get past me!");
	     fMatch = TRUE;
	     break;
	   }
	   if ( IS_SET( mob->acttwo, ACT2_BSOUTH ) && ( dir == DIR_SOUTH ) )
	   {
	     do_say(mob,"Haha! You'll have to do more then that to get past me!");
	     fMatch = TRUE;
	     break;
	   }
	   if ( IS_SET( mob->acttwo, ACT2_BEAST ) && ( dir == DIR_EAST ) )
	   {
	     do_say(mob,"Haha! You'll have to do more then that to get past me!");
	     fMatch = TRUE;
	     break;
	   }
	   if ( IS_SET( mob->acttwo, ACT2_BWEST ) && ( dir == DIR_WEST ) )
	   {
	     do_say(mob,"Haha! You'll have to do more then that to get past me!");
	     fMatch = TRUE;
	     break;
	   }
	   if ( IS_SET( mob->acttwo, ACT2_BUP ) && ( dir == DIR_UP ) )
	   {
	     do_say(mob,"Haha! You'll have to do more then that to get past me!");
	     fMatch = TRUE;
	     break;
	   }
	   if ( IS_SET( mob->acttwo, ACT2_BDOWN ) && ( dir == DIR_DOWN ) )
	   {
	     do_say(mob,"Haha! You'll have to do more then that to get past me!");
	     fMatch = TRUE;
	     break;
	   }
	}

	/* No Way Man! That direction is BLOCKED .. kick us out! */
	if ( fMatch )
	   return rNONE;
      }

    if ( ch->in_room->sector_type == 15 )
      {
       do_checkout(ch,"");
      }

    if ( !IS_IMMORTAL(ch)
    &&  !IS_NPC(ch)
    &&  ch->in_room->area != to_room->area )
    {
      if ( area_level_check( ch, to_room->area )==FALSE)
        return rNONE;
    }
        
    if ( !fall && !IS_NPC(ch) )
    {
	/*int iClass;*/
	int move;

/* Prevent deadlies from entering a nopkill-flagged area from a 
   non-flagged area, but allow them to move around if already
   inside a nopkill area. - Blodkai
*/

        if (  IS_SET( to_room->area->flags, AFLAG_NOPKILL )
        &&   !IS_SET( ch->in_room->area->flags, AFLAG_NOPKILL )
        && (  IS_PKILL (ch) && !IS_IMMORTAL (ch) ) )
        {
	    set_char_color( AT_MAGIC, ch );
            send_to_char( "\n\rA godly force forbids deadly characters from entering that area...\n\r", ch);
            return rNONE;
        }

	if ( in_room->sector_type == SECT_AIR
	||   to_room->sector_type == SECT_AIR
	||   IS_SET( pexit->exit_info, EX_FLY ) )
	{
	    if ( ch->mount && !IS_AFFECTED( ch->mount, AFF_FLYING ) )
	    {
		send_to_char( "Your mount can't fly.\n\r", ch );
		return rNONE;
	    }
	    if ( !ch->mount && !IS_AFFECTED(ch, AFF_FLYING) )
	    {
		send_to_char( "You'd need to fly to go there.\n\r", ch );
		return rNONE;
	    }
	}

	if ( in_room->sector_type == SECT_WATER_NOSWIM
	||   to_room->sector_type == SECT_WATER_NOSWIM )
	{
	    OBJ_DATA *obj;
	    bool found;

	    found = FALSE;
	    if ( ch->mount )
	    {
		if ( IS_AFFECTED( ch->mount, AFF_FLYING )
		||   IS_AFFECTED( ch->mount, AFF_FLOATING ) )
		  found = TRUE;
	    }
	    else
	    if ( IS_AFFECTED(ch, AFF_FLYING)
	    ||   IS_AFFECTED(ch, AFF_FLOATING) )
		found = TRUE;

	    /*
	     * Look for a boat.
	     */
	    if ( !found )
	      for ( obj = ch->first_carrying; obj; obj = obj->next_content )
	      {
		 if ( obj->item_type == ITEM_BOAT )
		 {
		    found = TRUE;
		    if ( drunk )
		      txt = "paddles unevenly";
		     else
		      txt = "paddles";
		    break;
		 }
	      }

	    if ( !found )
	    {
		send_to_char( "You'd need a boat to go there.\n\r", ch );
		return rNONE;
	    }
	}

	if ( IS_SET( pexit->exit_info, EX_CLIMB ) )
	{
	    bool found;

	    found = FALSE;
	    if ( ch->mount && IS_AFFECTED( ch->mount, AFF_FLYING ) )
	      found = TRUE;
	    else
	    if ( IS_AFFECTED(ch, AFF_FLYING) )
	      found = TRUE;

	    if ( !found && !ch->mount )
	    {
		if ( ( !IS_NPC(ch) && number_percent( ) > ch->pcdata->learned[gsn_climb] )
		||      drunk || ch->mental_state < -90 )
		{
		   send_to_char( "You start to climb... but lose your grip and fall!\n\r", ch);
		   learn_from_failure( ch, gsn_climb );
		   if ( pexit->vdir == DIR_DOWN )
		   {
			retcode = move_char( ch, pexit, 1, FALSE );
			return retcode;
		   }
		   set_char_color( AT_HURT, ch );
		   send_to_char( "OUCH! You hit the ground!\n\r", ch );
		   WAIT_STATE( ch, 20 );
		   retcode = damage( ch, ch, (pexit->vdir == DIR_UP ? 10 : 5),
					TYPE_UNDEFINED );
		   return retcode;
		}
		found = TRUE;
		learn_from_success( ch, gsn_climb );
		WAIT_STATE( ch, skill_table[gsn_climb]->beats );
		txt = "climbs";
	    }

	    if ( !found )
	    {
		send_to_char( "You can't climb.\n\r", ch );
		return rNONE;
	    }
	}

	if ( ch->mount )
	{
	  switch (ch->mount->position)
	  {
	    case POS_DEAD:
            send_to_char( "Your mount is dead!\n\r", ch );
	    return rNONE;
            break;
        
            case POS_MORTAL:
            case POS_INCAP:
            send_to_char( "Your mount is hurt far too badly to move.\n\r", ch );
	    return rNONE;
            break;
            
            case POS_STUNNED:
            send_to_char( "Your mount is too stunned to do that.\n\r", ch );
     	    return rNONE;
            break;
       
            case POS_SLEEPING:
            send_to_char( "Your mount is sleeping.\n\r", ch );
	    return rNONE;
            break;
         
            case POS_RESTING:
            send_to_char( "Your mount is resting.\n\r", ch);
	    return rNONE;
            break;
        
            case POS_SITTING:
            send_to_char( "Your mount is sitting down.\n\r", ch);
	    return rNONE;
            break;

	    default:
	    break;
  	  }

	  if ( !IS_AFFECTED(ch->mount, AFF_FLYING)
	  &&   !IS_AFFECTED(ch->mount, AFF_FLOATING) )
	    move = movement_loss[UMIN(SECT_MAX-1, in_room->sector_type)];
	  else
	    move = 1;
	  if ( ch->mount->move < move )
	  {
	    send_to_char( "Your mount is too exhausted.\n\r", ch );
	    return rNONE;
	  }
	}
	else
	{
	  if ( !IS_AFFECTED(ch, AFF_FLYING)
	  &&   !IS_AFFECTED(ch, AFF_FLOATING) )
	    move = encumbrance( ch, movement_loss[UMIN(SECT_MAX-1, in_room->sector_type)] );
	  else
	    move = 1;
	  if ( ch->move < move )
	  {
	    send_to_char( "You are too exhausted.\n\r", ch );
	    return rNONE;
	  }
	}

	WAIT_STATE( ch, move );
	if ( ch->mount )
	  ch->mount->move -= move;
	else
	  ch->move -= move;
    }

    /*
     * Check if player can fit in the room
     */
    if ( to_room->tunnel > 0 )
    {
	CHAR_DATA *ctmp;
	int count = ch->mount ? 1 : 0;
	
	for ( ctmp = to_room->first_person; ctmp; ctmp = ctmp->next_in_room )
	  if ( ++count >= to_room->tunnel )
	  {
		if ( ch->mount && count == to_room->tunnel )
		  send_to_char( "There is no room for both you and your mount in there.\n\r", ch );
		else
		  send_to_char( "There is no room for you in there.\n\r", ch );
		return rNONE;
	  }
    }

    /* check for traps on exit - later */

    if ( !IS_AFFECTED(ch, AFF_SNEAK)
    && ( IS_NPC(ch) || !IS_SET(ch->act, PLR_WIZINVIS) ) )
    {
      if ( fall )
        txt = "falls";
      else
      if ( !txt )
      {
        if ( ch->mount )
        {
	  if ( IS_AFFECTED( ch->mount, AFF_FLOATING ) )
	    txt = "floats";
 	  else
	  if ( IS_AFFECTED( ch->mount, AFF_FLYING ) )
	    txt = "flys";
	  else
	    txt = "rides";
        }
        else
        {
	  if ( IS_AFFECTED( ch, AFF_FLOATING ) )
	  {
	    if ( drunk )
	      txt = "floats unsteadily";
	     else
	      txt = "floats";
	  }
	  else
	  if ( IS_AFFECTED( ch, AFF_FLYING ) )
	  {
	    if ( drunk )
	      txt = "flys shakily";
	     else
	      txt = "flys";
	  }
	  else
          if ( ch->position == POS_SHOVE )
            txt = "is shoved";
 	  else
	  if ( ch->position == POS_DRAG )
            txt = "is dragged";
  	  else
	  {
	    if ( drunk )
	      txt = "stumbles drunkenly";
	     else
	      txt = "leaves";
	  }
        }
      }

      /* underwater zones, drunk supported --GW*/
      if ( ch->in_room && ( ch->in_room->sector_type == 8 ) )      
      {
	if ( drunk )
	txt = "doggy paddles drunkenly";
       else
	txt = "swims";
      }
      if ( ch->mount )
      {
	sprintf( buf, "$n %s %s upon $N.", txt, dir_name[door] );
	act( AT_ACTION, buf, ch, NULL, ch->mount, TO_NOTVICT );
      }
      else
      {
	sprintf( buf, "$n %s $T.", txt );
	act( AT_ACTION, buf, ch, NULL, dir_name[door], TO_ROOM );
      }
    }

    rprog_leave_trigger( ch );
    if( char_died(ch) )
      return global_retcode;

    mprog_greet_trigger( ch );
    if ( ch && ( char_died(ch) ||  ch->fighting ) )
       return retcode;

    char_from_room( ch );
    if ( ch->mount )
    {
      rprog_leave_trigger( ch->mount );
      if( char_died(ch) )
        return global_retcode;
      if( ch->mount )
      {
        char_from_room( ch->mount );
        char_to_room( ch->mount, to_room );
      }
    }

    char_to_room( ch, to_room );

    if ( !IS_AFFECTED(ch, AFF_SNEAK)
    && ( IS_NPC(ch) || !IS_SET(ch->act, PLR_WIZINVIS) ) )
    {
      if ( fall )
        txt = "falls";
      else
      if ( ch->mount )
      {
	if ( IS_AFFECTED( ch->mount, AFF_FLOATING ) )
	  txt = "floats in";
	else
	if ( IS_AFFECTED( ch->mount, AFF_FLYING ) )
	  txt = "flys in";
	else
	  txt = "rides in";
      }
      else
      {
	if ( IS_AFFECTED( ch, AFF_FLOATING ) )
	{
	  if ( drunk )
	    txt = "floats in unsteadily";
	   else
	    txt = "floats in";
	}
	else
	if ( IS_AFFECTED( ch, AFF_FLYING ) )
	{
	  if ( drunk )
	    txt = "flys in shakily";
	   else
	    txt = "flys in";
	}
	else
	if ( ch->position == POS_SHOVE )
          txt = "is shoved in";
	else
	if ( ch->position == POS_DRAG )
	  txt = "is dragged in";
  	else
	{
	  if ( drunk )
	    txt = "stumbles drunkenly in";
	   else
	    txt = "arrives";
	}
      }
      switch( door )
      {
      default: dtxt = "somewhere";	break;
      case 0:  dtxt = "the south";	break;
      case 1:  dtxt = "the west";	break;
      case 2:  dtxt = "the north";	break;
      case 3:  dtxt = "the east";	break;
      case 4:  dtxt = "below";		break;
      case 5:  dtxt = "above";		break;
      case 6:  dtxt = "the south-west";	break;
      case 7:  dtxt = "the south-east";	break;
      case 8:  dtxt = "the north-west";	break;
      case 9:  dtxt = "the north-east";	break;
      }

	if ( txt[0] == '\0' )
	{
	bug("DEBUG: Move_char: txt is NULL! TELL GREYWOLF!!",0); 
	return retcode;
	}

	if ( dtxt[0] == '\0' )
	{
	bug("DEBUG: Move_char: dtxt is NULL! TELL GREYWOLF!!",0);
	return retcode;
	}

	if ( ch == NULL )
	{
	bug("DUBUG: Move_char: CH == NULL! BAD BAD!! TELL GW!!",0);
	return retcode;
	}

      if ( ch->mount )
      {
	sprintf( buf, "$n %s from %s upon $N.", txt, dtxt );
	act( AT_ACTION, buf, ch, NULL, ch->mount, TO_ROOM );
      }
      else
      {
	sprintf( buf, "$n %s from %s.", txt, dtxt );
	act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
      }
    }

    if ( !IS_IMMORTAL(ch)
    &&  !IS_NPC(ch)
    &&  ch->in_room->area != to_room->area )
    {
	if ( ch->level < to_room->area->low_soft_range )
	{
	   set_char_color( AT_MAGIC, ch );
	   send_to_char("You feel uncomfortable being in this strange land...\n\r", ch);
	}
	else
	if ( ch->level > to_room->area->hi_soft_range )
	{
	   set_char_color( AT_MAGIC, ch );
	   send_to_char("You feel there is not much to gain visiting this place...\n\r", ch);
	}
    }

    /* Make sure everyone sees the room description of death traps. */
    if ( IS_SET( ch->in_room->room_flags, ROOM_DEATH ) && !IS_IMMORTAL( ch ) )
    {
      if( IS_SET( ch->act, PLR_BRIEF ) )
        brief = TRUE;
      REMOVE_BIT( ch->act, PLR_BRIEF );
    }

    do_look( ch, "auto" );
    if ( brief ) 
      SET_BIT( ch->act, PLR_BRIEF );

    if ( !IS_NPC(ch) && IS_SET(ch->pcdata->flagstwo, MOREPC_ALIENS) && dt_scan(ch)==TRUE )
	send_to_char("\n\rYou sence that immediate death is very near..\n\r",ch);

    /* Hoarder Stuff --GW */
    if ( IS_HOARDER(ch) )
      hoarder_check(ch,NULL,HCHECK_MOVE);

    /*
     * Put good-old EQ-munching death traps back in!		-Thoric
     */
    if ( IS_SET( ch->in_room->room_flags, ROOM_DEATH ) && !IS_IMMORTAL( ch ) 
	&& !IS_NPC(ch) )
    {
       act( AT_DEAD, "$n falls prey to a terrible death!", ch, NULL, NULL, TO_ROOM );
       set_char_color( AT_DEAD, ch );
       send_to_char( "Oopsie... you're dead!\n\r", ch );
       sprintf(buf, "%s hit a DEATH TRAP in room %d!",
		     ch->name, ch->in_room->vnum );
       log_string( buf );
       to_channel( buf, CHANNEL_MONITOR, "[ ** ", LEVEL_IMMORTAL );
       do_save(ch,"");
	 make_corpse(ch,ch,TRUE);
         send_to_char("\n\r",ch);
         raw_kill(ch,ch);    
	
       return rCHAR_DIED;
    }

    /* BIG ugly looping problem here when the character is mptransed back
       to the starting room.  To avoid this, check how many chars are in 
       the room at the start and stop processing followers after doing
       the right number of them.  -- Narn
    */
    if ( !fall )
    {
      CHAR_DATA *fch;
      CHAR_DATA *nextinroom;
      int chars = 0, count = 0;

      if ( !from_room )
	{
	bug("Move_char: NULL FROM ROOM!!");
 	return rNONE;
	}

/*      for ( fch = from_room->first_person; fch; fch = fch->next_in_room )
        chars++;*/
       chars = from_room->room_cnt;
	
      for ( fch = from_room->first_person; fch && ( count < chars ); fch = nextinroom )
      {
	nextinroom = fch->next_in_room;
        count++;
	if ( fch && ch && fch != ch		/* loop room bug fix here by Thoric */
	&& fch->master == ch
	&& fch->position == POS_STANDING )
	{
	act( AT_ACTION, "You follow $N.", fch, NULL, ch, TO_CHAR );
	    move_char( fch, pexit, 0, TRUE );
	}
      }
    }
   

    if ( ch->in_room->first_content )
      retcode = check_room_for_traps( ch, TRAP_ENTER_ROOM );
    if ( retcode && retcode != rNONE )
      return retcode;

    if ( ch && char_died(ch) )
      return retcode;

    mprog_entry_trigger( ch );
    if ( ch && char_died(ch) )
      return retcode;

    rprog_enter_trigger( ch );
    if ( ch && char_died(ch) )
       return retcode;

    mprog_greet_trigger( ch );
    if (ch && char_died(ch) )
       return retcode;

    oprog_greet_trigger( ch );
    if (ch && char_died(ch) )
       return retcode;

    if ( ch && !will_fall( ch, fall )
    &&   fall > 0 )
    {
	if (!IS_AFFECTED( ch, AFF_FLOATING )
	|| ( ch->mount && !IS_AFFECTED( ch->mount, AFF_FLOATING ) ) )
	{
	  set_char_color( AT_HURT, ch );
	  send_to_char( "OUCH! You hit the ground!\n\r", ch );
	  WAIT_STATE( ch, 20 );
	  retcode = damage( ch, ch, 20 * fall, TYPE_UNDEFINED );
	}
	else
	{
	  set_char_color( AT_MAGIC, ch );
	  send_to_char( "You lightly float down to the ground.\n\r", ch );
	}
    }
    return retcode;
}


void do_north( CHAR_DATA *ch, char *argument )
{
char bug_buf[MAX_STRING_LENGTH];
CHAR_DATA *block;

    if ( !ch->in_room )
    {
    sprintf(bug_buf,"Do_North: Null In_Room!! (%s)",ch->name);
    bug(bug_buf);
    return;
    }

    if ( !IS_NPC(ch) && ch->pcdata->vehicle )
    {
      send_to_char("Use Steer, your in a vehicle!\n\r",ch);
      return;
    }

    for ( block = ch->in_room->first_person; block; block = block->next_in_room )
    {
	if ( IS_NPC(block) && block != ch )
	{
	 if ( IS_SET( block->acttwo,ACT2_BNORTH ) )
	 {
	   do_say(block,"Haha! You'll have to do more then that to get passed me!");
	   return;
	 }
	}
    }

    move_char( ch, get_exit(ch->in_room, DIR_NORTH), 0, FALSE );
    return;
}


void do_east( CHAR_DATA *ch, char *argument )
{
char bug_buf[MAX_STRING_LENGTH];
CHAR_DATA *block;

    if ( !ch->in_room )
    {
    sprintf(bug_buf,"Do_East: Null In_Room!! (%s)",ch->name);
    bug(bug_buf);
    return;
    }

    if ( !IS_NPC(ch) && ch->pcdata->vehicle )
    {
      send_to_char("Use Steer, your in a vehicle!\n\r",ch);
      return;
    }

    for ( block = ch->in_room->first_person; block; block = block->next_in_room )
    {
	if ( IS_NPC(block) && block != ch )
	{
	 if ( IS_SET( block->acttwo,ACT2_BEAST ) )
	 {
	   do_say(block,"Haha! You'll have to do more then that to get passed me!");
	   return;
	 }
	}
    }

    move_char( ch, get_exit(ch->in_room, DIR_EAST), 0, FALSE );
    return;
}


void do_south( CHAR_DATA *ch, char *argument )
{
char bug_buf[MAX_STRING_LENGTH];
CHAR_DATA *block;

    if ( !ch->in_room )
    {
    sprintf(bug_buf,"Do_South: Null In_Room!! (%s)",ch->name);
    bug(bug_buf);
    return;
    }

    if ( !IS_NPC(ch) && ch->pcdata->vehicle )
    {
      send_to_char("Use Steer, your in a vehicle!\n\r",ch);
      return;
    }

    for ( block = ch->in_room->first_person; block; block = block->next_in_room )
    {
	if ( IS_NPC(block) && block != ch )
	{
	 if ( IS_SET( block->acttwo,ACT2_BSOUTH ) )
	 {
	   do_say(block,"Haha! You'll have to do more then that to get passed me!");
	   return;
	 }
	}
    }
    move_char( ch, get_exit(ch->in_room, DIR_SOUTH), 0, FALSE );
    return;
}


void do_west( CHAR_DATA *ch, char *argument )
{
char bug_buf[MAX_STRING_LENGTH];
CHAR_DATA *block;

    if ( !ch->in_room )
    {
    sprintf(bug_buf,"Do_West: Null In_Room!! (%s)",ch->name);
    bug(bug_buf);
    return;
    }

    if ( !IS_NPC(ch) && ch->pcdata->vehicle )
    {
      send_to_char("Use Steer, your in a vehicle!\n\r",ch);
      return;
    }

    for ( block = ch->in_room->first_person; block; block = block->next_in_room )
    {
	if ( IS_NPC(block) && block != ch )
	{
	 if ( IS_SET( block->acttwo,ACT2_BWEST ) )
	 {
	   do_say(block,"Haha! You'll have to do more then that to get passed me!");
	   return;
	 }
	}
    }
    move_char( ch, get_exit(ch->in_room, DIR_WEST), 0, FALSE );
    return;
}


void do_up( CHAR_DATA *ch, char *argument )
{
char bug_buf[MAX_STRING_LENGTH];
CHAR_DATA *block;

    if ( !ch->in_room )
    {
    sprintf(bug_buf,"Do_Up: Null In_Room!! (%s)",ch->name);
    bug(bug_buf);
    return;
    }

    if ( !IS_NPC(ch) && ch->pcdata->vehicle )
    {
      send_to_char("Use Steer, your in a vehicle!\n\r",ch);
      return;
    }

    for ( block = ch->in_room->first_person; block; block = block->next_in_room )
    {
	if ( IS_NPC(block) && block != ch )
	{
	 if ( IS_SET( block->acttwo,ACT2_BUP ) )
	 {
	   do_say(block,"Haha! You'll have to do more then that to get passed me!");
	   return;
	 }
	}
    }

    move_char( ch, get_exit(ch->in_room, DIR_UP), 0, FALSE );
    return;
}


void do_down( CHAR_DATA *ch, char *argument )
{
char bug_buf[MAX_STRING_LENGTH];
CHAR_DATA *block;

    if ( !ch->in_room )
    {
    sprintf(bug_buf,"Do_Down: Null In_Room!! (%s)",ch->name);
    bug(bug_buf);
    return;
    }

    if ( !IS_NPC(ch) && ch->pcdata->vehicle )
    {
      send_to_char("Use Steer, your in a vehicle!\n\r",ch);
      return;
    }

    for ( block = ch->in_room->first_person; block; block = block->next_in_room )
    {
	if ( IS_NPC(block) && block != ch )
	{
	 if ( IS_SET( block->acttwo,ACT2_BDOWN ) )
	 {
	   do_say(block,"Haha! You'll have to do more then that to get passed me!");
	   return;
	 }
	}
    }

    move_char( ch, get_exit(ch->in_room, DIR_DOWN), 0, FALSE );
    return;
}

void do_northeast( CHAR_DATA *ch, char *argument )
{

    move_char( ch, get_exit(ch->in_room, DIR_NORTHEAST), 0, FALSE );
    return;
}

void do_northwest( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_NORTHWEST), 0, FALSE );
    return;
}

void do_southeast( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_SOUTHEAST), 0, FALSE );
    return;
}

void do_southwest( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_SOUTHWEST), 0, FALSE );
    return;
}



EXIT_DATA *find_door( CHAR_DATA *ch, char *arg, bool quiet )
{
    EXIT_DATA *pexit;
    int door;

    if (arg == NULL || !str_cmp(arg,"") || !ch || !ch->in_room )
      return NULL;

    pexit = NULL;
	 if ( !str_cmp( arg, "n"  ) || !str_cmp( arg, "north"	  ) ) door = 0;
    else if ( !str_cmp( arg, "e"  ) || !str_cmp( arg, "east"	  ) ) door = 1;
    else if ( !str_cmp( arg, "s"  ) || !str_cmp( arg, "south"	  ) ) door = 2;
    else if ( !str_cmp( arg, "w"  ) || !str_cmp( arg, "west"	  ) ) door = 3;
    else if ( !str_cmp( arg, "u"  ) || !str_cmp( arg, "up"	  ) ) door = 4;
    else if ( !str_cmp( arg, "d"  ) || !str_cmp( arg, "down"	  ) ) door = 5;
    else if ( !str_cmp( arg, "ne" ) || !str_cmp( arg, "northeast" ) ) door = 6;
    else if ( !str_cmp( arg, "nw" ) || !str_cmp( arg, "northwest" ) ) door = 7;
    else if ( !str_cmp( arg, "se" ) || !str_cmp( arg, "southeast" ) ) door = 8;
    else if ( !str_cmp( arg, "sw" ) || !str_cmp( arg, "southwest" ) ) door = 9;
    else
    {
	if ( !ch->in_room->first_exit )
	return NULL;

	for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
	{
	    if ( (quiet || IS_SET(pexit->exit_info, EX_ISDOOR))
	    &&    pexit->keyword
	    &&    nifty_is_name( arg, pexit->keyword ) )
		return pexit;
	}
	if ( !quiet )
	  act( AT_PLAIN, "You see no $T here.", ch, NULL, arg, TO_CHAR );
	return NULL;
    }

    if ( (pexit = get_exit( ch->in_room, door )) == NULL )
    {
	if ( !quiet)
	  act( AT_PLAIN, "You see no $T here.", ch, NULL, arg, TO_CHAR );
	return NULL;
    }

    if ( quiet )
	return pexit;

    if ( IS_SET(pexit->exit_info, EX_SECRET) )
    {
	act( AT_PLAIN, "You see no $T here.", ch, NULL, arg, TO_CHAR );
	return NULL;
    }

    if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
    {
	send_to_char( "You can't do that.\n\r", ch );
	return NULL;
    }

    return pexit;
}


void toggle_bexit_flag( EXIT_DATA *pexit, int flag )
{
    EXIT_DATA *pexit_rev;

    TOGGLE_BIT(pexit->exit_info, flag);
    if ( (pexit_rev = pexit->rexit) != NULL
    &&   pexit_rev != pexit )
	TOGGLE_BIT( pexit_rev->exit_info, flag );
}

void set_bexit_flag( EXIT_DATA *pexit, int flag )
{
    EXIT_DATA *pexit_rev;

    SET_BIT(pexit->exit_info, flag);
    if ( (pexit_rev = pexit->rexit) != NULL
    &&   pexit_rev != pexit )
	SET_BIT( pexit_rev->exit_info, flag );
}

void remove_bexit_flag( EXIT_DATA *pexit, int flag )
{
    EXIT_DATA *pexit_rev;

    REMOVE_BIT(pexit->exit_info, flag);
    if ( (pexit_rev = pexit->rexit) != NULL
    &&   pexit_rev != pexit )
	REMOVE_BIT( pexit_rev->exit_info, flag );
}

void do_open( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    EXIT_DATA *pexit;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Open what?\n\r", ch );
	return;
    }

    if ( ( pexit = find_door( ch, arg, TRUE ) ) != NULL )
    {
	/* 'open door' */
	EXIT_DATA *pexit_rev;

	if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
	    { send_to_char( "You can't do that.\n\r",      ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's already open.\n\r",      ch ); return; }
	if (  IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's locked.\n\r",            ch ); return; }

	if ( !IS_SET(pexit->exit_info, EX_SECRET)
	||   (pexit->keyword && nifty_is_name( arg, pexit->keyword )) )
	{
	    act( AT_ACTION, "$n opens the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	    act( AT_ACTION, "You open the $d.", ch, NULL, pexit->keyword, TO_CHAR );
	    if ( (pexit_rev = pexit->rexit) != NULL
	    &&   pexit_rev->to_room == ch->in_room )
	    {
		CHAR_DATA *rch;

		for ( rch = pexit->to_room->first_person; rch; rch = rch->next_in_room )
		    act( AT_ACTION, "The $d opens.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	    }
	    remove_bexit_flag( pexit, EX_CLOSED );
	    if ( (door=pexit->vdir) >= 0 && door < 10 )
		check_room_for_traps( ch, trap_door[door]);
            return;
	}
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'open object' */
	if ( obj->item_type != ITEM_CONTAINER )
	{ 
          ch_printf( ch, "%s is not a container.\n\r", capitalize( obj->short_descr ) ); 
          return;
        } 
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	{ 
          ch_printf( ch, "%s is already open.\n\r", capitalize( obj->short_descr ) ); 
          return;
        } 
	if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
	{ 
          ch_printf( ch, "%s cannot be opened or closed.\n\r", capitalize( obj->short_descr ) ); 
          return;
        } 
	if ( IS_SET(obj->value[1], CONT_LOCKED) )
	{ 
          ch_printf( ch, "%s is locked.\n\r", capitalize( obj->short_descr ) ); 
          return;
        } 

	REMOVE_BIT(obj->value[1], CONT_CLOSED);
	act( AT_ACTION, "You open $p.", ch, obj, NULL, TO_CHAR );
	act( AT_ACTION, "$n opens $p.", ch, obj, NULL, TO_ROOM );
	check_for_trap( ch, obj, TRAP_OPEN );
	return;
    }

    ch_printf( ch, "You see no %s here.\n\r", arg );
    return;
}



void do_close( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    EXIT_DATA *pexit;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Close what?\n\r", ch );
	return;
    }

    if ( ( pexit = find_door( ch, arg, TRUE ) ) != NULL )
    {
	/* 'close door' */
	EXIT_DATA *pexit_rev;

	if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
	    { send_to_char( "You can't do that.\n\r",      ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's already closed.\n\r",    ch ); return; }

	act( AT_ACTION, "$n closes the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	act( AT_ACTION, "You close the $d.", ch, NULL, pexit->keyword, TO_CHAR );

	/* close the other side */
	if ( ( pexit_rev = pexit->rexit ) != NULL
	&&   pexit_rev->to_room == ch->in_room )
	{
	    CHAR_DATA *rch;

	    SET_BIT( pexit_rev->exit_info, EX_CLOSED );
	    for ( rch = pexit->to_room->first_person; rch; rch = rch->next_in_room )
		act( AT_ACTION, "The $d closes.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	}
	set_bexit_flag( pexit, EX_CLOSED );
	if ( (door=pexit->vdir) >= 0 && door < 10 )
	  check_room_for_traps( ch, trap_door[door]);
        return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'close object' */
	if ( obj->item_type != ITEM_CONTAINER )
	{ 
          ch_printf( ch, "%s is not a container.\n\r", capitalize( obj->short_descr ) ); 
          return;
        } 
	if ( IS_SET(obj->value[1], CONT_CLOSED) )
	{ 
          ch_printf( ch, "%s is already closed.\n\r", capitalize( obj->short_descr ) ); 
          return;
        } 
	if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
        { 
         ch_printf( ch,"%s cannot be opened or closed.\n\r", capitalize( 
obj->short_descr ) ); 
          return;
        } 

	SET_BIT(obj->value[1], CONT_CLOSED);
	act( AT_ACTION, "You close $p.", ch, obj, NULL, TO_CHAR );
	act( AT_ACTION, "$n closes $p.", ch, obj, NULL, TO_ROOM );
	check_for_trap( ch, obj, TRAP_CLOSE );
	return;
    }

    ch_printf( ch, "You see no %s here.\n\r", arg );
    return;
}


bool has_key( CHAR_DATA *ch, int key )
{
    OBJ_DATA *obj;

    for ( obj = ch->first_carrying; obj; obj = obj->next_content )
	if ( obj->pIndexData->vnum == key || obj->value[0] == key )
	    return TRUE;

    return FALSE;
}


void do_lock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    EXIT_DATA *pexit;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Lock what?\n\r", ch );
	return;
    }

    if ( ( pexit = find_door( ch, arg, TRUE ) ) != NULL )
    {
	/* 'lock door' */

	if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
	    { send_to_char( "You can't do that.\n\r",      ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be locked.\n\r",     ch ); return; }
	if ( !has_key( ch, pexit->key) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already locked.\n\r",    ch ); return; }

	if ( !IS_SET(pexit->exit_info, EX_SECRET)
	||   (pexit->keyword && nifty_is_name( arg, pexit->keyword )) )
	{
	    send_to_char( "*Click*\n\r", ch );
	    act( AT_ACTION, "$n locks the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	    set_bexit_flag( pexit, EX_LOCKED );
            return;
        }
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'lock object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be locked.\n\r",     ch ); return; }
	if ( !has_key( ch, obj->value[2] ) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already locked.\n\r",    ch ); return; }

	SET_BIT(obj->value[1], CONT_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( AT_ACTION, "$n locks $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    ch_printf( ch, "You see no %s here.\n\r", arg );
    return;
}



void do_unlock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    EXIT_DATA *pexit;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Unlock what?\n\r", ch );
	return;
    }

    if ( ( pexit = find_door( ch, arg, TRUE ) ) != NULL )
    {
	/* 'unlock door' */

	if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
	    { send_to_char( "You can't do that.\n\r",      ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !has_key( ch, pexit->key) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

	if ( !IS_SET(pexit->exit_info, EX_SECRET)
	||   (pexit->keyword && nifty_is_name( arg, pexit->keyword )) )
	{
	    send_to_char( "*Click*\n\r", ch );
	    act( AT_ACTION, "$n unlocks the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	    remove_bexit_flag( pexit, EX_LOCKED );
            return;   
	}
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'unlock object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !has_key( ch, obj->value[2] ) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( AT_ACTION, "$n unlocks $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    ch_printf( ch, "You see no %s here.\n\r", arg );
    return;
}

void do_bashdoor( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *gch;
	EXIT_DATA *pexit;
	char       arg [ MAX_INPUT_LENGTH ];

	if ( !IS_NPC( ch )
	&& ch->level < skill_table[gsn_bashdoor]->skill_level[ch->class] )
	{
	    send_to_char( "You're not enough of a warrior to bash doors!\n\r", ch );
	    return;
	}

	one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
	    send_to_char( "Bash what?\n\r", ch );
	    return;
	}

	if ( ch->fighting )
	{
	    send_to_char( "You can't break off your fight.\n\r", ch );
	    return;
	}

	if ( ( pexit = find_door( ch, arg, FALSE ) ) != NULL )
	{
	    ROOM_INDEX_DATA *to_room;
	    EXIT_DATA       *pexit_rev;
	    int              chance;
	    char	    *keyword;

	    if ( !IS_SET( pexit->exit_info, EX_CLOSED ) )
	    {
		send_to_char( "Calm down.  It is already open.\n\r", ch );
		return;
	    }

	    WAIT_STATE( ch, skill_table[gsn_bashdoor]->beats );

	    if ( IS_SET( pexit->exit_info, EX_SECRET ) )
		keyword = "wall";
	    else
		keyword = pexit->keyword;
	    if ( !IS_NPC(ch) )
		chance = ch->pcdata->learned[gsn_bashdoor];
	    else
		chance = 90;
	    if ( IS_SET( pexit->exit_info, EX_LOCKED ) )
		chance /= 2;

	    if ( !IS_SET( pexit->exit_info, EX_BASHPROOF )
	    &&   ch->move >= 15
	    &&   number_percent( ) < ( chance + 4 * ( get_curr_str( ch ) - 19 ) ) )
	    {
		REMOVE_BIT( pexit->exit_info, EX_CLOSED );
		if ( IS_SET( pexit->exit_info, EX_LOCKED ) )
		REMOVE_BIT( pexit->exit_info, EX_LOCKED );
		SET_BIT( pexit->exit_info, EX_BASHED );

		act(AT_SKILL, "Crash!  You bashed open the $d!", ch, NULL, keyword, TO_CHAR );
		act(AT_SKILL, "$n bashes open the $d!",          ch, NULL, keyword, TO_ROOM );
		learn_from_success(ch, gsn_bashdoor);

		if ( (to_room = pexit->to_room) != NULL
		&&   (pexit_rev = pexit->rexit) != NULL
		&&    pexit_rev->to_room	== ch->in_room )
		{
			CHAR_DATA *rch;

			REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
			if ( IS_SET( pexit_rev->exit_info, EX_LOCKED ) )
			  REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
			SET_BIT( pexit_rev->exit_info, EX_BASHED );

			for ( rch = to_room->first_person; rch; rch = rch->next_in_room )
			{
			    act(AT_SKILL, "The $d crashes open!",
				rch, NULL, pexit_rev->keyword, TO_CHAR );
			}
		}
		damage( ch, ch, ( ch->max_hit / 20 ), gsn_bashdoor );

	    }
	    else
	    {
		act(AT_SKILL, "WHAAAAM!!!  You bash against the $d, but it doesn't budge.",
			ch, NULL, keyword, TO_CHAR );
		act(AT_SKILL, "WHAAAAM!!!  $n bashes against the $d, but it holds strong.",
			ch, NULL, keyword, TO_ROOM );
		damage( ch, ch, ( ch->max_hit / 20 ) + 10, gsn_bashdoor );
		learn_from_failure(ch, gsn_bashdoor);
	    }
	}    
	else
	{
	    act(AT_SKILL, "WHAAAAM!!!  You bash against the wall, but it doesn't budge.",
		ch, NULL, NULL, TO_CHAR );
	    act(AT_SKILL, "WHAAAAM!!!  $n bashes against the wall, but it holds strong.",
		ch, NULL, NULL, TO_ROOM );
	    damage( ch, ch, ( ch->max_hit / 20 ) + 10, gsn_bashdoor );
	    learn_from_failure(ch, gsn_bashdoor);
	}
	if ( !char_died( ch ) )
	   for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
	   {
		 if ( IS_AWAKE( gch )
		 && !gch->fighting
		 && ( IS_NPC( gch ) && !IS_AFFECTED( gch, AFF_CHARM ) )
		 && ( ch->level - gch->level <= 4 )
		 && number_bits( 2 ) == 0 )
		 multi_hit( gch, ch, TYPE_UNDEFINED );
	   }

        return;
}


void do_stand( CHAR_DATA *ch, char *argument )
{
    switch ( ch->position )
    {
    case POS_SLEEPING:
	if ( IS_AFFECTED(ch, AFF_SLEEP) )
	    { send_to_char( "You can't seem to wake up!\n\r", ch ); return; }

	send_to_char( "You wake and climb quickly to your feet.\n\r", ch );
	act( AT_ACTION, "$n arises from $s slumber.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_STANDING;
	break;

    case POS_RESTING:
        send_to_char( "You gather yourself and stand up.\n\r", ch );
	act( AT_ACTION, "$n rises from $s rest.", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_STANDING;
        break;

    case POS_SITTING:
	send_to_char( "You move quickly to your feet.\n\r", ch );
	act( AT_ACTION, "$n rises up.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_STANDING;
	break;

    case POS_STANDING:
	send_to_char( "You are already standing.\n\r", ch );
	break;

    case POS_FIGHTING:
	send_to_char( "You are already fighting!\n\r", ch );
	break;
    }

       if ( ch->in_room->vnum == BLACKJACK )
	do_blackjack_exit(ch);

    return;
}


void do_sit( CHAR_DATA *ch, char *argument )
{
    switch ( ch->position )
    {
    case POS_SLEEPING:
	if ( IS_AFFECTED(ch, AFF_SLEEP) )
            { send_to_char( "You can't seem to wake up!\n\r", ch ); return; }

	send_to_char( "You wake and sit up.\n\r", ch );
	act( AT_ACTION, "$n wakes and sits up.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_SITTING;
	break;

    case POS_RESTING:
        send_to_char( "You stop resting and sit up.\n\r", ch );
	act( AT_ACTION, "$n stops resting and sits up.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_SITTING;
	break;

    case POS_STANDING:
        send_to_char( "You sit down.\n\r", ch );
	act( AT_ACTION, "$n sits down.", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_SITTING;
        break;
    case POS_SITTING:
	send_to_char( "You are already sitting.\n\r", ch );
	return;

    case POS_FIGHTING:
	send_to_char( "You are busy fighting!\n\r", ch );
	return;
    case POS_MOUNTED:
        send_to_char( "You are already sitting - on your mount.\n\r", ch );
        return;
    }
       if ( ch->in_room->vnum == BLACKJACK )
	do_blackjack_enter(ch);

    return;
}


void do_rest( CHAR_DATA *ch, char *argument )
{
    switch ( ch->position )
    {
    case POS_SLEEPING:
	if ( IS_AFFECTED(ch, AFF_SLEEP) )
            { send_to_char( "You can't seem to wake up!\n\r", ch ); return; }

	send_to_char( "You rouse from your slumber.\n\r", ch );
	act( AT_ACTION, "$n rouses from $s slumber.", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_RESTING;
	break;

    case POS_RESTING:
	send_to_char( "You are already resting.\n\r", ch );
	return;

    case POS_STANDING:
        send_to_char( "You sprawl out haphazardly.\n\r", ch );
	act( AT_ACTION, "$n sprawls out haphazardly.", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_RESTING;
        break;

    case POS_SITTING:
        send_to_char( "You lie back and sprawl out to rest.\n\r", ch );
	act( AT_ACTION, "$n lies back and sprawls out to rest.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_RESTING;
	break;

    case POS_FIGHTING:
	send_to_char( "You are busy fighting!\n\r", ch );
	return;
    case POS_MOUNTED:
        send_to_char( "You'd better dismount first.\n\r", ch );
        return;
    }

    rprog_rest_trigger( ch );
    return;
}


void do_sleep( CHAR_DATA *ch, char *argument )
{
    switch ( ch->position )
    {
    case POS_SLEEPING:
	send_to_char( "You are already sleeping.\n\r", ch );
	return;

    case POS_RESTING:
	if ( ch->mental_state > 30 && (number_percent()+10) < ch->mental_state )
	{
	    send_to_char( "You just can't seem to calm yourself down enough to sleep.\n\r", ch );
	    act( AT_ACTION, "$n closes $s eyes for a few moments, but just can't seem to go to sleep.", ch, NULL, NULL, TO_ROOM );
	    return;
	}
        send_to_char( "You close your eyes and drift into slumber.\n\r", ch );
	act( AT_ACTION, "$n closes $s eyes and drifts into a deep slumber.", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_SLEEPING;
        break;

    case POS_SITTING:
	if ( ch->mental_state > 30 && (number_percent()+5) < ch->mental_state )
	{
	    send_to_char( "You just can't seem to calm yourself down enough to sleep.\n\r", ch );
	    act( AT_ACTION, "$n closes $s eyes for a few moments, but just can't seem to go to sleep.", ch, NULL, NULL, TO_ROOM );
	    return;
	}
        send_to_char( "You slump over and fall dead asleep.\n\r", ch );
	act( AT_ACTION, "$n nods off and slowly slumps over, dead asleep.", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_SLEEPING;
        break;

    case POS_STANDING: 
	if ( ch->mental_state > 30 && number_percent() < ch->mental_state )
	{
	    send_to_char( "You just can't seem to calm yourself down enough to sleep.\n\r", ch );
	    act( AT_ACTION, "$n closes $s eyes for a few moments, but just can't seem to go to sleep.", ch, NULL, NULL, TO_ROOM );
	    return;
	}
        send_to_char( "You collapse into a deep sleep.\n\r", ch );
	act( AT_ACTION, "$n collapses into a deep sleep.", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_SLEEPING;
        break;

    case POS_FIGHTING:
	send_to_char( "You are busy fighting!\n\r", ch );
	return;
    case POS_MOUNTED:
        send_to_char( "You really should dismount first.\n\r", ch );
        return;
    }

    rprog_sleep_trigger( ch );
    return;
}


void do_wake( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
	{ do_stand( ch, argument ); return; }

    if ( !IS_AWAKE(ch) )
	{ send_to_char( "You are asleep yourself!\n\r",       ch ); return; }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{ send_to_char( "They aren't here.\n\r",              ch ); return; }

    if ( IS_AWAKE(victim) )
    { act( AT_PLAIN, "$N is already awake.", ch, NULL, victim, TO_CHAR ); return; }

    if ( IS_AFFECTED(victim, AFF_SLEEP) || victim->position < POS_SLEEPING )
    { act( AT_PLAIN, "You can't seem to wake $M!",  ch, NULL, victim, TO_CHAR );  return; }

    act( AT_ACTION, "You wake $M.", ch, NULL, victim, TO_CHAR );
    victim->position = POS_STANDING;
    act( AT_ACTION, "$n wakes you.", ch, NULL, victim, TO_VICT );
    return;
}


/*
 * teleport a character to another room
 */
void teleportch( CHAR_DATA *ch, ROOM_INDEX_DATA *room, bool show )
{
    char buf[MAX_STRING_LENGTH];

    if ( room_is_private( room ) )
      return;
    act( AT_ACTION, "$n disappears suddenly!", ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, room );
    act( AT_ACTION, "$n arrives suddenly!", ch, NULL, NULL, TO_ROOM );
    if ( show )
      do_look( ch, "auto" );
    if ( IS_SET( ch->in_room->room_flags, ROOM_DEATH ) && !IS_IMMORTAL( ch ) )
    {
       act( AT_DEAD, "$n falls prey to a terrible death!", ch, NULL, NULL, TO_ROOM );
       set_char_color( AT_DEAD, ch );
       send_to_char( "Oopsie... you're dead!\n\r", ch );
       sprintf(buf, "%s hit a DEATH TRAP in room %d!",
		     ch->name, ch->in_room->vnum );
       log_string( buf );
       to_channel( buf, CHANNEL_MONITOR, "[ ** ", LEVEL_IMMORTAL );
       extract_char( ch, FALSE );
    }
}

void teleport( CHAR_DATA *ch, sh_int room, int flags )
{
    CHAR_DATA *nch, *nch_next;
    ROOM_INDEX_DATA *pRoomIndex;
    bool show;

    pRoomIndex = get_room_index( room,(int)ch->in_room->area->zone->number);
    if ( !pRoomIndex )
    {
	bug( "teleport: bad room vnum %d", room );
	return;
    }

    if ( IS_SET( flags, TELE_SHOWDESC ) )
      show = TRUE;
    else
      show = FALSE;
    if ( !IS_SET( flags, TELE_TRANSALL ) )
    {
	teleportch( ch, pRoomIndex, show );
	return;
    }
    for ( nch = ch->in_room->first_person; nch; nch = nch_next )
    {
	nch_next = nch->next_in_room;
	teleportch( nch, pRoomIndex, show );
    }
}

/*
 * "Climb" in a certain direction.				-Thoric
 */
void do_climb( CHAR_DATA *ch, char *argument )
{
    EXIT_DATA *pexit;
    bool found;

    found = FALSE;
    if ( argument[0] == '\0' )
    {
	for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
	    if ( IS_SET( pexit->exit_info, EX_xCLIMB ) )
	    {
		move_char( ch, pexit, 0, FALSE );
		return;
	    }
	send_to_char( "You cannot climb here.\n\r", ch );
	return;
    }

    if ( (pexit = find_door( ch, argument, TRUE )) != NULL
    &&   IS_SET( pexit->exit_info, EX_xCLIMB ))
    {
	move_char( ch, pexit, 0, FALSE );
	return;
    }
    send_to_char( "You cannot climb there.\n\r", ch );
    return;
}

/*
 * "enter" something (moves through an exit)			-Thoric
 */
void do_enter( CHAR_DATA *ch, char *argument )
{
    EXIT_DATA *pexit;
    bool found;
    OBJ_DATA *obj, *obj_next;
    char buf[MSL];
    
    found = FALSE;
    if ( argument[0] == '\0' )
    {
	for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
	    if ( IS_SET( pexit->exit_info, EX_xENTER ) )
	    {
		move_char( ch, pexit, 0, FALSE );
		return;
	    }
	send_to_char( "You cannot find an entrance here.\n\r", ch );
	return;
    }

    /* Vehicles --GW */
    if ( !IS_NPC(ch) )
    {
    for ( obj = ch->in_room->first_content; obj; obj = obj_next )
    {
	obj_next = obj->next_content;

	if ( !str_prefix( obj->name, argument ) && obj->item_type == ITEM_VEHICLE )
	{
	   sprintf(buf,"$n enters the %s.",obj->short_descr );
	   act( AT_GREEN, buf, ch,NULL,NULL,TO_ROOM);
	   ch->pcdata->vehicle = obj->pIndexData->vehicle;
	   return;
	}
    }
    }

    if ( (pexit = find_door( ch, argument, TRUE )) != NULL
    &&   IS_SET( pexit->exit_info, EX_xENTER ))
    {
	move_char( ch, pexit, 0, FALSE );
	return;
    }
    send_to_char( "You cannot enter that.\n\r", ch );
    return;
}

/*
 * Leave through an exit.					-Thoric
 */
void do_leave( CHAR_DATA *ch, char *argument )
{
    EXIT_DATA *pexit;
    bool found;
    OBJ_INDEX_DATA *obj;
    char buf[MSL];

    found = FALSE;
    if ( argument[0] == '\0' )
    {
	if ( !IS_NPC(ch) && ch->pcdata->vehicle )
        {
           obj = get_obj_index( (int)ch->pcdata->vehicle->obj_vnum, 1 );
	   send_to_char("You Leave your vehicle.\n\r",ch);
           sprintf(buf,"$n climbs out of the %s",obj->short_descr );
	   act(AT_GREEN,buf,ch,NULL,NULL,TO_ROOM);
	   ch->pcdata->vehicle = NULL;
	   return;
	}

	for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
	    if ( IS_SET( pexit->exit_info, EX_xLEAVE ) )
	    {
		move_char( ch, pexit, 0, FALSE );
		return;
	    }
	send_to_char( "You cannot find an exit here.\n\r", ch );
	return;
    }

    if ( (pexit = find_door( ch, argument, TRUE )) != NULL
    &&   IS_SET( pexit->exit_info, EX_xLEAVE ))
    {
	move_char( ch, pexit, 0, FALSE );
	return;
    }
    send_to_char( "You cannot leave that way.\n\r", ch );
    return;
}

void do_use( CHAR_DATA *ch, char *argument )
{
OBJ_DATA *obj;
char arg[MAX_INPUT_LENGTH];
ROOM_INDEX_DATA *location;

one_argument( argument, arg );

if (IS_NPC(ch))
    return;

if ( arg[0] == '\0' )
{
send_to_char("Use what?\n\r",ch);
return;
}
    if ( ms_find_obj(ch) )
        return;

   if ( ( obj = get_obj_wear( ch, arg ) ) == NULL )
    {
        send_to_char( "You are not using that item.\n\r", ch );
        return;
    }

      if ( obj->item_type != ITEM_INSIG && obj->wear_loc != WEAR_CLUB )
      {
      send_to_char("You can't use that!\n\r",ch);
      return;
      }

      if ( !ch->pcdata->clan )
      {
      send_to_char("You are a member of no clan!\n\r",ch);
      return;
      }
   
      if ( ch->fighting )
      {
	send_to_char("You can't do that well fighting!!\n\r",ch);
	return;
      }

      act( AT_MAGIC,"$n taps $s club insignia.", ch, NULL, NULL, TO_ROOM);
      send_to_char("You tap your club insignia.\n\r",ch);

      if( IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL) && !IS_SET(ch->in_room->area->flags,AFLAG_CLUB_HALL) )
      {
      send_to_char("Nothing happens!!\n\r",ch);
      return;
      }
   
      act( AT_ACTION,"$n disappears!",ch, NULL, NULL, TO_ROOM);

      location = get_room_index(ch->pcdata->clan->recall,1);
      char_from_room(ch);
      char_to_room( ch, location );
      act( AT_ACTION,"$n appears before you!",ch, NULL, NULL, TO_ROOM);
      do_look(ch,"auto");
      return;
}

/*
 * Restrict a zone to a certain level, quad class Complyant. --GW
 * TRUE is they may enter, FALSE if the CANNOT
 * Talk about a royal pain in the ass......... yeesh. --GW
 */
bool area_level_check( CHAR_DATA *ch, AREA_DATA *area )
{
int max_class_allowed=0, min_class_allowed=0, char_max_class=0, lvl=0;
char buf[MSL], high_name[MSL], low_name[MSL];
bool high_restricted=FALSE, low_restricted=FALSE;

/* What is the Char? */
if ( IS_ADV_DUAL(ch) )
{
char_max_class = 4;
lvl = ch->advlevel2;
}
else if ( IS_ADVANCED(ch) )
{
char_max_class = 3;
lvl = ch->advlevel;
}
else if ( ch->class2 >= 0 )
{
char_max_class = 2;
lvl = ch->level2;
}
else
{
char_max_class = 1;
lvl = ch->level;
}

/* Set the Default */
    min_class_allowed = 1;
    max_class_allowed = 4;
    strcpy(low_name,"Single Class");
    strcpy(high_name,"Dual Advanced Class");

/* Figure out if the zone is restricted, and set the maxes correctly */
  if ( IS_SET( area->flags, AFLAG_LOW_DUAL ) )
  {
    min_class_allowed = 2;
    low_restricted = TRUE;
    strcpy(low_name,"Dual Class");
  }

  if ( IS_SET( area->flags, AFLAG_LOW_ADV ) )
  {
    min_class_allowed = 3;
    low_restricted = TRUE;
    strcpy(low_name,"Advanced Class");
  }

  if ( IS_SET( area->flags, AFLAG_LOW_DUAL_ADV ) )
  {
    min_class_allowed = 4;
    low_restricted = TRUE;
    strcpy(low_name,"Dual Advanced Class");
  }

  if ( IS_SET( area->flags, AFLAG_HIGH_SINGLE ) )
  {
    max_class_allowed = 1;
    high_restricted = TRUE;
    strcpy(high_name,"Single Class");
  }

  if ( IS_SET( area->flags, AFLAG_HIGH_DUAL ) )
  {
    max_class_allowed = 2;
    high_restricted = TRUE;
    strcpy(high_name,"Dual Class");
  }

  if ( IS_SET( area->flags, AFLAG_HIGH_ADV ) )
  {
    max_class_allowed = 3;
    high_restricted = TRUE;
    strcpy(high_name,"Advanced Class");
  }

  if ( IS_SET( area->flags, AFLAG_HIGH_DUAL_ADV ) )
  {
    max_class_allowed = 4;
    high_restricted = TRUE;
    strcpy(high_name,"Dual Advanced Class");
  }

/*  bug("C:%s H%c L:%c MxA: %d MnA: %d LH: %d HH: %d", capitalize(ch->name),
	high_restricted ? 'Y' : 'N', low_restricted ? 'Y' : 'N',
max_class_allowed,
	min_class_allowed,area->low_hard_range,area->hi_hard_range);
*/

  /* Restriction Checks */
  if ( low_restricted )
  {
     if ( ( char_max_class < min_class_allowed ) ||
          ( lvl < area->low_hard_range ) )
     {
	sprintf(buf,"You must be Minimum %d %s to enter here.\n\r",area->low_hard_range, low_name);
	send_to_char(buf,ch);
	return FALSE;
     }
  }

  if ( high_restricted )
  {
     if ( ( char_max_class > max_class_allowed ) ||
          ( lvl > area->hi_hard_range ) )
     {
	sprintf(buf,"You must be Maximum %d %s to enter here.\n\r",area->hi_hard_range, high_name);
	send_to_char(buf,ch);
	return FALSE;
     }
  }
  
return TRUE;
}

/*
bool old_area_level_check( CHAR_DATA *ch, AREA_DATA *area )
{
int lvl=0;
char buf[MSL];
char classname[MSL];
int classes=0, needed_classes=0;

  if ( ch->level2 < 1 )
  {
    lvl=ch->level;
    classes=1;
  }
  else if ( !IS_ADVANCED(ch) ) 
  {
    lvl=ch->level2;
    classes=2;
  }
  else if ( !IS_ADV_DUAL(ch) ) 
  {
    lvl=ch->advlevel;
    classes=3;
  }
  else 
  {
    lvl=ch->advlevel2;
    classes=4;
  }

  if ( IS_SET( area->flags, AFLAG_SINGLE_CLASS ) )
  {
    strcpy(classname,"Single Class");
    needed_classes=1;
  }

  if ( IS_SET( area->flags, AFLAG_DUAL_CLASS ) )
  {
    strcpy(classname,"Dual Class");
    needed_classes=2;
  }

  if ( IS_SET( area->flags, AFLAG_ADVANCED_CLASS ) )
  {
    strcpy(classname,"Advanced Class");
    needed_classes=3;
  }

  if ( IS_SET( area->flags, AFLAG_QUAD_CLASS ) )
  {
    strcpy(classname,"Dual Advanced Class");
    needed_classes=4;
  }

  if ( ( needed_classes > 0 ) &&
      ( ( lvl < area->low_hard_range ) || ( needed_classes > classes ) ) )
  {
    sprintf(buf,"You must be level %d %s to enter this Zone.\n\r",
	area->low_hard_range, classname );
    send_to_char( buf, ch);
    return FALSE;
   }
   else if ( ( needed_classes > 0 ) &&
      ( ( lvl > area->hi_hard_range ) || ( needed_classes < classes ) ) )
   {
     sprintf(buf,"Only Characters level %d %s and Lower may enter here.\n\r",
	area->hi_hard_range, classname );
     send_to_char( buf, ch);
     return FALSE;
   }          

return TRUE;
}

*/
