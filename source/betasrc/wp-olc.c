/*--------------------------------------------------------------------------*
 *                         ** WolfPaw 1.0 **                                *
 *--------------------------------------------------------------------------*
 *               WolfPaw 1.0 (c) 1997,1998 by Dale Corse                    *
 *--------------------------------------------------------------------------*
 *            The WolfPaw Coding Team is headed by: Greywolf                *
 *  With the Assitance from: Callinon, Dhamon, Sentra, Wyverns, Altrag      *
 *  Scryn, Thoric, Justice, Tricops and Mask.                               *
 *--------------------------------------------------------------------------*
 *               Wolfpaw Custom Menu-Driven OLC Module                      *
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

#define version "1.0"

/* local function declarations and prototypes */
void display_main_redit_page  args( ( CHAR_DATA *ch ) );
int find_exit  args( ( ROOM_INDEX_DATA *room, int dir ) );
void gredit_engine args( ( DESCRIPTOR_DATA *d, char *argument ) );
void gredit_cleanup args( ( CHAR_DATA *ch ) );
void add_char args( ( CHAR_DATA *ch ) );
void remove_char args( ( CHAR_DATA *ch ) );

/*
 * Begin REDIT Functions. --GW
 */

/*
 * Command to start up Room Editor, takes them from game, into Editor,
 */
void do_gredit( CHAR_DATA *ch, char *argument )
{
DESCRIPTOR_DATA *d;
record_call("<do_gredit>");

d = ch->desc;

/* Get Going, remove char from the world, stick em in the editor!*/
act( AT_MAGIC, "$n begins changing the world.",ch,NULL,NULL,TO_ROOM);
/* start up the editor engine */
display_main_redit_page(ch);
remove_char(ch);
ch->substate = SUB_REDIT_MAIN;
ch->desc->connected = CON_GREDITING;
return;
}

/*
 * Cleans up the char.. and puts them back in the game.
 */
void gredit_cleanup( CHAR_DATA *ch )
{ 
record_call("<gredit_cleanup>");

/* finish up, put the char back in the world and exit */
ch->substate = SUB_NONE;
ch->desc->connected = CON_PLAYING;
add_char(ch);
act( AT_MAGIC, "$n has returned from changing the world.",ch,NULL,NULL,TO_ROOM);
return;
}

/*
 * Main GRedit Engine .. handles all the character interface command entry.
 */
void gredit_engine( DESCRIPTOR_DATA *d, char *argument )
{
CHAR_DATA *ch;
char arg[MSL];

ch = d->character;

switch( ch->substate )
{

   case SUB_REDIT_MAIN:
      display_main_redit_page(ch);
      one_argument( argument, arg );
      switch( *arg )
      {
	default:
	  write_to_buffer(d,"Invalid Choice.\n\r",0);
	  return;
	case '1':
	  ch->substate = SUB_REDIT_RNAME;
	  break;
	case '2':
	  ch->substate = SUB_REDIT_ROOMDESC;
	  break;
	case '3':
	  ch->substate = SUB_REDIT_ROOMFLAGS;
	  break;
	case '4':
	  ch->substate = SUB_REDIT_SECTOR;
	  break;
	case '5':
	  ch->substate = SUB_REDIT_EXIT_N;
	  break;
	case '6':
	  ch->substate = SUB_REDIT_EXIT_E;
	  break;
	case '7':
	  ch->substate = SUB_REDIT_EXIT_S;
	  break;
	case '8':
	  ch->substate = SUB_REDIT_EXIT_W;
	  break;
	case '9':
	  ch->substate = SUB_REDIT_EXIT_U;
	  break;
	case 'A':
	case 'a':
	  ch->substate = SUB_REDIT_EXIT_D;
	  break;
	case 'B':
	case 'b':
	  ch->substate = SUB_REDIT_EXTRA_DESC;
	  break;
	case 'Q':
	case 'q':
	  gredit_cleanup(ch);
	  break;
      }
      break;

   case SUB_REDIT_RNAME:
      write_to_buffer( d, "Incomplete..press enter.\n\r",0);
      ch->substate = SUB_REDIT_MAIN;
      break;
   case SUB_REDIT_ROOMDESC:
      write_to_buffer( d, "Incomplete..press enter.\n\r",0);
      ch->substate = SUB_REDIT_MAIN;
      break;
   case SUB_REDIT_ROOMFLAGS:
      write_to_buffer( d, "Incomplete..press enter.\n\r",0);
      ch->substate = SUB_REDIT_MAIN;
      break;
   case SUB_REDIT_SECTOR:
      write_to_buffer( d, "Incomplete..press enter.\n\r",0);
      ch->substate = SUB_REDIT_MAIN;
      break;
   case SUB_REDIT_EXIT_N:
      write_to_buffer( d, "Incomplete..press enter.\n\r",0);
      ch->substate = SUB_REDIT_MAIN;
      break;
   case SUB_REDIT_EXIT_E:
      write_to_buffer( d, "Incomplete..press enter.\n\r",0);
      ch->substate = SUB_REDIT_MAIN;
      break;
   case SUB_REDIT_EXIT_S:
      write_to_buffer( d, "Incomplete..press enter.\n\r",0);
      ch->substate = SUB_REDIT_MAIN;
      break;
   case SUB_REDIT_EXIT_W:
      write_to_buffer( d, "Incomplete..press enter.\n\r",0);
      ch->substate = SUB_REDIT_MAIN;
      break;
   case SUB_REDIT_EXIT_U:
      write_to_buffer( d, "Incomplete..press enter.\n\r",0);
      ch->substate = SUB_REDIT_MAIN;
      break;
   case SUB_REDIT_EXIT_D:
      write_to_buffer( d, "Incomplete..press enter.\n\r",0);
      ch->substate = SUB_REDIT_MAIN;
      break;
   case SUB_REDIT_EXTRA_DESC:
      write_to_buffer( d, "Incomplete..press enter.\n\r",0);
      ch->substate = SUB_REDIT_MAIN;
      break;

   default:
	bug( "GRedit_Engine: bad substate %d.",ch->substate );
	bug( "Calling GRedit_Cleanup on %s",capitalize(ch->name)); 	
	gredit_cleanup( ch );
	return;
   
}

return;
}

/*
 * Display the Header Line
 */
void display_header( CHAR_DATA *ch )
{
record_call("<display_header>");
do_clear(ch,"");
send_to_char("DONT USE THIS: IT IS INCOMPLETE! --GW\n\r",ch);
send_to_char("\n\r*----------------------------------------------------*\n\r",ch);
ch_printf(ch,"*       WolfPaw Menu Driven OLC -- Version %s     *",version);
send_to_char("\n\r*----------------------------------------------------*\n\r",ch);
return;
}

/*
 * Returns what Vnum an exit direction leads to, -1 if none.
 */
int find_exit( ROOM_INDEX_DATA *room, int dir )
{
    EXIT_DATA *pexit;
    int cnt;

        for ( cnt = 0, pexit = room->first_exit; pexit; pexit = pexit->next )
	{
	  if ( pexit->vdir == dir && pexit->to_room )
	  {
	    return pexit->to_room->vnum;
	  }
        }

return -1;
}

/*
 * Displays the Mian menu for GRedit
 */
void display_main_redit_page( CHAR_DATA *ch )
{
ROOM_INDEX_DATA *room;
ZONE_DATA *zone;

room = ch->in_room;
if ( ch->in_room->area )
zone = ch->in_room->area->zone;
else
zone = find_zone(1);

display_header(ch);

ch_printf( ch, "Room Number: [%d] Zone #: [%d]\n\r",room->vnum,zone->number);
ch_printf( ch, "Area: [%s]\n\r",room->area ? room->area->name : "None!");
ch_printf( ch, "1) Name: [%s]\n\r",room->name);
send_to_char("2) Description\n\r",ch);
ch_printf( ch, "3) Room Flags: %s\n\r",flag_string(ch->in_room->room_flags,r_flags));
ch_printf( ch, "4) Sector Type: %s\n\r",sector_name(ch->in_room->sector_type));
ch_printf( ch, "5) Exit North: %d\n\r", find_exit( room, DIR_NORTH ) );
ch_printf( ch, "6) Exit East:  %d\n\r", find_exit( room, DIR_EAST ) );
ch_printf( ch, "7) Exit South: %d\n\r", find_exit( room, DIR_SOUTH ) );
ch_printf( ch, "8) Exit West:  %d\n\r", find_exit( room, DIR_WEST ) );
ch_printf( ch, "9) Exit Up:    %d\n\r", find_exit( room, DIR_UP ) );
ch_printf( ch, "A) Exit Down:  %d\n\r", find_exit( room, DIR_DOWN ) );
send_to_char("B) Extra Descriptions Menu\n\r",ch);
send_to_char("Q) Quit\n\r",ch);
send_to_char("Enter Choice: ",ch);
return;
}


/* 
 * Begin GOedit Functions --GW
 */

void do_goedit( CHAR_DATA *ch, char *argument )
{
record_call("<do_goedit>");
return;
}

/*
 * Begin GMedit Functions --GW
 */
void do_gmedit( CHAR_DATA *ch, char *argument )
{
record_call("<do_gmedit>");
return;
}


