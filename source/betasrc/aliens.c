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
 *			    Aliens Mode Module				    *
 *--------------------------------------------------------------------------*/
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

void aliens_update(void);
void char_alien_update( CHAR_DATA *ch );
void init_abduction( void );
bool can_abduct( CHAR_DATA *ch );
void abduct_char( CHAR_DATA *ch );
bool dt_scan( CHAR_DATA *ch );

/*
 * Checks Game time, if it is 4 am it starts an abduction --GW
 */
void aliens_update( void )
{
char hourbuf[MSL], ampmbuf[MSL];
int hour;
extern bool ALIENS_RAN;

sprintf( hourbuf,"%d",(time_info.hour % 12 == 0) ? 12 : time_info.hour % 12);
sprintf( ampmbuf,"%s",time_info.hour >= 12 ? "pm" : "am");
hour = atoi(hourbuf);

if ( hour == 3 && !str_cmp( ampmbuf, "am" ) )
{
ALIENS_RAN = FALSE;
}

if ( hour == 4 && !str_cmp( ampmbuf, "am" ) && !ALIENS_RAN )
{
ALIENS_RAN = TRUE;
init_abduction( );
}

return;
}

/*
 * handles what happens to a char randomly as a result of having an 
 * aliens bit --GW
 */
void char_alien_update( CHAR_DATA *ch )
{
int random;
AFFECT_DATA af;
int sn;

random = number_range( 1, 40 );

switch(random)
{
/* Full Heal */
  case 1:
  ch->hit = ch->max_hit;
  if ( !IS_VAMPIRE(ch) )
  ch->mana = ch->max_mana;
  else
  ch->pcdata->condition[COND_BLOODTHIRST] = get_bloodthirst(ch);
  ch->move = ch->max_move;
  act(AT_RED,"You SCREAM as your wounds suddenly close!",ch,NULL,NULL,TO_CHAR);
  act(AT_RED,"$n SCREAM's as $s wounds suddenly close!",ch,NULL,NULL,TO_ROOM);
  break;

  /* Nothing yet */
  case 2:
  /* Nothing yet */
  case 3:
  /* Nothing yet */
  case 4:
  break;

  /* Double Heal */
  case 5:
  ch->hit = ch->max_hit*2;
  if ( !IS_VAMPIRE(ch) )
  ch->mana = ch->max_mana*2;
  else
  ch->pcdata->condition[COND_BLOODTHIRST] = get_bloodthirst(ch)*2;
  ch->move = ch->max_move*2;
  act(AT_RED,"You feel the strength of a BEAR!",ch,NULL,NULL,TO_CHAR);
  act(AT_RED,"$n ROARS with the strength of a BEAR!",ch,NULL,NULL,TO_ROOM);
  break;

  /* Nothing yet */
  case 6:
  /* Nothing yet */
  case 7:
  /* Nothing yet */
  case 8:
  /* Nothing yet */
  case 9:
  /* Blindness */
  break;

  case 10:
    sn = skill_lookup("blindness");
    af.type      = sn;
    af.location  = APPLY_HITROLL;
    af.modifier  = -4;
    af.duration  = 50;
    af.bitvector = meb(AFF_BLIND);
    affect_to_char( ch, &af );
  act(AT_RED,"You SCREAM and claw at your eyes!",ch,NULL,NULL,TO_CHAR);
  act(AT_RED,"$n SCREAM's and claw's at $s eyes!",ch,NULL,NULL,TO_ROOM);
    break;

  /* Nothing yet */
  case 11:
  /* Nothing yet */
  case 12:
  /* Nothing yet */
  case 13:
  /* Nothing yet */
  case 14:
  /* Nothing yet */
  case 15:
  /* Nothing yet */
  case 16:
  /* Nothing yet */
  case 17:
  /* Nothing yet */
  case 18:
  /* Nothing yet */
  case 19:
  /* Nothing yet */
  case 20:
  break;
  default:
  break;
}

return;
}

/*
 * Start an Abduction! --GW
 */
void init_abduction( void )
{
CHAR_DATA *ch, *next_ch;
int num=0;
bool FOUND;

FOUND = FALSE;

log_string("System Activating Global Mode: Aliens Mode");
for( ch = first_char; ch; ch = next_ch )
{
next_ch = ch->next;
send_to_char("You see a large space ship hovering far over your head.\n\r",ch);

  if ( can_abduct( ch ) )
  {
    num = number_range( 1, 20 );

    if ( num == 2 )
    {
     FOUND = TRUE;
     abduct_char(ch);
     break;
    }
    if ( !FOUND )
	send_to_char("The Space Shuttle vanishes in a bright flash!\n\r",ch);
  }
}

return;
}

/*
 * Checks to see if a char can be abducted, 35 advanced minimum --GW
 */ 
bool can_abduct( CHAR_DATA *ch )
{
if ( IS_NPC(ch) )
return FALSE;

if ( IS_IMMORTAL(ch) )
return FALSE;

/* No abducting the Bot! --GW */
if ( !str_cmp( ch->name, "ancientone" ) )
return FALSE;

if ( !IS_ADVANCED(ch) || ch->advlevel < 35 )
return FALSE;

if ( ch->fighting && ch->fighting->who )
return FALSE;

if ( IS_SET( ch->pcdata->flagstwo, MOREPC_ABDUCTING ) )
return FALSE;

/* not in the Arena --GW */
if ( in_arena( ch ) )
return FALSE;

/* Or in Kombat */
if ( IS_SET( ch->pcdata->flags, PCFLAG_KOMBAT ) )
return FALSE;

/* Not in the Hoarders Void --GW */
if ( IS_SET( ch->pcdata->flagstwo, MOREPC_HOARDER) )
return FALSE;

return TRUE;
}

void abduct_char( CHAR_DATA *ch )
{
char buf[MSL];

sprintf(buf,"Aliens: System Abducting: %s",capitalize(ch->name));
log_string(buf);

SET_BIT(ch->pcdata->flagstwo, MOREPC_ABDUCTING );
char_from_room(ch);
char_to_room(ch, get_room_index(4,1));

send_to_char("The Spacecraft begins to glow brightly as it slowly moves into position directly above your head.\n\r",ch);
WAIT_STATE( ch, 15 );
send_to_char("A column of light surrounds you!\n\r",ch);
WAIT_STATE( ch, 15 );
send_to_char("You are slowly lifted from the ground, and begin to feel light headed and disoriented.\n\r",ch);
WAIT_STATE( ch, 15 );
send_to_char("As you enter the Spacecraft, you see shadows hovering over you in the blinding light.\n\r",ch);
WAIT_STATE( ch, 15 );
send_to_char("You are taken down a long hallway, all you can see is bright overhead lights in strange shapes zipping by above you.\n\r",ch);
WAIT_STATE( ch, 15 );
send_to_char("You are wrestled to a table, by hands made of an unknown substance.\n\rYou are then straped down, with pure steel restraints.\n\r",ch);
WAIT_STATE( ch, 15 );
send_to_char("The strange life forms begin to probe at you, and speak in strange tongues you donot understand.\n\r",ch);
WAIT_STATE( ch, 15 );
send_to_char("You feel intense pain in your head as one of the life forms takes a spinning blade to your skull.... all fades slowly to blackness..\n\r",ch);
WAIT_STATE( ch, 15 );
char_from_room(ch);
char_to_room( ch, get_room_index( ch->pcdata->start_room, 1 ) );
send_to_char("You awaken on the ground, naked and curled in a ball, having no idea where you have been, or for how long.\n\r",ch);
REMOVE_BIT(ch->pcdata->flagstwo, MOREPC_ABDUCTING );
if ( !IS_SET( ch->pcdata->flagstwo, MOREPC_ALIENS ) )
SET_BIT( ch->pcdata->flagstwo, MOREPC_ALIENS );
else
REMOVE_BIT( ch->pcdata->flagstwo, MOREPC_ALIENS );
return;
}

bool dt_scan( CHAR_DATA *ch )
{
EXIT_DATA *xit;
bool FOUND;
int x=0;

FOUND = FALSE;
for( x = 0; x < 8; x++ )
{
xit = get_exit( ch->in_room, x );

if ( !xit )
continue;

if ( xit->to_room && IS_SET( xit->to_room->room_flags, ROOM_DEATH ) )
{
FOUND = TRUE;
break;
}

}

if ( FOUND )
return TRUE;

return FALSE;
}

