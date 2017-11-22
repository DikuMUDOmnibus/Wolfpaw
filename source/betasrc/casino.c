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
 *			Casino Module					    *
 *--------------------------------------------------------------------------*/

/* Original SillyMUD Based Code by: J. Hendrickson */

/*
 * Ported to Wolfpaw 2.0 by Greywolf - 5/03/99 
 */

#include <string.h>
#if USE_stdlib
#include <stdlib.h>
#endif
#include <stdio.h>

#include "mud.h"

#define MAX_BLACKJACK 10	/* Max number of Blackjack players */
#define HEARTS 128
#define DIAMONDS 64
#define CLUBS 32
#define SPADES 16

/* forward declarations */
int bj_index ( CHAR_DATA *ch );
void add_suit ( char *cat_msg, int card );
int best_bj_dealer ( int inx );
int best_bj_score ( int inx );
int min_bj_score ( int inx );


struct bj_players {
	char inuse;
	char name[40];
	unsigned char deck[52], hand[12], dealer[12], np, nd;
	int deck_inx;
	int bet;
};

static struct bj_players bj_data[MAX_BLACKJACK];

char *card_names[14] = {
	"Nothing",
	"Ace",
	"Two",
	"Three",
	"Four",
	"Five",
	"Six",
	"Seven",
	"Eight",
	"Nine",
	"Ten",
	"Jack",
	"Queen",
	"King" };

int check_blackjack ( CHAR_DATA *ch )
{

  if ( ch->in_room->vnum == BLACKJACK )
    return 1;
  else
    return 0;

}

void bj_shuffle ( int inx, CHAR_DATA *ch )
{
  int l1, l2, l3, tmp;

  send_to_char ("The dealer shuffles the deck.\r\n", ch );

  for (l1=0;l1<4;l1++)
    for (l2=0;l2<13;l2++)
    {
      l3=l1*13+l2;
      bj_data[inx].deck[l3]=l2+1;
      switch (l1)
      {
      case 0 : bj_data[inx].deck[l3] |= HEARTS;
	break;
      case 1 : bj_data[inx].deck[l3] |= DIAMONDS;
	break;
      case 2 : bj_data[inx].deck[l3] |= CLUBS;
	break;
      case 3 : bj_data[inx].deck[l3] |= SPADES;
	break;
      }
    }

  for ( l1=0;l1<1000;l1++)
  {
    l2=(rand()>>3)%52;
    l3=(rand()>>3)%52;
    tmp=bj_data[inx].deck[l2];
    bj_data[inx].deck[l2]=bj_data[inx].deck[l3];
    bj_data[inx].deck[l3]=tmp;
  }
  bj_data[inx].deck_inx = 0;

}

int do_blackjack_enter ( CHAR_DATA *ch )
{
  int l1, inx;

  for (l1=0,inx=-1;l1<MAX_BLACKJACK;l1++)
  {
    if ( !strcmp(ch->name, bj_data[l1].name) )
    {
      inx=l1;
      send_to_char ( "The dealer grins and says, 'Great, you have returned.'\n\r",ch);
    }
    if ( inx < 0 && !bj_data[l1].inuse )
      inx=l1;
  }
  if ( inx < 0 )
  {
    send_to_char ( "The table seems to be full at the moment.\n\r", ch );
    return 0;
  }

  send_to_char ( "You move up to the blackjack table and pull up a chair.\n\r", ch );
  bj_data[inx].inuse = 1;
  strcpy ( bj_data[inx].name, ch->name );
  bj_shuffle ( inx, ch );
  bj_data[inx].bet = 0;

  return 1;
}

int do_blackjack_exit ( CHAR_DATA *ch )
{
  char log_msg[80];
  int inx;

  inx = bj_index ( ch );
  if ( inx < 0 )
  {
      sprintf ( log_msg, "%s left a table he was not at!", ch->name);
    bug(log_msg,0);
    return 0;
  }

  bj_data[inx].name[0]=0;
  bj_data[inx].inuse=0;

  send_to_char ( "You decide to leave the blackjack table.\n\r", ch );

  return 1;
}

void do_bet2 ( CHAR_DATA *ch, char *argument )
{
  int inx, bet_amt, l1;
  char coin_str[20], log_msg[2048];

  if ( IS_NPC(ch) )
   return;

  if ( check_blackjack ( ch ) )
  {
    inx = bj_index (ch);
    if ( inx < 0 )
    {
      send_to_char ( "You are not sitting at the blackjack table yet!\n\r",
		    ch );
      return;
    }
    if ( bj_data[inx].bet > 0 )
    {
      send_to_char ( "Its too late to change your bet now.\n\r", ch );
      return;
    }
    one_argument ( argument, coin_str );
    if ( !*coin_str )
    {
      send_to_char ( "Bet how many coins?\n\r", ch );
      return;
    }
    sscanf ( coin_str, "%d", &bet_amt );
    if ( ch->gold < bet_amt )
    {
      send_to_char ( "You don't have that much money, pal!\n\r", ch );
      return;
    }
    if (bet_amt <=0)
    {
      send_to_char ("Your bets must be positive, they don't owe you money yet!\n\r", ch );
      return;
    }
    if (CASINO_BANK < 1)
    {
      send_to_char ("The dealer throws up his hands and says 'Sorry bud, the bank is BUSTED!'\n\r", ch);

      send_to_char ("The dealer then proceeds to fall to the ground, crying.\n\r", ch);
      return;
    }
    if ((bet_amt < 100) || (bet_amt > 1000000))
    {
      send_to_char ("Minimum bet is 100 coins, maximum bet is 1000000 coins.\n\r", ch);

      return;
    }
    bj_data[inx].bet = bet_amt;
    ch->gold -= bet_amt;
    bj_data[inx].nd=0;
    bj_data[inx].np=0;
    for ( l1=0;l1<12;bj_data[inx].hand[l1]=0,bj_data[inx].dealer[l1]=0,
	 l1++ );

    if ( bj_data[inx].deck_inx > 30 )
      bj_shuffle ( inx, ch );

    bj_data[inx].hand[bj_data[inx].np++] = 
      bj_data[inx].deck[bj_data[inx].deck_inx++];
    bj_data[inx].hand[bj_data[inx].np++] = 
      bj_data[inx].deck[bj_data[inx].deck_inx++];

    bj_data[inx].dealer[bj_data[inx].nd++] = 
      bj_data[inx].deck[bj_data[inx].deck_inx++];
    bj_data[inx].dealer[bj_data[inx].nd++] = 
      bj_data[inx].deck[bj_data[inx].deck_inx++];

    sprintf ( log_msg, "You are dealt:\n\r" );
    strcat ( log_msg, card_names[bj_data[inx].hand[0] & 0x0f] );
    add_suit ( log_msg, bj_data[inx].hand[0] );
    strcat ( log_msg, " (facedown)\n\r" );

    strcat ( log_msg, card_names[bj_data[inx].hand[1] & 0x0f] );
    add_suit ( log_msg, bj_data[inx].hand[1] );
    strcat ( log_msg, "\n\r\n\r" );

    strcat ( log_msg, "The dealer is showing:\n\r" );
    strcat ( log_msg, card_names[bj_data[inx].dealer[1] & 0x0f] );
    add_suit ( log_msg, bj_data[inx].dealer[1] );
    strcat ( log_msg, "\n\r" );

    send_to_char ( log_msg, ch );

    if ((((bj_data[inx].hand[0] & 0x0f)==1 ) &&
	 ((bj_data[inx].hand[1] & 0x0f)>= 10) ) ||
	(((bj_data[inx].hand[1] & 0x0f)==1 ) &&
	 ((bj_data[inx].hand[0] & 0x0f)>= 10) ))
    {
      send_to_char ( "You get a blackjack! Look at that money roll in!\n\r", ch );
      ch->gold += (int) ((double) bj_data[inx].bet * 2.5);
      CASINO_BANK -= (int) ((double) bj_data[inx].bet * 2.5);
      bj_data[inx].bet = 0;
    }

    if ((((bj_data[inx].dealer[0] & 0x0f)==1 ) &&
	 ((bj_data[inx].dealer[1] & 0x0f)>= 10) ) ||
	(((bj_data[inx].dealer[1] & 0x0f)==1 ) &&
	 ((bj_data[inx].dealer[0] & 0x0f)>= 10) ))
    {
      send_to_char ( "The dealer grins evilly as he gets a blackjack!\n\r", ch );
      CASINO_BANK += bj_data[inx].bet;
      bj_data[inx].bet = 0;
    }

  } else {
    send_to_char ( "So you think you are in a casino?\n\r", ch );
  }
}

void do_stay ( CHAR_DATA *ch, char *argument )
{
  int inx, pbest, dbest, l1;
  char log_msg[2048];

  if ( IS_NPC(ch) )
   return;

  if ( check_blackjack ( ch ) )
  {
    inx = bj_index (ch);
    if ( inx < 0 )
    {
      send_to_char ( "You are not sitting at the blackjack table yet.\n\r",
		    ch );
      return;
    }
    if ( bj_data[inx].bet == 0 )
    {
      send_to_char ( "You are not playing a game of blackjack yet!\n\r", ch );
      return;
    }

    sprintf(log_msg, "The dealer's hole card is ");
    strcat(log_msg, 
	   card_names[bj_data[inx].dealer[bj_data[inx].nd-1] & 0x0f] );
    add_suit ( log_msg, bj_data[inx].dealer[bj_data[inx].nd-1] );
    strcat ( log_msg, ".\n\r" );
    send_to_char(log_msg, ch);
		
    l1 = best_bj_dealer(inx);
    while ( l1 < 17 )
    {
      bj_data[inx].dealer[bj_data[inx].nd++] = 
	bj_data[inx].deck[bj_data[inx].deck_inx++];
      sprintf ( log_msg, "The dealer is dealt the " );
      strcat ( log_msg, 
	      card_names[bj_data[inx].dealer[bj_data[inx].nd-1] & 0x0f] );
      add_suit ( log_msg, bj_data[inx].dealer[bj_data[inx].nd-1] );
      strcat ( log_msg, ".\n\r" );

      send_to_char ( log_msg, ch );
		
      l1 = best_bj_dealer(inx);
      if ( l1 > 21 )
      {
	send_to_char ( "The dealer cries loudly as he busts.\n\r", ch );
	send_to_char ( "You win!!\n\r", ch );
	ch->gold += bj_data[inx].bet * 2;
	CASINO_BANK -= bj_data[inx].bet *2;
	bj_data[inx].bet = 0;
	return;
      }
    }

    strcpy ( log_msg, "Your final hand:\n\r" );
    for (l1=0;l1<bj_data[inx].np;l1++)
    {
      strcat ( log_msg, card_names[bj_data[inx].hand[l1]&0x0F]);
      add_suit ( log_msg, bj_data[inx].hand[l1] );
      strcat ( log_msg, "\r\n" );
    }

    strcat ( log_msg, "\n\rThe dealer's final hand:\n\r" );
    for (l1=0;l1<bj_data[inx].nd;l1++)
    {
      strcat ( log_msg, card_names[bj_data[inx].dealer[l1]&0x0F]);
      add_suit ( log_msg, bj_data[inx].dealer[l1] );
      strcat ( log_msg, "\r\n" );
    }

    send_to_char ( log_msg, ch );

    pbest = best_bj_score (inx);
    dbest = best_bj_dealer (inx);

    if ( pbest > dbest )
    {
      send_to_char ( "You win!!\n\r", ch );
      ch->gold += bj_data[inx].bet * 2;
      CASINO_BANK -= bj_data[inx].bet * 2;
    } else if ( pbest == dbest )
    {
      send_to_char ( "You tie.\n\r", ch );
      ch->gold += bj_data[inx].bet;
      CASINO_BANK -= bj_data[inx].bet;
    } else
    {
      send_to_char ( "You lose.\n\r", ch );
      CASINO_BANK += bj_data[inx].bet;
    }
    bj_data[inx].bet = 0;

  } else {
    send_to_char ( "So you think you are in a casino?\n\r", ch );
  }
}

void do_peek ( CHAR_DATA *ch, char *argument )
{
  char log_msg[2048];
  int l1, inx;

  if ( check_blackjack ( ch ) )
  {
    inx = bj_index ( ch );
    if ( inx < 0 )
    {
      send_to_char ( "You are not sitting at the blackjack table yet.\n\r",
		    ch );
      return;
    }
    if ( bj_data[inx].bet == 0 )
    {
      send_to_char ( "You are not playing a game of blackjack yet!\n\r", ch );
      return;
    }

    strcpy ( log_msg, "You peek at your hand:\n\r" );
    for (l1=0;l1<bj_data[inx].np;l1++)
    {
      strcat ( log_msg, card_names[bj_data[inx].hand[l1]&0x0F]);
      add_suit ( log_msg, bj_data[inx].hand[l1] );
      if ( l1==0 ) strcat ( log_msg, " (facedown)" );
      strcat ( log_msg, "\r\n" );
    }

    strcat ( log_msg, "\n\rThe dealer is showing:\n\r" );
    strcat ( log_msg, card_names[bj_data[inx].dealer[1] & 0x0f] );
    add_suit ( log_msg, bj_data[inx].dealer[1] );
    strcat ( log_msg, "\n\r" );

    send_to_char ( log_msg, ch );

  } else { 
	
    send_to_char ( "So you think you are in a casino?\n\r", ch );

  }
	
}

void do_bj_hit ( CHAR_DATA *ch, char *argument )
{
  int inx;
  char log_msg[2048];

  inx = bj_index ( ch );
  if ( inx < 0 )
  {
    send_to_char ( "You are not sitting at the blackjack table yet.\n\r",
		  ch );
    return;
  }
  if ( bj_data[inx].bet == 0 )
  {
    send_to_char ( "You are not playing a game of blackjack yet!\n\r", ch );
    return;
  }

  bj_data[inx].hand[bj_data[inx].np++] = 
    bj_data[inx].deck[bj_data[inx].deck_inx++];

  sprintf ( log_msg, "You are dealt the " );
  strcat ( log_msg, card_names[bj_data[inx].hand[bj_data[inx].np-1] & 0x0f] );
  add_suit ( log_msg, bj_data[inx].hand[bj_data[inx].np-1] );
  strcat ( log_msg, ".\n\r" );

  send_to_char ( log_msg, ch );

  if ( min_bj_score (inx) > 21 )
  {
    send_to_char ( "You have busted!\n\r", ch );
    bj_data[inx].bet = 0;
  }
}

int bj_index ( struct char_data *ch )
{
  int l1, inx;

  for (l1=0,inx=-1;inx<0 && l1<MAX_BLACKJACK;l1++)
  {
    if ( !strcmp(ch->name, bj_data[l1].name) )
      inx=l1;
  }
  return inx;
}

void add_suit ( char *cat_msg, int card )
{
  if ( card & HEARTS )
    strcat ( cat_msg, " of Hearts" );
  if ( card & DIAMONDS )
    strcat ( cat_msg, " of Diamonds" );
  if ( card & CLUBS )
    strcat ( cat_msg, " of Clubs" );
  if ( card & SPADES )
    strcat ( cat_msg, " of Spades" );
}

int min_bj_score ( int inx )
{
  int l1, l2;

  for ( l1=0,l2=0;l1<bj_data[inx].np;l1++ )
    if ( (bj_data[inx].hand[l1]&0x0f) > 10 )
    {
      l2 += 10;
    } else
    {
      l2 += bj_data[inx].hand[l1] & 0x0f;
    }

  return l2;

}

int best_bj_dealer ( int inx )
{
  int l1, l2, l3;

  for ( l1=0,l2=0,l3=0;l1<bj_data[inx].nd;l1++ )
  {
    if ( (bj_data[inx].dealer[l1]&0x0f) > 10 )
    {
      l2 += 10;
    } else
    {
      l2 += (bj_data[inx].dealer[l1] & 0x0f);
    }
    if ( (bj_data[inx].dealer[l1] & 0x0f) == 1 ) l3++;
  }
  for ( l1=0;l1<l3;l1++ )
    if ( (21-l2) > 10 ) l2 += 10;

  return l2;

}

int best_bj_score ( int inx )
{
  int l1, l2, l3;

  for ( l1=0,l2=0,l3=0;l1<bj_data[inx].np;l1++ )
  {
    if ( (bj_data[inx].hand[l1]&0x0f) > 10 )
    {
      l2 += 10;
    } else
    {
      l2 += (bj_data[inx].hand[l1] & 0x0f);
    }
    if ( (bj_data[inx].hand[l1] & 0x0f) == 1 ) l3++;
  }
  for ( l1=0;l1<l3;l1++ )
    if ( (21-l2) > 10 ) l2 += 10;

  return l2;
}

void setup_casino( void )
{
  CASINO_BANK = 1000000000;
  log_string("Done.");
return;
}

