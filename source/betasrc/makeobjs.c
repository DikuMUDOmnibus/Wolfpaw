/***************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.0 (C) 1994, 1995, 1996 by Derek Snider             |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh and Tricops  |~'~.VxvxV.~'~*
 * ------------------------------------------------------------------------ *
 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 * ------------------------------------------------------------------------ *
 *			Specific object creation module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

/*
 * Make a fire.
 */
void make_fire(ROOM_INDEX_DATA *in_room, sh_int timer)
{
    OBJ_DATA *fire;

    fire = create_object( get_obj_index( OBJ_VNUM_FIRE,1 ), 0,first_zone);
    fire->timer = number_fuzzy(timer);
    obj_to_room( fire, in_room );
    return;
}

/*
 * Make a trap.
 */
OBJ_DATA *make_trap(int v0, int v1, int v2, int v3)
{
    OBJ_DATA *trap;

    trap = create_object( get_obj_index( OBJ_VNUM_TRAP,1 ), 0,first_zone );

    if ( !trap )
    return NULL;

    trap->timer = 0;
    trap->value[0] = v0;
    trap->value[1] = v1;
    trap->value[2] = v2;
    trap->value[3] = v3;
    return trap;
}


/*
 * Turn an object into scraps.		-Thoric
 */
void make_scraps( OBJ_DATA *obj )
{
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA  *scraps, *tmpobj;
  CHAR_DATA *ch = NULL;

  separate_obj( obj );
  scraps	= create_object( get_obj_index( OBJ_VNUM_SCRAPS,1 ),0,first_zone );
  if ( !scraps )
	return;

  scraps->timer = number_range( 5, 15 );

  /* don't make scraps of scraps of scraps of ... */
  if ( obj->pIndexData->vnum == OBJ_VNUM_SCRAPS )
  {
     STRFREE( scraps->short_descr );
     scraps->short_descr = STRALLOC( "some debris" );
     STRFREE( scraps->description );
     scraps->description = STRALLOC( "Bits of debris lie on the ground here." );
  }
  else
  {
     sprintf( buf, scraps->short_descr, obj->short_descr );
     STRFREE( scraps->short_descr );
     scraps->short_descr = STRALLOC( buf );
     sprintf( buf, scraps->description, obj->short_descr );
     STRFREE( scraps->description );
     scraps->description = STRALLOC( buf );
  }

  if ( obj->carried_by )
  {
    act( AT_OBJECT, "$p falls to the ground in scraps!",
		  obj->carried_by, obj, NULL, TO_CHAR );
    if ( obj == get_eq_char( obj->carried_by, WEAR_WIELD )
    &&  (tmpobj = get_eq_char( obj->carried_by, WEAR_DUAL_WIELD)) != NULL )
       tmpobj->wear_loc = WEAR_WIELD;

    obj_to_room( scraps, obj->carried_by->in_room);
  }
  else
  if ( obj->in_room )
  {
    if ( (ch = obj->in_room->first_person ) != NULL )
    {
      act( AT_OBJECT, "$p is reduced to little more than scraps.",
	   ch, obj, NULL, TO_ROOM );
      act( AT_OBJECT, "$p is reduced to little more than scraps.",
	   ch, obj, NULL, TO_CHAR );
    }
    obj_to_room( scraps, obj->in_room);
  }
  if ( (obj->item_type == ITEM_CONTAINER || obj->item_type == ITEM_QUIVER
  ||   obj->item_type == ITEM_CORPSE_PC) && obj->first_content )
  {
    if ( ch && ch->in_room )
    {
	act( AT_OBJECT, "The contents of $p fall to the ground.",
	   ch, obj, NULL, TO_ROOM );
	act( AT_OBJECT, "The contents of $p fall to the ground.",
	   ch, obj, NULL, TO_CHAR );
    }
    if ( obj->carried_by )
	empty_obj( obj, NULL, obj->carried_by->in_room );
    else
    if ( obj->in_room )
	empty_obj( obj, NULL, obj->in_room );
    else
    if ( obj->in_obj )
	empty_obj( obj, obj->in_obj, NULL );
  }
  extract_obj( obj );
}


/*
 * Make a corpse out of a character.
 */
void make_corpse( CHAR_DATA *ch, CHAR_DATA *killer, bool DT )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *corpse;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    char *name;
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *old_room;
    ROOM_INDEX_DATA *dtroom;
    extern bool MAIM_DEATH;

    old_room = ch->died_in_room;
    location = get_room_index ( ROOM_VNUM_MORGUE,1 );
    dtroom   = get_room_index ( 4, 1 );
 
    if ( IS_NPC(ch) )
    {
	name		= ch->short_descr;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC,1), 0,first_zone);

	if ( !corpse)
	return;

	corpse->timer	= 6;
	if ( ch->gold > 0 )
	{
	    if ( old_room )
            old_room->area->gold_looted += ch->gold;

	    obj_to_obj( create_money( ch->gold, COPPER ), corpse );
	    ch->gold = 0;
	}

/* Cannot use these!  They are used.
	corpse->value[0] = (int)ch->pIndexData->vnum;
	corpse->value[1] = (int)ch->max_hit;
*/
/*	Using corpse cost to cheat, since corpses not sellable */
	corpse->cost     = (-(int)ch->pIndexData->vnum);
        corpse->value[2] = corpse->timer; 
    }
    else
    {
	name		= ch->name;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_PC,1), 0,first_zone);
	corpse->timer	= 40;
        corpse->value[2] = (int)(corpse->timer/8);
	if ( IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) )
	  SET_BIT( corpse->extra_flags, ITEM_CLANCORPSE );
	/* Pkill corpses get save timers, in ticks (approx 70 seconds)
	   This should be anough for the killer to type 'get all corpse'. */
	if ( !IS_NPC(ch) && !IS_NPC(killer) )
	  corpse->value[3] = 1;
	else
	  corpse->value[3] = 0;
    }

    /* Added corpse name - make locate easier , other skills */
    sprintf( buf, "corpse %s", name );
    STRFREE( corpse->name );
    corpse->name = STRALLOC( buf );

    sprintf( buf, corpse->short_descr, name );
    STRFREE( corpse->short_descr );
    corpse->short_descr = STRALLOC( buf );

    sprintf( buf, corpse->description, name );
    STRFREE( corpse->description );
    corpse->description = STRALLOC( buf );
   
    /* PC's Lose Gold on Death, it stays in the corpse! --GW */
    if ( !IS_NPC(ch) && ch->gold > 0 )
    {
       if ( ch->gold > 0 )
       obj_to_obj( create_money( ch->gold,COPPER ), corpse );
       if ( ch->silver > 0 )
       obj_to_obj( create_money( ch->silver,SILVER ), corpse );
       if ( ch->real_gold > 0 )
       obj_to_obj( create_money( ch->real_gold, GOLD ), corpse );
       ch->gold = 0;
       ch->silver = 0;
       ch->real_gold = 0;
    }

    for ( obj = ch->first_carrying; obj; obj = obj_next )
    {
	obj_next = obj->next_content;

	if ( !IS_SET( obj->second_flags, ITEM2_PERM ) )
	obj_from_char( obj );

	if ( IS_OBJ_STAT( obj, ITEM_INVENTORY )
	  || IS_OBJ_STAT( obj, ITEM_DEATHROT ) )
	    extract_obj( obj );
	else if ( !IS_SET(obj->second_flags, ITEM2_PERM ) )
	    obj_to_obj( obj, corpse );
	else
	    continue;
     }

 if ( !DT )
 {
    if ( IS_NPC(ch) || (!IS_NPC(ch) && ch->level > 10 && !MAIM_DEATH) )
        obj_to_room( corpse,old_room );
    else
        obj_to_room( corpse,location );
 }
 else
 obj_to_room( corpse, dtroom );

    	return;

}

/*
 * Make a new locker for a character.
 */
void create_locker( CHAR_DATA *ch, CHAR_DATA *killer )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *locker;
    char *name;
    ROOM_INDEX_DATA *location;

    if ( !ch || !ch->in_room )
    {
      bug("ERROR!!",0);
      return;
    }

   location = get_room_index ((int)ch->in_room->vnum,(int)ch->in_room->area->zone->number );
 
	name		= ch->name;
	locker		= create_object(get_obj_index(OBJ_VNUM_LOCKER,1),0,first_zone);

    /* Added corpse name - make locate easier , other skills */
    sprintf( buf, "locker %s", name );
    STRFREE( locker->name );
    locker->name = STRALLOC( buf );

    sprintf( buf, locker->short_descr, name );
    STRFREE( locker->short_descr );
    locker->short_descr = STRALLOC( buf );

    sprintf( buf, locker->description, name );
    STRFREE( locker->description );
    locker->description = STRALLOC( buf );

        obj_to_room( locker,location );
    	return;

}


void make_blood( CHAR_DATA *ch )
{
	OBJ_DATA *obj;

	obj		= create_object( get_obj_index( OBJ_VNUM_BLOOD,1), 0 ,first_zone);
	if ( !obj )
	return;

	obj->timer	= number_range( 2, 4 );
	obj->value[1]   = number_range( 3, UMIN(5, ch->level) );
	if ( !IS_NPC(ch) )
	obj_to_room( obj, ch->in_room );
	else
	obj_to_room( obj, ch->in_room );
}


void make_bloodstain( CHAR_DATA *ch )
{
	OBJ_DATA *obj;

	obj		= create_object(get_obj_index(OBJ_VNUM_BLOODSTAIN,1 ), 0,first_zone );
	if ( !obj )
	return;
	obj->timer	= number_range( 1, 2 );
	obj_to_room( obj, ch->in_room );
}


/*
 * make some coinage
 */
OBJ_DATA *create_money( int amount, int type )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj=NULL;

    if ( amount <= 0 )
    {
	bug( "Create_money: zero or negative money %d.", amount );
	amount = 1;
    }

switch( type )
{
case COPPER:
    if ( amount == 1 )
    {
	obj = create_object( get_obj_index( OBJ_VNUM_MONEY_COPPER_ONE,1 ),0,first_zone );
    }
    else
    {
	obj = create_object( get_obj_index( OBJ_VNUM_MONEY_COPPER_SOME,1 ),0,first_zone );

	if ( !obj )
	return NULL;

	sprintf( buf, obj->short_descr, amount );
	STRFREE( obj->short_descr );
	obj->short_descr = STRALLOC( buf );
	obj->value[0]	 = amount;
    }
break;
case SILVER:
    if ( amount == 1 )
    {
	obj = create_object( get_obj_index( OBJ_VNUM_MONEY_SILVER_ONE,1 ),0,first_zone );
    }
    else
    {
	obj = create_object( get_obj_index( OBJ_VNUM_MONEY_SILVER_SOME,1 ),0,first_zone );

	if ( !obj )
	return NULL;

	sprintf( buf, obj->short_descr, amount );
	STRFREE( obj->short_descr );
	obj->short_descr = STRALLOC( buf );
	obj->value[0]	 = amount;
    }
break;
case GOLD:
    if ( amount == 1 )
    {
	obj = create_object( get_obj_index( OBJ_VNUM_MONEY_ONE,1 ),0,first_zone );
    }
    else
    {
	obj = create_object( get_obj_index( OBJ_VNUM_MONEY_SOME,1 ),0,first_zone );

	if ( !obj )
	return NULL;

	sprintf( buf, obj->short_descr, amount );
	STRFREE( obj->short_descr );
	obj->short_descr = STRALLOC( buf );
	obj->value[0]	 = amount;
    }
break;

}

    return obj;
}
