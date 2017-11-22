/*--------------------------------------------------------------------------*
 *                         ** WolfPaw 1.0 **                                *
 *--------------------------------------------------------------------------*
 *               WolfPaw 1.0 (c) 1997,1998 by Dale Corse                    *
 *--------------------------------------------------------------------------*
 *            The WolfPaw Coding Team is headed by: Greywolf                *
 *  With the Assitance from: Callinon, Dhamon, Sentra, Wyverns, Altrag      *
 *  Scryn, Thoric, Justice, Tricops and Mask.                               *
 *--------------------------------------------------------------------------*
 *		Equipment Hoarders Handling Module			    *
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

/*
 * How many Items a Char can Carry before they are legal
 */
#define HOARDER_NUMBER		10

/* Prototypes */
void hoard_handler(CHAR_DATA *ch, OBJ_DATA *obj, int type );
void hoarder_check(CHAR_DATA *ch, OBJ_DATA *obj, int type );
int get_num_objs_char( CHAR_DATA *ch );

/*
 * Check the char, see if they are Hoarding. --GW
 */
void hoarder_check(CHAR_DATA *ch, OBJ_DATA *obj,  int type )
{
/* No Mobs! */
if ( IS_NPC(ch) )
return;

/* Gods are allowed */
if ( IS_IMMORTAL(ch) )
return;

/* No Char for some Reason? (Oinvoke?) */
if ( !ch )
return;

switch( type )
{
case  HCHECK_LOGIN:   /* Logging On? */
  if ( get_num_objs_char(ch) > HOARDER_NUMBER )
  {
      hoard_handler(ch,NULL,HOARD_SEND_TO_VOID);
      break;
  }
  break; /* Not a Hoarder */

case HCHECK_DROP:  /* Dropped an Object */
  if ( ch->pcdata->hoard_status == HOARD_IN_VOID )
  {
     hoard_handler(ch,obj,HOARD_DROP_TRIGGER);
     break;    
  }
  break; /* Sorting, dont destroy it */

case HCHECK_MOVE:  /* They Moved..? */
  /* Send Another Explanation when they go North */
  if ( ch->pcdata->hoard_status == HOARD_IN_VOID )
  {
    hoard_handler(ch,NULL,HOARD_SORTING_EQ);
    break;
  }
  else /* Were Done, put us back in the Hoarder's Void */
  {
    hoard_handler(ch,NULL,HOARD_DONE_SORTING);
    break;
  }

case HCHECK_AUCTION: /* Hoarder Auctioned an Item */
     hoard_handler(ch,obj,HOARD_AUCD_ITEM);
     break;

default:
    bug("Hoarder_Check: BAD TYPE!",0); /* Uh oh! */
    break;    
}

return;
}

/*
 * Main Interface... does it all! --GW
 */
void hoard_handler(CHAR_DATA *ch, OBJ_DATA *obj, int type )
{
int cnt=0;
char buf[MSL];

cnt = get_num_objs_char(ch);

switch( type )
{
 case HOARD_SEND_TO_VOID: /* Put a Char in the Void */
    log_string("Hoader's Void Activated:");
    sprintf(buf,"Sending %s to the Hoarder's Void with %d Limited Objects!",capitalize(ch->name),cnt);
    log_string(buf);
    act(AT_RED,"A Large hole opens in the ground! When the dust clears, $n is gone!",ch,NULL,NULL,TO_ROOM);
    char_from_room(ch);
    char_to_room(ch,get_room_index(ROOM_VNUM_HOARDERS_VOID,1));
    act(AT_RED,"$n falls in from above, and lands on $s ass.",ch,NULL,NULL,TO_ROOM);
    send_to_char("/a&RNOTICE:&W You have Been automatically sent to the Hoarder's Void.\n\r",ch);
    ch_printf(ch,"           &W You may not leave untill you have less then %d LIMITED\n\r",HOARDER_NUMBER);
    send_to_char("           &W objects in your inventory, or in containers. Anything\n\r",ch);
    send_to_char("           &W you drop in the Hoarder's Void will be &RDESTROYED&W.\n\r",ch);
    send_to_char("           &W you may proceed &RNORTH&W to sort equipment, in the&W\n\r",ch);
    send_to_char("           &W room &RNORTH&W objects dropped will &RNOT&W be destroyed\n\r",ch);
    send_to_char("           &W Please Sort your Eq there, and drop it in this room.\n\r",ch);
    send_to_char("           &W &RYou will be automatically removed from this void once\n\r",ch);
    ch_printf(ch,"           &W you have less then %d Limited Objects&W.\n\r",HOARDER_NUMBER);
    ch->pcdata->hoard_status = HOARD_IN_VOID;
    SET_BIT(ch->pcdata->flagstwo, MOREPC_HOARDER);
    break;

case HOARD_DROP_TRIGGER: /* They Dropped it .. Destroy it, let them out if done */
    act(AT_RED,"$p sinks into the abyss, lost forever.",ch,obj,NULL,TO_CHAR);
    act(AT_RED,"$p sinks into the abyss, lost forever.",ch,obj,NULL,TO_ROOM);
    separate_obj(obj);
    obj_from_room(obj);
    extract_obj(obj);
    cnt = get_num_objs_char(ch);
    if ( cnt <= HOARDER_NUMBER ) /* Let em Go */
    {
    send_to_char("You are no longer a Hoarder. The Void has Released you.\n\r",ch);
    act(AT_YELLOW,"$n is surrounded in a Godly light, and vanishes!",ch,NULL,NULL,TO_ROOM); 
    char_from_room(ch);
    char_to_room(ch,get_room_index(ROOM_VNUM_TEMPLE,1));
    act(AT_YELLOW,"$n appears in a flash of Godly Light!",ch,NULL,NULL,TO_ROOM);
    sprintf(buf,"Hoarder's Void Releasing: %s",capitalize(ch->name));
    log_string(buf);
    ch->pcdata->hoard_status = HOARD_NONE;
    REMOVE_BIT(ch->pcdata->flagstwo, MOREPC_HOARDER);
    break;
    }
    sprintf(buf,"A Raspy Voice Echos '$n, you are now carrying %d Limited Objects.",cnt);
    act(AT_MAGIC,buf,ch,NULL,NULL,TO_CHAR);
    act(AT_MAGIC,buf,ch,NULL,NULL,TO_ROOM);
    break;

case HOARD_SORTING_EQ:   /* Let them out to sort eq */
    send_to_char("&WYou May sort Eq, and Drop it in this room, it will &RNOT&W be Destroyed here.\n\r",ch);
    send_to_char("&WProceed South when you are ready to continue.\n\r",ch);
    ch->pcdata->hoard_status = HOARD_SORTING_EQ;
    break;

case HOARD_DONE_SORTING: /* Done Sorting, put em back */
    send_to_char("&WYou proceed back to the Hoarder's Void.\n\r",ch);
    send_to_char("&WAnything you drop here &RWILL&W be &RDESTROYED&W\n\r",ch);
    ch->pcdata->hoard_status = HOARD_IN_VOID;
    break;

case HOARD_AUCD_ITEM: /* Hoarder Auctioned an Item */
    if ( !IS_LIMITED(obj) )
       break;

    cnt = get_num_objs_char(ch);
    if ( cnt <= HOARDER_NUMBER ) /* Let em Go */
    {
    send_to_char("You are no longer a Hoarder. The Void has Released you.\n\r",ch);
    act(AT_YELLOW,"$n is surrounded in a Godly light, and vanishes!",ch,NULL,NULL,TO_ROOM); 
    char_from_room(ch);
    char_to_room(ch,get_room_index(ROOM_VNUM_TEMPLE,1));
    act(AT_YELLOW,"$n appears in a flash of Godly Light!",ch,NULL,NULL,TO_ROOM);
    sprintf(buf,"Hoarder's Void Releasing: %s",capitalize(ch->name));
    log_string(buf);
    ch->pcdata->hoard_status = HOARD_NONE;
    REMOVE_BIT(ch->pcdata->flagstwo, MOREPC_HOARDER);
    break;
    }
    sprintf(buf,"A Raspy Voice Echos '$n, you are now carrying %d Limited Objects.",cnt);
    act(AT_MAGIC,buf,ch,NULL,NULL,TO_CHAR);
    act(AT_MAGIC,buf,ch,NULL,NULL,TO_ROOM);
    break;

default:
   bug("Hoard_Handler: BAD TYPE!",0);
   break;
}

return;
}

/*
 * Find out How many Items a Character is Carrying --GW 
 */
int get_num_objs_char( CHAR_DATA *ch )
{
OBJ_DATA *obj, *next_obj, *container, *container_next;
int cnt=0;

/* Check Inventory */
for ( obj=ch->first_carrying; obj; obj = next_obj )
{
  next_obj = obj->next_content;
  if ( IS_LIMITED(obj) && obj->wear_loc == WEAR_NONE )
  {
    if ( obj->count > 1 )
      cnt += obj->count;
    else
      cnt++;
  }
}

/* Check for WORN Containers -- opps all containers */
for ( obj=ch->first_carrying; obj; obj = next_obj )
{
  next_obj = obj->next_content;
     if ( obj->item_type == ITEM_CONTAINER )
  {
     for ( container=obj->first_content; container; container = container_next )
     {
         container_next = container->next_content;
        
         if ( IS_LIMITED(container) )
	 {
          if ( container->count > 1 )
           cnt += container->count;
          else
           cnt++;
	 }
     }
  }
}

return cnt;
}

