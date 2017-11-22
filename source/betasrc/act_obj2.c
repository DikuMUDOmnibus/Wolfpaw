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
 *			Object Handling Module 2			    *
 *--------------------------------------------------------------------------*/
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "mud.h"

bool anti_class_check( CHAR_DATA *ch, OBJ_DATA *obj );

/*
 * Checks to see if the person has the right class to use the item --GW
 * Returns:
 * TRUE is they may use it
 * FALSE if they may not
 */
bool anti_class_check( CHAR_DATA *ch, OBJ_DATA *obj )
{
bool mage=FALSE;
bool cleric=FALSE;
bool warrior=FALSE;
bool thief=FALSE;
bool vampire=FALSE;
bool druid=FALSE;
bool ranger=FALSE;
bool augurer=FALSE;
bool kinju=FALSE;
bool paladin=FALSE;
bool assassin=FALSE;
bool psi=FALSE;
bool werewolf=FALSE;
bool avatar=FALSE;
bool dragon=FALSE;
bool bishop=FALSE;
bool archmage=FALSE;
bool blademaster=FALSE;
bool alchemist=FALSE;
bool prophet=FALSE;
bool crusader=FALSE;
bool samurai=FALSE;
bool hunter=FALSE;
bool adept=FALSE;
bool ninja=FALSE;
bool jackel=FALSE;
bool wraith=FALSE;
bool mystic=FALSE;

/* mobs can wear all ! */
if ( IS_NPC(ch) )
return TRUE;

/* Gods can wear all ! */
if ( IS_IMMORTAL(ch) )
return TRUE;

/* Figure out what bits are set on it  */
/* So its cludgy and slow .. sue me */
if ( IS_SET( obj->extra_flags, ITEM_MAGE_ONLY ) )
mage=TRUE;
if ( IS_SET( obj->extra_flags, ITEM_THIEF_ONLY ) )
thief=TRUE;
if ( IS_SET( obj->extra_flags, ITEM_WARRIOR_ONLY ) )
warrior=TRUE;
if ( IS_SET( obj->extra_flags, ITEM_CLERIC_ONLY ) )
cleric=TRUE;
if ( IS_SET( obj->extra_flags, ITEM_VAMPIRE_ONLY ) )
vampire=TRUE;
if ( IS_SET( obj->extra_flags, ITEM_DRUID_ONLY ) )
druid=TRUE;
if ( IS_SET( obj->second_flags, ITEM2_RANGER_ONLY ) )
ranger=TRUE;
if ( IS_SET( obj->second_flags, ITEM2_AUGURER_ONLY ) )
augurer=TRUE;
if ( IS_SET( obj->second_flags, ITEM2_KINJU_ONLY ) )
kinju=TRUE;
if ( IS_SET( obj->second_flags, ITEM2_PALADIN_ONLY ) )
paladin=TRUE;
if ( IS_SET( obj->second_flags, ITEM2_ASSASSIN_ONLY ) )
assassin=TRUE;
if ( IS_SET( obj->second_flags, ITEM2_PSI_ONLY ) )
psi=TRUE;
if ( IS_SET( obj->second_flags, ITEM2_WEREWOLF_ONLY ) )
werewolf=TRUE;
if ( IS_SET( obj->second_flags, ITEM2_AVATAR_ONLY ) )
avatar=TRUE;
if ( IS_SET( obj->second_flags, ITEM2_DRAGON_ONLY ) ) 
dragon=TRUE;
if ( IS_SET( obj->second_flags, ITEM2_BISHOP_ONLY ) ) 
bishop=TRUE;
if ( IS_SET( obj->second_flags, ITEM2_ARCHMAGE_ONLY ) )
archmage=TRUE;
if ( IS_SET( obj->second_flags, ITEM2_BLADEMASTER_ONLY ) )
blademaster=TRUE;
if ( IS_SET( obj->second_flags, ITEM2_ALCHEMIST_ONLY ) )
alchemist=TRUE;
if ( IS_SET( obj->second_flags, ITEM2_PROPHET_ONLY ) )
prophet=TRUE;
if ( IS_SET( obj->second_flags, ITEM2_CRUSADER_ONLY ) ) 
crusader=TRUE;
if ( IS_SET( obj->second_flags, ITEM2_SAMURAI_ONLY ) ) 
samurai=TRUE;
if ( IS_SET( obj->second_flags, ITEM2_WRAITH_ONLY ) )
wraith=TRUE;
if ( IS_SET( obj->second_flags, ITEM2_HUNTER_ONLY ) )
hunter=TRUE;
if ( IS_SET( obj->second_flags, ITEM2_ADEPT_ONLY ) )
adept=TRUE;
if ( IS_SET( obj->second_flags, ITEM2_NINJA_ONLY ) )
ninja=TRUE;
if ( IS_SET( obj->second_flags, ITEM2_JACKEL_ONLY ) ) 
jackel=TRUE;
if ( IS_SET( obj->second_flags, ITEM2_MYSTIC_ONLY ) )
mystic=TRUE;

/* 1st Classes */
switch( ch->class )
{
  case CLASS_MAGE:
     if ( mage )
       return TRUE;
      break;
  case CLASS_WARRIOR:
     if ( warrior )
       return TRUE;
      break;
  case CLASS_CLERIC:
     if ( cleric )
       return TRUE;
      break;
  case CLASS_THIEF:
     if ( thief )
       return TRUE;
      break;
  case CLASS_VAMPIRE:
     if ( vampire )
       return TRUE;
      break;
  case CLASS_DRUID:
     if ( druid )
       return TRUE;
      break;
  case CLASS_RANGER:
     if ( ranger )
       return TRUE;
      break;
  case CLASS_AUGURER:
     if ( augurer )
       return TRUE;
      break;
  case CLASS_KINJU:
     if ( kinju )
       return TRUE;
      break;
  case CLASS_PALADIN:
     if ( paladin )
       return TRUE;
      break;
  case CLASS_ASSASSIN:
     if ( assassin )
       return TRUE;
      break;
  case CLASS_PSIONICIST:
     if ( psi )
       return TRUE;
      break;
  case CLASS_WEREWOLF:
     if ( werewolf )
       return TRUE;
      break;
}

/* Second Classes */
switch( ch->class2 )
{
  case CLASS_MAGE:
     if ( mage )
       return TRUE;
      break;
  case CLASS_WARRIOR:
     if ( warrior )
       return TRUE;
      break;
  case CLASS_CLERIC:
     if ( cleric )
       return TRUE;
      break;
  case CLASS_THIEF:
     if ( thief )
       return TRUE;
      break;
  case CLASS_VAMPIRE:
     if ( vampire )
       return TRUE;
      break;
  case CLASS_DRUID:
     if ( druid )
       return TRUE;
      break;
  case CLASS_RANGER:
     if ( ranger )
       return TRUE;
      break;
  case CLASS_AUGURER:
     if ( augurer )
       return TRUE;
      break;
  case CLASS_KINJU:
     if ( kinju )
       return TRUE;
      break;
  case CLASS_PALADIN:
     if ( paladin )
       return TRUE;
      break;
  case CLASS_ASSASSIN:
     if ( assassin )
       return TRUE;
      break;
  case CLASS_PSIONICIST:
     if ( psi )
       return TRUE;
      break;
  case CLASS_WEREWOLF:
     if ( werewolf )
       return TRUE;
      break;
}

/* Advanced Classes */
switch( ch->advclass )
{
  case CLASS_AVATAR:
     if ( avatar )
       return TRUE;
      break;
  case CLASS_DRAGON:
     if ( dragon )
       return TRUE;
      break;
}

/* Dual Advanced Classes */
switch( ch->advclass2 )
{
  case CLASS_BISHOP:
     if ( bishop )
       return TRUE;
      break;
  case CLASS_ARCHMAGE:
     if ( archmage )
       return TRUE;
      break;
  case CLASS_BERSERKER:
     if ( blademaster )
       return TRUE;
      break;
  case CLASS_ALCHEMIST:
     if ( alchemist )
       return TRUE;
      break;
  case CLASS_PROPHET:
     if ( prophet )
       return TRUE;
      break;
  case CLASS_CRUSADER:
     if ( crusader )
       return TRUE;
      break;
  case CLASS_SAMURAI:
     if ( samurai )
       return TRUE;
      break;
  case CLASS_WRAITH:
     if ( wraith )
       return TRUE;
      break;
  case CLASS_HUNTER:
     if ( hunter )
       return TRUE;
      break;
  case CLASS_ADEPT:
     if ( adept )
       return TRUE;
      break;
  case CLASS_NINJA:
     if ( ninja )
       return TRUE;
      break;
  case CLASS_JACKEL:
     if ( jackel )
       return TRUE;
      break;
  case CLASS_MYSTIC:
     if ( mystic )
       return TRUE;
     break;
}

/* Ok So it has NONE of these bits .. let it pass --GW */
if ( !mage && !thief && !warrior && !cleric && !vampire && !druid &&
     !ranger && !augurer && !kinju && !paladin && !assassin && !psi &&
     !werewolf && !avatar && !dragon && !bishop && !archmage &&
     !blademaster && !alchemist && !prophet && !crusader && !samurai &&
     !wraith && !hunter && !adept && !ninja && !jackel && !mystic )
     return TRUE;

/* Guess your screwed.. --GW */
return FALSE;
}

/*
 * Figure out if the char has life protection on --GW
 * TRUE if they do
 * FALSE if not
 */
bool get_life_protection_char( CHAR_DATA *ch )
{
OBJ_DATA *obj;
ROOM_INDEX_DATA *startroom;
char buf[MSL];

startroom = get_room_index( ch->pcdata->start_room, 1 );

for( obj=ch->last_carrying; obj; obj=obj->prev_content )
{
  if ( obj->item_type == ITEM_LIFE_PROTECTION )
  {
	obj_from_char(obj);
	extract_obj(obj);
	send_to_char("Your Life Protection Crumbles to Dust..\n\r",ch);
	send_to_char("Your Life has been Spared!\n\r",ch);
	sprintf(buf,"%s had a LIFE_PROTECTION Object, voiding Death",
		capitalize(ch->name));
	log_string(buf);
	stop_fighting(ch,TRUE);
	char_from_room(ch);
	char_to_room(ch,startroom);
	ch->hit = ch->max_hit;
        ch->mana = ch->max_mana;
        ch->move = ch->max_move;
	save_char_obj(ch);
	return TRUE;
  }

}

 return FALSE;
}

