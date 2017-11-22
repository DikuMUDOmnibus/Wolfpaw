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
 *		   Wizard Commands Module II				    *
 *--------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>
#include "mud.h"

void do_khistory( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    MOB_INDEX_DATA *mob;
    int track;
    char arg[MIL];

    if( IS_NPC(ch) || !IS_IMMORTAL(ch) )
    {
	interpret(ch,"hfdlsa"); /* ;) */
	return;
    }

    one_argument( argument, arg );

    if( arg[0] == '\0' )
    {
	send_to_char( "syntax: khistory <player>\n\r",ch );
	return;
    }
    
    vch = get_char_world(ch,arg);

    if( !vch || IS_NPC(vch) || get_trust(ch) < get_trust(vch) )
    {
	send_to_char( "You can't read that person's kill history\n\r",ch );
	return;
    }

    ch_printf(ch, "&rKill history for:&c %s\n\r", vch->name);
    
    for( track = 0; 
	 track < MAX_KILLTRACK && vch->pcdata->killed[track].vnum;
	 track++ )
    {
	mob = get_mob_index( vch->pcdata->killed[track].vnum, 1 );
	if( !mob ) continue;
	
	ch_printf(ch,"&r   %-30s&c(&r%-5d&c)   &z-&r Killed &c%d &rtimes\n\r",
		capitalize(mob->short_descr),
		mob->vnum,
		vch->pcdata->killed[track].count);
    }

    return;
}

void do_showplayer( CHAR_DATA *ch, char *argument )
{
CHAR_DATA *victim;
char arg[MIL];
char buf[MSL];
char buf2[MSL];
record_call("<do_showplayer>");

one_argument( argument, arg );

if( arg[0] == '\0' ) 
   {
    send_to_char( "Show which player??\n\r", ch );
    return;
   }

if( IS_NPC(ch) )
   {
    send_to_char( "NPCs are too stupid too read files\n\r", ch );
    return;
   }

if( !IS_SET( ch->pcdata->flags, PCFLAG_PAGERON ) )
   {
    send_to_char( "You must type config +pager first\n\r", ch );
    return;
   }

set_char_color( AT_LOG, ch );

if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
    sprintf( buf, "gunzip -f %s%c/%s", PLAYER_DIR, tolower(arg[0]), capitalize(arg));
    system(buf);
    sprintf( buf2, "%s%c/%s", PLAYER_DIR, tolower(arg[0]), capitalize(arg));
    show_file( ch, buf2 );
    sprintf( buf, "gzip -f %s%c/%s", PLAYER_DIR, tolower(arg[0]), capitalize(arg));
    system(buf);
    return;
   } 
if( ( victim = get_char_world( ch, arg ) ) != NULL )
   {
    sprintf( buf2, "%s%c/%s", PLAYER_DIR, tolower(arg[0]), capitalize(arg));
    show_file( ch, buf2 );
    return;
   }
}

void do_startroom( CHAR_DATA *ch, char *argument )
{
char buf[MSL];
char arg[MIL];
ROOM_INDEX_DATA *location;

one_argument( argument, arg );

if ( IS_NPC(ch) )
   return;

if ( arg[0] == '\0' )
{
if ( ch->in_room->level <= get_trust(ch) )
{
ch->pcdata->start_room = ch->in_room->vnum;
sprintf(buf,"Start-Room set to %d.\n\r",ch->pcdata->start_room);
save_char_obj( ch );
sprintf(buf,"%s set %s's start-room to %d",QUICKLINK(ch->name),
        QUICKLINK(ch->name), ch->pcdata->start_room );
log_string_plus( buf, LOG_NORMAL,(int)ch->level );
SET_BIT( ch->pcdata->flagstwo, MOREPC_SETSTART );
return;
}
send_to_char("I Don't think so!\n\r",ch);
return;
}


if ( ( location = find_location( ch,arg,1) )==NULL )
{
send_to_char("No such room.\n\r",ch);
return;
}

if ( location->level <= get_trust(ch) )
{
ch->pcdata->start_room = atoi(arg);
sprintf(buf,"Start-Room set to %d.\n\r",ch->pcdata->start_room);
save_char_obj( ch );
sprintf(buf,"%s set %s's start-room to %d",QUICKLINK(ch->name),
        QUICKLINK(ch->name), ch->pcdata->start_room );
log_string_plus( buf, LOG_NORMAL, (int)ch->level );
SET_BIT( ch->pcdata->flagstwo, MOREPC_SETSTART );
return;
}
send_to_char("I Don't think so!\n\r",ch);
return;
}

void do_istart( CHAR_DATA *ch, char *argument )
{
char buf[MSL];
char arg[MIL];
sh_int level;

argument = one_argument( argument, arg );

if ( arg[0] == '\0' )
{
send_to_char( "Syntax: istart <wizi level #>\n\r",ch);
return;
}

level = atoi(arg);

if ( level > get_trust(ch) )
{
send_to_char("I think not.\n\r",ch);
return;
}

SET_BIT( ch->pcdata->flagstwo, MOREPC_STARTINVIS );
ch->pcdata->invis_start = level;
sprintf( buf,"%s set %s's Invis-Start to %d", QUICKLINK(ch->name),
	QUICKLINK(ch->name), ch->pcdata->invis_start);
log_string_plus( buf, LOG_NORMAL, (int)ch->level );
send_to_char("Done.\n\r",ch);
return;
}

void do_wizlog( CHAR_DATA *ch, char *argument )
{
char arg[MIL];

one_argument( argument, arg );

if ( arg[0] == '\0' )
{
send_to_char("&wWizard Log Status:\n\r",ch);
send_to_char("------------------\n\r",ch);

ch_printf(ch,"&G(1)&w Standard Log's          %s\n\r", !IS_SET(ch->pcdata->deaf,CHANNEL_LOG) ?
		"&G(ON)" : "&R(OFF)" );
ch_printf(ch,"&G(2)&w Building and OLC Logs   %s\n\r", !IS_SET(ch->pcdata->deaf,CHANNEL_BUILD) ?
		"&G(ON)" : "&R(OFF)" );
ch_printf(ch,"&G(3)&w Communications Logs     %s\n\r",!IS_SET(ch->pcdata->deaf,CHANNEL_COMM) ?
		"&G(ON)" : "&R(OFF)" );
ch_printf(ch,"&G(4)&w Low Imm Logs            %s\n\r",!IS_SET(ch->pcdata->deaf,CHANNEL_MONITOR) ?
		"&G(ON)" : "&R(OFF)" );
ch_printf(ch,"&G(5)&w Smart-Mob Logs	    %s\n\r",!IS_SET(ch->pcdata->deaf,CHANNEL_MOBLOG) ?
		"&G(ON)" : "&R(OFF)" );
return;
}

if ( !is_number(arg) )
{
  send_to_char("Syntax: wizlog <#>\n\r",ch);
  return;
}

switch(atoi(arg))
{
  /*CHANNEL_LOG*/
  case 1:
  TOGGLE_BIT( ch->pcdata->deaf, CHANNEL_LOG );
  send_to_char("Done.\n\r",ch);
  break;  
  /* CHANNEL_BUILD*/
  case 2:
  TOGGLE_BIT( ch->pcdata->deaf, CHANNEL_BUILD );
  send_to_char("Done.\n\r",ch);
  break;  
  /* CHANNEL_COMM*/
  case 3:
  TOGGLE_BIT( ch->pcdata->deaf, CHANNEL_COMM );
  send_to_char("Done.\n\r",ch);
  break;  
  /* CHANNEL_MONITOR*/
  case 4:
  TOGGLE_BIT( ch->pcdata->deaf, CHANNEL_MONITOR );
  send_to_char("Done.\n\r",ch);
  break;  
  case 5:
  TOGGLE_BIT( ch->pcdata->deaf, CHANNEL_MOBLOG );
  break;
  default:
  send_to_char("invalid Choice.\n\r",ch);
  break;
}

return;
}

void do_approve ( CHAR_DATA *ch, char *argument )
{
char arg[MIL];
char arg2[MIL];
APPROVE_DATA *approve=NULL, *next_approve=NULL;
char buf[MSL];
char type[MSL];

argument = one_argument( argument, type );
argument = one_argument( argument, arg );
argument = one_argument( argument, arg2 );

if ( type[0] == '\0' )
{
  send_to_char("Syntax: approve <create/delete/list> <email> <password>\n\r",ch);
  send_to_char("NOTE: password not required to delete, or list Entrys.\n\r",ch);
  return;
}

if ( !str_cmp( type, "create" ) )
{

if ( arg[0] == '\0' || arg2[0] == '\0' )
{
  send_to_char("Syntax: approve <create/delete/list> <email> <password>\n\r",ch);
  send_to_char("NOTE: password not required to delete, or list Entrys.\n\r",ch);
  return;
}

CREATE( approve, APPROVE_DATA, 1 );
approve->email = STRALLOC(arg);
approve->password = STRALLOC(arg2);
LINK(approve, sysdata.first_approve, sysdata.last_approve, next, prev );

send_to_char("Character Approved.\n\r",ch);
sprintf(buf,"Email: %s Password: %s\n\r",arg,arg2);
send_to_char(buf,ch);
save_sysdata(sysdata);
return;
}

if ( !str_cmp( type, "delete" ) )
{
bool fMatch = FALSE;

if ( arg[0] == '\0'  )
{
  send_to_char("Syntax: approve <create/delete/list> <email> <password>\n\r",ch);
  send_to_char("NOTE: password not required to delete, or list Entrys.\n\r",ch);
  return;
}

  for ( approve = sysdata.first_approve; approve; approve = next_approve )
  {
    next_approve = approve->next;
    if ( !str_cmp( arg, approve->email ) )
    {
      STRFREE(approve->email);
      STRFREE(approve->password);
      UNLINK(approve,sysdata.first_approve,sysdata.last_approve,next,prev);
      DISPOSE(approve);
      fMatch = TRUE;
      break;
    }
}
if ( !fMatch )
send_to_char("Entry not found.\n\r",ch);
else
ch_printf( ch, "Approval for %s Deleted.\n\r",arg);
save_sysdata(sysdata);
return;
}

if ( !str_cmp( type, "list" ) )
{
  for ( approve = sysdata.first_approve; approve; approve = next_approve )
  {
    next_approve = approve->next;
    sprintf(buf,"Email: %-20s Pass: %s\n\r",approve->email,approve->password);
    send_to_char(buf,ch);
  }
send_to_char("Done.\n\r",ch);
return;
}

  send_to_char("Syntax: approve <create/delete/list> <email> <password>\n\r",ch);
  send_to_char("NOTE: password not required to delete, or list Entrys.\n\r",ch);
  return;
}

/* Convert an IP to a Hostname - for Gods use --GW */
void do_nslookup( CHAR_DATA *ch, char *argument )
{
 struct hostent *from;

 from=gethostbyname( (char *)argument );

 if ( !from )
 {
  ch_printf(ch,"Host not found.\n\r",0);
  return;
 } 

 ch_printf(ch,"Name Server Lookup on %s:\n\r\n\r",argument);
 ch_printf(ch,"Hostname:   %s\n\r",from->h_name);
 ch_printf(ch,"Done.\n\r",0);
 return;
}

