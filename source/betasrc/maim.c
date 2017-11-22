/*--------------------------------------------------------------------------*
 *                         ** WolfPaw 1.0 **                                *
 *--------------------------------------------------------------------------*
 *               WolfPaw 1.0 (c) 1997,1998 by Dale Corse                    *
 *--------------------------------------------------------------------------*
 *            The WolfPaw Coding Team is headed by: Greywolf                *
 *  With the Assitance from: Callinon, Dhamon, Sentra, Wyverns, Altrag      *
 *  Scryn, Thoric, Justice, Tricops and Mask.                               *
 *--------------------------------------------------------------------------*
 *			     Maiming Module				    *
 *--------------------------------------------------------------------------*/
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

#define MAIM_BRAINS	67
#define MAIM_HEAD	68
#define MAIM_NECK	69
#define MAIM_CHEST	70
#define MAIM_ARMS	71
#define MAIM_WAIST	72
#define MAIM_WRIST	73
#define MAIM_FINGERS	74
#define MAIM_LEG	75
#define MAIM_FEET	76
#define MAIM_TOES	77

void maim_player( CHAR_DATA *victim, CHAR_DATA *mob );

void maim_update( CHAR_DATA *ch, CHAR_DATA *victim )
{
/* Big mobs only.. */
if ( ch->level < 100 )
return;

if ( IS_NPC(victim) )
  return;

  maim_player(victim,ch);

return;
}

void maim_player( CHAR_DATA *victim, CHAR_DATA *mob )
{
int num=0;
int num2=0;
int body_part=0;
ZONE_DATA *zone;
OBJ_DATA *obj=NULL;
char log_buf[MSL];
char echo_buf[MSL];
char buf[MSL];
extern bool MAIM_DEATH;

zone=find_zone(1);

/* Random Number to finalize the decision */
num=number_range(1,50);

if ( num < 30 || num > 35 )
return;

body_part=number_range(67,77);

if ( !mob->in_room )
{
  bug("MAIM_PLAYER: %s in NULL Room!\n\r",mob->short_descr);
  return;
}
if ( !victim->in_room )
{
  bug("MAIM_PLAYER: %s in NULL Room!\n\r",victim->name);
  return;
}

switch( body_part )
{
  case MAIM_BRAINS:
	obj=create_object( get_obj_index(MAIM_BRAINS,1),0,zone);
	act(AT_RED,"$n screams in pain as $s brains fall on the ground.",victim,NULL,NULL,TO_ROOM);
	act(AT_RED,"You scream in pain as you are struck in the head!",victim,NULL,NULL,TO_CHAR);
        sprintf( buf, obj->short_descr, capitalize(victim->name) );
        STRFREE( obj->short_descr );
        obj->short_descr = STRALLOC( buf );
        sprintf( buf, obj->description, capitalize(victim->name) );
        STRFREE( obj->description );
        obj->description = STRALLOC( buf );
	obj->timer = 20;
	obj_to_room(obj,victim->in_room);
	damage( mob, victim, 2000, TYPE_UNDEFINED);
	sprintf(log_buf,"(MAIM) %s was labotomized by %s at %d",capitalize(victim->name),
                (IS_NPC(mob) ? mob->short_descr : capitalize(mob->name)),victim->in_room->vnum);
	log_string(log_buf);
	break;
  case MAIM_HEAD:
	num2=number_range(1,3);
	if ( num2 != 2 );
	return;

	obj=create_object( get_obj_index(MAIM_HEAD,1),0,zone);
	act(AT_RED,"$n screams in pain as $e is decapitated!",victim,NULL,NULL,TO_ROOM);
	act(AT_RED,"You scream in pain as you are decapitated!",victim,NULL,NULL,TO_CHAR);
	act(AT_RED,"$n is DEAD! R.I.P!",victim,NULL,NULL,TO_ROOM);
        sprintf( buf, obj->short_descr, capitalize(victim->name) );
        STRFREE( obj->short_descr );
        obj->short_descr = STRALLOC( buf );
        sprintf( buf, obj->description, capitalize(victim->name) );
        STRFREE( obj->description );
        obj->description = STRALLOC( buf );
	obj->timer = 20;
	obj_to_room(obj,victim->in_room);
	sprintf(log_buf,"(MAIM) %s was decapitated by %s at %d",capitalize(victim->name),
                (IS_NPC(mob) ? mob->short_descr : capitalize(mob->name)),victim->in_room->vnum);
	log_string(log_buf);
	sprintf(echo_buf,"&R[DEATH] &W %s has been decapitated by %s!",capitalize(victim->name),
	(IS_NPC(mob) ? mob->short_descr : capitalize(mob->name)));
	echo_to_all(AT_PLAIN,echo_buf,ECHOTAR_ALL);
	MAIM_DEATH=TRUE;
	raw_kill(mob,victim);
	MAIM_DEATH=FALSE;
	break;
  case MAIM_NECK:
	obj=create_object( get_obj_index(MAIM_NECK,1),0,zone);
	act(AT_RED,"$n screams in pain as blood spurts from $s neck!",victim,NULL,NULL,TO_ROOM);
	act(AT_RED,"You scream in pain as your jugular is slashed!",victim,NULL,NULL,TO_CHAR);
        sprintf( buf, obj->short_descr, capitalize(victim->name) );
        STRFREE( obj->short_descr );
        obj->short_descr = STRALLOC( buf );
        sprintf( buf, obj->description, capitalize(victim->name) );
        STRFREE( obj->description );
        obj->description = STRALLOC( buf );
	obj->timer = 20;
	obj_to_room(obj,victim->in_room);
	damage( mob, victim, 2000, TYPE_UNDEFINED);
	sprintf(log_buf,"(MAIM) %s was throat cut by %s at %d",capitalize(victim->name),
                (IS_NPC(mob) ? mob->short_descr : capitalize(mob->name)),victim->in_room->vnum);
	log_string(log_buf);
	break;
  case MAIM_CHEST:
	obj=create_object( get_obj_index(MAIM_CHEST,1),0,zone);
	act(AT_RED,"$n screams in pain as $e is slashed across the chest!",victim,NULL,NULL,TO_ROOM);
	act(AT_RED,"You scream in pain as you are slashed across the chest!",victim,NULL,NULL,TO_CHAR);
        sprintf( buf, obj->short_descr, capitalize(victim->name) );
        STRFREE( obj->short_descr );
        obj->short_descr = STRALLOC( buf );
        sprintf( buf, obj->description, capitalize(victim->name) );
        STRFREE( obj->description );
        obj->description = STRALLOC( buf );
	obj->timer = 20;
	obj_to_room(obj,victim->in_room);
	damage( mob, victim, 2000, TYPE_UNDEFINED);
	sprintf(log_buf,"(MAIM) %s was slashed in the chest by %s at %d",capitalize(victim->name),
                (IS_NPC(mob) ? mob->short_descr : capitalize(mob->name)),victim->in_room->vnum);
	log_string(log_buf);
	break;
  case MAIM_ARMS:
	obj=create_object( get_obj_index(MAIM_ARMS,1),0,zone);
	act(AT_RED,"$n screams in pain as $e is disarmed -- the painfull way!",victim,NULL,NULL,TO_ROOM);
	act(AT_RED,"You scream in pain as you are disarmed -- the painful way!!",victim,NULL,NULL,TO_CHAR);
        sprintf( buf, obj->short_descr, capitalize(victim->name) );
        STRFREE( obj->short_descr );
        obj->short_descr = STRALLOC( buf );
        sprintf( buf, obj->description, capitalize(victim->name) );
        STRFREE( obj->description );
        obj->description = STRALLOC( buf );
	obj->timer = 20;
	obj_to_room(obj,victim->in_room);
	damage( mob, victim, 2000, TYPE_UNDEFINED);
	sprintf(log_buf,"(MAIM) %s was painfully disarmed by %s at %d",capitalize(victim->name),
                (IS_NPC(mob) ? mob->short_descr : capitalize(mob->name)),victim->in_room->vnum);
	log_string(log_buf);
	break;
  case MAIM_WAIST:
	num2=number_range(1,3);
	if ( num2 != 2 );
	return;

	obj=create_object( get_obj_index(MAIM_WAIST,1),0,zone);
	act(AT_RED,"$n screams in pain as $s legs are removed from $s body",victim,NULL,NULL,TO_ROOM);
	act(AT_RED,"You scream in pain as your legs are removed from your body!",victim,NULL,NULL,TO_CHAR);
	act(AT_RED,"$n is DEAD! R.I.P!",victim,NULL,NULL,TO_ROOM);
        sprintf( buf, obj->short_descr, capitalize(victim->name) );
        STRFREE( obj->short_descr );
        obj->short_descr = STRALLOC( buf );
        sprintf( buf, obj->description, capitalize(victim->name) );
        STRFREE( obj->description );
        obj->description = STRALLOC( buf );
	obj->timer = 20;
	obj_to_room(obj,victim->in_room);
	sprintf(log_buf,"(MAIM) %s was made legless by %s at %d",capitalize(victim->name),
                (IS_NPC(mob) ? mob->short_descr : capitalize(mob->name)),victim->in_room->vnum);
	log_string(log_buf);
	sprintf(echo_buf,"&R[DEATH] &W %s has been sliced in half by %s!",capitalize(victim->name),
	(IS_NPC(mob) ? mob->short_descr : capitalize(mob->name)));
	echo_to_all(AT_PLAIN,echo_buf,ECHOTAR_ALL);
	MAIM_DEATH=TRUE;
	raw_kill(mob,victim);
	MAIM_DEATH=FALSE;
	break;
  case MAIM_WRIST:
	obj=create_object( get_obj_index(MAIM_WRIST,1),0,zone);
	act(AT_RED,"$n screams in pain as $s hand falls to the ground!.",victim,NULL,NULL,TO_ROOM);
	act(AT_RED,"You scream in pain as your hand falls to the ground!",victim,NULL,NULL,TO_CHAR);
        sprintf( buf, obj->short_descr, capitalize(victim->name) );
        STRFREE( obj->short_descr );
        obj->short_descr = STRALLOC( buf );
        sprintf( buf, obj->description, capitalize(victim->name) );
        STRFREE( obj->description );
        obj->description = STRALLOC( buf );
	obj->timer = 20;
	obj_to_room(obj,victim->in_room);
	damage( mob, victim, 2000, TYPE_UNDEFINED);
	sprintf(log_buf,"(MAIM) %s was 'red handed' by %s at %d",capitalize(victim->name),
                (IS_NPC(mob) ? mob->short_descr : capitalize(mob->name)),victim->in_room->vnum);
	log_string(log_buf);
	break;
  case MAIM_FINGERS:
	obj=create_object( get_obj_index(MAIM_FINGERS,1),0,zone);
	act(AT_RED,"$n screams in pain as $s fingers scatter about the room!",victim,NULL,NULL,TO_ROOM);
	act(AT_RED,"You scream in pain as your fingers scatter about the room!",victim,NULL,NULL,TO_CHAR);
        sprintf( buf, obj->short_descr, capitalize(victim->name) );
        STRFREE( obj->short_descr );
        obj->short_descr = STRALLOC( buf );
        sprintf( buf, obj->description, capitalize(victim->name) );
        STRFREE( obj->description );
        obj->description = STRALLOC( buf );
	obj->timer = 20;
	obj_to_room(obj,victim->in_room);
	damage( mob, victim, 2000, TYPE_UNDEFINED);
	sprintf(log_buf,"(MAIM) %s got fingers scattered by %s at %d",capitalize(victim->name),
                (IS_NPC(mob) ? mob->short_descr : capitalize(mob->name)),victim->in_room->vnum);
	log_string(log_buf);
	break;
  case MAIM_LEG:
	obj=create_object( get_obj_index(MAIM_LEG,1),0,zone);
	act(AT_RED,"$n screams in pain as $s leg is severed at the knee!.",victim,NULL,NULL,TO_ROOM);
	act(AT_RED,"You scream in pain as your leg is severed at the knee!",victim,NULL,NULL,TO_CHAR);
        sprintf( buf, obj->short_descr, capitalize(victim->name) );
        STRFREE( obj->short_descr );
        obj->short_descr = STRALLOC( buf );
        sprintf( buf, obj->description, capitalize(victim->name) );
        STRFREE( obj->description );
        obj->description = STRALLOC( buf );
	obj->timer = 20;
	obj_to_room(obj,victim->in_room);
	damage( mob, victim, 2000, TYPE_UNDEFINED);
	sprintf(log_buf,"(MAIM) %s was made a foot shorter by %s at %d",capitalize(victim->name),
                (IS_NPC(mob) ? mob->short_descr : capitalize(mob->name)),victim->in_room->vnum);
	log_string(log_buf);
	break;
  case MAIM_FEET:
	obj=create_object( get_obj_index(MAIM_FEET,1),0,zone);
	act(AT_RED,"$n screams in pain as $s foot is sliced into peices!",victim,NULL,NULL,TO_ROOM);
	act(AT_RED,"You scream in pain as your foot is turned into dogfood!",victim,NULL,NULL,TO_CHAR);
        sprintf( buf, obj->short_descr, capitalize(victim->name) );
        STRFREE( obj->short_descr );
        obj->short_descr = STRALLOC( buf );
        sprintf( buf, obj->description, capitalize(victim->name) );
        STRFREE( obj->description );
        obj->description = STRALLOC( buf );
	obj->timer = 20;
	obj_to_room(obj,victim->in_room);
	damage( mob, victim, 2000, TYPE_UNDEFINED);
	sprintf(log_buf,"(MAIM) %s lost footing to %s at %d",capitalize(victim->name),
                (IS_NPC(mob) ? mob->short_descr : capitalize(mob->name)),victim->in_room->vnum);
	log_string(log_buf);
	break;
  case MAIM_TOES:
	obj=create_object( get_obj_index(MAIM_TOES,1),0,zone);
	act(AT_RED,"$n screams in pain as $s toes are scattered about the room!",victim,NULL,NULL,TO_ROOM);
	act(AT_RED,"You scream in pain as your toes are scattered about the room!",victim,NULL,NULL,TO_CHAR);
        sprintf( buf, obj->short_descr, capitalize(victim->name) );
        STRFREE( obj->short_descr );
        obj->short_descr = STRALLOC( buf );
        sprintf( buf, obj->description, capitalize(victim->name) );
        STRFREE( obj->description );
        obj->description = STRALLOC( buf );
	obj->timer = 20;
	obj_to_room(obj,victim->in_room);
	damage( mob, victim, 2000, TYPE_UNDEFINED);
	sprintf(log_buf,"(MAIM) %s got toes scattered by %s at %d",capitalize(victim->name),
                (IS_NPC(mob) ? mob->short_descr : capitalize(mob->name)),victim->in_room->vnum);
	log_string(log_buf);
	break;
}

return;
}

void dr_quack ( CHAR_DATA *victim )
{

return;
}

