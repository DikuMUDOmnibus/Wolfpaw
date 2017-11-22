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
 *			Death Handling Module			            *
 *--------------------------------------------------------------------------*/
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

bool death_handler args( ( CHAR_DATA *ch, CHAR_DATA *victim, bool npcvict ) );
void kombat_death args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void war_death args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void challenge_death args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void assassin_death args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void bounty_death args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
CHALLENGE_DATA *find_challenge args( ( CHAR_DATA *ch ) );
ARENA_DATA *find_arena args( ( CHALLENGE_DATA *challenge ) );
ARENA_DATA *find_arena_by_challenge args( ( CHALLENGE_DATA *challenge ) );
void raw_kill args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void remove_char        args( ( CHAR_DATA *ch ) );    
void do_char_ressurect args ( ( CHAR_DATA *ch ) );
void do_char_notressurect args ( ( CHAR_DATA *ch ) );
void ego_update	args( ( CHAR_DATA *ch, CHAR_DATA *victim, bool gain ) );
char *get_ego_text args( ( CHAR_DATA *ch ) );
CHAR_DATA *get_char args( ( char *name ) );
void create_ghost( CHAR_DATA *ch);
void ctf_death( CHAR_DATA *killer, CHAR_DATA *victim );
void ctf_channel( char *argument );
void flag_control( CHAR_DATA *ch, int flag, int type );

/*
 * Handles what happens when a Character Dies --GW
 */
bool death_handler( CHAR_DATA *ch, CHAR_DATA *victim, bool npcvict )
{
char sex[MSL];

   if ( !ch || !victim )
   {
      bug("Death_Handler: NULL ch || Victim!",0);
      return FALSE;
   }

   if ( !IS_NPC(ch) && !IS_NPC(victim) &&
    IS_SET(victim->pcdata->flags,PCFLAG_KOMBAT) )
   {
      kombat_death( ch, victim );
      return TRUE;
   }

/* CTF Death */
   if ( !IS_NPC(victim) && IS_SET(victim->pcdata->flagstwo, MOREPC_CTF))
   {
     ctf_death( ch, victim );
     return TRUE;
   }

/* Need 2 Challenge checks .. make sure we catch things both ways .. and dont
   have a character dying for real in the arena.. --GW */
   if( !IS_NPC(victim) && ((!IS_NPC(victim) && IS_SET(victim->pcdata->flags,PCFLAG_CHALLENGER)) ||
     (!IS_NPC(ch) && IS_SET( ch->pcdata->flags, PCFLAG_CHALLENGER))) &&
      ( in_arena(ch) && in_arena(victim) ) )
   {
     challenge_death( ch, victim );
     return TRUE;
   }

   if( !IS_NPC(victim) && ((!IS_NPC(victim) && IS_SET(victim->pcdata->flags,PCFLAG_CHALLENGED)) ||
     (!IS_NPC(ch) && IS_SET( ch->pcdata->flags, PCFLAG_CHALLENGED))) &&
        ( in_arena(ch) && in_arena(victim) ) )
   {
     challenge_death( ch, victim );
     return TRUE;
   }

   if ( !IS_NPC(victim) && !IS_NPC(ch) &&
    victim->position == POS_DEAD && (IS_SET(victim->pcdata->flagstwo,MOREPC_WAR)))
   {
      war_death( ch, victim );
      return TRUE;
   }

   if ( !IS_NPC(victim) && (get_life_protection_char(victim)==TRUE) )
    return TRUE;

   if ( !IS_NPC(ch) && !IS_NPC(victim) && IS_SET(victim->pcdata->flags, PCFLAG_ASSASSIN ) && ch != victim )
   {
      assassin_death( ch, victim );
      return TRUE;
   }

   if ( !IS_NPC(ch) && IS_AFFECTED(victim, AFF_BOUNTY) && ch != victim )
      bounty_death( ch, victim );

	if ( !IS_NPC(victim) && IS_NPC(ch) )
	{
          if ( victim != ch )
          {
            sprintf( log_buf, "&R[DEATH]&W %s Slaughtered by %s!",
                victim->name,(IS_NPC(ch) ? ch->short_descr : ch->name));
            echo_to_all(AT_PLAIN,log_buf,ECHOTAR_ALL);
          }
          else
          {
            sprintf( log_buf, "&R[DEATH]&W %s bleeds to death!",victim->name);
            echo_to_all(AT_PLAIN,log_buf,ECHOTAR_ALL);
          }
	}

	if ( !IS_NPC(victim) && !IS_NPC(ch) )
	{
          if ( victim != ch )
          {
            sprintf( log_buf, "&R[PLAYER KILL]&W %s just got terminally inconvienced by %s!",
                victim->name,(IS_NPC(ch) ? ch->short_descr : ch->name));
            echo_to_all(AT_PLAIN,log_buf,ECHOTAR_ALL);
            add_timer( victim, TIMER_PKILLED, 115, NULL, 0 );
 
	    ch->pcdata->pkills++;
	    victim->pcdata->pdeaths++;

	    if ( ch->pcdata->clan && victim->pcdata->clan )
		update_clan_pk_data(ch->pcdata->clan,victim->pcdata->clan);
          }
          else
          {
	    switch(victim->sex)
            {
	       case SEX_MALE:
		  sprintf(sex,"him");
	          break;
	       case SEX_FEMALE:
		  sprintf(sex,"her");
	          break;
	       case SEX_NEUTRAL:
		  sprintf(sex,"it");
	          break;
	    }

            sprintf( log_buf, "&C[BONEHEAD]&W %s killed %sself!",victim->name,
		sex);
            echo_to_all(AT_PLAIN,log_buf,ECHOTAR_ALL);
          }
	}

        if ( !npcvict )
        {
            sprintf( log_buf, "%s killed by %s at %d",
                victim->name,
                (IS_NPC(ch) ? ch->short_descr : ch->name),
                victim->in_room->vnum );
            log_string( log_buf );
            to_channel( log_buf, CHANNEL_MONITOR, "[ ** ",LEVEL_IMMORTAL);
        }

        stop_fighting(ch, TRUE);
        raw_kill( ch,victim );

return FALSE;
}

void kombat_death( CHAR_DATA *ch, CHAR_DATA *victim )
{
     int num_kom=0;
     char buf[MSL];

         send_to_char("&RYou &Bare &RDEAD &BSorry...\n\r",victim);
         stop_fighting(ch,TRUE);
         char_from_room(victim);
         char_to_room(victim,get_room_index(30601,1));
         sprintf(buf,"&R<&BMortal Kombat&R>&B %s has been &RSlain&B!",
                     capitalize(victim->name) );
         echo_to_all(AT_PLAIN,buf,ECHOTAR_ALL);
         ch->pcdata->num_kombatants++;
         REMOVE_BIT( victim->pcdata->flags, PCFLAG_KOMBAT );
         victim->hit = victim->max_hit;
         victim->position = 8;
         update_pos(victim);
         do_save(victim,"");
         num_kom = number_kombat( );
         if( num_kom == 1 )
         {
           sprintf(buf,"&R<&BMortal Kombat&R>&R %s &Bis &RViKtorious&B!",
                  capitalize(ch->name));
           echo_to_all(AT_PLAIN,buf,ECHOTAR_ALL);
           char_from_room(ch);
           char_to_room(ch,get_room_index(30601,1) );
           REMOVE_BIT( ch->pcdata->flags, PCFLAG_KOMBAT );
           ch->pcdata->num_kombats_won++;
           REMOVE_BIT( kombat->bits, KOMBAT_ON );
           REMOVE_BIT( kombat->bits, KOMBAT_DUAL );
           REMOVE_BIT( kombat->bits, KOMBAT_ADVANCED );
           REMOVE_BIT( kombat->bits, KOMBAT_NODRAGON );
           REMOVE_BIT( kombat->bits, KOMBAT_NOAVATAR );
           send_to_char("Your Quest Points Raise by 20!\n\r",ch);
           ch->pcdata->quest_curr += 20;
           ch->pcdata->quest_accum += 20;
           do_save(ch,"");
          }
         return;
}

void assassin_death( CHAR_DATA *ch, CHAR_DATA *victim )
{
        send_to_char("You are NO longer an ASSASSIN the price has been paid!", victim);
        REMOVE_BIT(victim->pcdata->flags, PCFLAG_ASSASSIN);
        do_save( victim, "" );
        stop_fighting(victim,TRUE);
        char_from_room(victim);
        char_to_room(victim,get_room_index(30601,1));
        victim->hit = victim->max_hit;
	update_pos(victim);
return;
}

void bounty_death( CHAR_DATA *ch, CHAR_DATA *victim )
{
char buf[MSL];

        ch->gold += victim->pcdata->bounty;
        send_to_char("You have recieved payment for your good work.\n\r",ch);
        sprintf(buf,"[ASSASSINATION] %s has completed the bounty on %s's head.\n\r",
        ch->name, victim->name);
        echo_to_all( AT_BLOOD,buf,ECHOTAR_ALL );
        SET_BIT(ch->pcdata->flags, PCFLAG_ASSASSIN);
        xREMOVE_BIT(victim->affected_by, AFF_BOUNTY);
        send_to_char("You are now an ASSASSIN!\n\r", ch );
        do_save( ch, "");
        stop_fighting(victim,TRUE);
        char_from_room(victim);
        char_to_room(victim,get_room_index(30601,1));
        victim->hit = victim->max_hit;
return;
}

void war_death( CHAR_DATA *ch, CHAR_DATA *victim )
{
char buf[MSL];

    sprintf(buf,"(WAR) -- %s has been SLAUGHTERED!!!!",capitalize(victim->name));
    echo_to_all(AT_BLOOD,buf,ECHOTAR_ALL);
    send_to_char("Your OUT OF HERE!!!!\n\r",victim);
    stop_fighting(ch, TRUE);
    stop_fighting(victim, TRUE);
    SET_BIT(victim->pcdata->flagstwo, MOREPC_OUTWAR);
    REMOVE_BIT(victim->pcdata->flagstwo, MOREPC_WAR);
    /* reset the players stats */
    SET_BIT( victim->position, POS_RESTING );
    victim->hit = victim->max_hit;
    update_pos( victim );
return;
}

void challenge_death(CHAR_DATA *ch, CHAR_DATA *victim)
{
CHALLENGE_DATA *challenge;
ARENA_DATA *arena;
char buf[MSL];
DESCRIPTOR_DATA *d;
CHAR_DATA *winner, *loser;

if(in_arena( ch )
    && in_arena( victim ) )
{
challenge = find_challenge( ch );
arena = find_arena_by_challenge(challenge);

/* 
 * Support for if the person kills themself eg: bleeds to death --GW
 */
if ( ch == victim )
{
if ( !str_cmp( ch->name, challenge->challenger ) )
winner=get_char(challenge->challenged);
else
winner=get_char(challenge->challenger);
loser=ch;
}
else
{
winner=ch;
loser=victim;
}

/*
 * Added Support for use of Charmed Mobies in the Arena .. weather or not there actually
 *  following there master or not --GW
 */
if ( IS_NPC(ch) ) /* The killer would be the mob . thats what kills the MUD */
{
  if ( ch->master ) /* Following a Master - Easy */
   winner = ch->master;
  else /* OK folks .. we have a Loose mobile here..... oy! */
  {
   CHAR_DATA *tmp, *tmp_next, *tmp2=NULL;
   bool found;

  /*
   * Unfortunetly we have to do this the long way - as Challenge could be NULL, and that
   * would bomb .. so .. well have to scan all the players online and find them ones in
   * the arena, and use those.. we can use the char list, as in this code .. mobs are in
   * the zone data .. not in the first/last_char lists. 
   * Note: We only need 1 guy to do a challenge lookup.. not 2.. the challenge arrays will do the rest
   * --GW
   */
   found = FALSE;
   for ( tmp=first_char; tmp; tmp = tmp_next )
   {
     tmp_next=tmp->next;

     /* Found the first guy */
     if ( in_arena(tmp) && !IS_NPC(tmp) &&
        (IS_SET(tmp->pcdata->flags, PCFLAG_CHALLENGED) || 
         IS_SET(tmp->pcdata->flags, PCFLAG_CHALLENGER)))
	{
         /* We have to re-lookup the arena there using, and the challenge
	    data, as we may have changed characters --GW */
   	challenge = find_challenge( tmp );

	if ( !challenge )
	  bug("NULL CHALLENGE!");

   	arena = find_arena_by_challenge(challenge);
	found = TRUE;
        break;
	}
   }

  if ( !found )
   bug("Challenge Not Found! We are GOING DOWN!!!",0);

       tmp2=get_char(challenge->challenger);

   if ( !str_cmp( victim->name, tmp2->name ) ) /* is it the Challenger thats dead? */
   {
     winner=get_char(challenge->challenged);
     victim=tmp2;
   }
   else /* Ok its the other fool */
   {
     winner=tmp2;
     victim=get_char(challenge->challenged);
   }
     
  }

   /* We have to re-lookup the arena there using, and the challenge data, as we may have changed
          characters --GW */
   /* Note it is needed in BOTH places - trust me =) --GW */
   challenge = find_challenge( winner );
   arena = find_arena_by_challenge(challenge);
}   

ch=winner;
victim=loser;

   if ( victim == ch && IS_SET( ch->pcdata->flags, PCFLAG_CHALLENGER ) )
   {
     if((victim=get_char_world(ch,challenge->challenged))==NULL)
     {
       victim = ch;
     }
   }

         sprintf(buf,"&r[CHALLENGE]&W %s has KICKED %s's BUTT!!!\n\r",
         ch->name, victim->name);
         echo_to_all( AT_PLAIN,buf,ECHOTAR_ALL );
         ch->pcdata->awins += 1;
         victim->pcdata->alosses += 1;
	 pk_champ_check(ch,victim);

for ( d = first_descriptor; d; d = d->next )
{

if( d->character && d->character->pcdata->gladiator && ch )
{

if ( d->character->pcdata->gladiator == ch->name )
{
  d->character->gold += (d->character->pcdata->plr_wager * 2);
  send_to_char("&R[&BGAMBLE&R]&W You won!\n\r",d->character);
  /* reset the info */
  d->character->pcdata->gladiator = NULL;
  d->character->pcdata->plr_wager = 0;
}

if(d->character->pcdata->gladiator != ch->name
&& d->character->pcdata->plr_wager >= 1)
{
 d->character->gold -= d->character->pcdata->plr_wager;
 send_to_char("&R[&BGAMBLE&R]&W You lose!\n\r",d->character);

 /* reset the betting info info */
 d->character->pcdata->gladiator = NULL;
 d->character->pcdata->plr_wager = 0;
}
}
}

/* now move both fighters out of arena
   and back to the regular "world"
   be sure to define ROOM_VNUM_AWINNER and ROOM_VNUM_ALOSER
*/
	 /* Arena Viewers gotta go now ..--GW */
	  stop_viewers( );

         stop_fighting(ch,TRUE);
         char_from_room(victim);
         char_to_room(victim,get_room_index(30601,1));
	 do_look(victim,"auto");
         char_from_room(ch);
         char_to_room(ch,get_room_index(30602,1));
	 do_look(ch,"auto");

        if(IS_SET(ch->pcdata->flags, PCFLAG_CHALLENGER))
         REMOVE_BIT(ch->pcdata->flags, PCFLAG_CHALLENGER);
        else  if(IS_SET(victim->pcdata->flags, PCFLAG_CHALLENGER))
         REMOVE_BIT(victim->pcdata->flags, PCFLAG_CHALLENGER);
        if(IS_SET(victim->pcdata->flags, PCFLAG_CHALLENGED))
         REMOVE_BIT(victim->pcdata->flags, PCFLAG_CHALLENGED);
        else if(IS_SET(ch->pcdata->flags, PCFLAG_CHALLENGED))
         REMOVE_BIT(ch->pcdata->flags, PCFLAG_CHALLENGED);

         /* reset the two players stats */
         SET_BIT( victim->position, POS_RESTING );
         SET_BIT( ch->position, POS_RESTING );
         ch->hit = ch->max_hit;
         victim->hit = victim->max_hit;
         update_pos( victim );

	 if ( !challenge )
	 return;

	 UNLINK(challenge, first_challenge, last_challenge, next, prev );
         DISPOSE(challenge);
	 arena->in_use = FALSE; /* Clear the Arena */
}
return;
}

void raw_kill( CHAR_DATA *ch, CHAR_DATA *victim )
{
   char buf[MAX_STRING_LENGTH];

    if ( !ch )
    {
      bug( "raw_kill: null char!",0);
        return;
    }

    if ( !victim )
    {
      bug( "raw_kill: null victim!", 0 );
      return;
    }
/* backup in case hp goes below 1 */
    if (!IS_NPC(victim) && NOT_AUTHED(victim))
    {
      bug( "raw_kill: killing unauthed", 0 );
      return;
    }

    stop_fighting( victim, TRUE );
    stop_fighting( ch, TRUE );

   if ( !IS_NPC(ch) )
    ch->alignment = align_compute( ch, victim ); /* Hopefully this'll
                                                     correct our little
                                                     bug -- Cal */

    mprog_death_trigger( ch, victim );
    if ( char_died(victim) )
      return;

    rprog_death_trigger( ch, victim );
    if ( char_died(victim) )
      return;

    /* Nuke auction if they are auctioning and Die --GW */
    if ( !IS_NPC(victim) && (auction->item != NULL && auction->seller != NULL &&
         auction->buyer != NULL &&
         ( !str_cmp( auction->buyer->name, victim->name ) ||
           !str_cmp( auction->seller->name, victim->name ) ) ) )
        {
          sprintf(buf,"&R%s has been killed, gold and auction object have been returned.&W",
		victim->name);
          talk_auction(buf,TRUE);
          obj_to_char( auction->item, auction->seller );
          auction->buyer->real_gold += auction->bet;
          auction->item = NULL;
        }

  if ( !victim->died_in_room )
    victim->died_in_room = victim->in_room;

    /* Follow */
    die_follower(victim);

    make_corpse( victim, ch, FALSE );
    make_blood( ch );

    /* Ghosts --GW */
    if ( !IS_NPC(victim) )
         create_ghost(victim);

    if ( IS_NPC(victim) )
    {
        victim->pIndexData->killed++;
        extract_char( victim, TRUE );
        victim = NULL;
        return;
    }

    ego_update( victim, ch, FALSE );

    extract_char( victim, FALSE );
    if ( !victim )
    {
      bug( "oops! raw_kill: extract_char destroyed pc char", 0 );
      return;
    }
    while ( victim->first_affect )
        affect_remove( victim, victim->first_affect );
    xSET_BITS(victim->affected_by, race_table[victim->race]->affected);
    victim->resistant   = 0;
    victim->susceptible = 0;
    victim->immune      = 0;
    victim->carry_weight= 0;
    victim->armor       = 100;
    victim->mod_str     = 0;
    victim->mod_dex     = 0;
    victim->mod_wis     = 0;
    victim->mod_int     = 0;
    victim->mod_con     = 0;
    victim->mod_cha     = 0;
    victim->mod_lck     = 0;
    victim->damroll     = 0;
    victim->hitroll     = 0;
    victim->mental_state = -10;
    victim->alignment   = URANGE( -1000, victim->alignment, 1000 );
    victim->saving_spell_staff = 0;
    victim->position    = POS_RESTING;
    victim->hit         = UMAX( 1, victim->hit  );
    victim->mana        = UMAX( 1, victim->mana );
    victim->move        = UMAX( 1, victim->move );
             victim->exp -= (victim->exp/2);

    if (victim->exp < 0 )
        victim->exp = 0;

    if ( IS_SET( sysdata.save_flags, SV_DEATH ) )
        save_char_obj( victim );

    do_char_notressurect(victim);
    char_to_room(victim,get_room_index(30601,1));
    return;
}

/*
 * Improved Death_cry contributed by Diavolo.
 * Additional improvement by Thoric (and removal of turds... sheesh!)
 */
void death_cry( CHAR_DATA *ch )
{
    ROOM_INDEX_DATA *was_in_room;
    char *msg;
    EXIT_DATA *pexit; 
    int vnum;
    int cnt=0;

    if ( !ch )
    {
      bug( "DEATH_CRY: null ch!", 0 );
      return;
    }

    msg = NULL;
    vnum = 0;
    switch ( number_bits( 4 ) )
    {
    default: msg  = "You hear $n's blood chilling last scream!"; break;
    case  0: msg  = "$n hits the ground ... DEAD."; break;
    case  1: msg  = "$n splatters blood on your armor."; break;
    case  2: if ( HAS_BODYPART(ch, PART_GUTS) )
             {
                msg  = "$n's guts spill all over the ground.";
                vnum = OBJ_VNUM_SPILLED_GUTS;
             }
             else
                msg = "$n collapses lifeless to the ground.";
             break;
    case  3: if ( HAS_BODYPART(ch, PART_HEAD) )
             {
                msg  = "$n's severed head plops on the ground.";
                vnum = OBJ_VNUM_SEVERED_HEAD;
             }
             else
                msg = "You hear $n's death cry.";
             break;
    case  4: if ( HAS_BODYPART(ch, PART_HEART) )
             {
                msg  = "$n's heart is torn from $s chest.";
                vnum = OBJ_VNUM_TORN_HEART;
             }
             else
                msg = "$n collapses lifeless to the ground.";
             break;
    case  5: if ( HAS_BODYPART(ch, PART_ARMS) )
             {
                msg  = "$n's arm is sliced from $s dead body.";
                vnum = OBJ_VNUM_SLICED_ARM;
             }
             else
                msg = "You hear $n's death cry.";
             break;
    case  6: if ( HAS_BODYPART( ch, PART_LEGS) )
             {
                msg  = "$n's leg is sliced from $s dead body.";
                vnum = OBJ_VNUM_SLICED_LEG;
             }
             else
                msg = "$n collapses lifeless to the ground.";
             break;
    }

    act( AT_CARNAGE, msg, ch, NULL, NULL, TO_ROOM );

    if ( vnum )
    {
        char buf[MAX_STRING_LENGTH];
        OBJ_DATA *obj;
        char *name;
        ZONE_DATA *zone;

        zone = find_zone(1);
        name            = IS_NPC(ch) ? ch->short_descr : ch->name;
        obj             = create_object(get_obj_index(vnum,(int)zone->number ),0,zone);
	if ( !obj )
	return;

        obj->timer      = number_range( 4, 7 );

        sprintf( buf, obj->short_descr, name );
        STRFREE( obj->short_descr );
        obj->short_descr = STRALLOC( buf );

        sprintf( buf, obj->description, name );
        STRFREE( obj->description );
        obj->description = STRALLOC( buf );

        obj = obj_to_room( obj, ch->in_room );
    }

    if ( IS_NPC(ch) )
        msg = "You hear something's blood chilling last scream!";
    else
        msg = "You hear someone's blood chilling last scream!";

    was_in_room = ch->in_room;
    cnt=0;
    for ( pexit = was_in_room->first_exit; pexit; pexit = pexit->next )
    {
	if ( cnt >= 10 )
	break;

        if ( pexit->to_room
        &&   pexit->to_room != was_in_room )
        {
    	    cnt++;
            char_from_room(ch);
            char_to_room(ch,pexit->to_room);
            act( AT_CARNAGE, msg, ch, NULL, NULL, TO_ROOM );
        }
    }

    return;
}

/*
 * The Grim Reaper! Who Else! MuHAHAHAHA! --GW
 */
void grim_reaper( CHAR_DATA *victim, bool olddeath )
{
char message[MSL];

/* Alignment Based Death Messages, vs. $n is dead.. much nicer 
 * I plan to do tons to the death system, and this is where 
 * the action will be (this file)--GW
 */

if ( !olddeath )
{
 
if ( IS_EVIL(victim) )
{
sprintf(message,
 "\n\r"
 "&BA Pulsing Bright Gateway slowly opens above $n.&W\n\r"
 "&BYou see the spirit of $n before you, giving you a&W\n\r"
 "&Blook of pure &RTerror&B as $e is dragged into the&W\n\r"
 "&BPortal, by the Minions of Hell itself...\n\r"
 "\n\r"
 "&RInside your mind, you hear the Raspy Voice of the Reaper&W\n\r"
 "&RHe says '&WThank-you&R'&W\n\r"
 "\n\r");
}
else if ( IS_GOOD(victim) || IS_NEUTRAL(victim) )
{
sprintf(message,
 "\n\r"
 "&WYou see the soul of $n slowly rise from $s body,&W\n\r"
 "&Wthe soul turn's to you, with a happy joyous look&W\n\r"
 "&Wupon it, and fills you with a feeling of warmth and&W\n\r"
 "&Whappyness at $n's passing,&W\n\r"
 "\n\r"
 "The Voice of the Gods can be Heard saying 'Welcome Home $n'\n\r"
 "\n\r");
}

 act( AT_PLAIN, message, victim, 0, 0, TO_ROOM );
}
else
act( AT_RED, "$n is DEAD!! R.I.P!",victim,NULL,NULL,TO_ROOM );

victim->died_in_room = NULL;
victim->died_in_room = victim->in_room;
return;
}

void do_char_ressurect( CHAR_DATA *ch )
{

if ( IS_IMMORTAL(ch) )
return;

if ( ch->perm_con <= 0 )
{
send_to_char("Ressurection Failed. You have no Constitution!\n\r",ch);
do_char_notressurect( ch );
return;
}

ch->perm_con--;

return;
}

void do_char_notressurect( CHAR_DATA *ch )
{
if( IS_IMMORTAL(ch) )
return;

        /* New Ressurect Stuff --GW */
        ch->pcdata->old_hp = ch->max_hit;
        ch->pcdata->old_mana = ch->max_mana;
        ch->pcdata->old_move = ch->max_move;
        ch->pcdata->old_exp = ch->exp;

/* bug fix for the 'get 30k hp' bug */
ch->max_hit = ch->pcdata->old_hp;
ch->max_mana = ch->pcdata->old_mana;

/* Single or Dual class and Level > 2 */
if ( !IS_ADVANCED(ch) && ch->level > 2 )
{
lose_level(ch,ch->level-1,1);
ch->level--;
ch->pcdata->clssdropped = 1;
}
/* Dual Classed And
 * Char First Level == 2 and char second level > 1
 */
else if ( !IS_ADVANCED(ch) && ch->level == 2 && ch->class2 > -1 && ch->level2 > 1 )
{
lose_level(ch,ch->level2-1,2);
ch->level2--;
ch->pcdata->clssdropped = 2;
}
/* Advanced and advlevel > 1 */
else if ( IS_ADVANCED(ch) && !IS_ADV_DUAL(ch) && ch->advlevel > 1 )
{
lose_level(ch,ch->advlevel-1,3);
ch->advlevel--;
ch->pcdata->clssdropped = 3;
}
/* Advanced Dual and advlevel2 > 1 */
else if ( IS_ADV_DUAL(ch) && ch->advlevel2 > 1 )
{
lose_level( ch, ch->advlevel2-1,4);
ch->advlevel2--;
ch->pcdata->clssdropped = 4;
}
else
{
send_to_char("Level loss Voided!\n\r",ch);
ch->pcdata->clssdropped = 0;
}

/* Added a Save here --GW */
save_char_obj(ch);

return;
}

void ego_update( CHAR_DATA *ch, CHAR_DATA *victim, bool gain )
{
int ego;

if ( IS_NPC( ch ) )
return;

if ( gain )
{

ego = ch->pcdata->ego;

if ( ego == 100 )
return;

/* for NPC, 1/2 of what lvl the npc is, or full level for PC kills */
if ( IS_NPC( victim ) )
ego += victim->level * 0.5;
else
ego += victim->level;

ch->pcdata->ego = ego;

/* Ego goes to 100 only. */
if ( ch->pcdata->ego > 100 )
ch->pcdata->ego = 100;

}
else
{
ego = ch->pcdata->ego;

/*for death, you lose 50 points */
ego -= 50;

if ( ego < 0 )
ego = 0;

ch->pcdata->ego = ego;
}

return;
}

char *get_ego_text( CHAR_DATA *ch )
{
int ego;

ego = ch->pcdata->ego;

if ( ego <= 10 )
return "is of teeny weenie loser type proportions";
if ( ego <= 20 )
return "is of tiny proportions";
if ( ego <= 30 )
return "is of small proportions";
if ( ego <= 40 )
return "is of humble proportions";
if ( ego <= 50 )
return "is of medium proportions";
if ( ego <= 60 )
return "is of bold proportions";
if ( ego <= 70 )
return "is of brave proportions";
if ( ego <= 80 )
return "is of GIGANTIC proportions";
if ( ego <= 90 )
return "is just plain EGOTISTICAL!";
if ( ego <= 100 )
return "is of Giganctic-Super-Google-Plex proportions";
else
return "is GODLY!";
}

/*
 * Make Chars have Ghosts! --GW
 */
void create_ghost( CHAR_DATA *ch )
{
CHAR_DATA *ghost;
char buf[MSL];
AFFECT_DATA ghost_aff;
AFFECT_DATA *paf;

/* Create the ghost */
ghost=create_mobile( get_mob_index( 67, 1 ), find_zone(1));

/* Set the text */
sprintf(buf,"ghost %s",capitalize(ch->name));
ghost->name = STRALLOC(buf);
sprintf(buf,"The Ghost of %s",capitalize(ch->name));
ghost->short_descr = STRALLOC( buf ); 
sprintf(buf,"The Ghost of %s floats here..\n\r",ch->pcdata->title);
ghost->long_descr = STRALLOC(buf);

/* Hitpoints, mana, and Move */
ghost->max_hit  = ch->max_hit;
ghost->max_mana = ch->max_mana;
ghost->max_move = ch->max_move;

/* Restore it .. */
ghost->hit =  ghost->max_hit;
ghost->mana = ghost->max_mana;
ghost->move = ghost->max_move;

/* AC */
ghost->armor = ch->armor;

/* Hitroll */
ghost->hitroll = GET_HITROLL(ch);

/* Damage Roll */
ghost->damroll = GET_DAMROLL(ch);

/* Affects */
xSET_BITS( ghost->affected_by, ch->affected_by );

for( paf = ch->first_affect; paf; paf = paf->next )
{
ghost_aff.type = -1;
ghost_aff.duration = paf->duration;
ghost_aff.location = paf->location;
ghost_aff.modifier = paf->modifier;
xSET_BITS(ghost_aff.bitvector,paf->bitvector);
}

/* Level */
ghost->level = 100;

/* Numattacks */
ghost->numattacks = 5;

/* Make me a ghost! */
xSET_BIT(ghost->affected_by, AFF_GHOST);

/* Send it to the room */
char_to_room(ghost,ch->died_in_room);
act(AT_MAGIC,"As $N dies, a ghost slowly rises from the corpse...",ghost,NULL,ch,TO_ROOM);
return;
}

void ctf_death( CHAR_DATA *killer, CHAR_DATA *victim )
{
char buf[MSL];
extern int CTF_BLUE_FRAGS;
extern int CTF_RED_FRAGS;
extern int CTF_RED_CAPS;
extern int CTF_BLUE_CAPS;
OBJ_DATA *obj, *next_obj=NULL;

send_to_char("&RYou are Dead, Sorry....&W\n\r",victim);
stop_fighting(victim,TRUE);
char_from_room(victim);

sprintf(buf,"%s's Slaughtered by %s!",QUICKLINK(victim->name),QUICKLINK(killer->name));
ctf_channel(buf);

/* Score */
if ( (IS_NPC(killer) && victim->pcdata->team==TEAM_RED) || (!IS_NPC(killer) && killer->pcdata->team == TEAM_RED) )
CTF_RED_FRAGS++;
if ( (IS_NPC(killer) && victim->pcdata->team==TEAM_BLUE) || (!IS_NPC(killer) && killer->pcdata->team == TEAM_BLUE) )
CTF_BLUE_FRAGS++;

/* Show Score */
sprintf(buf,"Score: &RRed:&W %d Frags, %d Captures.",CTF_RED_FRAGS,CTF_RED_CAPS);
ctf_channel(buf);
sprintf(buf,"Score: &BBlue:&W %d Frags, %d Captures.",CTF_BLUE_FRAGS,CTF_BLUE_CAPS);
ctf_channel(buf);

if ( victim->pcdata->team == TEAM_RED )
 char_to_room(victim,get_room_index(CTF_RED_BASE,1));
else
 char_to_room(victim,get_room_index(CTF_BLUE_BASE,1));

victim->hit = victim->max_hit;
victim->mana = victim->max_mana;
victim->move = victim->max_move;
update_pos(victim);
do_look(victim,"auto");
send_to_char("You have been Restored. Back to the Fray!!\n\r",victim);

/* ctf flags stuff */
      /* Red Flag Carrier */
      if ( victim->pcdata->team == RED_FLAG_CARRIER )
        flag_control( victim, TEAM_RED, 3 );
   
      /* Blue Flag Carrier */
      if ( victim->pcdata->team == BLUE_FLAG_CARRIER )
        flag_control( victim, TEAM_BLUE, 3 );

for ( obj=victim->first_carrying; obj; obj = next_obj )
{
  next_obj=obj->next_content;
  if ( victim->pcdata->team == RED_FLAG_CARRIER && obj->pIndexData->vnum == CTF_RED_FLAG )
  {
    obj_from_char(obj);
    obj_to_room(obj,get_room_index(CTF_RED_BASE,1));
    break;
  }
  if ( victim->pcdata->team == BLUE_FLAG_CARRIER && obj->pIndexData->vnum == CTF_BLUE_FLAG )
  {
    obj_from_char(obj);
    obj_to_room(obj,get_room_index(CTF_BLUE_BASE,1));
    break;
  }
}
  
return;
}

