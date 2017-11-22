/*--------------------------------------------------------------------------*
 *                         ** WolfPaw 1.0 **                                *
 *--------------------------------------------------------------------------*
 *               WolfPaw 1.0 (c) 1997,1998 by Dale Corse                    *
 *--------------------------------------------------------------------------*
 *            The WolfPaw Coding Team is headed by: Greywolf                *
 *  With the Assitance from: Callinon, Dhamon, Sentra, Wyverns, Altrag      *
 *  Scryn, Thoric, Justice, Tricops and Mask.                               *
 *--------------------------------------------------------------------------*
 *			Vehicle Handling Module				    *
 *--------------------------------------------------------------------------*/
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "mud.h"

OBJ_DATA *get_vehicle( CHAR_DATA *ch );

/*
 * Move the vehicle in a certain direction --GW
 */
void do_steer( CHAR_DATA *ch, char *argument )
{
VEHICLE_DATA *vehicle;
char arg[MIL];
int dir=0;
EXIT_DATA *xit;
char *dtxt;
char buf[MSL];
OBJ_DATA *obj;
CHAR_DATA *temp, *temp_next;

if ( IS_NPC(ch) )
return;

if ( !ch->pcdata->vehicle )
{
send_to_char("Your not in a Vehicle!\n\r",ch);
return;
}

vehicle = ch->pcdata->vehicle;

/* Disables for Now -- as vehicles only support 1 occupant at this time --GW */
/*
if ( !str_cmp( vehicle->operator, ch->name ) )
{
send_to_char("But you are not driving this vehicle!\n\r",ch);
return;
}
*/

one_argument( argument, arg );

if ( !str_prefix( arg, "north" ) )
dir = DIR_NORTH;
else if ( !str_prefix( arg, "east" ) )
dir = DIR_EAST;
else if ( !str_prefix( arg, "south" ) )
dir = DIR_SOUTH;
else if ( !str_prefix( arg, "west" ) )
dir = DIR_WEST;
else if ( !str_prefix( arg, "up" ) )
dir = DIR_UP;
else if ( !str_prefix( arg, "down" ) )
dir = DIR_DOWN;

xit = NULL;
xit = get_exit( ch->in_room, dir );

if ( !xit || !xit->to_room )
{
send_to_char("Your cannot go that way!\n\r",ch);
return;
}

obj = get_vehicle( ch );

if ( !obj )
{
ch->pcdata->vehicle = NULL;
return;
}

sprintf(buf,"A %s drives %s.",obj->short_descr,dir_name[dir]);
act(AT_GREEN,buf,ch,NULL,NULL,TO_ROOM);

obj_from_room( obj );
obj_to_room( obj, xit->to_room );

for ( temp = ch->in_room->first_person; temp; temp = temp_next )
{
temp_next = temp->next_in_room; 

if ( !IS_NPC(temp) && temp->pcdata->vehicle &&
     temp->pcdata->vehicle == ch->pcdata->vehicle )
{
char_from_room(temp);
char_to_room( temp, xit->to_room);
}

}
      switch( dir )
      {
      default: dtxt = "somewhere";      break;
      case 0:  dtxt = "the south";      break;
      case 1:  dtxt = "the west";       break;
      case 2:  dtxt = "the north";      break;
      case 3:  dtxt = "the east";       break;
      case 4:  dtxt = "below";          break;
      case 5:  dtxt = "above";          break;
      case 6:  dtxt = "the south-west"; break;
      case 7:  dtxt = "the south-east"; break;
      case 8:  dtxt = "the north-west"; break;
      case 9:  dtxt = "the north-east"; break;
      }

sprintf(buf,"A %s drives in from %s.",obj->short_descr,dtxt);
act( AT_GREEN,buf,ch,NULL,NULL,TO_ROOM);
do_look(ch,"auto");
return;
}

/*
 * Make a new Vehicle online --GW
 */
void do_makevehicle( CHAR_DATA *ch, char *argument )
{

return;
}

/*
 * Set a Vehicle's stuff Online --GW
 */
void do_setvehicle( CHAR_DATA *ch, char *argument )
{

return;
}

/*
 * Remove a Vehicle from the game --GW
 */
void do_vehicledel( CHAR_DATA *ch, char *argument )
{

return;
}

OBJ_DATA *get_vehicle( CHAR_DATA *ch )
{
OBJ_DATA *obj, *obj_next;

for ( obj = ch->in_room->first_content; obj; obj = obj_next )
{
	obj_next = obj->next_content;

	if ( obj->item_type == ITEM_VEHICLE &&
             obj->pIndexData->vehicle == ch->pcdata->vehicle )
	   return obj;
}

return NULL;
}

