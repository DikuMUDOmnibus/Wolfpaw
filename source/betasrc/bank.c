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
 *		Player Banking Module					    *
 *--------------------------------------------------------------------------*/

#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mud.h"

char *currency_string( int currency );

void do_balance ( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    record_call( "<do_balance>" );

    if (IS_NPC(ch))
	return;

    sprintf( buf, "You have %ld coins in the bank.\n\r", ch->pcdata->bank );
    send_to_char( buf, ch );
    return;
}

void do_deposit ( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *banker;
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int amnt;
    int currency;
    record_call( "<do_deposit>" );

    if (IS_NPC(ch))
	return;

    if (!IS_SET(ch->in_room->room_flags, ROOM_BANK) ) 
    {
	send_to_char(  "But you are not in a bank.\n\r", ch  );
	return;
    }

    banker = NULL;
    for ( banker = ch->in_room->first_person; banker; banker = banker->next_in_room )
    {
	if ( IS_NPC( banker ) && IS_SET(banker->pIndexData->act, ACT_BANKER) )
	    break;
    }

    if ( !banker )
    {
	send_to_char( "The banker is currently not available.\n\r",ch );
	return;
    }
 
    argument=one_argument( argument, arg );
    argument=one_argument( argument, arg2 );

    if ( arg[0] == '\0' )
    {
	send_to_char( "How much gold do you wish to deposit?\n\r", ch );
	return;
    }

    amnt = atoi( arg );

    /* Currency */
    switch(arg2[0])
    {
      /* Copper */
      case 'C':
      case 'c':
		currency=COPPER;
		break;

      /* Silver */
      case 'S':
      case 's':
		currency=SILVER;
		break;

      /* Gold */
      case 'G':
      case 'g':
		currency=GOLD;
		break;
      default:
		send_to_char("Syntax: deposit [amount] [currency]\n\r",ch);
		return;
    }
    

    if ( (currency==COPPER && amnt >= (ch->gold + 1) ) ||
         (currency==SILVER && amnt >= (ch->silver + 1) ) ||
         (currency==GOLD && amnt >= (ch->real_gold + 1) ) )
    {
	sprintf( buf, "%s, you do not have %d %s coins.", ch->name, amnt, currency_string(currency));
	do_say( banker, buf );
	return;
    }

    if ( amnt < 0 )
    {
        send_to_char( "Uh... No\n\r", ch );
        return;
    }

    if ( (currency==COPPER && (amnt+ch->pcdata->bank) > 2100000000) ||
         (currency==SILVER && (amnt+ch->pcdata->silver_bank) > 2100000000) ||
         (currency==GOLD && (amnt+ch->pcdata->gold_bank) > 2100000000))
    {
        send_to_char("You cant only deposit 2,100,000,000 coins!\n\r",ch);
	return;
    }

 switch(currency)
 {
   case COPPER:
    ch->pcdata->bank += amnt;
    ch->gold -= amnt;
    sprintf( buf, "%s, your account now contains: %ld %s coins,",ch->name,ch->pcdata->bank,currency_string(currency) );
    break;
   case SILVER:
    ch->pcdata->silver_bank += amnt;
    ch->silver -= amnt;
    sprintf( buf, "%s, your account now contains: %ld %s coins,",ch->name,ch->pcdata->silver_bank,currency_string(currency) );
    break;
   case GOLD:
    ch->pcdata->gold_bank += amnt;
    ch->real_gold -= amnt;
    sprintf( buf, "%s, your account now contains: %ld %s coins,",ch->name,ch->pcdata->gold_bank,currency_string(currency) );
    break;
 }

    do_say( banker, buf );
    sprintf( buf, "after depositing: %d %s coins.", amnt, currency_string(currency) );
    do_say( banker, buf );
    return;
}

void do_withdraw ( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *banker;
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int amnt,currency=-1;
    record_call( "<do_withdraw>" );

    if (IS_NPC(ch))
	return;

    if (!IS_SET(ch->in_room->room_flags, ROOM_BANK) ) 
    {
	send_to_char( "But you are not in a bank.\n\r", ch );
	send_to_char( buf, ch );
	return;
    }

    banker = NULL;
    for ( banker = ch->in_room->first_person; banker; banker = banker->next_in_room )
    {
	if ( IS_NPC( banker ) && IS_SET(banker->pIndexData->act, ACT_BANKER) )
	    break;
    }

    if ( !banker )
    {
	 send_to_char( "The banker is currently not available.\n\r", ch);
	 send_to_char( buf, ch );
	 return;
    }
 
    argument=one_argument( argument, arg );
    argument=one_argument( argument, arg2 );

    if ( arg[0] == '\0' )
    {
	 send_to_char( "How much gold do you wish to withdraw?\n\r", ch);
	 send_to_char( buf, ch );
	 return;
    }

    amnt = atoi( arg );
    switch(arg2[0])
    {
      case 'G':
      case 'g':
	currency=GOLD;
        break;
      case 'S':
      case 's':
	currency=SILVER;
	break;
      case 'C':
      case 'c':
	currency=COPPER;
	break;
      default:
	send_to_char("Syntax: withdraw [coins] [currency]\n\r",ch);
	return;
    }

    if ( (currency==COPPER && amnt >= (ch->pcdata->bank + 1) ) ||
	 (currency==SILVER && amnt >= (ch->pcdata->silver_bank + 1) ) ||
	 (currency==GOLD && amnt >= (ch->pcdata->gold_bank + 1) ) )
     {
	sprintf( buf, "%s, you do not have %d %s coins in the bank.", ch->name, amnt,currency_string(currency) );
	do_say( banker, buf );
	return;
    }

    if ( amnt < 0 )
    {
	send_to_char( "Uh... No\n\r", ch );
        return;
    }

  switch(currency)
  {
   case COPPER:
    ch->gold += amnt;
    ch->pcdata->bank -= amnt;
    sprintf( buf, "%s, your account now contains %ld %s coins,",ch->name,ch->pcdata->bank,currency_string(currency) );
    break;
   case SILVER:
    ch->silver += amnt;
    ch->pcdata->silver_bank -= amnt;
    sprintf( buf, "%s, your account now contains %ld %s coins,",ch->name,ch->pcdata->silver_bank,currency_string(currency) );
    break;
   case GOLD:
    ch->real_gold += amnt;
    ch->pcdata->gold_bank -= amnt;
    sprintf( buf, "%s, your account now contains %ld %s coins,",ch->name,ch->pcdata->gold_bank,currency_string(currency) );
    break;
  }

    do_say( banker, buf );
    sprintf( buf, "after withdrawing %d %s coins.",amnt,currency_string(currency) );
    do_say( banker, buf );
    return;
}



/* Shares Stuff --GW */

/* Share Types */
#define SHARE_SCROLL_SOURCE	1
#define SHARE_ARMOURY		2
#define SHARE_WEAPONSMITH	3

/*
 * Each boot has a random share price between 20 - 60 Million each
 */
void setup_shares( void )
{
char buf[MSL];

SHARE_STATUS = TRUE;
log_string("Shares Activated.\n\r");
SHARE_COST = number_range(20000000,35000000);
sprintf(buf,"Share cost for this boot is: %ld.",SHARE_COST);
log_string(buf);
}

int get_total_levels( CHAR_DATA *ch )
{
if ( !IS_ADVANCED(ch) && ch->class2 == -1 ) /* Single */
return ch->level;
else if ( !IS_ADVANCED(ch) ) /* Dual */
return ( ch->level + ch->level2 );
else if ( IS_ADVANCED(ch) ) /* Advanced */
return ( ch->level + ch->level2 + ch->advlevel );
else /* Quad */
return ( ch->level + ch->level2 + ch->advlevel + ch->advlevel2 );
}

/* Main Interface 
 * Syn: Share <buy/sell/transfer/list> <scroll/weapon/armor> <char>
 */
void do_share( CHAR_DATA *ch, char *argument )
{
char arg1[MIL];
char arg2[MIL];
char arg3[MIL];
char arg4[MIL];
sh_int share_type=0;
int amnt=0;
CHAR_DATA *banker, *victim;
long cost=0;
char buf[MSL];

argument = one_argument( argument, arg1 );
argument = one_argument( argument, arg2 );
argument = one_argument( argument, arg3 );
argument = one_argument( argument, arg4 );

if ( IS_NPC(ch) )
  return;

    if (!IS_SET(ch->in_room->room_flags, ROOM_BANK) ) 
    {
	send_to_char(  "But you are not in a bank.\n\r", ch  );
	return;
    }

    banker = NULL;
    for ( banker = ch->in_room->first_person; banker; banker = banker->next_in_room )
    {
	if ( IS_NPC( banker ) && IS_SET(banker->pIndexData->act, ACT_BANKER) )
	    break;
    }

    if ( !banker )
    {
	send_to_char( "The banker is currently not available.\n\r",ch );
	return;
    }
 
if ( arg1[0] == '\0' )
{
   send_to_char("Syntax: Share <buy/sell/transfer/list> <scroll/weapon/armor> <#> <char>\n\r",ch);
   return;
}

if ( !str_cmp( arg1, "buy" ) )
{

if ( arg2[0] == '\0' || arg3[0] == '\0' || !is_number(arg3) )
{
   send_to_char("Syntax: Share <buy/sell/transfer/list> <scroll/weapon/armor> <#> <char>\n\r",ch);
   return;
}

/* Get the share type */ 
if ( !str_cmp( arg2, "scroll" ) )  
share_type = SHARE_SCROLL_SOURCE;  
else if ( !str_cmp( arg2, "weapon" ) )  
share_type = SHARE_WEAPONSMITH;  
else if ( !str_cmp( arg2, "armor" ) )  
share_type = SHARE_ARMOURY; 

if ( share_type == 0 )
{
  send_to_char("What kind of share?\n\r",ch);
  return;
}

amnt = atoi(arg3);

/* How many??*/
if ( ( (  ch->pcdata->shares[SHARE_ARMOURY] +
     ch->pcdata->shares[SHARE_WEAPONSMITH] +
     ch->pcdata->shares[SHARE_SCROLL_SOURCE] )+amnt) > (get_total_levels(ch)/2))
{
  ch_printf( ch, "You may only buy between 1 and %d shares!\n\r",(get_total_levels(ch)/2));
  return;
}

/* Make the cost double at 1 quad - Gorbash's Request --GW */
if ( get_total_levels(ch) < 151 )
cost = (amnt * SHARE_COST);
else
cost = (amnt * (SHARE_COST * 2));

if ( ch->gold < cost )
{
  send_to_char("You havent got enough gold for that!\n\r",ch);
  return;
}

ch->gold -= cost;
ch->pcdata->shares[share_type] += amnt;
sprintf(buf,"You purchase %d %s shares.\n\r",amnt,arg2);
send_to_char(buf,ch);
save_char_obj(ch);
return;
}

if ( !str_cmp( arg1, "sell" ) )
{

if ( arg2[0] == '\0' || arg3[0] == '\0' || !is_number(arg3))
{
   send_to_char("Syntax: Share <buy/sell/transfer/list> <scroll/weapon/armor> <#> <char>\n\r",ch);
   return;
}

/* Get the share type */ 
if ( !str_cmp( arg2, "scroll" ) )  
share_type = SHARE_SCROLL_SOURCE;  
else if ( !str_cmp( arg2, "weapon" ) )  
share_type = SHARE_WEAPONSMITH;  
else if ( !str_cmp( arg2, "armor" ) )  
share_type = SHARE_ARMOURY; 

if ( share_type == 0 )
{
  send_to_char("What kind of share?\n\r",ch);
  return;
}

amnt = atoi(arg3);

if ( amnt < 1 )
{
  send_to_char("You must sell at least 1 share!\n\r",ch);
  return;
}

if ( ch->pcdata->shares[share_type] < amnt )
{
  send_to_char("You donot have that many!\n\r",ch);
  return;
}

if ( get_total_levels(ch) < 151 )
{
if ( ( (amnt * SHARE_COST) + ch->gold) > 2100000000 )
{
  send_to_char("You can only carry 2.1 Billion coins!\n\r",ch);
  return;  
}
}
else
{
if ( ( (amnt * (SHARE_COST * 2)) + ch->gold) > 2100000000 )
{
  send_to_char("You can only carry 2.1 Billion coins!\n\r",ch);
  return;  
}
}

ch->pcdata->shares[share_type] -= amnt;

if ( get_total_levels(ch) < 151 )
cost = (amnt * SHARE_COST);
else
cost = (amnt * (SHARE_COST * 2));

ch->gold += cost;

sprintf(buf,"You sell %d shares of %s to the bank.\n\r",amnt,arg2);
send_to_char(buf,ch);
sprintf(buf,"You are paid %ld gold.\n\r",cost);
send_to_char(buf,ch);
save_char_obj(ch);
return;
}

if ( !str_cmp( arg1, "transfer" ) )
{

if ( arg2[0] == '\0' || arg3[0] == '\0' || !is_number(arg3) || arg4[0] == '\0' )
{
   send_to_char("Syntax: Share <buy/sell/transfer/list> <scroll/weapon/armor> <#> <char>\n\r",ch);
   return;
}

if ( ( victim = get_char_room( ch, arg4 ) )==NULL)
{
  send_to_char("I don't see that person here.\n\r",ch);
  return;
}

if ( IS_NPC(victim) )
{
  send_to_char("You cannot transfer shares to mobiles!\n\r",ch);
  return;
}

amnt = atoi(arg3);

if ( amnt < 1 )
{
  send_to_char("You must transfer at least 1 share!\n\r",ch);
  return;
}

if ( !str_cmp( arg2, "scroll" ) )  
share_type = SHARE_SCROLL_SOURCE;  
else if ( !str_cmp( arg2, "weapon" ) )  
share_type = SHARE_WEAPONSMITH;  
else if ( !str_cmp( arg2, "armor" ) )  
share_type = SHARE_ARMOURY; 

if ( share_type == 0 )
{
  send_to_char("What kind of share?\n\r",ch);
  return;
}

if ( (  victim->pcdata->shares[SHARE_ARMOURY] +
     victim->pcdata->shares[SHARE_WEAPONSMITH] +
     victim->pcdata->shares[SHARE_SCROLL_SOURCE] + amnt ) > (get_total_levels(victim)/2))
{
  send_to_char("Sorry, that would put that player over thier share limit!\n\r",ch);
  return;
}

ch->pcdata->shares[share_type] -= amnt;
victim->pcdata->shares[share_type] += amnt;
sprintf(buf,"You transfer %d %s shares to %s.\n\r",amnt,arg2,capitalize(victim->name));
send_to_char(buf,ch);
sprintf(buf,"%s has transfered %d shares of %s to you.\n\r",capitalize(ch->name),amnt,arg2);
send_to_char(buf,ch);
save_char_obj(ch);
save_char_obj(victim);
return;
}

if ( !str_cmp( arg1, "list" ) )
{
send_to_char("Shares Listing\n\r",ch);
send_to_char("==============\n\r",ch);
ch_printf(ch,"Armoury Shares:        %d\n\r",ch->pcdata->shares[SHARE_ARMOURY]);
ch_printf(ch,"Weapon Shop Shares:    %d\n\r",ch->pcdata->shares[SHARE_WEAPONSMITH]);
ch_printf(ch,"Scroll Source Shares:  %d\n\r",ch->pcdata->shares[SHARE_SCROLL_SOURCE]);
ch_printf(ch,"               Total:  %d\n\r",(ch->pcdata->shares[SHARE_ARMOURY] +
		ch->pcdata->shares[SHARE_WEAPONSMITH] + ch->pcdata->shares[SHARE_SCROLL_SOURCE]));
ch_printf(ch,"  Max Shares Allowed:  %d\n\r",(get_total_levels(ch)/2));

if ( get_total_levels(ch) < 151 )
ch_printf(ch,"Share Value this boot: %ld\n\r",SHARE_COST);
else
ch_printf(ch,"Share Value this boot: %ld\n\r",(SHARE_COST*2));
return;
}

send_to_char("Syntax: Share <buy/sell/transfer/list> <scroll/weapon/armor> <#> <char>\n\r",ch);
return;
}

/* Add the ac from shares */
int add_share_ac( CHAR_DATA *ch )
{
if ( IS_NPC(ch) )
return 0;

return ch->pcdata->shares[SHARE_ARMOURY];
}

/* Add the MAC from shares */
int add_share_mac( CHAR_DATA *ch )
{
if ( IS_NPC(ch) )
return 0;

return ch->pcdata->shares[SHARE_SCROLL_SOURCE];
}

/* Add the Hit'n Dam from shares */
int add_share_hitndam( CHAR_DATA *ch )
{
if ( IS_NPC(ch) )
return 0;

return ch->pcdata->shares[SHARE_WEAPONSMITH];
}

/* WME Currency System --GW Aug 31/99 */

/* Currency Exchange Rate --GW */
#define EXCHANGE_RATE				1000

/* Minimum Trans --GW */
#define EXCH_MIN_ALL			1

/* Maximums --GW*/
/* These are the Maximum amount that can be exchanged certain ways, DONOT CHANGE! --GW */
#define EXCH_MAX_COPPER_SILVER		2000000000
#define EXCH_MAX_COPPER_GOLD		2000000000
#define EXCH_MAX_SILVER_COPPER		2000000
#define EXCH_MAX_SILVER_GOLD		2000000000
#define EXCH_MAX_GOLD_COPPER		2000
#define EXCH_MAX_GOLD_SILVER		2000000

/*
 * Convert Currency --GW
 */

/* Up */

long currency_copper_to_silver( long total )
{
return total / EXCHANGE_RATE;
}
long currency_copper_to_gold( long total )
{
long amnt=0;
amnt=(total / EXCHANGE_RATE); /*to Silver */
amnt=(amnt / EXCHANGE_RATE); /*to Gold */
return amnt;
}
long currency_silver_to_gold( long total )
{
return total / EXCHANGE_RATE;
}

/* Down */

long currency_silver_to_copper( long total )
{
long amnt=0;
amnt = total * EXCHANGE_RATE;
return amnt;
}
long currency_gold_to_copper( long total )
{
long amnt=0;

amnt=(total * EXCHANGE_RATE);
amnt=(amnt * EXCHANGE_RATE);
return amnt;
}
long currency_gold_to_silver( long total )
{
return total * EXCHANGE_RATE;
}

void do_exchange( CHAR_DATA *ch, char *argument )
{
  long old_coin=0;
  long new_coin=0;
  int old_type=0;
  int new_type=0;
  char arg1[MIL]; /* How much Orig */
  char arg2[MIL]; /* What Currency */
  char arg3[MIL]; /* To what Currency */
  char buf[MSL]; /* buffer */
  bool die=FALSE;

  argument=one_argument(argument,arg1);
  argument=one_argument(argument,arg2);
  argument=one_argument(argument,arg3);

  if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
  {
    send_to_char("Syntax: exchange [amount] [currency] [new currency]\n\r",ch);
    return;
  }

  old_coin = atoi(arg1); /* How much */

  /* Get the Currency or Orig */  
  switch(LOWER(arg2[0]))
  {
    case 'c':
	old_type = COPPER;
	break;
    case 's':
	old_type = SILVER;
	break;
    case 'g':
	old_type = GOLD;
	break;
    default:
	send_to_char("Valid Currency types are Copper, Silver, or Gold.\n\r",ch);
	die=TRUE;
	break;
  }


   if ( die )
	return;

  /* Get the type to convert to */
  switch(LOWER(arg3[0]))
  {
    case 'c':
	new_type = COPPER;
	break;
    case 's':
	new_type = SILVER;
	break;
    case 'g':
	new_type = GOLD;
	break;
    default:
	send_to_char("Valid Currency types are Copper, Silver, or Gold.\n\r",ch);
	die=TRUE;
	break;
  }

   if ( die ) 
	return;

   if ( old_type == new_type )
   {
	send_to_char("You cannot convert to the same Currency you started with!\n\r",ch);
	return;
   }

   if ( old_coin < 1 || old_coin > 2100000000 )
   {
	send_to_char("Come on now, thats rather stupid.\n\r",ch);
	return;
   }

   /* Do we have the Amount? */
   if ( ( old_type == COPPER && ( ch->gold < old_coin ) ) ||
	( old_type == SILVER && ( ch->silver < old_coin ) ) ||
	( old_type == GOLD && ( ch->real_gold < old_coin ) ) )
	{
	   send_to_char("But you donot have that much to exchange!\n\r",ch);
	   return;
	}

/* Max's and Min's --GW */
switch(old_type)
{
  case COPPER:
    switch(new_type)
    {
	case SILVER:
	  if ( old_coin < EXCH_MIN_ALL || old_coin > EXCH_MAX_COPPER_SILVER )
	  {
	   sprintf(buf,"Transaction range from %s, to %s is %d to %d\n\r",currency_string(old_type),
		currency_string(new_type),EXCH_MIN_ALL,EXCH_MAX_COPPER_SILVER);
	   send_to_char(buf,ch);
	   return;
	  }
	  break;
	case GOLD:
	  if ( old_coin < EXCH_MIN_ALL || old_coin > EXCH_MAX_COPPER_GOLD )
	  {
	   sprintf(buf,"Transaction range from %s, to %s is %d to %d\n\r",currency_string(old_type),
		currency_string(new_type),EXCH_MIN_ALL,EXCH_MAX_COPPER_GOLD);
	   send_to_char(buf,ch);
	   return;
	  }
	  break;
    }
    break;
  case SILVER:
    switch(new_type)
    {
	case COPPER:
	  if ( old_coin < EXCH_MIN_ALL || old_coin > EXCH_MAX_SILVER_COPPER )
	  {
	   sprintf(buf,"Transaction range from %s, to %s is %d to %d\n\r",currency_string(old_type),
		currency_string(new_type),EXCH_MIN_ALL,EXCH_MAX_SILVER_COPPER);
	   send_to_char(buf,ch);
	   return;
	  }
	  break;
	case GOLD:
	  if ( old_coin < EXCH_MIN_ALL || old_coin > EXCH_MAX_SILVER_GOLD )
	  {
	   sprintf(buf,"Transaction range from %s, to %s is %d to %d\n\r",currency_string(old_type),
		currency_string(new_type),EXCH_MIN_ALL,EXCH_MAX_SILVER_GOLD);
	   send_to_char(buf,ch);
	   return;
	  }
	  break;
    }
    break;
  case GOLD:
    switch(new_type)
    {
	case COPPER:
	  if ( old_coin < EXCH_MIN_ALL || old_coin > EXCH_MAX_GOLD_COPPER )
	  {
	   sprintf(buf,"Transaction range from %s, to %s is %d to %d\n\r",currency_string(old_type),
		currency_string(new_type),EXCH_MIN_ALL,EXCH_MAX_GOLD_COPPER);
	   send_to_char(buf,ch);
	   return;
	  }
	  break;
	case SILVER:
	  if ( old_coin < EXCH_MIN_ALL || old_coin > EXCH_MAX_GOLD_SILVER )
	  {
	   sprintf(buf,"Transaction range from %s, to %s is %d to %d\n\r",currency_string(old_type),
		currency_string(new_type),EXCH_MIN_ALL,EXCH_MAX_GOLD_SILVER);
	   send_to_char(buf,ch);
	   return;
	  }
	  break;
    }
    break;
}

   /* Convert */
   if ( old_type == COPPER )
   {
	switch( new_type )
	{
	  case SILVER:
	    new_coin = currency_copper_to_silver(old_coin);
	    if ( new_coin > 2100000000 || new_coin < 1 )
	    {
	      send_to_char("Sorry, the allowed transaction range is 1 to 2,100,000\n\r",ch);
	      return;
	    }
	    if ( (ch->silver + new_coin) <  0 || (ch->silver + new_coin) > 2100000000 )
	    {
	       send_to_char("That would roll your Silver negative, try again.\n\r",ch);
	       return;
	    }
	    ch_printf(ch,"You purchase %ld Silver, at a price of %ld Copper.\n\r",new_coin,old_coin);
	    ch->gold -= old_coin;
	    ch->silver += new_coin; 
	    break;
	  case GOLD:
	    new_coin = currency_copper_to_gold(old_coin);
	    if ( new_coin > 2100000000 || new_coin < 1 )
	    {
	      send_to_char("Sorry, the allowed transaction range is 1 to 2,100,000\n\r",ch);
	      return;
	    }
	    if ( (ch->real_gold + new_coin) <  0 || (ch->real_gold  + new_coin) > 2100000000 )
	    {
	       send_to_char("That would roll your Gold negative, try again.\n\r",ch);
	       return;
	    }
	    ch_printf(ch,"You purchase %ld Gold, at a price of %ld Copper.\n\r",new_coin,old_coin);
	    ch->gold -= old_coin;
	    ch->real_gold += new_coin; 
	    break;
	}
   }

   if ( old_type == SILVER )
   {
	switch( new_type )
	{
	  case GOLD:
	    new_coin = currency_silver_to_gold(old_coin);
	    if ( new_coin > 2100000000 || new_coin < 1 )
	    {
	      send_to_char("Sorry, the allowed transaction range is 1 to 2,100,000\n\r",ch);
	      return;
	    }
	    if ( (ch->real_gold + new_coin) <  0 || (ch->real_gold + new_coin) > 2100000000 )
	    {
	       send_to_char("That would roll your Gold negative, try again.\n\r",ch);
	       return;
	    }
	    ch_printf(ch,"You purchase %ld Gold, at a price of %ld Silver.\n\r",new_coin,old_coin);
	    ch->silver -= old_coin;
	    ch->real_gold += new_coin; 
	    break;
	  case COPPER:
	    new_coin = currency_silver_to_copper(old_coin);
	    if ( new_coin > 2100000000 || new_coin < 1 )
	    {
	      send_to_char("Sorry, the allowed transaction range is 1 to 2,100,000\n\r",ch);
	      return;
	    }
	    if ( (ch->gold + new_coin) <  0 || (ch->gold + new_coin) > 2100000000 )
	    {
	       send_to_char("That would roll your Copper negative, try again.\n\r",ch);
	       return;
	    }
	    ch_printf(ch,"You purchase %ld Copper, at a price of %ld Silver.\n\r",new_coin,old_coin);
	    ch->silver -= old_coin;
	    ch->gold += new_coin; 
	    break;
	}
   }

   if ( old_type == GOLD )
   {
	switch( new_type )
	{
	  case COPPER:
	    new_coin = currency_gold_to_copper(old_coin);
	    if ( new_coin > 2100000000 || new_coin < 1 )
	    {
	      send_to_char("Sorry, the allowed transaction range is 1 to 2,100,000\n\r",ch);
	      return;
	    }
	    if ( (ch->gold + new_coin) <  0 || (ch->gold + new_coin) > 2100000000 )
	    {
	       send_to_char("That would roll your Silver negative, try again.\n\r",ch);
	       return;
	    }
	    ch_printf(ch,"You purchase %ld Copper, at a price of %ld Gold.\n\r",new_coin,old_coin);
	    ch->real_gold -= old_coin;
	    ch->gold += new_coin; 
	    break;
	  case SILVER:
	    new_coin = currency_gold_to_silver(old_coin);
	    if ( new_coin > 2100000000 || new_coin < 1 )
	    {
	      send_to_char("Sorry, the allowed transaction range is 1 to 2,100,000\n\r",ch);
	      return;
	    }
	    if ( (ch->silver + new_coin) <  0 || (ch->silver + new_coin) > 2100000000 )
	    {
	       send_to_char("That would roll your Silver negative, try again.\n\r",ch);
	       return;
	    }
	    ch_printf(ch,"You purchase %ld Silver, at a price of %ld Gold.\n\r",new_coin,old_coin);
	    ch->real_gold -= old_coin;
	    ch->silver += new_coin; 
	    break;
	}
   }
save_char_obj(ch);
return;
}

char *currency_string( int currency )
{
switch(currency)
{
  case COPPER:
    return "Copper";
  case SILVER:
    return "Silver";
  case GOLD:
    return "Gold";
}

return "<BUG!>";
}

/*
 * Make shop keepers be able to automatically exchange money when things are purchased --GW 
 * TRUE = Exchange took place
 * FALSE = Exchange failed
 */
bool shop_auto_exchange( CHAR_DATA *ch, CHAR_DATA *keeper, long amnt )
{
char buf[MSL];
int change=0;

buf[0] = '\0';

act(AT_PLAIN,"$n gives you a dirty look, as $e opens the till to exchange your currency for you.",keeper,NULL,ch,TO_VICT);

/* Can we exchange this to meet the bill? */
/* Enough Copper to exchange? */
if ( ch->gold > 0 && (ch->gold >= (amnt * EXCHANGE_RATE)))
{
ch->gold -= (amnt * EXCHANGE_RATE);
keeper->gold += (amnt * EXCHANGE_RATE);
sprintf(buf,"%s exchanges %ld Copper for %ld Silver, and your bill is paid.\n\r",keeper->short_descr,
	(amnt * EXCHANGE_RATE),amnt);
send_to_char(buf,ch);
interpret(keeper,"smile");
sprintf(buf,"thanks %s for doing buisness here, and closes the till with a Slam!",capitalize(ch->name));
do_emote(keeper,buf);
return TRUE;
}
/* Enough gold?? */

/* cost is less then the EXCHANGE_RATE for a coin? .. give the bastards change! */
if ( ch->real_gold > 0 && amnt < EXCHANGE_RATE )
{
change=(EXCHANGE_RATE - amnt);
ch->real_gold--;
keeper->real_gold++;
ch->silver += change;
keeper->silver -= change;
sprintf(buf,"%s exchanges 1 Gold for 1000 Silver\n\r",keeper->short_descr);
send_to_char(buf,ch);
sprintf(buf,"%s takes %ld Silver, and gives you %d Silver change.. your bill is paid.\n\r",keeper->short_descr,
		amnt,change);
send_to_char(buf,ch);
interpret(keeper,"mutter");
sprintf(buf,"curses %s for doing buisness here, and closes the till with a Slam!",capitalize(ch->name));
do_emote(keeper,buf);
do_say(keeper,"Bring the right change, or piss off!");
return TRUE;
}

if (ch->real_gold > 0 && (ch->real_gold >= ( amnt / EXCHANGE_RATE )))
{
ch->real_gold -= (amnt / EXCHANGE_RATE);
keeper->real_gold += (amnt / EXCHANGE_RATE);
sprintf(buf,"%s exchanges %ld Gold for %ld Silver, and your bill is paid.\n\r",keeper->short_descr,
	(amnt / EXCHANGE_RATE),amnt);
send_to_char(buf,ch);
interpret(keeper,"smile");
sprintf(buf,"thanks %s for doing buisness here, and closes the till with a Slam!",capitalize(ch->name));
do_emote(keeper,buf);
return TRUE;
}
/* Guess not.. */
interpret(keeper,"snarl");
sprintf(buf,"Bah! Get out of my store %s! Your a poor bastard!\n\r",capitalize(ch->name));
do_say(keeper,buf);
return FALSE;
}

