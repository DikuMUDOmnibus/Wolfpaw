/*--------------------------------------------------------------------------*
 *                         ** WolfPaw 1.0 **                                *
 *--------------------------------------------------------------------------*
 *               WolfPaw 1.0 (c) 1997,1998 by Dale Corse                    *
 *--------------------------------------------------------------------------*
 *            The WolfPaw Coding Team is headed by: Greywolf                *
 *  With the Assitance from: Callinon, Dhamon, Sentra, Wyverns, Altrag      *
 *  Scryn, Thoric, Justice, Tricops and Mask.                               *
 *--------------------------------------------------------------------------*
 *			Magic Use Module			    	    *
 *--------------------------------------------------------------------------*/
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

/* Protos */
int get_num_mobs_master( int vnum, char *master );

/* Create Life Defs */
#define MOB_CREATE_WEMIC	 17
#define MOB_CREATE_GOBLIN	 18
#define MOB_CREATE_GARGOYLE	 19
#define MOB_CREATE_SMALL_DRAGON  20
#define MOB_CREATE_GIANT_DRAGON  21
#define MOB_CREATE_PSI_BLADE	 22
	 
/*
 * Creates Life .. generic --GW
 */
ch_ret spell_create_life( int sn, int level, CHAR_DATA *ch, void *vo )
{
CHAR_DATA *mob;
struct skill_type *skill = get_skilltype(sn);
sh_int num=0, cnt=0, vnum=0;
MOB_INDEX_DATA *mobi;
CHAR_DATA *victim=NULL;
CHAR_DATA *temp, *temp_next;
char buf[MSL];

if ( IS_NPC(ch) ) {
	return rNONE;
}

send_to_char("This spell has been disabled for Debugging.\n\r",ch);
return rNONE;

   if ( skill->hit_char && skill->hit_char[0] != '\0' )
      act( AT_MAGIC, skill->hit_char, ch, NULL, victim, TO_CHAR );
   if ( skill->hit_vict && skill->hit_vict[0] != '\0' )
      act( AT_MAGIC, skill->hit_vict, ch, NULL, victim, TO_VICT );
   if ( skill->hit_room && skill->hit_room[0] != '\0' )
      act( AT_MAGIC, skill->hit_room, ch, NULL, victim, TO_ROOM );


vnum = number_range(MOB_CREATE_WEMIC, MOB_CREATE_GIANT_DRAGON);
num = number_range( 1, 10 );
mobi = get_mob_index(vnum,1);
sprintf(buf,"%s", ch->name);

switch( vnum )
{
   case MOB_CREATE_WEMIC:
   break;
   case MOB_CREATE_GOBLIN:
   if ( num > 7 )
      num = 7;
   break;
   case MOB_CREATE_GARGOYLE:
   if ( num > 5 )
     num = 5;
   break;
   case MOB_CREATE_SMALL_DRAGON:
   if ( num > 3 )
     num = 2;
   break;
   case MOB_CREATE_GIANT_DRAGON:
     num = 1;
   break;
}

/* Slay any current Mobiles following the Char --GW */
for( temp = ch->in_room->first_person; temp; temp = temp_next )
{
  temp_next = temp->next_in_room;
  if ( temp->master == ch && IS_NPC(temp) )
    raw_kill(ch,temp);
}

/* Create More Followers --GW */
for( cnt = 1; cnt <= num; cnt++ )
{
mob = create_mobile( mobi, find_zone(1) );
xSET_BIT(mob->affected_by, AFF_CHARM );
char_to_room(mob,ch->in_room);
do_follow(mob,buf);
}

return rNONE;
}

/*
 * Skills and Spells for the 'Mystic's' Follow --GW
 */
ch_ret skill_focusing( int sn, int level, CHAR_DATA *ch, void *vo )
{
int move_cost=0;

move_cost = ch->max_move/4;
if ( ch->move < move_cost )
{
  send_to_char("You don't have enough Movement.\n\r",ch);
  return rSPELL_FAILED;
}

act(AT_MAGIC,"$n closes $s eyes, and wringles $s brow.. focusing.",ch,NULL,NULL,TO_ROOM);
act(AT_MAGIC,"You close your eyes, and concentrate .. focusing all your powers!",ch,NULL,NULL,TO_CHAR);
ch->mana = URANGE(0,(ch->mana + ch->max_mana/4), ch->max_mana);
ch->move = ch->move - move_cost;
return rNONE;
}

ch_ret spell_sphere( int sn, int level, CHAR_DATA *ch, void *vo )
{
AFFECT_DATA af;
CHAR_DATA *victim = (CHAR_DATA *) vo;

if ( is_affected( ch, sn ) )
  return rSPELL_FAILED;

if ( !victim )
victim = ch;

/*Room */
act(AT_MAGIC,"$n spreads $s arms wide, uttering strange words",ch,NULL,victim,TO_ROOM);
act(AT_MAGIC,"$n brings traces an outline around $s body...",ch,NULL,victim,TO_ROOM);
act(AT_RED,"$N is surrounded by a Powerful Sphere...",ch,NULL,victim,TO_ROOM);

/* Char */
act(AT_MAGIC,"You spread your arms wide, demanding protection",ch,NULL,NULL,TO_CHAR);
act(AT_MAGIC,"You trace an outline around your body...",ch,NULL,NULL,TO_CHAR);
act(AT_RED,"You are surrounded by a Powerful Sphere...",ch,NULL,victim,TO_CHAR);

af.type = sn;
af.duration = level*2 - 5;
af.location = APPLY_RESISTANT;
af.modifier = RIS_BLUNT;
xCLEAR_BITS(af.bitvector);
affect_to_char( victim, &af );
af.type = sn;
af.duration = level*2 - 5;
af.location = APPLY_RESISTANT;
af.modifier = RIS_SLASH;
xCLEAR_BITS(af.bitvector);
affect_to_char( victim, &af );
af.type = sn;
af.duration = level*2 - 5;
af.location = APPLY_RESISTANT;
af.modifier = RIS_PIERCE;
xCLEAR_BITS(af.bitvector);
affect_to_char( victim, &af );

return rNONE;
}

ch_ret spell_psi_blade( int sn, int level, CHAR_DATA *ch, void *vo )
{
CHAR_DATA *mob, *temp, *temp_next;
sh_int rand=0, cnt=0;
char buf[MSL];
MOB_INDEX_DATA *mobi;

if ( IS_NPC(ch) ) {
	return rNONE;
}

if ( in_arena(ch) || IS_SET(ch->pcdata->flags, PCFLAG_KOMBAT) )
{
  send_to_char("Win your own fights, Weenie!\n\r",ch);
  return rNONE;
}

mobi = get_mob_index(MOB_CREATE_PSI_BLADE,1);
sprintf(buf,"%s",ch->name);
rand = number_range(1,4);

/* Slay any current Mobiles following the Char --GW */
for( temp = ch->in_room->first_person; temp; temp = temp_next )
{
  temp_next = temp->next_in_room;
  if ( temp->master == ch && IS_NPC(temp) )
    raw_kill(ch,temp);
}

/* Ok - No more 'Psi Army's' --GW */
if ( !IS_IMMORTAL(ch) && ((rand+get_num_mobs_master(MOB_CREATE_PSI_BLADE,ch->name)) > 4 ))
{
char tmpbuf[MSL];

tmpbuf[0] = '\0';

  sprintf(tmpbuf,"%s tried to Create a 'Psi Army'! Die Sucker...!",capitalize(ch->name));
  log_string(tmpbuf);
  send_to_char("&W&wThe &RStrain &W&wis just &RTO MUCH&W&w, your &Rmind reals&W&w, and all goes black...\n\r",ch);
  act(AT_PLAIN,"$n falls to the ground &RSCREAMING!&W&w",ch,NULL,NULL,TO_ROOM);
  sprintf(tmpbuf,"&R[TSK TSK!]&w&W %s's Head Explodes from to much Mind Strain!",ch->name);
  echo_to_all(AT_PLAIN,tmpbuf,ECHOTAR_ALL);
  raw_kill(ch,ch);
  return rNONE;
}

for( cnt = 1; cnt <= rand; cnt++ )
{
mob = create_mobile(mobi, find_zone(1));
xSET_BIT(mob->affected_by, AFF_CHARM);
char_to_room(mob,ch->in_room);
do_follow(mob,buf);
}

return rNONE;
}

ch_ret spell_mind_bash( int sn, int level, CHAR_DATA *ch, void *vo )
{
AFFECT_DATA af;
CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( is_affected( victim, sn ) )
        return rSPELL_FAILED;

act(AT_RED,"$n summons $s power and creates a MASSIVE Fist!",ch,NULL,NULL,TO_ROOM);
act(AT_RED,"The MASSIVE Fist Pounds $N into the ground, OBLITERATING $M!",ch,NULL,victim,TO_ROOM);

af.type =	sn;
af.duration =   ch->level/3;
af.location =   APPLY_DAMROLL;
af.modifier =   -30;
xCLEAR_BITS(af.bitvector);
affect_to_char(victim, &af );

af.type =	sn;
af.duration =   ch->level/3;
af.location =   APPLY_HITROLL;
af.modifier =   -30;
xCLEAR_BITS(af.bitvector);
affect_to_char(victim, &af );

if ( !IS_NPC(ch) )
{
af.type =	sn;
af.duration =   ch->level/3;
af.location =   APPLY_MENTALSTATE;
af.modifier =   30;
xCLEAR_BITS(af.bitvector);
affect_to_char(victim, &af );
}

af.type =	sn;
af.duration =   ch->level/3;
af.location =   APPLY_AC;
af.modifier =   30;
xCLEAR_BITS(af.bitvector);
affect_to_char(victim, &af );

damage( ch, victim, 100, TYPE_UNDEFINED );
return rNONE;
}

ch_ret spell_warp_reality( int sn, int level, CHAR_DATA *ch, void *vo )
{

return rNONE;
}

ch_ret spell_ethereal_strike( int sn, int level, CHAR_DATA *ch, void *vo )
{
CHAR_DATA *victim = (CHAR_DATA *) vo;
sh_int rand=0;

act(AT_MAGIC,"$n's entire body distorts fading in and out of exsistance..",ch,NULL,NULL,TO_ROOM);
act(AT_MAGIC,"Gateways open in the palms of $s hands, pulsating for a moment",ch,NULL,NULL,TO_ROOM);
act(AT_RED,"Beams of Psionic Power SLAM into $N sending $M sprawling!",ch,NULL,victim,TO_ROOM);

act(AT_MAGIC,"Your entire body distorts fading in and out of exsistance..",ch,NULL,NULL,TO_CHAR);
act(AT_MAGIC,"Gateways open in the palms of your hands, pulsating for a moment",ch,NULL,NULL,TO_CHAR);
act(AT_RED,"Beams of Psionic Power SLAM into $N sending $M sprawling!",ch,NULL,victim,TO_CHAR);

damage( ch, victim, (ch->level*4), TYPE_UNDEFINED);

rand = number_range(1,6);
if ( rand == 2 )
{
act(AT_RED,"$n is Stunned!",victim,NULL,NULL,TO_CHAR);
act(AT_RED,"$n are Stunned!",victim,NULL,NULL,TO_ROOM);
victim->stun_rounds = 3;
}

return rNONE;
}
/*
 * New Archmage Spells --GW
 */
ch_ret spell_merlins_robe( int sn, int level, CHAR_DATA *ch, void *vo )
{
AFFECT_DATA af;
CHAR_DATA *victim = (CHAR_DATA *) vo;
OBJ_DATA *robe;

    if ( is_affected( victim, sn ) )
        return rSPELL_FAILED;

/* Room */
act(AT_MAGIC,"$n kneels down, making some strange sounds, waving $s arms",ch,NULL,NULL,TO_ROOM);
act(AT_MAGIC,"You hear the voice of the Ancient Wizard Merlin Greeting $n!",ch,NULL,NULL,TO_ROOM);
act(AT_MAGIC,"\n\r$n's body glows with a bright light, and $e seems stronger..",victim,NULL,NULL,TO_ROOM);

/* Char */
act(AT_MAGIC,"You kneel down, making some strange sounds, waving your arms",ch,NULL,NULL,TO_CHAR);
act(AT_MAGIC,"You hear the voice of the Ancient Wizard Merlin Greeting You!",ch,NULL,NULL,TO_CHAR);
act(AT_MAGIC,"\n\r$n's body glows with a bright light, and seems stronger..",victim,NULL,NULL,TO_CHAR);

af.type		= sn;
af.duration	= level*2 - 5;
af.location	= APPLY_NONE;
af.modifier	= 0;
af.bitvector    = meb(AFF_MERLINS_ROBE);
affect_to_char( victim, &af );

robe = create_object( get_obj_index(66,1), 0,first_zone);

obj_to_char(robe,victim);
equip_char(victim,robe,WEAR_ABOUT);

act(AT_RED,"$n's robes rustle quietly..",victim,NULL,NULL,TO_ROOM);
act(AT_RED,"Your robes rustle quietly..",victim,NULL,NULL,TO_CHAR);

return rNONE;
}

