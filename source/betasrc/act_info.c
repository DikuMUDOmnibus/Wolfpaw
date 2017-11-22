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
 *			     Informational module			    *
 ****************************************************************************/


#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "mud.h"

void send_to_irc_channel args( (CHAR_DATA *master, CHAR_DATA *sender,char *txt ) );
CMAP_DATA *find_char_map_by_type(CHAR_DATA *ch, int map_type);
char *colormap_parse( CHAR_DATA *ch, int map_type, bool advanced, char *speaker, char *string);

char *	const	where_name	[] =
{
    "&B<&cused as light&B>&W     ",
    "&B<&cworn on finger&B>&W    ",
    "&B<&cworn on finger&B>&W    ",
    "&B<&cworn around neck&B>&W  ",
    "&B<&cworn around neck&B>&W  ",
    "&B<&cworn on body&B>&W      ",
    "&B<&cworn on head&B>&W      ",
    "&B<&cworn on legs&B>&W      ",
    "&B<&cworn on feet&B>&W      ",
    "&B<&cworn on hands&B>&W     ",
    "&B<&cworn on arms&B>&W      ",
    "&B<&cworn as shield&B>&W    ",
    "&B<&cworn about body&B>&W   ",
    "&B<&cworn about waist&B>&W  ",
    "&B<&cworn around wrist&B>&W ",
    "&B<&cworn around wrist&B>&W ",
    "&B<&cwielded&B>&W           ",
    "&B<&cheld&B>&W              ",
    "&B<&cdual wielded&B>&W      ",
    "&B<&cworn on ears&B>&W      ",
    "&B<&cworn on eyes&B>&W      ",
    "&B<&cmissile wielded&B>&W   ",
    "&B<&cpinned to chest&B>&W   ",
    "&B<&cawarded&B>          &W ",
    "&B<&cclub insignia&B>&W     "
};


/*
 * Local functions.
 */
void	show_char_to_char_0	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char_1	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char	args( ( CHAR_DATA *list, CHAR_DATA *ch ) );
bool	check_blind		args( ( CHAR_DATA *ch ) );
void    show_condition          args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
char    *sector_name		args( ( int sector ) );
char 	*get_class		args( ( CHAR_DATA *ch ) );
char 	*get_class2		args( ( CHAR_DATA *ch ) );
char 	*get_advclass		args( ( CHAR_DATA *ch ) );
char 	*get_advclass2		args( ( CHAR_DATA *ch ) );

char *format_obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort )
{
    static char buf[MAX_STRING_LENGTH];

    buf[0] = '\0';
    if ( IS_OBJ_STAT(obj, ITEM_INVIS)     )   strcat( buf, "&B(Invis&B)&W " );
    if ( IS_AFFECTED(ch, AFF_DETECT_EVIL)
	 && IS_OBJ_STAT(obj, ITEM_EVIL)   )   strcat( buf, "&b(&RRed Aura&b)&W ");
    if ( IS_AFFECTED(ch, AFF_DETECT_MAGIC)
	 && IS_OBJ_STAT(obj, ITEM_MAGIC)  )   strcat( buf,"&Y(&BMagical&Y)&W " );
    if ( IS_OBJ_STAT(obj, ITEM_GLOW)      )   strcat( buf,"&W(&YGlowing&W) " );
    if ( IS_OBJ_STAT(obj, ITEM_HUM)       )   strcat( buf,"&W(&GHumming)&W " );
    if ( IS_OBJ_STAT(obj, ITEM_HIDDEN)	  )   strcat( buf,"&G(&RHidden&G)&W " );
    if ( IS_OBJ_STAT(obj, ITEM_BURRIED)	  )   strcat( buf,"&G(&RBurried&G)&W " );
    if ( IS_IMMORTAL(ch)
	 && IS_OBJ_STAT(obj, ITEM_PROTOTYPE) ) strcat( buf,"&R(&WPROTO&R)&W " );
    if ( IS_AFFECTED(ch, AFF_DETECTTRAPS)
	 && is_trapped(obj)   )   strcat( buf, "&W(&RTrap&W) "  );

    if ( fShort )
    {
	if ( obj->short_descr )
	    strcat( buf, obj->short_descr );
    }
    else
    {
	if ( obj->description )
	    strcat( buf, obj->description );
    }

    return buf;
}


/*
 * Some increasingly freaky halucinated objects		-Thoric
 */
char *halucinated_object( int ms, bool fShort )
{
    int sms = URANGE( 1, (ms+10)/5, 20 );

    if ( fShort )
    switch( number_range( 6-URANGE(1,sms/2,5), sms ) )
    {
	case  1: return "a sword";
	case  2: return "a stick";
	case  3: return "something shiny";
	case  4: return "something";
	case  5: return "something interesting";
	case  6: return "something colorful";
	case  7: return "something that looks cool";
	case  8: return "a nifty thing";
	case  9: return "a cloak of flowing colors";
	case 10: return "a mystical flaming sword";
	case 11: return "a swarm of insects";
	case 12: return "a deathbane";
	case 13: return "a figment of your imagination";
	case 14: return "your gravestone";
	case 15: return "the long lost boots of Ranger Thoric";
	case 16: return "a glowing tome of arcane knowledge";
	case 17: return "a long sought secret";
	case 18: return "the meaning of it all";
	case 19: return "the answer";
	case 20: return "the key to life, the universe and everything";
    }
    switch( number_range( 6-URANGE(1,sms/2,5), sms ) )
    {
	case  1: return "A nice looking sword catches your eye.";
	case  2: return "The ground is covered in small sticks.";
	case  3: return "Something shiny catches your eye.";
	case  4: return "Something catches your attention.";
	case  5: return "Something interesting catches your eye.";
	case  6: return "Something colorful flows by.";
	case  7: return "Something that looks cool calls out to you.";
	case  8: return "A nifty thing of great importance stands here.";
	case  9: return "A cloak of flowing colors asks you to wear it.";
	case 10: return "A mystical flaming sword awaits your grasp.";
	case 11: return "A swarm of insects buzzes in your face!";
	case 12: return "The extremely rare Deathbane lies at your feet.";
	case 13: return "A figment of your imagination is at your command.";
	case 14: return "You notice a gravestone here... upon closer examination, it reads your name.";
	case 15: return "The long lost boots of Ranger Thoric lie off to the side.";
	case 16: return "A glowing tome of arcane knowledge hovers in the air before you.";
	case 17: return "A long sought secret of all mankind is now clear to you.";
	case 18: return "The meaning of it all, so simple, so clear... of course!";
	case 19: return "The answer.  One.  It's always been One.";
	case 20: return "The key to life, the universe and everything awaits your hand.";
    }
    return "Whoa!!!";
}


int scan_room (CHAR_DATA *ch, const ROOM_INDEX_DATA *room,char *buf)
{
    CHAR_DATA *target = room->first_person;
    int number_found = 0;

	if ( ch == NULL )
	return 0;

    while (target != NULL) /* repeat as long more peple in the room */
    {
        if (can_see(ch,target)) /* show only if the character can see the target */
        {
            strcat (buf, " - ");
            strcat (buf, IS_NPC(target) ? target->short_descr : target->name);
            strcat (buf, "\n\r");
            number_found++;
        }
        target = target->next_in_room;

  }
    return number_found;
}

void do_allscan (CHAR_DATA *ch, char *argument)
{
    EXIT_DATA *pexit;
    ROOM_INDEX_DATA * room;
    ROOM_INDEX_DATA * oldroom=NULL;
    extern char * const dir_name[];
    char buf[MAX_STRING_LENGTH];
    int dir;
    int distance;

	if ( ch == NULL || ch->in_room == NULL )
	return;

    sprintf (buf, "Right here you see:\n\r");
    if (scan_room(ch,ch->in_room,buf) == 0)
        strcat (buf, "Noone\n\r");
    send_to_char (buf,ch);

    for (dir = 0; dir < 6; dir++) /* look in every direction */
    {
        room = ch->in_room; /* starting point */

        for (distance = 1 ; distance < 6; distance++)
        {
	    if ( room == oldroom )
		break;

          pexit = get_exit(room,dir);	     

		buf[0] = '\0';

	        if ( pexit == NULL || (pexit->to_room == NULL) || IS_SET(pexit->exit_info, EX_CLOSED))
                break; /* exit not there OR points to nothing OR is closed*/

            /* char can see the room */
            sprintf (buf, "%d %s from here you see:\n\r", distance, dir_name[dir]);
            if (scan_room(ch,pexit->to_room,buf)) /* if there is something there */
                send_to_char (buf,ch);
	    
	    if ( pexit->to_room )	
	    {
	    oldroom = room;
            room = pexit->to_room; /* go to the next room */
	    }
	    else
	    break;

        } /* for distance */
    } /* for dir */
}


/* adding guild to name thing --GW */
char *GUILDCHECK( CHAR_DATA *ch )
{
char *name2;
char name[MSL];

if ( IS_NPC(ch) )
return ch->name;

        if ( !IS_NPC(ch) && IS_SET(ch->pcdata->flagstwo, MOREPC_NAME_SET) && 
	    ( ch->pcdata->pre_name_toggle == 1 ) )
        sprintf(name,"%s %s",ch->pcdata->name,QUICKLINK(ch->name));
        else
        sprintf(name,"%s",QUICKLINK(ch->name));

name2 = name;

return name2;
}

/*
 ******************** IRC CHANNEL STUFF --GW *********************
 */

/*
 * to talk on the channel, or see whos on it.
 */
void do_irc( CHAR_DATA *ch, char *argument )
{
char buf[MSL];
CHAR_DATA *irc;
int cnt;

if ( IS_NPC(ch))
 return;

buf[0] = '\0';

if( IS_SET(ch->act, PLR_SILENCE) )
{
send_to_char("You can't use Channels!\n\r",ch);
return;
}

if( !ch->pcdata->irc_channel_master )
{
send_to_char("You are not part of any irc channel.\n\r",ch);
return;
}

if( !*argument )
{
send_to_char("Members of your IRC Channel:\n\r",ch);
sprintf(buf,"Channel Master: %s\n\r",capitalize(ch->pcdata->irc_channel_master->name));
send_to_char(buf,ch);
send_to_char("Others:\n\r",ch);
cnt=0;
for( irc = first_char; irc; irc = irc->next )
{
if( irc->pcdata->irc_channel_master &&
	irc->pcdata->irc_channel_master == ch->pcdata->irc_channel_master &&
	irc != irc->pcdata->irc_channel_master && irc->pcdata->irc_channel_master != ch )
{
sprintf(buf,"%s\n\r",capitalize(irc->name));
send_to_char(buf,ch);
cnt++;
}
}

send_to_char(buf,ch);
return;
}

send_to_irc_channel(ch->pcdata->irc_channel_master,ch,argument);
return;
}

/*
 * Add a player to the Channel
 */
void do_addirc( CHAR_DATA *ch, char *argument )
{
CHAR_DATA *victim;
char buf[MSL];
char arg[MAX_INPUT_LENGTH];

one_argument( argument, arg );

if ( IS_NPC(ch))
 return;

if( IS_SET(ch->act, PLR_SILENCE) )
{
send_to_char("You can't use Channels!\n\r",ch);
return;
}

if ( arg[0] == '\0' )
{
send_to_char("Addirc who?\n\r",ch);
return;
}

if ( ch->pcdata->irc_channel_master && ch->pcdata->irc_channel_master != ch )
{
send_to_char("You can only add to your own channel.\n\r",ch);
return;
}
 
if( ( victim=get_char_world(ch,arg) )==NULL )
{
send_to_char("No such player logged in.\n\r",ch);
return;
}

if ( IS_NPC(victim) )
{
send_to_char("Mobs can't use IRC!!\n\r",ch);
return;
}

if ( is_ignoring( victim, ch ) )
{
send_to_char("That Person is Ignoring you.\n\r",ch);
return;
}

if ( victim->pcdata->irc_channel_master != NULL )
{
sprintf(buf,"%s is part of another IRC Channel.\n\r",capitalize(victim->name));
send_to_char(buf,ch);
return;
}

if( ch->pcdata->irc_channel_master == NULL )
{
sprintf(buf,"%s has been added to the IRC Channel.",capitalize(victim->name));
send_to_irc_channel(ch,NULL,buf);
victim->pcdata->irc_channel_master = ch;
ch->pcdata->irc_channel_master = ch;
sprintf(buf,"%s is now part of your IRC Channel.\n\r",victim->name);
send_to_char(buf,ch);
sprintf(buf,"You have been added to %s's IRC Channel.\n\r",capitalize(ch->name));
send_to_char(buf,victim);
return;
}

sprintf(buf,"%s is now part of your IRC Channel.\n\r",victim->name);
send_to_char(buf,ch);
sprintf(buf,"%s has been added to the IRC Channel.",capitalize(victim->name));
send_to_irc_channel(ch,NULL,buf);
sprintf(buf,"You have been added to %s's IRC Channel.\n\r",capitalize(ch->name));
send_to_char(buf,victim);
victim->pcdata->irc_channel_master = ch;
return;
}

/*
 * End the channel
 */
void do_endirc( CHAR_DATA *ch, char *argument )
{
CHAR_DATA *irc,*irc_next;
CHAR_DATA *master;
char buf[MSL];
int channel_cnt=0;
bool kill_channel=FALSE;
if ( IS_NPC(ch))
 return;

if ( !ch->pcdata->irc_channel_master )
{
send_to_char("You are not part of any IRC Channel.\n\r",ch);
return;
}

if ( !IS_NPC(ch) && ch->pcdata->irc_channel_master != ch )
{
send_to_char("You quit the IRC Channel.\n\r",ch);
master = ch->pcdata->irc_channel_master;
sprintf(buf,"%s has left the IRC Channel.",capitalize(ch->name));
send_to_irc_channel(master,NULL,buf);

/* Count em up, and kill the channel if no one is left*/
channel_cnt=0;
for( irc = first_char; irc; irc = irc_next )
{
irc_next = irc->next;
if( !IS_NPC(irc) && irc->pcdata->irc_channel_master &&
    irc->pcdata->irc_channel_master == ch)
channel_cnt++;
}

if ( channel_cnt<=1)
{
 sprintf(buf,"No other channel members left, channel terminated.");
 send_to_irc_channel(master,NULL,buf);
 kill_channel=TRUE;
}

}
else
{
send_to_char("You Terminate your IRC Channel.\n\r",ch);
master = ch;
ch->pcdata->irc_channel_master = NULL;
sprintf(buf,"%s has Terminated the IRC Channel.",capitalize(master->name));
send_to_irc_channel(master,NULL,buf);
kill_channel=TRUE;
}

if ( kill_channel )
{
  /* Channels dead Jim.. --GW*/
  for( irc = first_char; irc; irc = irc_next)
  {
   irc_next = irc->next;
    if( !IS_NPC(irc) && irc->pcdata->irc_channel_master &&
      irc->pcdata->irc_channel_master == ch)
       irc->pcdata->irc_channel_master = NULL;
  }
}

  ch->pcdata->irc_channel_master = NULL;

return;
}

/*
 * Kick a user from your own channel
 */
void do_irckick( CHAR_DATA *ch, char *argument )
{
CHAR_DATA *victim;
char buf[MSL];
char arg[MAX_INPUT_LENGTH];
CHAR_DATA *irc;
int cnt;

if ( IS_NPC(ch))
 return;

one_argument( argument, arg );

if ( ch->pcdata->irc_channel_master != ch )
{
send_to_char("You can only kick a user off your own IRC Channel.\n\r",ch);
return;
}

if( arg[0] == '\0' )
{
send_to_char("Kick who off your IRC Channel.\n\r",ch);
return;
}

if( ( victim=get_char_world(ch,arg) )==NULL)
{
send_to_char("That player is not logged in.\n\r",ch);
return;
}

if ( IS_NPC(victim) )
{
send_to_char("You Can't add NPC's!\n\r",ch);
return;
}

if ( victim->pcdata->irc_channel_master != ch )
{
send_to_char("That person is not part of your IRC Channel.\n\r",ch);
return;
}

send_to_char("You have been kicked off the IRC Channel.\n\r",victim);
victim->pcdata->irc_channel_master = NULL;
send_to_char("Done.\n\r",ch);
sprintf(buf,"%s has kicked %s off the IRC Channel.",
	capitalize(ch->name), QUICKLINK(victim->name));
send_to_irc_channel(ch,NULL,buf);
cnt=0;
for( irc = first_char; irc; irc = irc->next )
{
if ( irc->pcdata->irc_channel_master != NULL &&
	irc->pcdata->irc_channel_master == ch )
	cnt++;
}
if ( cnt <= 1 )
do_endirc(ch,"");

return;
}

void send_to_irc_channel( CHAR_DATA *master, CHAR_DATA *sender, char *txt)
{
CHAR_DATA *irc;
char arg[MSL];
char buf[MSL];
char txtbuf[MSL];
bool channel_social = FALSE;
bool channel_emote = FALSE;
CHAR_DATA *victim;
SOCIALTYPE *social=NULL;
char tosender[MSL],tovictim[MSL],tochan[MSL];

tochan[0] = '\0';
tosender[0] = '\0';
tovictim[0] = '\0';
victim = NULL;

/* Socials --GW */
if ( txt[0] == '@' )
{
txt = one_argument( txt, arg);
if ( !IS_AWAKE(sender) )
{
  send_to_char("Wake up first!\n\r",sender);
  return;
}

if ( arg[1] == '\0' )
{
  send_to_char("What Social?\n\r",sender);
  return;
}
  
if ( (social = find_social(sender,arg+1,FALSE)) == NULL)
{
  if ((social = find_social(sender,arg+1,TRUE)) == NULL)
  {
    send_to_char("No such social exsists.\n\r",sender);
    return;
  }
}

        /* Make sure the social is complete -- or it WILL crash! --GW */
        if ( !social->char_no_arg || !social->others_no_arg || !social->char_found ||
             !social->others_found || !social->vict_found || !social->char_auto ||
             !social->others_auto )
        { 
          send_to_char("Sorry, that social is not complete.\n\r",sender);
          return;
        } 

        if(txt[0] != '\0')
          if((victim = get_char_world(sender, txt)) == NULL)
	  {
            sprintf(buf, "You can't find any %s!\n\r", txt);
	    send_to_char(buf,sender);
	  }

        if ( victim )
	if(IS_NPC(victim) || (!IS_NPC(victim) && (victim->pcdata->irc_channel_master != master) ))
	{
	  send_to_char("But that person is not in the channel!!",sender);
	  return;
	}

channel_social = TRUE;
}
/* Emotes */
else if ( txt[0] == ',' )
{
if ( txt[1] == '\0' )
{
  send_to_char("Emote what over IRC?\n\r",sender);
  return;
}

txt++;
sprintf(txtbuf,"%s%s\n\r",capitalize(sender->name),txt);
channel_emote = TRUE;
}
else
 strcpy(txtbuf,txt);

if ( channel_social )
{
CHAR_DATA *ch;

ch = sender;

  /* Send to the Sender */
  if ( victim && victim != sender )
    sprintf(tosender,"IRC:: %s",act_string(social->char_found,ch,ch,NULL,victim));
  else if ( victim && victim == sender )
    sprintf(tosender,"IRC:: %s",act_string(social->char_auto,ch,ch,NULL,victim));
  else
    sprintf(tosender,"IRC:: %s",act_string(social->char_no_arg,ch,ch,NULL,victim));
  send_to_char(tosender,ch);

  /* Output to the 'victim' */
  if ( victim && victim != sender )
  {
      sprintf(tovictim,"IRC:: %s",act_string(social->vict_found,victim,ch,NULL,victim));
      send_to_char(tovictim,victim);
  }
}


for ( irc = first_char; irc; irc = irc->next )
{
if ( irc->pcdata->irc_channel_master &&
     irc->pcdata->irc_channel_master == master )
 {
  if ( channel_social )
  {

  if ( victim && victim != sender )
    sprintf(tochan,"IRC:: %s",act_string(social->others_found,irc,sender,NULL,victim));
  else if ( victim && victim == sender )
    sprintf(tochan,"IRC:: %s",act_string(social->others_auto,irc,sender,NULL,victim));
  else
    sprintf(tochan,"IRC:: %s",act_string(social->others_no_arg,irc,sender,NULL,victim));

    if ( victim && (irc != victim) && (irc != sender) )
      send_to_char(tochan,irc);

    if ( !victim && (irc != sender))
      send_to_char(tochan,irc);
  }
  else if ( channel_emote )
  {
    sprintf(buf,"IRC:: %s",txtbuf);
    send_to_char(buf,irc);
  }
  else
  {
    sprintf(buf,"IRC:%s: %s\n\r",sender ? capitalize(sender->name) : "IRC-DAEMON",txtbuf);
    send_to_char(buf,irc);
  }
 }
}

return;
}

/*
 ******************* END OF IRC STUFF --GW ******************
 */

/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing )
{
    char **prgpstrShow=NULL;
    int *prgnShow=0;
    int *pitShow=0;
    char *pstrShow;
    OBJ_DATA *obj;
    int nShow;
    int iShow;
    int count, offcount, tmp, ms, cnt;
    bool fCombine;
    CMAP_DATA *map;

    if ( !ch->desc )
	return;

    /*
     * if there's no list... then don't do all this crap!  -Thoric
     */
    if ( !list )
    {
    	if ( fShowNothing )
    	{
	   if ( IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE) )
	      send_to_char( "     ", ch );
	   send_to_char( "Nothing.\n\r", ch );
	}
	return;
    }
    /*
     * Alloc space for output lines.
     */
    count = 0;
    for ( obj = list; obj; obj = obj->next_content )
	count++;

    ms  = (ch->mental_state ? ch->mental_state : 1)
	* (IS_NPC(ch) ? 1 : (ch->pcdata->condition[COND_DRUNK] ? (ch->pcdata->condition[COND_DRUNK]/12) : 1));

    /*
     * If not mentally stable...
     */
    if ( abs(ms) > 40 )
    {
	offcount = URANGE( -(count), (count * ms) / 100, count*2 );
	if ( offcount < 0 )
	  offcount += number_range(0, abs(offcount));
	else
	if ( offcount > 0 )
	  offcount -= number_range(0, offcount);
    }
    else
	offcount = 0;

    if ( count + offcount <= 0 )
    {
    	if ( fShowNothing )
    	{
	   if ( IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE) )
	      send_to_char( "     ", ch );
	   send_to_char( "Nothing.\n\r", ch );
	}
	return;
    }

    CREATE( prgpstrShow,	char*,	count + ((offcount > 0) ? offcount : 0) );
    CREATE( prgnShow,		int,	count + ((offcount > 0) ? offcount : 0) );
    CREATE( pitShow,		int,	count + ((offcount > 0) ? offcount : 0) );
    nShow	= 0;
    tmp		= (offcount > 0) ? offcount : 0;
    cnt		= 0;

    /*
     * Format the list of objects.
     */
    for ( obj = list; obj; obj = obj->next_content )
    {
	/* Don't show a characters vehicle to them in the room --GW */
	if ( !IS_NPC(ch) && ch->pcdata->vehicle &&
             obj->pIndexData->vehicle &&
             obj->pIndexData->vehicle == ch->pcdata->vehicle )
	continue;

	if ( offcount < 0 && ++cnt > (count + offcount) )
	    break;
	if ( tmp > 0 && number_bits(1) == 0 )
	{
	    prgpstrShow [nShow] = str_dup( halucinated_object(ms, fShort) );
	    prgnShow	[nShow] = 1;
	    pitShow	[nShow] = number_range( ITEM_LIGHT, ITEM_BOOK );
	    nShow++;
	    --tmp;
	}
	if ( obj->wear_loc == WEAR_NONE
	&& can_see_obj( ch, obj )
	&& (obj->item_type != ITEM_TRAP || IS_AFFECTED(ch, AFF_DETECTTRAPS) ) )
	{
	    pstrShow = format_obj_to_char( obj, ch, fShort );
	    fCombine = FALSE;

	    if ( IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE) )
	    {
		/*
		 * Look for duplicates, case sensitive.
		 * Matches tend to be near end so run loop backwords.
		 */
		for ( iShow = nShow - 1; iShow >= 0; iShow-- )
		{
		    if ( !strcmp( prgpstrShow[iShow], pstrShow ) )
		    {
			prgnShow[iShow] += obj->count;
			fCombine = TRUE;
			break;
		    }
		}
	    }

	    pitShow[nShow] = obj->item_type;
	    /*
	     * Couldn't combine, or didn't want to.
	     */
	    if ( !fCombine )
	    {
		prgpstrShow [nShow] = str_dup( pstrShow );
		prgnShow    [nShow] = obj->count;
		nShow++;
	    }
	}
    }
    if ( tmp > 0 )
    {
	int x;
	for ( x = 0; x < tmp; x++ )
	{
	    prgpstrShow [nShow] = str_dup( halucinated_object(ms, fShort) );
	    prgnShow	[nShow] = 1;
	    pitShow	[nShow] = number_range( ITEM_LIGHT, ITEM_BOOK );
	    nShow++;
	}
    }

    /*
     * Output the formatted list.		-Color support by Thoric
     */
    for ( iShow = 0; iShow < nShow; iShow++ )
    {
	switch(pitShow[iShow]) {
	default:
	  set_char_color( AT_OBJECT, ch );
	  break;
	case ITEM_BLOOD:
	  set_char_color( AT_BLOOD, ch );
	  break;
	case ITEM_MONEY:
	case ITEM_TREASURE:
	  set_char_color( AT_YELLOW, ch );
	  break;
	case ITEM_FOOD:
	  set_char_color( AT_HUNGRY, ch );
	  break;
	case ITEM_DRINK_CON:
	case ITEM_FOUNTAIN:
	  set_char_color( AT_THIRSTY, ch );
	  break;
	case ITEM_FIRE:
	  set_char_color( AT_FIRE, ch );
	  break;
	case ITEM_SCROLL:
	case ITEM_WAND:
	case ITEM_STAFF:
	  set_char_color( AT_MAGIC, ch );
	  break;
	}

	if ( colormap_check(ch,CMAP_OBJ) ) /* Color Maps --GW */
	{
	  map=find_char_map_by_type(ch,CMAP_OBJ);	  
	  set_char_color( get_real_act_color_num(map->mapped_color),ch);
        }

	if ( fShowNothing )
	    send_to_char( "     ", ch );
	send_to_char( prgpstrShow[iShow], ch );
/*	if ( IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE) ) */
	{
	    if ( prgnShow[iShow] != 1 )
		ch_printf( ch, " (%d)", prgnShow[iShow] );
	}

	send_to_char( "\n\r", ch );
	DISPOSE( prgpstrShow[iShow] );
    }

    if ( fShowNothing && nShow == 0 )
    {
	if ( IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE) )
	    send_to_char( "     ", ch );
	send_to_char( "Nothing.\n\r", ch );
    }

    /*
     * Clean up.
     */
    DISPOSE( prgpstrShow );
    DISPOSE( prgnShow	 );
    DISPOSE( pitShow	 );
    return;
}


/*
 * Show fancy descriptions for certain spell affects		-Thoric
 */
void show_visible_affects_to_char( CHAR_DATA *victim, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    char sex[MAX_STRING_LENGTH];

    switch( victim->sex )
    {
	case SEX_MALE:
		strcpy( sex, "he" );
		break;
	case SEX_FEMALE:
		strcpy( sex, "she" );
		break;
	default:
		strcpy( sex, "it" );
		break;
    }

    if( !IS_NPC(ch) && !IS_NPC(victim) && IS_SET( ch->pcdata->flagstwo, MOREPC_AFFMSGS ) )
	return;

    if ( IS_AFFECTED(victim, AFF_SANCTUARY) )
    {
        if ( IS_GOOD(victim) )
        {
            set_char_color( AT_WHITE, ch );
            ch_printf( ch, "...%s glows with an aura of divine radiance.\n\r",
		sex );
        }
        else if ( IS_EVIL(victim) )
        {
            set_char_color( AT_WHITE, ch );
            ch_printf( ch, "...%s shimmers beneath an aura of dark energy.\n\r",
		sex );
        }
        else
        {
            set_char_color( AT_WHITE, ch );
            ch_printf( ch, "...%s is shrouded in flowing shadow and light.\n\r",
		sex );
        }
    }

    /* 
     * Pk Champ Stuff
     */
     if ( !IS_NPC(victim) && IS_SET(victim->pcdata->flags, PCFLAG_PKCHAMP ) )
     act( AT_PLAIN,"&R...you cower before $S Might!&W",ch,NULL,victim,TO_CHAR);

     /*
      * Aliens --GW
      */
     if ( !IS_NPC(victim) && IS_SET(victim->pcdata->flagstwo, MOREPC_ALIENS ) )
     act( AT_PLAIN,"&R...you see a triangular mark in $S forehead.&W",ch,NULL,victim,TO_CHAR);


    /*
     * Advanced Class Fancy Stuff
     */

    if ( !IS_NPC(victim) && IS_ADVANCED(victim) )
    {    

     switch( victim->advclass )
     {

     case 12: /* avatar */
     act( AT_MAGIC, "...you discover a mystic glow around $M.",ch,NULL,victim,TO_CHAR);
     break;

     case 14: /* dragon */
     act( AT_BLOOD, "...the heavy scent of blood surrounds $M.",ch,NULL,victim,TO_CHAR);
     break;
    
     }
     }

     /*
      * Adv Dual fancy Stuff --GW
      */
     if ( !IS_NPC(victim) && IS_ADV_DUAL(victim ) )
     {
     switch( victim->advclass2 )
     {
     case 15: /* BladeMaster */
     act( AT_MAGIC,"...you see $S muscles bulge and ripple under $S skin.",ch,NULL,victim,TO_CHAR ); 
     break;

     case 16: /* ArchMage */
     act( AT_MAGIC,"...a mystical power eminates from $M.",ch,NULL,victim,TO_CHAR);
     break;

     case 17: /* Bishop */
     act( AT_MAGIC,"...$S eyes glow with the wisdom of the ages.",ch,NULL,victim,TO_CHAR );
     break;

     case 18: /* Alchemist */
     act( AT_MAGIC,"...the smell of forest and earth surrounds $M.",ch,NULL,victim,TO_CHAR);
     break;

     case 19: /* Prophet */
     act( AT_MAGIC,"...$S glows with the very knowledge of time.",ch,NULL,victim,TO_CHAR);
     break;

     case 20: /* Crusader */
     break;

     case 21: /* Samurai */
     break;

     case 22: /* Wraith */
     act( AT_CARNAGE,"...$S skin seems that of living stone.",ch,NULL,victim,TO_CHAR);
     break;

     case 23: /* Hunter */
     break;

     case 24: /* Adept */
     break;

     case 25: /* Ninja */
     break;

     case 26: /* Jackel */
     break;

     case 27: /* Psi Adv Class here ... */
     break;

     default:
     break;
    }
   }

    if ( IS_AFFECTED(victim, AFF_FIRESHIELD) )
    {
        set_char_color( AT_FIRE, ch );
        ch_printf( ch, "...%s is engulfed within a blaze of mystical flame.\n\r",
		sex );
    }
    if ( IS_AFFECTED(victim, AFF_SHOCKSHIELD) )
    {
        set_char_color( AT_BLUE, ch );
	ch_printf( ch, "...%s is surrounded by cascading torrents of energy.\n\r",
		sex );
    }
/*Scryn 8/13*/
    if ( IS_AFFECTED(victim, AFF_ICESHIELD) )
    {
        set_char_color( AT_LBLUE, ch );
        ch_printf( ch, "...%s is ensphered by shards of glistening ice.\n\r",
		sex );
    }
    if ( IS_AFFECTED(victim, AFF_REFLECT_MAGIC) )
    {
       set_char_color( AT_MAGIC, ch );
       ch_printf( ch, "...%s is surrounded by a mystical shield.\n\r",
		sex );
    }
    if ( IS_AFFECTED(victim, AFF_CHARM)       )
    {
	set_char_color( AT_MAGIC, ch );
	ch_printf( ch, "...%s wanders in a dazed, zombie-like state.\n\r",
		sex );
    }
    if ( IS_AFFECTED(victim, AFF_GHOST) )
    {
	set_char_color( AT_MAGIC, ch );
	ch_printf( ch, "...%s is totally Transparent! Like a Ghost....\n\r",
		sex );
    }
    if ( !IS_NPC(victim) && !victim->desc
    &&    victim->switched && IS_AFFECTED(victim->switched, AFF_POSSESS) )
    {
	set_char_color( AT_MAGIC, ch );
	strcpy( buf, PERS( victim, ch ) );
	strcat( buf, " appears to be in a deep trance...\n\r" );
    }
    if ( !IS_NPC(victim) && IS_SET( victim->pcdata->flags, PCFLAG_DEADLY ) 
	&& !IS_IMMORTAL(victim) )
    {
        set_char_color( AT_CARNAGE, ch );
	ch_printf( ch, "...%s smiles at you with a feral grin.\n\r",
		capitalize(victim->name) );
    }
}

void show_char_to_char_0( CHAR_DATA *victim, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    CMAP_DATA *map;

    buf[0] = '\0';

    if ( !victim || !victim->in_room || !ch || !ch->in_room )
    return;

    if ( !IS_NPC(victim) && !victim->desc )
    {
	if ( !victim->switched )		strcat( buf, "(Link Dead) "  );
	else
	if ( !IS_AFFECTED(victim->switched, AFF_POSSESS) )
						strcat( buf, "(Switched) " );
    }
    if ( !IS_NPC(victim)
    && IS_SET(victim->act, PLR_AFK) )		strcat( buf, "[AFK] ");        

    /* Quest Target --GW */
    if ( IS_NPC(victim) &&
    IS_SET( victim->act, ACT_QUESTTARGET ) )
    						strcat( buf, "&G[&RTARGET&G]&W");

    if ( (!IS_NPC(victim) && IS_SET(victim->act, PLR_WIZINVIS))
      || (IS_NPC(victim) && IS_SET(victim->act, ACT_MOBINVIS)) ) 
    {
        if (!IS_NPC(victim))
	sprintf( buf1,"(Invis %d) ", victim->pcdata->wizinvis );
        else sprintf( buf1,"(Mobinvis %d) ", victim->mobinvis);
	strcat( buf, buf1 );
    }

    if (!IS_NPC(victim) && IS_SET(victim->pcdata->flagstwo, MOREPC_INCOG)
       && !IS_SET(victim->act, PLR_WIZINVIS))
    {
	if (!IS_NPC(victim))
	sprintf( buf1,"(Incog %d) ", victim->pcdata->incog_level );
	strcat( buf, buf1 );
    }
    if ( IS_AFFECTED(victim, AFF_INVISIBLE)   ) strcat( buf, "&B(Invis)&W "  );
    if ( IS_AFFECTED(victim, AFF_HIDE)        ) strcat( buf, "&c(Hide)&W ");
    if ( IS_AFFECTED(victim, AFF_PASS_DOOR)   ) strcat( buf, "(Translucent) ");
    if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) ) strcat( buf, "&P(Pink Aura)&W ");
    if ( IS_EVIL(victim)
    &&   IS_AFFECTED(ch, AFF_DETECT_EVIL)     ) strcat( buf, "&R(Red Aura)&W ");
    if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_KILLER ) )
						strcat( buf, "(KILLER) " );
    if ( !IS_NPC(victim) && IS_IMMORTAL(victim) && (victim->level >= 57 ) )
					strcat( buf, "&W(&BAura of Power&W) ");
    if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_THIEF  ) )
						strcat( buf, "(THIEF) "      );
    if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_LITTERBUG  ) )
						strcat( buf, "(LITTERBUG) "  );
    if ( IS_NPC(victim) && IS_IMMORTAL(ch)
	 && IS_SET(victim->act, ACT_PROTOTYPE) ) strcat( buf, "(PROTO) " );
    if ( !IS_NPC(victim) && victim->desc && victim->desc->connected == CON_EDITING )
						strcat( buf, "(Writing) " );

   if ( !colormap_check(ch,CMAP_MOB) )
      set_char_color( AT_PERSON, ch );
   else /* Color maps --GW */
   {
      map=find_char_map_by_type(ch,CMAP_MOB);
      set_char_color(get_real_act_color_num(map->mapped_color), ch);
   }
  
    if ( victim->position == victim->defposition && victim->long_descr[0] != '\0' )
    {
	strcat( buf, victim->long_descr );
	send_to_char( buf, ch );
	show_visible_affects_to_char( victim, ch );
	return;
    }

    strcat( buf, TPERS( victim, ch ) );
    if ( !IS_NPC(victim) && !IS_SET(ch->act, PLR_BRIEF) )
	strcat( buf, victim->pcdata->title );

    switch ( victim->position )
    {
    case POS_DEAD:     strcat( buf, " is DEAD!!" );			break;
    case POS_MORTAL:   strcat( buf, " is mortally wounded." );		break;
    case POS_INCAP:    strcat( buf, " is incapacitated." );		break;
    case POS_STUNNED:  strcat( buf, " is lying here stunned." );	break;
    case POS_SLEEPING:
        if (ch->position == POS_SITTING
        ||  ch->position == POS_RESTING )
            strcat( buf, " is sleeping nearby." );
	else
            strcat( buf, " is deep in slumber here." );
        break;
    case POS_RESTING:
        if (ch->position == POS_RESTING)
            strcat ( buf, " is sprawled out alongside you." );
        else
	if (ch->position == POS_MOUNTED)
	    strcat ( buf, " is sprawled out at the foot of your mount." );
	else
            strcat (buf, " is sprawled out here." );
        break;
    case POS_SITTING:
        if (ch->position == POS_SITTING)
            strcat( buf, " sits here with you." );
        else
        if (ch->position == POS_RESTING)
            strcat( buf, " sits nearby as you lie around." );
        else
            strcat( buf, " sits upright here." );
        break;
    case POS_STANDING:
        if ( IS_IMMORTAL(victim) && (victim->level >= 57 ))
	    strcat( buf, " is shrouded in Mystical Flames before you." );
	else
	if ( IS_IMMORTAL(victim) )
            strcat( buf, " is here before you." );
	else
        if ( ( victim->in_room->sector_type == SECT_UNDERWATER )
        && !IS_AFFECTED(victim, AFF_AQUA_BREATH) && !IS_NPC(victim) )
            strcat( buf, " is drowning here." );
	else
	if ( victim->in_room->sector_type == SECT_UNDERWATER )
            strcat( buf, " is here in the water." );
	else
	if ( ( victim->in_room->sector_type == SECT_OCEANFLOOR )
	&& !IS_AFFECTED(victim, AFF_AQUA_BREATH) && !IS_NPC(victim) )
	    strcat( buf, " is drowning here." );
	else
	if ( victim->in_room->sector_type == SECT_OCEANFLOOR )
	    strcat( buf, " is standing here in the water." );
	else
	if ( IS_AFFECTED(victim, AFF_FLOATING)
        || IS_AFFECTED(victim, AFF_FLYING) )
          strcat( buf, " is hovering here." );
        else
          strcat( buf, " is standing here." );
        break;
    case POS_SHOVE:    strcat( buf, " is being shoved around." );	break;
    case POS_DRAG:     strcat( buf, " is being dragged around." );	break;
    case POS_MOUNTED:
	strcat( buf, " is here, upon " );
	if ( !victim->mount )
	    strcat( buf, "thin air???" );
	else
	if ( victim->mount == ch )
	    strcat( buf, "your back." );
	else
	if ( victim->in_room == victim->mount->in_room )
	{
	    strcat( buf, PERS( victim->mount, ch ) );
	    strcat( buf, "." );
	}
	else
	    strcat( buf, "someone who left??" );
	break;
    case POS_FIGHTING:
	strcat( buf, " is here, fighting " );
	if ( !victim->fighting )
	    strcat( buf, "thin air???" );
	else if ( who_fighting( victim ) == ch )
	    strcat( buf, "YOU!" );
	else if ( victim->in_room == victim->fighting->who->in_room )
	{
	    strcat( buf, PERS( victim->fighting->who, ch ) );
	    strcat( buf, "." );
	}
	else
	    strcat( buf, "someone who left??" );
	break;
    }

    if ( !IS_NPC(victim) && IS_SET(victim->pcdata->flags,PCFLAG_SEVERED ) )
    {
      sprintf(buf,"%s's upper torso is here, twitching.\n",capitalize(victim->name));
    }

    if ( IS_NPC(victim) && IS_SET(victim->acttwo, ACT2_SEVERED ) )
    {
      sprintf(buf,"%s's upper torso is here, twitching.\n",victim->short_descr);
    }

    strcat( buf, "\n\r" );
    buf[0] = UPPER(buf[0]);

  if ( !colormap_check(ch,CMAP_MOB) )
    send_to_char( buf, ch );
  else /* Color Maps --GW */
    send_to_char( colormap_parse(ch,CMAP_MOB,FALSE,NULL,buf), ch);

    show_visible_affects_to_char( victim, ch );
    return;
}



void show_char_to_char_1( CHAR_DATA *victim, CHAR_DATA *ch )
{
    OBJ_DATA *obj;
    int iWear;
    bool found;

    if ( can_see( victim, ch ) )
    {
    act( AT_ACTION, "$n looks at you.", ch, NULL, victim, TO_VICT    );
    act( AT_ACTION, "$n looks at $N.",  ch, NULL, victim, TO_NOTVICT );
    }

    if ( victim->description[0] != '\0' )
    {
	send_to_char( victim->description, ch );
    }
    else
    {
    act( AT_PLAIN, "You see nothing special about $M.", ch, NULL, victim, TO_CHAR );
    }

    show_condition( ch, victim );

    found = FALSE;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	if ( ( obj = get_eq_char( victim, iWear ) ) != NULL
	&&   can_see_obj( ch, obj ) )
	{
	    if ( !found )
	    {
		send_to_char( "\n\r", ch );
		act( AT_PLAIN, "$N is using:", ch, NULL, victim, TO_CHAR );
		found = TRUE;
	    }
	    send_to_char( where_name[iWear], ch );
	    send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
	    send_to_char( "\n\r", ch );
	}
    }

    /*
     * Crash fix here by Thoric
     */
    if ( IS_NPC(ch) || victim == ch )
      return;

    if ( number_percent( ) < ch->pcdata->learned[gsn_peek] )
    {
	send_to_char( "\n\rYou peek at the inventory:\n\r", ch );
	show_list_to_char( victim->first_carrying, ch, TRUE, TRUE );
	learn_from_success( ch, gsn_peek );
    }
    else
      if ( ch->pcdata->learned[gsn_peek] )
        learn_from_failure( ch, gsn_peek );

    return;
}


void show_char_to_char( CHAR_DATA *list, CHAR_DATA *ch )
{
    CHAR_DATA *rch;

    if ( !ch )
       return;

    if ( !list )
       return;

    for ( rch = list; rch; rch = rch->next_in_room )
    {
	if ( !rch )
	continue;

	if ( rch == ch )
	    continue;

	if ( !ch )
		continue;

	if ( can_see( ch, rch ) )
	{
	    show_char_to_char_0( rch, ch );
	}
	else if ( room_is_dark( ch->in_room )
	&&        IS_AFFECTED(rch, AFF_INFRARED ) )
	{
	    set_char_color( AT_BLOOD, ch );
	    send_to_char( "The red form of a living creature is here.\n\r", ch );
	}
    }

    return;
}



bool check_blind( CHAR_DATA *ch )
{
    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT) )
	return TRUE;
	
    if ( IS_AFFECTED(ch, AFF_TRUESIGHT) )
      return TRUE;

    if ( IS_AFFECTED(ch, AFF_BLIND) )
    {
	send_to_char( "You can't see a thing!\n\r", ch );
	return FALSE;
    }

    return TRUE;
}

/*
 * Returns classical DIKU door direction based on text in arg	-Thoric
 */
int get_door( char *arg )
{
    int door;

	 if ( !str_cmp( arg, "n"  ) || !str_cmp( arg, "north"	  ) ) door = 0;
    else if ( !str_cmp( arg, "e"  ) || !str_cmp( arg, "east"	  ) ) door = 1;
    else if ( !str_cmp( arg, "s"  ) || !str_cmp( arg, "south"	  ) ) door = 2;
    else if ( !str_cmp( arg, "w"  ) || !str_cmp( arg, "west"	  ) ) door = 3;
    else if ( !str_cmp( arg, "u"  ) || !str_cmp( arg, "up"	  ) ) door = 4;
    else if ( !str_cmp( arg, "d"  ) || !str_cmp( arg, "down"	  ) ) door = 5;
    else if ( !str_cmp( arg, "ne" ) || !str_cmp( arg, "northeast" ) ) door = 6;
    else if ( !str_cmp( arg, "nw" ) || !str_cmp( arg, "northwest" ) ) door = 7;
    else if ( !str_cmp( arg, "se" ) || !str_cmp( arg, "southeast" ) ) door = 8;
    else if ( !str_cmp( arg, "sw" ) || !str_cmp( arg, "southwest" ) ) door = 9;
    else door = -1;
    return door;
}

void do_look( CHAR_DATA *ch, char *argument )
{
    char arg  [MAX_INPUT_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    EXIT_DATA *pexit;
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *original;
    char *pdesc;
    bool doexaprog; 
    sh_int door;
    int number, cnt;
    char buf[MAX_STRING_LENGTH];

    if ( !ch || !ch->in_room )
      return;

    if ( !IS_NPC(ch) && !ch->desc )
	return;

    if ( ch->position < POS_SLEEPING )
    {
	send_to_char( "You can't see anything but stars!\n\r", ch );
	return;
    }

    if ( ch->position == POS_SLEEPING )
    {
	send_to_char( "You can't see anything, you're sleeping!\n\r", ch );
	return;
    }

    if ( !check_blind( ch ) )
	return;

    if ( !IS_NPC(ch)
    &&   !IS_SET(ch->act, PLR_HOLYLIGHT)
    &&   !IS_AFFECTED(ch, AFF_TRUESIGHT)
    &&   room_is_dark( ch->in_room ) )
    {
	set_char_color( AT_DGREY, ch );
	send_to_char( "It is pitch black ... \n\r", ch );
	show_char_to_char( ch->in_room->first_person, ch );
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    doexaprog = str_cmp( "noprog", arg2 ) && str_cmp( "noprog", arg3 );

    if ( arg1[0] == '\0' || !str_cmp( arg1, "auto" ) )
    {
	/* 'look' or 'look auto' */
	if( !IS_NPC(ch)
	&&  IS_SET(ch->pcdata->flags, PCFLAG_SOUND)
	&&  !str_cmp( arg1, "auto" )
	&&  ch->in_room->music
	&&  str_cmp( ch->in_room->music, "(null)" ) )
	{
	  sprintf( buf, "!!MUSIC(aota%s%s)\n\r", SLASH, ch->in_room->music );
	  send_to_char( buf, ch );
	}
	set_char_color( AT_RMNAME, ch );
     if ( IS_SET(ch->act, PLR_HOLYLIGHT) )
       {
        sprintf(buf,"(%d)[%d] ", ch->in_room->level,ch->in_room->vnum);
	send_to_char( buf, ch );
       }

      if ( !colormap_check(ch,CMAP_ROOMNAME) )
      {
	set_char_color( AT_DGREEN, ch );
	if ( ch->in_room->area && IS_SET(ch->in_room->area->flags,AFLAG_CLUB_HALL) )
	{
	sprintf( buf,"%-45s %-6s",ch->in_room->name,ch->in_room->area->club );
	send_to_char(buf,ch);
	}
	else
	{
	send_to_char( ch->in_room->name, ch );
	}
      }
      else /* Color Maps --GW */
      {
	set_char_color( AT_DGREEN, ch );
	if ( ch->in_room->area && IS_SET(ch->in_room->area->flags,AFLAG_CLUB_HALL) )
	{
	sprintf( buf,"%-45s %-6s",ch->in_room->name,ch->in_room->area->club );
	sprintf( buf,"%s",colormap_parse(ch,CMAP_ROOMNAME,FALSE,NULL,buf));
	send_to_char(buf,ch);
	}
	else
	{
	sprintf(buf,"%s",colormap_parse(ch,CMAP_ROOMNAME,FALSE,NULL,ch->in_room->name));
	send_to_char( buf, ch );
	}
       }

	send_to_char( "\n\r", ch );
	if( ch->in_room->music
	&&  IS_SET(ch->act, PLR_HOLYLIGHT) )
	{
	sprintf(buf,"Room Music: %s\n\r",ch->in_room->music);
	send_to_char(buf,ch);
	}
	set_char_color( AT_PLAIN, ch );
     if ( IS_SET(ch->act, PLR_HOLYLIGHT) )
       {
        send_to_char("[ ",ch);
	ch_printf( ch, "%s", flag_string(ch->in_room->room_flags, r_flags) );
        send_to_char(" ]\n\r",ch);
        ch_printf( ch, "Sector: %s",sector_name(ch->in_room->sector_type));
       }
	send_to_char( "\n\r", ch );

    if ( !colormap_check(ch,CMAP_ROOMDESC) )
    {
	set_char_color( AT_CYAN, ch );
	if ( arg1[0] == '\0'
	|| ( !IS_NPC(ch) && !IS_SET(ch->act, PLR_BRIEF) ) )
	    send_to_char( ch->in_room->description, ch );
    }
    else /* Color Maps --GW */
    {
	if ( arg1[0] == '\0'
	|| ( !IS_NPC(ch) && !IS_SET(ch->act, PLR_BRIEF) ) )
        {
	buf[0]='\0';
	sprintf(buf,"%s",
          colormap_parse( ch,CMAP_ROOMDESC,FALSE,NULL,ch->in_room->description ));
	    send_to_char( buf, ch );
	}
    }

	if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT) )
	    do_exits( ch, "auto" );

	    send_to_char( "\n\r",ch);

	show_list_to_char( ch->in_room->first_content, ch, FALSE, FALSE );
	show_char_to_char( ch->in_room->first_person,  ch );
	return;
    }

    if ( !str_cmp( arg1, "under" ) )
    {
	int count;

	/* 'look under' */
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "Look beneath what?\n\r", ch );
	    return;
	}

	if ( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "You do not see that here.\n\r", ch );
	    return;
	}
	if ( ch->carry_weight + obj->weight > can_carry_w( ch ) )
	{
	    send_to_char( "It's too heavy for you to look under.\n\r", ch );
	    return;
	}
	count = obj->count;
	obj->count = 1;
	act( AT_PLAIN, "You lift $p and look beneath it:", ch, obj, NULL, TO_CHAR );
	act( AT_PLAIN, "$n lifts $p and looks beneath it:", ch, obj, NULL, TO_ROOM );
	obj->count = count;
	if ( IS_OBJ_STAT( obj, ITEM_COVERING ) )
	   show_list_to_char( obj->first_content, ch, TRUE, TRUE );
	else
	   send_to_char( "Nothing.\n\r", ch );
	if ( doexaprog ) oprog_examine_trigger( ch, obj );
	return;
    }

    if ( !str_cmp( arg1, "i" ) || !str_cmp( arg1, "in" ) )
    {
	int count;

	/* 'look in' */
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "Look in what?\n\r", ch );
	    return;
	}

	if ( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "You do not see that here.\n\r", ch );
	    return;
	}

	switch ( obj->item_type )
	{
	default:
	    send_to_char( "That is not a container.\n\r", ch );
	    break;

	case ITEM_DRINK_CON:
	    if ( obj->value[1] <= 0 )
	    {
		send_to_char( "It is empty.\n\r", ch );
	        if ( doexaprog ) oprog_examine_trigger( ch, obj );
		break;
	    }

	    ch_printf( ch, "It's %s full of a %s liquid.\n\r",
		obj->value[1] <     obj->value[0] / 4
		    ? "less than" :
		obj->value[1] < 3 * obj->value[0] / 4
		    ? "about"     : "more than",
		liq_table[obj->value[2]].liq_color
		);

	    if ( doexaprog ) oprog_examine_trigger( ch, obj );
	    break;

	case ITEM_PORTAL:
	    for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
	    {
		if ( pexit->vdir == DIR_PORTAL
		&&   IS_SET(pexit->exit_info, EX_PORTAL) )
		{
		    if ( room_is_private( pexit->to_room )
		    &&   get_trust(ch) < sysdata.level_override_private )
		    {
			set_char_color( AT_WHITE, ch );
			send_to_char( "That room is private buster!\n\r", ch );
			return;
		    }
		   if( !IS_SET( pexit->exit_info, EX_xNO_LOOK ) )
		   {
		    original = ch->in_room;
		    char_from_room( ch );
		    char_to_room( ch, pexit->to_room );
		    do_look( ch, "auto" );
		    char_from_room( ch );
		    char_to_room( ch, original );
		   }
		   else
			send_to_char("You Can't seem to see anything..\n\r",ch);

		    return;
		}
	    }
	    send_to_char( "You see a swirling chaos...\n\r", ch );
	    break;
	case ITEM_CONTAINER:
	case ITEM_QUIVER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
		    if ( IS_SET(obj->value[1], CONT_CLOSED) )
	    {
		send_to_char( "It is closed.\n\r", ch );
		break;
	    }

	    count = obj->count;
	    obj->count = 1;
            sprintf(buf,"$p (%s) contains:", obj->carried_by ? "(carried)" : "(on ground)");
	    act( AT_PLAIN, buf, ch, obj, NULL, TO_CHAR );
	    obj->count = count;
	    show_list_to_char( obj->first_content, ch, TRUE, TRUE );
	    if ( doexaprog ) oprog_examine_trigger( ch, obj );
	    break;
	}
	return;
    }

    if ( (pdesc=get_extra_descr(arg1, ch->in_room->first_extradesc)) != NULL )
    {
	send_to_char( pdesc, ch );
	return;
    }

    door = get_door( arg1 );
    if ( ( pexit = find_door( ch, arg1, TRUE ) ) != NULL )
    {
      if ( pexit->keyword
      &&   pexit->keyword[0] != '\0'
      &&   pexit->keyword[0] != ' ' )
      {
	  if ( IS_SET(pexit->exit_info, EX_CLOSED)
	  &&  !IS_SET(pexit->exit_info, EX_WINDOW) )
	  {
	      if ( IS_SET(pexit->exit_info, EX_SECRET)
	      &&   door != -1 )
		send_to_char( "Nothing special there.\n\r", ch );
	      else
		act( AT_PLAIN, "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
	      return;
	  }
	  if ( IS_SET( pexit->exit_info, EX_BASHED ) )
	      act(AT_RED, "The $d has been bashed from its hinges!",ch, NULL, pexit->keyword, TO_CHAR);
      }

      if ( pexit->description && pexit->description[0] != '\0' )
  	send_to_char( pexit->description, ch );
      else
	send_to_char( "Nothing special there.\n\r", ch );

      /*
       * Ability to look into the next room			-Thoric
       */
      if ( pexit->to_room       &&  !IS_SET( pexit->exit_info, EX_xNO_LOOK )
      && ( IS_AFFECTED( ch, AFF_SCRYING )
      ||   IS_SET( pexit->exit_info, EX_xLOOK )
      ||   get_trust(ch) >= LEVEL_IMMORTAL ) )
      {
        if ( !IS_SET( pexit->exit_info, EX_xLOOK )
        &&    get_trust( ch ) < LEVEL_IMMORTAL )
        {
  	  set_char_color( AT_MAGIC, ch );
	  send_to_char( "You attempt to scry...\n\r", ch );
          /* Change by Narn, Sept 96 to allow characters who don't have the
             scry spell to benefit from objects that are affected by scry.
          */
	  if (!IS_NPC(ch) )
          {
            int percent = ch->pcdata->learned[ skill_lookup("scry") ];
            if ( !percent )
              percent = 55;		/* 95 was too good -Thoric */
 
	    if(  number_percent( ) > percent ) 
	    {
	      send_to_char( "You fail.\n\r", ch );
	      return;
	    }
          }
        }
        if ( room_is_private( pexit->to_room )
        &&   get_trust(ch) < sysdata.level_override_private )
        {
	  set_char_color( AT_WHITE, ch );
	  send_to_char( "That room is private buster!\n\r", ch );
	  return;
        }
        original = ch->in_room;
        char_from_room( ch );
        char_to_room( ch, pexit->to_room );
        do_look( ch, "auto" );
        char_from_room( ch );
        char_to_room( ch, original );
      }
      return;
    }
    else
    if ( door != -1 )
    {
	send_to_char( "Nothing special there.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) != NULL )
    {
	show_char_to_char_1( victim, ch );
	return;
    }


    /* finally fixed the annoying look 2.obj desc bug	-Thoric */
    number = number_argument( arg1, arg );
    for ( cnt = 0, obj = ch->last_carrying; obj; obj = obj->prev_content )
    {
	if ( can_see_obj( ch, obj ) )
	{
	    if ( (pdesc=get_extra_descr(arg, obj->first_extradesc)) != NULL )
	    {
		if ( (cnt += obj->count) < number )
		  continue;
		send_to_char( pdesc, ch );
	        if ( doexaprog ) oprog_examine_trigger( ch, obj );
		return;
	    }

	    if ( (pdesc=get_extra_descr(arg, obj->pIndexData->first_extradesc)) != NULL )
	    {
		if ( (cnt += obj->count) < number )
		  continue;
		send_to_char( pdesc, ch );
	        if ( doexaprog ) oprog_examine_trigger( ch, obj );
		return;
	    }
	    if ( nifty_is_name_prefix( arg, obj->name ) )
	    {
		if ( (cnt += obj->count) < number )
		  continue;
		pdesc = get_extra_descr( obj->name, obj->pIndexData->first_extradesc );
		if ( !pdesc )
		  pdesc = get_extra_descr( obj->name, obj->first_extradesc );
		if ( !pdesc )
		  send_to_char( "You see nothing special.\r\n", ch );
		else
		  send_to_char( pdesc, ch );
		if ( doexaprog ) oprog_examine_trigger( ch, obj );
		  return;
	    }
	}
    }

    for ( obj = ch->in_room->last_content; obj; obj = obj->prev_content )
    {
	if ( can_see_obj( ch, obj ) )
	{
	    if ( (pdesc=get_extra_descr(arg, obj->first_extradesc)) != NULL )
	    {
		if ( (cnt += obj->count) < number )
		  continue;
		send_to_char( pdesc, ch );
	        if ( doexaprog ) oprog_examine_trigger( ch, obj );
		return;
	    }

	    if ( (pdesc=get_extra_descr(arg, obj->pIndexData->first_extradesc)) != NULL )
	    {
		if ( (cnt += obj->count) < number )
		  continue;
		send_to_char( pdesc, ch );
	        if ( doexaprog ) oprog_examine_trigger( ch, obj );
		return;
	    }
	    if ( nifty_is_name_prefix( arg, obj->name ) )
	    {
		if ( (cnt += obj->count) < number )
		  continue;
		pdesc = get_extra_descr( obj->name, obj->pIndexData->first_extradesc );
		if ( !pdesc )
		  pdesc = get_extra_descr( obj->name, obj->first_extradesc );
		if ( !pdesc )
		  send_to_char( "You see nothing special.\r\n", ch );
		else
		  send_to_char( pdesc, ch );
		if ( doexaprog ) oprog_examine_trigger( ch, obj );
		  return;
	    }
	}
    }

    send_to_char( "You do not see that here.\n\r", ch );
    return;
}

void show_condition( CHAR_DATA *ch, CHAR_DATA *victim )
{
    char buf[MAX_STRING_LENGTH];
    int percent;

    if ( victim->max_hit > 0 )
        percent = ( 100 * victim->hit ) / victim->max_hit;
    else
        percent = -1;


    strcpy( buf, PERS(victim, ch) );

         if ( percent >= 100 ) strcat( buf, " is in perfect health.\n\r"  );
    else if ( percent >=  90 ) strcat( buf, " is slightly scratched.\n\r" );
    else if ( percent >=  80 ) strcat( buf, " has a few bruises.\n\r"     );
    else if ( percent >=  70 ) strcat( buf, " has some cuts.\n\r"         );
    else if ( percent >=  60 ) strcat( buf, " has several wounds.\n\r"    );
    else if ( percent >=  50 ) strcat( buf, " has many nasty wounds.\n\r" );
    else if ( percent >=  40 ) strcat( buf, " is bleeding freely.\n\r"    );
    else if ( percent >=  30 ) strcat( buf, " is covered in blood.\n\r"   );
    else if ( percent >=  20 ) strcat( buf, " is leaking guts.\n\r"       );
    else if ( percent >=  10 ) strcat( buf, " is almost dead.\n\r"        );
    else                       strcat( buf, " is DYING.\n\r"              );

    buf[0] = UPPER(buf[0]);
    send_to_char( buf, ch );
    return;
}

/* A much simpler version of look, this function will show you only
the condition of a mob or pc, or if used without an argument, the
same you would see if you enter the room and have config +brief.
-- Narn, winter '96
*/
void do_glance( CHAR_DATA *ch, char *argument )
{
  char arg1 [MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int save_act;

  if ( !ch->desc )
    return;

  if ( ch->position < POS_SLEEPING )
  {
    send_to_char( "You can't see anything but stars!\n\r", ch );
    return;
  }

  if ( ch->position == POS_SLEEPING )
  {
    send_to_char( "You can't see anything, you're sleeping!\n\r", ch );
    return;
  }

  if ( !check_blind( ch ) )
    return;

  argument = one_argument( argument, arg1 );

  if ( arg1[0] == '\0' )
  {
    save_act = ch->act;
    SET_BIT( ch->act, PLR_BRIEF );
    do_look( ch, "auto" );
    ch->act = save_act;
    return;
  }

  if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
  {
    send_to_char( "They're not here.", ch );
    return;
  }
  else
  {
    if ( can_see( victim, ch ) )
    {
      act( AT_ACTION, "$n glances at you.", ch, NULL, victim, TO_VICT    );
      act( AT_ACTION, "$n glances at $N.",  ch, NULL, victim, TO_NOTVICT );
    }

    show_condition( ch, victim );
    return;
  }

  return;
}


void do_examine( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    BOARD_DATA *board;
    sh_int dam;

    if ( !argument )
    {
	bug( "do_examine: null argument.", 0);
	return;
    }

    if ( !ch )
    {
	bug( "do_examine: null ch.", 0);
	return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Examine what?\n\r", ch );
	return;
    }

    sprintf( buf, "%s noprog", arg );
    do_look( ch, buf );

    /*
     * Support for looking at boards, checking equipment conditions,
     * and support for trigger positions by Thoric
     */
    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	if ( (board = get_board( obj )) != NULL )
	{
	   if ( board->num_posts )
	     ch_printf( ch, "There are about %d notes posted here.  Type 'note list' to list them.\n\r", board->num_posts );
	   else
	     send_to_char( "There aren't any notes posted here.\n\r", ch );
	}

	switch ( obj->item_type )
	{
	default:
	    break;

	case ITEM_ARMOR:
	    if ( obj->value[1] == 0 )
	      obj->value[1] = obj->value[0];
	    if ( obj->value[1] == 0 )
	      obj->value[1] = 1;
	    dam = (sh_int) ((obj->value[0] * 10) / obj->value[1]);
	    strcpy( buf, "As you look more closely, you notice that it is ");
	    if (dam >= 10) strcat( buf, "in superb condition.");
       else if (dam ==  9) strcat( buf, "in very good condition.");
       else if (dam ==  8) strcat( buf, "in good shape.");
       else if (dam ==  7) strcat( buf, "showing a bit of wear.");
       else if (dam ==  6) strcat( buf, "a little run down.");
       else if (dam ==  5) strcat( buf, "in need of repair.");
       else if (dam ==  4) strcat( buf, "in great need of repair.");
       else if (dam ==  3) strcat( buf, "in dire need of repair.");
       else if (dam ==  2) strcat( buf, "very badly worn.");
       else if (dam ==  1) strcat( buf, "practically worthless.");
       else if (dam <=  0) strcat( buf, "broken.");
	    strcat( buf, "\n\r" );
	    send_to_char( buf, ch );
	    break;

	case ITEM_WEAPON:
	    dam = INIT_WEAPON_CONDITION - obj->value[0];
	    strcpy( buf, "As you look more closely, you notice that it is ");
	    if (dam ==  0) strcat( buf, "in superb condition.");
       else if (dam ==  1) strcat( buf, "in excellent condition.");
       else if (dam ==  2) strcat( buf, "in very good condition.");
       else if (dam ==  3) strcat( buf, "in good shape.");
       else if (dam ==  4) strcat( buf, "showing a bit of wear.");
       else if (dam ==  5) strcat( buf, "a little run down.");
       else if (dam ==  6) strcat( buf, "in need of repair.");
       else if (dam ==  7) strcat( buf, "in great need of repair.");
       else if (dam ==  8) strcat( buf, "in dire need of repair.");
       else if (dam ==  9) strcat( buf, "very badly worn.");
       else if (dam == 10) strcat( buf, "practically worthless.");
       else if (dam == 11) strcat( buf, "almost broken.");
       else if (dam == 12) strcat( buf, "broken.");
	    strcat( buf, "\n\r" );
	    send_to_char( buf, ch );
	    break;

	case ITEM_FOOD:
	    if ( obj->timer > 0 && obj->value[1] > 0 )
	      dam = (obj->timer * 10) / obj->value[1];
	    else
	      dam = 10;
	    strcpy( buf, "As you examine it carefully you notice that it " );
	    if (dam >= 10) strcat( buf, "is fresh.");
       else if (dam ==  9) strcat( buf, "is nearly fresh.");
       else if (dam ==  8) strcat( buf, "is perfectly fine.");
       else if (dam ==  7) strcat( buf, "looks good.");
       else if (dam ==  6) strcat( buf, "looks ok.");
       else if (dam ==  5) strcat( buf, "is a little stale.");
       else if (dam ==  4) strcat( buf, "is a bit stale.");
       else if (dam ==  3) strcat( buf, "smells slightly off.");
       else if (dam ==  2) strcat( buf, "smells quite rank.");
       else if (dam ==  1) strcat( buf, "smells revolting.");
       else if (dam <=  0) strcat( buf, "is crawling with maggots.");
	    strcat( buf, "\n\r" );
	    send_to_char( buf, ch );
	    break;

	case ITEM_SWITCH:
	case ITEM_LEVER:
	case ITEM_PULLCHAIN:
	    if ( IS_SET( obj->value[0], TRIG_UP ) )
		send_to_char( "You notice that it is in the up position.\n\r", ch );
	    else
		send_to_char( "You notice that it is in the down position.\n\r", ch );
	    break;
	case ITEM_BUTTON:
	    if ( IS_SET( obj->value[0], TRIG_UP ) )
		send_to_char( "You notice that it is depressed.\n\r", ch );
	    else
		send_to_char( "You notice that it is not depressed.\n\r", ch );
	    break;

/* Not needed due to check in do_look already
	case ITEM_PORTAL:
	    sprintf( buf, "in %s noprog", arg );
	    do_look( ch, buf );
	    break;
*/

        case ITEM_CORPSE_PC:
	case ITEM_CORPSE_NPC:
            {
		sh_int timerfrac = obj->timer;
		if ( obj->item_type == ITEM_CORPSE_PC )
		timerfrac = (int)obj->timer / 8 + 1; 

		switch (timerfrac)
		{
		    default:
			send_to_char( "This corpse has recently been slain.\n\r", ch );
			break;
		    case 4:
			send_to_char( "This corpse was slain a little while ago.\n\r", ch );
			break;
		    case 3:
			send_to_char( "A foul smell rises from the corpse, and it is covered in flies.\n\r", ch );
			break;
		    case 2:
			send_to_char( "A writhing mass of maggots and decay, you can barely go near this corpse.\n\r", ch );
			break;
		    case 1:
		    case 0:
			send_to_char( "Little more than bones, there isn't much left of this corpse.\n\r", ch );
			break;
		}
            }
	case ITEM_CONTAINER:
	    if ( IS_OBJ_STAT( obj, ITEM_COVERING ) )
	      break;

	case ITEM_DRINK_CON:
	case ITEM_QUIVER:
	    send_to_char( "When you look inside, you see:\n\r", ch );
	    sprintf( buf, "in %s noprog", arg );
	    do_look( ch, buf );
	}
	if ( IS_OBJ_STAT( obj, ITEM_COVERING ) )
	{
	    sprintf( buf, "under %s noprog", arg );
	    do_look( ch, buf );
	}
	oprog_examine_trigger( ch, obj );
	if( char_died(ch) || obj_extracted(obj) )
	  return;

	check_for_trap( ch, obj, TRAP_EXAMINE );
    }
    return;
}


void do_exits( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    EXIT_DATA *pexit;
    bool found;
    bool fAuto;

    set_char_color( AT_EXITS, ch );
    buf[0] = '\0';
    fAuto  = !str_cmp( argument, "auto" );

    if ( !check_blind( ch ) )
	return;

    strcpy( buf, fAuto ? "Exits:" : "Obvious exits:\n\r" );

    found = FALSE;
    for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
    {
	if ( pexit->to_room 
	&& (!IS_SET(pexit->exit_info, EX_WINDOW)
	||   IS_SET(pexit->exit_info, EX_ISDOOR))
	&&  !IS_SET(pexit->exit_info, EX_HIDDEN) )
	{
	    found = TRUE;
	    if ( fAuto )
	    {
   	 	/* Changed this to show a + if the door is open, - if closed,
		 * by popular demand --GW */
 	        sprintf(buf,"%s%s%s", (buf[0] == '\0') ? " " : buf, 
		     IS_SET(pexit->exit_info, EX_CLOSED) ? " -" : " +",
		     dir_name[pexit->vdir]);
	    }
	    else
	    {
		sprintf( buf + strlen(buf), "%-5s - %s",
		    capitalize( dir_name[pexit->vdir] ),
		    ( room_is_dark( pexit->to_room ) && !IS_IMMORTAL(ch) )
			?  "Too dark to tell"
			: pexit->to_room->name
		    );

		/* Show what vnum it leads too if yer a god --GW */
		if ( IS_IMMORTAL(ch) )
		  sprintf( buf, "%s <#%d>\n\r",buf,pexit->to_room->vnum);
		else
	          strcat(buf,"\n\r");
	    }
	}
    }

    if ( !found )
	strcat( buf, fAuto ? " none]\n\r" : "None\n\r" );
    else
      if ( fAuto )
      {
	if ( !colormap_check(ch,CMAP_EXITS) )
	   send_to_char("[",ch);
	else /* Color Maps --GW */
	   send_to_char(colormap_parse(ch,CMAP_EXITS,FALSE,NULL,"["),ch);

	    strcat( buf,"]" );
      }

    if ( !colormap_check( ch, CMAP_EXITS ) )
      send_to_char( buf, ch );
    else /* Color maps --GW */
    {
      sprintf(buf2,"%s",colormap_parse(ch,CMAP_EXITS,FALSE,NULL,buf));
      send_to_char(buf2,ch);
    }   
    send_to_char( "\n\r", ch );
    return;
}

char *	const	day_name	[] =
{
    "the Moon", "the Bull", "Deception", "Thunder", "Freedom",
    "the Great Gods", "the Sun"
};

char *	const	month_name	[] =
{
    "Winter", "the Winter Wolf", "the Frost Giant", "the Old Forces",
    "the Grand Struggle", "the Spring", "Nature", "Futility", "the Dragon",
    "the Sun", "the Heat", "the Battle", "the Dark Shades", "the Shadows",
    "the Long Shadows", "the Ancient Darkness", "the Great Evil"
};

void do_time( CHAR_DATA *ch, char *argument )
{
    extern char str_boot_time[];
    extern char reboot_time[];
    long int time_passed;
    int hour, minute;
    char *suf;
    int day;

    day     = time_info.day + 1;

	 if ( day > 4 && day <  20 ) suf = "th";
    else if ( day % 10 ==  1       ) suf = "st";
    else if ( day % 10 ==  2       ) suf = "nd";
    else if ( day % 10 ==  3       ) suf = "rd";
    else                             suf = "th";

    set_char_color( AT_YELLOW, ch );
    ch_printf( ch,
	"It is %d o'clock %s, Day of %s, %d%s the Month of %s.\n\r"  
        "The mud started up at:    %s\r"
        "The system time (M.S.T.): %s\r"
        "Next Warmboot Set for:    %s\r",
	(time_info.hour % 12 == 0) ? 12 : time_info.hour % 12,
	time_info.hour >= 12 ? "pm" : "am",
	day_name[day % 7],
	day, suf,
	month_name[time_info.month],
	str_boot_time,
	(char *) ctime( &current_time ),
        reboot_time
          );

	/* ripped Restoretime off for Uptime -- GW */
     time_passed = current_time - boot_time;
     hour = (int) ( time_passed / 3600 );
     minute = (int) ( ( time_passed - ( hour * 3600 ) ) / 60 );
     set_char_color( AT_BLUE, ch );
     ch_printf( ch, "Uptime: %d hours and %d minutes\n\r",hour, minute );

    return;
}



void do_weather( CHAR_DATA *ch, char *argument )
{
    static char * const sky_look[4] =
    {
	"cloudless",
	"cloudy",
	"rainy",
	"lit by flashes of lightning"
    };

    if ( !IS_OUTSIDE(ch) )
    {
	send_to_char( "You can't see the sky from here.\n\r", ch );
	return;
    }

    set_char_color( AT_BLUE, ch );
    ch_printf( ch, "The sky is %s and %s.\n\r",
	sky_look[weather_info.sky],
	weather_info.change >= 0
	? "a warm southerly breeze blows"
	: "a cold northern gust blows"
	);
    return;
}


/*
 * Moved into a separate function so it can be used for other things
 * ie: online help editing				-Thoric
 */
HELP_DATA *get_help( CHAR_DATA *ch, char *argument )
{
    char argall[MAX_INPUT_LENGTH];
    char argone[MAX_INPUT_LENGTH];
    char argnew[MAX_INPUT_LENGTH];
    HELP_DATA *pHelp;
    int lev;

    if ( argument[0] == '\0' )
	argument = "summary";

    if ( isdigit(argument[0]) )
    {
	lev = number_argument( argument, argnew );
	argument = argnew;
    }
    else
	lev = -2;
    /*
     * Tricky argument handling so 'help a b' doesn't match a.
     */
    argall[0] = '\0';
    while ( argument[0] != '\0' )
    {
	argument = one_argument( argument, argone );
	if ( argall[0] != '\0' )
	    strcat( argall, " " );
	strcat( argall, argone );
    }

    for ( pHelp = first_help; pHelp; pHelp = pHelp->next )
    {
	if ( pHelp->level > get_trust( ch ) )
	    continue;
	if ( lev != -2 && pHelp->level != lev )
	    continue;

	if ( is_name( argall, pHelp->keyword ) )
	    return pHelp;
    }

    return NULL;
}


/*
 * Now this is cleaner
 */
void do_help( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;

    if ( (pHelp = get_help( ch, argument )) == NULL )
    {
	send_to_char( "No help on that word.\n\r", ch );
	return;
    }

    if ( pHelp->level >= 0 && str_cmp( argument, "imotd" ) )
    {
	send_to_pager( pHelp->keyword, ch );
	send_to_pager( "\n\r", ch );
    }

    if ( !str_cmp( argument, "imotd") && !IS_IMMORTAL(ch))
    {
	send_to_char("Uh .. No.. Thats for Immorts ... and uh .. YOUR NOT AN IMMORTAL, FOOL!\n\r",ch);
	return;
    }

    /*
     * Strip leading '.' to allow initial blanks.
     */
    if ( pHelp->text[0] == '.' )
    {
	send_to_pager_color( pHelp->text+1, ch );
	send_to_pager_color( pHelp->update, ch );
    }
    else
    {
	send_to_pager_color( pHelp->text  , ch );
	send_to_pager_color( pHelp->update  , ch );
    }
    return;
}

/*
 * Help editor							-Thoric
 */
void do_hedit( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;
    char *strtime;
    char datebuf[MSL];

    if ( !ch->desc )
    {
	send_to_char( "You have no descriptor.\n\r", ch );
	return;
    }

    switch( ch->substate )
    {
	default:
	  break;
	case SUB_HELP_EDIT:
	  if ( (pHelp = ch->dest_buf) == NULL )
	  {
		bug( "hedit: sub_help_edit: NULL ch->dest_buf", 0 );
		stop_editing( ch );
		return;
	  }
	  STRFREE( pHelp->text );
	  pHelp->text = copy_buffer( ch );
	  stop_editing( ch );
          strtime                    = ctime(&current_time);
          strtime[strlen(strtime)-9] = '\0';
          sprintf(datebuf,"\n      -=-=-=-=- Updated: %s MST, by: %s -=-=-=-=-\n\r\n\r",
                     strtime, capitalize(ch->name));
	  pHelp->update = STRALLOC(datebuf);
	  return;
    }
    if ( (pHelp = get_help( ch, argument )) == NULL )	/* new help */
    {
	char argnew[MAX_INPUT_LENGTH];
	int lev;

	if ( isdigit(argument[0]) )
	{
	    lev = number_argument( argument, argnew );
	    argument = argnew;
	}
	else
	    lev = get_trust(ch);
	CREATE( pHelp, HELP_DATA, 1 );
	pHelp->keyword = STRALLOC( strupper(argument) );
	pHelp->text    = STRALLOC( "" );
	pHelp->level   = lev;
	add_help( pHelp );
    }
    ch->substate = SUB_HELP_EDIT;
    ch->dest_buf = pHelp;
    start_editing( ch, pHelp->text );
}

/*
 * Stupid leading space muncher fix				-Thoric
 */
char *help_fix( char *text )
{
    char *fixed;

    if ( !text )
      return "";
    fixed = strip_cr(text);
    if ( fixed[0] == ' ' )
      fixed[0] = '.';
    return fixed;
}

void do_hset( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Syntax: hset <field> [value] [help page]\n\r",	ch );
	send_to_char( "\n\r",						ch );
	send_to_char( "Field being one of:\n\r",			ch );
	send_to_char( "  level keyword remove save\n\r",		ch );
	return;
    }

    if ( !str_cmp( arg1, "save" ) )
    {
	FILE *fpout;

	log_string_plus( "Saving help.are...", LOG_NORMAL, LEVEL_GOD );

	rename( "zone1/help.are", "zone1/help.are.bak" );
//	new_fclose( fpReserve );
	if ( ( fpout = fopen( "zone1/help.are", "w" ) ) == NULL )
	{
	   bug( "hset save: fopen", 0 );
	   perror( "help.are" );
//	   fpReserve = fopen( NULL_FILE, "r" );
	   return;
	}
    
	fprintf( fpout, "#HELPS\n\n" );
	for ( pHelp = first_help; pHelp; pHelp = pHelp->next )
	    fprintf( fpout, "%d %s~\n%s~\n%s~\n\n",
			pHelp->level, pHelp->keyword,pHelp->update,help_fix(pHelp->text) );

	fprintf( fpout, "0 $~\n\n\n#$\n" );
	new_fclose( fpout );
//	fpReserve = fopen( NULL_FILE, "r" );
	send_to_char( "Saved.\n\r", ch );
	return;
    }
    if ( str_cmp( arg1, "remove" ) )
	argument = one_argument( argument, arg2 );

    if ( (pHelp = get_help( ch, argument )) == NULL )
    {
	send_to_char( "Cannot find help on that subject.\n\r", ch );
	return;
    }
    if ( !str_cmp( arg1, "remove" ) )
    {
	UNLINK( pHelp, first_help, last_help, next, prev );
	STRFREE( pHelp->text );
	STRFREE( pHelp->keyword );
	DISPOSE( pHelp );
	send_to_char( "Removed.\n\r", ch );
	return;
    }
    if ( !str_cmp( arg1, "level" ) )
    {
	pHelp->level = atoi( arg2 );
	send_to_char( "Done.\n\r", ch );
	return;
    }
    if ( !str_cmp( arg1, "keyword" ) )
    {
	STRFREE( pHelp->keyword );
	pHelp->keyword = STRALLOC( strupper(arg2) );
	send_to_char( "Done.\n\r", ch );
	return;
    }

    do_hset( ch, "" );
}

/*
 * Show help topics in a level range				-Thoric
 * Idea suggested by Gorog
 */
void do_hlist( CHAR_DATA *ch, char *argument )
{
    int min, max, minlimit, maxlimit, cnt;
    char arg[MAX_INPUT_LENGTH];
    HELP_DATA *help;

    maxlimit = get_trust(ch);
    minlimit = maxlimit >= LEVEL_GOD ? -1 : 0;
    argument = one_argument( argument, arg );
    if ( arg[0] != '\0' )
    {
	min = URANGE( minlimit, atoi(arg), maxlimit );
	if ( argument[0] != '\0' )
	    max = URANGE( min, atoi(argument), maxlimit );
	else
	    max = maxlimit;
    }
    else
    {
	min = minlimit;
	max = maxlimit;
    }
    set_pager_color( AT_GREEN, ch );
    pager_printf( ch, "Help Topics in level range %d to %d:\n\r\n\r", min, max );
    for ( cnt = 0, help = first_help; help; help = help->next )
	if ( help->level >= min && help->level <= max )
	{
	    pager_printf( ch, "  %3d %s\n\r", help->level, help->keyword );
	    ++cnt;
	}
    if ( cnt )
	pager_printf( ch, "\n\r%d pages found.\n\r", cnt );
    else
	send_to_char( "None found.\n\r", ch );
}


/* 
 * New do_who with WHO REQUEST, clan, race and homepage support.  -Thoric
 *
 * Latest version of do_who eliminates redundant code by using linked lists.
 * Shows imms separately, indicates guest and retired immortals.
 * Narn, Oct/96 -- REVAMPED by Greywolf -- April/97
 */
void do_who( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char clan_name[MAX_INPUT_LENGTH];
    char council_name[MAX_INPUT_LENGTH];
    char invis_str[MAX_INPUT_LENGTH];
    char char_name[MAX_INPUT_LENGTH];
    char *extra_title;
    char class_text[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    int iClass, iRace;
    int iLevelLower;
    int iLevelUpper;
    int nNumber;
    int nMatch;
    bool rgfClass[MAX_CLASS];
    bool rgfRace[MAX_RACE];
    bool fClassRestrict;
    bool fRaceRestrict;
    bool fImmortalOnly;
    bool fPkill;
    bool fShowHomepage;
    bool fClanMatch; /* SB who clan (order),who guild, and who council */
    bool fCouncilMatch;
    bool fDeityMatch;
    bool fLevelShow=FALSE;
    CLAN_DATA *pClan=NULL;
    COUNCIL_DATA *pCouncil=NULL;
    DEITY_DATA *pDeity=NULL;
    FILE *whoout=NULL;
    char arg[MAX_STRING_LENGTH];
    char *class1;
    char *class2;
    char *class3;
    char *class4;
    char guild_name[MSL];

    /*
    #define WT_IMM    0;
    #define WT_MORTAL 1;
    #define WT_DEADLY 2;
    */

    WHO_DATA *cur_who = NULL;
    WHO_DATA *next_who = NULL;
    WHO_DATA *first_mortal = NULL;
    WHO_DATA *first_imm = NULL;


    /*
     * Set default arguments.
     */
    iLevelLower    = 0;
    iLevelUpper    = MAX_LEVEL;
    fClassRestrict = FALSE;
    fRaceRestrict  = FALSE;
    fImmortalOnly  = FALSE;
    fPkill         = FALSE;
    fShowHomepage  = FALSE;
    fClanMatch	   = FALSE; /* SB who clan (order), who guild, who council */
    fCouncilMatch  = FALSE;
    fDeityMatch    = FALSE;
    for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
	rgfClass[iClass] = FALSE;
    for ( iRace = 0; iRace < MAX_RACE; iRace++ )
	rgfRace[iRace] = FALSE;

    /*
     * Parse arguments.
     */
    nNumber = 0;
    for ( ;; )
    {

	argument = one_argument( argument, arg );
	if ( arg[0] == '\0' )
	    break;

	if ( is_number( arg ) )
	{
	    switch ( ++nNumber )
	    {
	    case 1: iLevelLower = atoi( arg ); break;
	    case 2: iLevelUpper = atoi( arg ); break;
	    default:
		send_to_char( "Only two level numbers allowed.\n\r", ch );
		return;
	    }
	}
	else if ( arg[0] != '\0' && !str_cmp(arg, "l" ) ) /* L Option --GW*/
	{
	    fLevelShow = TRUE;
	}
	else
	{

	    if ( strlen(arg) < 2 )
	    {
		send_to_char( "Classes must be longer than that.\n\r", ch );
		return;
	    }

	    /*
	     * Look for classes to turn on.
	     */
            if ( arg != '\0' )
	    {
            if ( !str_cmp( arg, "deadly" ) || !str_cmp( arg, "pkill" ) )
              fPkill = TRUE;
            else
	    if ( !str_cmp( arg, "imm" ) || !str_cmp( arg, "gods" ) )
		fImmortalOnly = TRUE;
	    else
	    if ( !str_cmp( arg, "www" ) )
		fShowHomepage = TRUE;
            else		 /* SB who clan (order), guild, council */
             if  ( ( pClan = get_clan (arg) ) )
	   	fClanMatch = TRUE;
            else
             if ( ( pCouncil = get_council (arg) ) )
                fCouncilMatch = TRUE;
	    else
	     if ( ( pDeity = get_deity (arg) ) )
		fDeityMatch = TRUE;
	    }
	    else
	    {
		for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
		{
		    if ( !str_cmp( arg, class_table[iClass]->who_name ) )
		    {
			rgfClass[iClass] = TRUE;
			break;
		    }
		}
		if ( iClass != MAX_CLASS )
		  fClassRestrict = TRUE;

		for ( iRace = 0; iRace < MAX_RACE; iRace++ )
		{
		    if ( !str_cmp( arg, race_table[iRace]->race_name ) )
		    {
			rgfRace[iRace] = TRUE;
			break;
		    }
		}
		if ( iRace != MAX_RACE )
		  fRaceRestrict = TRUE;

		if ( iClass == MAX_CLASS && iRace == MAX_RACE 
 		 && fClanMatch == FALSE 
                 && fCouncilMatch == FALSE
		 && fDeityMatch == FALSE )
		{
		 send_to_char( "That's not a class, race, order, guild,"
			" council or deity.\n\r", ch );
		    return;
		}
	    }
	}
    }

    /*
     * Now find matching chars.
     */
    nMatch = 0;
    buf[0] = '\0';
    if ( ch )
	set_pager_color( AT_GREEN, ch );
    else
    {
	if ( fShowHomepage )
	  whoout = fopen( WEBWHO_FILE, "w" );
	else
	  whoout = fopen( WHO_FILE, "w" );
    }

/* start from last to first to get it in the proper order */
    for ( d = last_descriptor; d; d = d->prev )
    {
	CHAR_DATA *wch;
	char const *class;

	if ( (d->connected != CON_PLAYING && d->connected != CON_EDITING)
	||   !can_see( ch, d->character ) || d->original)
	    continue;
	wch   = d->original ? d->original : d->character;
	if ( wch->level < iLevelLower
	||   wch->level > iLevelUpper
	|| ( fPkill && !CAN_PKILL( wch ) ) 
	|| ( fImmortalOnly  && wch->level < LEVEL_IMMORTAL )
	|| ( fClassRestrict && !rgfClass[wch->class] )
	|| ( fRaceRestrict && !rgfRace[wch->race] )
	|| ( fClanMatch && ( pClan != wch->pcdata->clan ))  /* SB */
        || ( fCouncilMatch && ( pCouncil != wch->pcdata->council )) /* SB */ 
	|| ( fDeityMatch && ( pDeity != wch->pcdata->deity )) )
	    continue;

	nMatch++;

	if ( fShowHomepage
	&&   wch->pcdata->homepage
	&&   wch->pcdata->homepage[0] != '\0' )
	  sprintf( char_name, "<A HREF=\"%s\">%s</A>",
		show_tilde( wch->pcdata->homepage ), wch->name );
	else
	  strcpy( char_name, wch->name );

	sprintf( class_text, "%s %s", IS_SET(wch->pcdata->flags,
	PCFLAG_UNAUTHED) ? "N" : " ", class_table[wch->class]->who_name );
	class = class_text;
	switch ( wch->level )
	{
	default: break;
/*60*/	case MAX_LEVEL -  0: class = "HYPERION";break;
/*59*/	case MAX_LEVEL -  1: class = "COMMITTEE";break;
/*58*/	case MAX_LEVEL -  2: class = "COUNCIL";break;
/*57*/	case MAX_LEVEL -  3: class = "HEAD GOD";break;
/*56*/	case MAX_LEVEL -  4: class = "GOD";	break;
/*55*/	case MAX_LEVEL -  5: class = "FATE";	break;
/*54*/	case MAX_LEVEL -  6: class = "TITAN";	break;
/*53*/  case MAX_LEVEL -  7: class = "LORD";	break;
/*52*/	case MAX_LEVEL -  8: class = "WIZARD";	break;
/*51*/	case MAX_LEVEL -  9: class = "EMPATH";	break;
	}

        if ( !str_cmp( wch->name, sysdata.guild_overseer ) )
          extra_title = "";
        else if ( !str_cmp( wch->name, sysdata.guild_advisor ) )
          extra_title = "";
        else
          extra_title = "";
 
        if ( IS_RETIRED( wch ) && wch->level < 59 )
          class = "Retired"; 
        else if ( IS_GUEST( wch ) )
          class = "Guest"; 
	else if ( wch->pcdata->rank && wch->pcdata->rank[0] != '\0' )
	  class = wch->pcdata->rank;

	if ( wch->pcdata->clan )
	{
          CLAN_DATA *pclan = wch->pcdata->clan;
	  if ( pclan->clan_type == CLAN_GUILD )
	    strcpy( clan_name, "" );
	  else
	    strcpy( clan_name, "" );

          if ( pclan->clan_type == CLAN_ORDER )
          {
	    if ( !str_cmp( wch->name, pclan->deity ) )
	      strcat( clan_name, "" );
            else
	    if ( !str_cmp( wch->name, pclan->leader ) )
	      strcat( clan_name, "" );
	    else
	      strcat( clan_name, "" );
          }
          else
	  if ( pclan->clan_type == CLAN_GUILD )
	  {
	    if ( !str_cmp( wch->name, pclan->leader ) )
              strcat( clan_name, "" );
	  }
	  else
          {
	    if ( !str_cmp( wch->name, pclan->deity ) )
	      strcat( clan_name, "" );
	    else
	    if ( !str_cmp( wch->name, pclan->leader ) )
	      strcat( clan_name, "" );
          } 
	  strcat( clan_name, pclan->whoname );
	  if ( pclan->clan_type == CLAN_GUILD )
	    strcat( clan_name, "" );
	  else
	    strcat( clan_name, "" );
	}
	else
	  clan_name[0] = '\0';

	if ( wch->pcdata->council )
	{
          strcpy( council_name, " [" );
          if ( !str_cmp( wch->name, wch->pcdata->council->head ) )
            strcat( council_name, "Head of " );
          strcat( council_name, wch->pcdata->council_name );
          strcat( council_name, "]" );
	}
	else
          council_name[0] = '\0';

	guild_name[0] = '\0';

        if ( wch->pcdata->guild )
          sprintf(guild_name,"&z[&w%-3s&z]&w",wch->pcdata->guild->whoname);



if ( !fLevelShow )
{
	if ( IS_SET(wch->act, PLR_WIZINVIS) )
	  sprintf( invis_str, "&R(&W%d&R)&W", wch->pcdata->wizinvis );
	else
	  invis_str[0] = '\0';

if ( !IS_IMMORTAL(wch) )
{
	sprintf( buf,
"&R[%-10s&R]&W%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n\r",
          clan_name,
	  invis_str,
	  (d->idle >= 1200) ? "&G(&RIDLE&G)&W " : "",
          IS_AFFECTED(wch, AFF_INVISIBLE) ? "&B(&WInvis&B)&W ": "",
 	  IS_SET(wch->pcdata->flags, PCFLAG_DEADLY) ? "&G(&RPK&G)&W " : "",
	  IS_SET(wch->pcdata->flagstwo, MOREPC_HOARDER) ? "&G[&RHOARDER&G]&W " : "",
	  IS_SET(wch->pcdata->flagstwo, MOREPC_WAR) ? "&G[&RWAR&G]&W " : "",
	  IS_SET(wch->act, PLR_AFK) ? "&G[&RAFK&G]&W " : "",
	  IS_SET(wch->act, PLR_KILLER) ? "&G(&RKILLER&G)&W " : "",
	  IS_SET(wch->act, PLR_THIEF)  ? "&G(&RTHIEF&G)&W "  : "",
	  IS_SET(wch->pcdata->flags, PCFLAG_WED) ? "&Y[&WWED&Y]&W " : "",
	  IS_SET(wch->pcdata->flags, PCFLAG_ENGAGED) ? "&Y[&WENGAGED&Y]&W " : "",
	  IS_AFFECTED(wch,AFF_BOUNTY) ? "&G**<&RBOUNTY&G>**&W " : "",
	  IS_SET(wch->pcdata->flags, PCFLAG_ASSASSIN) ? "&G(&RASSASSIN&G)&W " : "",
	  IS_SET(wch->pcdata->flags, PCFLAG_CHEATER) ? "&G(&RCHEATER&G)&W " : "",
	  IS_SET(wch->pcdata->flags, PCFLAG_KOMBAT) ? "&G[&RKOMBAT&G]&W " : "",
	  IS_SET(wch->pcdata->flags, PCFLAG_CHALLENGER) ? "&R[&rARENA&R]&W ": "",
  	  IS_SET(wch->pcdata->flags, PCFLAG_CHALLENGED) ? "&R[&rARENA&R]&W ": "",
	  IS_SET(wch->pcdata->flagstwo, MOREPC_OWNED) ? "&B[&pOWNED&B]&W " : "",
	  IS_SET(wch->pcdata->flagstwo, MOREPC_CTF) ? "&G[&RCTF&G]&W " : "",
	  IS_SET(wch->pcdata->flagstwo, MOREPC_KNIGHT) ? "&G[&RKNIGHT&G]&W " : "",
	  wch->pcdata->team == TEAM_RED ? "&R[RED]&W" : "",
	  wch->pcdata->team == TEAM_BLUE ? "&B[BLUE]&W" : "",
	  wch->pcdata->team == RED_FLAG_CARRIER ? "&R[HAS RED FLAG]&W" : "",
	  wch->pcdata->team == BLUE_FLAG_CARRIER ? "&B[HAS BLUE FLAG]&W" : "",
	  guild_name[0] != '\0' ? guild_name : "",
	  wch->pcdata->title );
}
else
{

        sprintf(
buf,"&R[&W%-10s&R]&W%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n\r",
          class,
          invis_str,
	  (d->idle >= 1200) ? "&G(&RIDLE&G)&W " : "",
          IS_AFFECTED(wch, AFF_INVISIBLE) ? "&B(&WInvis&B)&W ": "",
 	  IS_SET(wch->pcdata->flags, PCFLAG_DEADLY) ? "&G(&RPK&G)&W" : "",
	  IS_SET(wch->pcdata->flagstwo, MOREPC_WAR) ? "&G[&RWAR&G]&W " : "",
	  IS_SET(wch->act, PLR_AFK) ? "&G[&RAFK&G]&W " : "",
	  IS_SET(wch->act, PLR_KILLER) ? "&G(&RKILLER&G)&W " : "",
	  IS_SET(wch->act, PLR_THIEF)  ? "&G(&RTHIEF&G)&W "  : "",
	  IS_SET(wch->pcdata->flags, PCFLAG_WED) ? "&Y[&WWED&Y]&W " : "",
	  IS_SET(wch->pcdata->flags, PCFLAG_ENGAGED) ? "&Y[&WENGAGED&Y]&W " : "",
	  IS_AFFECTED(wch,AFF_BOUNTY) ? "&G**<&RBOUNTY&G>**&W " : "",
	  IS_SET(wch->pcdata->flags, PCFLAG_ASSASSIN) ? "&G(&RASSASSIN&G)&W " : "",
	  IS_SET(wch->pcdata->flags, PCFLAG_CHEATER) ? "&G(&RCHEATER&G)&W " : "",
	  IS_SET(wch->pcdata->flags, PCFLAG_KOMBAT) ? "&G[&RKOMBAT&G]&W " : "",
          IS_SET(wch->pcdata->flags, PCFLAG_CHALLENGER) ? "&R[&rARENA&R]&W ": "",
          IS_SET(wch->pcdata->flags, PCFLAG_CHALLENGED) ? "&R[&rARENA&R]&W ": "",
	  IS_SET(wch->pcdata->flagstwo, MOREPC_OWNED) ? "&B[&pOWNED&B]&W " : "",
	  guild_name[0] != '\0' ? guild_name : "",
	  wch->pcdata->title );
}
}

if ( fLevelShow ) /* Who L (level listing) --GW */
{
	if ( IS_SET(wch->act, PLR_WIZINVIS) )
	  sprintf( invis_str, "&R(&W%d&R)&W", wch->pcdata->wizinvis );
	else
	  invis_str[0] = '\0';

class1 = STRALLOC(capitalize(get_class(wch)));
class2 = STRALLOC(capitalize(get_class2(wch)));
class3 = STRALLOC(capitalize(get_advclass(wch)));
class4 = STRALLOC(capitalize(get_advclass2(wch)));

if ( !IS_IMMORTAL(wch) )
{
	sprintf( buf,
"&w%-13s -- &R[&B%2.2d&R/&B%2.2d&R/&B%2.2d&R/&B%2.2d %s&R/&B%s&R/&B%s&R/&B%s&R]&W\n\r",
  	  capitalize(wch->name),
	  wch->level,
	  wch->level2,
	  wch->advlevel,
	  wch->advlevel2,
	  class1,
	  class2,
	  class3,
	  class4);
}
else
{

        sprintf(
buf,"&w%-13s -- &RIMMORTAL &B-- &RRANK: &G[&R%-10s&G]&W\n\r",
	  capitalize(wch->name),
          class);
}
}

          /*  
           * This is where the old code would display the found player to the ch.
           * What we do instead is put the found data into a linked list
           */ 

          /* First make the structure. */
	  cur_who=NULL;
          CREATE( cur_who, WHO_DATA, 1 );
          cur_who->text = str_dup( buf );
          if (wch->level >= 51 )
            cur_who->type = WT_IMM;
          else if ( CAN_PKILL( wch ) ) 
            cur_who->type = WT_MORTAL;
          else
            cur_who->type = WT_MORTAL;

	  /* Rp Seperation --GW */
	   if ( IS_SET( wch->pcdata->flagstwo, MOREPC_IC) && !IS_IMMORTAL(wch) )
	    cur_who->ic = 1;
	   else
	    cur_who->ic = 0;

          /* Then put it into the appropriate list. */
          switch ( cur_who->type )
          {
            case WT_MORTAL:
              cur_who->next = first_mortal;
              first_mortal = cur_who;
              break;
            case WT_IMM:
              cur_who->next = first_imm;
              first_imm = cur_who;
              break;
          }

    }


    /* Ok, now we have three separate linked lists and what remains is to 
     * display the information and clean up.
     */

if ( !ch )
{
fprintf(whoout,"&W		 &G[&RPlayers Currently In Character&G]&W\n\r");
fprintf(whoout,"&W               &R  ==============================&W\n\r\n\r");		
}
else
{
send_to_pager("&W		 &G[&RPlayers Currently In Character&G]&W\n\r",ch);
send_to_pager("&W                &R  ==============================&W\n\r\n\r",ch);		
}


    for ( cur_who = first_mortal; cur_who; cur_who = next_who )
    {

      next_who = cur_who->next;
      if ( cur_who->ic == 0 )
	continue;   

      if ( !ch )
        fprintf( whoout, cur_who->text );
      else
        send_to_pager( cur_who->text, ch );
    } 

if ( !ch )
{
fprintf(whoout,"&W\n\r		 &G[&RPlayers Currently Out of Character&G]&W\n\r");
fprintf(whoout,"&W                &R ==================================&W\n\r\n\r");		
}
else
{
send_to_pager("&W\n\r		 &G[&RPlayers Currently Out of Character&G]&W\n\r",ch);
send_to_pager("&W                &R  ==================================&W\n\r\n\r",ch);		
}

    for ( cur_who = first_mortal; cur_who; cur_who = next_who )
    {
      next_who = cur_who->next;
      if ( cur_who->ic == 1 )
	continue;

      if ( !ch )
        fprintf( whoout, cur_who->text );
      else
        send_to_pager( cur_who->text, ch );
    } 

    /* Free it all --GW */
    for ( cur_who = first_mortal; cur_who; cur_who = next_who )
    {
      next_who = cur_who->next;
      DISPOSE( cur_who->text );
      if ( cur_who )
      DISPOSE( cur_who ); 
    }
     

if ( !ch )
{
fprintf(whoout,"&W\n\r		 &G[&RCurrent Dungeon Masters Visable to you&G]&W\n\r");
fprintf(whoout,"&W                 &R======================================&W\n\r\n\r");
}
else
{
send_to_pager("&W\n\r		 &G[&RCurrent Dungeon Masters Visable to you&G]&W\n\r",ch);
send_to_pager("&W                  &R======================================&W\n\r\n\r",ch);		
}

    for ( cur_who = first_imm; cur_who; cur_who = next_who )
    {
      if ( !ch )
        fprintf( whoout, cur_who->text );
      else
        send_to_pager( cur_who->text, ch );
      next_who = cur_who->next;
      DISPOSE( cur_who->text );
	if ( cur_who )
      DISPOSE( cur_who ); 
    } 

    if ( !ch )
    {
	fprintf( whoout, "\n\r%d player%s.\n\r", nMatch, nMatch == 1 ? "" : "s" );
	new_fclose( whoout );
	return;
    }

    set_char_color( AT_YELLOW, ch );
ch_printf( ch,"\n\r");
ch_printf( ch, "&G( &R%d&W Player%s Connected &G| &R%d &WLogins &G| &R%d &WPlayers at Peak &G)&W",
 nMatch, nMatch == 1 ? "" : "s",sysdata.logins,sysdata.maxplayers );
ch_printf( ch,"\n\r");
    return;
}


void do_compare( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj1;
    OBJ_DATA *obj2;
    int value1;
    int value2;
    char *msg;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Compare what to what?\n\r", ch );
	return;
    }

    if ( ( obj1 = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( arg2[0] == '\0' )
    {
	for ( obj2 = ch->first_carrying; obj2; obj2 = obj2->next_content )
	{
	    if ( obj2->wear_loc != WEAR_NONE
	    &&   can_see_obj( ch, obj2 )
	    &&   obj1->item_type == obj2->item_type
	    && ( obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0 )
		break;
	}

	if ( !obj2 )
	{
	    send_to_char( "You aren't wearing anything comparable.\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( obj2 = get_obj_carry( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}
    }

    msg		= NULL;
    value1	= 0;
    value2	= 0;

    if ( obj1 == obj2 )
    {
	msg = "You compare $p to itself.  It looks about the same.";
    }
    else if ( obj1->item_type != obj2->item_type )
    {
	msg = "You can't compare $p and $P.";
    }
    else
    {
	switch ( obj1->item_type )
	{
	default:
	    msg = "You can't compare $p and $P.";
	    break;

	case ITEM_ARMOR:
	    value1 = obj1->value[0];
	    value2 = obj2->value[0];
	    break;

	case ITEM_WEAPON:
	    value1 = obj1->value[1] + obj1->value[2];
	    value2 = obj2->value[1] + obj2->value[2];
	    break;
	}
    }

    if ( !msg )
    {
	     if ( value1 == value2 ) msg = "$p and $P look about the same.";
	else if ( value1  > value2 ) msg = "$p looks better than $P.";
	else                         msg = "$p looks worse than $P.";
    }

    act( AT_PLAIN, msg, ch, obj1, obj2, TO_CHAR );
    return;
}



void do_where( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    bool found;

    one_argument( argument, arg );

    set_pager_color( AT_PERSON, ch );
    if ( arg[0] == '\0' )
    {
	if (!IS_IMMORTAL(ch)) {
        pager_printf( ch, "Players near you in %s:\n\r", ch->in_room->area->name );
	} else {
        pager_printf( ch, "You are in %s:\n\r", ch->in_room->area->name );
        }
	found = FALSE;
	for ( d = first_descriptor; d; d = d->next )
	    if ( (d->connected == CON_PLAYING || d->connected == CON_EDITING )
	    && ( victim = d->character ) != NULL
	    &&   !IS_NPC(victim)
	    &&   victim->in_room
	    &&   (( !IS_IMMORTAL(ch) && victim->in_room->area == ch->in_room->area) || (IS_IMMORTAL(ch)))
	    &&   can_see( ch, victim ) )
	    {
		found = TRUE;
                if ( IS_IMMORTAL(ch) )
                 {
                  pager_printf( ch, "%-28s %s <#%d>\n\r",
                    victim->name,
                    victim->in_room->name,
                    victim->in_room->vnum );
                 }
                else {
		pager_printf( ch, "%-28s %s\n\r",
		    victim->name,
                    victim->in_room->name );
                      }
	    }
	if ( !found )
	    send_to_char( "None\n\r", ch );
    }
    else
    {
	found = FALSE;
	for ( victim = first_char; victim; victim = victim->next )
	    if ( victim->in_room
	    &&   victim->in_room->area == ch->in_room->area
	    &&   !IS_AFFECTED(victim, AFF_HIDE)
	    &&   !IS_AFFECTED(victim, AFF_SNEAK)
	    &&   can_see( ch, victim )
	    &&   is_name( arg, victim->name ) )
	    {
		found = TRUE;
		pager_printf( ch, "%-28s %s\n\r",
		    PERS(victim, ch), victim->in_room->name );
		break;
	    }
	if ( !found )
	    act( AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
    }

    return;
}




void do_consider( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char *msg;
    int diff;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Consider killing whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They're not here.\n\r", ch );
	return;
    }

    diff = victim->level - ch->level;

	 if ( diff <= -10 ) msg = "You are far more experienced than $N.";
    else if ( diff <=  -5 ) msg = "$N is not nearly as experienced as you.";
    else if ( diff <=  -2 ) msg = "You are more experienced than $N.";
    else if ( diff <=   1 ) msg = "You are just about as experienced as $N.";
    else if ( diff <=   4 ) msg = "You are not nearly as experienced as $N.";
    else if ( diff <=   9 ) msg = "$N is far more experienced than you!";
    else                    msg = "$N would make a great teacher for you!";
    act( AT_CONSIDER, msg, ch, NULL, victim, TO_CHAR );

    diff = (int) (victim->max_hit - ch->max_hit) / 6;

	 if ( diff <= -200) msg = "$N looks like a feather!";
    else if ( diff <= -150) msg = "You could kill $N with your hands tied!";
    else if ( diff <= -100) msg = "Hey! Where'd $N go?";
    else if ( diff <=  -50) msg = "$N is a wimp.";
    else if ( diff <=    0) msg = "$N looks weaker than you.";
    else if ( diff <=   50) msg = "$N looks about as strong as you.";
    else if ( diff <=  100) msg = "It would take a bit of luck...";
    else if ( diff <=  150) msg = "It would take a lot of luck, and equipment!";
    else if ( diff <=  200) msg = "Why don't you dig a grave for yourself first?";
    else                    msg = "$N is built like a TANK!";
    act( AT_CONSIDER, msg, ch, NULL, victim, TO_CHAR );

    return;
}


/*
 * Place any skill types you don't want them to be able to practice
 * normally in this list.  Separate each with a space.
 * (Uses an is_name check). -- Altrag
 */
#define CANT_PRAC "Tongue"

void do_practice( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int sn;

    if ( IS_NPC(ch) )
	return;

    if ( argument[0] == '\0' )
    {
	int	col;
	sh_int	lasttype, cnt;
	buf[0] = '\0';

	col = cnt = 0;	lasttype = SKILL_SPELL;
	set_pager_color( AT_MAGIC, ch );
	for ( sn = 0; sn < top_sn; sn++ )
	{
	    if ( !skill_table[sn]->name )
		break;

	    if ( skill_table[sn]->type != lasttype )
	    {
		if ( col % 3 != 0 )
		    send_to_pager( "\n\r", ch );
		pager_printf( ch,
"------------------------------[ &C%ss&B ]--------------------------------\n\r",
			 skill_tname[skill_table[sn]->type]);
		col = cnt = 0;
	    }
	    lasttype = skill_table[sn]->type;

/* We never use guild skills anyway --GW
	    if (!IS_IMMORTAL(ch) 
	    && ( skill_table[sn]->guild != CLASS_NONE 
	       && ( !IS_GUILDED(ch)
		  || (ch->pcdata->clan->class != skill_table[sn]->guild) ) ) )
		continue;
*/
	    if ( ch->level < skill_table[sn]->skill_level[ch->class]
	    || (!IS_IMMORTAL(ch) &&
           skill_table[sn]->skill_level[ch->class] == 0))  
            if ( ch->level2 < skill_table[sn]->skill_level[ch->class2]
	    || (!IS_IMMORTAL(ch) &&
           skill_table[sn]->skill_level[ch->class2] == 0)) 
            if ( ch->advlevel < skill_table[sn]->skill_level[ch->advclass]
	    || (!IS_IMMORTAL(ch) &&
           skill_table[sn]->skill_level[ch->advclass] == 0)) 
            if ( ch->advlevel2 < skill_table[sn]->skill_level[ch->advclass2]
	    || (!IS_IMMORTAL(ch) &&
           skill_table[sn]->skill_level[ch->advclass] == 0)) 
            if ( ch->level < skill_table[sn]->race_skill_level[ch->race]
	    || (!IS_IMMORTAL(ch) &&
           skill_table[sn]->race_skill_level[ch->race] == 0)) 
          continue;

	    if ( ch->pcdata->learned[sn] == 0
	    &&   SPELL_FLAG(skill_table[sn], SF_SECRETSKILL) )
		continue;

	    ++cnt;
           if( ch->pcdata->learned[sn] > 0 )
	      { pager_printf( ch, "%18s &C%3d%%&B  ",
		skill_table[sn]->name, ch->pcdata->learned[sn] ); }
               else {
	    pager_printf( ch, "%18s %3d%%  ",
		skill_table[sn]->name, ch->pcdata->learned[sn] ); }
	    if ( ++col % 3 == 0 )
		send_to_pager( "\n\r", ch );
	}


        if ( buf[strlen( buf )-1] != ':' )
        {
            strcat( buf, ".\n\r" );
            send_to_char( buf, ch );
        }


        
    
	if ( col % 3 != 0 )
	    send_to_pager( "\n\r", ch );

	pager_printf( ch, "You have %d practice sessions left.\n\r",
	    ch->practice );
    }

else
    {
	CHAR_DATA *mob;
	int adept=0;

	if ( !IS_AWAKE(ch) )
	{
	    send_to_char( "In your dreams, or what?\n\r", ch );
	    return;
	}

	for ( mob = ch->in_room->first_person; mob; mob = mob->next_in_room )
	    if ( IS_NPC(mob) && IS_SET(mob->act, ACT_PRACTICE) )
		break;

	if ( !mob )
	{
	    send_to_char( "You can't do that here.\n\r", ch );
	    return;
	}


	if ( ch->practice <= 0 )
	{
	    act( AT_TELL, "$n tells you 'You must earn some more practice sessions.'",
		mob, NULL, ch, TO_VICT );
	    return;
	}
  
	sn = skill_lookup( argument );

 if ( sn == -1 )
{
 send_to_char("Practice what?\n\r",ch);
 return;
}  
        if ( !IS_NPC(ch) && 
             (ch->level < skill_table[sn]->skill_level[ch->class]) &&
	     (ch->level2 < skill_table[sn]->skill_level[ch->class2]) &&
             (ch->advlevel < skill_table[sn]->skill_level[ch->advclass]) &&
             (ch->advlevel2 < skill_table[sn]->skill_level[ch->advclass2]) &&
	     (ch->level < skill_table[sn]->race_skill_level[ch->race] )	)
         {
	    act( AT_TELL, "$n tells you 'You're not ready to learn that yet...'",
		mob, NULL, ch, TO_VICT );
	 }

 	if ( is_name( skill_tname[skill_table[sn]->type], CANT_PRAC ) )
	{
	    act( AT_TELL, "$n tells you 'I do not know how to teach that.'",  
		  mob, NULL, ch, TO_VICT );
	}

	/*
	 * Skill requires a special teacher
	 */
	if ( skill_table[sn]->teachers && skill_table[sn]->teachers[0] != '\0' )
	{
	    sprintf( buf, "%d", mob->pIndexData->vnum );
	    if ( !is_name( buf, skill_table[sn]->teachers ) )
	    {
		act( AT_TELL, "$n tells you 'I no not know how to teach that.'",
		    mob, NULL, ch, TO_VICT );
		return;
	    }
	}

/*Only causes problems
	if ( !IS_NPC(ch) && skill_table[sn]->guild != CLASS_NONE)
	{
	    act( AT_TELL, "$n tells you 'That is only for members of guilds...'",
		mob, NULL, ch, TO_VICT );
	    return;
	}
*/
	/* Figure out what class has this skill, use that adept --GW */
        if ( (ch->level > skill_table[sn]->skill_level[ch->class]) ||
             (ch->level > skill_table[sn]->race_skill_level[ch->race]) )
	adept = class_table[ch->class]->skill_adept * 0.2;
        if ( ch->level2 > skill_table[sn]->skill_level[ch->class2] )
	adept = class_table[ch->class2]->skill_adept * 0.2;
        if ( ch->advlevel > skill_table[sn]->skill_level[ch->advclass] )
	adept = class_table[ch->advclass]->skill_adept * 0.2;
        if ( ch->advlevel2 > skill_table[sn]->skill_level[ch->advclass2] )
	adept = class_table[ch->advclass2]->skill_adept * 0.2;

	/* Make it so you can only prac skills from your class/race --GW*/
	if ( !CAN_USE_SK( ch, skill_table[sn] ) && 
		  !CAN_USE_RACE_SK( ch, skill_table[sn] ) ) 
	{
	   act( AT_TELL, "$n tells you 'You arn't able to learn that!'",mob,NULL,ch,TO_VICT);
	   return;
	}
	else if ( ch->pcdata->learned[sn] >= adept && ch->pcdata->learned[sn] != 0 )
	{
	    sprintf( buf, "$n tells you, 'I've taught you everything I can about %s.'",
		skill_table[sn]->name );
	    act( AT_TELL, buf, mob, NULL, ch, TO_VICT );
	    act( AT_TELL, "$n tells you, 'You'll have to practice it on your own now...'",
		mob, NULL, ch, TO_VICT );
   		return;
	}
	else
	{

	    ch->practice--;
	    ch->pcdata->learned[sn] += int_app[get_curr_int(ch)].learn;
	    act( AT_ACTION, "You practice $T.",
		    ch, NULL, skill_table[sn]->name, TO_CHAR );
	    act( AT_ACTION, "$n practices $T.",
		    ch, NULL, skill_table[sn]->name, TO_ROOM );
	    if ( ch->pcdata->learned[sn] >= adept )
	    {
		ch->pcdata->learned[sn] = adept;
		act( AT_TELL,
		 "$n tells you. 'You'll have to practice it on your own now...'",
		 mob, NULL, ch, TO_VICT );
  		return;
	    }
 } 
 send_to_char("Practice what?\n\r",ch);
 return;
}

    return;
}



void do_wimpy( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int wimpy;
    int max_wimp;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
	wimpy = (int) ch->max_hit / 5;
    else
	wimpy = atoi( arg );

    if ( wimpy < 0 )
    {
	send_to_char( "Your courage exceeds your wisdom.\n\r", ch );
	return;
    }

    if ( wimpy > ch->max_hit )
    {
	send_to_char( "Such cowardice ill becomes you.\n\r", ch );
	return;
    }

    max_wimp = ch->max_hit * 0.70;
    if ( wimpy > max_wimp )
    {
        ch_printf(ch, "You may not set Wimpy any higher then 70%% of your Max HP (70%%=%d)\n\r",max_wimp);
	return;
    }

    ch->wimpy	= wimpy;
    ch_printf( ch, "Wimpy set to %d hit points.\n\r", wimpy );
    return;
}



void do_password( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char *pArg;
    char *pwdnew;
    char *p;
    char cEnd;

    if ( IS_NPC(ch) )
	return;

    /*
     * Can't use one_argument here because it smashes case.
     * So we just steal all its code.  Bleagh.
     */
    pArg = arg1;
    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    pArg = arg2;
    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: password <old> <new>.\n\r", ch );
	return;
    }

    if ( strcmp( crypt( arg1, ch->pcdata->pwd ), ch->pcdata->pwd ) )
    {
	WAIT_STATE( ch, 40 );
	send_to_char( "Wrong password.  Wait 10 seconds.\n\r", ch );
	return;
    }

    if ( strlen(arg2) < 5 )
    {
	send_to_char(
	    "New password must be at least five characters long.\n\r", ch );
	return;
    }

    /*
     * No tilde allowed because of player file format.
     */
    pwdnew = crypt( arg2, ch->name );
    for ( p = pwdnew; *p != '\0'; p++ )
    {
	if ( *p == '~' )
	{
	    send_to_char(
		"New password not acceptable, try again.\n\r", ch );
	    return;
	}
    }

    DISPOSE( ch->pcdata->pwd );
    ch->pcdata->pwd = str_dup( pwdnew );
    if ( IS_SET(sysdata.save_flags, SV_PASSCHG) )
	save_char_obj( ch );
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_socials( CHAR_DATA *ch, char *argument )
{
    int iHash;
    int col = 0;
    SOCIALTYPE *social;

    set_pager_color( AT_PLAIN, ch );
    for ( iHash = 0; iHash < 27; iHash++ )
	for ( social = social_index[iHash]; social; social = social->next )
	{
	    pager_printf( ch, "%-12s", social->name );
	    if ( ++col % 6 == 0 )
		send_to_pager( "\n\r", ch );
	}

    if ( col % 6 != 0 )
	send_to_pager( "\n\r", ch );
    return;
}


void do_commands( CHAR_DATA *ch, char *argument )
{
    int col;
    bool found;
    int hash;
    CMDTYPE *command;

    col = 0;
    set_pager_color( AT_PLAIN, ch );
    if ( argument[0] == '\0' )
    {
	for ( hash = 0; hash < 126; hash++ )
	    for ( command = command_hash[hash]; command; command = command->next )
		if ( command->level <  LEVEL_AVATAR
		&&   command->level <= get_trust( ch )
		&&  (command->name[0] != 'm'
		&&   command->name[1] != 'p') )
		{
		    pager_printf( ch, "%-12s", command->name );
		    if ( ++col % 6 == 0 )
			send_to_pager( "\n\r", ch );
		}
	if ( col % 6 != 0 )
	    send_to_pager( "\n\r", ch );
    }
    else
    {
	found = FALSE;
	for ( hash = 0; hash < 126; hash++ )
	    for ( command = command_hash[hash]; command; command = command->next )
		if ( command->level <  LEVEL_AVATAR
		&&   command->level <= get_trust( ch )
		&&  !str_prefix(argument, command->name)
		&&  (command->name[0] != 'm'
		&&   command->name[1] != 'p') )
		{
		    pager_printf( ch, "%-12s", command->name );
		    found = TRUE;
		    if ( ++col % 6 == 0 )
			send_to_pager( "\n\r", ch );
		}

	if ( col % 6 != 0 )
	    send_to_pager( "\n\r", ch );
	if ( !found )
	    ch_printf( ch, "No command found under %s.\n\r", argument);
    }
    return;
}


void do_channels( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if ( IS_NPC(ch) )
      return;

	if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_SILENCE) )
	{
	    send_to_char( "You are silenced.\n\r", ch );
	    return;
	}

    if ( arg[0] == '\0' )
    {
	send_to_char("Please type: channel <public/private/god>, or\n\r",ch);
	send_to_char("             channel <channel name> to switch it on, or off\n\r",ch);
        return;
    }

    if ( !str_cmp( arg, "public" ) )
    {
	send_to_char( "&GPublic Channels:\n\r", ch );

        if ( get_trust( ch ) > 2 && !NOT_AUTHED( ch ) )
        {
	ch_printf( ch,"&wAuction:    %s\n\r", !IS_SET(ch->pcdata->deaf, CHANNEL_AUCTION) ?
		"&G(ON)" : "&R(OFF)");
        }

	ch_printf( ch,"&wRumor:      %s\n\r", !IS_SET(ch->pcdata->deaf, CHANNEL_CHAT) ?
		"&G(ON)" : "&R(OFF)");

	ch_printf( ch,"&wIC:         %s\n\r", !IS_SET(ch->pcdata->deaf, CHANNEL_IC) ?
		"&G(ON)" : "&R(OFF)");

	ch_printf( ch,"&wInfo:       %s\n\r", !IS_SET(ch->pcdata->deaf, CHANNEL_INFO) ?
		"&G(ON)" : "&R(OFF)");

	ch_printf( ch,"&wQuest:      %s\n\r", !IS_SET(ch->pcdata->deaf, CHANNEL_QUEST) ?
		"&G(ON)" : "&R(OFF)");

	ch_printf( ch,"&wWar:        %s\n\r", !IS_SET(ch->pcdata->deaf, CHANNEL_WARTALK) ?
		"&G(ON)" : "&R(OFF)");

	ch_printf( ch,"&wMusic:      %s\n\r", !IS_SET(ch->pcdata->deaf, CHANNEL_MUSIC) ?
		"&G(ON)" : "&R(OFF)");

	ch_printf( ch,"&wAsk:        %s\n\r", !IS_SET(ch->pcdata->deaf, CHANNEL_ASK) ?
		"&G(ON)" : "&R(OFF)");

	ch_printf( ch,"&wFlame:      %s\n\r", !IS_SET(ch->pcdata->deaf, CHANNEL_FLAME) ?
		"&G(ON)" : "&R(OFF)");
 
	ch_printf( ch,"&wShout:      %s\n\r", !IS_SET(ch->pcdata->deaf, CHANNEL_SHOUT) ?
		"&G(ON)" : "&R(OFF)" );

	ch_printf( ch,"&wYell:	    %s\n\r", !IS_SET(ch->pcdata->deaf, CHANNEL_YELL) ?
		"&G(ON)" : "&R(OFF)" );

    return;
    }

    if ( !str_cmp( arg, "private" ) )
    {
	send_to_char(  "\n\r&GPrivate Channels:\n\r", ch );

        if ( !IS_NPC( ch ) && ch->pcdata->clan )
        {
          if ( ch->pcdata->clan->clan_type == CLAN_ORDER )
          {
	    ch_printf(ch,"&wOrder:       %s\n\r", !IS_SET(ch->pcdata->deaf, CHANNEL_ORDER) ?
		"&G(ON)" : "&R(OFF)" );
	  }
          else
          if ( ch->pcdata->clan->clan_type == CLAN_GUILD )
          { 
	    ch_printf(ch,"&wGuild:       %s\n\r", !IS_SET(ch->pcdata->deaf, CHANNEL_GUILD) ?
		"&G(ON)" : "&R(OFF)" );
          }
	  else
          {
	    ch_printf(ch,"&wClan:        %s\n\r", !IS_SET(ch->pcdata->deaf, CHANNEL_CLAN) ?
		"&G(ON)" : "&R(OFF)" );
          }
        }
        if ( !IS_NPC( ch ) && ch->pcdata->council )
        {
	    ch_printf(ch,"&wCouncil:   %s\n\r",!IS_SET(ch->pcdata->deaf,CHANNEL_COUNCIL) ?
		"&G(ON)" : "&R(OFF)" );
	}
        if ( IS_HERO(ch) )
        {   
	    ch_printf(ch,"&wAvatar:      %s\n\r",!IS_SET(ch->pcdata->deaf,CHANNEL_AVTALK) ?
		"&G(ON)" : "&R(OFF)" );
        }
	    ch_printf(ch,"&wTells:       %s\n\r", !IS_SET(ch->pcdata->deaf,CHANNEL_TELLS) ?
		"&G(ON)" : "&R(OFF)" );
    return;
    }


   if ( !str_cmp( arg,"god" ) )
   {
	if ( IS_IMMORTAL(ch) )
	{
	    send_to_char( "\n\r&GImmortal Channels:\n\r", ch );
	    ch_printf(ch,"&wImmtalk:    %s\n\r",!IS_SET(ch->pcdata->deaf,CHANNEL_IMMTALK) ?
		"&G(ON)" : "&R(OFF)" );
            
        if ( get_trust(ch) >= sysdata.muse_level )  
        {
	    ch_printf(ch,"&wImp:        %s\n\r",!IS_SET(ch->pcdata->deaf,CHANNEL_HIGHGOD) ?
		"&G(ON)" : "&R(OFF)" );
        }   

        if ( get_trust(ch) >= LEVEL_COUNCIL )  
        {
	    ch_printf(ch,"&wOlympus: 	%s\n\r",!IS_SET(ch->pcdata->deaf,CHANNEL_OLYMPUS) ?
		"&G(ON)" : "&R(OFF)" );
        }   

        if ( IS_IMMORTAL(ch) || ch->pcdata->council )
	{
	    ch_printf(ch,"&wNewbiechat:	%s\n\r",!IS_SET(ch->pcdata->deaf,CHANNEL_NEWBIE) ?
		"&G(ON)" : "&R(OFF)" );
   	}

	    if ( get_trust(ch) >= sysdata.think_level )
            ch_printf(ch,"&wThink:      %s\n\r", !IS_SET(ch->pcdata->deaf,CHANNEL_HIGH) ?
		"&G(ON)" : "&R(OFF)" );
	}
      return;     
    }
    else
    {
	bool fClear;
	bool ClearAll;
	int bit;

        bit=0;
        ClearAll = FALSE;

	     if ( !str_cmp( arg, "auction"  ) ) bit = CHANNEL_AUCTION;
	else if ( !str_cmp( arg, "rumor"    ) ) bit = CHANNEL_CHAT;
	else if ( !str_cmp( arg, "info"     ) ) bit = CHANNEL_INFO;
	else if ( !str_cmp( arg, "clan"     ) ) bit = CHANNEL_CLAN;
	else if ( !str_cmp( arg, "council"  ) ) bit = CHANNEL_COUNCIL;
        else if ( !str_cmp( arg, "guild"    ) ) bit = CHANNEL_GUILD;  
	else if ( !str_cmp( arg, "quest"    ) ) bit = CHANNEL_QUEST;
	else if ( !str_cmp( arg, "tells"    ) ) bit = CHANNEL_TELLS;
	else if ( !str_cmp( arg, "immtalk"  ) ) bit = CHANNEL_IMMTALK;
	else if ( !str_cmp( arg, "think"    ) ) bit = CHANNEL_HIGH;
	else if ( !str_cmp( arg, "olympus"  ) ) bit = CHANNEL_OLYMPUS;
	else if ( !str_cmp( arg, "pray"     ) ) bit = CHANNEL_PRAY;
	else if ( !str_cmp( arg, "avatar"   ) ) bit = CHANNEL_AVTALK;
	else if ( !str_cmp( arg, "newbie"   ) ) bit = CHANNEL_NEWBIE;
	else if ( !str_cmp( arg, "music"    ) ) bit = CHANNEL_MUSIC;
	else if ( !str_cmp( arg, "ask"      ) ) bit = CHANNEL_ASK;
	else if ( !str_cmp( arg, "shout"    ) ) bit = CHANNEL_SHOUT;
	else if ( !str_cmp( arg, "yell"     ) ) bit = CHANNEL_YELL;
	else if ( !str_cmp( arg, "order"    ) ) bit = CHANNEL_ORDER;
	else if ( !str_cmp( arg, "imp"	    ) ) bit = CHANNEL_HIGHGOD;
        else if ( !str_cmp( arg, "war"  ) ) bit = CHANNEL_WARTALK;
        else if ( !str_cmp( arg, "flame"    ) ) bit = CHANNEL_FLAME;
        else if ( !str_cmp( arg, "ic"       ) ) bit = CHANNEL_IC;
	else if ( !str_cmp( arg, "all"      ) ) ClearAll = TRUE;
	else
	{
	    send_to_char( "Set or clear which channel?\n\r", ch );
	    return;
	}

	     if ( IS_SET(ch->pcdata->deaf, bit) ) fClear = TRUE;
	     else fClear = FALSE;


	if (( fClear ) && ( ClearAll ))
	{
            REMOVE_BIT (ch->pcdata->deaf, CHANNEL_AUCTION);
            REMOVE_BIT (ch->pcdata->deaf, CHANNEL_CHAT);
            REMOVE_BIT (ch->pcdata->deaf, CHANNEL_QUEST);
            /*  REMOVE_BIT (ch->pcdata->deaf, CHANNEL_IMMTALK); */
            REMOVE_BIT (ch->pcdata->deaf, CHANNEL_PRAY);
            REMOVE_BIT (ch->pcdata->deaf, CHANNEL_MUSIC);
            REMOVE_BIT (ch->pcdata->deaf, CHANNEL_ASK);
            REMOVE_BIT (ch->pcdata->deaf, CHANNEL_SHOUT);
            REMOVE_BIT (ch->pcdata->deaf, CHANNEL_YELL);

       /*     if (ch->pcdata->clan)
              REMOVE_BIT (ch->pcdata->deaf, CHANNEL_CLAN);

	    if (ch->pcdata->council)
	      REMOVE_BIT (ch->pcdata->deaf, CHANNEL_COUNCIL);

            if (ch->pcdata->guild)
              REMOVE_BIT (ch->pcdata->deaf, CHANNEL_GUILD);
       */
            if (ch->level >= LEVEL_IMMORTAL)
              REMOVE_BIT (ch->pcdata->deaf, CHANNEL_AVTALK);
 

        } else if ((!fClear) && (ClearAll))
        {
            SET_BIT (ch->pcdata->deaf, CHANNEL_AUCTION);
            SET_BIT (ch->pcdata->deaf, CHANNEL_CHAT);
            SET_BIT (ch->pcdata->deaf, CHANNEL_QUEST);
       /*     SET_BIT (ch->pcdata->deaf, CHANNEL_IMMTALK); */
            SET_BIT (ch->pcdata->deaf, CHANNEL_PRAY);
            SET_BIT (ch->pcdata->deaf, CHANNEL_MUSIC);
            SET_BIT (ch->pcdata->deaf, CHANNEL_ASK);
            SET_BIT (ch->pcdata->deaf, CHANNEL_SHOUT);
            SET_BIT (ch->pcdata->deaf, CHANNEL_YELL);
          
       /*     if (ch->pcdata->clan)
              SET_BIT (ch->pcdata->deaf, CHANNEL_CLAN);

	    if (ch->pcdata->council)
	      SET_BIT (ch->pcdata->deaf, CHANNEL_COUNCIL);

            if ( IS_GUILDED(ch) )
              SET_BIT (ch->pcdata->deaf, CHANNEL_GUILD);
       */
            if (ch->level >= LEVEL_IMMORTAL)
              SET_BIT (ch->pcdata->deaf, CHANNEL_AVTALK);

         } else if (fClear)
         {
	    REMOVE_BIT (ch->pcdata->deaf, bit);
         } else
         {
	    SET_BIT    (ch->pcdata->deaf, bit);
         }

	  send_to_char( "Ok.\n\r", ch );
    }
  
    return;
}


/*
 * display WIZLIST file						-Thoric
 */
void do_wizlist( CHAR_DATA *ch, char *argument )
{
char arg[MIL];
WIZENT *wiz, *wiz_next;
extern WIZENT *first_wiz;

one_argument( argument, arg );

if ( arg[0] == '\0' || get_trust(ch) < LEVEL_HYPERION )
{
    set_pager_color( AT_IMMORT, ch );
    show_file( ch, WIZLIST_FILE );
return;
}

if ( !str_cmp( arg, "retired" ) )
{
send_to_char("Retired Immortals Listing:\n",ch);
send_to_char("==========================\n",ch);

  for( wiz = first_wiz; wiz; wiz = wiz_next )
  {

   wiz_next = wiz->next;
    if ( IS_SET( wiz->flags, PCFLAG_RETIRED ) )
    {
     ch_printf( ch, "%s ", capitalize( wiz->name ));
    }
  }
send_to_char("\n\r",ch);
return;
}

if ( !str_cmp( arg, "guest" ) )
{
send_to_char("Guest Immortals Listing:\n",ch);
send_to_char("========================\n",ch);

  for( wiz = first_wiz; wiz; wiz = wiz_next )
  {
   wiz_next = wiz->next;
    if ( IS_SET( wiz->flags, PCFLAG_GUEST ) )
    {
     ch_printf( ch, "%s ", capitalize( wiz->name ));
    }
  }
send_to_char("\n\r",ch);
return;
}

send_to_char("Syntax: Wizlist <no arg/guest/wizlist>\n\r",ch);
return;
}

/*
 * Contributed by Grodyn.
 */
void do_config( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    if ( IS_NPC(ch) )
	return;

    one_argument( argument, arg );

    set_char_color( AT_WHITE, ch );
    if ( arg[0] == '\0' )
    {
	send_to_char( "[ Keyword  ] Option\n\r", ch );

    if ( !IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) )
      {
	send_to_char(  IS_SET(ch->act, PLR_NICE)
	    ? "[+NICE     ] You are nice to other players.\n\r"
	    : "[-nice     ] You are not nice to other players.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_FLEE)
	    ? "[+FLEE     ] You flee if you get attacked.\n\r"
	    : "[-flee     ] You fight back if you get attacked.\n\r"
	    , ch );
      }

	send_to_char(  IS_SET(ch->pcdata->flags, PCFLAG_NORECALL)
	    ? "[+NORECALL ] You fight to the death, link-dead or not.\n\r"
	    : "[-norecall ] You try to recall if fighting link-dead.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_AUTOEXIT)
	    ? "[+AUTOEXIT ] You automatically see exits.\n\r"
	    : "[-autoexit ] You don't automatically see exits.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_AUTOLOOT)
	    ? "[+AUTOLOOT ] You automatically loot corpses.\n\r"
	    : "[-autoloot ] You don't automatically loot corpses.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_AUTOSAC)
	    ? "[+AUTOSAC  ] You automatically sacrifice corpses.\n\r"
	    : "[-autosac  ] You don't automatically sacrifice corpses.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_AUTOGOLD)
	    ? "[+AUTOGOLD ] You automatically split gold from kills in groups.\n\r"
	    : "[-autogold ] You don't automatically split gold from kills in groups.\n\r"
	    , ch );

        send_to_char(  IS_SET(ch->pcdata->flags, PCFLAG_GAG)       
            ? "[+GAG      ] You see only necessary battle text.\n\r"
            : "[-gag      ] You see full battle text.\n\r"
            , ch );

        send_to_char(  IS_SET(ch->pcdata->flags, PCFLAG_PAGERON)
            ? "[+PAGER    ] Long output is page-paused.\n\r"
            : "[-pager    ] Long output scrolls to the end.\n\r"
            , ch );
	
	send_to_char(  IS_SET(ch->act, PLR_BLANK)
	    ? "[+BLANK    ] You have a blank line before your prompt.\n\r"
	    : "[-blank    ] You have no blank line before your prompt.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_BRIEF)
	    ? "[+BRIEF    ] You see brief descriptions.\n\r"
	    : "[-brief    ] You see long descriptions.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_COMBINE)
	    ? "[+COMBINE  ] You see object lists in combined format.\n\r"
	    : "[-combine  ] You see object lists in single format.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->pcdata->flags, PCFLAG_NOINTRO)
	    ? "[+NOINTRO  ] You don't see the ascii intro screen on login.\n\r"
	    : "[-nointro  ] You see the ascii intro screen on login.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_PROMPT)
	    ? "[+PROMPT   ] You have a prompt.\n\r"
	    : "[-prompt   ] You don't have a prompt.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_TELNET_GA)
	    ? "[+TELNETGA ] You receive a telnet GA sequence.\n\r"
	    : "[-telnetga ] You don't receive a telnet GA sequence.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_ANSI)
	    ? "[+ANSI     ] You receive ANSI color sequences.\n\r"
	    : "[-ansi     ] You don't receive receive ANSI colors.\n\r"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_RIP)
	    ? "[+RIP      ] You receive RIP graphic sequences.\n\r"
	    : "[-rip      ] You don't receive RIP graphics.\n\r"
	    , ch );

	if ( !IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) )
	  send_to_char(  IS_SET(ch->act, PLR_SHOVEDRAG)
	      ? "[+SHOVEDRAG] You allow yourself to be shoved and dragged around.\n\r"
	      : "[-shovedrag] You'd rather not be shoved or dragged around.\n\r"
	      , ch );

	send_to_char(  IS_SET( ch->pcdata->flags, PCFLAG_NOSUMMON )
	      ? "[+NOSUMMON ] You do not allow other players to summon you.\n\r"
	      : "[-nosummon ] You allow other players to summon you.\n\r"
	      , ch );

	if ( IS_IMMORTAL( ch ) )
	  send_to_char(  IS_SET(ch->act, PLR_ROOMVNUM)
	      ? "[+VNUM     ] You can see the VNUM of a room.\n\r"
	      : "[-vnum     ] You do not see the VNUM of a room.\n\r"
	      , ch );

	if ( IS_IMMORTAL( ch ) )
	  send_to_char(  IS_SET(ch->act, PLR_AUTOMAP)    /* maps */
	      ? "[+MAP      ] You can see the MAP of a room.\n\r"
	      : "[-map      ] You do not see the MAP of a room.\n\r"
	      , ch );

	send_to_char(  IS_SET(ch->act, PLR_SILENCE)
	    ? "[+SILENCE  ] You are silenced.\n\r"
	    : ""
	    , ch );

	send_to_char( !IS_SET(ch->act, PLR_NO_EMOTE)
	    ? ""
	    : "[-emote    ] You can't emote.\n\r"
	    , ch );

	send_to_char( !IS_SET(ch->act, PLR_NO_TELL)
	    ? ""
	    : "[-tell     ] You can't use 'tell'.\n\r"
	    , ch );
	send_to_char( !IS_SET(ch->act, PLR_LITTERBUG)
	    ? ""
	    : "[-litter  ] A convicted litterbug. You cannot drop anything.\n\r"
	    , ch );
	send_to_char( IS_SET(ch->pcdata->flags, PCFLAG_SOUND)
	    ? "[+SOUND    ] You can hear MSP sounds.\n\r"
	    : "[-sound    ] You don't hear MSP sounds.\n\r"
	    , ch );
	send_to_char( IS_SET(ch->pcdata->flags, PCFLAG_OLD_DEATH)
	    ? "[+OLDDEATH ] You see the OLD Death Message.\n\r"
	    : "[-olddeath ] You see the NEW Death Messages.\n\r"
	    , ch );
	send_to_char( IS_SET(ch->pcdata->flagstwo, MOREPC_AFFMSGS)
	    ? "[+NOAFFMSG ] You don't the affect messages.\n\r"
	    : "[-noaffmsg ] You see the affect messages.\n\r"
	    , ch );
/*	send_to_char( IS_SET(ch->pcdata->flagstwo, MOREPC_NOSPAM)
	    ? "[+NOSPAM   ] You don't see unnecessary battle spam.\n\r"
	    : "[-nospam   ] You see full battle spam.\n\r"
	    , ch );*/
    }
    else
    {
	bool fSet;
	int bit = 0;

	     if ( arg[0] == '+' ) fSet = TRUE;
	else if ( arg[0] == '-' ) fSet = FALSE;
	else
	{
	    send_to_char( "Config -option or +option?\n\r", ch );
	    return;
	}

	if ( !str_prefix( arg+1, "noaffmsg" ) ) {
	  bit = MOREPC_AFFMSGS;
	  if ( fSet )
	    SET_BIT    (ch->pcdata->flagstwo, bit);
	  else
	    REMOVE_BIT (ch->pcdata->flagstwo, bit);
	  send_to_char( "Ok.\n\r", ch );
          return;
        }/*
	else if ( !str_prefix( arg+1, "nospam" ) ) {
	  bit = MOREPC_NOSPAM;
	  if ( fSet )
	    SET_BIT    (ch->pcdata->flagstwo, bit);
	  else
	    REMOVE_BIT (ch->pcdata->flagstwo, bit);
	  send_to_char( "Ok.\n\r", ch );
          return;
        }*/
	else
	{


	     if ( !str_prefix( arg+1, "autoexit" ) ) bit = PLR_AUTOEXIT;
	else if ( !str_prefix( arg+1, "autoloot" ) ) bit = PLR_AUTOLOOT;
	else if ( !str_prefix( arg+1, "autosac"  ) ) bit = PLR_AUTOSAC;
	else if ( !str_prefix( arg+1, "autogold" ) ) bit = PLR_AUTOGOLD;
	else if ( !str_prefix( arg+1, "blank"    ) ) bit = PLR_BLANK;
	else if ( !str_prefix( arg+1, "brief"    ) ) bit = PLR_BRIEF;
	else if ( !str_prefix( arg+1, "combine"  ) ) bit = PLR_COMBINE;
	else if ( !str_prefix( arg+1, "prompt"   ) ) bit = PLR_PROMPT;
	else if ( !str_prefix( arg+1, "telnetga" ) ) bit = PLR_TELNET_GA;
	else if ( !str_prefix( arg+1, "ansi"     ) ) bit = PLR_ANSI;
	else if ( !str_prefix( arg+1, "rip"      ) ) bit = PLR_RIP;
	else if ( !str_prefix( arg+1, "flee"     ) ) bit = PLR_FLEE;
	else if ( !str_prefix( arg+1, "nice"     ) ) bit = PLR_NICE;
	else if ( !str_prefix( arg+1, "shovedrag") ) bit = PLR_SHOVEDRAG;
	else if ( IS_IMMORTAL( ch )
	     &&   !str_prefix( arg+1, "vnum"     ) ) bit = PLR_ROOMVNUM;
	else if ( IS_IMMORTAL( ch )
	     &&   !str_prefix( arg+1, "map"      ) ) bit = PLR_AUTOMAP;     /* maps */

	if (bit)
        {
  	  if ( (bit == PLR_FLEE || bit == PLR_NICE || bit == PLR_SHOVEDRAG) 
	  &&  IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) )
          {
	    send_to_char( "Pkill characters can not config that option.\n\r", ch );
	    return;
          }
          
	  if ( fSet )
	    SET_BIT    (ch->act, bit);
	  else
	    REMOVE_BIT (ch->act, bit);
	  send_to_char( "Ok.\n\r", ch );
          return;
        }
        else
        {
	       if ( !str_prefix( arg+1, "norecall" ) ) bit = PCFLAG_NORECALL;
	  else if ( !str_prefix( arg+1, "nointro"  ) ) bit = PCFLAG_NOINTRO;
	  else if ( !str_prefix( arg+1, "nosummon" ) ) bit = PCFLAG_NOSUMMON;
          else if ( !str_prefix( arg+1, "gag"      ) ) bit = PCFLAG_GAG; 
          else if ( !str_prefix( arg+1, "pager"    ) ) bit = PCFLAG_PAGERON;
	  else if ( !str_prefix( arg+1, "sound"	   ) ) bit = PCFLAG_SOUND;
	  else if ( !str_prefix( arg+1, "olddeath" ) ) bit = PCFLAG_OLD_DEATH;
          else
	  {
	    send_to_char( "Config which option?\n\r", ch );
	    return;
    	  }

          if ( fSet )
	    SET_BIT    (ch->pcdata->flags, bit);
	  else
	    REMOVE_BIT (ch->pcdata->flags, bit);

	  send_to_char( "Ok.\n\r", ch );
          return;
        }
	}
    }

    return;
}


void do_credits( CHAR_DATA *ch, char *argument )
{
  do_help( ch, "credits" );
}


extern int top_area;

/*
void do_areas( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea1;
    AREA_DATA *pArea2;
    int iArea;
    int iAreaHalf;

    iAreaHalf = (top_area + 1) / 2;
    pArea1    = first_area;
    pArea2    = first_area;
    for ( iArea = 0; iArea < iAreaHalf; iArea++ )
	pArea2 = pArea2->next;

    for ( iArea = 0; iArea < iAreaHalf; iArea++ )
    {
	ch_printf( ch, "%-39s%-39s\n\r",
	    pArea1->name, pArea2 ? pArea2->name : "" );
	pArea1 = pArea1->next;
	if ( pArea2 )
	    pArea2 = pArea2->next;
    }

    return;
}
*/

/* 
 * New do_areas with soft/hard level ranges 
 */

void do_areas( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;

    set_pager_color( AT_PLAIN, ch );
    send_to_pager("\n\r   Author    |             Area                     | Recommended |  Enforced\n\r", ch);
    send_to_pager("-------------+--------------------------------------+-------------+-----------\n\r", ch);

    for ( pArea = first_area; pArea; pArea = pArea->next )
	pager_printf(ch, "%-12s | %-36s | %4d - %-4d | %3d - %-3d \n\r", 
	 	pArea->author, pArea->name, pArea->low_soft_range, 
		pArea->hi_soft_range, pArea->low_hard_range, 
		pArea->hi_hard_range);
    return;
}

void do_afk( CHAR_DATA *ch, char *argument )
{
     if ( IS_NPC(ch) )
     return;
     
     if IS_SET(ch->act, PLR_AFK)
     {
    	REMOVE_BIT(ch->act, PLR_AFK);
	send_to_char( "You are no longer afk.\n\r", ch );
	act(AT_GREY,"$n is no longer afk.", ch, NULL, NULL, TO_ROOM);
     }
     else
     {
	SET_BIT(ch->act, PLR_AFK);
	send_to_char( "You are now afk.\n\r", ch );
	act(AT_GREY,"$n is now afk.", ch, NULL, NULL, TO_ROOM);
	return;
     }
         
}

void do_slist( CHAR_DATA *ch, char *argument )
{
   int sn, i, lFound;
   char skn[MAX_INPUT_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH]; 
   int lowlev, hilev;
   sh_int lasttype = SKILL_SPELL;

   if ( IS_NPC(ch) )
     return;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   lowlev=1;
   hilev=50;

   if (arg1[0]!='\0')
      lowlev=atoi(arg1);

   if ((lowlev<1) || (lowlev>LEVEL_IMMORTAL))
      lowlev=1;

   if (arg2[0]!='\0')
      hilev=atoi(arg2);

   if ((hilev<0) || (hilev>=LEVEL_IMMORTAL))
      hilev=LEVEL_AVATAR;

   if(lowlev>hilev)
      lowlev=hilev;

   set_pager_color( AT_MAGIC, ch );
   send_to_pager("SPELL & SKILL LIST\n\r",ch);
   send_to_pager("------------------\n\r",ch);

   for (i=lowlev; i <= hilev; i++)
   {
	lFound= 0;
	sprintf(skn,"Spell");   
	for ( sn = 0; sn < top_sn; sn++ )
	{
	    if ( !skill_table[sn]->name )
		break;

	    if ( skill_table[sn]->type != lasttype )
	    {
		lasttype = skill_table[sn]->type;
		strcpy( skn, skill_tname[lasttype] );
	    }

	    if ( ch->pcdata->learned[sn] == 0
	    &&   SPELL_FLAG(skill_table[sn], SF_SECRETSKILL) )
		continue;
	
	    if(i==skill_table[sn]->skill_level[ch->class] ||
 i==skill_table[sn]->skill_level[ch->class2]  )
	    {
		if( !lFound )
		{
		    lFound=1;
		    pager_printf( ch, "Level %d\n\r", i );
		}
		pager_printf(ch, "%7s: %20.20s \t Current: %d Max: %d \n\r",
                        skn, skill_table[sn]->name, 
                        ch->pcdata->learned[sn],
                        skill_table[sn]->skill_adept[ch->class] );
	    }
	}
   }
   return;
}


 
void do_engage( CHAR_DATA *ch, char *argument )

/*      Written By: Greywolf      */

{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *vict1;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
   {
        send_to_char( "Syntax:  engage <person> \n\r", ch );
        return;
    }

    if ( ( vict1 = get_char_world( ch, arg1 ) ) == NULL )
    {
        sprintf( buf, "%s is not connected.\n\r", capitalize(arg1) );
        send_to_char( buf, ch );
        return;
    }

    if ( ch == NULL || vict1 == NULL )
    {
	return;
    }

    if ( IS_NPC(vict1) || IS_NPC(ch) )
    {
        send_to_char( "You cannot be engaged to a mob!\n\r",ch );
        return;
    }

   if ( IS_SET( ch->pcdata->flags, PCFLAG_ENGAGED) && ch->pcdata->engaged != vict1->name )
    {
	send_to_char( "You are not Engaged to that person!", ch );
	return;
    } 

     if ( IS_SET( ch->pcdata->flags, PCFLAG_ENGAGED ) &&
ch->pcdata->engaged ==
vict1->name )
   {
        send_to_char("You are no longer Engaged.\n\r", ch);        
        send_to_char("You are no longer Engaged.\n\r", vict1);        
	REMOVE_BIT( ch->pcdata->flags, PCFLAG_ENGAGED );
	REMOVE_BIT( vict1->pcdata->flags, PCFLAG_ENGAGED );
        return;
    }

     if ( IS_SET( vict1->pcdata->flags, PCFLAG_ENGAGED ) )
   {
        send_to_char("They are currently Engaged.\n\r", ch);
        return;
    }

    if ( vict1 == ch )
    {
        send_to_char( "You cannot do that.\n\r", ch );
        return;
    }

    if ( IS_SET( vict1->pcdata->flags, PCFLAG_WED ) )
    
   {
        sprintf( buf, "%s is already married!\n\r", vict1->name);
        send_to_char( buf, ch );
        return;
    }

    if ( IS_SET( ch->pcdata->flags, PCFLAG_WED ) )
    {
        sprintf( buf, "You is already married to
%s!\n\r",ch->pcdata->spouse ); 
        send_to_char( buf, ch );
        return;
    }

 
    SET_BIT( vict1->pcdata->flags, PCFLAG_ENGAGED );
    SET_BIT( ch->pcdata->flags, PCFLAG_ENGAGED );
    vict1->pcdata->engaged = ch->name;
    ch->pcdata->engaged = vict1->name;


    do_title( vict1, "." );
    do_title( ch, "." );

    sprintf( buf, "%s and %s are now engaged.", vict1->name, ch->name
);
    echo_to_all( AT_PLAIN,buf,ECHOTAR_ALL ); 
}


void do_whois( CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];

  buf[0] = '\0';

  if(IS_NPC(ch))
    return;

  if(argument[0] == '\0')
  {
    send_to_char("You must input the name of a player online.\n\r", ch);
    return;
  }

  strcat(buf, "0.");
  strcat(buf, argument);
  if( ( ( victim = get_char_world(ch, buf) ) == NULL ))
  {
    send_to_char("No such player online.\n\r", ch);
    return;
  }


  if(IS_NPC(victim))
  {
    send_to_char("That's not a player!\n\r", ch);
    return;
  }

  if(IS_IMMORTAL(victim) && ( ch->level < victim->level))
  {
  send_to_char("I think not.\n\r",ch);
  return;
  }

  if ( victim->class2 >= 0)
  {
  ch_printf(ch, "%s is a %s level %d/%d %s %s/%s",
	victim->name, 
	victim->sex == SEX_MALE ? "male" : 
	victim->sex == SEX_FEMALE ? "female" : "neutral",
	victim->level,
	victim->level2, 
	npc_race[victim->race],
	npc_class[victim->class],
	npc_class[victim->class2]);
  }
  else
  {
  ch_printf(ch, "%s is a %s level %d %s %s",
	victim->name, 
	victim->sex == SEX_MALE ? "male" : 
	victim->sex == SEX_FEMALE ? "female" : "neutral",
	victim->level,
	npc_race[victim->race],
	npc_class[victim->class]);
  }
  if (IS_IMMORTAL(ch))
    ch_printf(ch, " in room %d.\n\r",
	victim->in_room->vnum);
  else
    ch_printf(ch, ".\n\r");

  if (IS_ADVANCED(victim))
    {
    ch_printf(ch, "%s has achieved level %d %s,\n",
	victim->sex == SEX_MALE ? "He" :
	victim->sex == SEX_FEMALE ? "She" : "It",
        victim->advlevel,
	npc_class[victim->advclass]);
    }

  ch_printf(ch, "%s is a %sdeadly player",
	victim->sex == SEX_MALE ? "He" :
	victim->sex == SEX_FEMALE ? "She" : "It",
	IS_SET(victim->pcdata->flags, PCFLAG_DEADLY) ? "" : "non-");

  if ( victim->pcdata->clan )
  {
	if ( victim->pcdata->clan->clan_type == CLAN_ORDER )
	   send_to_char( ", and belongs to the Order ", ch );
	else
	if ( victim->pcdata->clan->clan_type == CLAN_GUILD )
	   send_to_char( ", and belongs to the ", ch );
	else
	   send_to_char( ", and belongs to Clan ", ch );
	send_to_char( victim->pcdata->clan->name, ch );
  }
  send_to_char( ".\n\r", ch );

  if(victim->pcdata->council)
    ch_printf(ch, "%s belongs to the %s.\n\r", 
	victim->name,
	victim->pcdata->council->name);
    
  if(victim->pcdata->deity)
    ch_printf(ch, "%s has found succor in the deity %s.\n\r",
	victim->name,
	victim->pcdata->deity->name);
  
  if(victim->pcdata->homepage && victim->pcdata->homepage[0] != '\0')
    ch_printf(ch, "%s's homepage can be found at %s.\n\r", 
	victim->name,
	victim->pcdata->homepage);

  if(victim->pcdata->bio && victim->pcdata->bio[0] != '\0')
    ch_printf(ch, "%s's personal bio:\n\r%s",
	victim->name,
	victim->pcdata->bio);

  if(IS_IMMORTAL(ch))
  {  
    send_to_char("----------------------------------------------------\n\r", ch);

    send_to_char("Info for immortals:\n\r", ch);

    if ( victim->pcdata->authed_by && victim->pcdata->authed_by[0] != '\0' )
	ch_printf(ch, "%s was authorized by %s.\n\r",
		victim->name, victim->pcdata->authed_by);

    ch_printf(ch, "%s has killed %d mobiles, and been killed by a mobile %d times.\n\r",
		victim->name, victim->pcdata->mkills, victim->pcdata->mdeaths );
    if ( victim->pcdata->pkills || victim->pcdata->pdeaths )
	ch_printf(ch, "%s has killed %d players, and been killed by a player %d times.\n\r",
		victim->name, victim->pcdata->pkills, victim->pcdata->pdeaths );
    if ( victim->pcdata->illegal_pk )
	ch_printf(ch, "%s has committed %d illegal player kills.\n\r",
		victim->name, victim->pcdata->illegal_pk );

    ch_printf(ch, "%s is %shelled at the moment.\n\r",
	victim->name, 
	(victim->pcdata->release_date == 0) ? "not " : "");

    if(victim->pcdata->release_date != 0)
      ch_printf(ch, "%s was helled by %s, and will be released on %24.24s.\n\r",
	victim->sex == SEX_MALE ? "He" :
	victim->sex == SEX_FEMALE ? "She" : "It",
        victim->pcdata->helled_by,
	ctime(&victim->pcdata->release_date));

    if(get_trust(victim) < get_trust(ch))
    {
      sprintf(buf2, "list %s", buf);
      do_comment(ch, buf2);
    }

    if(IS_SET(victim->act, PLR_SILENCE) || IS_SET(victim->act, PLR_NO_EMOTE) 
    || IS_SET(victim->act, PLR_NO_TELL) || IS_SET(victim->act, PLR_THIEF) 
    || IS_SET(victim->act, PLR_KILLER) )
    {
      sprintf(buf2, "This player has the following flags set:");
      if(IS_SET(victim->act, PLR_SILENCE)) 
        strcat(buf2, " silence");
      if(IS_SET(victim->act, PLR_NO_EMOTE)) 
        strcat(buf2, " noemote");
      if(IS_SET(victim->act, PLR_NO_TELL) )
        strcat(buf2, " notell");
      if(IS_SET(victim->act, PLR_THIEF) )
        strcat(buf2, " thief");
      if(IS_SET(victim->act, PLR_KILLER) )
        strcat(buf2, " killer");
      strcat(buf2, ".\n\r");
      send_to_char(buf2, ch);
    }
    if ( victim->pcdata->clan )
    {
    ch_printf(ch, "%s has achieved clan level %d\n",
             capitalize(victim->name),
             victim->pcdata->clevel);
    }
    if ( get_trust(victim) != victim->level && !IS_HIMP(victim) )
    {
    ch_printf(ch, "%s is trusted at level %d\n",
             capitalize(victim->name),
             get_trust(victim));
    }
    if ( ch->level >= 58 )
    {
      if ( victim->pcdata->email != NULL )
	ch_printf(ch, "%s's email address is: %s\n",capitalize(victim->name),victim->pcdata->email);
      else
	ch_printf(ch, "%s has NO EMAIL Specified.\n",capitalize(victim->name));
    }

  if ( victim->desc && victim->desc->host[0]!='\0' )   /* added by Gorog */
     {
     sprintf (buf2, "%s's IP info: %s ", victim->name, victim->desc->host);
     if (get_trust(ch) >= LEVEL_GOD)
        strcat (buf2, victim->desc->user);
     strcat (buf2, "\n\r");
     send_to_char(buf2, ch);
     }
  }
}

void do_pager( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  
  if ( IS_NPC(ch) )
    return;
  argument = one_argument(argument, arg);
  if ( !*arg )
  {
    if ( IS_SET(ch->pcdata->flags, PCFLAG_PAGERON) )
      do_config(ch, "-pager");
    else
      do_config(ch, "+pager");
    return;
  }
  if ( !is_number(arg) )
  {
    send_to_char( "Set page pausing to how many lines?\n\r", ch );
    return;
  }
  ch->pcdata->pagerlen = atoi(arg);
  if ( ch->pcdata->pagerlen < 5 )
    ch->pcdata->pagerlen = 5;
  ch_printf( ch, "Page pausing set to %d lines.\n\r", ch->pcdata->pagerlen );
  return;
}

/*
 * Originally by Callinon, rewritten by Greywolf 
 */
void do_mudstat( CHAR_DATA *ch, char *argument )
{
extern int top_room;
extern int top_zone;
extern int top_area;
extern int top_help;

   if ( IS_NPC(ch) )
   {
    send_to_char( "Why do you care you're a mob\n\r", ch );
    return;
   }
   set_char_color( AT_IMMORT, ch );
   ch_printf( ch, "Rooms: %d  Objects: %d  Mobiles: %d\n", top_room,numobjsloaded, nummobsloaded );
   ch_printf( ch, "Zones: %d  Areas:   %d  Helps:   %d\n",top_zone, top_area, top_help );
   ch_printf( ch, "\n\rArea File Version:  %s  \n\rZone File Version:  ZV-1-TXT\n\r",AREA_VERSION_NAME);
   show_mode( ch->desc );
   show_version( ch );
   show_boot_and_compile( ch );
   return;
}

