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
 *			   Player communication module			    *
 ****************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include "mud.h"
/*
 *  Externals
 */
void send_obj_page_to_char(CHAR_DATA * ch, OBJ_INDEX_DATA * idx, char page);
void send_room_page_to_char(CHAR_DATA * ch, ROOM_INDEX_DATA * idx, char page);
void send_page_to_char(CHAR_DATA * ch, MOB_INDEX_DATA * idx, char page);
void send_control_page_to_char(CHAR_DATA * ch, char page);
CHAR_DATA *get_char_list  args( ( CHAR_DATA *ch ) );
ARENA_DATA *find_arena args( ( CHALLENGE_DATA *challenge ) );
CHALLENGE_DATA *find_challenge args( ( CHAR_DATA *ch ) );
CMAP_DATA *find_char_map_by_type(CHAR_DATA *ch, int map_type);
int GET_MAX_LEVEL( CHAR_DATA *ch);
char *GET_MAX_CLASS( CHAR_DATA *ch);
void pit_monster( OBJ_DATA *pit, CHAR_DATA *ch );
bool write_to_desc_color( char *txt, DESCRIPTOR_DATA *d );

/* #include <stdio.h> */

/*
 * Local functions.
 */
void	talk_channel	args( ( CHAR_DATA *ch, char *argument,
			    int channel, char *verb ) );
void	remove_char	args( ( CHAR_DATA *ch ) );

char *  scramble        args( ( const char *argument, int modifier ) );			    
char *  drunk_speech    args( ( const char *argument, CHAR_DATA *ch ) ); 
CHAR_DATA *get_char	args( ( char *name ) );
char *smash_curses( const char *ip );

/* Text scrambler -- Altrag */
char *scramble( const char *argument, int modifier )
{
    static char arg[MAX_INPUT_LENGTH];
    sh_int position;
    sh_int conversion = 0;
    record_call("<scramble>");    
	modifier %= number_range( 80, 300 ); /* Bitvectors get way too large #s */
    for ( position = 0; position < MAX_INPUT_LENGTH; position++ )
    {
    	if ( argument[position] == '\0' )
    	{
    		arg[position] = '\0';
    		return arg;
    	}
    	else if ( argument[position] >= 'A' && argument[position] <= 'Z' )
	    {
	    	conversion = -conversion + position - modifier + argument[position] - 'A';
	    	conversion = number_range( conversion - 5, conversion + 5 );
	    	while ( conversion > 25 )
	    		conversion -= 26;
	    	while ( conversion < 0 )
	    		conversion += 26;
	    	arg[position] = conversion + 'A';
	    }
	    else if ( argument[position] >= 'a' && argument[position] <= 'z' )
	    {
	    	conversion = -conversion + position - modifier + argument[position] - 'a';
	    	conversion = number_range( conversion - 5, conversion + 5 );
	    	while ( conversion > 25 )
	    		conversion -= 26;
	    	while ( conversion < 0 )
	    		conversion += 26;
	    	arg[position] = conversion + 'a';
	    }
	    else if ( argument[position] >= '0' && argument[position] <= '9' )
	    {
	    	conversion = -conversion + position - modifier + argument[position] - '0';
	    	conversion = number_range( conversion - 2, conversion + 2 );
	    	while ( conversion > 9 )
	    		conversion -= 10;
	    	while ( conversion < 0 )
	    		conversion += 10;
	    	arg[position] = conversion + '0';
	    }
	    else
	    	arg[position] = argument[position];
	}
	arg[position] = '\0';
	return arg;	     
}

/* I'll rewrite this later if its still needed.. -- Altrag */
char *translate( CHAR_DATA *ch, CHAR_DATA *victim, const char *argument )
{
	return "";
}

char *drunk_speech( const char *argument, CHAR_DATA *ch )
{
  const char *arg = argument;
  static char buf[MAX_INPUT_LENGTH*2];
  char buf1[MAX_INPUT_LENGTH*2];
  sh_int drunk;
  char *txt;
  char *txt1;  
  record_call("<drunk_speech>");    

  if( !ch )
    return (char *) argument;

  if ( IS_NPC( ch ) || !ch->pcdata ) return (char *) argument;

  drunk = ch->pcdata->condition[COND_DRUNK];

  if ( drunk <= 0 )
    return (char *) argument;

  buf[0] = '\0';
  buf1[0] = '\0';

  if ( !argument )
  {
     bug( "Drunk_speech: NULL argument", 0 );
     return "";
  }

  txt = buf;
  txt1 = buf1;

  while ( *arg != '\0' )
  {
    if ( toupper(*arg) == 'S' )
    {
	if ( number_percent() < ( drunk * 2 ) )		/* add 'h' after an 's' */
	{
	   *txt++ = *arg;
	   *txt++ = 'h';
	}
       else
	*txt++ = *arg;
    }
   else if ( toupper(*arg) == 'X' )
    {
	if ( number_percent() < ( drunk * 2 / 2 ) )
	{
	  *txt++ = 'c', *txt++ = 's', *txt++ = 'h';
	}
       else
	*txt++ = *arg;
    }
   else if ( number_percent() < ( drunk * 2 / 5 ) )  /* slurred letters */
    {
      sh_int slurn = number_range( 1, 2 );
      sh_int currslur = 0;	

      while ( currslur < slurn )
	*txt++ = *arg, currslur++;
    }
   else
    *txt++ = *arg;

    arg++;
  };

  *txt = '\0';

  txt = buf;

  while ( *txt != '\0' )   /* Let's mess with the string's caps */
  {
    if ( number_percent() < ( 2 * drunk / 2.5 ) )
    {
      if ( isupper(*txt) )
        *txt1 = tolower( *txt );
      else
      if ( islower(*txt) )
        *txt1 = toupper( *txt );
      else
        *txt1 = *txt;
    }
    else
      *txt1 = *txt;

    txt1++, txt++;
  };

  *txt1 = '\0';
  txt1 = buf1;
  txt = buf;

  while ( *txt1 != '\0' )   /* Let's make them stutter */
  {
    if ( *txt1 == ' ' )  /* If there's a space, then there's gotta be a */
    {			 /* along there somewhere soon */

      while ( *txt1 == ' ' )  /* Don't stutter on spaces */
        *txt++ = *txt1++;

      if ( ( number_percent() < ( 2 * drunk / 4 ) ) && *txt1 != '\0' )
      {
	sh_int offset = number_range( 0, 2 );
	sh_int pos = 0;

	while ( *txt1 != '\0' && pos < offset )
	  *txt++ = *txt1++, pos++;

	if ( *txt1 == ' ' )  /* Make sure not to stutter a space after */
	{		     /* the initial offset into the word */
	  *txt++ = *txt1++;
	  continue;
	}

	pos = 0;
	offset = number_range( 2, 4 );	
	while (	*txt1 != '\0' && pos < offset )
	{
	  *txt++ = *txt1;
	  pos++;
	  if ( *txt1 == ' ' || pos == offset )  /* Make sure we don't stick */ 
	  {		               /* A hyphen right before a space	*/
	    txt1--;
	    break;
	  }
	  *txt++ = '-';
	}
	if ( *txt1 != '\0' )
	  txt1++;
      }     
    }
   else
    *txt++ = *txt1++;
  }

  *txt = '\0';

  return buf;
}
void do_newbie( CHAR_DATA *ch, char *argument)
{
CHAR_DATA *victim;
char    buf[MAX_STRING_LENGTH];
char    arg[MAX_INPUT_LENGTH];
int     rannum;
record_call("<do_newbie>");    

victim = ch;

one_argument( argument, arg );

if ( arg[0] == '\0' )
{
send_to_char("Syntax:  Newhelp <on/off>\n\r",victim);
return;
}

if ( !str_cmp( arg, "on" ))
{
send_to_char("Newbie Help channel ON.\n\r",ch);
SET_BIT(victim->pcdata->flags, PCFLAG_NEWBIEHELP);
return;
}

if ( !str_cmp( arg, "off" ))
{
send_to_char("Newbie Help channel OFF.\n\r",ch);
REMOVE_BIT(victim->pcdata->flags, PCFLAG_NEWBIEHELP);
return;
}

if ( !str_cmp( arg, "show" ))
{
rannum = dice( 1, 12 );

if ( rannum == 1 )
{
sprintf(buf,"[NewbieHelp] Type WEAR ALL to wear all eq before you fight!\n\r");
send_to_char(buf,ch);
return;
}
if ( rannum == 2 )
{
sprintf(buf,"[NewbieHelp] Read HELP LEVEL, you must train levels here!\n\r");
send_to_char(buf,ch);
return;
}

if ( rannum == 3 )
{
sprintf(buf,"[NewbieHelp] To Train and Practice type RECALL then Up, North!");
send_to_char(buf,ch);
return;
}

if ( rannum == 4 )
{
sprintf(buf,"[NewbieHelp] Died?, Ouch, type RECALL then west, your corpse will be there!\n\r");
send_to_char(buf,ch);
return;
}

if ( rannum == 5 )
{
sprintf(buf,"[NewbieHelp] Lost all EQ? Type REFIT for new stuff!\n\r");
send_to_char(buf,ch);
return;
}

if ( rannum == 6 )
{
sprintf(buf,"[NewbieHelp] Want to kill someone, HELP CHALLENGE!\n\r");
send_to_char(buf,ch);
return;
}

if ( rannum == 8 || rannum == 7 )
{
sprintf(buf,"[NewbieHelp] How much exp to level? Type LEVEL\n\r");
send_to_char(buf,ch);
return;
}

if ( rannum == 9 )
{
sprintf(buf,"You suddenly feel the need to gain experience!\n\r");
send_to_char(buf,ch);
return;
}

if ( rannum == 10 )
{
sprintf(buf,"[NewbieHelp] Man I Need to HEAL!!! HELP CHECKIN!\n\r");
send_to_char(buf,ch);
return;
}

if ( rannum == 11 )
{
sprintf(buf,"[NewbieHelp] Turn this annoying thing off! Type NEWBIE OFF\n\r");
send_to_char(buf,ch);
return;
}
if ( rannum == 12 )
{
sprintf(buf,"[NewbieHelp] Try the NEWBIE GUIDE, go RECALL, up!\n\r");
send_to_char(buf,ch);
return;
}
}
send_to_char("Syntax: Newbie <on/off>",ch);
return;
}


void do_bonus( CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    char       buf  [ MAX_STRING_LENGTH ];
    char       arg1 [ MAX_INPUT_LENGTH ];
    char       arg2 [ MAX_INPUT_LENGTH ];
    int      value;
    record_call("<do_bonus>");    

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: bonus <char> <Exp>.\n\r", ch );
	return;
    }
    
    if (( victim = get_char_world ( ch, arg1 ) ) == NULL )
    {
      send_to_char("That Player is not here.\n\r", ch);
      return;
    }
       
    if ( IS_NPC( victim ) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "You may not bonus yourself.\n\r", ch );
	return;
    }

    if (IS_IMMORTAL(victim) || victim->level >= LEVEL_IMMORTAL)
    {
    send_to_char("You can't bonus Immortals.\n\r", ch);
    return;
    }

    value = atoi( arg2 );

   
    sprintf( buf,"You have bonused %s %d experience points.\n\r",
    		victim->name, value);
    		send_to_char(buf, ch);

    if ( value > 0 )
    { 
      sprintf( buf,"You have been bonused %d experience points.\n\r", value );
      send_to_char( buf, victim );
      gain_exp(victim, value, TRUE );
    }
    else
    {
      sprintf( buf,"You have been penalized %d experience points.\n\r", value );
      send_to_char( buf, victim );
      gain_exp(victim, value, FALSE );
    }
 
    return;
}

void do_perm( CHAR_DATA *ch, char *argument )
{
  char item[MAX_INPUT_LENGTH], affect[MAX_INPUT_LENGTH], mod[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  AFFECT_DATA *aff=NULL;
  record_call("<do_perm>");    

  argument = one_argument( argument, item );
  argument = one_argument( argument, affect );
  argument = one_argument( argument, mod );
 
  if (item[0] == '\0' || affect[0] == '\0')
  {
    send_to_char( "Perm <item> <affect sn> [modifier]\n\r", ch );
    return;
  }
 
  if( ( obj = get_obj_carry( ch, item )) == NULL )
  {
    send_to_char( "You don't have that item.\n\r", ch );
    return;
  }

  CREATE( aff, AFFECT_DATA, 1 );
  aff->duration = -1;
  xCLEAR_BITS(aff->bitvector);
  aff->type = aff->location == APPLY_WEARSPELL;
  if (mod [0] != '\0')
     aff->modifier = atoi (mod);
  else
     aff->modifier = ch-> level;

 
  return;
}

void do_bounty ( CHAR_DATA *ch , char *argument)

{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
char buf[MAX_STRING_LENGTH];
AFFECT_DATA af;
int cost;
record_call("<do_bounty>");    

    argument = one_argument( argument, arg1);
               one_argument( argument, arg2);

if (ch->in_room->vnum == 30601) /*You can change this to*/
/*whatever room you want or you can get rid of it*/
/* And the point of it is??? -- Cal */
{
send_to_char("You cannot do that here.\n\r",ch);
return;
}

if ( ( victim = get_char_world(ch,arg1) ) == NULL )
    {
        send_to_char("That person is not on the mud.\n\r",ch);
        return;
    }

cost = atoi(arg2);

if (IS_NPC(victim))
    {
	send_to_char("Not on mobs dummy.\n\r",ch);
return;
}

	if ( cost < 1 )
	{
	    send_to_char( "How generous of you\n\r", ch );
	    return;
	}

        if ( ch->gold < cost )
        {
            send_to_char( "You can't afford it.\n\r", ch );
            return;
        }

    if (ch->pcdata->clevel <= 7 )
    {
        send_to_char("Only Clan leaders may bounty people.(CLevel 8 +)\n\r",ch);
        return;
    }

if (IS_AFFECTED(victim,AFF_BOUNTY))
{
send_to_char( "That person is already bountied.\n\r",ch);
return;
}
    ch->gold -= cost;
    af.type      = 0;
    af.duration  = -1;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = meb(AFF_BOUNTY);
    affect_to_char( victim, &af );
    victim->pcdata->bounty = cost;

send_to_char( "You have been bountied!\n\r", victim );
send_to_char( "Target bountied\n\r",ch);
sprintf( buf, "[ASSASSINATION] A bounty has been placed on %s for %d gold!.\n\r",victim->name,cost);
echo_to_all( AT_BLOOD,buf,ECHOTAR_ALL );
return;
}

void do_unbounty ( CHAR_DATA *ch , char *argument)

{
    char arg1[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
int cost;
char buf[MAX_STRING_LENGTH];
    argument = one_argument( argument, arg1);
    record_call("<do_unbounty>");    

if (ch->in_room->vnum == 30601)
{
send_to_char("You cannot do that here.\n\r",ch);
return;
}

if ( ( victim = get_char_world(ch,arg1) ) == NULL )
    {
        send_to_char("That person is not on the mud.\n\r",ch);
        return;
    }

if (IS_NPC(victim))
    {
        send_to_char("Not on mobs dummy.\n\r",ch);
	return;
     }

cost = victim->pcdata->bounty;

        if (ch->gold < cost )
        {
            send_to_char( "You can't afford it.\n\r", ch );
            return;
        }
if (!IS_AFFECTED(victim,AFF_BOUNTY))
{
send_to_char("That person is not bountied.\n\r",ch);
return;
}
else
{
           xREMOVE_BIT( victim->affected_by, AFF_BOUNTY );
	affect_strip(victim,0);
            send_to_char( "Bounty flag removed.\n\r", ch );
            send_to_char( "You are no longer bountied.\n\r", victim );
		ch->gold -= cost;
		victim->pcdata->bounty = 0;

        sprintf( buf, "[ASSASSINATION] The bounty on %s has been removed.\n\r", victim->name);
	echo_to_all( AT_BLOOD,buf,ECHOTAR_ALL );
return;
}
}

void do_pbounty ( CHAR_DATA *ch , char *argument)

{
    char arg1[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    argument = one_argument( argument, arg1);
    record_call("<do_pbounty>");


if ( ( victim = get_char_world(ch,arg1) ) == NULL )
    {
        send_to_char("That person is not on the mud.\n\r",ch);
        return;
    }

if (IS_NPC(victim))
    {
        send_to_char("Not on mobs dummy.\n\r",ch);
	return;
     }

if (!IS_AFFECTED(victim,AFF_BOUNTY))
{
send_to_char("That person is not bountied.\n\r",ch);
return;
}
else
{
            xREMOVE_BIT( victim->affected_by, AFF_BOUNTY );
	affect_strip(victim,0);
            send_to_char( "Bounty flag removed.\n\r", ch );
            send_to_char( "You are no longer bountied.\n\r", victim );
	    victim->pcdata->bounty = 0;
        sprintf( buf, "[ASSASSINATION] The bounty on %s has been pardoned by an Immortal.\n\r", victim->name);
	echo_to_all( AT_BLOOD,buf,ECHOTAR_ALL );
return;
}
}


void do_rental( CHAR_DATA *ch, char *argument )
{
   
    record_call("<do_rental>");

    if (ch->in_room->sector_type == 14) 
    {

     if ( IS_NPC(ch))
       return;
    
     if ( IS_SET(ch->pcdata->flags, PCFLAG_TENNANT))
      {
       send_to_char("You haven't checked out from last time, yet!\n\r", ch);
       return;
      }
    
     send_to_char("You rent the room.\n\r", ch);
     ch->in_room->sector_type += 1;
     SET_BIT(ch->pcdata->flags, PCFLAG_TENNANT);
     ch->gold -= 100;
     return;
    }
    else
    {
     send_to_char("You can't rent this room.\n\r", ch);
     return;
    }
}

void do_checkout( CHAR_DATA *ch, char *argument )
{
    
    record_call("<do_checkout>");    

    if (ch->in_room->sector_type == 15)
    {
	
     if ( IS_NPC(ch))
       return;
    
     if ( (IS_SET(ch->pcdata->flags, PCFLAG_TENNANT)) && (  ch->in_room->sector_type == 15 ) )
      {
       send_to_char("You check out of the room.\n\r", ch);
       REMOVE_BIT(ch->pcdata->flags, PCFLAG_TENNANT);
       ch->in_room->sector_type -= 1;
       ch->pcdata->rentroom = 0;
       return;
      }
     else
     {
       send_to_char("This is not your room.\n\r", ch);
       return;
     }
    }
       send_to_char("You have to Checkin first\n\r", ch);
	return;
}


void do_donate( CHAR_DATA *ch, char *argument)
{
   OBJ_DATA *obj;
   OBJ_DATA *pit;
   ROOM_INDEX_DATA *original;
   char arg[MAX_INPUT_LENGTH];
   CLAN_DATA *clan;
   bool NO_PIT=FALSE;

   record_call("<do_donate>");    

   argument = one_argument(argument, arg);

   if (ch == NULL )
	return;

   if (arg[0] == '\0' )
   {
      send_to_char("Donate what?\n\r",ch);
      return;
   }

   original = ch->in_room;
   if (ch->position == POS_FIGHTING)
   {
      send_to_char(" You're fighting!\n\r",ch);
      return;
   }

   if ( !IS_NPC(ch) && IS_SET(ch->pcdata->flagstwo, MOREPC_HOARDER) )
   {
     send_to_char("No.\n\r",ch);
     return;
   }

   if ( (obj = get_obj_carry (ch, arg)) == NULL)
   {
      send_to_char("You do not have that!\n\r",ch);
      return;
   }
   else
   {
      if (!can_drop_obj(ch, obj) && ch->level < 51)
      {
         send_to_char("Its stuck to you.\n\r",ch);
         return;
      }
	if ( !obj )
	return;

      if ((obj->item_type == ITEM_CORPSE_NPC) ||
         (obj->item_type == ITEM_CORPSE_PC))
      {
         send_to_char("You cannot donate that!\n\r",ch);
         return;
      }
      if (obj->timer > 0)
      {
         send_to_char("You cannot donate that.\n\r",ch);
         return;
      }

      if ( IS_SET( obj->second_flags, ITEM2_LIMITED ) )
      {
        separate_obj(obj);
	act(AT_MAGIC,"$p is destroyed as you attempt to donate it.",ch,obj,NULL,TO_CHAR);
	act(AT_MAGIC,"$p is destroyed as $n attempts to donate it.",ch,obj,NULL,TO_ROOM);
	extract_obj(obj);
	return;
      }

      if ( obj->item_type == ITEM_CONTAINER )
      {
        send_to_char("Containers cannot be donated.\n\r",ch);
        return;
      }

      act(AT_PLAIN,"$n donates $p.",ch,obj,NULL,TO_ROOM);
      act(AT_PLAIN,"You donate $p.",ch,obj,NULL,TO_CHAR);
         
	 separate_obj(obj);
         char_from_room(ch);
         char_to_room(ch,get_room_index(ROOM_VNUM_DONATE,1)); 
         if ( ( pit = get_obj_here( ch, "donation" ) ) == NULL )
	 NO_PIT=TRUE; 
         obj_from_char(obj);
	 if ( !NO_PIT )
	 {
           obj_to_obj( obj, pit );
	   pit_monster(pit,ch);
	 }
	 else
	   obj_to_room( obj, ch->in_room );

    /* Clan storeroom checks */
    if ( IS_SET(ch->in_room->room_flags, ROOM_CLANSTOREROOM))
        for ( clan = first_clan; clan; clan = clan->next )
          if ( clan->storeroom == ch->in_room->vnum )
            save_clan_storeroom(ch, clan);

    /* Save-Rooms --GW */
    if ( IS_SET(ch->in_room->room_flags, ROOM_SAVE) )
    save_save_room(ch);

         char_from_room(ch);
         char_to_room(ch,original);
         return;
     }   
}


/*
 * Generic channel function.
 */
void talk_channel( CHAR_DATA *ch, char *argument, int channel, char *verb )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char toself[MAX_STRING_LENGTH];
    char toall[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int position;
    SOCIALTYPE *social;
    CHAR_DATA *victim;
    bool sent_to_all;
    bool IS_EMOTE;
    bool clan;
    int str=0;
    int cmap=0;
    int chancolor=0;

    sent_to_all = FALSE;
    victim = (CHAR_DATA*)NULL;
    social = (SOCIALTYPE*)NULL;

    record_call("<talk_channel>");    

    strcpy(buf,"(None)");
    strcpy(toall,"(None)");

    IS_EMOTE = FALSE;
    clan = FALSE;

    if ( IS_NPC( ch ) && channel == CHANNEL_CLAN )
    {
	send_to_char( "Mobs can't be in clans.\n\r", ch );
	return;
    }

    if ( IS_NPC( ch ) && channel == CHANNEL_ORDER )
    {
	send_to_char( "Mobs can't be in orders.\n\r", ch );
	return;
    }

    if ( IS_NPC( ch ) && channel == CHANNEL_COUNCIL )
    {
	send_to_char( "Mobs can't be in councils.\n\r", ch);
	return;
    }

    if ( IS_NPC( ch ) && channel == CHANNEL_GUILD )
    {
        send_to_char( "Mobs can't be in guilds.\n\r", ch );
	return;
    }
                                              
    if ( IS_SET( ch->in_room->room_flags, ROOM_SILENCE ) )
    {
	send_to_char( "You can't do that here.\n\r", ch );
	return;
    }
    
    if ( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
      if ( ch->master )
	send_to_char( "I don't think so...\n\r", ch->master );
      return;
    }

    if ( !IS_NPC(ch) && IS_SET( ch->pcdata->flags, PCFLAG_CENSORED ) )
    argument = smash_curses(argument);
    
    if ( argument[0] == '\0' )
    {
	sprintf( buf, "%s what?\n\r", capitalize(verb) );
	buf[0] = UPPER(buf[0]);
	send_to_char( buf, ch );	/* where'd this line go? */
	return;
    }

    /* Nuke the $p Crasher */
    for( str=0; argument[str]; str++ )
    {
       if ( argument[str] == '$' && argument[str+1] )
       {
	 if( !isdigit( argument[str+1] ) )
	 {
	    send_to_char("The Only Thing that may be after a $ is a Number.\n\r",ch);
	    return;
	 }
       }
     }

    if ( argument[0] == '@')
    {
        argument = one_argument(argument, arg);

	if ( !IS_AWAKE(ch) )
	{
	  send_to_char("You cant use global socials while asleep!\n\r",ch);
	  return;
	}

	if( (social = find_social(ch,arg+1,FALSE)) == NULL)
	{
  	  if( (social = find_social(ch,arg+1,TRUE)) == NULL)
          {
	     send_to_char("No such social.\n\r", ch);
	     return;
          }
	}

	/* Make sure the social is complete -- or it WILL crash! --GW */
	if ( !social->char_no_arg || !social->others_no_arg || !social->char_found ||
	     !social->others_found || !social->vict_found || !social->char_auto ||
	     !social->others_auto )
        {
	  send_to_char("Sorry, that social is not complete.\n\r",ch);
	  return;
        }

	if(argument[0] != '\0')
	  if((victim = get_char_world(ch, argument)) == NULL)
	  { 
	   sprintf(buf, "You can't find any %s!\n\r", argument);
	   send_to_char(buf,ch);
	  }	    
    }

    if ( argument[0] == ',')
    {
      if ( argument[1] == '\0' )
      {
      sprintf(buf, "Emote what over %s?\n\r", capitalize(verb) );
      send_to_char(buf,ch);
      return;
      }
      IS_EMOTE = TRUE;
   }

   if( !IS_IMMORTAL(ch) && !IS_EMOTE && (argument[0] != '@'))
    argument = drunk_speech(argument,ch);

   if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_SILENCE) )
    {
	ch_printf( ch, "You can't %s.\n\r", capitalize(verb) );
	return;
    }
  
    if ( !IS_NPC(ch) )
    REMOVE_BIT(ch->pcdata->deaf, channel);

    cmap=get_channel_cmap_num(channel);
    switch ( channel )
    {
    default:
    if ( !IS_CHAN_MAPPED( ch ) || !colormap_check(ch,cmap))
    {
	set_char_color( AT_GOSSIP, ch );
	if(!social && IS_EMOTE == FALSE)
	{
	  sprintf( toself, "[%s] $n> %s",capitalize(verb),argument );
          act( AT_GOSSIP, toself, ch, NULL, NULL,TO_CHAR);
	  sprintf( buf, "[%s] $n> $t",capitalize(verb) );
	}
	else if(IS_EMOTE == TRUE)
	{
     	        sprintf(buf, "[%s] %s",capitalize(verb),QUICKLINK(ch->name));
	 	  strcat(buf, argument+1);
		  strcat(buf, "\n\r"); 
		send_to_char(buf,ch);
	}
	else
	{
	  if(!victim)
	    sprintf(toself, "[%s] %s", capitalize(verb), social->char_no_arg);
	  else
	    sprintf(toself, "[%s] %s", capitalize(verb),social->char_found);
	toself[1] = UPPER(toself[1]);
        act( AT_GOSSIP, toself, ch, NULL, victim,TO_CHAR);
	}

	break;
    }
    else /* Color Maps --GW */
    {
	if(!social && IS_EMOTE == FALSE)
	{
	  sprintf( toself,"%s%s",colormap_parse(ch,cmap,TRUE,ch->name,NULL),argument );
          act( AT_GOSSIP, toself, ch, NULL, NULL,TO_CHAR);
	  sprintf( buf, "[%s] $n> $t",capitalize(verb) );
	}
	else if(IS_EMOTE == TRUE)
	{
	  sprintf( buf, "%s %s",colormap_parse(ch,cmap,TRUE,NULL,NULL),QUICKLINK(ch->name));
	 	  strcat(buf, argument+1);
		  strcat(buf, "\n\r"); 
		send_to_char(buf,ch);
	}
	else
	{
	  if(!victim)

	sprintf(toself,"%s%s\n\r",colormap_parse(ch,cmap,TRUE,NULL,NULL),social->char_no_arg);
	  else
	sprintf(toself,"%s%s\n\r",colormap_parse(ch,cmap,TRUE,NULL,NULL),social->char_found);
        act( AT_GOSSIP, toself, ch, NULL, victim,TO_CHAR);
	}

	break;
      }
    case CHANNEL_WARTALK:
    if ( !colormap_check( ch, cmap ) )
    {
        set_char_color( AT_WARTALK, ch );
	if(!social && IS_EMOTE == FALSE)
	{
          sprintf( toself, "[%s] $n> %s",capitalize(verb),argument );
          act( AT_WARTALK, toself, ch, NULL, NULL,TO_CHAR);
          sprintf( buf, "[%s] $n> $t", capitalize(verb) );
	}
	else if (IS_EMOTE == TRUE)
	{
     	        sprintf(buf, "[%s] %s ", capitalize(verb),capitalize(ch->name));
	 	  strcat(buf, argument+1); 
		  strcat(buf, "\n\r"); 
		send_to_char(buf,ch);
	}
	else
	{
	  if(!victim)
	    sprintf(toself, "[%s] %s", capitalize(verb), social->char_no_arg);
	  else
	    sprintf(toself, "[%s] %s", capitalize(verb), social->char_found);
	toself[1] = UPPER(toself[1]);
	  act( AT_WARTALK, toself, ch, NULL, victim, TO_CHAR);
	}
        break;
     }
     else  /* Color Maps --GW */
     {
        set_char_color( AT_WARTALK, ch );
	if(!social && IS_EMOTE == FALSE)
	{
	  sprintf( toself,"%s%s",colormap_parse(ch,cmap,TRUE,ch->name,NULL),argument );
          act( AT_WARTALK, toself, ch, NULL, NULL,TO_CHAR);
          sprintf( buf, "[%s] $n> $t", capitalize(verb) );
	}
	else if (IS_EMOTE == TRUE)
	{
	  sprintf( buf, "%s %s",colormap_parse(ch,cmap,TRUE,NULL,NULL),QUICKLINK(ch->name));
	 	  strcat(buf, argument+1); 
		  strcat(buf, "\n\r"); 
		send_to_char(buf,ch);
	}
	else
	{
	  if(!victim)
	sprintf(toself,"%s%s\n\r",colormap_parse(ch,cmap,TRUE,NULL,NULL),social->char_no_arg);
	  else
	sprintf(toself,"%s%s\n\r",colormap_parse(ch,cmap,TRUE,NULL,NULL),social->char_found);
	  act( AT_WARTALK, toself, ch, NULL, victim, TO_CHAR);
	}
	break;
      }

    case CHANNEL_AOTA:
        set_char_color( AT_IMMORT, ch );
	if(!social && IS_EMOTE == FALSE)
	{
          sprintf( toself, ">> [%s] $n >> %s",capitalize(verb),argument );
          act( AT_IMMORT, toself, ch, NULL, NULL,TO_CHAR);
          sprintf( buf, ">> [%s] $n >> $t", capitalize(verb) );
	}
	else if (IS_EMOTE == TRUE)
	{
     	        sprintf(buf, "[%s] %s ", capitalize(verb),capitalize(ch->name));
	 	  strcat(buf, argument+1); 
		  strcat(buf, "\n\r");
                set_char_color( AT_IMMORT,ch); 
		send_to_char(buf,ch);
	}
	else
	{
	  if(!victim)
	    sprintf(toself, "[%s] %s", capitalize(verb), social->char_no_arg);
	  else
	    sprintf(toself, "[%s] %s", capitalize(verb), social->char_found);
	toself[1] = UPPER(toself[1]);
	  act( AT_IMMORT, toself, ch, NULL, victim, TO_CHAR);
	}
        break;

    case CHANNEL_ORDER:
    if ( !colormap_check(ch,cmap) )
    {
	set_char_color( AT_GOSSIP, ch );
	if(!social && IS_EMOTE == FALSE)
	{
	  sprintf( toself, "[%s] $n -> %s",ch->pcdata->clan->name,argument );
          act( AT_GOSSIP, toself, ch, NULL, NULL,TO_CHAR);
	  sprintf( buf, "[%s] $n -> $t",ch->pcdata->clan->name);
	}
	else if(IS_EMOTE == TRUE)
	{
	sprintf(buf, "[%s] %s",ch->pcdata->clan->name,capitalize(ch->name));
                  strcat(buf, argument+1);
                  strcat(buf, "\n\r");
                send_to_char(buf,ch);
	}
	else
	{
          if(!victim)
            sprintf(toself, "[%s] %s",ch->pcdata->clan->name,social->char_no_arg);
          else
            sprintf(toself, "[%s] %s",ch->pcdata->clan->name,social->char_found);
        toself[1] = UPPER(toself[1]);
          act( AT_GOSSIP, toself, ch, NULL, victim, TO_CHAR);
        }
        break;
    }
    else /* Color Maps --GW */
    {
	set_char_color( AT_GOSSIP, ch );
	if(!social && IS_EMOTE == FALSE)
	{
	  sprintf( toself,"%s%s",colormap_parse(ch,cmap,TRUE,ch->name,NULL),argument );
          act( AT_GOSSIP, toself, ch, NULL, NULL,TO_CHAR);
	  sprintf( buf, "[%s] $n -> $t",ch->pcdata->clan->name);
	}
	else if(IS_EMOTE == TRUE)
	{
	  sprintf( buf, "%s %s",colormap_parse(ch,cmap,TRUE,NULL,NULL),QUICKLINK(ch->name));
                  strcat(buf, argument+1);
                  strcat(buf, "\n\r");
                send_to_char(buf,ch);
	}
	else
	{
          if(!victim)
	sprintf(toself,"%s%s\n\r",colormap_parse(ch,cmap,TRUE,NULL,NULL),social->char_no_arg);
	  else
	sprintf(toself,"%s%s\n\r",colormap_parse(ch,cmap,TRUE,NULL,NULL),social->char_found);
          act( AT_GOSSIP, toself, ch, NULL, victim, TO_CHAR);
        }
        break;
     }
    case CHANNEL_CLAN:
	clan = TRUE;
    if ( !colormap_check(ch,cmap) )
    {
	set_char_color( AT_GOSSIP, ch );
	if(!social && IS_EMOTE == FALSE)
	{
          sprintf( toself, "[%s] $n -> %s",ch->pcdata->clan->name,argument );
          act( AT_GOSSIP, toself, ch, NULL, NULL,TO_CHAR);
          sprintf( buf, "[%s] $n -> $t",ch->pcdata->clan->name);
	}
	else if(IS_EMOTE == TRUE)
	{
     	        sprintf(buf, "[%s] %s",ch->pcdata->clan->name,capitalize(ch->name));
	 	  strcat(buf, argument+1); 
		  strcat(buf, "\n\r"); 
		send_to_char(buf,ch);
	}
	else
	{
	  if(!victim)
	    sprintf(toself, "[%s] %s",ch->pcdata->clan->name,social->char_no_arg);
	  else
	    sprintf(toself, "[%s] %s",ch->pcdata->clan->name,social->char_found);
	toself[1] = UPPER(toself[1]);
	  act( AT_GOSSIP, toself, ch, NULL, victim, TO_CHAR);
	}
        break;
     }
     else /* Color Maps --GW */
     {
	set_char_color( AT_GOSSIP, ch );
	if(!social && IS_EMOTE == FALSE)
	{
	  sprintf( toself,"%s%s",colormap_parse(ch,cmap,TRUE,ch->name,NULL),argument );
          act( AT_GOSSIP, toself, ch, NULL, NULL,TO_CHAR);
          sprintf( buf, "[%s] $n -> $t",ch->pcdata->clan->name );
	}
	else if(IS_EMOTE == TRUE)
	{
	  sprintf( buf, "%s %s",colormap_parse(ch,cmap,TRUE,NULL,NULL),QUICKLINK(ch->name));
	 	  strcat(buf, argument+1); 
		  strcat(buf, "\n\r"); 
		send_to_char(buf,ch);
	}
	else
	{
	  if(!victim)
	sprintf(toself,"%s%s\n\r",colormap_parse(ch,cmap,TRUE,NULL,NULL),social->char_no_arg);
	  else
	sprintf(toself,"%s%s\n\r",colormap_parse(ch,cmap,TRUE,NULL,NULL),social->char_found);
	  act( AT_GOSSIP, toself, ch, NULL, victim, TO_CHAR);
	}
        break;
     }

    case CHANNEL_FLAME:
     if ( !colormap_check(ch,cmap) )
     {
        set_char_color( AT_WARTALK, ch );
	if(!social && IS_EMOTE == FALSE)
	{
          sprintf( toself, "[%s] $n> %s",capitalize(verb),argument );
          act( AT_WARTALK, toself, ch, NULL, NULL,TO_CHAR);
          sprintf( buf, "[%s] $n> $t", verb );
	}
	else if (IS_EMOTE == TRUE)
	{
     	        sprintf(buf, "[%s] %s ", capitalize(verb),capitalize(ch->name));
	 	  strcat(buf, argument+1); 
		  strcat(buf, "\n\r"); 
		send_to_char(buf,ch);
	}
	else
	{
	  if(!victim)
	    sprintf(toself, "[%s] %s", capitalize(verb), social->char_no_arg);
	  else
	    sprintf(toself, "[%s] %s", capitalize(verb), social->char_found);
	toself[1] = UPPER(toself[1]);
	  act( AT_WARTALK, toself, ch, NULL, victim, TO_CHAR);
	}
        break;
     }
     else  /* Color Maps --GW */
     {
        set_char_color( AT_WARTALK, ch );
	if(!social && IS_EMOTE == FALSE)
	{
	  sprintf( toself,"%s%s",colormap_parse(ch,cmap,TRUE,ch->name,NULL),argument );
          act( AT_WARTALK, toself, ch, NULL, NULL,TO_CHAR);
          sprintf( buf, "[%s] $n> $t", verb );
	}
	else if (IS_EMOTE == TRUE)
	{
	  sprintf( buf, "%s %s",colormap_parse(ch,cmap,TRUE,NULL,NULL),QUICKLINK(ch->name));
	 	  strcat(buf, argument+1); 
		  strcat(buf, "\n\r"); 
		send_to_char(buf,ch);
	}
	else
	{
	  if(!victim)
	sprintf(toself,"%s%s\n\r",colormap_parse(ch,cmap,TRUE,NULL,NULL),social->char_no_arg);
	  else
	sprintf(toself,"%s%s\n\r",colormap_parse(ch,cmap,TRUE,NULL,NULL),social->char_found);
	  act( AT_WARTALK, toself, ch, NULL, victim, TO_CHAR);
	}
        break;
      }

    case CHANNEL_HIGHGOD:
    case CHANNEL_HIGH:
    case CHANNEL_OLYMPUS:
    if ( !colormap_check(ch,cmap) )
    {
        set_char_color( AT_CYAN, ch );
	if ( !IS_NPC(ch) )
        ch->pcdata->pre_name_toggle = 0;

	if(!social && IS_EMOTE == FALSE)
	{
          sprintf( toself, "[%s] $n:: %s",capitalize(verb), argument);
          act( AT_CYAN, toself, ch, NULL, NULL,TO_CHAR);
          sprintf( buf, "[%s] $n:: $t", capitalize(verb) );
	}
	else if (IS_EMOTE == TRUE)
	{
     	        sprintf(buf, "[%s] %s ", capitalize(verb),capitalize(ch->name));
	 	  strcat(buf, argument+1); 
		  strcat(buf, "\n\r"); 
		send_to_char(buf,ch);
	}
	else
	{
	  if(!victim)
	    sprintf(toself, "[%s] %s", capitalize(verb), social->char_no_arg);
	  else
	    sprintf(toself, "[%s] %s", capitalize(verb), social->char_found);
	toself[1] = UPPER(toself[1]);
	  act( AT_CYAN, toself, ch, NULL, victim, TO_CHAR);
	}
        break;
     }
     else  /* Color Maps --GW */
     {
        set_char_color( AT_CYAN, ch );
	if ( !IS_NPC(ch) )
        ch->pcdata->pre_name_toggle = 0;

	if(!social && IS_EMOTE == FALSE)
	{
	  sprintf( toself,"%s%s",colormap_parse(ch,cmap,TRUE,ch->name,NULL),argument );
          act( AT_CYAN, toself, ch, NULL, NULL,TO_CHAR);
          sprintf( buf, "[%s] $n:: $t", capitalize(verb) );
	}
	else if (IS_EMOTE == TRUE)
	{
	  sprintf( buf, "%s %s",colormap_parse(ch,cmap,TRUE,NULL,NULL),QUICKLINK(ch->name));
	 	  strcat(buf, argument+1); 
		  strcat(buf, "\n\r"); 
		send_to_char(buf,ch);
	}
	else
	{
	  if(!victim)
	sprintf(toself,"%s%s\n\r",colormap_parse(ch,cmap,TRUE,NULL,NULL),social->char_no_arg);
	  else
	sprintf(toself,"%s%s\n\r",colormap_parse(ch,cmap,TRUE,NULL,NULL),social->char_found);
	  act( AT_CYAN, toself, ch, NULL, victim, TO_CHAR);
	}
        break;
      }

    case CHANNEL_IMMTALK:
    case CHANNEL_PRAY:
    case CHANNEL_AVTALK:
    if ( !colormap_check(ch,cmap) )
    {
        if(!social && IS_EMOTE == FALSE)
	{
	switch( channel )
	{
	  case CHANNEL_IMMTALK:
	    if ( !IS_NPC(ch) )
            ch->pcdata->pre_name_toggle = 0;

  	    sprintf( buf, "[%s] $n:: $t", capitalize(verb) );
            break;
	  case CHANNEL_AVTALK:
  	    sprintf( buf, "[%s] $n>> $t", capitalize(verb) );
	    break;
	  default:
	  sprintf( buf, "%s($n): $t", "&R>&GPRAYER&R>&W");
	  break;
	}
	}

	else if ( IS_EMOTE == TRUE )
	{
	        if ( !IS_NPC(ch) && channel == CHANNEL_IMMTALK )
                  ch->pcdata->pre_name_toggle = 0;

     	        sprintf(buf, "[%s] $n ",(channel == CHANNEL_IMMTALK ? "Imm" : "Avatar"));
	 	  strcat(buf, argument+1); 
		act(AT_IMMORT,buf,ch,NULL,NULL,TO_CHAR);
	}
	else
	{
	        if ( !IS_NPC(ch) && channel == CHANNEL_IMMTALK )
                  ch->pcdata->pre_name_toggle = 0;

	  if(!victim)
	    sprintf(toself, "[%s] %s", channel == CHANNEL_IMMTALK ? "Imm": "Avatar", social->char_no_arg);
	  else
	    sprintf(toself, "[%s] %s", channel == CHANNEL_IMMTALK ? "Imm": "Avatar", social->char_found);
	toself[1] = UPPER(toself[1]);
	}

	        if ( !IS_NPC(ch) && channel == CHANNEL_IMMTALK )
                  ch->pcdata->pre_name_toggle = 0;
	
	position	= ch->position;
	ch->position	= POS_STANDING;

	if ( IS_EMOTE == FALSE )
	{
	if(!social)
  	  act( AT_IMMORT, buf, ch, argument, NULL, TO_CHAR );
	else
	  act( AT_IMMORT, toself, ch, NULL, victim, TO_CHAR);
	ch->position	= position;
	}
	break;
    }
    else /* Color Maps --GW */
    {
        if(!social && IS_EMOTE == FALSE)
	{
	switch( channel )
	{
	  case CHANNEL_IMMTALK:
	    if ( !IS_NPC(ch) )
            ch->pcdata->pre_name_toggle = 0;

	  sprintf(buf,"%s%s",colormap_parse(ch,cmap,TRUE,ch->name,NULL),argument );
            break;
	  case CHANNEL_AVTALK:
	  sprintf(buf,"%s%s",colormap_parse(ch,cmap,TRUE,ch->name,NULL),argument );
	    break;
	  default:
	  sprintf( buf, "%s($n): $t", "&R>&GPRAYER&R>&W");
	  break;
	}
	}

	else if ( IS_EMOTE == TRUE )
	{
	        if ( !IS_NPC(ch) && channel == CHANNEL_IMMTALK )
                  ch->pcdata->pre_name_toggle = 0;

	  sprintf( buf, "%s %s",colormap_parse(ch,cmap,TRUE,NULL,NULL),QUICKLINK(ch->name));
	 	  strcat(buf, argument+1); 
		act(AT_IMMORT,buf,ch,NULL,NULL,TO_CHAR);
	}
	else
	{
	        if ( !IS_NPC(ch) && channel == CHANNEL_IMMTALK )
                  ch->pcdata->pre_name_toggle = 0;

	  if(!victim)
	sprintf(toself,"%s%s\n\r",colormap_parse(ch,cmap,TRUE,NULL,NULL),social->char_no_arg);
	  else
	sprintf(toself,"%s%s\n\r",colormap_parse(ch,cmap,TRUE,NULL,NULL),social->char_found);
	}

	        if ( !IS_NPC(ch) && channel == CHANNEL_IMMTALK )
                  ch->pcdata->pre_name_toggle = 0;
	
	position	= ch->position;
	ch->position	= POS_STANDING;

	if ( IS_EMOTE == FALSE )
	{
	if(!social)
  	  act( AT_IMMORT, buf, ch, argument, NULL, TO_CHAR );
	else
	  act( AT_IMMORT, toself, ch, NULL, victim, TO_CHAR);
	ch->position	= position;
	}
	break;
    }
    }

    if ( IS_SET( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
    {
	sprintf( buf2, "%s: %s (%s)", IS_NPC( ch ) ? ch->short_descr : ch->name,
		 argument, verb );
	append_to_file( LOG_FILE, buf2 );
    }

    strcpy(buf2,buf);

    for ( d = first_descriptor; d; d = d->next )
    {
	CHAR_DATA *och;
	CHAR_DATA *vch;

	och = d->character;
	vch = d->character;

	if ( d->connected == CON_PLAYING
	&&   vch != ch
	&&  !IS_SET(och->pcdata->deaf, channel) )
	{
		char *sbuf = argument;

	    if ( !och->in_room )
		continue;

	    if ( channel != CHANNEL_NEWBIE &&
   	          sysdata.WAIT_FOR_AUTH && och->level <= 3 )
		continue;		
	    if ( channel == CHANNEL_IMMTALK && !IS_IMMORTAL(och) )
		continue;
	    if ( channel == CHANNEL_PRAY && !IS_IMMORTAL(och) )
		continue;
            if ( channel == CHANNEL_WARTALK && NOT_AUTHED( och ) )
                continue;
	    if ( channel == CHANNEL_AVTALK && !IS_HERO(och) )
		continue;
	    if ( channel == CHANNEL_OLYMPUS && get_trust( och ) < 58 )
                continue;
	    if ( channel == CHANNEL_HIGHGOD && get_trust( och ) < sysdata.muse_level )
		continue;
	    if ( channel == CHANNEL_HIGH    && get_trust( och ) < sysdata.think_level )
		continue;

	    if ( channel == CHANNEL_IC	&& !IS_NPC(och) )
	    {
		if ( !IS_SET( och->pcdata->flagstwo, MOREPC_IC ) && !IS_IMMORTAL(och) )
	        continue;
	    }

            /* Fix by Narn to let newbie council members see the newbie channel. */
	    if ( channel == CHANNEL_NEWBIE  && 
                  ( !IS_IMMORTAL(och) && och->level >= 4 
                  && !( och->pcdata->council && 
                     !str_cmp( och->pcdata->council->name, "Newbie Council" ) ) ) )
		continue;
	    if ( IS_SET( vch->in_room->room_flags, ROOM_SILENCE ) )
	    	continue;
	    if ( channel == CHANNEL_YELL
	    &&   vch->in_room->area != ch->in_room->area )
		continue;

	    if ( channel == CHANNEL_CLAN || channel == CHANNEL_ORDER )
	    {
		if ( IS_NPC( vch ) )
		  continue;

	        if ( !vch->pcdata->clan )
		   continue;

		if ( vch->pcdata->clan != ch->pcdata->clan  &&
		     strcmp( vch->pcdata->clan->associate,ch->pcdata->clan->name ) )
	    	  continue;
	    }

	    if ( channel == CHANNEL_GUILD )
	    {
		if ( IS_NPC( vch ) )
		  continue;

	        if ( !vch->pcdata->guild )
		   continue;

		if ( vch->pcdata->guild != ch->pcdata->guild )
	    	  continue;
	    }

	    if ( channel == CHANNEL_COUNCIL )
	    {
		if ( IS_NPC( vch ) )
		  continue;
		if ( vch->pcdata->council != ch->pcdata->council )
	    	  continue;
	    }

	    position		= vch->position;
	    if ( channel != CHANNEL_SHOUT && channel != CHANNEL_YELL )
		vch->position	= POS_STANDING;
		if ( !knows_language( vch, ch->speaking, ch ) &&
			 (!IS_NPC(ch) || ch->speaking != 0) )
			sbuf = scramble(argument, ch->speaking);
	    MOBtrigger = FALSE;

	    strcpy(buf,buf2);

	    if(social)
	    {
	     if ( !colormap_check(vch,cmap) )
             {
		if ( !clan )
     	        sprintf(toall, "[%s] ", verb);
	        else
		sprintf(toall, "[%s] ", ch->pcdata->clan->name );
		
		if (!victim) 
	 	  strcat(toall, social->others_no_arg); 
		else
		  if (victim == vch)
		    strcat(toall, social->vict_found);
		  else
//		    if(!sent_to_all)
		      strcat(toall, social->others_found);
		toall[1] = UPPER(toall[1]);
	      }
	      else /* Color Maps --GW */
	      {
     	        sprintf(toall, "%s ",colormap_parse(vch,cmap,TRUE,NULL,NULL));
		
		if (!victim) 
	 	  strcat(toall, social->others_no_arg); 
		else
		  if (victim == vch)
		    strcat(toall, social->vict_found);
		  else
//		    if(!sent_to_all)
		      strcat(toall, social->others_found);
	      }
            }

	    if(IS_EMOTE == TRUE)
	    {
	      if ( !colormap_check( vch, cmap ) )
	      {
		if ( !clan )
     	        sprintf(buf, "[%s] %s ", verb,PERS(ch,d->character));
		else
     	        sprintf(buf, "[%s] %s ",ch->pcdata->clan->name,PERS(ch,d->character));

	 	  strcat(buf, argument+1); 
		buf[1] = UPPER(buf[1]);
	     }
	     else /* Color Maps --GW */
             {
     	        sprintf(buf, "%s %s",colormap_parse(vch,cmap,TRUE,NULL,NULL),PERS(ch,d->character));
	 	  strcat(buf, argument+1); 
             }

            }

	    /* Shorten This alot... --GW */
	    switch(channel)
            {
	      case CHANNEL_IMMTALK:
	      case CHANNEL_AVTALK:
              case CHANNEL_PRAY:
              case CHANNEL_AOTA:
                chancolor=AT_IMMORT;
   	        break;
	      case CHANNEL_WARTALK:
	      case CHANNEL_FLAME:
	        chancolor=AT_WARTALK;
		break;
	      case CHANNEL_OLYMPUS:
	      case CHANNEL_HIGHGOD:
		chancolor=AT_CYAN;
		break;
	      default:
		chancolor=AT_GOSSIP;
		break;
	    }

	    if ( !colormap_check( vch, cmap ) )
            {
	      if(!social)
	        act( chancolor, buf, ch, sbuf, vch, TO_VICT );
	      else
  	        if(!victim)
		  act( chancolor, toall, ch, sbuf, vch, TO_VICT );
	        else
		  if(victim == vch)
		    act( chancolor, toall, ch, sbuf, vch, TO_VICT );
		  else
		    if(!sent_to_all)
		    {
		      act( chancolor, toall, ch, sbuf, victim,TO_NOTVICT);
		      sent_to_all = TRUE;
		    }
		}
  		else /* Color Maps --GW */
		{
	      if ( !IS_EMOTE )
	      sprintf(buf,"%s %s",colormap_parse(vch,cmap,TRUE,QUICKLINK(ch->name),NULL),argument);
	      if(!social)
	        act( chancolor, buf, ch, sbuf, vch, TO_VICT );
	      else
  	        if(!victim)
		  act( chancolor, toall, ch, sbuf, vch, TO_VICT );
	        else
		  if(victim == vch)
		    act( chancolor, toall, ch, sbuf, vch, TO_VICT );
		  else
		    if(!sent_to_all)
		    {
		      act( chancolor, toall, ch, sbuf, victim,TO_NOTVICT);
		      sent_to_all = TRUE;
		    }
	     }

	    vch->position	= position;
	}
    }
    if ( !IS_NPC(ch) )
    ch->pcdata->pre_name_toggle = 1;

    return;
}

void to_channel( const char *argument, int channel, const char *verb, sh_int level )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    /*record_call("<to_channel>");    -- leave this out --GW*/

    if ( !first_descriptor || argument[0] == '\0' )
      return;

    sprintf(buf, "%s %s ** ]\r\n", verb, argument );

    for ( d = first_descriptor; d; d = d->next )
    {
	CHAR_DATA *och;
	CHAR_DATA *vch;

	och = d->original ? d->original : d->character;
	vch = d->character;

	if ( !och || !vch )
	  continue;
	if ( !IS_IMMORTAL(vch)
	|| ( get_trust(vch) < sysdata.build_level && channel == CHANNEL_BUILD )
	|| ( get_trust(vch) < sysdata.log_level
	&& ( channel == CHANNEL_LOG || channel == CHANNEL_HIGH || channel == CHANNEL_COMM || channel == CHANNEL_MOBLOG) ) )
	  continue;

	if ( d->connected == CON_PLAYING
	&&  !IS_SET(och->pcdata->deaf, channel)
	&&   get_trust( vch ) >= level )
	{
	  set_char_color( AT_LOG, vch );
	  send_to_char( buf, vch );
	}
    }

    return;
}


/*
void do_auction( CHAR_DATA *ch, char *argument )
{
    record_call("<do_auction>");    

    talk_channel( ch, argument, CHANNEL_AUCTION, "auction" );
    return;
}
*/

void do_aota( CHAR_DATA *ch, char *argument )
{
    talk_channel( ch, argument, CHANNEL_AOTA, "AOTA" );
    record_call("<do_aota>");    

    return;
}
   
void do_chat( CHAR_DATA *ch, char *argument )
{
     record_call("<do_chat>");    

    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }

    if (ch->level <= 3  && sysdata.WAIT_FOR_AUTH )
    {
      send_to_char("Sorry, you must be level 4 to use Gossip.\n\r",ch);
      send_to_char("Use: Newchat <string> instead, only gods can see it.\n\r",ch);
      return;
    }

    talk_channel( ch, argument, CHANNEL_CHAT, "Rumor" );
    return;

}

void do_play( CHAR_DATA *ch, char *argument )
{
     record_call("<do_play>");    

    if ( IS_NPC(ch) )
	return;

    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }

    if (ch->level <= 3  && sysdata.WAIT_FOR_AUTH )
    {
      send_to_char("Sorry, you must be level 4 to use Play.\n\r",ch);
      send_to_char("Use: Newchat <string> instead, only gods can see it.\n\r",ch);
      return;
    }

    if ( !IS_SET( ch->pcdata->flagstwo, MOREPC_IC ) )
    {
	send_to_char("Only people In Character may use this channel!\n\r",ch);
	return;
    }

    talk_channel( ch, argument, CHANNEL_IC, "IC" );
    return;

}

void do_clantalk( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }

    if ( IS_NPC( ch ) || !ch->pcdata->clan 
    ||   ch->pcdata->clan->clan_type == CLAN_ORDER
    ||   ch->pcdata->clan->clan_type == CLAN_GUILD )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }
    talk_channel( ch, argument, CHANNEL_CLAN, "gossip" );
    return;
}

void do_newbiechat( CHAR_DATA *ch, char *argument )
{
    record_call("<do_newbiechat>");    

    if ( IS_NPC( ch )
       || ( ch->level >= 4 && !IS_IMMORTAL(ch) 
       && !( ch->pcdata->council && 
          !str_cmp( ch->pcdata->council->name, "Newbie Council" ) ) ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    talk_channel( ch, argument, CHANNEL_NEWBIE, "newbiechat" );
    return;
}

void do_ot( CHAR_DATA *ch, char *argument )
{
  do_ordertalk( ch, argument );
}

void do_ordertalk( CHAR_DATA *ch, char *argument )
{
    record_call("<do_ordertalk>");    

    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }

    if ( !IS_NPC(ch) && ch->pcdata->clan && ch->pcdata->clan->clan_type == CLAN_ORDER )
    {
    talk_channel( ch, argument, CHANNEL_ORDER, "ordertalk" );
    return;
    }
    else
    {
    send_to_char("You are not in an Order!\n\r",ch);
    return;
    }
}

void do_counciltalk( CHAR_DATA *ch, char *argument )
{
    record_call("<do_counciltalk>");    

    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }

    if ( IS_NPC( ch ) || !ch->pcdata->council )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch) && ch->pcdata->council )
    {
    talk_channel( ch, argument, CHANNEL_COUNCIL, "counciltalk" );
    return;
    }
    else
    {
    send_to_char("You are not a council member!\n\r",ch);
    return;
    }
}

void do_guildtalk( CHAR_DATA *ch, char *argument )
{
    record_call("<do_guildtalk>");    

    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if ( !ch->pcdata->guild )
    {
	send_to_char( "You're not in a guild\n\r", ch );
	return;
    }

    talk_channel( ch, argument, CHANNEL_GUILD, "Guildtalk" );
    return;
}

void do_music( CHAR_DATA *ch, char *argument )
{
    record_call("<do_music>");    

    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }
    talk_channel( ch, argument, CHANNEL_MUSIC, "music" );
    return;
}


void do_quest( CHAR_DATA *ch, char *argument )
{
    record_call("<do_quest>");    

    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }
    talk_channel( ch, argument, CHANNEL_QUEST, "quest" );
    return;
}

void do_ask( CHAR_DATA *ch, char *argument )
{
    record_call("<do_ask>");    

    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }
    talk_channel( ch, argument, CHANNEL_ASK, "ask" );
    return;
}



void do_answer( CHAR_DATA *ch, char *argument )
{
    record_call("<do_answer>");    

    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }
    talk_channel( ch, argument, CHANNEL_ASK, "answer" );
    return;
}



void do_shout( CHAR_DATA *ch, char *argument )
{
    record_call("<do_shout>");    

    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }
  talk_channel( ch, drunk_speech( argument, ch ), CHANNEL_SHOUT, "shout" );
  WAIT_STATE( ch, 12 );
  return;
}



void do_yell( CHAR_DATA *ch, char *argument )
{
    record_call("<do_yell>");    

    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }
  talk_channel( ch, drunk_speech( argument, ch ), CHANNEL_YELL, "yell" );
  return;
}


void do_flame( CHAR_DATA *ch, char *argument )
{
    record_call("<do_flame>");    

    talk_channel( ch, drunk_speech( argument, ch ),CHANNEL_FLAME, "flame");
    return;
}


void do_immtalk( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }
    talk_channel( ch, argument, CHANNEL_IMMTALK, "immtalk" );
    return;
}


void do_muse( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }
    talk_channel( ch, argument, CHANNEL_HIGHGOD, "Imp" );
    return;
}


void do_think( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }
    talk_channel( ch, argument, CHANNEL_HIGH, "think" );
    return;
}

void do_olympus( CHAR_DATA *ch, char *argument )
{
    talk_channel( ch, argument, CHANNEL_OLYMPUS, "Olympus" );
    return;
}


void do_avtalk( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }
    talk_channel( ch, drunk_speech( argument, ch ), CHANNEL_AVTALK, "avtalk" );
    return;
}


void do_say( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *vch;
    int actflags;
    char word_buf[MSL];
    char word_buf2[MSL];
    char sp_buf[MSL];
    int str;
    char *arg;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Say what?\n\r", ch );
	return;
    }

    if ( !ch->in_room )
      return;

    if ( IS_SET( ch->in_room->room_flags, ROOM_SILENCE ) )
    {
	send_to_char( "You can't do that here.\n\r", ch );
	return;
    }

    actflags = ch->act;
    if ( IS_NPC( ch ) ) REMOVE_BIT( ch->act, ACT_SECRETIVE );
	for ( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
	{
		char *sbuf = argument;

		if ( vch == ch )
			continue;
		if ( !knows_language(vch, ch->speaking, ch) &&
			 (!IS_NPC(ch) || ch->speaking != 0) )
			sbuf = scramble(argument, ch->speaking);
	      sbuf = drunk_speech( sbuf, ch );

/*		MOBtrigger = FALSE;
		act( AT_SAY, "$n says '$t'", ch, sbuf, vch, TO_VICT );*/
	}

	strcpy( word_buf, "says" );
	strcpy( word_buf2, "say" );

    for ( str = 0; argument[str] != '\0'; str++ )
    {
    if ( argument[str] == '?' )
    {
	strcpy( word_buf, "asks" );
	strcpy( word_buf2, "ask" );
	break;
    }
    if ( argument[str] == '!' )
    {
	strcpy( word_buf, "exclaims" );
	strcpy( word_buf2, "exclaim" );
	break;
    }
    if ( argument[str] == ':' && argument[str+1] == ')' )
    {
	strcpy( word_buf, "smiles and says" );
	strcpy( word_buf2, "smile and say" );
	break;
    }
    if ( argument[str] == ':' && argument[str+1] == '(' )
    {
	strcpy( word_buf, "says frowningly" );
	strcpy( word_buf2, "say frowningly" );
	break;
    }

    if ( argument[str] == ':' && argument[str+1] == 'P' )
    {
	strcpy( word_buf, "sticks out $s tongue and says" );
	strcpy( word_buf2, "stick out your tongue and say" );
	break;
    }
  }
        /* Censor Stuff --GW */
    if ( !IS_NPC(ch) && IS_SET( ch->pcdata->flags, PCFLAG_CENSORED ) )
    arg = smash_curses(argument);
    else
    arg = argument;

    MOBtrigger = FALSE;
    sprintf(sp_buf, "$n %s '$T'", word_buf ); 
    act( AT_SAY,sp_buf, ch, NULL, arg, TO_ROOM );
    sp_buf[0] = '\0';
    ch->act = actflags;
    MOBtrigger = FALSE;
    sprintf(sp_buf, "You %s '$T'", word_buf2 ); 
    act( AT_SAY,sp_buf, ch, NULL, drunk_speech( arg,ch),TO_CHAR ); 
    sp_buf[0] = '\0';
    if ( IS_SET( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
    {
	sprintf( buf, "%s: %s", IS_NPC( ch ) ? ch->short_descr : ch->name,
		 arg );
	append_to_file( LOG_FILE, buf );
    }
    mprog_speech_trigger( argument, ch );
    if ( char_died(ch) )
      return;
    oprog_speech_trigger( argument, ch ); 
    if ( char_died(ch) )
      return;
    rprog_speech_trigger( argument, ch ); 
    return;
}

void do_newsay( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *vch;
    int actflags;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Say what?\n\r", ch );
	return;
    }

    if ( IS_SET( ch->in_room->room_flags, ROOM_SILENCE ) )
    {
	send_to_char( "You can't do that here.\n\r", ch );
	return;
    }

    actflags = ch->act;
    if ( IS_NPC( ch ) ) REMOVE_BIT( ch->act, ACT_SECRETIVE );
	for ( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
	{
		char *sbuf = argument;

		if ( vch == ch )
			continue;
		if ( !knows_language(vch, ch->speaking, ch) &&
			 (!IS_NPC(ch) || ch->speaking != 0) )
			sbuf = scramble(argument, ch->speaking);
	      sbuf = drunk_speech( sbuf, ch );

/*		MOBtrigger = FALSE;
		act( AT_SAY, "$n says '$t'", ch, sbuf, vch, TO_VICT );*/
	}
    if (ch->level >= 60 && ch->pcdata->stitle) {
    MOBtrigger = FALSE;
    act( AT_SAY, "$T '$T'", ch, ch->pcdata->stitle, argument,TO_ROOM );
    ch->act = actflags;
    MOBtrigger = FALSE;
    act( AT_SAY, "You say '$T'", ch, NULL, drunk_speech( argument, ch ), TO_CHAR ); 

    if ( IS_SET( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
    {
	sprintf( buf, "%s: %s", IS_NPC( ch ) ? ch->short_descr : ch->name,
		 argument );
	append_to_file( LOG_FILE, buf );
    }
    mprog_speech_trigger( argument, ch );
    if ( char_died(ch) )
      return;
    oprog_speech_trigger( argument, ch ); 
    if ( char_died(ch) )
      return;
    rprog_speech_trigger( argument, ch ); 
    return;
    } else
    {
    MOBtrigger = FALSE;
    act( AT_SAY, "$n says '$T'", ch, NULL, argument, TO_ROOM );
    ch->act = actflags;
    MOBtrigger = FALSE;
    act( AT_SAY, "You say '$T'", ch, NULL, drunk_speech( argument, ch ), TO_CHAR ); 
    if ( IS_SET( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
    {
	sprintf( buf, "%s: %s", IS_NPC( ch ) ? ch->short_descr : ch->name,
		 argument );
	append_to_file( LOG_FILE, buf );
    }
    mprog_speech_trigger( argument, ch );
    if ( char_died(ch) )
      return;
    oprog_speech_trigger( argument, ch ); 
    if ( char_died(ch) )
      return;
    rprog_speech_trigger( argument, ch ); 
    return;

    }
}



void do_tell( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int position;
    CHAR_DATA *switched_victim;
    char *arg2;
    CMAP_DATA *victmap, *chmap;
    int chcolornum=0, victcolornum=0;

    if ( IS_NPC(ch) )
     return;

    switched_victim = NULL;

    REMOVE_BIT( ch->pcdata->deaf, CHANNEL_TELLS );

    if ( IS_SET( ch->in_room->room_flags, ROOM_SILENCE ) )
    {
	send_to_char( "You can't do that here.\n\r", ch );
	return;
    }

    if (!IS_NPC(ch)
    && ( IS_SET(ch->act, PLR_SILENCE)
    ||   IS_SET(ch->act, PLR_NO_TELL) ) )
    {
	send_to_char( "You can't do that.\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );

    if ( !IS_NPC(ch) && IS_SET( ch->pcdata->flags, PCFLAG_CENSORED ) )
    arg2 = smash_curses(argument);
    else
    arg2 = argument;

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Tell whom what?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg,"newbie") && (!str_cmp(arg2,"summon")))
	{
	if (ch->level > 5)
	{
	send_to_char("Newbie shakes its head.\n\r",ch);
	return;
	}
	send_to_char("Newbie nods.\n\r",ch);
	char_from_room(ch);
	char_to_room(ch,get_room_index(30602,1));	
	return;
	}
    if ( ( victim = get_char_world( ch, arg ) ) == NULL 
       || ( IS_NPC(victim) && victim->in_room != ch->in_room ) 
       || (!NOT_AUTHED(ch) && NOT_AUTHED(victim) && !IS_IMMORTAL(ch) ) )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "You have a nice little chat with yourself.\n\r", ch );
	return;
    }

    if (NOT_AUTHED(ch) && !NOT_AUTHED(victim) && !IS_IMMORTAL(victim) )
    {
	send_to_char( "They can't hear you because you are not authorized.\n\r", ch);
	return;
    }


    if ( !IS_NPC( victim ) && ( victim->switched ) 
	&& ( get_trust( ch ) > LEVEL_AVATAR ) 
        && !IS_SET(victim->switched->act, ACT_POLYMORPHED)
	&& !IS_AFFECTED(victim->switched, AFF_POSSESS) )
    {
      send_to_char( "That player is switched.\n\r", ch );
      return;
    }

   else if ( !IS_NPC( victim ) && ( victim->switched ) 
        && (IS_SET(victim->switched->act, ACT_POLYMORPHED) 
 	||  IS_AFFECTED(victim->switched, AFF_POSSESS) ) )
     switched_victim = victim->switched;

   else if ( !IS_NPC( victim ) && ( !victim->desc ) )
    {
      send_to_char( "That player is link-dead.\n\r", ch );
      return;
    }

    if ( !IS_NPC (victim) && ( IS_SET (victim->act, PLR_AFK ) ) )
      {
      send_to_char( "That player is afk.\n\r", ch );
      return;
      }

    if ( !IS_NPC(victim) && IS_SET( victim->pcdata->deaf, CHANNEL_TELLS ) 
    && ( !IS_IMMORTAL( ch ) || ( get_trust( ch ) < get_trust( victim ) ) ) )
    {
      act( AT_PLAIN, "$E has $S tells turned off.", ch, NULL, victim,
		TO_CHAR );
      return;
    }

    if ( !IS_NPC (victim) && ( IS_SET (victim->act, PLR_SILENCE ) ) )
      {
      send_to_char( "That player is silenced.  They will receive your message but can not respond.\n\r", ch );
      }   

    if ( (!IS_IMMORTAL(ch) && !IS_AWAKE(victim) )
    || (!IS_NPC(victim)&&IS_SET(victim->in_room->room_flags, ROOM_SILENCE ) ) )
    {
    act( AT_PLAIN, "$E can't hear you.", ch, 0, victim, TO_CHAR );
	return;
    }

    if ( victim->desc		/* make sure desc exists first  -Thoric */
    &&   victim->desc->connected == CON_EDITING 
    &&   get_trust(ch) < LEVEL_GOD )
    {
	act( AT_PLAIN, "$E is currently in a writing buffer.  Please try again in a few minutes.", ch, 0, victim, TO_CHAR );
        return;
    }

 
    if(switched_victim)
      victim = switched_victim;

    if ( !colormap_check(ch, CMAP_TELLS) )
	chcolornum=AT_TELL;
    else /* Color Maps --GW */
    {
	chmap=find_char_map_by_type(ch,CMAP_TELLS);
        chcolornum=get_real_act_color_num(chmap->mapped_color);
    }  

    if ( !colormap_check(victim, CMAP_TELLS) )
	victcolornum=AT_TELL;
    else /* Color Maps --GW */
    {
	victmap=find_char_map_by_type(victim,CMAP_TELLS);
        victcolornum=get_real_act_color_num(victmap->mapped_color);
    }  

    act( chcolornum, "You tell $N '$t'", ch, arg2, victim, TO_CHAR );
    position		= victim->position;
    victim->position	= POS_STANDING;
    if ( knows_language( victim, ch->speaking, ch )
    ||  (IS_NPC(ch) && !ch->speaking) )
	act( victcolornum, "$n tells you '$t'", ch, arg2, victim, TO_VICT );
    else
	act( victcolornum, "$n tells you '$t'", ch, scramble(arg2, ch->speaking),victim, TO_VICT );
    victim->position	= position;

    if ( !IS_NPC(victim) )
    victim->pcdata->reply	= ch;

    if ( IS_SET( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
    {
	sprintf( buf, "%s: %s (tell to) %s.",
		 IS_NPC( ch ) ? ch->short_descr : ch->name,
		 arg2,
		 IS_NPC( victim ) ? victim->short_descr : victim->name );
	append_to_file( LOG_FILE, buf );
    }
    mprog_speech_trigger( argument, ch );
    return;
}



void do_reply( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int position;
    CHAR_DATA *ch_save;
    char *arg;

    if ( !ch )
	return;

    if ( IS_NPC(ch) )
       return;

    REMOVE_BIT( ch->pcdata->deaf, CHANNEL_TELLS );
    if ( IS_SET( ch->in_room->room_flags, ROOM_SILENCE ) )
    {
	send_to_char( "You can't do that here.\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_SILENCE) )
    {
	send_to_char( "Your message didn't get through.\n\r", ch );
	return;
    }

    if ( ( victim = ch->pcdata->reply ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char("You can't reply to a mobile!\n\r",ch);
	return;
    }

    if ( !IS_NPC( victim ) && ( victim->switched )
	&& can_see( ch, victim ) && ( get_trust( ch ) > LEVEL_AVATAR ) )
    {
      send_to_char( "That player is switched.\n\r", ch );
      return;
    }
   else if ( !IS_NPC( victim ) && ( !victim->desc ) )
    {
      send_to_char( "That player is link-dead.\n\r", ch );
      return;
    }
    ch_save = victim;
    victim = NULL;
    if ( ( victim = get_char_world( ch, ch_save->name ) )==NULL )
       {
	 send_to_char("No one like that is logged in.\n\r",ch);
         return;
       }

    if ( !IS_NPC (victim) && ( IS_SET (victim->act, PLR_AFK ) ) )
      {
      send_to_char( "That player is afk.\n\r", ch );
      return;
      }

    if ( !IS_NPC(victim) && IS_SET( victim->pcdata->deaf, CHANNEL_TELLS ) 
    && ( !IS_IMMORTAL( ch ) || ( get_trust( ch ) < get_trust( victim ) ) ) )
    {
      act( AT_PLAIN, "$E has $S tells turned off.", ch, NULL, victim,
	TO_CHAR );
      return;
    }

    if ( ( !IS_IMMORTAL(ch) && !IS_AWAKE(victim) )
    || ( !IS_NPC(victim) && IS_SET( victim->in_room->room_flags, ROOM_SILENCE ) ) )
    {
    act( AT_PLAIN, "$E can't hear you.", ch, 0, victim, TO_CHAR );
	return;
    }

    if ( !IS_NPC(ch) && IS_SET( ch->pcdata->flags, PCFLAG_CENSORED ) )
    arg = smash_curses(argument);
    else
    arg = argument;

    act( AT_TELL, "You tell $N '$t'", ch, arg, victim, TO_CHAR );
    position		= victim->position;
    victim->position	= POS_STANDING;
    if ( knows_language( victim, ch->speaking, ch ) ||
    	 (IS_NPC(ch) && !ch->speaking) )
	    act( AT_TELL, "$n tells you '$t'", ch, arg, victim, TO_VICT );
	else
		act( AT_TELL, "$n tells you '$t'", ch, scramble(arg, ch->speaking), victim, TO_VICT );
    victim->position	= position;

    if ( !IS_NPC(victim) )
    victim->pcdata->reply = ch;

    if ( IS_SET( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
    {
	sprintf( buf, "%s: %s (reply to) %s.",
		 IS_NPC( ch ) ? ch->short_descr : ch->name,
		 arg,
		 IS_NPC( victim ) ? victim->short_descr : victim->name );
	append_to_file( LOG_FILE, buf );
    }

    return;
}



void do_emote( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char *plast;
    CHAR_DATA *vch;
    int actflags;
    char *arg;

    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_NO_EMOTE) )
    {
	send_to_char( "You can't show your emotions.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Emote what?\n\r", ch );
	return;
    }

    actflags = ch->act;
    if ( IS_NPC( ch ) ) REMOVE_BIT( ch->act, ACT_SECRETIVE );
    for ( plast = argument; *plast != '\0'; plast++ )
	;

    if ( !IS_NPC(ch) && IS_SET( ch->pcdata->flags, PCFLAG_CENSORED ) )
    arg = smash_curses(argument);
    else
    arg = argument;

    strcpy( buf, arg );
    if ( isalpha(plast[-1]) )
	strcat( buf, "." );
	for ( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
	{
		char *sbuf = buf;
		
		if ( !knows_language( vch, ch->speaking, ch ) &&
			 (!IS_NPC(ch) && ch->speaking != 0) )
			sbuf = scramble(buf, ch->speaking);
		MOBtrigger = FALSE;
		act( AT_ACTION, "$n $t", ch, sbuf, vch, (vch == ch ? TO_CHAR : TO_VICT) );
	}
/*    MOBtrigger = FALSE;
    act( AT_ACTION, "$n $T", ch, NULL, buf, TO_ROOM );
    MOBtrigger = FALSE;
    act( AT_ACTION, "$n $T", ch, NULL, buf, TO_CHAR );*/
    ch->act = actflags;
    if ( IS_SET( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
    {
	sprintf( buf, "%s %s (emote)", IS_NPC( ch ) ? ch->short_descr : ch->name,
		 arg );
	append_to_file( LOG_FILE, buf );
    }
    return;
}


void do_bug( CHAR_DATA *ch, char *argument )
{
    if ( argument
    &&   argument != "show" )
      {
    append_file( ch, BUG_FILE, argument );
    send_to_char( "Bug recorded. Thanks\n\r", ch );
    sprintf( log_buf, "%s reports a bug: %s", ch->name, argument );
    log_string_plus( log_buf, LOG_NORMAL, ch->level );
    return;
      }
    else
      {
    set_pager_color( AT_LOG, ch );
    show_file( ch, BUG_FILE );
    return;
      }

  return;
}


void do_ide( CHAR_DATA *ch, char *argument )
{
    send_to_char("If you want to send an idea, type 'idea <message>'.\n\r", ch);
    send_to_char("If you want to identify an object and have the identify spell,\n\r", ch);
    send_to_char("Type 'cast identify <object>'.\n\r", ch);
    return;
}

void do_idea( CHAR_DATA *ch, char *argument )
{
    if ( argument
    &&   argument != "show" )
      {
    append_file( ch, IDEA_FILE, argument );
    send_to_char( "Idea recorded. Thanks.\n\r", ch );
    return;
      }
    else
      {
    set_pager_color( AT_LOG, ch );
    show_file( ch, IDEA_FILE );
    return;
      }
  return;
}



void do_typo( CHAR_DATA *ch, char *argument )
{
    if (!argument)
      send_to_char("What typo have you found?\r\n", ch );
    append_file( ch, TYPO_FILE, argument );
    send_to_char( "Ok.  Thanks.\n\r", ch );
    return;
}

void do_appeal( CHAR_DATA *ch, char *argument )
{
    if (!argument)
      send_to_char("What would you like to appeal?\r\n", ch );
    append_file( ch, APPEAL_FILE, argument );
    send_to_char( "Your appeal has been heard...\n\r", ch );
    sprintf(log_buf, "%s would like to appeal: %s", ch->name, argument );
    log_string_plus( log_buf, LOG_NORMAL, ch->level );
    return;
}



void do_rent( CHAR_DATA *ch, char *argument )
{
    set_char_color( AT_WHITE, ch );
    send_to_char( "There is no rent here.  Just save and quit.\n\r", ch );
    return;
}



void do_qui( CHAR_DATA *ch, char *argument )
{
    set_char_color( AT_RED, ch );
    send_to_char( "If you want to QUIT, you have to spell it out.\n\r", ch );
    return;
}

void do_quit2( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *temp;
  extern CHAR_DATA *loading_char;
  char *name;
  char buf[MSL];
        int level;
        int x;
        int y;
  CHAR_DATA *victim;
  CHALLENGE_DATA *challenge;
  ARENA_DATA *arena;
  record_call("QUIT2!");

    if( !ch )
    {
     bug("do_quit2: Quit failed due to NULL ch!",0);
     return;
    }

if ( IS_NPC(ch) )
return;

if( ch->in_room && ch->in_room->sector_type == 15 )
{
do_recall( ch, "" );
}

if (ch->mount)
{
do_dismount(ch,"");
}

if( IS_SET(ch->pcdata->flagstwo, MOREPC_WAR) )
  {
   send_to_char("You are removed from the game, and may not rejoin.\n\r",ch);
   REMOVE_BIT(ch->pcdata->flagstwo, MOREPC_WAR);
   SET_BIT(ch->pcdata->flagstwo, MOREPC_OUTWAR);
   do_save(ch,"");
 }

if( IS_SET(ch->pcdata->flags, PCFLAG_CHALLENGER))
{
 challenge=find_challenge(ch);
 arena=find_arena(challenge);
 if ( challenge != NULL && challenge->challenged )
 {
 sprintf(buf,"&R[CHALLENGE]&W Challenge is VOID, %s has left the game.",ch->name);
 echo_to_all( AT_PLAIN,buf,ECHOTAR_ALL);
 REMOVE_BIT(ch->pcdata->flags, PCFLAG_CHALLENGER);
 if ( ( victim=get_char(challenge->challenged))!=NULL )
 {
 ch_printf( victim, "%s has left the game, the challenge is canceled.\n\r",capitalize(ch->name));
 REMOVE_BIT( victim->pcdata->flags, PCFLAG_CHALLENGED );
 do_save( victim, "" );
 }
 do_save( ch, "");
 UNLINK(challenge, first_challenge, last_challenge, next, prev );
 DISPOSE(challenge);
 if ( arena )
 arena->in_use = FALSE;
}
}

if( IS_SET(ch->pcdata->flags, PCFLAG_CHALLENGED))
{
 challenge=find_challenge(ch);
 arena=find_arena(challenge);
 if ( challenge != NULL && challenge->challenger )
 {
 sprintf(buf,"&R[CHALLENGE]&W Challenge is VOID, %s has left the game.",ch->name);
 echo_to_all( AT_PLAIN,buf,ECHOTAR_ALL);
 REMOVE_BIT(ch->pcdata->flags, PCFLAG_CHALLENGED);
 if ( ( victim=get_char( challenge->challenger))!=NULL )
 {
 ch_printf( victim, "%s has left the game, the challenge is canceled.\n\r",capitalize(ch->name));
 if ( !IS_NPC(victim))
 REMOVE_BIT( victim->pcdata->flags, PCFLAG_CHALLENGER );
 do_save( victim, "" );
 }
 do_save( ch, "");
 UNLINK(challenge, first_challenge, last_challenge, next, prev );
 DISPOSE(challenge);
 if ( arena )
 arena->in_use = FALSE;
}
}

if ( ch->pcdata->irc_channel_master != NULL )
{
	do_endirc(ch,"");
}

  if ( !IS_NPC(ch) && IS_SET( ch->pcdata->flags, PCFLAG_OPEN_LOCKER) )
    do_locker( ch, "close" );

    level = get_trust(ch);

    sprintf( log_buf, "%s (%s) has disconnected.", ch->name,
		ch->desc ? ch->desc->host : "LINK-DEAD" );

   if ( ch->in_room )
    char_from_room(ch);

    quitting_char = ch;
    save_char_obj( ch );
    save_finger( ch );
    saving_char = NULL;
    /*
     * After extract_char the ch is no longer valid!
     */
     name = ch->name;
     temp = ch;
    load_limited_toggle(FALSE);
    extract_char( ch, TRUE );
    load_limited_toggle(TRUE);
    for ( x = 0; x < MAX_WEAR; x++ )
        for ( y = 0; y < MAX_LAYERS; y++ )
            save_equipment[x][y] = NULL;


    /* don't show who's logging off to leaving player */
    log_string_plus( log_buf, LOG_COMM, level );

    /*no zipping if the char is being loaded --GW */
    if ( temp != loading_char )
    {
    sprintf(buf,"gzip -f %s%c/%s",PLAYER_DIR,tolower(name[0]),
    capitalize(name));
    system(buf);
    sprintf(log_buf,"Player Data for %s Compressed.",name);
    log_string_plus( log_buf, LOG_COMM, level );
    }

temp = NULL;
return;
}

void do_quit( CHAR_DATA *ch, char *argument )
{
//  char buf[MAX_STRING_LENGTH];
  int qnum;
  char bye_buf[MAX_STRING_LENGTH];
  bool wizi;

  wizi = FALSE;

    if ( !ch )
    {
	bug("do_quit: NULL ch!",0);
	return;
    }
    if ( !ch->desc )
    {
	bug("do_quit: ch has no descriptor",0);
	return;
    }

    if ( IS_NPC(ch) && IS_SET(ch->act, ACT_POLYMORPHED))
    { 
      send_to_char("You can't quit while polymorphed.\n\r", ch);
      return;
    }

    if ( IS_NPC(ch) )
	return;

 if( in_arena( ch ) 
 &&  ch->in_room->vnum != 8 )
  {
   send_to_char("You can't quit while in the arena!\n\r",ch);
   return;
  }

if ( IS_SET(ch->pcdata->flags, PCFLAG_KOMBAT) )
{
send_to_char("NO! DIE FIRST!!!!\n\r",ch);
return;
}

    if ( ch->position == POS_FIGHTING )
    {
	set_char_color( AT_RED, ch );
	send_to_char( "No way! You are fighting.\n\r", ch );
	return;
    }

    if ( ch->position  < POS_STUNNED  )
    {
	set_char_color( AT_BLOOD, ch );
	send_to_char( "You're not DEAD yet.\n\r", ch );
	return;
    }

    if (( get_timer(ch, TIMER_RECENTFIGHT) > 0)
    &&  (!IS_IMMORTAL(ch)) )
    {
	set_char_color( AT_RED, ch );
	send_to_char( "Your adrenaline is pumping too hard to quit now!\n\r", ch );
	return;
    }

    if ( auction->item != NULL && ((ch == auction->buyer) || (ch == auction->seller) ) )
    {
	send_to_char("Wait until you have bought/sold the item on auction.\n\r", ch);
	return;
    }
    if ( !ch->in_room )
      {
         sprintf( log_buf, "%s attempting quit from NULL room", ch->name );
         bug( log_buf,0);
         send_to_char( "Try in a few seconds", ch );
         return;
      }

set_char_color( AT_WHITE, ch );
qnum = dice ( 1 , 6 );
send_to_char("\n\r", ch );
if ( qnum == 1 )
send_to_char( "Your surroundings begin to fade as a mystical swirling vortex as the darkness surrounds you ...", ch );
if ( qnum == 2 )
send_to_char( "OH MY GOD - SORRY I PURGED YOU!!!!!!", ch);
if ( qnum == 3 )
send_to_char( "FINE LEAVE!!! SEE IF I CARE!!!", ch);
if ( qnum == 4 )
send_to_char( "Fine ... Leave ... Have a life. ", ch);
if ( qnum == 5 )
send_to_char( "Your leaving!?! Why, the Immortals havent had a chance to ABUSE YOU yet!", ch);
if ( qnum == 6 )
send_to_char( "HEY! YOU! You can only leave when I say YOU can LEAVE!! Get BACK HERE!!! ", ch );
send_to_char("\n\r", ch );
set_char_color( AT_WHITE, ch );

   act( AT_RED, "A Portal to the feared realm of Real Life Appears..", ch, NULL, NULL, TO_ROOM );
   act( AT_RED, "$n steps through the portal.", ch, NULL, NULL, TO_ROOM );

    sprintf( bye_buf, "The Raspy voice of the Keeper intones '%s has Escaped me for now ..'",ch->name);
    if ( IS_SET(ch->act, PLR_WIZINVIS ) || ch->level == 59 ||
         IS_SET(ch->pcdata->flagstwo, MOREPC_INCOG ) )
    wizi = TRUE;

    if ( wizi == FALSE )
    echo_to_all(AT_MAGIC,bye_buf,ECHOTAR_ALL);

	send_to_char("\n\r",ch);
	do_quit2(ch,"");
	return;
}

void send_rip_screen( CHAR_DATA *ch )
{
    FILE *rpfile;
    int num=0;
    char BUFF[MAX_STRING_LENGTH*2];

    if ((rpfile = fopen(RIPSCREEN_FILE,"r")) !=NULL) {
      while ((BUFF[num]=fgetc(rpfile)) != EOF)
	 num++;
      new_fclose(rpfile);
      BUFF[num] = 0;
      write_to_buffer(ch->desc,BUFF,num);
    }
}

void send_rip_title( CHAR_DATA *ch )
{
    FILE *rpfile;
    int num=0;
    char BUFF[MAX_STRING_LENGTH*2];

    if ((rpfile = fopen(RIPTITLE_FILE,"r")) !=NULL) {
      while ((BUFF[num]=fgetc(rpfile)) != EOF)
	 num++;
      new_fclose(rpfile);
      BUFF[num] = 0;
//      write_to_buffer(ch->desc,BUFF,num);
	/* make it color!! --GW */
	write_to_desc_color(BUFF,ch->desc);
    }
}

void send_ansi_title( CHAR_DATA *ch )
{
    FILE *rpfile;
    int num=0;
    char BUFF[MAX_STRING_LENGTH*2];

   do_clear( ch, "" );

    if ((rpfile = fopen(ANSITITLE_FILE,"r")) !=NULL) {
      while ((BUFF[num]=fgetc(rpfile)) != EOF)
	 num++;
      new_fclose(rpfile);
      BUFF[num] = 0;
//      write_to_buffer(ch->desc,BUFF,num);
	/* Make it Color! --GW */
	write_to_desc_color(BUFF,ch->desc);
    }
}

void send_ascii_title( CHAR_DATA *ch )
{
    FILE *rpfile;
    int num=0;
    char BUFF[MAX_STRING_LENGTH];

  do_clear( ch, "" );

    if ((rpfile = fopen(ASCTITLE_FILE,"r")) !=NULL) {
      while ((BUFF[num]=fgetc(rpfile)) != EOF)
	 num++;
      new_fclose(rpfile);
      BUFF[num] = 0;
//      write_to_buffer(ch->desc,BUFF,num);
	/* make it color!! --GW */
	write_to_desc_color(BUFF,ch->desc);
    }
}

void do_rip( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Rip ON or OFF?\n\r", ch );
	return;
    }
    if ( (strcmp(arg,"on")==0) || (strcmp(arg,"ON") == 0) ) {
	send_rip_screen(ch);
	SET_BIT(ch->act,PLR_RIP);
	SET_BIT(ch->act,PLR_ANSI);
	return;
    }

    if ( (strcmp(arg,"off")==0) || (strcmp(arg,"OFF") == 0) ) {
	REMOVE_BIT(ch->act,PLR_RIP);
	send_to_char( "!|*\n\rRIP now off...\n\r", ch );
	return;
    }
}

/* 
 * Divert them to Color ... --GW
 */
void do_ansi( CHAR_DATA *ch, char *argument )
{
send_to_char("Read help Color!\n\r",ch);
return;
}

void do_save( CHAR_DATA *ch, char *argument )
{
char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) && IS_SET(ch->act, ACT_POLYMORPHED))
    { 
      send_to_char("You can't save while polymorphed.\n\r", ch);
      return;
    }

    if ( IS_NPC(ch) )
	return;

    if ( ch->level < 2 )
    {
	set_char_color( AT_BLUE, ch );
	send_to_char( "You must be at least second level to save.\n\r", ch );
	return;
    }
   
    sprintf(buf,"Saving %s.\n\r",capitalize(ch->name));
    send_to_char(buf,ch);


    if ( !xHAS_BITS( ch->affected_by, race_table[ch->race]->affected ) )
	xSET_BITS( ch->affected_by, race_table[ch->race]->affected );
    if ( !IS_SET( ch->resistant, race_table[ch->race]->resist ) )
	SET_BIT( ch->resistant, race_table[ch->race]->resist );
    if ( !IS_SET( ch->susceptible, race_table[ch->race]->suscept ) )
	SET_BIT( ch->susceptible, race_table[ch->race]->suscept );	   

    if ( ch->pcdata->deity )
    {
	if ( !xHAS_BITS( ch->affected_by, ch->pcdata->deity->affected ) )
	   xSET_BITS( ch->affected_by, ch->pcdata->deity->affected );
	if ( !IS_SET( ch->resistant, ch->pcdata->deity->element ) )
	   SET_BIT( ch->resistant, ch->pcdata->deity->element );
	if ( !IS_SET( ch->susceptible, ch->pcdata->deity->suscept ) )
	   SET_BIT( ch->susceptible, ch->pcdata->deity->suscept );
    }
    save_char_obj( ch );
    save_finger( ch );
    saving_char = NULL;
    send_to_char( "Done.\n\r", ch );
    return;
}


/*
 * Something from original DikuMUD that Merc yanked out.
 * Used to prevent following loops, which can cause problems if people
 * follow in a loop through an exit leading back into the same room
 * (Which exists in many maze areas)			-Thoric
 */
bool circle_follow( CHAR_DATA *ch, CHAR_DATA *victim )
{
    CHAR_DATA *tmp, *tmp_next;

    for ( tmp = victim; tmp; tmp = tmp_next )
    {
      tmp_next = tmp->master;

	if ( tmp == ch )
	  return TRUE;
    }

    return FALSE;
}


void do_follow( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Follow whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master )
    {
	act( AT_PLAIN, "But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR );
	return;
    }

    if ( victim == ch )
    {
	if ( !ch->master )
	{
	    send_to_char( "You already follow yourself.\n\r", ch );
	    return;
	}
	stop_follower( ch );
	return;
    }

    if ( circle_follow( ch, victim ) )
    {
	send_to_char( "Following in loops is not allowed... sorry.\n\r", ch );
	return;
    }

    if ( ch->master )
	stop_follower( ch );

    add_follower( ch, victim );
    return;
}



void add_follower( CHAR_DATA *ch, CHAR_DATA *master )
{
    if ( ch->master )
    {
	bug( "Add_follower: non-null master.", 0 );
	return;
    }

    ch->master        = master;
    ch->leader        = NULL;

    if ( can_see( master, ch ) )
    act( AT_ACTION, "$n starts following you.", ch, NULL, master, TO_VICT );

    act( AT_ACTION, "You start following $N.",  ch, NULL, master, TO_CHAR );

    /* Shrug, want the room to see this --GW */
    act( AT_ACTION, "$n starts following $N.",  ch, NULL, master, TO_NOTVICT );

    return;
}



void stop_follower( CHAR_DATA *ch )
{
    if ( !ch->master )
    {
	bug( "Stop_follower: null master.", 0 );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) )
    {
	xREMOVE_BIT( ch->affected_by, AFF_CHARM );
	affect_strip( ch, gsn_charm_person );
    }

    if ( can_see( ch->master, ch ) )
    act( AT_ACTION, "$n stops following you.",     ch, NULL, ch->master, TO_VICT    );
    act( AT_ACTION, "You stop following $N.",      ch, NULL, ch->master, TO_CHAR    );

    /* make room see it --GW */
    act( AT_ACTION, "$n stops following $N.",      ch, NULL,ch->master,TO_NOTVICT   );

    ch->master = NULL;
    ch->leader = NULL;
    return;
}



void die_follower( CHAR_DATA *ch )
{
    CHAR_DATA *fch;
    ZONE_DATA *in_zone;

    if ( ch->master )
	stop_follower( ch );

    ch->leader = NULL;

    for ( fch = first_char; fch; fch = fch->next )
    {
	if ( fch->master == ch )
	    stop_follower( fch );
	if ( fch->leader == ch )
	    fch->leader = fch;
    }

for( in_zone = first_zone; in_zone; in_zone = in_zone->next )
{
   for( fch = in_zone->first_mob; fch; fch = fch->next )
   {
        if ( fch->master == ch )
            stop_follower( fch );
        if ( fch->leader == ch )
            fch->leader = fch;
   }
}

    return;
}



void do_order( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char argbuf[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *och;
    CHAR_DATA *och_next;
    bool found;
    bool fAll;

    strcpy( argbuf, argument );
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Order whom to do what?\n\r", ch );
	return;
    }

    while( isspace( *argument ) )
    argument++;

    /* Nuke the mp assholes.. GW */
    if ( argument[0] == 'm' && argument[1] == 'p' )
    {
	send_to_char("Um .. No .. screwball.\n\r",ch);
	return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You feel like taking, not giving, orders.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	fAll   = TRUE;
	victim = NULL;
    }
    else
    {
	fAll   = FALSE;
	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "Aye aye, right away!\n\r", ch );
	    return;
	}

	if ( !IS_AFFECTED(victim, AFF_CHARM) || victim->master != ch )
	{
	    send_to_char( "Do it yourself!\n\r", ch );
	    return;
	}
    }

    found = FALSE;
    for ( och = ch->in_room->first_person; och; och = och_next )
    {
	och_next = och->next_in_room;

	if ( IS_AFFECTED(och, AFF_CHARM)
	&&   och->master == ch
	&& ( fAll || och == victim ) )
	{
	    found = TRUE;
	act( AT_ACTION, "$n orders you to '$t'.", ch, argument, och, TO_VICT );
	    interpret( och, argument );
	}
    }

    if ( found )
    {
        sprintf( log_buf, "%s: order %s.", ch->name, argbuf );
        fprintf( stderr, log_buf );
 	send_to_char( "Ok.\n\r", ch );
        WAIT_STATE( ch, 12 );
    }
    else
	send_to_char( "You have no followers here.\n\r", ch );
    return;
}

/*
char *itoa(int foo)
{
  static char bar[256];

  sprintf(bar,"%d",foo);
  return(bar);

}
*/

void do_group( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim=NULL;
    ZONE_DATA *in_zone;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	CHAR_DATA *gch;
	CHAR_DATA *leader;

	leader = ch->leader ? ch->leader : ch;
	set_char_color( AT_GREEN, ch );
	ch_printf( ch, "%s's group:\n\r", PERS(leader, ch) );

/* Changed so that no info revealed on possess */
	for ( gch = first_char; gch; gch = gch->next )
	{
	    if ( is_same_group( gch, ch ) )
	    {
	        gch->pcdata->pre_name_toggle=0;
		set_char_color( AT_DGREEN, ch );
		if (IS_AFFECTED(gch, AFF_POSSESS))
		{
		  ch_printf( ch,
		" [%2d %s] %-16s %4s/%4s hp %4s/%4s %s %4s/%4s mv %5s xp\n\r",
              
		    GET_MAX_LEVEL(gch),
		    IS_NPC(gch) ? "Mob" : GET_MAX_CLASS(gch),
		    capitalize( PERS(gch, ch) ),
		    "????",   
		    "????",
		    "????",
		    "????",
		    IS_VAMPIRE(gch) ? "bp" : "mana",
		    "????",  
		    "????",  
		    "?????"    );
	        gch->pcdata->pre_name_toggle=1;
		}
		else
		{
		gch->pcdata->pre_name_toggle=0;
		  ch_printf( ch,
		"(%d) [%2d %s] %-16s %4d/%4d hp %4d/%4d %s %4d/%4d mv %5d xp\n\r",
                    
                    gch->alignment, 
		    GET_MAX_LEVEL(gch),
		    IS_NPC(gch) ? "Mob" : GET_MAX_CLASS(gch),
		    capitalize( PERS(gch, ch) ),
		    gch->hit,   
		    gch->max_hit,
		    IS_VAMPIRE(gch) ? gch->pcdata->condition[COND_BLOODTHIRST]
				    : gch->mana,
		    IS_VAMPIRE(gch) ? get_bloodthirst(ch) : gch->max_mana,
		    IS_VAMPIRE(gch) ? "bp" : "mana",
		    gch->move,  
		    gch->max_move,  
		    gch->exp    );
		gch->pcdata->pre_name_toggle=1;
		}
	    }
	}
for( in_zone = first_zone; in_zone; in_zone = in_zone->next )
{
/* Changed so that no info revealed on possess */
	for ( gch = in_zone->first_mob; gch; gch = gch->next )
	{
	    if ( is_same_group( gch, ch ) )
	    {
		set_char_color( AT_DGREEN, ch );
		if (IS_AFFECTED(gch, AFF_POSSESS))
		  ch_printf( ch,
		" [%2d %s] %-16s %4s/%4s hp %4s/%4s %s %4s/%4s mv %5s xp\n\r",
              
		    gch->level,
		    IS_NPC(gch) ? "Mob" : class_table[gch->class]->who_name,
		    capitalize( PERS(gch, ch) ),
		    "????",   
		    "????",
		    "????",
		    "????",
		    IS_VAMPIRE(gch) ? "bp" : "mana",
		    "????",  
		    "????",  
		    "?????"    );

		else
		  ch_printf( ch,
		"(%d) [%2d %s] %-16s %4d/%4d hp %4d/%4d %s %4d/%4d mv %5d
xp\n\r",
                    
                    gch->alignment, 
		    gch->level,
		    IS_NPC(gch) ? "Mob" : class_table[gch->class]->who_name,
		    capitalize( PERS(gch, ch) ),
		    gch->hit,   
		    gch->max_hit,
		    IS_VAMPIRE(gch) ? gch->pcdata->condition[COND_BLOODTHIRST]
				    : gch->mana,
		    IS_VAMPIRE(gch) ? get_bloodthirst(ch) : gch->max_mana,
		    IS_VAMPIRE(gch) ? "bp" : "mana",
		    gch->move,  
		    gch->max_move,  
		    gch->exp    );
	    }
	}
   }
	return;
    }

    if ( !strcmp( arg, "disband" ))
    {
	CHAR_DATA *gch;
	int count = 0;

	if ( ch->leader || ch->master )
	{
	    send_to_char( "You cannot disband a group if you're following someone.\n\r", ch );
	    return;
	}

for( in_zone = first_zone; in_zone; in_zone = in_zone->next )
{	
	for ( gch = in_zone->first_mob; gch; gch = gch->next )
	{
	    if ( is_same_group( ch, gch )
	    && ( ch != gch ) )
	    {
		gch->leader = NULL;
		gch->master = NULL;
		count++;
		send_to_char( "Your group is disbanded.\n\r", gch );
	    }
	}
	for ( gch = first_char; gch; gch = gch->next )
	{
	    if ( is_same_group( ch, gch )
	    && ( ch != gch ) )
	    {
		gch->leader = NULL;
		gch->master = NULL;
		count++;
		send_to_char( "Your group is disbanded.\n\r", gch );
	    }
	}
   }

	if ( count == 0 )
	   send_to_char( "You have no group members to disband.\n\r", ch );
	else
	   send_to_char( "You disband your group.\n\r", ch );
	
    return;
    }

    if ( !strcmp( arg, "all" ) )
    {
	CHAR_DATA *rch;
	int count = 0;

        for ( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
	{
           if ( ch != rch
           &&   !IS_NPC( rch )
	   &&   can_see( ch, rch )
	   &&   rch->master == ch
	   &&   !ch->master
	   &&   !ch->leader
    	   &&   abs( ch->level - rch->level ) < 8
    	   &&   !is_same_group( rch, ch )
           &&   (ch->level <= (rch->level+15))
	   &&   IS_ADVANCED( ch ) == IS_ADVANCED( rch)
	      )
	   {
		rch->leader = ch;
		count++;
	   }
	}
	
	if ( count == 0 )
	  send_to_char( "You have no eligible group members.\n\r", ch );
	else
	{
     	   act( AT_ACTION, "$n groups $s followers.", ch, NULL, victim, TO_ROOM );
	   send_to_char( "You group your followers.\n\r", ch );
	}
    return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch->master || ( ch->leader && ch->leader != ch ) )
    {
	send_to_char( "But you are following someone else!\n\r", ch );
	return;
    }

    if ( victim->master != ch && ch != victim )
    {
    act( AT_PLAIN, "$N isn't following you.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( is_same_group( victim, ch ) && ch != victim )
    {
	victim->leader = NULL;
    act( AT_ACTION, "$n removes $N from $s group.",   ch, NULL, victim, TO_NOTVICT );
    act( AT_ACTION, "$n removes you from $s group.",  ch, NULL, victim, TO_VICT    );
    act( AT_ACTION, "You remove $N from your group.", ch, NULL, victim, TO_CHAR    );
	return;
    }

    if ( ch->level - victim->level < -15
    ||   ch->level - victim->level >  15
    || IS_ADVANCED( ch ) != IS_ADVANCED( victim ) )
    
    {
    act( AT_PLAIN, "$N cannot join $n's group.",     ch, NULL, victim, TO_NOTVICT );
    act( AT_PLAIN, "You cannot join $n's group.",    ch, NULL, victim, TO_VICT    );
    act( AT_PLAIN, "$N cannot join your group.",     ch, NULL, victim, TO_CHAR    );
	return;
    }

    victim->leader = ch;
    act( AT_ACTION, "$N joins $n's group.", ch, NULL, victim, TO_NOTVICT );
    act( AT_ACTION, "You join $n's group.", ch, NULL, victim, TO_VICT    );
    act( AT_ACTION, "$N joins your group.", ch, NULL, victim, TO_CHAR    );
    return;
}



/*
 * 'Split' originally by Gnort, God of Chaos.
 */
void do_split( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    int members;
    int amount;
    int share;
    int extra;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Split how much?\n\r", ch );
	return;
    }

    amount = atoi( arg );

    if ( amount < 0 )
    {
	send_to_char( "Your group wouldn't like that.\n\r", ch );
	return;
    }

    if ( amount == 0 )
    {
	send_to_char( "You hand out zero coins, but no one notices.\n\r", ch );
	return;
    }

    if ( ch->gold < amount )
    {
	send_to_char( "You don't have that much gold.\n\r", ch );
	return;
    }

    members = 0;
    for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
    {
	if ( is_same_group( gch, ch ) )
	    members++;
    }

    
    if (( IS_SET(ch->act, PLR_AUTOGOLD)) && (members < 2))
    return;

    if ( members < 2 )
    {
	send_to_char( "Just keep it all.\n\r", ch );
	return;
    }

    share = amount / members;
    extra = amount % members;

    if ( share == 0 )
    {
	send_to_char( "Don't even bother, cheapskate.\n\r", ch );
	return;
    }

    ch->gold -= amount;
    ch->gold += share + extra;

    set_char_color( AT_GOLD, ch );
    ch_printf( ch,
	"You split %d gold coins.  Your share is %d gold coins.\n\r",
	amount, share + extra );

    sprintf( buf, "$n splits %d gold coins.  Your share is %d gold coins.",
	amount, share );

    for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
    {
	if ( gch != ch && is_same_group( gch, ch ) )
	{
	    act( AT_GOLD, buf, ch, NULL, gch, TO_VICT );
	    gch->gold += share;
	}
    }
    return;
}



void do_gtell( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *gch;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Tell your group what?\n\r", ch );
	return;
    }

    if ( IS_SET( ch->act, PLR_NO_TELL ) )
    {
	send_to_char( "Your message didn't get through!\n\r", ch );
	return;
    }

    /*
     * Note use of send_to_char, so gtell works on sleepers.
     */
/*    sprintf( buf, "%s tells the group '%s'.\n\r", ch->name, argument );*/
    for ( gch = first_char; gch; gch = gch->next )
    {
	if ( is_same_group( gch, ch ) )
	{
	    set_char_color( AT_GTELL, gch );
	    /* Groups unscrambled regardless of clan language.  Other languages
		   still garble though. -- Altrag */
	    if ( knows_language( gch, ch->speaking, gch )
	    ||  (IS_NPC(ch) && !ch->speaking) )
		ch_printf( gch, "%s tells the group '%s'.\n\r", ch->name, argument );
	    else
		ch_printf( gch, "%s tells the group '%s'.\n\r", ch->name, scramble(argument, ch->speaking) );
	}
    }

    return;
}


/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group( CHAR_DATA *ach, CHAR_DATA *bch )
{
    if ( ach->leader ) ach = ach->leader;
    if ( bch->leader ) bch = bch->leader;
    return ach == bch;
}

/*
 * this function sends raw argument over the AUCTION: channel
 * I am not too sure if this method is right..
 */

void talk_auction (char *argument, bool chan_name )
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *original;

if ( chan_name )
    sprintf (buf,"&Y[&BAUCTION&Y]&W %s", argument); /* last %s to reset color */
else
    sprintf (buf,"          &W%s", argument);  

    for (d = first_descriptor; d; d = d->next)
    {
        original = d->original ? d->original : d->character; /* if switched */
        if ((d->connected == CON_PLAYING) && !IS_SET(original->pcdata->deaf,CHANNEL_AUCTION) 
        && !IS_SET(original->in_room->room_flags, ROOM_SILENCE) && !NOT_AUTHED(original))
            act( AT_GOSSIP, buf, original, NULL, NULL, TO_CHAR );
    }
}

/*
 * Language support functions. -- Altrag
 * 07/01/96
 */
bool knows_language( CHAR_DATA *ch, int language, CHAR_DATA *cch )
{
	sh_int sn;

	if ( !IS_NPC(ch) && IS_IMMORTAL(ch) )
		return TRUE;
	if ( IS_NPC(ch) && !ch->speaks ) /* No langs = knows all for npcs */
		return TRUE;
	if ( IS_NPC(ch) && IS_SET(ch->speaks, (language & ~LANG_CLAN)) )
		return TRUE;
	/* everyone KNOWS common tongue */
	if ( IS_SET(language, LANG_COMMON) )
		return TRUE;
	if ( language & LANG_CLAN )
	{
		/* Clan = common for mobs.. snicker.. -- Altrag */
		if ( IS_NPC(ch) || IS_NPC(cch) )
			return TRUE;
		if ( ch->pcdata->clan == cch->pcdata->clan &&
			 ch->pcdata->clan != NULL )
			return TRUE;
	}
	if ( !IS_NPC( ch ) )
	{
	    int lang;
	    
		/* Racial languages for PCs */
	    if ( IS_SET(race_table[ch->race]->language, language) )
	    	return TRUE;

	    for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
	      if ( IS_SET(language, lang_array[lang]) &&
	      	   IS_SET(ch->speaks, lang_array[lang]) )
	      {
		  if ( (sn = skill_lookup(lang_names[lang])) != -1
		  &&    ch->pcdata->learned[sn] >= 60 )
		    return TRUE;
	      }
	}
	return FALSE;
}

bool can_learn_lang( CHAR_DATA *ch, int language )
{
	if ( language & LANG_CLAN )
		return FALSE;
	if ( IS_NPC(ch) || IS_IMMORTAL(ch) )
		return FALSE;
	if ( race_table[ch->race]->language & language )
		return FALSE;
	if ( ch->speaks & language )
	{
		int lang;
		
		for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
			if ( language & lang_array[lang] )
			{
				int sn;
				
				if ( !(VALID_LANGS & lang_array[lang]) )
					return FALSE;
				if ( ( sn = skill_lookup( lang_names[lang] ) ) < 0 )
				{
					bug( "Can_learn_lang: valid language without sn: %d", lang );
					continue;
				}
				if ( ch->pcdata->learned[sn] >= 99 )
					return FALSE;
			}
	}
	if ( VALID_LANGS & language )
		return TRUE;
	return FALSE;
}

int const lang_array[] = { LANG_COMMON, LANG_ELVEN, LANG_DWARVEN, LANG_PIXIE,
	       LANG_OGRE, LANG_ORCISH, LANG_TROLLISH, LANG_LEPRACHAUN,
   	       LANG_INSECTOID, LANG_MAMMAL, LANG_MRRSHAN, LANG_DRAGON,
               LANG_SPIRITUAL, LANG_MAGICAL, LANG_GOBLIN, LANG_GOD, 
               LANG_ANCIENT, LANG_HALFLING, LANG_CLAN, LANG_GITH,
               LANG_VAMPIRIC, LANG_WOLFISH, LANG_DRACONIAN, LANG_SATYR,
               LANG_WRAITH, LANG_CENTAUR, LANG_DRIDER, LANG_DROWISH,
               LANG_MINOTAUR, LANG_CYCLOPS, LANG_AREWYNDEL, LANG_BROWNIE, 
               LANG_UNKNOWN };
/* Note: does not count racial language.  This is intentional (for now). */
int countlangs( int languages )
{
	int numlangs = 0;
	int looper;

	for ( looper = 0; lang_array[looper] != LANG_UNKNOWN; looper++ )
	{
		if ( lang_array[looper] == LANG_CLAN )
			continue;
		if ( languages & lang_array[looper] )
			numlangs++;
	}
	return numlangs;
}

char * const lang_names[] = { "common", "elvish", "dwarven", "pixie", "ogre",
                              "orcish", "trollese", "rodent", "insectoid",
			      "mammal", "reptile", "dragon", "spiritual",
			      "magical", "goblin", "god", "ancient",
			      "halfling", "clan", "gith", "vampiric",
                              "wolfish", "draconian","satyr","wraith",
                              "centaur", "drider", "drowish","minotaur",
                              "cyclops", "arewyndel", "brownie"
};
void do_speak( CHAR_DATA *ch, char *argument )
{
	int langs;
	char arg[MAX_INPUT_LENGTH];
	
	argument = one_argument(argument, arg );
	
	if ( !str_cmp( arg, "all" ) && IS_IMMORTAL( ch ) )
	{
		set_char_color( AT_SAY, ch );
		ch->speaking = ~LANG_CLAN;
		send_to_char( "Now speaking all languages.\n\r", ch );
		return;
	}
	for ( langs = 0; lang_array[langs] != LANG_UNKNOWN; langs++ )
		if ( !str_prefix( arg, lang_names[langs] ) )
			if ( knows_language( ch, lang_array[langs], ch ) )
			{
				if ( lang_array[langs] == LANG_CLAN &&
					(IS_NPC(ch) || !ch->pcdata->clan) )
					continue;
				ch->speaking = lang_array[langs];
				set_char_color( AT_SAY, ch );
				ch_printf( ch, "You now speak %s.\n\r", lang_names[langs] );
				return;
			}
	set_char_color( AT_SAY, ch );
	send_to_char( "You do not know that language.\n\r", ch );
}

void do_languages( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	int lang;
	
	argument = one_argument( argument, arg );
	if ( arg[0] != '\0' && !str_prefix( arg, "learn" ) &&
		!IS_IMMORTAL(ch) && !IS_NPC(ch) )
	{
		CHAR_DATA *sch;
		char arg2[MAX_INPUT_LENGTH];
		int sn;
		int prct;
		int prac;
		
		argument = one_argument( argument, arg2 );
		if ( arg2[0] == '\0' )
		{
			send_to_char( "Learn which language?\n\r", ch );
			return;
		}
		for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
		{
			if ( lang_array[lang] == LANG_CLAN )
				continue;
			if ( !str_prefix( arg2, lang_names[lang] ) )
				break;
		}
		if ( lang_array[lang] == LANG_UNKNOWN )
		{
			send_to_char( "That is not a language.\n\r", ch );
			return;
		}
		if ( !(VALID_LANGS & lang_array[lang]) )
		{
			send_to_char( "You may not learn that language.\n\r", ch );
			return;
		}
		if ( ( sn = skill_lookup( lang_names[lang] ) ) < 0 )
		{
			send_to_char( "That is not a language.\n\r", ch );
			return;
		}
		if ( race_table[ch->race]->language & lang_array[lang] ||
			 lang_array[lang] == LANG_COMMON ||
			 ch->pcdata->learned[sn] >= 99 )
		{
			act( AT_PLAIN, "You are already fluent in $t.", ch,
				 lang_names[lang], NULL, TO_CHAR );
			return;
		}
		for ( sch = ch->in_room->first_person; sch; sch = sch->next )
			if ( IS_NPC(sch) && IS_SET(sch->act, ACT_SCHOLAR) &&
					knows_language( sch, ch->speaking, ch ) &&
					knows_language( sch, lang_array[lang], sch ) &&
					(!sch->speaking || knows_language( ch, sch->speaking, sch )) )
				break;
		if ( !sch )
		{
			send_to_char( "There is no one who can teach that language here.\n\r", ch );
			return;
		}
		if ( countlangs(ch->speaks) >= (ch->level / 10) &&
			 ch->pcdata->learned[sn] <= 0 )
		{
			act( AT_TELL, "$n tells you 'You may not learn a new language yet.'",
				 sch, NULL, ch, TO_VICT );
			return;
		}
		/* 0..16 cha = 2 pracs, 17..25 = 1 prac. -- Altrag */
		prac = 2 - (get_curr_cha(ch) / 17);
		if ( ch->practice < prac )
		{
			act( AT_TELL, "$n tells you 'You do not have enough practices.'",
				 sch, NULL, ch, TO_VICT );
			return;
		}
		ch->practice -= prac;
		/* Max 12% (5 + 4 + 3) at 24+ int and 21+ wis. -- Altrag */
		prct = 5 + (get_curr_int(ch) / 6) + (get_curr_wis(ch) / 7);
		ch->pcdata->learned[sn] += prct;
		ch->pcdata->learned[sn] = UMIN(ch->pcdata->learned[sn], 99);
		SET_BIT( ch->speaks, lang_array[lang] );
		if ( ch->pcdata->learned[sn] == prct )
			act( AT_PLAIN, "You begin lessons in $t.", ch, lang_names[lang],
				 NULL, TO_CHAR );
		else if ( ch->pcdata->learned[sn] < 60 )
			act( AT_PLAIN, "You continue lessons in $t.", ch, lang_names[lang],
				 NULL, TO_CHAR );
		else if ( ch->pcdata->learned[sn] < 60 + prct )
			act( AT_PLAIN, "You feel you can start communicating in $t.", ch,
				 lang_names[lang], NULL, TO_CHAR );
		else if ( ch->pcdata->learned[sn] < 99 )
			act( AT_PLAIN, "You become more fluent in $t.", ch,
				 lang_names[lang], NULL, TO_CHAR );
		else
			act( AT_PLAIN, "You now speak perfect $t.", ch, lang_names[lang],
				 NULL, TO_CHAR );
		return;
	}
	for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
		if ( knows_language( ch, lang_array[lang], ch ) )
		{
			if ( ch->speaking & lang_array[lang] ||
				(IS_NPC(ch) && !ch->speaking) )
				set_char_color( AT_RED, ch );
			else
				set_char_color( AT_SAY, ch );
			send_to_char( lang_names[lang], ch );
			send_to_char( " ", ch );
		}
	send_to_char( "\n\r", ch );
	return;
}

void do_wartalk( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }
    talk_channel( ch, argument, CHANNEL_WARTALK, "war" );
    return;
}

/*
 * Many thanks to Altrag who contributed this function! --GW
 */
char *smash_curses(const char *ip)
{
  static struct
  {
    const char *text;
    const char *html;
    int tlen, hlen;
  } convert_table[] =
  { { "fuck", "love" }, { "shit", "doodoo" }, { "bitch", "female-doggie" }, 
    { "bastard", "fatherless-person" }, { "ass", "buttock" },
    { "asshole", "bumhole" }, {"motherfucker", "mommy-lover" },
    {"cock", "weewee"}, { "cocksucker", "weewee-licker"}, { NULL, NULL } 
  };

  static char buf[MAX_STRING_LENGTH*2];/* Safety here .. --GW */
  char *bp = buf;
  int i;

  if (!convert_table[0].tlen)
  {
    for (i = 0; convert_table[i].text; ++i)
    {
      convert_table[i].tlen = strlen(convert_table[i].text);
      convert_table[i].hlen = strlen(convert_table[i].html);
    }
  }
  while (*ip)
  {
    for (i = 0; convert_table[i].text; ++i)
      if (!strncmp(ip, convert_table[i].text, convert_table[i].tlen))
        break;
    if (convert_table[i].text)
    {
      strcpy(bp, convert_table[i].html);
      bp += convert_table[i].hlen;
      ip += convert_table[i].tlen;
    }
    else
      *bp++ = *ip++;
  }
  *bp = '\0';
  return buf;
}

