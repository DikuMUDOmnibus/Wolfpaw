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
 *			Arena Handling Functions Module			    *
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

ARENA_DATA *find_arena args( ( CHALLENGE_DATA *challenge ) );
CHALLENGE_DATA *find_challenge args( ( CHAR_DATA *ch ) );
ARENA_DATA *find_arena_by_challenge args( ( CHALLENGE_DATA *challenge ) );
void challenge_cancel args( ( CHAR_DATA *ch, CHAR_DATA *victim, char *argument ) );
CHAR_DATA *get_char args( ( char *name ) );
void pk_champ_check( CHAR_DATA *ch, CHAR_DATA *victim );
void give_pk_pin( CHAR_DATA *old_champ, CHAR_DATA *new_champ );
void do_bet2(CHAR_DATA *ch, char *argument );

/*=======================================================================*
 * function: do_challenge                                                *
 * purpose: sends initial arena match query                              *
 * written by: Doug Araya (whiplash@tft.nacs.net) 6-10-96                *
 * ReVamped by Greywolf, May 1998					 *
 *=======================================================================*/

void do_challenge(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim; 
  char buf[MAX_STRING_LENGTH];
  char arg[MIL];
  CHALLENGE_DATA *challenge=NULL;

  record_call( "<do_challenge>" ); 

  one_argument( argument, arg );

 if(IS_NPC(ch))
 return; 

if ( in_arena(ch) )
{
  send_to_char("Your Already in the Arena!!\n\r",ch);
  return;
}

if ( ch->fighting )
{
  send_to_char("While Fighting!?! Noooo..!!!\n\r",ch);
  return;
}

if ( arg[0] == '\0' ) /* Status */
{

if ( ( challenge = find_challenge(ch) )==NULL )
{
send_to_char("But you have not been challenged, or challenged anyone!\n\r",ch);
return;
}

if ( IS_SET( ch->pcdata->flags, PCFLAG_CHALLENGER ) )
{
send_to_char("Your Challenge Status:\n\r",ch);
ch_printf( ch, "You have challenged: %s\n\r", capitalize(challenge->challenged) );
ch_printf( ch, "The Challenge is Currently %s.",
	challenge->fight_in_progress == FALSE ? "Waiting for Acceptance" :
	"in Progress" );
return;
}

if ( IS_SET( ch->pcdata->flags, PCFLAG_CHALLENGED ) )
{
send_to_char("Your Challenge Status:\n\r",ch);
ch_printf( ch, "You have been challenged by: %s\n\r", capitalize(challenge->challenger ) );
ch_printf( ch, "The Challenge is Currently %s.",
	challenge->fight_in_progress == FALSE ? "Waiting for Acceptance" :
	"in Progress" );
return;
}
}
/* The routine above handles this -- Cal
if ( IS_SET( ch->pcdata->flags, PCFLAG_CHALLENGER ) )
{
send_to_char("You have already challenged someone else!\n\r",ch);
return;
}

if ( IS_SET( ch->pcdata->flags, PCFLAG_CHALLENGED ) )
{
send_to_char("You have already been challenged!\n\r",ch);
return;
} */

if ( !str_cmp( arg, "accept" ) ) /* Accept the Challenge */
{
 char pk_buf[MAX_STRING_LENGTH]; 
 ARENA_DATA *arena;

if ( ( challenge = find_challenge(ch) )==NULL )
{
send_to_char("But you have not been challenged, or challenged anyone!\n\r",ch);
return;
}

if ( ( arena = find_arena(challenge) )==NULL)
{
send_to_char("All arena's are Full, please wait a few moments.\n\r",ch);
return;
}


 if((victim = get_char_world(ch,challenge->challenger)) == NULL)
 {
    send_to_char("Your Opponent is not logged in!\n\r",ch);
    return;
 }

 if ( !IS_SET( ch->pcdata->flags, PCFLAG_CHALLENGED ) )
 {
    send_to_char("But you have not been challenged!\n\r",ch);
    return;
 }

send_to_char("You have accepted the challenge!\n\r",ch);
act(AT_BLOOD,"$n accepts your challenge!",ch,NULL,victim,TO_VICT);
challenge->fight_in_progress = TRUE;
arena->in_use = TRUE;

/* == announce the upcoming event == */
sprintf(buf,"&r[CHALLENGE]&W %s has accepted %s's challenge!\n\r",ch->name, victim->name);
echo_to_all( AT_BLOOD,buf,ECHOTAR_ALL ); 

/* == now move them both to an arena for the fun == */
act( AT_BLOOD,"$n dissapears suddenly, eyes full of blood lust!",ch,NULL,NULL,TO_ROOM);
send_to_char("You make your way into the arena.\n\r",ch);
char_from_room(ch); 
char_to_room(ch,get_room_index(arena->char_room,1));
do_look( ch,"auto");

act( AT_BLOOD,"$n dissapears suddenly, eyes full of blood lust!",victim,NULL,NULL,TO_ROOM);
send_to_char("You make your way to the arena.\n\r",victim); 
char_from_room(victim); 
char_to_room(victim,get_room_index(arena->vict_room,1)); 
do_look( victim,"auto");

/* == announce the bets == */
echo_to_all( AT_PLAIN,"&B[GAMBLE]&W To wager on the fight, type bet <amount> <playername> \n\r",ECHOTAR_ALL );

/* Viewing --GW */
echo_to_all( AT_PLAIN,"&B[&GVIEW IT!&B]&W To View the Battle, type view <char name>\n\r",ECHOTAR_ALL);


if( IS_PKILL(ch) && IS_PKILL(victim) )
{
if ( IS_SET( victim->pcdata->flags, PCFLAG_PKCHAMP) )
{
sprintf(pk_buf,"&BYou hear the voice of &r%s &BScream...",capitalize(victim->name)); 
echo_to_all(AT_PLAIN,pk_buf,ECHOTAR_ALL);
echo_to_all(AT_PLAIN,"\n&RTHERE CAN BE ONLY ONE!!!&W\n",ECHOTAR_ALL);
}
if ( IS_SET( ch->pcdata->flags, PCFLAG_PKCHAMP) )
{
sprintf(pk_buf,"&BYou hear the voice of &r%s &BScream...",capitalize(ch->name));
echo_to_all(AT_PLAIN,pk_buf,ECHOTAR_ALL);
echo_to_all(AT_PLAIN,"\n&RTHERE CAN BE ONLY ONE!!!&W\n",ECHOTAR_ALL);
}
}
return; 
}
else if ( !str_cmp( arg, "decline" ) ) /* Decline the Challenge */
{

if ( ( challenge = find_challenge(ch) )==NULL )
{
send_to_char("But you have not been challenged, or challenged anyone!\n\r",ch);
return;
}

 if((victim = get_char_world(ch,challenge->challenger)) == NULL)
 {
    send_to_char("Your opponent logged in!\n\r",ch);
    return;
 }


 if(victim == ch && IS_SET(ch->pcdata->flags, PCFLAG_CHALLENGER))
 {
    REMOVE_BIT(victim->pcdata->flags, PCFLAG_CHALLENGER);
    send_to_char("Challenge removed!\n\r",ch);
    sprintf(buf,"&R[CHALLENGE]&W %s has STEPED DOWN from the challenge! WHAT A WUSS!!\n\r",victim->name);
    if ( ( victim = get_char_world(ch,challenge->challenged))!=NULL && !IS_NPC(victim) )
    {
       ch_printf(victim,"%s has STEPPED DOWN from the challenge!\n\r",capitalize(ch->name) );
       REMOVE_BIT(victim->pcdata->flags, PCFLAG_CHALLENGED);
    }
    echo_to_all( AT_BLOOD,buf,ECHOTAR_ALL );
    UNLINK(challenge, first_challenge, last_challenge, next, prev );
    DISPOSE(challenge);
    return;
 }

if (victim->desc && IS_SET(victim->pcdata->flags, PCFLAG_CHALLENGER) )
{
    REMOVE_BIT(victim->pcdata->flags, PCFLAG_CHALLENGER);
    REMOVE_BIT(ch->pcdata->flags, PCFLAG_CHALLENGED); 
    send_to_char("Challenge declined!\n\r",ch);
    act(AT_BLOOD,"$n has declined your challenge.",ch,NULL,victim,TO_VICT);
    sprintf(buf,"&R[CHALLENGE]&W %s has DECLINED %s's challenge! WHAT A WUSS!!\n\r",ch->name, victim->name);
    echo_to_all( AT_BLOOD,buf,ECHOTAR_ALL );
    UNLINK(challenge, first_challenge, last_challenge, next, prev );
    DISPOSE(challenge);
    return;
}

return;
}
else /* New Challenge */
 {

 if(IS_SET(ch->pcdata->flags, PCFLAG_CHALLENGED))
 {
    send_to_char("You have already been challenged, either ACCEPT or DECLINE first.\n\r",ch);
    return; 
 }

 if(IS_SET(ch->pcdata->flags, PCFLAG_CHALLENGER))
 {
    send_to_char("You have already challenged someone!\n\r", ch );
    return;
 }

 if (IS_SET(ch->pcdata->flagstwo, MOREPC_HOARDER) )
 {
    send_to_char("Get out of the Hoarder's Void First!\n\r",ch);
    return;
 }

 if ( ch->in_room->vnum == 8 )
 {
   send_to_char("But .... But ... YOUR IN HELL! Hello?\n\r",ch);
   return;
 }

 if((victim = get_char_world(ch,arg)) == NULL)
 {
    send_to_char("They are not playing.\n\r",ch);
    return;
 }

 if ( IS_NPC(victim) )
 {
    send_to_char("Not on NPC's!\n\r",ch);
    return;
 }

 if(IS_SET(victim->pcdata->flags, PCFLAG_CHALLENGED))
 {
    send_to_char("That person has already been challenged.\n\r",ch);
    return; 
 }

 if(IS_SET(victim->pcdata->flags, PCFLAG_CHALLENGER))
 {
    send_to_char("That person has already challenged someone!\n\r", ch );
    return;
 }

 if (ch->in_room->sector_type == 15 )
 {
    send_to_char("You cannot challenge well in a rented room\n\r", ch);
    return;
 }

 if (victim->in_room->sector_type == 15 )
 {
    send_to_char("Sorry they are in a rented room at the moment\n\r", ch);
    return;
 }

 if(IS_NPC(victim) || victim == ch )
 {
    send_to_char("You cannot challenge NPC's, or yourself.\n\r",ch);
    return;
 }

 if (IS_SET(victim->pcdata->flagstwo, MOREPC_HOARDER) )
 {
    send_to_char("You cant challenge someone who is in the Hoarder's Void!\n\r",ch);
    return;
 }

 if ( victim->in_room->vnum == 8 )
 {
    send_to_char("That character pissed someone off, and is in Hell. No can do!\n\r",ch);
    return;
 }

 if(IS_SET(victim->pcdata->flags, PCFLAG_CHALLENGER))
 {
    send_to_char("They have already challenged someone else.\n\r",ch);
    return;
 }

 if(victim->fighting != NULL )
 {
    send_to_char("That person is engaged in battle right now.\n\r",ch); 
    return; 
 }

 if( IS_IMMORTAL(ch) && IS_IMMORTAL(victim) )
 {
    send_to_char("God vs God? yeah ok right... NOT\n\r", ch);
    return;
 }

/* == Now for the challenge == */
 CREATE( challenge, CHALLENGE_DATA, 1 );
 SET_BIT(ch->pcdata->flags, PCFLAG_CHALLENGER);
 SET_BIT(victim->pcdata->flags, PCFLAG_CHALLENGED);
 send_to_char("Challenge ISSUED!\n\r",ch);

 act(AT_BLOOD,"$n has challenged you to a death match.",ch,NULL,victim,TO_VICT);

 send_to_char("type: CHALLENGE ACCEPT to meet the challenge.\n\r",victim);
 send_to_char("OR:\n\r",victim);
 send_to_char("type: CHALLENGE DECLINE to chicken out.\n\r",victim); 

 sprintf(buf,"0.%s",ch->name);
 challenge->challenger = STRALLOC(buf);
 sprintf(buf,"0.%s",victim->name);
 challenge->challenged = STRALLOC(buf);
 challenge->fight_in_progress = FALSE;
 challenge->arena_number = 1;
 LINK( challenge, first_challenge, last_challenge, next, prev );

/* == announce the upcoming event == */
sprintf(buf,"&r[CHALLENGE]&W %s has issued a challenge to %s!\n\r",
	ch->name, victim->name);
echo_to_all( AT_PLAIN,buf,ECHOTAR_ALL );

return;
}
}

/*======================================================================*
 * function: do_bet                                                     *
 * purpose: to allow players to wager on the outcome of arena battles   *
 * written by: Doug Araya (whiplash@tft.nacs.net) 6-10-96               *
 *======================================================================*/
void do_bet(CHAR_DATA *ch, char *argument)
 {
 char arg[MAX_INPUT_LENGTH]; 
 char buf[MAX_STRING_LENGTH]; 
 CHAR_DATA *fighter; 
 int wager; 
 record_call( "<do_bet>" );

 if ( ch->in_room->vnum == BLACKJACK )
 {
   do_bet2(ch,argument);
   return;
 }

 argument = one_argument(argument, arg); 

 if(argument[0] == '\0' || !is_number(arg))
  {
send_to_char("Syntax: BET [amount] [player]\n\r",ch); 
return;
  }

/*== disable the actual fighters from betting ==*/
 if(IS_SET(ch->pcdata->flags, PCFLAG_CHALLENGER) ||
IS_SET(ch->pcdata->flags, PCFLAG_CHALLENGED))
  {
send_to_char("You can't bet on this battle.\n\r",ch); 
return; 
  }

/*== make sure the choice is valid ==*/
 if((fighter = get_char_world(ch,argument)) == NULL)
  {
send_to_char("That player is not in the arena.\n\r",ch); 
return; 
  }

/*== do away with the negative number trickery ==*/
 if(!str_prefix("-",arg))
  {
send_to_char("Error: Invalid argument!\n\r",ch); 
return; 
  }

 wager   = atoi(arg);

 if(wager > 5001 || wager < 1)
  {
send_to_char("Wager range is between 1 and 5000\n\r",ch);
return; 
  }

/*== make sure they have the cash ==*/
 if(wager > ch->gold)
  {
send_to_char("You don't have that much gold to wager!\n\r",ch); 
return; 
  }

/*== now set the info ==*/
ch->pcdata->gladiator = fighter->name;
ch->pcdata->plr_wager = wager;
sprintf(buf,"You have placed a %d gold wager on %s\n\r",wager,fighter->name);
send_to_char(buf,ch);
return; 
}

/*
 * Find the right Challenge Array for the char --GW
 */
CHALLENGE_DATA *find_challenge( CHAR_DATA *ch )
{
CHALLENGE_DATA *temp, *temp_next;
char buf[MSL];

for ( temp = first_challenge; temp; temp = temp_next )
{
temp_next = temp->next;

sprintf(buf,"0.%s",ch->name);
if ( !str_cmp( buf, temp->challenger ) )
return temp;

if ( !str_cmp( buf, temp->challenged ) )
return temp;
}

return NULL;
}

/*
 * Find an Open Arena --GW
 */
ARENA_DATA *find_arena( CHALLENGE_DATA *challenge )
{
ARENA_DATA *temp, *temp_next;

for( temp = first_arena; temp; temp = temp_next )
{

temp_next = temp->next;

if ( !challenge )
{
bug( "Null challenge!",0);
return NULL;
}

if ( temp->in_use == FALSE )
{
challenge->arena_number = temp->arena_number;
return temp;
}

}

return NULL;
}

ARENA_DATA *find_arena_by_challenge( CHALLENGE_DATA *challenge )
{
ARENA_DATA *temp, *next_temp;

for ( temp = first_arena; temp; temp = next_temp )
{
next_temp = temp->next;

if ( !challenge )
return NULL;

if ( temp->arena_number == challenge->arena_number )
return temp;
} 

return NULL;
}

/*
 * Terminate a Challenge --GW 
 */
void challenge_cancel( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
char arg[MSL];
CHAR_DATA *challenger;
CHAR_DATA *challenged;
CHALLENGE_DATA *temp;
CHALLENGE_DATA *temp_next;

one_argument( argument, arg );

/* All First */
if ( !str_cmp( arg, "all" ) )
{
  for ( temp = first_challenge; temp; temp = temp_next )
  {
	temp_next = temp->next;
	challenger = get_char_world(ch, temp->challenger );
	challenged = get_char_world(ch, temp->challenged );
	ch = challenger;
        victim = challenged;

	if ( challenger && challenged && in_arena( challenger ) )
	{
	send_to_char("\aYour Challenge has been Canceled.\n\r",challenger);
	send_to_char("\aYour Challenge has been Canceled.\n\r",challenged);
	ch->hit = ch->max_hit;
        victim->hit = victim->max_hit;
        update_pos(ch);
        update_pos(victim);
	stop_fighting( challenger, TRUE );
	char_from_room( challenger );
	char_from_room( challenged );
        char_to_room( challenger, get_room_index( 30601,1 ) );
        char_to_room( challenged, get_room_index( 30602,1 ) );
	REMOVE_BIT( challenger->pcdata->flags, PCFLAG_CHALLENGER ); 
	REMOVE_BIT( challenged->pcdata->flags, PCFLAG_CHALLENGED ); 
	UNLINK(temp, first_challenge, last_challenge, next, prev );
	DISPOSE(temp);
	return;
        }
	else
	{
	if ( challenger && challenged )
	{
	send_to_char("\aYour Challenge has been Canceled.\n\r",challenger);
	send_to_char("\aYour Challenge has been Canceled.\n\r",challenged);
	REMOVE_BIT( challenger->pcdata->flags, PCFLAG_CHALLENGER ); 
	REMOVE_BIT( challenged->pcdata->flags, PCFLAG_CHALLENGED ); 
	UNLINK(temp, first_challenge, last_challenge, next, prev );
	DISPOSE(temp);
	}
	}
}

}
return;
}

/*
 * Get a char from the game, regardless of anything, just do it --GW
 */
CHAR_DATA *get_char( char *name )
{
CHAR_DATA *ch1;
char buf[MSL];
char buf2[MSL];

for( ch1 = first_char; ch1; ch1 = ch1->next )
{

if ( name[0] != '0' && name[1] != '.' )
sprintf(buf,"0.%s",name);
else
sprintf(buf,"%s",name);

sprintf(buf2,"0.%s",ch1->name);

if ( !str_cmp( buf, buf2 ) )
return ch1;
}

return NULL;
}

/*
 * Pass the Pin! --GW
 * ch is always Winner, Vict Loser
 */
void pk_champ_check( CHAR_DATA *ch, CHAR_DATA *victim )
{
char buf[MSL];

if ( IS_SET( ch->pcdata->flagstwo, MOREPC_ETERNAL_PK ) )
return;

/* We beat the Champion! -- Lets Championize! */
if ( IS_SET( victim->pcdata->flags, PCFLAG_PKCHAMP ) )
{
echo_to_all(AT_PLAIN,"&RThe Realms Rise up in PRAISE!",ECHOTAR_ALL);
echo_to_all(AT_PLAIN,"&WFrom this Match rises a &RNew Champion!&W",ECHOTAR_ALL);
sprintf(buf,"&R%s&W is  the New &RPlayer Kill Champion&W!",ch->name);
echo_to_all(AT_PLAIN,buf,ECHOTAR_ALL);
REMOVE_BIT(victim->pcdata->flags, PCFLAG_PKCHAMP);
SET_BIT(ch->pcdata->flags, PCFLAG_PKCHAMP);
give_pk_pin(victim,ch);
ch->pcdata->pkchamps++;
sysdata.pkchamp = str_dup(ch->name);
save_sysdata(sysdata);
return;
}

return;
}

void give_pk_pin( CHAR_DATA *old_champ, CHAR_DATA *new_champ )
{
OBJ_DATA *obj;

        if ( ( obj = get_obj_wear( old_champ, "pkchamppin" ) ) == NULL )
        {
	bug("Give_Pk_Pin: Champ not wearing a Pin!",0);
        }

unequip_char( old_champ, obj );
obj_from_char( obj );
obj_to_char( obj, new_champ );
equip_char( new_champ, obj, WEAR_CHAMP );
return;
}

/*
 * Allow people to view a match in progress as if they were in the room --GW
 */
void do_view( CHAR_DATA *ch, char *argument )
{
char arg1[MSL];
CHAR_DATA *victim;
int gallery_vnum=41;
ROOM_INDEX_DATA *gallery;

one_argument( argument, arg1 );

if ( IS_NPC(ch) )
return;

if ( IS_SET(ch->pcdata->flagstwo, MOREPC_HOARDER) )
{
  send_to_char("I don't think so, Tim.\n\r",ch);
  return;
}


if ( arg1[0] == '\0' )
{
   send_to_char("Syntax: view <char name>/<all>\n\r",ch);
   return;
}

if ( ( victim=get_char_world( ch, arg1 ) )==NULL)
{
  send_to_char("You can't seem to find that person\n\r",ch);
  return;
}

if ( !in_arena( victim ) )
{
  send_to_char("That person is not in the Arena!\n\r",ch);
  return;
}

if ( ch->fighting && ch->fighting->who )
{
  send_to_char("Not while you are fighting! You better watch this match closer!\n\r",ch);
  return;
}

if ( ch->in_room && ch->in_room->vnum == 8 )
{
send_to_char("Ya right.\n\r",ch);
return;
}

if ( IS_SET(ch->pcdata->flagstwo, MOREPC_ARRESTED ) )
{
send_to_char("Uh .. no.\n\r",ch);
return;
}

if ( IS_SET( ch->pcdata->flags, PCFLAG_CHALLENGED ) ||
     IS_SET( ch->pcdata->flags, PCFLAG_CHALLENGER ) )
{
send_to_char("And just how do you intend to pull that off?\n\r",ch);
return;
}

if ( str_cmp(ch->pcdata->arena_viewing, "None" ) )
{
send_to_char("Sorry, you are already Viewing a Match.\n\r",ch);
return;
}

if ( IS_SET( ch->pcdata->flags, PCFLAG_KOMBAT ) )
{
send_to_char("Um....... No.\n\r",ch);
return;
}

if ( ( gallery=get_room_index( gallery_vnum, 1 ) )==NULL )
{
send_to_char("Can't seem to find the Viewing Gallery.. talk to a god..\n\r",ch);
return;
}

char_from_room(ch);
char_to_room(ch,gallery);
ch->pcdata->arena_viewing = STRALLOC( victim->name );
send_to_char("You start viewing the match.\n\r",ch);
return;
}

void stop_viewers( void )
{
CHAR_DATA *ch, *ch_next;
ROOM_INDEX_DATA *recall;

recall=get_room_index( 30601, 1 );

for( ch=first_char; ch; ch = ch_next )
{
ch_next = ch->next;

if ( str_cmp( ch->pcdata->arena_viewing,"none" )
&& ch->in_room && ch->in_room->vnum == 41 )
{
send_to_char("The Match is over, you make your way back to Mithras.\n\r",ch);
ch->pcdata->arena_viewing = STRALLOC( "None" );
char_from_room( ch );
char_to_room( ch, recall );
do_look( ch, "auto" ); 
}

}

return;
}

/* 
 * Write the Fight stuff to the Gallery --GW
 */
void write_to_gallery( CHAR_DATA *fighter, char *message )
{
DESCRIPTOR_DATA *d, *d_next;
CHAR_DATA *ch;

if ( !message )
return;

for ( d=first_descriptor; d; d = d_next )
{
d_next = d->next;

if ( !d )
continue;

ch = d->character;

if ( !ch )
continue;

if( d->connected == CON_PLAYING && ch->pcdata->arena_viewing )
{
if ( !str_cmp(ch->pcdata->arena_viewing, fighter->name ) )
send_to_char(message,ch);
}

}

return;
}

