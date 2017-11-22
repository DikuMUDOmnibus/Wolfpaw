/*--------------------------------------------------------------------------*
 *                         ** WolfPaw 1.0 **                                *
 *--------------------------------------------------------------------------*
 *               WolfPaw 1.0 (c) 1997,1998 by Dale Corse                    *
 *--------------------------------------------------------------------------*
 *            The WolfPaw Coding Team is headed by: Greywolf                *
 *  With the Assitance from: Callinon, Dhamon, Sentra, Wyverns, Altrag      *
 *  Scryn, Thoric, Justice, Tricops and Mask.                               *
 *--------------------------------------------------------------------------*
 *    	        Smart Mobs Mobile Action Automation Module		    *
 *--------------------------------------------------------------------------*/
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

void smart_update args( ( CHAR_DATA *mob, CHAR_DATA *fighter ) );
void kill_weak	  args( ( CHAR_DATA *mob ) );
void sac_inventory args( ( CHAR_DATA *mob ) );
void corpse_sac   args( ( CHAR_DATA *mob ) );
void corpse_loot  args( ( CHAR_DATA *mob ) );
void smart_archmage( CHAR_DATA *ch, CHAR_DATA *victim );
void smart_blademaster( CHAR_DATA *ch, CHAR_DATA *victim );
void smart_bishop( CHAR_DATA *ch, CHAR_DATA *victim );
void smart_wraith( CHAR_DATA *ch, CHAR_DATA *victim );

/* This file contains the code needed to make mobs do things
 * set by the mset <mob> smart command --GW
 */

/* 
 * Triggers all the 'brains' of the operation --GW
 */
void smart_update( CHAR_DATA *mob, CHAR_DATA *fighter )
{

/* Attack Weakest Opponent --GW */
if ( IS_SET( mob->smart, SMART_ATTACK_WEAK ) )
kill_weak(mob);

/* Loot a Corpse, and sac what it grabs --GW */
if ( IS_SET( mob->smart, SMART_LOOTSAC ) )
sac_inventory( mob );

/* Sac any corpse lying around --GW */
if ( IS_SET( mob->smart, SMART_CORPSESAC ) )
corpse_sac(mob);

/* Just loots Corpses --GW*/
if ( IS_SET( mob->smart, SMART_LOOTER ) )
corpse_loot(mob);

/* Smart Classes --GW */
if ( IS_SET( mob->smart, SMART_ARCHMAGE ) )
smart_archmage(mob,fighter);

if ( IS_SET( mob->smart, SMART_BLADEMASTER ) )
smart_blademaster(mob,fighter);

if ( IS_SET( mob->smart, SMART_BISHOP ) )
smart_bishop(mob,fighter);

if ( IS_SET( mob->smart, SMART_WRAITH ) )
smart_wraith(mob,fighter);

return;
}

/*
 * Find the Weakest Playerin the room, and slaughter them --GW
 */
void kill_weak( CHAR_DATA *mob )
{
CHAR_DATA *temp, *temp_next;
int low_hit=32000;
char buf[MSL];
char attack[MSL];

if ( !mob->fighting || !mob->fighting->who )
return;

for( temp = mob->in_room->first_person; temp; temp = temp_next )
{

if ( temp == NULL )
{
bug("Temp is null!",0);
return;
}

if ( !IS_NPC(temp) && temp->max_hit < low_hit )
{
low_hit = temp->max_hit;
strcpy(attack,temp->name);
}

if ( temp->next_in_room != NULL )
temp_next = temp->next_in_room;
else
{
break;
}

}

if ( attack[0] == '\0' )
return;

if ( ( temp = get_char_room( mob, attack ) )==NULL ||
temp->in_room->vnum != mob->in_room->vnum )
{
return;
}

if ( temp && mob->fighting->who != temp && temp != mob )
{
stop_fighting( mob, TRUE );
act( AT_ACTION, "$n realizes $N is the weaker opponent! BANZAI!!",
	mob,NULL,temp,TO_ROOM);
act( AT_ACTION, "$n realizes YOU are the WEAKER opponent! BANZAI!!",
	mob,NULL,temp,TO_VICT);
multi_hit( mob, temp, TYPE_UNDEFINED );
multi_hit( temp, mob, TYPE_UNDEFINED );
sprintf(buf,"SMART-MOB-LOG: %s Switching to Weaker Opponent %s",
	mob->short_descr, temp->name );
to_channel(buf,CHANNEL_MOBLOG,"[ ** ",LEVEL_LOG);
}

return;
}

/*
 * sac items looted or given or picked up. --GW
 */ 
void sac_inventory( CHAR_DATA *mob )
{
OBJ_DATA *obj;
char buf[MSL];
int rand=0;

for ( obj = mob->first_carrying; obj; obj = obj->next_content )
{

if ( obj )
{

if ( IS_SET( obj->second_flags, ITEM2_NO_SAC ) )
continue;

/* Hrmm .. lets give them SOME chance.. --GW*/
rand=number_range(1,2);
if ( rand==1 )
{
  do_wear(mob,"all");
  sprintf(buf,"SMART-MOB-LOG: %s WEARING %s.", mob->short_descr, 
	obj->short_descr );
to_channel(buf,CHANNEL_MOBLOG,"[ ** ",LEVEL_LOG);
  return;
}

sprintf(buf,"$n sacrifices %s",obj->short_descr );
act(AT_ACTION,buf,mob,NULL,NULL,TO_ROOM);
sprintf(buf,"SMART-MOB-LOG: %s sacrificing %s.", mob->short_descr, 
	obj->short_descr );
extract_obj( obj );
to_channel(buf,CHANNEL_MOBLOG,"[ ** ",LEVEL_LOG);
break;
}

}

return;
}

/*
 * Sacrifice the ENTIRE Corpse. --GW
 */
void corpse_sac( CHAR_DATA *mob )
{
OBJ_DATA *temp, *temp_next;
char buf[MSL];

if ( !mob->in_room )
return;

/* Find out if theres a Corpse (pc or NPC) on the ground */
for( temp = mob->in_room->first_content; temp; temp = temp_next )
{
temp_next = temp->next_content;
if ( temp->item_type == ITEM_CORPSE_NPC || 
     temp->item_type == ITEM_CORPSE_PC )
{
/*found 1, sac it.*/
sprintf(buf,"$n sacrifices %s.",temp->short_descr);
act(AT_ACTION,buf,mob,NULL,NULL,TO_ROOM);
extract_obj( temp );
sprintf(buf,"SMART-MOB-LOG: %s sacrificing %s.", mob->short_descr,
	temp->short_descr );
to_channel(buf,CHANNEL_MOBLOG,"[ ** ",LEVEL_LOG);
break;
}
}

return;
}

void corpse_loot( CHAR_DATA *mob )
{
OBJ_DATA *temp, *temp_next, *temp2;
char buf[MSL];

/* Make Sure Mob is in a room */
if ( !mob->in_room )
return;

/* Find out if theres a Corpse (pc or NPC) on the ground */
for( temp = mob->in_room->first_content; temp; temp = temp_next )
{
temp_next = temp->next_content;
if ( temp->item_type == ITEM_CORPSE_NPC || 
     temp->item_type == ITEM_CORPSE_PC )
{
/*found 1, loot it.*/
if ( !temp->first_content )
{
extract_obj(temp);
continue;
}

sprintf(buf,"SMART-MOB-LOG: %s looting %s.", mob->short_descr,
	temp->short_descr );
to_channel(buf,CHANNEL_MOBLOG,"[ ** ",LEVEL_LOG);

for ( temp2 = temp->first_content; temp2; temp2 = temp2->next_content )
{
if ( !temp2 )
break;

separate_obj(temp2);

sprintf(buf,"$n gets $p from %s",temp->short_descr);
act(AT_PLAIN,buf,mob,temp2,NULL,TO_ROOM);
obj_from_obj( temp2 );
obj_to_char( temp2, mob );
do_wear(mob,"all"); /* Sneer .. lets see what people think of this one .. =) --GW */
sprintf(buf,"SMART-MOB-LOG: %s WEARING %s.", mob->short_descr,
	temp->short_descr );
to_channel(buf,CHANNEL_MOBLOG,"[ ** ",LEVEL_LOG);
}
 
break;
}
}

return;
}

/*
 * SMART Class Settings Follow, makeing a mob fight smarter, being that
 * they will cast the spells and use the skills of the class bit they have
 * set on them .. an attempt to i guess make fighting mobiles more like 
 * fighting something with a brain, not just hitting a brick wall --GW
 */
void smart_archmage( CHAR_DATA *ch, CHAR_DATA *victim )
{
int rand;
OBJ_DATA *obj;
int cnt;


/* Remove my weapon if the person is IMMUNE to it */
/* Supports just peirce right now, as thats the only total immunity out there --GW*/
if ( victim && IS_SET( victim->immune, RIS_PIERCE) &&
   ((( obj = get_eq_char( ch, WEAR_WIELD )) != NULL) &&
   obj->value[3] == 11 ))
   remove_obj(ch, obj->wear_loc, TRUE );

/* Make the Mobile heal itself 4 times if hp is less then max*/
if ( ch->hit < ch->max_hit )
   for( cnt=1; cnt < 4; cnt++ )
     interpret( ch, "cast 'life force'" );

rand = number_range( 1, 11 );

if ( !ch->fighting || !victim )
  return;

switch( rand )   
{
   case 1: /* Fire Storm */
      interpret( ch, "cast 'fire storm'");
      break;
   case 2: /* Lightning Ball */
      interpret( ch, "cast 'lightning ball'");
      break;
   case 5: /* Mystic Mist */
      interpret( ch, "cast 'mystic mist'");
      break;
   case 6: /* Wizard Fire */
      interpret( ch, "cast 'wizard fire'");
      break;
   case 7: /* Orb of Draining */
      interpret( ch, "cast 'orb of draining'");
      break;
   case 8: /* Webbing */
      interpret( ch, "cast 'webbing'");
      break;
   case 9: /* Life Force */
      interpret( ch, "cast 'life force'");
      break;
   case 10: /* Devil's Hand */
      interpret( ch, "cast 'devils hand'");
      break;
   case 11: /* Merlin's Robe */
      interpret( ch, "cast 'merlins robe'");
      break;
}

return;
}

void smart_blademaster( CHAR_DATA *ch, CHAR_DATA *victim )
{
OBJ_DATA *obj;
int rand;
int cnt;

/* Remove my weapon if the person is IMMUNE to it */
/* Supports just peirce right now, as thats the only total immunity out there --GW*/
if (victim && IS_SET( victim->immune, RIS_PIERCE) &&
   ((( obj = get_eq_char( ch, WEAR_WIELD )) != NULL) &&
   obj->value[3] == 11 ))
   remove_obj(ch, obj->wear_loc, TRUE );

/* Make the Mobile heal itself 4 times if hp is less then max*/
if ( ch->hit < ch->max_hit )
   for( cnt=1; cnt < 4; cnt++ )
     interpret( ch, "cas 'life force'" );

rand = number_range( 1, 7 );

if ( !ch->fighting || !victim )
  return;

switch( rand )
{
   case 1: /* berserk */
    interpret( ch, "berserk" );
    break;
   case 2: /* rage */
    interpret( ch, "rage" );
    break;
   case 3:
   case 4: /* hack */
    interpret( ch, "hack" );
    break;
   case 5: /* trash */
    interpret( ch, "trash" );
    break;
   case 6: /* dpunch */
    interpret( ch, "dpunch" );
    break;
   case 7: /* dtouch */
    interpret( ch, "dtouch" );
    break;
   case 8: /* bandage */
    interpret( ch, "bandage" );
    break;
}

return;
}

void smart_bishop( CHAR_DATA *ch, CHAR_DATA *victim )
{
OBJ_DATA *obj;
int rand;
int cnt;

/* Remove my weapon if the person is IMMUNE to it */
/* Supports just peirce right now, as thats the only total immunity out there --GW*/
if (victim && IS_SET( victim->immune, RIS_PIERCE) &&
   ((( obj = get_eq_char( ch, WEAR_WIELD )) != NULL) &&
   obj->value[3] == 11 ))
   remove_obj(ch, obj->wear_loc, TRUE );

/* Make the Mobile heal itself 4 times if hp is less then max*/
if ( ch->hit < ch->max_hit )
   for( cnt=1; cnt < 4; cnt++ )
     interpret( ch, "cas 'life force'" );

rand = number_range( 1, 9 );

if ( !ch->fighting || !victim )
  return;

switch( rand )
{
case 1: /* Greater Heal */
  interpret( ch, "cast 'greater heal'" );
  break;
case 2: /* revive */
  interpret( ch, "cast 'revive'" );
  break;
case 3:
case 4: /* inner light */
  interpret( ch, "cast 'inner light'" );
  break;
case 5: /* constriction */
  interpret( ch, "cast 'constriction'" );
  break;
case 6: /* breath of life */
  interpret( ch, "cast 'breath of life'" );
  break;
case 7: /* wrath of god */
  interpret( ch, "cast 'wrath of god'" );
  break;
case 8: /* webbing */
  interpret( ch, "cast 'webbing'" );
  break;
case 9: /* life force */
  interpret( ch, "cast 'life force'" );
  break;
}

return;
}

void smart_wraith( CHAR_DATA *ch, CHAR_DATA *victim )
{
OBJ_DATA *obj;
int rand;
int cnt;

/* Remove my weapon if the person is IMMUNE to it */
/* Supports just peirce right now, as thats the only total immunity out there --GW*/
if (victim && IS_SET( victim->immune, RIS_PIERCE) &&
   ((( obj = get_eq_char( ch, WEAR_WIELD )) != NULL) &&
   obj->value[3] == 11 ))
   remove_obj(ch, obj->wear_loc, TRUE );

/* Make the Mobile heal itself 4 times if hp is less then max*/
if ( ch->hit < ch->max_hit )
   for( cnt=1; cnt < 4; cnt++ )
     interpret( ch, "cas 'life force'" );

rand = number_range( 1, 8 );

if ( !ch->fighting || !victim )
  return;

switch(rand)
{
case 1: /* drain essance */
  interpret( ch, "cas 'drain essence'" );
  break;
case 2: /* necro */
  interpret( ch, "cas 'necro'" );
  break;
case 3: /* undead shriek */
  interpret( ch, "cas 'undead shriek'" );
  break;
case 4: /* magic vaccum */
  interpret( ch, "cas 'magic vaccum'" );
  break;
case 5: /* mind wrack */
  interpret( ch, "cas 'mind wrack'" );
  break;
case 6: /* mind wrench */
  interpret( ch, "cas 'mind wrench'" );
  break;
case 7: /* unseen claw */
  interpret( ch, "cas 'unseen claw'" );
  break;
case 8: /* life force */
  interpret( ch, "cas 'life force'" );
  break;
}

return;
}

