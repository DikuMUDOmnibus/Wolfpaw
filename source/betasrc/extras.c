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
 *			Misc Functions Module                               *
 *--------------------------------------------------------------------------*/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>   
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include "mud.h"

char    *get_multi_command      args((DESCRIPTOR_DATA *d,char *argument));
void syspanic args( ( void ) );
void show_mode args( ( DESCRIPTOR_DATA *d ) );
void write_memlog_line args( ( char *log ) );

void do_sever( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    ZONE_DATA *zone;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
       send_to_char("Who are you going to rip in half?\n\r",ch);
       return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    zone = find_zone(1);

    if(victim == ch)
    {
        send_to_char("You reach your neck fine, but cant' get your ankles.\n\r",ch);
        return;
    }

if ( !IS_NPC(victim) )
{
    if(IS_SET(victim->pcdata->flags,PCFLAG_SEVERED))
    {
	send_to_char("They have no legs to rip off!!!\n\r",ch);
	return;
    }
}
else
{
    if (IS_SET(victim->acttwo,ACT2_SEVERED) )
    {
	send_to_char("They have no legs to rip off!!!\n\r",ch);
	return;
    }
}

    if(!IS_NPC(victim))
    act(AT_CARNAGE,"$n picks you up and rips you in half! Oh Shit!",ch,NULL,victim,TO_VICT);

    act(AT_CARNAGE,"$n picks up $N and savagly rips $S legs off!",ch,NULL,victim,TO_NOTVICT);
    send_to_char("You rip them in half! *Cackle*\n\r",ch);

    obj = create_object( get_obj_index(OBJ_VNUM_LEGS,1), 0,zone );

    obj->timer = 5;

if(IS_NPC(victim))
    sprintf(buf,"A pair of %s's legs are here,twitching.",victim->short_descr);
else
    sprintf(buf,"A pair of %s's legs are here, twitching.",victim->name);
    STRFREE(obj->description);
    obj->description = STRALLOC( buf );


if(IS_NPC(victim))
    sprintf(buf,"A pair of %s's legs",victim->short_descr);
else
    sprintf(buf,"A pair of %s's legs",victim->name);
    STRFREE(obj->short_descr);
    obj->short_descr = STRALLOC( buf );

    obj_to_char(obj,ch);

 victim->hit = -30000;
 victim->position = POS_STUNNED;
 update_pos(victim);

 if ( !IS_NPC(victim) )
    SET_BIT(victim->pcdata->flags, PCFLAG_SEVERED);
 else
    SET_BIT(victim->acttwo,ACT2_SEVERED);

return;
}

char *get_multi_command( DESCRIPTOR_DATA *d, char *argument )
{
    int counter, counter2;
    char leftover[MAX_INPUT_LENGTH];
    extern char command[MSL];

    command[0] = '\0';

    for ( counter = 0; argument[counter] != '\0'; counter++ )
    {
        if ( argument[counter] == ';' &&  argument[counter+1] != ';' )
        {
             command[counter] = '\0';
             counter++;
             for (counter2 = 0; argument[counter] != '\0';
                  counter2++,counter++)
                 leftover[counter2] = argument[counter];
             leftover[counter2] = '\0';
             strcpy( d->incomm,leftover );
             return (command);
        }
        else if (argument[counter] == ';' && argument[counter+1] == ';')
            for (counter2 = counter; argument[counter2] != '\0';
                            counter2++)
                argument[counter2] = argument[counter2+1];

        command[counter] = argument[counter];

    }
    d->incomm[0] = '\0';
    command[counter] = '\0';
    return (command);
}

void do_beep ( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
	return;

    argument = one_argument( argument, arg );

    if  ( arg[0] == '\0' )
    {
	send_to_char( "Beep who?\n\r", ch );
	return;
    }

    if ( !( victim = get_char_world( ch, arg ) ) )
    {
	send_to_char( "They are not here.\n\r", ch );
	return;
    }


    if ( IS_NPC(victim))
    {
	send_to_char( "They are not beepable.\n\r", ch );
	return;
    }

    sprintf( buf, "\aYou beep %s.\n\r", victim->name );
    send_to_char( buf, ch );

    sprintf( buf, "\a%s has beeped you.\n\r", ch->name );
    send_to_char( buf, victim );
    return;
}

/*
 * Command used to test sections of the code that cannot be tested
 * in the usual running system enviroment --GW
 * make an arg, and setup the enviroment you need to test it.
 */
void do_test ( CHAR_DATA *ch, char *argument )
{
char arg[MIL];
CHAR_DATA *victim=NULL;

argument = one_argument( argument, arg );

if ( !str_cmp( arg, "syspanic" ) )
{
syspanic( );
return;
}

if ( !str_cmp( arg, "crashgame" ) )
{
send_to_char("blah",victim);
victim->pcdata->flags = 1285;
}

if ( !str_cmp( arg, "dragondrop" ) )
{
extern bool DRAGONS_RAN;
extern bool FORCE_DRAGON_RUN;

 send_to_char("Testing Dragon Drop..\n\r",ch);
 FORCE_DRAGON_RUN = TRUE;
 DRAGONS_RAN = FALSE;
 dragon_update( );
 FORCE_DRAGON_RUN = FALSE;
 return;
}

send_to_char("Not a Valid Choice, Current Choices are:\n\r",ch);
send_to_char("syspanic - test the system panic code\n\r",ch);
send_to_char("crashgame - crash the game\n\r",ch);

send_to_char("\n\rWARNING: This command is only to be used by those who\n\r",ch);
send_to_char("KNOW what these things do! --GW\n\r",ch);
return;
}

bool is_ignoring(CHAR_DATA *ch, CHAR_DATA *victim)
{
  int pos;

  if ( IS_NPC( ch ) || IS_NPC( victim ) )
  return FALSE;

  if ( !ch || !victim )
  return FALSE;

  if ( !ch->pcdata->ignore )
  return FALSE;

  for (pos = 0; pos < MAX_IGNORE; pos++)
  {
    if ( !ch->pcdata->ignore[pos] )
      break;

    if (!str_cmp(ch->pcdata->ignore[pos], victim->name))
      return TRUE;
  }

  return FALSE;
}

void do_ignore(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim, *rch;
  char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  int pos;

  one_argument(argument, arg);

  rch = ch;

  if (IS_NPC(rch))
    return;

  if (arg[0] == '\0')
  {

   if ( rch->pcdata->ignore[0] == NULL )
   {
    send_to_char("You arn't Ignoring anyone.\n\r",rch);
    return;
   }

  send_to_char("You are Ignroing:\n\r",rch);
  for (pos = 0; pos < MAX_IGNORE; pos++)
  {
    if (rch->pcdata->ignore[pos] == NULL)
      break;
    ch_printf(ch,"%s\n",capitalize(rch->pcdata->ignore[pos]) );   
    
  }

  send_to_char("\n\r",rch);
  return;
  } 

  if ((victim = get_char_world(rch, argument)) == NULL)
  {
    send_to_char("They aren't here.\n\r", ch);
    return;
  }

  if (IS_NPC(victim))
  {
    send_to_char("Ignore a mob?  I don't think so.\n\r", ch);
    return;
  }

  if (ch == victim)
  {
    send_to_char("I don't think you really want to ignore yourself.\n\r",ch);
    return;
  }

  if ( IS_IMMORTAL(victim) )
  {
    send_to_char("You cannot Ignore Immortals.\n\r",ch);
    return;
  }

  for (pos = 0; pos < MAX_IGNORE; pos++)
  {
    if (rch->pcdata->ignore[pos] == NULL)
      break;


    if (!str_cmp(victim->name, rch->pcdata->ignore[pos]))
    {
      STRFREE(rch->pcdata->ignore[pos]);
      rch->pcdata->ignore[pos] = NULL;
      sprintf(buf, "You stop ignoring %s.\n\r", victim->name);
      send_to_char(buf, ch);
      sprintf(buf, "%s stops ignoring you.\n\r", ch->name);
      send_to_char(buf, victim);
      return;
    }
   }  

  if (pos >= MAX_IGNORE)
  {
    send_to_char("You can't ignore anymore people\n\r", ch);
    return;
  }

  rch->pcdata->ignore[pos] = STRALLOC(victim->name);
  sprintf(buf, "You now ignore %s.\n\r", victim->name);
  send_to_char(buf, ch);
  sprintf(buf, "%s ignores you.\n\r", ch->name);
  send_to_char(buf, victim);
  return;
}

/*
 * Write the Game Status to a Descriptor .. for the Menu --GW 
 */
void show_mode( DESCRIPTOR_DATA *d )
{
char buf[MSL];
char buf2[MSL];
bool fMatch = FALSE;
extern bool EMAIL_AUTH;
extern bool wizlock;
extern bool ENABLE_DEATH_CRY;

if ( !d )
return;

if ( IS_NPC(d->character) )
return;

if ( mudarg_scan('B') )
{
  set_char_color( AT_PLAIN,d->character);
  sprintf(buf,"\n\nPoints of Contact for this Port:\n\r");
  write_to_buffer(d,buf,0);
  sprintf(buf,"================================\n\r");
  write_to_buffer(d,buf,0);
  sprintf(buf,"Build Port Administrators:  %s %s\n\r","Greywolf","Silvermoon");
  write_to_buffer(d,buf,0);
  sprintf(buf,"Assigned Head Builders:     %s\n\r","<NONE!?!>");
  write_to_buffer(d,buf,0);
}

set_char_color( AT_PLAIN,d->character);
strcpy(buf,"\nCurrent Game Settings:\n");
strcat(buf,"----------------------\n");

if ( chaos == 1 )
{
sprintf(buf2,"[%s] Chaos Mode\n","Mud");
strcat(buf,buf2);
fMatch = TRUE;
}
if ( doubleexp  == 1 )
{
sprintf(buf2,"[%s] Double Experiance Mode\n","Mud");
strcat(buf,buf2);
fMatch = TRUE;
}
if ( war == 1 )
{
sprintf(buf2,"[%s] War Mode\n","Mud");
strcat(buf,buf2);
fMatch = TRUE;
}
if ( TURBO_FIGHT == TRUE )
{
sprintf(buf2,"[%s] Turbo Fighting Mode\n","Mud");
strcat(buf,buf2);
fMatch = TRUE;
}
if ( IS_SET( kombat->bits, KOMBAT_ON ) )
{
sprintf(buf2,"[%s] Mortal Kombat Mode\n","Mud");
strcat(buf,buf2);
fMatch = TRUE;
}
if ( noenforce == TRUE )
{
sprintf(buf2,"[%s] In Town Law Enforcement Disabled\n","Mud");
strcat(buf,buf2);
fMatch = TRUE;
}

if ( !sysdata.NO_NAME_RESOLVING )
{
sprintf(buf2,"[%s] Hostname Resolving Enabled\n","Mud");
strcat(buf,buf2);
fMatch = TRUE;
}

if ( WEBSERVER_STATUS == FALSE )
{
sprintf(buf2,"[%s] Webserver Offline\n","Mud");
strcat(buf,buf2);
fMatch = TRUE;
}

if ( !TOPTEN_ACTIVE )
{
sprintf(buf2,"[%s] TonTen Disabled\n","Mud");
strcat(buf,buf2);
fMatch = TRUE;
}

if ( !DRAGON_DROP )
{
sprintf(buf2,"[%s] Dragon Drop Disbaled\n","Mud");
strcat(buf,buf2);
fMatch = TRUE;
}

if ( CTF_ACTIVE )
{
sprintf(buf2,"[%s] CTF Mode\n","Mud");
strcat(buf,buf2);
fMatch = TRUE;
}

if ( EMAIL_AUTH )
{
sprintf(buf2,"[%s] Email Authorization Enabled\n","Mud");
strcat(buf,buf2);
fMatch = TRUE;
}

if ( mudarg_scan('L') )
{
sprintf(buf2,"[%s] Limit Scan Disabled\n","Mud");
strcat(buf,buf2);
fMatch = TRUE;
}

if ( mudarg_scan('B') )
{
sprintf(buf2,"[%s] Game Configured as a Building Port\n","Mud");
strcat(buf,buf2);
fMatch = TRUE;
}

if ( mudarg_scan('I') )
{
sprintf(buf2,"[%s] Inter-MUD Chat offline\n","Mud");
strcat(buf,buf2);
fMatch = TRUE;
}

if ( wizlock )
{
sprintf(buf2,"[%s] Wizlock Enabled\n","Mud");
strcat(buf,buf2);
fMatch = TRUE;
}

if ( !ENABLE_DEATH_CRY )
{
sprintf(buf2,"[%s] Death Cry Disabled\n","Mud");
strcat(buf,buf2);
fMatch = TRUE;
}

if ( !fMatch )
strcat(buf,"Default Game Mode\n");

strcat(buf,"\n\r");
write_to_buffer(d,buf,0);
return;
}

/*
 * Used for Signing things .. 
 */
void do_sign( CHAR_DATA *ch, char *argument )
{
OBJ_DATA *obj;
char arg[MIL];
char buf[MSL];

buf[0] = '\0';

one_argument( argument, arg );

  if ( IS_NPC(ch) )
    return;

  obj = get_obj_list( ch, arg, ch->in_room->first_content );

  if ( !obj || (obj->in_room->vnum != ch->in_room->vnum) )
  {
  act( AT_PLAIN, "I see nothing signable here.", ch, NULL,NULL,TO_CHAR );
  return;
  }

  if (obj->item_type != ITEM_PK_BOOK )
  {
   send_to_char("I see nothing signable here..\n\r",ch);
   return;
  }
  else
  {
   act( AT_BLOOD,"$n signs $s name in blood! $n is now a Player Killer!",ch,NULL,NULL,TO_ROOM);
   act( AT_BLOOD,"You sign your name in the Book of Blood.",ch,NULL,NULL,TO_CHAR);
   SET_BIT( ch->pcdata->flags, PCFLAG_DEADLY );
   SET_BIT( ch->pcdata->flagstwo, MOREPC_SIGNEDPKILL );
   act( AT_BLOOD,"You are now a Player Killer!",ch,NULL,NULL,TO_CHAR);
   do_save(ch,"");
   sprintf(buf,"&w[&WFRESH MEAT&w]&R%s &wSigned the Book of Blood!",capitalize(ch->name));
   echo_to_all(AT_PLAIN,buf,ECHOTAR_ALL);
   return;
  }
}

void send_system_busy_message( DESCRIPTOR_DATA *d )
{
bool system_busy = FALSE;
char buf[MSL];

if ( !system_busy )
return;

sprintf( buf, "/n/r" 
"**************************************************************\n\r"
"***     The System Is Currently Busy Doing Bootup Tasks    ***\n\r"
"**************************************************************\n\r"
"**************************************************************\n\r"
"*********************** PLEASE WAIT **************************\n\r"
"**************************************************************\n\r"
"**************************************************************\n\r"
"\n\r");

return;
}

/*
 * Writes a Line to the MemLog, keeps tabs on how many lines
 * have been written, and flips over when needed --GW
 */
void write_memlog_line( char *log )
{
FILE *fp;
extern sh_int MemLogCount;
extern sh_int MemLogMax;

if ( MemLogCount < MemLogMax ) /* Not At Maximum */
{

if ( ( fp=fopen( MEMLOG_FILE, "a" ) )==NULL )
{
bug("UnAble to Open %s",MEMLOG_FILE);
perror(MEMLOG_FILE);
return;
}

MemLogCount++;
fprintf( fp, log );
new_fclose(fp);
}
else /* flip and Write */
{
MemLogCount = 0;
if ( ( fp=fopen( MEMLOG_FILE, "w" ) )==NULL )
{
bug("UnAble to Open %s",MEMLOG_FILE);
perror(MEMLOG_FILE);
return;
}

fprintf( fp, log );
new_fclose(fp);
}

return;
}

/*
 * Writes a Line to the RoomLog, keeps tabs on how many lines
 * have been written, and flips over when needed --GW
 * ADDED: accepts ags like sprintf does (vsprintf) Nov. 09/99 --GW
 */
void roomlog( char *log, ... )
{
FILE *fp;
extern sh_int RoomLogCount;
extern sh_int RoomLogMax;
char buf[MSL];

if ( RoomLogCount < RoomLogMax ) /* Not At Maximum */
{

if ( ( fp=fopen( ROOMLOG_FILE, "a" ) )==NULL )
{
bug("UnAble to Open %s",ROOMLOG_FILE);
perror(ROOMLOG_FILE);
return;
}

    buf[0] = '\0';
    {
        va_list param;
        
        va_start(param, log);
        vsprintf( buf + strlen(buf), log, param );
        va_end(param);
    }


RoomLogCount++;
fprintf( fp, buf );
new_fclose(fp);
}
else /* flip and Write */
{
RoomLogCount = 0;
if ( ( fp=fopen( ROOMLOG_FILE, "w" ) )==NULL )
{
bug("UnAble to Open %s",ROOMLOG_FILE);
perror(ROOMLOG_FILE);
return;
}

    buf[0] = '\0';
    {
        va_list param;
        
        va_start(param, log);
        vsprintf( buf + strlen(buf), log, param );
        va_end(param);
    }

fprintf( fp, buf );
new_fclose(fp);
}

return;
}

/*
 * Set the System Message --GW
 * this is to announce things .. cause no one reads MOTD's
 */
void do_sysmessage( CHAR_DATA *ch, char *argument )
{

return;
}

/* Show it to the person --GW */
void show_sys_message( void )
{

return;
}

/* Loading and Saving the Messages --GW */
void load_system_messages( void )
{

return;
}

void save_system_messages( void )
{

return;
}

void info ( char *txt )
{
char buf[MSL];
DESCRIPTOR_DATA *d;

sprintf(buf,"&B[&WINFO&B]&W %s\n\r", txt );

for( d = first_descriptor; d; d = d->next )
{
if ( d->connected == CON_PLAYING && !IS_SET( d->character->pcdata->deaf, CHANNEL_INFO ) )
send_to_char_color( buf,d->character);
}

return;
}

/* 
 * Mortal Command -- Allows you to set text BEFORE your name --GW 
 */
void do_mname( CHAR_DATA *ch, char *argument )
{

if ( IS_NPC(ch) )
return;

if ( IS_SET( ch->pcdata->flags, PCFLAG_NOTITLE ) )
{
send_to_char("The Gods Prohibit you from setting this!\n\r",ch);
return;
}

if ( argument[0] == '\0' )
{
REMOVE_BIT( ch->pcdata->flagstwo, MOREPC_NAME_SET);
send_to_char("Pre-name turned off.\n\r",ch);
return;
}

while(isspace(*argument))
argument++;
if ( argument[0] == '\0' )
return;

if ( ch->pcdata->name )
STRFREE( ch->pcdata->name );

SET_BIT( ch->pcdata->flagstwo, MOREPC_NAME_SET);
ch->pcdata->name = STRALLOC(argument);
send_to_char( "Pre-name turned on and set.\n\r",ch);
return;
}

void show_version( CHAR_DATA *ch )
{
extern char MUD_VERSION[MSL];

ch_printf(ch, "Currently Running: %s\n\r",MUD_VERSION );
return;
}

void show_boot_and_compile( CHAR_DATA *ch )
{
extern char BOOTED_BY[MSL];
extern char BOOT_TIME[MSL];
extern char COMPILED_BY[MSL];
extern char COMPILED_TIME[MSL];

ch_printf(ch, "Booted by:         %s", BOOTED_BY );
ch_printf(ch, "Boot Time:         %s", BOOT_TIME );
ch_printf(ch, "Compiled by:       %s", COMPILED_BY );
ch_printf(ch, "Compile Time:      %s", COMPILED_TIME );
return;
}

/* Area Maps.. --GW */
void do_map( CHAR_DATA *ch, char *argument )
{
char buf[MSL];
FILE *fp;

if ( IS_NPC(ch) )
  return;

if ( ch->in_room && ch->in_room->area && ch->in_room->area->map_file )
{
   if ( ch->in_room->area->map_file == NULL || !str_cmp(ch->in_room->area->map_file, "None") )
   {
     send_to_char("Hrmm.. You can't seem to find a Map for this place....\n\r",ch);
     return;
   }

   sprintf(buf,"%s%s",MAP_DIR,ch->in_room->area->map_file);
   if ( (fp=fopen(buf,"r"))==NULL)
   {
     send_to_char("Hrmm.. You can't seem to find a Map for this place....\n\r",ch);
     bug("do_map: filename specified does not exist! (%s)",ch->in_room->area->filename);
     return;
   }
   new_fclose(fp);

   show_file(ch,buf);
   return;
}   
     send_to_char("Hrmm.. You can't seem to find a Map for this place....\n\r",ch);
     return;
}
