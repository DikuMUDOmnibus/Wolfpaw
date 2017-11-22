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
 *			   Wizard/god command module			    *
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "mud.h"

CHAR_DATA *get_char( char *name );
LIMIT_DATA *limit_lookup( int zone, int vnum );
void update_obj_load(OBJ_DATA *obj,CHAR_DATA *ch);
char *connection_names( sh_int conn );

#define RESTORE_INTERVAL 21600

char * const save_flag[] =
{ "death", "kill", "passwd", "drop", "put", "give", "auto", "zap",
"auction", "get", "receive", "idle", "backup", "r13", "r14", "r15", "r16",
"r17", "r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25", "r26", "r27",
"r28", "r29", "r30", "r31" };


/* from comm.c */
bool	write_to_descriptor	args( ( int desc, char *txt, int length ) );

/*
 * Local functions.
 */
ROOM_INDEX_DATA * find_location	args( ( CHAR_DATA *ch, char *arg,int zone ) );
void              save_banlist  args( ( void ) );
void              close_area    args( ( AREA_DATA *pArea ) );
bool 		  fight_in_progress;
int               get_color (char *argument); /* function proto */
char  *		  sector_name   args( ( int sector ) );

/*
 * Global variables.
 */

char reboot_time[50];
time_t new_boot_time_t;
extern struct tm new_boot_struct;
/*extern OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
*/

int get_saveflag( char *name )
{
    int x;

    for ( x = 0; x < sizeof(save_flag) / sizeof(save_flag[0]); x++ )
      if ( !str_cmp( name, save_flag[x] ) )
        return x;
    return -1;
}

/* Written by takeda (takeda@mathlab.sunysb.edu) */
void do_avator( CHAR_DATA *ch, char *argument ) /* Procedure Avator */
{  /* Declaration */
    char buf[MAX_STRING_LENGTH];        /* buf */
    char arg1[MAX_INPUT_LENGTH];        /* arg1 */
//    OBJ_DATA *obj_next;                 /* obj data which is a pointer */
//     OBJ_DATA *obj;                     /* obj */
    int level;                          /* level */
    int iLevel;                         /* ilevel */

    argument = one_argument( argument, arg1 );
/* Check statements */
   if ( arg1[0] == '\0' || !is_number( arg1 ) )
    {
        send_to_char( "Syntax: avator <level>.\n\r", ch );
        return;
    }

    if ( IS_NPC(ch) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

        if ( ( level = atoi( arg1 ) ) < 1 || level > MAX_LEVEL )
        {
       sprintf(buf, "Level must be 1 to %d.\n\r", MAX_LEVEL );
        send_to_char( buf, ch );
      return;
    }

    if ( level > get_trust( ch ) )
    {
        send_to_char( "Limited to your trust level.\n\r", ch );
        sprintf(buf, "Your Trust is %d.\n\r",ch->pcdata->trust);
        send_to_char(buf,ch);
        return;
    }
/* Your trust stays so you will have all immortal command */
        /* SO their trust stays there */
        if(ch->pcdata->trust == 0) {
        ch->pcdata->trust = ch->level;
	do_rank(ch,"none");
        }

/* Level gains*/
   if ( level <= ch->level )
    {
       int temp_prac;

        send_to_char( "Lowering a player's level!\n\r", ch );
        send_to_char( "**** OOOOHHHHHHHHHH  NNNNOOOO ****\n\r",ch );
        temp_prac = ch->practice;
        ch->level    = 1;
        ch->exp      = 0;
        ch->max_hit  = 20;
        ch->max_mana = 100;
        ch->max_move = 100;
        ch->practice = 0;
        ch->hit      = ch->max_hit;
        ch->mana     = ch->max_mana;
        ch->move     = ch->max_move;
        advance_level( ch, FALSE, 1 );
        ch->practice = temp_prac;
    }
    else
    {
        send_to_char( "Raising a player's level!\n\r", ch );
        send_to_char( "**** OOOOHHHHHHHHHH  YYYYEEEESSS ****\n\r", ch);
    }

    for ( iLevel = ch->level ; iLevel < level; iLevel++ )
    {
        ch->level += 1;
        advance_level( ch, FALSE, 1 );
    }
    sprintf(buf,"You are now level %d.\n\r",ch->level);
    send_to_char(buf,ch);
    ch->exp   = 0;
    save_char_obj(ch); /* save character */
    return;
}

void do_history( CHAR_DATA *ch, char *argument )
{
char arg[MAX_INPUT_LENGTH];
int comnum;
char buf[MSL];
CHAR_DATA *victim;

one_argument( argument, arg );

if ( arg[0] == '\0' )
{
send_to_char("Syntax: history <char>\n\r",ch);
return;
}

if( ( victim=get_char_world(ch,arg))==NULL)
{
send_to_char("No such player logged in.\n\r",ch);
return;
}

if( get_trust(ch) <= get_trust(victim) && victim != ch )
{
send_to_char("I think not.\n\r",ch);
return;
}

/* Where did this go!! --GW */
if ( IS_NPC(victim) )
{
  send_to_char("Not on NPC's\n\r",ch);
  return;
}

sprintf(buf,"\nCommand History for %s\n\r\n\r",capitalize(victim->name));
send_to_char(buf,ch);
buf[0] = '\0';

for( comnum = 0; comnum < MAX_COM; comnum++ )
{
if( victim->pcdata->lastcom[comnum] == NULL )
break;

sprintf(buf,"%d. %s\n",comnum+1,victim->pcdata->lastcom[comnum]);
send_to_char(buf,ch);
}
send_to_char("\nDone.\n\r",ch);
return;
}
 
void do_warmode( CHAR_DATA *ch, char *argument )
{
char arg[MAX_INPUT_LENGTH];

argument = one_argument( argument, arg );

if ( arg[0] == '\0' )
{
  if ( war == 0 )
   {
   send_to_char("War Mode [OFF]\n\r",ch);
   }
  if ( war == 1 )
   {
   send_to_char("War Mode [ACTIVE]\n\r",ch);
   }

  send_to_char("\n\rREAD HELP WARMODE BEFORE YOU ACTIVATE THIS!!\n\r",ch);
  return;
}

if ( !str_cmp( arg, "on" ) )
{
send_to_char("WARMODE ACTIVATED!\n\r",ch);
echo_to_all(AT_BLOOD,"WARNING: WARMODE ACTIVE!! (HELP WAR)",ECHOTAR_ALL);
war++;
do_chaos(ch,"on");
return;
}
if ( !str_cmp( arg, "off" ) )
{
send_to_char("WARMODE DEACTIVATED!\n\r",ch);
echo_to_all(AT_BLOOD,"WARNING: WARMODE OFF!! (DAMN!!)",ECHOTAR_ALL);
war--;
do_chaos(ch,"off");
return;
}
}
void do_warme( CHAR_DATA *ch, char *argument )
{
char buf[MAX_STRING_LENGTH];

if (IS_NPC(ch))
   return;

if ( war == 0 )
{
send_to_char("WarMode not active.\n\r",ch);
return;
}

if ( IS_SET(ch->pcdata->flagstwo, MOREPC_OUTWAR) )
{
send_to_char("You cannot rejoin untill next time!\n\r",ch);
return;
}

SET_BIT(ch->pcdata->flagstwo, MOREPC_WAR);
send_to_char("You Join the WAR Game! BANZAI!!!!!",ch);
sprintf(buf,"%s has JOINED the WAR GAME!! BANZAI!!!!\n\r",capitalize(ch->name));
echo_to_all(AT_BLOOD,buf,ECHOTAR_ALL);
do_save(ch,"");
return;
}

void do_procure( CHAR_DATA *ch, char *argument )
{
CHAR_DATA *victim;
char arg[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
OBJ_DATA *obj;
char buf[MAX_STRING_LENGTH];
char buf2[MAX_STRING_LENGTH];

argument = one_argument( argument, arg );
argument = one_argument( argument, arg2 );

if ( arg[0] == '\0' || arg2[0] == '\0' )
{
send_to_char("Syntax: procure <object> <char>\n\r",ch);
return;
}

if ( ( victim = get_char_world( ch, arg2 ) ) == NULL )
{
send_to_char( "They aren't playing.\n\r", ch);
return;
}

if (IS_NPC(victim) )
{
send_to_char("Not on NPC's\n\r",ch);
return;
}

if (get_trust(victim) >= get_trust(ch) && ch != victim)
{
send_to_char("I dont think you want to do that.\n\r",ch);
return;
}

    if ( ms_find_obj(victim) )
        return;

   if ( ( obj = get_obj_carry( victim, arg ) ) == NULL )
    {
	if ( ( obj = get_obj_wear( victim, arg ) ) == NULL )
        {
        send_to_char( "They do not have that item.\n\r", ch );
        return;
	}
        sprintf(buf,"%s suddenly falls off, into your inventory!\n\r",
            obj->name);
        send_to_char(buf,victim);
	unequip_char( victim, obj );
    }

    obj_from_char(obj);
    obj_to_char( obj, ch );
    sprintf(buf,"%s suddenly DISAPPEARS from your inventory!\n\r",
            obj->name);
    send_to_char(buf,victim);
    
    
    sprintf(buf2,"$n flicks his finger and a %s flies into his hand",
    obj->name);
    act(AT_MAGIC,buf2,ch,NULL,NULL,TO_ROOM);

    send_to_char("Item Procured.\n\r",ch);
    
    return;
}
void do_suspend( CHAR_DATA *ch, char *argument )
{
CHAR_DATA *victim;
char arg[MAX_INPUT_LENGTH];

argument = one_argument( argument, arg );

if ( arg[0] == '\0' )
{
send_to_char("Suspend who?\n\r",ch);
return;
}

if ( ( victim = get_char_world( ch, arg ) ) == NULL )
{
send_to_char( "They aren't playing.\n\r", ch);
return;
}

if (IS_NPC(victim) )
{
send_to_char("Not on NPC's\n\r",ch);
return;
}

if (get_trust(victim) >= get_trust(ch) && ch != victim)
{
send_to_char("I dont think you want to do that.\n\r",ch);
return;
}

send_to_char("*************************************************\n\r",victim);
send_to_char("***\a YOU HAVE BEEN SUSPENDED!!! GOOD-BYE!!   ***\a\n\r",victim);
send_to_char("*************************************************\n\r",victim);
SET_BIT(victim->pcdata->flags,PCFLAG_SUSPENDED);
do_save(victim,"");
do_quit2(victim,"");
send_to_char("Victim suspended.\n\r",ch);
return;
}

void do_balzhur( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
/*    AREA_DATA *pArea;   */
    int sn;

    argument = one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
    {
        send_to_char( "Who is deserving of such a fate?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't playing.\n\r", ch);
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( victim->level >= get_trust( ch ) )
    {
        send_to_char( "I wouldn't even think of that if I were you...\n\r", ch );
        return;
    }

        set_char_color( AT_WHITE, ch );
        send_to_char( "You summon the demon Balzhur to wreak your wrath!\n\r", ch);
        send_to_char( "Balzhur sneers at you evilly, then vanishes in a puff of smoke\n\r", ch);
        set_char_color( AT_IMMORT, victim );
        send_to_char( "You hear an ungodly sound in the distance that makes you want to scream in terror\n\r", ch );
        sprintf( buf, "Balzhur screams, 'You are MINE %s!!!'", victim->name );
        echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
        victim->level    = 2;
        victim->level2   = 0;
        victim->class2   = -1;
        victim->advclass = -1;
	victim->advclass2 = -1;
	victim->advlevel = 0;
	victim->advlevel2 = 0;
	REMOVE_BIT(ch->pcdata->flags, PCFLAG_ADVANCED );
	REMOVE_BIT(ch->pcdata->flags, PCFLAG_ADV_DUAL );
        victim->pcdata->trust    = 0;
        victim->exp      = 2000;
        victim->max_hit  = 10;
        victim->max_mana = 100;
        victim->max_move = 100;
        for ( sn = 0; sn < top_sn; sn++ )
            victim->pcdata->learned[sn] = 0;
        victim->practice = 0;
        victim->hit      = victim->max_hit;
        victim->mana     = victim->max_mana;
        victim->move     = victim->max_move;


    sprintf( buf, "%s%s", GOD_DIR, capitalize(victim->name) );

    if ( !remove( buf ) )
      send_to_char( "Player's immortal data destroyed.\n\r", ch );
    else if ( errno != ENOENT )
    {
      ch_printf( ch, "Unknown error #%d - %s (immortal data).  Report to
Thoric\n\r",
              errno, strerror( errno ) );
      sprintf( buf2, "%s balzhuring %s", ch->name, buf );
      perror( buf2 );
    }
/*    sprintf( buf2, "%s.are", capitalize(arg) );
    for ( pArea = first_build; pArea; pArea = pArea->next )
      if ( !strcmp( pArea->filename, buf2 ) )
      {
        sprintf( buf, "%s%s", BUILD_DIR, buf2 );
        if ( IS_SET( pArea->status, AREA_LOADED ) )
          fold_area( pArea, buf, FALSE );
        close_area( pArea );
        sprintf( buf2, "%s.bak", buf );
        if ( !rename( buf, buf2 ) )
          send_to_char( "Player's area data destroyed.  Area saved as
backup.\n\r", ch);
        else if ( errno != ENOENT )
        {
          ch_printf( ch, "Unknown error #%d - %s (area data).  Report to
Thoric.\n\r",
                  errno, strerror( errno ) );
          sprintf( buf2, "%s destroying %s", ch->name, buf );
          perror( buf2 );
        }
      }*/


        make_wizlist();
        advance_level( victim, TRUE, 1 );
        do_help(victim, "M_BALZHUR_" );
        set_char_color( AT_WHITE, victim );
        send_to_char( "You awake after a long period of time...\n\r",
victim );
        while ( victim->first_carrying )
             extract_obj( victim->first_carrying );
    return;
}

/* Doing Some cleaning and recoding here.. Aug 06/1998 --GW */
void do_atrain( CHAR_DATA *ch, char *argument )
{
     CHAR_DATA *mob;
     char      *pOutput;
     char       buf [ MAX_STRING_LENGTH ];
     int        pAbility;
     int        cost;
     bool	lvl=FALSE;
     bool	lvl2=FALSE;

     if ( IS_NPC( ch ) )
          return;
 
     /*
      * Check for trainer.
      */
          for ( mob = ch->in_room->first_person; mob; mob = mob->next_in_room )
	  { 
            if ( IS_NPC(mob) && IS_SET(mob->act, ACT_PRACTICE) )
                 break;
          }
 
     if ( !mob )
     {
         send_to_char( "You can't do that here.\n\r", ch );
         return;
     }
 
     if ( argument[0] == '\0' )
     {
      send_to_char("\n\r Cost: 5 practices for Attributes",ch);
      send_to_char(" Note: Vampires Cannot train Blood points.\n\r ",ch);
      send_to_char(" Attributes can be trained to 25 max.\n\r\n\r",ch);
      
         strcpy( buf, "You can train:" );
         if ( ch->perm_str < 25 ) strcat( buf, " str" );
 
         if ( ch->perm_int < 25 ) strcat( buf, " int" );
         if ( ch->perm_wis < 25 ) strcat( buf, " wis" );
         if ( ch->perm_dex < 25 ) strcat( buf, " dex" );
         if ( ch->perm_con < 25 ) strcat( buf, " con" );
         if ( ch->perm_lck < 25 ) strcat( buf, " lck" );
         if ( ch->perm_cha < 25 ) strcat( buf, " cha" );

	 /* Show What you can REALLY train levels wise, meaning
            the class names .. not just level --GW .. Aug/98*/
         if ( ch->exp >= exp_level(ch, ch->level+1))
	 sprintf( buf, "%s %s",buf, npc_class[ch->class]);
     
         if ( ch->class2 > -1 && ( ch->exp >= exp_level2(ch, ch->level2+1)))
	 sprintf( buf, "%s %s",buf, npc_class[ch->class2]);

 
         if ( buf[strlen( buf )-1] != ':' )
         {
             strcat( buf, ".\n\r" );
             send_to_char( buf, ch );
         }
 
         sprintf( buf, "\n\rYou have %d practice sessions.\n\r", ch->practice );
         send_to_char( buf, ch );
         return;
    }

/* Class: Mage : #: 0 */ 
if (!str_cmp(argument, "mage" ))
{
 
     if ( ch->class != 0 && ch->class2 != 0 )
     {
	send_to_char("You're not a Mage!\n\r",ch);
        return;
     }

     if ( ( ch->class == 0 && ch->level >= 50 ) ||
          ( ch->class2 == 0 && ch->level2 >= 50 ) )
     {
        send_to_char("Dont you wish!\n\r", ch);
        return;
     }

  if ( ch->class == 0 )
  {
      if ( ch->exp >= exp_level(ch, ch->level+1))
      {
	 lvl=TRUE;
         pOutput = "Mage";
         ch->level += 1;
         advance_level(ch,TRUE,1);
         send_to_char( "You advance your training in Magery!\n\r", ch );
	 act(AT_MAGIC,"$n glows shortly as $e rises a level in Magery",
            ch,NULL,NULL,TO_ROOM);
	 if ( lvl )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level,pOutput);
	 if ( lvl2 )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level2,pOutput);
	 info(buf);
         return;
       }
  }
  else
  {

    if ( ch->exp >= exp_level(ch,ch->level2+1))
    {
       lvl2=TRUE;
       pOutput = "Mage";
       ch->level2 += 1;
       advance_level2(ch,TRUE);
       send_to_char( "You advance your training in Magery!\n\r", ch );
       act(AT_MAGIC,"$n glows shortly as $e rises a level in Magery",
            ch,NULL,NULL,TO_ROOM);
	 if ( lvl )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level,pOutput);
	 if ( lvl2 )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level2,pOutput);
	 info(buf);
       return;
    }
 }
   send_to_char( "You dont have enough experience to level!\n\r", ch);
   return;
}

/* Class: Cleric: #1 */
if (!str_cmp(argument, "cleric" ))
{
 
     if ( ch->class != 1 && ch->class2 != 1 )
     {
	send_to_char("You're not a Cleric!\n\r",ch);
        return;
     }

     if ( ( ch->class == 1 && ch->level >= 50 ) ||
          ( ch->class2 == 1 && ch->level2 >= 50 ) )
     {
        send_to_char("Dont you wish!\n\r", ch);
        return;
     }

  if ( ch->class == 1 )
  {
      if ( ch->exp >= exp_level(ch, ch->level+1))
      {
	 lvl=TRUE;
         pOutput = "Cleric";
         ch->level += 1;
         advance_level(ch,TRUE,1);
         send_to_char( "You advance your training in Cleristics!\n\r", ch );
         act(AT_MAGIC,"$n glows shortly as $e rises a level in Cleristics",
            ch,NULL,NULL,TO_ROOM);
	 if ( lvl )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level,pOutput);
	 if ( lvl2 )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level2,pOutput);
	 info(buf);
         return;
       }
  }
  else
  {

    if ( ch->exp >= exp_level(ch,ch->level2+1))
    {
       ch->level2 += 1;
       lvl2=TRUE;
       pOutput = "Cleric";
       advance_level2(ch,TRUE);
       send_to_char( "You advance your training in Cleristics!\n\r", ch );
       act(AT_MAGIC,"$n glows shortly as $e rises a level in Cleristics",
            ch,NULL,NULL,TO_ROOM);
	 if ( lvl )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level,pOutput);
	 if ( lvl2 )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level2,pOutput);
	 info(buf);
       return;
    }
 }
   send_to_char( "You dont have enough experience to level!\n\r", ch);
   return;
}

/* Class: Thief: #2 */
if (!str_cmp(argument, "thief" ))
{
 
     if ( ch->class != 2 && ch->class2 != 2 )
     {
	send_to_char("You're not a Thief!\n\r",ch);
        return;
     }

     if ( ( ch->class == 2 && ch->level >= 50 ) ||
          ( ch->class2 == 2 && ch->level2 >= 50 ) )
     {
        send_to_char("Dont you wish!\n\r", ch);
        return;
     }

  if ( ch->class == 2 )
  {
      if ( ch->exp >= exp_level(ch, ch->level+1))
      {
         ch->level += 1;
         advance_level(ch,TRUE,1);
 	 lvl=TRUE;
         pOutput = "Thief";
         send_to_char( "You advance your training in Thievery!\n\r", ch );
         act(AT_MAGIC,"$n glows shortly as $e rises a level in Thievery",
            ch,NULL,NULL,TO_ROOM);
	 if ( lvl )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level,pOutput);
	 if ( lvl2 )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level2,pOutput);
	 info(buf);
         return;
       }
  }
  else
  {

    if ( ch->exp >= exp_level(ch,ch->level2+1))
    {
       ch->level2 += 1;
       advance_level2(ch,TRUE);
       lvl2=TRUE;
       pOutput = "Thief";
       send_to_char( "You advance your training in Thievery!\n\r", ch );
       act(AT_MAGIC,"$n glows shortly as $e rises a level in Thievery",
            ch,NULL,NULL,TO_ROOM);
	 if ( lvl )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level,pOutput);
	 if ( lvl2 )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level2,pOutput);
	 info(buf);
       return;
    }
} 
   send_to_char( "You dont have enough experience to level!\n\r", ch);
   return;
}

/* Class: Warrior: #3 */
if (!str_cmp(argument, "warrior" ))
{
 
     if ( ch->class != 3 && ch->class2 != 3 )
     {
	send_to_char("You're not a Warrior!\n\r",ch);
        return;
     }

     if ( ( ch->class == 3 && ch->level >= 50 ) ||
          ( ch->class2 == 3 && ch->level2 >= 50 ) )
     {
        send_to_char("Dont you wish!\n\r", ch);
        return;
     }

  if ( ch->class == 3 )
  {
      if ( ch->exp >= exp_level(ch, ch->level+1))
      {
	 lvl=TRUE;
         pOutput = "Warrior";
         ch->level += 1;
         advance_level(ch,TRUE,1);
         send_to_char( "You advance your training in the art of War!\n\r", ch );
         act(AT_MAGIC,"$n glows shortly as $e rises a level in the art of War",
            ch,NULL,NULL,TO_ROOM);
	 if ( lvl )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level,pOutput);
	 if ( lvl2 )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level2,pOutput);
	 info(buf);
         return;
       }
  }
  else
  {

    if ( ch->exp >= exp_level(ch,ch->level2+1))
    {
       lvl2=TRUE;
       pOutput = "Warrior";
       ch->level2 += 1;
       advance_level2(ch,TRUE);
       send_to_char( "You advance your training in the art of War!\n\r", ch );
       act(AT_MAGIC,"$n glows shortly as $e rises a level in the art of War",
            ch,NULL,NULL,TO_ROOM);
	 if ( lvl )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level,pOutput);
	 if ( lvl2 )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level2,pOutput);
	 info(buf);
       return;
    }
} 
   send_to_char( "You dont have enough experience to level!\n\r", ch);
   return;
}

/* Class: Vampire: #4 */

if (!str_cmp(argument, "vampire" ))
{
 
     if ( ch->class != 4 && ch->class2 != 4 )
     {
	send_to_char("You're not a Vampire!\n\r",ch);
        return;
     }

     if ( ( ch->class == 4 && ch->level >= 50 ) ||
          ( ch->class2 == 4 && ch->level2 >= 50 ) )
     {
        send_to_char("Dont you wish!\n\r", ch);
        return;
     }

  if ( ch->class == 4 )
  {
      if ( ch->exp >= exp_level(ch, ch->level+1))
      {
	 lvl=TRUE;
         pOutput = "Vampire";
         ch->level += 1;
         advance_level(ch,TRUE,1);
         send_to_char( "You advance your training in the ways of the Vampires!\n\r", ch );
         act(AT_MAGIC,"$n glows shortly as $e rises a level the ways of the Vampires",
            ch,NULL,NULL,TO_ROOM);
	 if ( lvl )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level,pOutput);
	 if ( lvl2 )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level2,pOutput);
	 info(buf);
         return;
       }
  }
  else
  {

    if ( ch->exp >= exp_level(ch,ch->level2+1))
    {
       lvl2=TRUE;
       pOutput = "Vampire";
       ch->level2 += 1;
       advance_level2(ch,TRUE);
       send_to_char( "You advance your training in the ways of the Vampires!\n\r", ch );
       act(AT_MAGIC,"$n glows shortly as $e rises a level in ways of the Vampires",
            ch,NULL,NULL,TO_ROOM);
	 if ( lvl )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level,pOutput);
	 if ( lvl2 )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level2,pOutput);
	 info(buf);
       return;
    }
} 
   send_to_char( "You dont have enough experience to level!\n\r", ch);
   return;
}

/* Class: Druid: #5 */

if (!str_cmp(argument, "druid" ))
{
 
     if ( ch->class != 5 && ch->class2 != 5 )
     {
	send_to_char("You're not a Druid!\n\r",ch);
        return;
     }

     if ( ( ch->class == 5 && ch->level >= 50 ) ||
          ( ch->class2 == 5 && ch->level2 >= 50 ) )
     {
        send_to_char("Dont you wish!\n\r", ch);
        return;
     }

  if ( ch->class == 5 )
  {
      if ( ch->exp >= exp_level(ch, ch->level+1))
      {
	 lvl=TRUE;
         pOutput = "Druid";
         ch->level += 1;
         advance_level(ch,TRUE,1);
         send_to_char( "You advance your training in the ways of Healing!\n\r", ch );
         act(AT_MAGIC,"$n glows shortly as $e rises a level in the ways of Healing",
            ch,NULL,NULL,TO_ROOM);
	 if ( lvl )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level,pOutput);
	 if ( lvl2 )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level2,pOutput);
	 info(buf);
         return;
       }
  }
  else
  {

    if ( ch->exp >= exp_level(ch,ch->level2+1))
    {
       lvl2=TRUE;
       pOutput = "Druid";
       ch->level2 += 1;
       advance_level2(ch,TRUE);
       send_to_char( "You advance your training in the ways of Healing!\n\r", ch );
       act(AT_MAGIC,"$n glows shortly as $e rises a level in the ways of Healing",
            ch,NULL,NULL,TO_ROOM);
	 if ( lvl )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level,pOutput);
	 if ( lvl2 )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level2,pOutput);
	 info(buf);
       return;
    }
} 
   send_to_char( "You dont have enough experience to level!\n\r", ch);
   return;
}

/* Class: Ranger: #6 */

if (!str_cmp(argument, "ranger" ))
{
 
     if ( ch->class != 6 && ch->class2 != 6 )
     {
	send_to_char("You're not a Ranger!\n\r",ch);
        return;
     }

     if ( ( ch->class == 6 && ch->level >= 50 ) ||
          ( ch->class2 == 6 && ch->level2 >= 50 ) )
     {
        send_to_char("Dont you wish!\n\r", ch);
        return;
     }

  if ( ch->class == 6 )
  {
      if ( ch->exp >= exp_level(ch, ch->level+1))
      {
	 lvl=TRUE;
         pOutput = "Ranger";
         ch->level += 1;
         advance_level(ch,TRUE,1);
         send_to_char( "You advance your training in the ways of the Forests!\n\r", ch );
         act(AT_MAGIC,"$n glows shortly as $e rises a level in the ways of the Forests",
            ch,NULL,NULL,TO_ROOM);
	 if ( lvl )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level,pOutput);
	 if ( lvl2 )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level2,pOutput);
	 info(buf);
         return;
       }
  }
  else
  {

    if ( ch->exp >= exp_level(ch,ch->level2+1))
    {
       lvl2=TRUE;
       pOutput = "Ranger";
       ch->level2 += 1;
       advance_level2(ch,TRUE);
       send_to_char( "You advance your training in the ways of the Forests!\n\r", ch );
       act(AT_MAGIC,"$n glows shortly as $e rises a level in the ways of the Forests",
            ch,NULL,NULL,TO_ROOM);
	 if ( lvl )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level,pOutput);
	 if ( lvl2 )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level2,pOutput);
	 info(buf);
       return;
    }
} 
   send_to_char( "You dont have enough experience to level!\n\r", ch);
   return;
}

/* Class: Augurer: #7 */

if (!str_cmp(argument, "augurer" ))
{
 
     if ( ch->class != 7 && ch->class2 != 7 )
     {
	send_to_char("You're not an Augurer!\n\r",ch);
        return;
     }

     if ( ( ch->class == 7 && ch->level >= 50 ) ||
          ( ch->class2 == 7 && ch->level2 >= 50 ) )
     {
        send_to_char("Dont you wish!\n\r", ch);
        return;
     }

  if ( ch->class == 7 )
  {
      if ( ch->exp >= exp_level(ch, ch->level+1))
      {
	 lvl=TRUE;
         pOutput = "Augurer";
         ch->level += 1;
         advance_level(ch,TRUE,1);
         send_to_char( "You advance your training in the ways of Augury!\n\r", ch );
         act(AT_MAGIC,"$n glows shortly as $e rises a level in the ways of Augery",
            ch,NULL,NULL,TO_ROOM);
	 if ( lvl )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level,pOutput);
	 if ( lvl2 )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level2,pOutput);
	 info(buf);
         return;
       }
  }
  else
  {

    if ( ch->exp >= exp_level(ch,ch->level2+1))
    {
       lvl2=TRUE;
       pOutput = "Augurer";
       ch->level2 += 1;
       advance_level2(ch,TRUE);
       send_to_char( "You advance your training in the ways of Augury!\n\r", ch );
       act(AT_MAGIC,"$n glows shortly as $e rises a level in the ways of Augery",
            ch,NULL,NULL,TO_ROOM);
	 if ( lvl )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level,pOutput);
	 if ( lvl2 )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level2,pOutput);
	 info(buf);
       return;
    }
} 
   send_to_char( "You dont have enough experience to level!\n\r", ch);
   return;
}

/* Class: Paladin: #8 */

if (!str_cmp(argument, "paladin" ))
{
 
     if ( ch->class != 8 && ch->class2 != 8 )
     {
	send_to_char("You're not a Paladin!\n\r",ch);
        return;
     }

     if ( ( ch->class == 8 && ch->level >= 50 ) ||
          ( ch->class2 == 8 && ch->level2 >= 50 ) )
     {
        send_to_char("Dont you wish!\n\r", ch);
        return;
     }

  if ( ch->class == 8 )
  {
      if ( ch->exp >= exp_level(ch, ch->level+1))
      {
	 lvl=TRUE;
         pOutput = "Paladin";
         ch->level += 1;
         advance_level(ch,TRUE,1);
         send_to_char( "You advance your training in the ways of Right and Wrong!\n\r", ch );
         act(AT_MAGIC,"$n glows shortly as $e rises a level in the ways of Right and Wrong",
            ch,NULL,NULL,TO_ROOM);
	 if ( lvl )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level,pOutput);
	 if ( lvl2 )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level2,pOutput);
	 info(buf);
         return;
       }
  }
  else
  {

    if ( ch->exp >= exp_level(ch,ch->level2+1))
    {
       lvl2=TRUE;
       pOutput = "Paladin";
       ch->level2 += 1;
       advance_level2(ch,TRUE);
       send_to_char( "You advance your training in the ways of Right and Wrong!\n\r", ch );
       act(AT_MAGIC,"$n glows shortly as $e rises a level in the ways of Right and Wrong",
            ch,NULL,NULL,TO_ROOM);
	 if ( lvl )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level,pOutput);
	 if ( lvl2 )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level2,pOutput);
	 info(buf);
       return;
    }
} 
   send_to_char( "You dont have enough experience to level!\n\r", ch);
   return;
}

/* Class: Assassin: #9 */

if (!str_cmp(argument, "assassin" ))
{
 
     if ( ch->class != 9 && ch->class2 != 9 )
     {
	send_to_char("You're not an Assassin!\n\r",ch);
        return;
     }

     if ( ( ch->class == 9 && ch->level >= 50 ) ||
          ( ch->class2 == 9 && ch->level2 >= 50 ) )
     {
        send_to_char("Dont you wish!\n\r", ch);
        return;
     }

  if ( ch->class == 9 )
  {
      if ( ch->exp >= exp_level(ch, ch->level+1))
      {
	 lvl=TRUE;
         pOutput = "Assassin";
         ch->level += 1;
         advance_level(ch,TRUE,1);
         send_to_char( "You advance your training in the ways of Death!\n\r", ch );
         act(AT_MAGIC,"$n glows shortly as $e rises a level in the ways of Death",
            ch,NULL,NULL,TO_ROOM);
	 if ( lvl )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level,pOutput);
	 if ( lvl2 )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level2,pOutput);
	 info(buf);
         return;
       }
  }
  else
  {

    if ( ch->exp >= exp_level(ch,ch->level2+1))
    {
       lvl2=TRUE;
       pOutput = "Assassin";
       ch->level2 += 1;
       advance_level2(ch,TRUE);
       send_to_char( "You advance your training in the ways of Death!\n\r", ch );
       act(AT_MAGIC,"$n glows shortly as $e rises a level in the ways of Death",
            ch,NULL,NULL,TO_ROOM);
	 if ( lvl )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level,pOutput);
	 if ( lvl2 )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level2,pOutput);
	 info(buf);
       return;
    }
} 
   send_to_char( "You dont have enough experience to level!\n\r", ch);
   return;
}

/* Class: Werewolf: #10 */

if (!str_cmp(argument, "werewolf" ))
{
 
     if ( ch->class != 10 && ch->class2 != 10 )
     {
	send_to_char("You're not a Werewolf!\n\r",ch);
        return;
     }

     if ( ( ch->class == 10 && ch->level >= 50 ) ||
          ( ch->class2 == 10 && ch->level2 >= 50 ) )
     {
        send_to_char("Dont you wish!\n\r", ch);
        return;
     }

  if ( ch->class == 10 )
  {
      if ( ch->exp >= exp_level(ch, ch->level+1))
      {
	 lvl=TRUE;
         pOutput = "Werewolf";
         ch->level += 1;
         advance_level(ch,TRUE,1);
         send_to_char( "You advance your training in the ways of the Night!\n\r", ch );
         act(AT_MAGIC,"$n glows shortly as $e rises a level in the ways of the Night",
            ch,NULL,NULL,TO_ROOM);
	 if ( lvl )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level,pOutput);
	 if ( lvl2 )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level2,pOutput);
	 info(buf);
         return;
       }
  }
  else
  {

    if ( ch->exp >= exp_level(ch,ch->level2+1))
    {
       lvl2=TRUE;
       pOutput = "Werewolf";
       ch->level2 += 1;
       advance_level2(ch,TRUE);
       send_to_char( "You advance your training in the ways of the Night!\n\r", ch );
       act(AT_MAGIC,"$n glows shortly as $e rises a level in the ways of the Night",
            ch,NULL,NULL,TO_ROOM);
	 if ( lvl )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level,pOutput);
	 if ( lvl2 )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level2,pOutput);
	 info(buf);
       return;
    }
} 
   send_to_char( "You dont have enough experience to level!\n\r", ch);
   return;
}

/* Class: Kinju: #11 */

if (!str_cmp(argument, "kinju" ))
{
 
     if ( ch->class != 11 && ch->class2 != 11 )
     {
	send_to_char("You're not a Kinju!\n\r",ch);
        return;
     }

     if ( ( ch->class == 11 && ch->level >= 50 ) ||
          ( ch->class2 == 11 && ch->level2 >= 50 ) )
     {
        send_to_char("Dont you wish!\n\r", ch);
        return;
     }

  if ( ch->class == 11 )
  {
      if ( ch->exp >= exp_level(ch, ch->level+1))
      {
	 lvl=TRUE;
         pOutput = "Kinju";
         ch->level += 1;
         advance_level(ch,TRUE,1);
         send_to_char( "You advance your training in the ways of the Ninjas!\n\r", ch );
         act(AT_MAGIC,"$n glows shortly as $e rises a level in the ways of the Ninjas",
            ch,NULL,NULL,TO_ROOM);
	 if ( lvl )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level,pOutput);
	 if ( lvl2 )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level2,pOutput);
	 info(buf);
         return;
       }
  }
  else
  {

    if ( ch->exp >= exp_level(ch,ch->level2+1))
    {
       lvl2=TRUE;
       pOutput = "Kinju";
       ch->level2 += 1;
       advance_level2(ch,TRUE);
       send_to_char( "You advance your training in the ways of the Ninjas!\n\r", ch );
       act(AT_MAGIC,"$n glows shortly as $e rises a level in the ways of Ninjas",
            ch,NULL,NULL,TO_ROOM);
	 if ( lvl )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level,pOutput);
	 if ( lvl2 )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level2,pOutput);
	 info(buf);
       return;
    }
} 
   send_to_char( "You dont have enough experience to level!\n\r", ch);
   return;
}

/* Class: Psionicist: #13 */

if (!str_cmp(argument, "psionicist" ))
{
 
     if ( ch->class != 13 && ch->class2 != 13 )
     {
	send_to_char("You're not a Psionicist!\n\r",ch);
        return;
     }

     if ( ( ch->class == 13 && ch->level >= 50 ) ||
          ( ch->class2 == 13 && ch->level2 >= 50 ) )
     {
        send_to_char("Dont you wish!\n\r", ch);
        return;
     }

  if ( ch->class == 13 )
  {
      if ( ch->exp >= exp_level(ch, ch->level+1))
      {
	 lvl=TRUE;
         pOutput = "Psionicist";
         ch->level += 1;
         advance_level(ch,TRUE,1);
         send_to_char( "You advance your training in the ways of Mind Control!\n\r", ch );
         act(AT_MAGIC,"$n glows shortly as $e rises a level in the ways of Mind Control",
            ch,NULL,NULL,TO_ROOM);
	 if ( lvl )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level,pOutput);
	 if ( lvl2 )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level2,pOutput);
	 info(buf);
         return;
       }
  }
  else
  {

    if ( ch->exp >= exp_level(ch,ch->level2+1))
    {
       lvl2=TRUE;
       pOutput = "Psionicist";
       ch->level2 += 1;
       advance_level2(ch,TRUE);
       send_to_char( "You advance your training in the ways of Mind Control!\n\r", ch );
       act(AT_MAGIC,"$n glows shortly as $e rises a level in the ways of Mind Control",
            ch,NULL,NULL,TO_ROOM);
	 if ( lvl )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level,pOutput);
	 if ( lvl2 )
	 sprintf(buf,"%s is now a level %d %s",QUICKLINK(ch->name),ch->level2,pOutput);
	 info(buf);
       return;
    }
} 
   send_to_char( "You dont have enough experience to level!\n\r", ch);
   return;
}

     
       if ( ch->practice <= 4 )
         {
      send_to_char ( "You dont have enough practices!", ch);
       return;
         }
 
   
     if ( !str_cmp( argument, "int" ) )
 {
           if ( class_table[ch->class]->attr_prime == APPLY_INT )  
         if ( ch->practice <= 4 )
         {
      send_to_char ( "You dont have enough practices!", ch);
       return;
         }
             cost    = 3;
         pAbility    = ch->perm_int;
      if ( pAbility >= 25 && ch->race != 31 )
     {
         send_to_char( "That attribute is already at maximum.", ch);
        return;
 
     }
     pOutput     = "intelligence";
         ch->perm_int += 1;
         ch->practice -= 5;
         sprintf(buf, "Your %s increases!", pOutput );
         send_to_char(buf, ch );
         return;
 
     }
     else if ( !str_cmp( argument, "lck" ) )
     {
 
            if ( class_table[ch->class]->attr_prime == APPLY_LCK )   
 if ( ch->practice <= 4 )
        {
      send_to_char ( "You dont have enough practices!", ch);
       return;
         }
 
             cost    = 3;
         pAbility    = ch->perm_lck;
         pOutput     = "luck";
  if ( pAbility >= 25 && ch->race != 31 )
     {
         send_to_char( "That attribute is already at maximum.", ch);
       return;
 
     }
         ch->perm_lck += 1;
         ch->practice -= 5;
         sprintf(buf, "Your %s increases!", pOutput );
         send_to_char(buf, ch );
         return;
     }
     else if ( !str_cmp( argument, "cha" ) )
     {
           if ( class_table[ch->class]->attr_prime == APPLY_CHA )  
        if ( ch->practice <= 4 )
         {
      send_to_char ( "You dont have enough practices!", ch);
       return;
         }
             cost    = 3;
         pAbility    = ch->perm_cha;
  if ( pAbility >= 25 && ch->race != 31 )
     {
         send_to_char( "That attribute is already at maximum.", ch);
        return;
 
     }
         pOutput     = "charm";
         ch->perm_cha += 1;
         ch->practice -= 5;
         sprintf(buf, "Your %s increases!", pOutput );
         send_to_char(buf, ch );
         return;
     }
 
      else if ( !str_cmp( argument, "str" ) )
     {
           if ( class_table[ch->class]->attr_prime == APPLY_STR )      
     if ( ch->practice <= 4 )
         {
      send_to_char ( "You dont have enough practices!", ch);
      return;
         sprintf(buf, "Your %s increases!", pOutput );
         send_to_char(buf, ch );
         return;
         }

            cost    = 3;

           cost    = 3;
        pAbility    = ch->perm_str;
  if ( pAbility >= 25 && ch->race != 31 )
     {
         send_to_char( "That attribute is already at maximum.", ch);
        return;

     }
         pOutput     = "strength";
        ch->perm_str += 1;
         ch->practice -= 5;
         sprintf(buf, "Your %s increases!", pOutput );
         send_to_char(buf, ch );
         return;
 
     }
 
     else if ( !str_cmp( argument, "wis" ) )
 
     {
           if ( class_table[ch->class]->attr_prime == APPLY_WIS )  
              if ( ch->practice <= 4 )
         {
      send_to_char ( "You dont have enough practices!", ch);
       return;
         }
        cost    = 3;
         pAbility    = ch->perm_wis;
  if ( pAbility >= 25 && ch->race != 31 )
    {
         send_to_char( "That attribute is already at maximum.", ch);
        return;
 
     }
         pOutput     = "wisdom";
        ch->perm_wis += 1;
         ch->practice -= 5;
         sprintf(buf, "Your %s increases!", pOutput );
       send_to_char(buf, ch );
         return;
 
     }
 
     else if ( !str_cmp( argument, "dex" ) )
     {
           if ( class_table[ch->class]->attr_prime == APPLY_DEX )       
    if ( ch->practice <= 4 )
         {
      send_to_char ( "You dont have enough practices!", ch);
       return;
         }
             cost    = 3;
         pAbility    = ch->perm_dex;
  if ( pAbility >= 25 && ch->race != 31 )
     {
         send_to_char( "That attribute is already at maximum.", ch); 
return;
     }
         pOutput     = "dexterity";
         ch->perm_dex += 1;
         ch->practice -= 5;
         sprintf(buf, "Your %s increases!", pOutput );
         send_to_char(buf, ch );
         return;
     }
 
     else if ( !str_cmp( argument, "con" ) )
     {
           if ( class_table[ch->class]->attr_prime == APPLY_CON )      
     if ( ch->practice <= 4 )
         {
      send_to_char ( "You dont have enough practices!", ch);
       return;
         }
             cost    = 3;
         pAbility    = ch->perm_con;
  if ( pAbility >= 25 && ch->race != 31 )
     {
         send_to_char( "That attribute is already at maximum.", ch);
        return;
 
     }
         pOutput     = "constitution";
         ch->perm_con += 1;
         ch->practice -= 5;
 
        sprintf(buf, "Your %s increases!", pOutput );
         send_to_char(buf, ch );
         return;
     }

send_to_char("Do what?\n\r",ch);
return;

  }
  
void do_wizhelp( CHAR_DATA *ch, char *argument )
{
 CMDTYPE * cmd;
 char wizcols[MAX_LEVEL - ( LEVEL_AVATAR - 1)] = "GcYPgROBCW";
 int col, hash;
 int curr_lvl;
 col = 0;
 set_pager_color( AT_PLAIN, ch );
 for ( curr_lvl = LEVEL_AVATAR; curr_lvl <= get_trust( ch ); curr_lvl++)
 {
    pager_printf( ch, "\n\r&w[&WLEVEL %-2d&w] \n\r", curr_lvl);
    col =0;
    for (hash = 0; hash < 126; hash++ )
       for ( cmd = command_hash[hash]; cmd; cmd = cmd->next)
          if (( cmd->level == curr_lvl)
           &&   cmd->level <= get_trust( ch ) )
          {
             pager_printf(ch, "&%c%-12s",
                 wizcols[URANGE(0, cmd->level - LEVEL_AVATAR,
                        MAX_LEVEL-LEVEL_AVATAR)], cmd->name);
             if ( ++col % 6 == 0 )
               send_to_pager( "\n\r", ch );
          }
 }
 send_to_pager( "\n\r", ch );
 return;
}

void do_nukerep( CHAR_DATA *ch, char *argument )
{
   char arg[MAX_STRING_LENGTH];
   CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
   {
   send_to_char( "nukerep <player>\n\r", ch );
   return;
   }

   if ((victim = get_char_world(ch, arg)) == NULL)
   {
   send_to_char( "No one like that around ...\n\r", ch );
   return;
   }

   if ( IS_NPC(victim) )
     return;

	if (!victim->pcdata->reply)
	{
	send_to_char( "That person has not replyed to you.\n\r", ch );
	return;
	}
	victim->pcdata->reply = NULL;
	send_to_char( "Player reply queue cleared.\n\r", ch );
	return;
}

void do_incognito( CHAR_DATA *ch, char *argument )
{
    int level;
    char arg[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
       return;

    /* RT code for taking a level argument */
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    /* take the default path */
 
      if ( ch->pcdata->incog_level)
      {
          ch->pcdata->incog_level = 0;
          act(AT_RED,"$n is no longer cloaked.", ch, NULL, NULL, TO_ROOM
);
          send_to_char( "You are no longer cloaked.\n\r", ch );
	REMOVE_BIT( ch->pcdata->flagstwo, MOREPC_INCOG);
      }
      else
      {
          ch->pcdata->incog_level = get_trust(ch);
          act(AT_RED, "$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You cloak your presence.\n\r", ch );
	SET_BIT( ch->pcdata->flagstwo, MOREPC_INCOG);
      }
    else
    /* do the level thing */
    {
      level = atoi(arg);
      if (level < 2 || level > get_trust(ch))
      {
        send_to_char("Incog level must be between 2 and your
level.\n\r",ch);
        return;
      }
      else
      {
          ch->pcdata->reply = NULL;
          ch->pcdata->incog_level = level;
          act(AT_RED, "$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You cloak your presence.\n\r", ch );
	SET_BIT( ch->pcdata->flagstwo, MOREPC_INCOG);
      }
    }

    return;
}

void do_unmarry( CHAR_DATA *ch, char *argument )

/*      Written April 1997 by Greywolf    */
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *vict1;
    CHAR_DATA *vict2;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
   {
        send_to_char( "Syntax:  divorce <person 1> <person 2>\n\r", ch );
        return;
    }

    if ( ( vict1 = get_char_world( ch, arg1 ) ) == NULL )
    {
        sprintf( buf, "%s is not connected.\n\r", capitalize(arg1) );
        send_to_char( buf, ch );
        return;
    }

    if ( ( vict2 = get_char_world( ch, arg2 ) ) == NULL )
    {
        sprintf( buf, "%s is not connected.\n\r", capitalize(arg2) );
        send_to_char( buf, ch );
        return;
   }

    if ( IS_NPC(vict1) || IS_NPC(vict2) )
    {
        send_to_char( "You cannot devorce a mob!\n\r",ch );
        return;
    }

    if ( vict1 == vict2 )
    {
        send_to_char( "You cannot do that.\n\r", ch );
        return;
    }

    if ( !IS_SET( vict1->pcdata->flags, PCFLAG_WED ) )
    {
   
        send_to_char( "They arn't even married!\n\r",ch);
        return;
    }

    if ( !IS_SET( vict2->pcdata->flags, PCFLAG_WED ) )
    {
        send_to_char("They arnt even married!\n\r", ch);
        return;
    }

    vict1->pcdata->spouse = NULL;
    vict2->pcdata->spouse = NULL;
 
    REMOVE_BIT( vict1->pcdata->flags, PCFLAG_WED );
    REMOVE_BIT( vict2->pcdata->flags, PCFLAG_WED );

    do_title( vict1, "." );
    do_title( vict2, "." );

    send_to_char( "They are no longer married", ch );
}

void do_marry( CHAR_DATA *ch, char *argument )

/*      Written June 1996 by Richard Sanders     */
/*      Ported to Smaug By Greywolf */
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *vict1;
    CHAR_DATA *vict2;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
   {
        send_to_char( "Syntax:  marry <person 1> <person 2>\n\r", ch );
        return;
    }

    if ( ( vict1 = get_char_world( ch, arg1 ) ) == NULL )
    {
        sprintf( buf, "%s is not connected.\n\r", capitalize(arg1) );
        send_to_char( buf, ch );
        return;
    }

    if ( ( vict2 = get_char_world( ch, arg2 ) ) == NULL )
    {
        sprintf( buf, "%s is not connected.\n\r", capitalize(arg2) );
        send_to_char( buf, ch );
        return;
   }

    if ( IS_NPC(vict1) || IS_NPC(vict2) )
    {
        send_to_char( "You cannot marry a player to a mob, silly!\n\r",
                                                                ch );
        return;
    }

    if ( vict1 == vict2 )
    {
        send_to_char( "You cannot do that.\n\r", ch );
        return;
    }

    if ( IS_SET( vict1->pcdata->flags, PCFLAG_WED ) )
    {
   
        sprintf( buf, "%s is already married to %s!\n\r", vict1->name,
                                                vict1->pcdata->spouse );
        send_to_char( buf, ch );
        return;
    }

    if ( IS_SET( vict2->pcdata->flags, PCFLAG_WED ) )
    {
        sprintf( buf, "%s is already married to %s!\n\r", vict2->name,
                                                vict2->pcdata->spouse );
        send_to_char( buf, ch );
        return;
    }

    vict1->pcdata->spouse = vict2->name;
    vict2->pcdata->spouse = vict1->name;
    vict1->pcdata->engaged = NULL;
    vict2->pcdata->engaged = NULL;
    REMOVE_BIT( vict1->pcdata->flags, PCFLAG_ENGAGED ); 
    REMOVE_BIT( vict2->pcdata->flags, PCFLAG_ENGAGED ); 
    SET_BIT( vict1->pcdata->flags, PCFLAG_WED );
    SET_BIT( vict2->pcdata->flags, PCFLAG_WED );

    do_title( vict1, "." );
    do_title( vict2, "." );

    sprintf( buf, "%s and %s are now married.", vict1->name, vict2->name );
    echo_to_all( AT_PLAIN,buf,ECHOTAR_ALL ); 
}

void do_addlag(CHAR_DATA *ch, char *argument)
{

	CHAR_DATA *victim;
	char arg1[MAX_STRING_LENGTH];
	int x;

	argument = one_argument(argument, arg1);

	if (arg1[0] == '\0')
	{
		send_to_char("addlag to who?", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg1)) == NULL)
	{
		send_to_char("They're not here.", ch);
		return;
	}

	if ((x = atoi(argument)) <= 0)
	{
		send_to_char("That makes a LOT of sense.", ch);
		return;
	}

	if (x > 100)
	{
		send_to_char("There's a limit to cruel and unusual punishment", ch);
		return;
	}

	
	WAIT_STATE(victim, x);
	send_to_char("Adding lag now...\n\r", ch);
	return;
}




void do_restrict( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    sh_int level, hash;
    CMDTYPE *cmd;
    bool found;

    found = FALSE;

    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Restrict which command?\n\r", ch );
	return;
    }

    argument = one_argument ( argument, arg2 );
    if ( arg2[0] == '\0' )
      level = get_trust( ch );
    else
      level = atoi( arg2 );

    level = UMAX( UMIN( get_trust( ch ), level ), 0 );

    hash = arg[0] % 126;
    for ( cmd = command_hash[hash]; cmd; cmd = cmd->next )
    {
	if ( !str_prefix( arg, cmd->name )
	&&    cmd->level <= get_trust( ch ) )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( found )
    {
    	if ( !str_prefix( arg2, "show" ) )
    	{
    		sprintf(buf, "%s show", cmd->name);
    		do_cedit(ch, buf);
/*    		ch_printf( ch, "%s is at level %d.\n\r", cmd->name, cmd->level );*/
    		return;
    	}
	cmd->level = level;
	ch_printf( ch, "You restrict %s to level %d\n\r",
	   cmd->name, level );
	sprintf( buf, "%s restricting %s to level %d",
	     ch->name, cmd->name, level );
	log_string( buf );
    }
    else
    	send_to_char( "You may not restrict that command.\n\r", ch );

    return;
}


void do_authorize( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  DESCRIPTOR_DATA *d;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' )
     {
     send_to_char( "Usage:  authorize <player> <yes|name|no/deny>\n\r", ch );
     send_to_char( "Pending authorizations:\n\r", ch );
     send_to_char( " Chosen Character Name\n\r", ch );
     send_to_char( "---------------------------------------------\n\r", ch );
     for ( d = first_descriptor; d; d = d->next )
         if ( (victim = d->character) != NULL &&
IS_SET(victim->pcdata->flags, PCFLAG_UNAUTHED) )
	    ch_printf( ch, " %s@%s new %s %s...\n\r",
               victim->name,
               victim->desc->host,
               race_table[victim->race]->race_name,
               class_table[victim->class]->who_name );
    return;
    }


  if ( arg2[0]=='\0' || !str_cmp( arg2,"accept" ) || !str_cmp( arg2,"yes" ))
     {
     for ( d = first_descriptor; d; d = d->next )
         if ( (victim = d->character) != NULL &&
 IS_SET(victim->pcdata->flags, PCFLAG_UNAUTHED) )
{
  if ( victim == NULL )
     return;

	if( !IS_SET(victim->pcdata->flags, PCFLAG_UNAUTHED) )
	{
	send_to_char("No one like that waiting for Authorization.\n\r",ch);
	return;
	}

	victim->pcdata->auth_state = 3;
	if ( victim->pcdata->authed_by )
	   STRFREE( victim->pcdata->authed_by );
	victim->pcdata->authed_by = QUICKLINK( ch->name );
	sprintf( buf, "%s authorized %s", ch->name,
					  victim->name );
	to_channel( buf, CHANNEL_MONITOR, "[ ** ", ch->level );
	ch_printf( ch, "You have authorized %s.\n\r", victim->name);
    
     /* Below sends a message to player when name is accepted - Brittany   */                           
        
       send_to_char( "The MUD Administrators have accepted you name.\n\r", victim);
    act( AT_MAGIC, "$N is suddenly surrounded in a godly mist!", ch,NULL,
victim, TO_ROOM );
	 char_from_room(victim);
	 char_to_room(victim,get_room_index(25518,1));
	 REMOVE_BIT(victim->pcdata->flags, PCFLAG_UNAUTHED);
	 victim->level = 2;
	advance_level(victim,TRUE,1);
     return;
}
     }
     else if ( !str_cmp( arg2, "no" ) || !str_cmp( arg2, "deny" ) )
     {
     for ( d = first_descriptor; d; d = d->next )
         if ( (victim = d->character) != NULL &&
 IS_SET(victim->pcdata->flags, PCFLAG_UNAUTHED) )
{
  if ( victim == NULL )
     return;

	send_to_char( "You have been denied authorization.\n\r", victim);
	sprintf( buf, "%s denied authorization to %s", ch->name,
						       victim->name );
     	log_string( buf );
	ch_printf( ch, "You have denied %s.\n\r", victim->name);
	do_quit2(victim, "");
}
     }

     else if ( !str_cmp( arg2, "name" ) || !str_cmp(arg2, "n" ) )
     {
     for ( d = first_descriptor; d; d = d->next )
         if ( (victim = d->character) != NULL &&
 IS_SET(victim->pcdata->flags, PCFLAG_UNAUTHED) )
{
  if ( victim == NULL )
     return;

	victim->pcdata->auth_state = 2;
	sprintf( buf, "%s has denied %s's name", ch->name,
						       victim->name );
	to_channel( buf, CHANNEL_MONITOR, "[ ** ", ch->level );
        ch_printf (victim,
          "The MUD Administrators have found the name %s "
          "to be unacceptable.\n\r"
          "You may choose a new name when you reach "               /* B */
          "the end of this area.\n\r"                               /* B */
          "The name you choose must be medieval and original.\n\r"
          "No titles, descriptive words, or names close to any existing "
          "Immortal's name.\n\r", victim->name);
	ch_printf( ch, "You requested %s change names.\n\r", victim->name);
	return;
     }
}
   
     else
     {
	send_to_char("Invalid argument.\n\r", ch);
	return;
     }

}


void do_bamfin( CHAR_DATA *ch, char *argument )
{
bool is_ok;
char bamfin_buf[MAX_STRING_LENGTH];
int str = 0;

    is_ok = FALSE;

    for ( str = 0; argument[str] != '\0'; str++ )
    {
    if ( argument[str] == '@' )
    {
       argument[str] = '%';
       argument[str+1] = 's';
       is_ok = TRUE;
       break;
    }
    }

     if ( is_ok == TRUE )
      sprintf(bamfin_buf,argument,QUICKLINK( ch->name ) );


     if ( is_ok == TRUE )
     {
	smash_tilde( bamfin_buf );
	DISPOSE( ch->pcdata->bamfin );
	ch->pcdata->bamfin = str_dup( bamfin_buf );
	send_to_char( "Ok.\n\r", ch );
	return; 
    }

     if ( ch->level == 59 )
     {
	smash_tilde( argument );
	DISPOSE( ch->pcdata->bamfin );
	ch->pcdata->bamfin = str_dup( argument );
	send_to_char( "Ok.\n\r", ch );
	return; 
    }

     send_to_char("You must have an @@ symbol in the string, indicating your name.",ch);
     return;
}



void do_bamfout( CHAR_DATA *ch, char *argument )
{
bool is_ok;
char bamfout_buf[MAX_STRING_LENGTH];
int str = 0;

    is_ok = FALSE;

    for ( str = 0; argument[str] != '\0'; str++ )
    {
    if ( argument[str] == '@' )
    {
       argument[str] = '%';
       argument[str+1] = 's';
       is_ok = TRUE;
       break;
    }
    }

     if ( is_ok == TRUE )
      sprintf(bamfout_buf,argument,QUICKLINK( ch->name ) );


     if ( is_ok == TRUE )
     {
	smash_tilde( bamfout_buf );
	DISPOSE( ch->pcdata->bamfout );
	ch->pcdata->bamfout = str_dup( bamfout_buf );
	send_to_char( "Ok.\n\r", ch );
	return; 
    }

     if ( ch->level == 59 )
     {
	smash_tilde( argument );
	DISPOSE( ch->pcdata->bamfout );
	ch->pcdata->bamfout = str_dup( argument );
	send_to_char( "Ok.\n\r", ch );
	return; 
    }

     send_to_char("You must have an @@ symbol in the string, indicating your name.",ch);
     return;

    if ( !IS_NPC(ch) )
    {
	smash_tilde( argument );
	DISPOSE( ch->pcdata->bamfout );
	ch->pcdata->bamfout = str_dup( argument );
	send_to_char( "Ok.\n\r", ch );
    }
    return;
}

void do_stitle( CHAR_DATA *ch, char *argument )
{
bool is_ok;
char stitle_buf[MAX_STRING_LENGTH];
int str = 0;

    is_ok = FALSE;

    for ( str = 0; argument[str] != '\0'; str++ )
    {
    if ( argument[str] == '@' )
    {
       argument[str] = '%';
       argument[str+1] = 's';
       is_ok = TRUE;
       break;
    }
    }

     if ( is_ok == TRUE )
      sprintf(stitle_buf,argument,QUICKLINK( ch->name ) );


     if ( is_ok == TRUE )
     {
	smash_tilde( stitle_buf );
	DISPOSE( ch->pcdata->stitle );
	ch->pcdata->stitle = str_dup( stitle_buf );
	send_to_char( "Ok.\n\r", ch );
	return; 
    }

     if ( ch->level == 59 )
     {
	smash_tilde( argument );
	DISPOSE( ch->pcdata->stitle );
	ch->pcdata->stitle = str_dup( argument );
	send_to_char( "Ok.\n\r", ch );
	return; 
    }

     send_to_char("You must have an @@ symbol in the string, indicating your name.",ch);
     return;

    if ( !IS_NPC(ch) )
    {
	smash_tilde( argument );
	DISPOSE( ch->pcdata->stitle );
	ch->pcdata->stitle = str_dup( argument );
	send_to_char( "Ok.\n\r", ch );
    }
    return;
}

void do_rank( CHAR_DATA *ch, char *argument )
{
  if ( IS_NPC(ch) )
    return;

  if ( !argument || argument[0] == '\0' )
  {
    send_to_char( "Usage: rank <string>.\n\r", ch );
    send_to_char( "   or: rank none.\n\r", ch );
    return;
  }

  smash_tilde( argument );
  DISPOSE( ch->pcdata->rank );
  if ( !str_cmp( argument, "none" ) )
    ch->pcdata->rank = str_dup( "" );
  else
    ch->pcdata->rank = str_dup( argument );
  send_to_char( "Ok.\n\r", ch );

  return;
}


void do_retire( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Retire whom?\n\r", ch );
	return;
    }

   if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( victim->level < 51 )
    {
	send_to_char( "The minimum level for retirement is 51.\n\r", ch );
	return;
    }

    if ( IS_RETIRED( victim ) )
    {
      REMOVE_BIT( victim->pcdata->flags, PCFLAG_RETIRED );
      ch_printf( ch, "%s returns from retirement.\n\r", victim->name );
      ch_printf( victim, "%s brings you back from retirement.\n\r", ch->name );
    }
    else
    { 
      SET_BIT( victim->pcdata->flags, PCFLAG_RETIRED );
      ch_printf( ch, "%s is now a retired immortal.\n\r", victim->name );
      ch_printf( victim, "Courtesy of %s, you are now a retired immortal.\n\r", ch->name );
    }
    return;
}

void do_deny( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Deny whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    SET_BIT(victim->act, PLR_DENY);
    send_to_char( "You are denied access!\n\r", victim );
    send_to_char( "OK.\n\r", ch );
    do_quit2( victim, "" );

    return;
}



void do_disconnect( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Disconnect whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
    act( AT_PLAIN, "$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( get_trust(ch) <= get_trust( victim ) )
    {
	send_to_char( "They might not like that...\n\r", ch );
	return;
    }

    for ( d = first_descriptor; d; d = d->next )
    {
	if ( d == victim->desc )
	{
	    close_socket( d, FALSE );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
    }

    bug( "Do_disconnect: *** desc not found ***.", 0 );
    send_to_char( "Descriptor not found!\n\r", ch );
    return;
}

/*
 * Force a player to quit.
 */
void do_fquit( CHAR_DATA *ch, char *argument ) 
{ 
  CHAR_DATA *victim; 
  char arg1[MAX_INPUT_LENGTH]; 
  argument = one_argument( argument, arg1 ); 
 
  if ( arg1[0] == '\0' ) 
     { 
     send_to_char( "Force whom to quit?\n\r", ch ); 
     return; 
     } 
 
  if ( !( victim = get_char_world( ch, arg1 ) ) )
     { 
     send_to_char( "They aren't here.\n\r", ch ); 
     return; 
     } 

   if ( IS_NPC(victim) )
   {
	send_to_char("You can't force a MOB to QUIT! Hello? McFly!!\n\r",ch);
	return;
   }
  
  if ( ch->level <= victim->level && ch != victim )
   {
    send_to_char("Hrm ....\n\r",ch);
    do_fquit(ch,"");
    return;
   }
  send_to_char( "The MUD administrators force you to quit\n\r", victim );
  do_quit2(victim, "");
  send_to_char( "Ok.\n\r", ch ); 
  return; 
} 


void do_forceclose( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    int desc;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Usage: forceclose <descriptor#>\n\r", ch );
	return;
    }
    desc = atoi( arg );

    for ( d = first_descriptor; d; d = d->next )
    {
	if ( d->descriptor == desc )
	{
	    if ( d->character && get_trust(d->character) >= get_trust(ch) )
	    {
		send_to_char( "They might not like that...\n\r", ch );
		return;
	    }
	    close_socket( d, FALSE );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
    }

    send_to_char( "Not found!\n\r", ch );
    return;
}



void do_pardon( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: pardon <character> <killer|cheater|thief|assassin/townoutcast> .\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "assassin" ) )
    {
	if ( IS_SET(victim->pcdata->flags, PCFLAG_ASSASSIN) )
	{
	    REMOVE_BIT( victim->pcdata->flags, PCFLAG_ASSASSIN );
	    send_to_char( "Assassin flag removed.\n\r", ch );
	    send_to_char( "You are no longer an ASSASSIN.\n\r", victim );
	}
	return;
    }

    if ( !str_cmp( arg2, "cheater" ) )
    {
	if ( IS_SET(victim->pcdata->flags, PCFLAG_CHEATER) )
	{
	    REMOVE_BIT( victim->pcdata->flags, PCFLAG_CHEATER );
	    send_to_char( "Cheater flag removed.\n\r", ch );
	    send_to_char( "You are no longer an CHEATER.\n\r", victim );
	}
	return;
    }

    if ( !str_cmp( arg2, "killer" ) )
    {
	if ( IS_SET(victim->act, PLR_KILLER) )
	{
	    REMOVE_BIT( victim->act, PLR_KILLER );
	    send_to_char( "Killer flag removed.\n\r", ch );
	    send_to_char( "You are no longer a KILLER.\n\r", victim );
	}
	return;
    }

    if ( !str_cmp( arg2, "thief" ) )
    {
	if ( IS_SET(victim->act, PLR_THIEF) )
	{
	    REMOVE_BIT( victim->act, PLR_THIEF );
	    send_to_char( "Thief flag removed.\n\r", ch );
	    send_to_char( "You are no longer a THIEF.\n\r", victim );
	}
	return;
    }

    if ( !str_cmp( arg2, "townoutcast" ) )
    {
	if ( IS_SET(victim->pcdata->flagstwo, MOREPC_TOWNOUTCAST ) )
	{
	    REMOVE_BIT( victim->pcdata->flagstwo, MOREPC_TOWNOUTCAST );
	    send_to_char( "TownOutcast flag removed.\n\r", ch );
	    send_to_char( "You are no longer a Town Outcast.\n\r", victim);
	}
	return;
    }

    send_to_char( "Syntax: pardon <character> <killer|thief>.\n\r", ch );
    return;
}


void echo_to_all( sh_int AT_COLOR, char *argument, sh_int tar )
{
    DESCRIPTOR_DATA *d;

    if ( !argument || argument[0] == '\0' )
	return;

    for ( d = first_descriptor; d; d = d->next )
    {
        /* Added showing echoes to players who are editing, so they won't
           miss out on important info like upcoming reboots. --Narn */ 
	if ( d->connected == CON_PLAYING || d->connected == CON_EDITING )
	{
	    /* This one is kinda useless except for switched.. */
	    if ( tar == ECHOTAR_PC && IS_NPC(d->character) )
	      continue;
	    else if ( tar == ECHOTAR_IMM && !IS_IMMORTAL(d->character) )
	      continue;
	    else if ( tar == ECHOTAR_SOUND && !IS_SET(d->character->pcdata->flags, PCFLAG_SOUND) )
	      continue;
	    set_char_color( AT_COLOR, d->character );
	    send_to_char( argument, d->character );
	    send_to_char( "\n\r",   d->character );
	}
    }
    return;
}

void do_echo( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    sh_int color;
    int target;
    char *parg;

    if ( IS_SET(ch->pcdata->flags, PCFLAG_NO_ECHO) )
    {
        send_to_char( "You may not echo with a noecho flag.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Echo what?\n\r", ch );
	return;
    }

    if ( (color = get_color(argument)) )
      argument = one_argument(argument, arg);
    parg = argument;
    argument = one_argument(argument, arg);
    if ( !str_cmp( arg, "PC" )
    ||   !str_cmp( arg, "player" ) )
      target = ECHOTAR_PC;
    else if ( !str_cmp( arg, "imm" ) )
      target = ECHOTAR_IMM;
    else if ( !str_cmp( arg, "sound" )
	 ||   !str_cmp( arg, "music" ) )
      target = ECHOTAR_SOUND;
    else
    {
      target = ECHOTAR_ALL;
      argument = parg;
    }
    if ( !color && (color = get_color(argument)) )
      argument = one_argument(argument, arg);
    if ( !color )
      color = AT_IMMORT;
    one_argument(argument, arg);
    if ( !str_cmp( arg, "Thoric" )
    ||   !str_cmp( arg, "Dominus" )
    ||   !str_cmp( arg, "Circe" )
    ||   !str_cmp( arg, "Haus" )
    ||   !str_cmp( arg, "Narn" )
    ||   !str_cmp( arg, "Scryn" )
    ||   !str_cmp( arg, "Damian" )
    ||   !str_cmp( arg, "Blodkai" )
    ||   !str_cmp( arg, "Greywolf" )
    ||   !str_cmp( arg, "Callinon" ))
    {
	ch_printf( ch, "I don't think %s would like that!\n\r", arg );
	return;
    }
    echo_to_all ( color, argument, target );
}

void echo_to_room( sh_int AT_COLOR, ROOM_INDEX_DATA *room, char *argument )
{
    CHAR_DATA *vic;

    for ( vic = room->first_person; vic; vic = vic->next_in_room )
    {
	set_char_color( AT_COLOR, vic );
	send_to_char( argument, vic );
	send_to_char( "\n\r",   vic );
    }
}

void do_recho( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    sh_int color;

    if ( IS_SET(ch->pcdata->flags, PCFLAG_NO_ECHO) )
    {
        send_to_char( "You may not recho with a noecho flag.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Recho what?\n\r", ch );
	return;
    }

    one_argument( argument, arg );
    if ( !str_cmp( arg, "Thoric" )
    ||   !str_cmp( arg, "Dominus" )
    ||   !str_cmp( arg, "Circe" )
    ||   !str_cmp( arg, "Haus" )
    ||   !str_cmp( arg, "Narn" )
    ||   !str_cmp( arg, "Scryn" )
    ||   !str_cmp( arg, "Blodkai" )
    ||   !str_cmp( arg, "Greywolf" )
    ||   !str_cmp( arg, "Damian" ) )
    {
	ch_printf( ch, "I don't think %s would like that!\n\r", arg );
	return;
    }
    if ( (color = get_color ( argument )) )
       {
       argument = one_argument ( argument, arg );
       echo_to_room ( color, ch->in_room, argument );
       }
    else
       echo_to_room ( AT_IMMORT, ch->in_room, argument );
}


ROOM_INDEX_DATA *find_location( CHAR_DATA *ch, char *arg, int zone )
{
    char arg1[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    extern int top_zone;

    one_argument( arg, arg1 );

    if ( zone<=0 )
    zone = 1;
    if ( zone > top_zone )
    {
     bug("Find_Location: zone greater then top_zone!!",0);
     bug("Fixing...",0);
     zone = top_zone;
    }
    if ( is_number(arg1) )
	return get_room_index( atoi( arg1 ), zone );

    if ( ( victim = get_char_world( ch, arg1 ) ) != NULL )
	return victim->in_room;

    if ( ( obj = get_obj_world( ch, arg1 ) ) != NULL )
	return obj->in_room;

    return NULL;
}



void do_transfer( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;
    int zone=1;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Transfer whom (and where)?\n\r", ch );
	return;
    }

    if ( arg3[0] != '\0' )
    zone = atoi(arg3);

    if ( !str_cmp( arg1, "all" ) )
    {
	for ( d = first_descriptor; d; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   d->character != ch
	    &&   d->character->in_room
	    &&   d->newstate != 2
	    &&   can_see( ch, d->character ) )
	    {
		char buf[MAX_STRING_LENGTH];
		sprintf( buf, "%s %s", d->character->name, arg2 );
		do_transfer( ch, buf );
	    }
	}
	return;
    }

    /*
     * Thanks to Grodyn for the optional location parameter.
     */
    if ( arg2[0] == '\0' )
    {
	location = ch->in_room;
	if ( ch->in_room && ch->in_room->area && ch->in_room->area->zone )
	zone = (int)ch->in_room->area->zone->number;
	else
	zone = 1;
    }
    else
    {
	if ( ( location = find_location( ch, arg2, zone ) ) == NULL )
	{
	    send_to_char( "No such location.\n\r", ch );
	    return;
	}

	if ( room_is_private( location ) )
	{
	    send_to_char( "That room is private right now.\n\r", ch );
	    return;
	}
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
      send_to_char( "You cannot transfer yourself, use goto.\n\r",ch);
      return;
    }
    
    if ( victim->level > ch->level && !IS_NPC(victim))
    {
      send_to_char( "You cannot transfer people higher level than you!\n\r", ch);
      return;
    }

    if (NOT_AUTHED(victim))
    {
	send_to_char( "They are not authorized yet!\n\r", ch);
	return;
    }

    if ( !victim->in_room )
    {
	send_to_char( "They are in limbo.\n\r", ch );
	return;
    }
    
    if ( ch->level < location->level )
    {
      send_to_char("You arn't allowed in there!",ch);
      return;
    }

    if ( victim->fighting )
	stop_fighting( victim, TRUE );
    act( AT_MAGIC, "$n disappears in a cloud of swirling colors.", victim, NULL, NULL, TO_ROOM );
    victim->retran = victim->in_room->vnum;
    char_from_room( victim );
    char_to_room( victim, location );
    act( AT_MAGIC, "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM );
    if ( ch != victim )
    act( AT_IMMORT, "$n has transferred you.", ch, NULL, victim, TO_VICT );
    do_look( victim, "auto" );
    send_to_char( "Ok.\n\r", ch );
    if (!IS_IMMORTAL(victim) && !IS_NPC(victim) 
    &&  !in_hard_range( victim, location->area ) ) 
    send_to_char("Warning: the player's level is not within the area's level range.\n\r", ch);
}

void do_retran( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	char buf[MAX_STRING_LENGTH];
	
	argument = one_argument( argument, arg );
	if ( arg[0] == '\0' )
	{
		send_to_char("Retransfer whom?\n\r", ch );
		return;
	}
	if ( !(victim = get_char_world(ch, arg)) )
	{
		send_to_char("They aren't here.\n\r", ch );
		return;
	}
	sprintf(buf, "'%s' %d", victim->name, victim->retran);
	do_transfer(ch, buf);
	return;
}

void do_regoto( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];

        if ( IS_NPC(ch) )
           return;	

	sprintf(buf, "%d", ch->pcdata->regoto);
	do_goto(ch, buf);
	return;
}
	
void do_at( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    CHAR_DATA *wch;
    int	zone;
    ZONE_DATA *in_zone;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "At where what?\n\r", ch );
	return;
    }

    if ( ch->in_room->area && ch->in_room->area->zone )
    zone = (int)ch->in_room->area->zone->number;
    else
    zone = 1;

    if (!str_cmp( argument, "maim" ) )
    {
	send_to_char("No... dont use at, just maim <player>\n\r",ch);
	return;
    }

    if ( ( location = find_location( ch, arg, zone ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if ( room_is_private( location ) )
    {
      if ( get_trust( ch ) < LEVEL_GOD )
      {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
      }
      else
      {
	send_to_char( "Overriding private flag!\n\r", ch );
      }
      
    }

    if ( location->level > ch->level )
    {
      send_to_char("You arn't allowed in there!\n\r",ch);
      return;
    }
    original = ch->in_room;
    char_from_room( ch );
    char_to_room( ch, location );
    interpret( ch, argument );

    /*
     * See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case.
     */
    for ( wch = first_char; wch; wch = wch->next )
    {
	if ( wch == ch )
	{
	    char_from_room( ch );
	    char_to_room( ch, original );
	    break;
	}
    }

    for( in_zone = first_zone; in_zone; in_zone = in_zone->next )
    {
    for ( wch = in_zone->first_mob; wch; wch = wch->next )
    {
	if ( wch == ch )
	{
	    char_from_room( ch );
	    char_to_room( ch, original );
	    break;
	}
    }
}
    return;
}

void do_rat( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    int Start, End, vnum;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Syntax: rat <start> <end> <command>\n\r", ch );
	return;
    }

    Start = atoi( arg1 );	End = atoi( arg2 );

    if ( Start < 1 || End < Start || Start > End || Start == End || End > 32767 )
    {
	send_to_char( "Invalid range.\n\r", ch );
	return;
    }

    if ( !str_cmp( argument, "quit" ) )
    {
	send_to_char( "I don't think so!\n\r", ch );
	return;
    }

    original = ch->in_room;
    for ( vnum = Start; vnum <= End; vnum++ )
    {
	if ( (location =get_room_index(vnum,(int)ch->in_room->area->zone->number)) == NULL )
	  continue;
	char_from_room( ch );
	char_to_room( ch, location );
	interpret( ch, argument );
    }

    char_from_room( ch );
    char_to_room( ch, original );
    send_to_char( "Done.\n\r", ch );
    return;
}


/*
 * Modified RAT to work with Mobiles --GW
 */
void do_mat( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *mob;
    int Start, End, vnum;
    char buf[MSL];

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Syntax: mat <start> <end> <command>\n\r", ch );
	return;
    }

    Start = atoi( arg1 );	End = atoi( arg2 );

    if ( Start < 1 || End < Start || Start > End || Start == End || End > 32767 )
    {
	send_to_char( "Invalid range.\n\r", ch );
	return;
    }

    if ( !str_cmp( argument, "quit" ) )
    {
	send_to_char( "I don't think so!\n\r", ch );
	return;
    }

    for ( vnum = Start; vnum <= End; vnum++ )
    {
	if ( (mob=get_mob_index(vnum,(int)ch->in_room->area->zone->number)) ==NULL)
	  continue;

	/* If the Mobile has no Proto Flag,.. set one on it --GW */
	if ( !IS_SET( mob->act, ACT_PROTOTYPE ) )
	SET_BIT(mob->act, ACT_PROTOTYPE);

	sprintf(buf,"%d %s", mob->vnum, argument );
	do_mset( ch, buf );
    }

    send_to_char( "Done.\n\r", ch );
    return;
}


/*
 * Modified RAT to work with Objects --GW
 */
void do_oat( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *obj;
    int Start, End, vnum;
    char buf[MSL];

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Syntax: mat <start> <end> <command>\n\r", ch );
	return;
    }

    Start = atoi( arg1 );	End = atoi( arg2 );

    if ( Start < 1 || End < Start || Start > End || Start == End || End > 32767 )
    {
	send_to_char( "Invalid range.\n\r", ch );
	return;
    }

    if ( !str_cmp( argument, "quit" ) )
    {
	send_to_char( "I don't think so!\n\r", ch );
	return;
    }

    for ( vnum = Start; vnum <= End; vnum++ )
    {
	if ( (obj=get_obj_index(vnum,(int)ch->in_room->area->zone->number))==NULL)
	  continue;

	/* If the Object has no Proto Flag,.. set one on it --GW */
	if ( !IS_SET( obj->extra_flags, ITEM_PROTOTYPE ) )
	SET_BIT(obj->extra_flags, ITEM_PROTOTYPE);

	sprintf(buf,"%d %s", obj->vnum, argument );
	do_oset( ch, buf );
    }

    send_to_char( "Done.\n\r", ch );
    return;
}


void do_rstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    OBJ_DATA *obj;
    CHAR_DATA *rch;
    EXIT_DATA *pexit;
    int cnt,zone;
    static char *dir_text[] = { "n", "e", "s", "w", "u", "d", "ne", "nw", "se", "sw", "?" };

    one_argument( argument, arg );
    if ( !str_cmp( arg, "exits" ) )
    {
	location = ch->in_room;

	ch_printf( ch, "Exits for room '%s.' vnum %d\n\r",
		location->name,
		location->vnum );

	for ( cnt = 0, pexit = location->first_exit; pexit; pexit = pexit->next )
	    ch_printf( ch,
		"%2d) %2s to %-5d.  Key: %d  Flags: %d  Keywords: '%s'.\n\rDescription: %sExit links back to vnum: %d  Exit's RoomVnum: %d  Distance: %d\n\r",
		++cnt,
		dir_text[pexit->vdir],
		pexit->to_room ? pexit->to_room->vnum : 0,
		pexit->key,
		pexit->exit_info,
		pexit->keyword,
		pexit->description[0] != '\0'
		    ? pexit->description : "(none).\n\r",
		pexit->rexit ? pexit->rexit->vnum : 0,
		pexit->rvnum,
		pexit->distance );
	return;
    }
    
    if ( ch->in_room->area->zone )
    zone = (int)ch->in_room->area->zone->number;
    else
    zone = 1;

    location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg,zone );
    if ( !location )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if ( ch->in_room != location && room_is_private( location ) )
    {
      if ( get_trust( ch ) < LEVEL_GOD )
      {
        send_to_char( "That room is private right now.\n\r", ch );
        return;
      }
      else
      {
        send_to_char( "Overriding private flag!\n\r", ch );
      }

    }

    ch_printf( ch, "Name: %s.\n\rArea: %s  Filename: %s.\n\r",
	location->name,
	location->area ? location->area->name : "None????",
	location->area ? location->area->filename : "None????" );

    ch_printf( ch,
	"Vnum: %d.  Sector: %s.  Light: %d.  TeleDelay: %d.  TeleVnum: %d Tunnel: %d.\n\r",
	location->vnum,
	sector_name(location->sector_type),
	location->light,
	location->tele_delay,
	location->tele_vnum,
	location->tunnel );

    ch_printf( ch, "Room flags: %s\n\r",
	flag_string(location->room_flags, r_flags) );
    ch_printf( ch, "Character Count: %d\n\r",location->room_cnt);
    ch_printf( ch, "Description:\n\r%s", location->description );

    if ( location->first_extradesc )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Extra description keywords: '", ch );
	for ( ed = location->first_extradesc; ed; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next )
		send_to_char( " ", ch );
	}
	send_to_char( "'.\n\r", ch );
    }

    send_to_char( "Characters:", ch );
    for ( rch = location->first_person; rch; rch = rch->next_in_room )
    {
	if ( can_see( ch, rch ) )
	{
	  send_to_char( " ", ch );
	  one_argument( rch->name, buf );
	  send_to_char( buf, ch );
	}
    }

    send_to_char( ".\n\rObjects:   ", ch );
    for ( obj = location->first_content; obj; obj = obj->next_content )
    {
	send_to_char( " ", ch );
	one_argument( obj->name, buf );
	send_to_char( buf, ch );
    }
    send_to_char( ".\n\r", ch );
    sprintf(buf,"Level: %d.\n\r", location->level);
	send_to_char( buf, ch );
    ch_printf( ch,"Zone: %d   Area: %s\n\r",
        location->area->zone ? location->area->zone->number : 0,
	location->area ? location->area->name : "None" );

    ch_printf( ch,"Modified by: %s  Date: %s\n\r",location->modified_by,location->modified_date);

    send_to_char( ".\n\r", ch );
     
    if ( location->first_exit )
	send_to_char( "------------------- EXITS -------------------\n\r", ch );
    for ( cnt = 0, pexit = location->first_exit; pexit; pexit = pexit->next )
	ch_printf( ch,
	  "%2d) %-2s to %-5d.  Key: %d  Flags: %d  Keywords: %s.\n\r",
		++cnt,
		dir_text[pexit->vdir],
		pexit->to_room ? pexit->to_room->vnum : 0,
		pexit->key,
		pexit->exit_info,
		pexit->keyword[0] != '\0' ? pexit->keyword : "(none)" );
    return;
}



void do_ostat( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    OBJ_DATA *obj;
    LIMIT_DATA *limit;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Ostat what?\n\r", ch );
	return;
    }
    if ( arg[0] != '\'' && arg[0] != '"' && strlen(argument) > strlen(arg) )
	strcpy( arg, argument );

    if ( ( obj = get_obj_world( ch, arg ) ) == NULL )
    {
	send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
	return;
    }

    ch_printf( ch, "Name: %s.\n\r",
	obj->name );

    ch_printf( ch, "Vnum: %d.  Type: %s.  Count: %d  Gcount: %d\n\r",
	obj->pIndexData->vnum, item_type_name( obj ), obj->pIndexData->count,
	obj->count );
    
    if( !obj->pIndexData->area )
       { 
    send_to_char( "Zone: (No Zone) Area: (No Area)\n\r", ch );
       }
    else {
    ch_printf( ch, "Zone: %d  Area: %s\n\r",obj->pIndexData->area->zone->number,
	obj->pIndexData->area->name );
         }

    ch_printf( ch, "Index Modified by: %s  Date: %s\n\r", obj->pIndexData->modified_by,
		obj->pIndexData->modified_date);

    if ( obj->modified_by && str_cmp( obj->modified_by, "(null)") )
    ch_printf( ch, "Obj Modified by: %s Date: %s\n\r", obj->modified_by,obj->modified_date);

    ch_printf( ch, "Loaded By: %s\n\r", obj->loaded_by ? obj->loaded_by : "System" );

    ch_printf( ch, "Serial#: %d  TopIdxSerial#: %d  TopSerial#: %d\n\r",
	obj->serial, obj->pIndexData->serial, cur_obj_serial );

    ch_printf( ch, "Short description: %s.\n\rLong description: %s\n\r",
	obj->short_descr, obj->description );

    if ( obj->action_desc[0] != '\0' )
	ch_printf( ch, "Action description: %s.\n\r", obj->action_desc );

    ch_printf( ch, "Wear flags : %s\n\r", flag_string(obj->wear_flags, w_flags) );
    ch_printf( ch, "(1)Extra flags: %s\n\r",flag_string(obj->extra_flags,o_flags)); 
    ch_printf( ch, "(2)Extra flags: %s\n\r", flag_string(obj->second_flags,obj_flags) );

    ch_printf( ch, "Number: %d/%d.  Weight: %d/%d.  Layers: %d\n\r",
	1,           get_obj_number( obj ),
	obj->weight, get_obj_weight( obj ), obj->pIndexData->layers );

    ch_printf( ch, "Cost: %d.  Rent: %d.  Timer: %d.  Level: %d\n\r",
	obj->cost, obj->pIndexData->rent, obj->timer, obj->level );

    limit=limit_lookup(1,(int)obj->pIndexData->vnum);

    if ( limit )
    {
    ch_printf( ch, "Minimum Level: %d.  LIMIT:[%d/%d]\n\r",
        obj->pIndexData->minlevel,limit->loaded,limit->limit);
    }
    else
    {
    ch_printf( ch, "Minimum Level: %d. Limit:[N/A]\n\r",
	obj->pIndexData->minlevel);
    }

    ch_printf( ch, "Ego: %d\n\r",obj->pIndexData->ego );
    ch_printf( ch,
	"In room: %d.  In object: %s.  Carried by: %s.  Wear_loc: %d.\n\r",
	obj->in_room    == NULL    ?        0 : obj->in_room->vnum,
	obj->in_obj     == NULL    ? "(none)" : obj->in_obj->short_descr,
	obj->carried_by == NULL    ? "(none)" : obj->carried_by->name,
	obj->wear_loc );

    ch_printf( ch, "Index Values : %d %d %d %d %d %d.\n\r",
	obj->pIndexData->value[0], obj->pIndexData->value[1],
	obj->pIndexData->value[2], obj->pIndexData->value[3],
	obj->pIndexData->value[4], obj->pIndexData->value[5] );
    ch_printf( ch, "Object Values: %d %d %d %d %d %d.\n\r",
	obj->value[0], obj->value[1], obj->value[2], obj->value[3], obj->value[4], obj->value[5] );

    if ( obj->pIndexData->first_extradesc )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Primary description keywords:   '", ch );
	for ( ed = obj->pIndexData->first_extradesc; ed; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next )
		send_to_char( " ", ch );
	}
	send_to_char( "'.\n\r", ch );
    }
    if ( obj->first_extradesc )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Secondary description keywords: '", ch );
	for ( ed = obj->first_extradesc; ed; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next )
		send_to_char( " ", ch );
	}
	send_to_char( "'.\n\r", ch );
    }

    for ( paf = obj->first_affect; paf; paf = paf->next )
	ch_printf( ch, "Affects %s by %d. (extra)\n\r",
	    affect_loc_name( paf->location ), paf->modifier );

    for ( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
	ch_printf( ch, "Affects %s by %d.\n\r",
	    affect_loc_name( paf->location ), paf->modifier );

    return;
}

void do_pstat( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
        
    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if( arg[0] == '\0' )
       {
    send_to_char( "Pstat whom?\n\r", ch );
    return;
       }
    if( (victim = get_char_world(ch,arg)) == NULL )
       {
    send_to_char( "They aren't here\n\r", ch );
    return;
       }

    if( IS_NPC(victim) )
       {
    send_to_char( "Not on NPCs\n\r", ch );
    return;
       }
    if( get_trust(ch) < get_trust(victim) )   
       {
    send_to_char( "You failed..\n\r", ch );
    return;
       }
    set_char_color( AT_SCORE, ch );
    ch_printf( ch, "Name: %s     Level: %d    \n\r",
	victim->name,
	victim->level );
    ch_printf( ch, "HP: %d/%d     %s:%d/%d     Move: %d/%d     \n\r",
	victim->hit,
	victim->max_hit,
	IS_VAMPIRE(victim) ? "Blood" : "Mana",
	IS_VAMPIRE(victim) ? victim->pcdata->condition[COND_BLOODTHIRST] : victim->mana, 
        IS_VAMPIRE(victim) ? get_bloodthirst(victim) : victim->max_mana,
	victim->move,
	victim->max_move );
    ch_printf( ch, "Mental: %d     Emotion: %d          \n\r",
	victim->mental_state,
	victim->emotional_state );
    ch_printf( ch, "Glory: %d     Favor: %d        \n\r",
	victim->pcdata->quest_curr,
	victim->pcdata->favor );
    if ( victim->pcdata->bounty > 0 )
	ch_printf( ch, "%s has %d gold on %s head\n\r",
	   victim->name,
	   victim->pcdata->bounty,
	   victim->sex == SEX_MALE ? "his"
	 : victim->sex == SEX_FEMALE ? "her" : "its" );
    if( victim->pcdata->clan )
       {
	 send_to_char( "---------------------------\n\r", ch );
	 send_to_char( "Clan Data \n\r", ch );
	 ch_printf( ch, "Name: %s   Level: %d \n\r",
		victim->pcdata->clan->name,
		victim->pcdata->clevel );
       }
    else
       {
	 send_to_char( "---------------------------\n\r", ch );
	 send_to_char( "Clan Data \n\r", ch );
	 ch_printf( ch, "No Clan      %s  \n\r",
		IS_SET(victim->act, PLR_OUTCAST) 
		? "(OUTCAST)"
		: " " );
       }
       send_to_char("Use pstat <char> objs to show a list of the obj's the player has on this character.\n\r",ch);

	if ( arg2[0] == '\0' )
	return;
       
  if ( !str_cmp( arg2, "objs" ) )
  {
    FILE *fp;
    char buf[MSL];

    send_to_char("The code for this is not complete.\n\r",ch);
    return;

           sprintf(buf,"%s%c/%s", PLAYER_OBJ_DIR,tolower(victim->name[0]),
                capitalize(victim->name));

    if ( ( fp=fopen( buf, "r" ) )==NULL)
    {
      send_to_char("No Objects File found for that character.\n\r",ch);
      return;
    }
        for ( ; ; )
        {
            char letter;
            char *word;

            letter = fread_letter( fp );
            if ( letter == '*' )
            {
                fread_to_eol( fp );
                continue;
            }

            if ( letter != '#' )
            {
                bug( "Load_char_obj: # not found.", 0 );
                break;
            }

            word = fread_word( fp );
            if ( !str_cmp( word, "OBJECT" ) )
            {
                load_limited_toggle(FALSE);
                fread_obj  ( ch, fp, OS_PCO );
                load_limited_toggle(TRUE);
            }
            else if ( !str_cmp( word, "END" ) )
                break;
            else
            {
                bug("Fread_Player: Bad Obj File. (%s)", word);
                break;
            }
           }
                new_fclose(fp);
  }
  return;

}		

void do_mstat( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    CHAR_DATA *victim;
    SKILLTYPE *skill;
    int x=0;

    set_char_color( AT_PLAIN, ch );

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Mstat whom?\n\r", ch );
	return;
    }

    if ( arg[0] != '\'' && arg[0] != '"' && strlen(argument) > strlen(arg))
	strcpy( arg, argument );

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( get_trust( ch ) < get_trust( victim ) && !IS_NPC(victim) )
    {
	set_char_color( AT_IMMORT, ch );
	send_to_char( "Their godly glow prevents you from getting a good look.\n\r", ch );
	return;
    }

    ch_printf( ch, "Name: %s     Clan: %s     %s\n\r",
	victim->name,
	( IS_NPC( victim ) || !victim->pcdata->clan ) ? "(none)" 
			       : victim->pcdata->clan->name,
        IS_NPC( victim ) ? "" : IS_SET( victim->pcdata->flags, PCFLAG_DEADLY )
        ? "Path: Pkill" : "Path: Non-pkill" );
    if( get_trust(ch) >= LEVEL_GOD && !IS_NPC(victim) && victim->desc )
	ch_printf( ch, "User: %s@%s   Descriptor: %d   Trust: %d   AuthedBy: %s\n\r",
		victim->desc->user,victim->desc->host,victim->desc->descriptor,
		victim->level,victim->pcdata->authed_by[0] != '\0'
		? victim->pcdata->authed_by : "(unknown)" );
    if ( !IS_NPC(victim) && victim->pcdata->release_date != 0 )
      ch_printf(ch, "Helled until %24.24s by %s.\n\r",
              ctime(&victim->pcdata->release_date),
              victim->pcdata->helled_by);
  
  if( IS_NPC(victim)
  &&  victim->pIndexData->area )
  {
  ch_printf( ch, "Zone: %d   Area: %s\n\r",victim->pIndexData->area->zone->number,
	victim->pIndexData->area->name);  
  }
  ch_printf( ch, "Vnum: %d   Sex: %s   Room: %d   Count: %d  Killed: %d\n\r",
	IS_NPC(victim) ? victim->pIndexData->vnum : 0,
	victim->sex == SEX_MALE    ? "male"   :
	victim->sex == SEX_FEMALE  ? "female" : "neutral",
	victim->in_room == NULL    ?        0 : victim->in_room->vnum,
	IS_NPC(victim) ? victim->pIndexData->count : 1,
	IS_NPC(victim) ? victim->pIndexData->killed
		       : victim->pcdata->mdeaths + victim->pcdata->pdeaths
	);

  if ( IS_NPC(victim) )
  ch_printf( ch, "Modified by: %s  Modified Date: %s\n\r",
		victim->pIndexData->modified_by,victim->pIndexData->modified_date);

    ch_printf( ch, "Str: %d   Int: %d   Wis: %d   Dex: %d   Con: %d   Cha: %d   Lck: %d\n\r",
	get_curr_str(victim),
	get_curr_int(victim),
	get_curr_wis(victim),
	get_curr_dex(victim),
	get_curr_con(victim),
	get_curr_cha(victim),
	get_curr_lck(victim) );
    if (IS_VAMPIRE(victim) && !IS_NPC(victim))
    ch_printf( ch, "Hps: %d/%d   Blood: %d/%d   Move: %d/%d   Practices: %d\n\r",
        victim->hit,         victim->max_hit,  
        victim->pcdata->condition[COND_BLOODTHIRST], get_bloodthirst(victim),
        victim->move,        victim->max_move,
        victim->practice );
    else
    ch_printf( ch, "Hps: %d/%d   Mana: %d/%d   Move: %d/%d   Practices: %d\n\r",
        victim->hit,         victim->max_hit,
        victim->mana,        victim->max_mana,
        victim->move,        victim->max_move,
        victim->practice );  
    ch_printf( ch,
	"Level: %d  Class/Race: %d/%d  Align: %d  AC: %d  Gold: %d  XP: %d\n\r",
	victim->level,  victim->class,  victim->race,   victim->alignment,
	GET_AC(victim),      victim->gold,         victim->exp );
    if ( victim->class < MAX_NPC_CLASS && victim->class >= 0
    &&   victim->race  < MAX_NPC_RACE  && victim->race  >= 0 )
	ch_printf( ch, "Class: %s  Race: %s  Deity: %s\n\r",
	  npc_class[victim->class],
	  npc_race[victim->race],
          ( IS_NPC( victim ) || !victim->pcdata->deity ) ? "(none)"
                               : victim->pcdata->deity->name );
    if ( !IS_NPC(victim) && victim->class2 >= 0 && victim->level2 >= 0 )
    ch_printf( ch, "Class 2: %s Level 2: %d\n\r",
      npc_class[victim->class2], victim->level2 );
    ch_printf( ch, "Hitroll: %d   Damroll: %d   Position: %d   Wimpy: %d \n\r",
	GET_HITROLL(victim), GET_DAMROLL(victim),
	victim->position,    victim->wimpy );
    ch_printf( ch, "Ego: %d\n\r", IS_NPC(victim) ? 100 : ch->pcdata->ego );
    ch_printf( ch, "Fighting: %s    Master: %s    Leader: %s\n\r",
	victim->fighting ? victim->fighting->who->name : "(none)",
        victim->master      ? victim->master->name   : "(none)",
        victim->leader      ? victim->leader->name   : "(none)" );
    if ( !IS_NPC(victim) )
	ch_printf( ch,
	    "Thirst: %d   Full: %d   Drunk: %d     Favor: %d    Glory: %d/%d\n\r",
	    victim->pcdata->condition[COND_THIRST],
	    victim->pcdata->condition[COND_FULL],
	    victim->pcdata->condition[COND_DRUNK],
	    victim->pcdata->favor,
	    victim->pcdata->quest_curr,
	    victim->pcdata->quest_accum );
    else
	ch_printf( ch, "Hit dice: %dd%d+%d.  Damage dice: %dd%d+%d.\n\r",
		victim->pIndexData->hitnodice,
		victim->pIndexData->hitsizedice,
		victim->pIndexData->hitplus,
		victim->pIndexData->damnodice,
		victim->pIndexData->damsizedice,
		victim->pIndexData->damplus );
    ch_printf( ch, "MentalState: %d   EmotionalState: %d\n\r",
 		victim->mental_state, victim->emotional_state );
    ch_printf( ch, "Saving throws: %d %d %d %d %d.\n\r",
		victim->saving_poison_death,
		victim->saving_wand,
		victim->saving_para_petri,
		victim->saving_breath,
		victim->saving_spell_staff  );
    ch_printf( ch, "Carry figures: items (%d/%d)  weight (%d/%d)   Numattacks: %d\n\r",
	victim->carry_number, can_carry_n(victim), victim->carry_weight, can_carry_w(victim), victim->numattacks );
    ch_printf( ch, "Years: %d   Seconds Played: %d   Timer: %d   Act: %d\n\r",
	get_age( victim ), IS_NPC(victim) ? 0 : (int) victim->pcdata->played, victim->timer, victim->act );

    if ( IS_NPC( victim ) )
    {
	ch_printf( ch, "(1)Act flags: %s\n\r", flag_string(victim->act,act_flags) );
	ch_printf( ch, "(2)Act flags: %s\n\r",flag_string(victim->acttwo,acttwo_flags));
	ch_printf( ch, "SMART-Flags:  %s\n\r",flag_string(victim->smart,smart_flags));
    }
    else
    { 
	ch_printf( ch, "(1)Player flags: %s\n\r",
		flag_string(victim->act, plr_flags) );
	ch_printf( ch, "(2)Player flags: %s\n\r",
		flag_string(victim->pcdata->flagstwo, morepc_flags) );
	ch_printf( ch, "Pcflags: %s\n\r",
		flag_string(victim->pcdata->flags, pc_flags) );
    }
    ch_printf( ch, "Affected by: %s\n\r",
	affect_bit_name( &victim->affected_by ) );
    ch_printf( ch, "Speaks: %d   Speaking: %d\n\r",
    	victim->speaks, victim->speaking );
    send_to_char( "Languages: ", ch );
    for ( x = 0; lang_array[x] != LANG_UNKNOWN; x++ )
    	if ( knows_language( victim, lang_array[x], victim )
    	||  (IS_NPC(victim) && victim->speaks == 0) )
    	{
		if ( IS_SET(lang_array[x], victim->speaking)
		||  (IS_NPC(victim) && !victim->speaking) )
    		   set_char_color( AT_RED, ch );
 		send_to_char( lang_names[x], ch );
 		send_to_char( " ", ch );
 		set_char_color( AT_PLAIN, ch );
 	}
 	else
 	if ( IS_SET(lang_array[x], victim->speaking)
 	||  (IS_NPC(victim) && !victim->speaking) )
 	{
 		set_char_color( AT_PINK, ch );
 		send_to_char( lang_names[x], ch );
 		send_to_char( " ", ch );
 		set_char_color( AT_PLAIN, ch );
 	}
    send_to_char( "\n\r", ch );
    if ( victim->pcdata && victim->pcdata->bestowments 
         && victim->pcdata->bestowments[0] != '\0' )
      ch_printf( ch, "Bestowments: %s\n\r", victim->pcdata->bestowments );
    ch_printf( ch, "Short description: %s\n\rLong  description: %s",
	victim->short_descr,
	victim->long_descr[0] != '\0' ? victim->long_descr : "(none)\n\r" );
    if ( IS_NPC(victim) && victim->spec_fun )
	ch_printf( ch, "Mobile has spec fun: %s\n\r",
		lookup_spec( victim->spec_fun ) );
    ch_printf( ch, "Body Parts : %s\n\r",
	flag_string(victim->xflags, part_flags) );
    ch_printf( ch, "Resistant  : %s\n\r",
	flag_string(victim->resistant, ris_flags) );
    ch_printf( ch, "Immune     : %s\n\r",
	flag_string(victim->immune, ris_flags) );
    ch_printf( ch, "Susceptible: %s\n\r",
	flag_string(victim->susceptible, ris_flags) );
    ch_printf( ch, "Attacks    : %s\n\r",
	flag_string(victim->attacks, attack_flags) );
    ch_printf( ch, "Defenses   : %s\n\r",
	flag_string(victim->defenses, defense_flags) );
    for ( paf = victim->first_affect; paf; paf = paf->next )
	if ( (skill=get_skilltype(paf->type)) != NULL )
	  ch_printf( ch,
	    "%s: '%s' modifies %s by %d for %d rounds with bits %s.\n\r",
	    skill_tname[skill->type],
	    skill->name,
	    affect_loc_name( paf->location ),
	    paf->modifier,
	    paf->duration,
	    affect_bit_name( &paf->bitvector )
	    );
    return;
}



void do_mfind( CHAR_DATA *ch, char *argument )
{
/*  extern int top_mob_index; */
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
/*  int vnum; */
    int hash;
    int nMatch;
    bool fAll;
    ZONE_DATA *in_zone;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Mfind whom?\n\r", ch );
	return;
    }

    fAll	= !str_cmp( arg, "all" );
    nMatch	= 0;
    set_pager_color( AT_PLAIN, ch );

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_mob_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
/*  for ( vnum = 0; nMatch < top_mob_index; vnum++ )
    {
	if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
	{
	    if ( fAll || is_name( arg, pMobIndex->player_name ) )
	    {
		nMatch++;
		sprintf( buf, "[%5d] %s\n\r",
		    pMobIndex->vnum, capitalize( pMobIndex->short_descr ) );
		send_to_char( buf, ch );
	    }
	}
    }
     */

    /*
     * This goes through all the hash entry points (1024), and is therefore
     * much faster, though you won't get your vnums in order... oh well. :)
     *
     * Tests show that Furey's method will usually loop 32,000 times, calling
     * get_mob_index()... which loops itself, an average of 1-2 times...
     * So theoretically, the above routine may loop well over 40,000 times,
     * and my routine bellow will loop for as many index_mobiles are on
     * your mud... likely under 3000 times.
     * -Thoric
     */
for( in_zone = first_zone; in_zone; in_zone = in_zone->next )
{
    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
	for ( pMobIndex = in_zone->mob_index_hash[hash];
	      pMobIndex;
	      pMobIndex = pMobIndex->next )
	    if ( fAll || nifty_is_name( arg, pMobIndex->player_name ) )
	    {
		nMatch++;
		pager_printf( ch, "[%d] [%5d] %s\n\r",in_zone->number,
		    pMobIndex->vnum, capitalize( pMobIndex->short_descr ) );
	    }
}
    if ( nMatch )
	pager_printf( ch, "Number of matches: %d\n", nMatch );
    else
	send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );

    return;
}



void do_ofind( CHAR_DATA *ch, char *argument )
{
/*  extern int top_obj_index; */
    char arg[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
/*  int vnum; */
    int hash;
    int nMatch;
    bool fAll;
    ZONE_DATA *in_zone;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Ofind what?\n\r", ch );
	return;
    }

    set_pager_color( AT_PLAIN, ch );
    fAll	= !str_cmp( arg, "all" );
    nMatch	= 0;
/*  nLoop	= 0; */

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_obj_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
    for ( vnum = 0; nMatch < top_obj_index; vnum++ )
    {
	nLoop++;
	if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
	{
	    if ( fAll || nifty_is_name( arg, pObjIndex->name ) )
	    {
		nMatch++;
		sprintf( buf, "[%5d] %s\n\r",
		    pObjIndex->vnum, capitalize( pObjIndex->short_descr ) );
		send_to_char( buf, ch );
	    }
	}
    }
     */

    /*
     * This goes through all the hash entry points (1024), and is therefore
     * much faster, though you won't get your vnums in order... oh well. :)
     *
     * Tests show that Furey's method will usually loop 32,000 times, calling
     * get_obj_index()... which loops itself, an average of 2-3 times...
     * So theoretically, the above routine may loop well over 50,000 times,
     * and my routine bellow will loop for as many index_objects are on
     * your mud... likely under 3000 times.
     * -Thoric
     */
for( in_zone = first_zone; in_zone; in_zone = in_zone->next )
{
    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
	for ( pObjIndex = in_zone->obj_index_hash[hash];
	      pObjIndex;
	      pObjIndex = pObjIndex->next )
	    if ( fAll || nifty_is_name( arg, pObjIndex->name ) )
	    {
		nMatch++;
		pager_printf( ch, "[%d] [%5d] %s\n\r",in_zone->number,
		    pObjIndex->vnum, capitalize( pObjIndex->short_descr ) );
	    }
}
    if ( nMatch )
	pager_printf( ch, "Number of matches: %d\n", nMatch );
    else
	send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );

    return;
}



void do_mwhere( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    bool found;
    ZONE_DATA *in_zone;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Mwhere whom?\n\r", ch );
	return;
    }

    set_pager_color( AT_PLAIN, ch );
    found = FALSE;
    for ( victim = first_char; victim; victim = victim->next )
    {
	if ( IS_NPC(victim)
	&&   victim->in_room
	&&   nifty_is_name( arg, victim->name ) )
	{
	    found = TRUE;
	    pager_printf( ch, "[%d][%5d] %-28s [%5d] %s\n\r",
		victim->in_room->area->zone->number,
		victim->pIndexData->vnum,
		victim->short_descr,
		victim->in_room->vnum,
		victim->in_room->name );
	}
    }

for( in_zone = first_zone; in_zone; in_zone = in_zone->next )
{
    for ( victim = in_zone->first_mob; victim; victim = victim->next )
    {
	if ( IS_NPC(victim)
	&&   victim->in_room
	&&   nifty_is_name( arg, victim->name ) )
	{
	    found = TRUE;
	    pager_printf( ch, "[%d][%5d] %-28s [%5d] %s\n\r",
		victim->in_room->area->zone->number,
		victim->pIndexData->vnum,
		victim->short_descr,
		victim->in_room->vnum,
		victim->in_room->name );
	}
    }
}

    if ( !found )
	act( AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );

    return;
}

void do_chaos( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
     if( chaos == 0 )
     send_to_char("Chaos Currently set to OFF.\n\r",ch);
     else
     send_to_char("*** WARNING: CHAOS IS ON !!!!\n\r",ch);
     send_to_char("\n\r\n\rSyntax: chaos <on/off>\n\r", ch );
     return;
     }
   if ( !str_cmp( arg, "off") )
   {
     chaos--;
     send_to_char("Chaos is OFF\n\r", ch);
     sprintf(buf1,"****** WARNING: CHAOS IS ** OFF ** ");
     echo_to_all( AT_BLOOD, buf1, ECHOTAR_ALL ); 
    return;
    }

    if (!str_cmp( arg, "on" ) )
    {
    chaos++;
    send_to_char("******* WARNING: CHAOS IS ON!!!!!!\n\r", ch );
    send_to_char("NOTE: DONOT DONOT DONOT LEAVE THIS COMMAND ON\n\r", ch );
    send_to_char("      WHEN YOU LEAVE UNLESS YOU HAVE ARRANGED\n\r", ch );
    send_to_char("      WITH ANOTHER GOD TO HAVE IT TURNED OFF!\n\r", ch );
    send_to_char("      I WILL NOT HESITATE TO DEMOTE ANYONE WHO\n\r", ch );
    send_to_char("      LEAVES IT IN THE ON STATE AND LOGS OFF!\n\r\n\r", ch );
    send_to_char("      GREYWOLF!\n\r\n\r", ch);
    send_to_char("**REFERENCE: THIS COMMAND ENABLES FREE PKILL FOR ALL!\n\r", ch);

sprintf(buf1,"****************************************************\n\r");
sprintf(buf2,"***** WARNING: CHAOS IS ON !!!! FREE PKILL!!!! *****\n\r");
sprintf(buf3,"****************************************************\n\r");
     echo_to_all( AT_BLOOD, buf1, ECHOTAR_ALL ); 
     echo_to_all( AT_BLOOD, buf2, ECHOTAR_ALL ); 
     echo_to_all( AT_BLOOD, buf3, ECHOTAR_ALL ); 
     return;
    }
}

void do_turbospeed( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
     send_to_char("\n\r\n\rSyntax: turbospeed <5-20>\n\r", ch );
     return;
     }

   if ( atoi(arg) < 5 || atoi(arg) > 20 )
   {
     send_to_char("\n\r\n\rSyntax: turbospeed <5-20>\n\r", ch );
     return;
   }

   if ( TURBO_FIGHT == TRUE )
   {
     TURBO_FIGHT = FALSE;
     TURBO_NUM = 4;
     send_to_char("TURBOSpeed is OFF\n\r", ch);
     sprintf(buf1,"****** TURBOSpeed is ** OFF ** ");
     echo_to_all( AT_BLOOD, buf1, ECHOTAR_ALL ); 
    return;
    }

    if ( TURBO_FIGHT == FALSE )
    {
    TURBO_FIGHT = TRUE;
    TURBO_NUM = atoi(arg);
    send_to_char("******* WARNING: TURBOSpeed IS ON!!!!!!\n\r", ch );
    send_to_char("NOTE: DONOT DONOT DONOT LEAVE THIS COMMAND ON\n\r", ch );
    send_to_char("      WHEN YOU LEAVE UNLESS YOU HAVE ARRANGED\n\r", ch );
    send_to_char("      WITH ANOTHER GOD TO HAVE IT TURNED OFF!\n\r", ch );
    send_to_char("      I WILL NOT HESITATE TO DEMOTE ANYONE WHO\n\r", ch );
    send_to_char("      LEAVES IT IN THE ON STATE AND LOGS OFF!\n\r\n\r", ch );
    send_to_char("      GREYWOLF!\n\r\n\r", ch);
    send_to_char("**REFERENCE: THIS COMMAND ENABLES FREE PKILL FOR ALL!\n\r", ch);

sprintf(buf1,"****************************************************\n\r");
sprintf(buf2,"***** TURBO FIGHTING SPEED IS ON!!!! Wheeee!!! *****\n\r");
sprintf(buf3,"****************************************************\n\r");
     echo_to_all( AT_BLOOD, buf1, ECHOTAR_ALL ); 
     echo_to_all( AT_BLOOD, buf2, ECHOTAR_ALL ); 
     echo_to_all( AT_BLOOD, buf3, ECHOTAR_ALL ); 
     return;
    }
}

void do_doubleexp( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
     if( doubleexp == 0 )
     send_to_char("DoubleEXP Currently set to OFF.\n\r",ch);
     else
     send_to_char("*** WARNING: EXP x 2 IS ON !!!!\n\r",ch);
     send_to_char("\n\r\n\rSyntax: DoubleEXP <on/off>\n\r", ch );
     return;
     }
   if ( !str_cmp( arg, "off") )
   {
     doubleexp--;
     send_to_char("DoubleEXP is OFF\n\r", ch);
     sprintf(buf1,"****** BOO!! EXP x 2 IS ** OFF ** ");
     echo_to_all( AT_BLOOD, buf1, ECHOTAR_ALL ); 
    return;
    }

    if (!str_cmp( arg, "on" ) )
    {
    doubleexp++;
    send_to_char("******* WARNING: DOUBLEEXP IS ON!!!!!!\n\r", ch );
    send_to_char("NOTE: DONOT DONOT DONOT LEAVE THIS COMMAND ON\n\r", ch );
    send_to_char("      WHEN YOU LEAVE UNLESS YOU HAVE ARRANGED\n\r", ch );
    send_to_char("      WITH ANOTHER GOD TO HAVE IT TURNED OFF!\n\r", ch );
    send_to_char("      I WILL NOT HESITATE TO DEMOTE ANYONE WHO\n\r", ch );
    send_to_char("      LEAVES IT IN THE ON STATE AND LOGS OFF!\n\r\n\r", ch );
    send_to_char("      GREYWOLF!\n\r\n\r", ch);
    send_to_char("**REFERENCE: THIS COMMAND ENABLES EXP x 2 FOR ALL!\n\r", ch);

sprintf(buf1,"****************************************************\n\r");
sprintf(buf2,"***** YAHOO EXP x 2 IS ON !!!! DOUBLE EXP!!!!! *****\n\r");
sprintf(buf3,"****************************************************\n\r");
     echo_to_all( AT_BLOOD, buf1, ECHOTAR_ALL ); 
     echo_to_all( AT_BLOOD, buf2, ECHOTAR_ALL ); 
     echo_to_all( AT_BLOOD, buf3, ECHOTAR_ALL ); 
     return;
    }
}

void do_arena( CHAR_DATA *ch, char *argument )
{
 extern bool fight_in_progress;
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
     if( fight_in_progress)
     send_to_char("Arena set [BUSY]\n\r",ch);
     else
     send_to_char("Arena set [CLEARED]\n\r",ch);
     send_to_char("\n\r\n\rSyntax: arena <cleared/busy>\n\r", ch );
     return;
     }
   if ( !str_cmp( arg, "cleared") )
   {
    fight_in_progress = FALSE;
    send_to_char("Arena now set to [CLEARED]\n\r", ch);
     return;
    }

    if (!str_cmp( arg, "busy" ) )
    {
    fight_in_progress = TRUE;
    send_to_char("Arena now set to [BUSY]\n\r", ch );
     return;
    }
}

void do_bodybag( CHAR_DATA *ch, char *argument )
{
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    ZONE_DATA *in_zone;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Bodybag whom?\n\r", ch );
	return;
    }

    /* make sure the buf3 is clear? */
    sprintf(buf3, " ");
    /* check to see if vict is playing? */
    sprintf(buf2,"the corpse of %s",arg); 
    found = FALSE;

for( in_zone = first_zone; in_zone; in_zone = in_zone->next )
{
    for ( obj = in_zone->first_obj; obj; obj = obj->next )
    {
	if ( obj->in_room 
        && !str_cmp( buf2, obj->short_descr ) 
        && (obj->pIndexData->vnum == 11 )
	&& (obj->in_room->level <= get_trust(ch)) )
	{
	    found = TRUE;
	    ch_printf( ch, "Bagging body: [%5d] %-28s [%5d] %s\n\r",
		obj->pIndexData->vnum,
		obj->short_descr,
		obj->in_room->vnum,
		obj->in_room->name );
            obj_from_room(obj); 
            obj_to_char(obj, ch);
	    obj->timer = -1;
            save_char_obj( ch );
	}
    }
}
    if ( !found )
	ch_printf(ch," You couldn't find any %s\n\r",buf2);
    return;
}


/* New owhere by Altrag, 03/14/96 */
void do_owhere( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    ZONE_DATA *in_zone;
    bool found;
    int icnt = 0;

    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Owhere what?\n\r", ch );
	return;
    }
    argument = one_argument(argument, arg1);
    
    set_pager_color( AT_PLAIN, ch );
    if ( arg1[0] != '\0' && !str_prefix(arg1, "nesthunt") )
    {
      if ( !(obj = get_obj_world(ch, arg)) )
      {
        send_to_char( "Nesthunt for what object?\n\r", ch );
        return;
      }
      for ( ; obj->in_obj; obj = obj->in_obj )
      {
	pager_printf(ch, "[%5d] %-28s in object [%5d] %s\n\r",
                obj->pIndexData->vnum, obj_short(obj),
                obj->in_obj->pIndexData->vnum, obj->in_obj->short_descr);
	++icnt;
      }
      sprintf(buf, "[%5d] %-28s in ", obj->pIndexData->vnum,
		obj_short(obj));
      if ( obj->carried_by )
        sprintf(buf+strlen(buf), "invent [%5d] %s\n\r",
                (IS_NPC(obj->carried_by) ? obj->carried_by->pIndexData->vnum
                : 0), PERS(obj->carried_by, ch));
      else if ( obj->in_room )
        sprintf(buf+strlen(buf), "room   [%5d] %s\n\r",
                obj->in_room->vnum, obj->in_room->name);
      else if ( obj->in_obj )
      {
        bug("do_owhere: obj->in_obj after NULL!",0);
        strcat(buf, "object??\n\r");
      }
      else
      {
//        bug("do_owhere: object doesnt have location!",0);
        strcat(buf, "nowhere??\n\r");
      }
      send_to_pager(buf, ch);
      ++icnt;
      pager_printf(ch, "Nested %d levels deep.\n\r", icnt);
      return;
    }

    found = FALSE;
for( in_zone = first_zone; in_zone; in_zone = in_zone->next )
{
    for ( obj = in_zone->first_obj; obj; obj = obj->next )
    {
        if ( !nifty_is_name( arg, obj->name ) )
            continue;
        found = TRUE;
        
        sprintf(buf, "(%3d) [%5d] %-28s in ", ++icnt, obj->pIndexData->vnum,
                obj_short(obj));
        if ( obj->carried_by )
          sprintf(buf+strlen(buf), "invent [%5d] %s\n\r",
                  (IS_NPC(obj->carried_by) ? obj->carried_by->pIndexData->vnum
                  : 0), PERS(obj->carried_by, ch));
        else if ( obj->in_room )
          sprintf(buf+strlen(buf), "room   [%5d] %s\n\r",
                  obj->in_room->vnum, obj->in_room->name);
        else if ( obj->in_obj )
          sprintf(buf+strlen(buf), "object [%5d] %s\n\r",
                  obj->in_obj->pIndexData->vnum, obj_short(obj->in_obj));
        else
        {
//          bug("do_owhere: object doesnt have location!",0);
          strcat(buf, "nowhere??\n\r");
        }
        send_to_pager(buf, ch);
    }
}
    if ( !found )
      act( AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
    else
      pager_printf(ch, "%d matches.\n\r", icnt);

    return;
}


void do_reboo( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to REBOOT, spell it out.\n\r", ch );
    return;
}



void do_reboot( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    extern bool mud_down;
    CHAR_DATA *vch;

    if ( str_cmp( argument, "mud now" )
    &&   str_cmp( argument, "nosave" )
    &&   str_cmp( argument, "and sort skill table" ) )
    {
	send_to_char( "Syntax: 'reboot mud now' or 'reboot nosave'\n\r", ch );
	return;
    }

    if ( auction->item )
	do_auction( ch, "stop");

    sprintf( buf, "Reboot by %s.", ch->name );
    do_echo( ch, buf );

    if ( !str_cmp(argument, "and sort skill table") )
    {
	sort_skill_table();
	save_skill_table();
    }

    /* Save all characters before booting. */
    if ( str_cmp(argument, "nosave") )
	for ( vch = first_char; vch; vch = vch->next )
	    if ( !IS_NPC( vch ) )
		save_char_obj( vch );

    mud_down = TRUE;
    return;
}



void do_shutdow( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to SHUTDOWN, spell it out.\n\r", ch );
    return;
}



void do_shutdown( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    extern bool mud_down;
    CHAR_DATA *vch;

    if ( str_cmp( argument, "mud now" ) && str_cmp(argument, "nosave") )
    {
	send_to_char( "Syntax: 'shutdown mud now' or 'shutdown nosave'\n\r", ch );
	return;
    }

    if ( auction->item )
	do_auction( ch, "stop");

    sprintf( buf, "Shutdown by %s.", ch->name );
    append_file( ch, SHUTDOWN_FILE, buf );
    strcat( buf, "\n\r" );
    do_echo( ch, buf );

    /* Save all characters before booting. */
    if ( str_cmp(argument, "nosave") )
	for ( vch = first_char; vch; vch = vch->next )
	    if ( !IS_NPC( vch ) )
		save_char_obj( vch );
    mud_down = TRUE;
    return;
}


void do_snoop( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Snoop whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !victim->desc )
    {
	send_to_char( "No descriptor to snoop.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Cancelling all snoops.\n\r", ch );
	for ( d = first_descriptor; d; d = d->next )
	    if ( d->snoop_by == ch->desc )
		d->snoop_by = NULL;
	return;
    }

    if ( victim->desc->snoop_by )
    {
	send_to_char( "Busy already.\n\r", ch );
	return;
    }

    /*
     * Minimum snoop level... a secret mset value
     * makes the snooper think that the victim is already being snooped
     */
    if ( get_trust( victim ) >= get_trust( ch )
    ||  (victim->pcdata && victim->pcdata->min_snoop > get_trust( ch )) )
    {
	send_to_char( "Busy already.\n\r", ch );
	return;
    }

    if ( ch->desc )
    {
	for ( d = ch->desc->snoop_by; d; d = d->snoop_by )
	    if ( d->character == victim || d->original == victim )
	    {
		send_to_char( "No snoop loops.\n\r", ch );
		return;
	    }
    }

/*  Snoop notification for higher imms, if desired, uncomment this
    if ( get_trust(victim) > LEVEL_GOD && get_trust(ch) < LEVEL_SUPREME )
      write_to_descriptor( victim->desc->descriptor, "\n\rYou feel like someone is watching your every move...\n\r", 0 );
*/
    victim->desc->snoop_by = ch->desc;
    send_to_char( "Ok.\n\r", ch );
    return;
}

 

void do_switch( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    send_to_char("This command has yet to be coded.\n\r",ch);
    return;

    if ( arg[0] == '\0' )
    {
        send_to_char( "Switch into whom?\n\r", ch );
        return;
    }

    if ( !ch->desc )
        return;

    if ( ch->desc->original )
    {
        send_to_char( "You are already switched.\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

 
    if ( victim == ch )
    {
        send_to_char( "Ok.\n\r", ch );
        return;
    }

    if ( victim->desc )
    {
        send_to_char( "Character in use.\n\r", ch );
        return;
    }

    if ( !IS_NPC(victim) && ch->level < LEVEL_HEAD_GOD )
    {
        send_to_char( "You cannot switch into a player!\n\r", ch );
        return;
    }

    ch->desc->character = victim;
    ch->desc->original  = ch;
    victim->desc        = ch->desc;
    ch->desc            = NULL;
    ch->switched        = victim;
    send_to_char( "Ok.\n\r", victim );
    return;
}

void do_return( CHAR_DATA *ch, char *argument )
{
    if ( !ch->desc )
        return;

    send_to_char("This command has been disabled.\n\r",ch);
    return;

    if ( !ch->desc->original )
    {
        send_to_char( "You aren't switched.\n\r", ch );
        return;
    }
    if (IS_SET(ch->act, ACT_POLYMORPHED))
    {
      send_to_char("Use revert to return from a polymorphed mob.\n\r",
ch);
      return;
    }

    send_to_char( "You return to your original body.\n\r", ch );
        if ( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_POSSESS ) )
        {
                affect_strip( ch, gsn_possess );
                xREMOVE_BIT( ch->affected_by, AFF_POSSESS );
        }
/*    if ( IS_NPC( ch->desc->character ) )
      xREMOVE_BIT( ch->desc->character->affected_by, AFF_POSSESS );*/
    ch->desc->character       = ch->desc->original;
    ch->desc->original        = NULL;
    ch->desc->character->desc = ch->desc;
    ch->desc->character->switched = NULL;
    ch->desc                  = NULL;
    return;
}
void do_delete( CHAR_DATA *ch, char *argument)
{
   char strsave[MAX_INPUT_LENGTH];
   char strsave2[MAX_INPUT_LENGTH];
   char strsave3[MAX_INPUT_LENGTH];
   char strsave4[MAX_INPUT_LENGTH];
   char strsave5[MAX_INPUT_LENGTH];
   char strsave6[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   char *name;

   if (IS_NPC(ch))
        return;

   if (IS_IMMORTAL(ch))
   {
    send_to_char("Only a Council or Higher can Delete Immortals\n\r",ch);
    return;
   }

    if (argument[0] == '\0')
    {
    send_to_char("Syntax: delete <your password>\n\r\n\r",ch);
    send_to_char("WARNING: this command is irreversible.\n\r",ch);
    send_to_char("WARNING: NO REIMBERSALS IF YOU DO THIS!!!.\n\r",ch);
    sprintf(buf,"%s is contemplating deletion.",capitalize(ch->name));
    log_string(buf);
    return;
    }

        if ( !strcmp(crypt( argument, ch->pcdata->pwd ),ch->pcdata->pwd) )
        {
	    sprintf(buf,"Character '%s' deleted.\n\r",capitalize(ch->name));
	    send_to_char(buf,ch);
            send_to_char( "Return again someday lost traveller.\n\r\n\r",ch);
	    stop_fighting(ch,TRUE);
            sprintf( strsave,  "%s%c/%s.gz", PLAYER_DIR,tolower(ch->name[0]),capitalize( ch->name ) );
            sprintf( strsave2, "%s%c/%s", PLAYER_DIR,tolower(ch->name[0]),capitalize( ch->name ) );
	    sprintf( strsave3, "%s%c/%s",LOCKER_DIR,tolower(ch->name[0]),capitalize(ch->name));
	    sprintf( strsave4, "%s%c/%s.F",FINGER_DIR,tolower(ch->name[0]),capitalize(ch->name));
	    sprintf( strsave5, "%s%c/%s",CORPSE_DIR,tolower(ch->name[0]),capitalize(ch->name));
	    sprintf( strsave6, "%s%c/%s",PLAYER_OBJ_DIR,tolower(ch->name[0]),capitalize(ch->name));
	    name = ch->name;
            do_quit( ch,"");
	    ch=NULL;

	    if ( (ch=get_char(name))!=NULL )
		return;

            sprintf(buf,"%s has commited suicide!! =(", capitalize(name));
            log_string(buf);
	    unlink(strsave);
	    unlink(strsave2);
	    unlink(strsave3);
	    unlink(strsave4);
	    unlink(strsave5);
	    unlink(strsave6);
            return;
	}

send_to_char("Password Incorrect.\n\r",ch);
return;
}

void do_minvoke( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *victim;
    ZONE_DATA *in_zone;
    sh_int vnum;
    sh_int zone;
       
argument = one_argument( argument, arg );
argument = one_argument( argument, arg2 );    

    if ( arg[0] == '\0' )
    {
	send_to_char( "Syntax: minvoke <vnum/name> <zone>.\n\r", ch );
	return;
    }
     
    if ( !is_number( arg2 ) )
    {
	send_to_char("Syntax: minvoke <vnum/name> <zone #>\n\r",ch);
        return;
    }

        zone = atoi( arg2 );
    
    if ( zone <= 0 || ( (in_zone=find_zone(zone))==NULL)  )
    {
      send_to_char("No such Zone Exsists.\n\r",ch);
      return;
    }

    if ( !is_number( arg ) )
    {
	int  hash, cnt;
	int  count = number_argument( arg, arg2 );

	vnum = -1;
	for ( hash = cnt = 0; hash < MAX_KEY_HASH; hash++ )
	    for ( pMobIndex = in_zone->mob_index_hash[hash];
		  pMobIndex;
		  pMobIndex = pMobIndex->next )
	    if ( nifty_is_name( arg, pMobIndex->player_name ) )
	    {
		vnum = pMobIndex->vnum;
		count++;
		break;
	    }

	if ( vnum == -1 )
	{
	    send_to_char( "No such mobile exists.\n\r", ch );
	    return;
	}
    }
    else
	vnum = atoi( arg );

    if ( get_trust(ch) < LEVEL_GOD )
    {
	AREA_DATA *pArea;

	if ( IS_NPC(ch) )
	{
	  send_to_char( "Huh?\n\r", ch );
	  return;
	}

	if ( !ch->pcdata || !(pArea=ch->pcdata->area) )
	{
	  send_to_char( "You must have an assigned area to invoke this mobile.\n\r", ch );
	  return;
	}
	if ( vnum < pArea->low_m_vnum
	&&   vnum > pArea->hi_m_vnum )
	{
	  send_to_char( "That number is not in your allocated range.\n\r", ch );
	  return;
	}
    }

    if ( ( pMobIndex = get_mob_index( vnum, zone ) ) == NULL )
    {
	send_to_char( "No mobile has that vnum.\n\r", ch );
	return;
    }

/* Commented out by Narn, not sure what it was for.  Anyway, it's a 
   pain for those who use instazone. 
    if ( IS_SET( pMobIndex->act, ACT_PROTOTYPE )
    &&	 pMobIndex->count > 5 )
    {
	send_to_char( "That mobile is at its limit.\n\r", ch );
	return;
    }
*/

    if ( ch->in_room->area && ch->in_room->area->zone )
    in_zone = find_zone(ch->in_room->area->zone->number);
    else
    in_zone = find_zone(1);

    victim = create_mobile( pMobIndex, in_zone );
    char_to_room( victim, ch->in_room );
    act( AT_IMMORT, "$n has created $N!", ch, NULL, victim, TO_ROOM );
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_oinvoke( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    ZONE_DATA *in_zone;
    sh_int vnum;
    int zone;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Syntax: oinvoke <vnum/name> <zone>.\n\r", ch );
	return;
    }

    if ( arg2[0] == '\0' )
    {
	send_to_char( "Syntax: oinvoke <vnum/name> <zone>.\n\r", ch );
	return;
    }
    else
    {
	if ( !is_number( arg2 ) )
	{
	    send_to_char( "Syntax: oinvoke <vnum/name> <zone>.\n\r", ch);
	    return;
	}
    }

	zone = atoi( arg2 );

    if ( zone <= 0 || ( (in_zone=find_zone(zone))==NULL)  )
    {
      send_to_char("No such Zone Exsists.\n\r",ch);
      return;
    }

    if ( !is_number( arg1 ) )
    {
	char arg[MAX_INPUT_LENGTH];
	int  hash, cnt;
	int  count = number_argument( arg1, arg );


	vnum = -1;
	for ( hash = cnt = 0; hash < MAX_KEY_HASH; hash++ )
	    for ( pObjIndex = in_zone->obj_index_hash[hash];
		  pObjIndex;
		  pObjIndex = pObjIndex->next )
            if ( nifty_is_name( arg1, pObjIndex->name ) )
            {
                vnum = pObjIndex->vnum;
		count++;
                break;
            }

	if ( vnum == -1 )
	{
	    send_to_char( "No such object exists.\n\r", ch );
	    return;
	}
    }
    else
	vnum = atoi( arg1 );

    if ( get_trust(ch) < LEVEL_GOD )
    {
	AREA_DATA *pArea;

	if ( IS_NPC(ch) )
	{
	  send_to_char( "Huh?\n\r", ch );
	  return;
	}
	
	if ( !ch->pcdata || !(pArea=ch->pcdata->area) )
	{
	  send_to_char( "You must have an assigned area to invoke this object.\n\r", ch );
	  return;
	}
	if ( vnum < pArea->low_o_vnum
	&&   vnum > pArea->hi_o_vnum )
	{
	  send_to_char( "That number is not in your allocated range.\n\r", ch );
	  return;
	}
    }

    if ( ( pObjIndex = get_obj_index( vnum,zone ) ) == NULL )
    {
	send_to_char( "No object has that vnum.\n\r", ch );
	return;
    }

/* Commented out by Narn, it seems outdated
    if ( IS_OBJ_STAT( pObjIndex, ITEM_PROTOTYPE )
    &&	 pObjIndex->count > 5 )
    {
	send_to_char( "That object is at its limit.\n\r", ch );
	return;
    }
*/
    in_zone = find_zone(ch->in_room->area->zone->number);
    load_limited_toggle(FALSE);
    obj = create_object( pObjIndex, (int)ch->level, in_zone );

    if ( !IS_SET( obj->extra_flags, ITEM_PROTOTYPE ) &&
          IS_SET( obj->second_flags, ITEM2_LIMITED ) &&
          ch->level < 58 )
    {
    send_to_char( "Only a Council+ may load Limited Eq.\n\r",ch);
    extract_obj(obj);
    load_limited_toggle(TRUE);
    return;
    }

    if ( !IS_SET( obj->extra_flags, ITEM_PROTOTYPE ) &&
          IS_SET( obj->second_flags, ITEM2_LIMITED ) &&
          obj->pIndexData->loaded > obj->pIndexData->limit )
    {
	if ( ch->level < 60 )
	{
          send_to_char( "That Item is overlimit. Please Talk to a Hyp.\n\r",ch);
          extract_obj(obj);
          load_limited_toggle(TRUE);
          return;
	}
	if ( ch->level >= 60 )
	  send_to_char("&RWARNING: ITEM IS OVER LIMIT! EXTREME CIRCUMSTANCES ONLY!\n\r",ch);
    }

    /* Load Sucessful, adjust the limit --GW */
    load_limited_toggle(TRUE);
    adjust_limits( pObjIndex, 1, LOADED_ADJUST_UP );

    update_obj_load(obj,ch);

    act( AT_MAGIC, "$n makes a strange magical geasture.",ch,NULL,NULL,TO_ROOM );
 
    if ( CAN_WEAR(obj, ITEM_TAKE) )
    {
	obj_to_char( obj, ch );
	act( AT_IMMORT, "$n has created $p!", ch, obj, NULL, TO_ROOM );
    }
    else
    {
	obj_to_room( obj, ch->in_room );
	act( AT_IMMORT, "$n has created $p!", ch, obj, NULL, TO_ROOM );
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_purge( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( ch->substate > SUB_NONE )
    {
	send_to_char("Doing that now would crash the game!\n\r",ch);
	return;
    }

    if ( arg[0] == '\0' )
    {
	/* 'purge' */
	CHAR_DATA *vnext;
	OBJ_DATA  *obj_next;

	for ( victim = ch->in_room->first_person; victim; victim = vnext )
	{
	    vnext = victim->next_in_room;
	    if ( IS_NPC(victim) && victim != ch && !IS_SET(victim->act, ACT_POLYMORPHED))
		extract_char( victim, TRUE );
	}

	for ( obj = ch->in_room->first_content; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    extract_obj( obj );
	}

	act( AT_IMMORT, "$n purges the room!", ch, NULL, NULL, TO_ROOM);
	send_to_char( "Ok.\n\r", ch );
	return;
    }
    victim = NULL; obj = NULL;

    /* fixed to get things in room first -- i.e., purge portal (obj),
     * no more purging mobs with that keyword in another room first
     * -- Tri */
    if ( ( victim = get_char_room( ch, arg ) ) == NULL 
    && ( obj = get_obj_here( ch, arg ) ) == NULL )
    {
      if ( ( victim = get_char_world( ch, arg ) ) == NULL
      &&   ( obj = get_obj_world( ch, arg ) ) == NULL )  /* no get_obj_room */
      {
	send_to_char( "They aren't here.\n\r", ch );
	return;
      }
    }

/* Single object purge in room for high level purge - Scryn 8/12*/
    if ( obj )
    {
	separate_obj( obj );
	act( AT_IMMORT, "$n purges $p.", ch, obj, NULL, TO_ROOM);
	act( AT_IMMORT, "You make $p disappear in a puff of smoke!", ch, obj, NULL, TO_CHAR);          
	extract_obj( obj );
	return;
    }


    if ( !IS_NPC(victim) )
    {
	send_to_char( "Not on PC's.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
    	send_to_char( "You cannot purge yourself!\n\r", ch );
    	return;
    }

    if (IS_SET(victim->act, ACT_POLYMORPHED))
    {
      send_to_char("You cannot purge a polymorphed player.\n\r", ch);
      return;
    }
    act( AT_IMMORT, "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
    act( AT_IMMORT, "You purge $N.", ch, NULL, victim, TO_CHAR );
    extract_char( victim, TRUE );
    return;
}


void do_low_purge( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    char * name;
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];
    int cnt;
    OBJ_INDEX_DATA  *vnum=NULL;
    char  sex1[MAX_STRING_LENGTH];
    char  sex2[MAX_STRING_LENGTH];
    ZONE_DATA *zone;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "maim who?\n\r", ch );
	return;
    }

    victim = NULL; obj = NULL;
    if ( ( victim = get_char_world( ch, arg ) ) == NULL
    &&	 ( obj    = get_obj_here ( ch, arg ) ) == NULL )
    {
	send_to_char( "You can't find that here.\n\r", ch );
	return;
    }

    if ( obj )
    {
	separate_obj( obj );
	act( AT_IMMORT, "$n purges $p!", ch, obj, NULL, TO_ROOM );
	act( AT_IMMORT, "You make $p disappear in a puff of smoke!", ch, obj, NULL, TO_CHAR );
	extract_obj( obj );
	return;
    }

/*    if ( !IS_NPC(victim) )
    {
	send_to_char( "Not on PC's.\n\r", ch );
	return;
    }*/

/*    if ( victim == ch )
    {
    	send_to_char( "You cannot purge yourself!\n\r", ch );
    	return;
    }*/

    if ( victim->level >= ch->level && ch != victim )
    {
     send_to_char("Heh, FUCK OFF!!\n\r",ch);
     do_low_purge(ch,"");
     return;
    }

    act( AT_IMMORT, "$n purges $N.", ch, NULL, victim, TO_NOTVICT );

   if ( !IS_NPC(victim) )
    {
    do_remove( victim, "all" );
    do_drop( victim, "all");
    do_purge( ch, "" );
    do_recall(victim,"");
    do_save( victim, "" );
    
if (victim->sex == 1)
{
strcpy(sex1,"his");
strcpy(sex2,"his");
}
if (victim->sex == 2 )
{
strcpy(sex1,"her");
strcpy(sex2,"her");
}
if (victim->sex == 0 )
{
strcpy(sex1,"its");
strcpy(sex2,"it");
}
    sprintf(buf,
    "THUNDER RIPS ACROSS THE SKIES!!!!
    Sudddenly above you, %s spins abound over and over
    as all %s limbs and skin are ripped from %s body!!!\n\r",
    (capitalize(victim->name)),sex1,sex2);
    echo_to_all(AT_BLOOD,buf,ECHOTAR_ALL);
    sprintf(buf,"The body parts of %s fall everywhere!!\n\r",capitalize (victim->name));
    echo_to_all(AT_BLOOD,buf,ECHOTAR_ALL);

   for ( d = first_descriptor; d; d = d->next )
   {
    if (d->character && d->character->in_room)
    {
     for( cnt=1; cnt < 7; cnt++ )
       {
         if (cnt == 1 ) vnum = get_obj_index(OBJ_VNUM_BLOOD,1);
         if (cnt == 2 ) vnum = get_obj_index(OBJ_VNUM_SPILLED_GUTS,1);
         if (cnt == 3 ) vnum = get_obj_index(OBJ_VNUM_SEVERED_HEAD,1);
         if (cnt == 4 ) vnum = get_obj_index(OBJ_VNUM_TORN_HEART,1);
         if (cnt == 5 ) vnum = get_obj_index(OBJ_VNUM_SLICED_ARM,1);
         if (cnt == 6 ) vnum = get_obj_index(OBJ_VNUM_SLICED_LEG,1);

	zone = find_zone( 1 );

        name            = IS_NPC(victim) ? victim->short_descr : victim->name;
        obj             = create_object(vnum, 0, zone );
        obj->timer      = number_range( 4, 7 );

        sprintf( buf, obj->short_descr, name );
        STRFREE( obj->short_descr );
        obj->short_descr = STRALLOC( buf );

        sprintf( buf, obj->description, name );
        STRFREE( obj->description );
        obj->description = STRALLOC( buf );

        obj_to_room( obj,d->character->in_room );
     }
  }
}
}

    if ( !IS_NPC(victim) )
    {
    set_cur_char(victim);
    raw_kill(ch,victim);
    return;
    }

    act( AT_IMMORT, "You make $N disappear in a puff of smoke!", ch, NULL, victim, TO_CHAR );
    extract_char( victim, TRUE );
    return;
}


/* Advance a Level - Rewritten to support the mud better, June 15, 1999 --GW */
void do_advance( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;
    int iLevel;
    int classnum=0,curlev=0;
    bool LOWER=FALSE;
    bool HIGHER=FALSE;
    char buf[MSL];
    char clsname[MSL];

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' || !is_number( arg2 ) || !is_number(arg3))
    {
	send_to_char( "Syntax: advance2 <char> <class#> <level>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't playing.\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    /* You can demote yourself but not someone else at your own trust. -- Narn */
    if ( get_trust( ch ) <= get_trust( victim ) && ch != victim )
    {
	send_to_char( "You can't do that.\n\r", ch );
	return;
    }

    if ( ( level = atoi( arg3 ) ) < 1 || level > MAX_LEVEL )
    {
	send_to_char( "Level must be 1 to 59.\n\r", ch );
	return;
    }

    if ( level > get_trust( ch ) )
    {
	send_to_char( "Limited to your trust level.\n\r", ch );
	return;
    }

    if (  (classnum = atoi(arg2)) < 1 || classnum > 4 )
    {
        send_to_char( "Classnum must be between 1 and 4!\n\r",ch);
	return;
    }

    if ( (classnum == 2 && victim->class2 <= -1 ) ||
	 (classnum == 3 && !IS_ADVANCED(victim)) ||
	 (classnum == 4 && !IS_ADV_DUAL(victim)))
	{
	  ch_printf(ch,"That player does not have %d classes!\n\r",classnum);
	  return;
	}


    if ( ( classnum == 1 && level <= victim->level ) ||
	 ( classnum == 2 && level <= victim->level2 ) ||
	 ( classnum == 3 && level <= victim->advlevel ) ||
	 ( classnum == 4 && level <= victim->advlevel2 ) )
    {
	send_to_char( "Lowering a player's level!\n\r", ch );
	LOWER=TRUE;
	set_char_color( AT_IMMORT, victim );
	send_to_char( "Cursed and forsaken! The gods have lowered your level.\n\r", victim );

        /* Rank fix added by Narn. */   
        DISPOSE( victim->pcdata->rank );
        victim->pcdata->rank = str_dup( "" );

/* Stuff added to make sure players wizinvis level doesnt stay higher 
 * than their actual level and to take wizinvis away from advance below 50
 */
     if (IS_SET (victim->act, PLR_WIZINVIS) )
       victim->pcdata->wizinvis = victim->pcdata->trust;

     if (IS_SET (victim->act, PLR_WIZINVIS) 
             && (victim->level <= LEVEL_AVATAR))
     {
       REMOVE_BIT(victim->act, PLR_WIZINVIS);
       victim->pcdata->wizinvis = victim->pcdata->trust;
     }   
    }
    else if ( ( classnum == 1 && level > victim->level ) ||
	 ( classnum == 2 && level > victim->level2 ) ||
	 ( classnum == 3 && level > victim->advlevel ) ||
	 ( classnum == 4 && level > victim->advlevel2 ) )
    {
	send_to_char( "Raising a player's level!\n\r", ch );
	HIGHER=TRUE;

	  set_char_color( AT_IMMORT, victim );
	  act( AT_IMMORT, "$n makes some arcane gestures with $s hands, then points $s finger at you!",
	       ch, NULL, victim, TO_VICT );
	  act( AT_IMMORT, "$n makes some arcane gestures with $s hands, then points $s finger at $N!",
	       ch, NULL, victim, TO_NOTVICT );
	  set_char_color( AT_WHITE, victim );
	  send_to_char( "You suddenly feel very strange...\n\r\n\r", victim );
	  set_char_color( AT_LBLUE, victim );

	switch(level)
	{
	default:
	  send_to_char( "The gods feel fit to raise your level!\n\r", victim );
	  break;
	case LEVEL_IMMORTAL:
	  do_help(victim, "M_IMMORTAL_" );
	  set_char_color( AT_WHITE, victim );
	  send_to_char( "You awake... all your possessions are gone.\n\r", victim );

	  while ( victim->first_carrying )
	    extract_obj( victim->first_carrying );
	  break;
	case LEVEL_WIZARD:
	  do_help(victim, "M_WIZARD_" );
	  break;
	case LEVEL_LORD:
	  do_help(victim, "M_LORD_" );
	  break;
	case LEVEL_TITAN:
	  do_help(victim, "M_TITAN_" );
	  break;
	case LEVEL_FATE:
	  do_help(victim, "M_FATE_" );
	  break;
	case LEVEL_GOD:
	  do_help(victim, "M_GOD_" );
	  break;
	case LEVEL_HEAD_GOD:
	  do_help(victim, "M_HEADGOD_" );
	  break;
	case LEVEL_COUNCIL:
	  do_help(victim, "M_COUNCIL_" );
	  break;
	case LEVEL_HYPERION:
	  do_help(victim, "M_HYPERION_" );
	  break;
	}
    }


    switch(classnum)
    {
	case 1:
	  curlev = victim->level;
	  break;
	case 2:
	  curlev = victim->level2;
	  break;
	case 3:
	  curlev = victim->advlevel;
	  break;
	case 4:
	  curlev = victim->advlevel2;
	  break;
    }

    for ( iLevel = curlev ; ((HIGHER && iLevel < level) || (LOWER && iLevel > level)); iLevel = iLevel )
    {
      if ( classnum == 1 )
      {
        if ( HIGHER )
        {
  	  advance_level( victim, FALSE, 1 );
  	  victim->level++;
	  iLevel++;
	}
	else
	{
	  lose_level(victim, victim->level-1,1);
	  victim->level--;	  
	  iLevel--;
	}
      }

      if ( classnum == 2 )
      {
	if ( HIGHER )
	{
	  advance_level2( victim, FALSE );
  	  victim->level2++;
	  iLevel++;
	}
	else
	{
	  lose_level(victim, victim->level2-1,2);
	  victim->level2--;
	  iLevel--;
	}
      }

      if ( classnum == 3 )
      {
	if ( HIGHER )
	{
	  advance_level( victim, FALSE, 3 );
  	  victim->advlevel++;
	  iLevel++;
	}
	else
	{
	  lose_level(victim, victim->advlevel-1,3);
	  victim->advlevel--;
	  iLevel--;
	}
      }

      if ( classnum == 4 )
      {
	if ( HIGHER )
	{
 	  advance_level( victim, FALSE, 4 );
  	  victim->advlevel2++;
	  iLevel++;
	}
	else
	{
	  lose_level(victim, victim->advlevel2-1,4);
	  victim->advlevel2--;
	  iLevel--;
	}
      }
    }
    
    switch(classnum)
    {
	case 1:
	   ch_printf(victim,"You are now level %d in your first class.\n\r",victim->level);
	   strcpy(clsname,"Single Class");
	   break;
	case 2:
	   ch_printf(victim,"You are now level %d in your second class.\n\r",victim->level2);
	   strcpy(clsname,"Dual Class");
	   break;
	case 3:
	   ch_printf(victim,"You are now level %d in your advanced class.\n\r",victim->advlevel);
	   strcpy(clsname,"Advanced Class");
	   break;
	case 4:
	   ch_printf(victim,"You are now level %d in your quad class.\n\r",victim->advlevel2);
	   strcpy(clsname,"Quad Class");
	   break;
    }

    victim->exp   = 0;
    victim->pcdata->trust = 0;
    /* Log it! */
    sprintf(buf,"%s %s %s to level %d (%s)",QUICKLINK(ch->name),HIGHER ? "PROMOTED" : "DEMOTED",
	QUICKLINK(victim->name),level,!IS_IMMORTAL(victim) ? clsname : "IMMORTAL");
    log_string_plus(buf,LOG_NORMAL,ch->level);
    return;
}

void do_cperm( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int lev;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number(arg2) )
    {
	send_to_char( "Syntax: cperm <char> <level>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    } 

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( ( lev = atoi( arg2 ) ) < 1 || lev > 10 )
    {
	send_to_char( "Level must be 1 to 10.\n\r", ch );
	return;
    }

    if ( ch->level < LEVEL_HYPERION && ch->pcdata->cperm < lev )
    {
	send_to_char( "Limited to your Security level.\n\r", ch );
	return;
    }
   

victim->pcdata->cperm = lev;
sprintf(buf,"%s set %s's Creation Security to %d", QUICKLINK(ch->name),
QUICKLINK(victim->name), lev );
log_string(buf);
send_to_char("Creation Security Level Set.\n\r",ch);
sprintf(buf2,"%s has set your Creation Security Level to %d.\n\r",ch->name,
	lev);
send_to_char(buf2,victim);
do_save(victim,"");
return;
}
void do_immortalize( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Syntax: immortalize <char>\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) )
     return;

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( victim->level != LEVEL_AVATAR )
    {
	send_to_char( "This player is not worthy of immortality yet.\n\r", ch );
	return;
    }

    send_to_char( "Immortalizing a player...\n\r", ch );
    set_char_color( AT_IMMORT, victim );
    act( AT_IMMORT, "$n begins to chant softly... then raises $s arms to the sky...",
	 ch, NULL, NULL, TO_ROOM );
    set_char_color( AT_WHITE, victim );
    send_to_char( "You suddenly feel very strange...\n\r\n\r", victim );
    set_char_color( AT_LBLUE, victim );

    do_help(victim, "M_GODLVL1_" );
    set_char_color( AT_WHITE, victim );
    send_to_char( "You awake... all your possessions are gone.\n\r", victim );
    while ( victim->first_carrying )
	extract_obj( victim->first_carrying );

    victim->level = LEVEL_IMMORTAL;
    advance_level( victim, FALSE, 1 );

    victim->exp   = exp_level( victim, victim->level );
    victim->pcdata->trust = 0;
    return;
}



void do_trust( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: trust <char> <level>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) )
       return;

    if ( ( level = atoi( arg2 ) ) < 0 || level > 59 )
    {
	send_to_char( "Level must be 0 (reset) or 1 to 59.\n\r", ch );
	return;
    }

    if ( level > get_trust( ch ) )
    {
	send_to_char( "Limited to your own trust.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You can't do that.\n\r", ch );
	return;
    }

    victim->pcdata->trust = level;
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_restore( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Restore whom?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

        if ( !ch->pcdata )
          return;

        if ( get_trust( ch ) < LEVEL_HEAD_GOD )
        {
          if ( IS_NPC( ch ) )
          {
  	    send_to_char( "You can't do that.\n\r", ch );
 	    return;
          }
          else
          {
            /* Check if the player did a restore all within the last 18 hours. */
            if ( current_time - last_restore_all_time < RESTORE_INTERVAL ) 
            {
              send_to_char( "Sorry, you can't do a restore all yet.\n\r", ch ); 
              do_restoretime( ch, "" );
              return;
            }
          }
        }
        last_restore_all_time    = current_time;
        ch->pcdata->restore_time = current_time;
        save_char_obj( ch );
        send_to_char( "Ok.\n\r", ch);
	for ( vch = first_char; vch; vch = vch_next )
	{
	    vch_next = vch->next;

	    if ( !IS_NPC( vch ) && !IS_IMMORTAL( vch ) )
	    {
   		vch->hit = vch->max_hit;
		vch->mental_state = 0;
		vch->mana = vch->max_mana;
		vch->move = vch->max_move;
		vch->pcdata->condition[COND_BLOODTHIRST] = get_bloodthirst(vch);
		update_pos (vch);
		act( AT_IMMORT, "$n has restored you.", ch, NULL, vch, TO_VICT);
	    }
	}
    }
    else
    {    

    CHAR_DATA *victim;

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( get_trust( ch ) < LEVEL_GOD 
      &&  victim != ch
      && !( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) ) )
    { 
      send_to_char( "You can't do that.\n\r", ch );
      return;
    }

    victim->hit  = victim->max_hit;
    victim->mental_state = 0;
    victim->mana = victim->max_mana;
    victim->move = victim->max_move;
    if ( victim->pcdata )
      victim->pcdata->condition[COND_BLOODTHIRST] = get_bloodthirst(victim);
    update_pos( victim );
    if ( ch != victim )
      act( AT_IMMORT, "$n has restored you.", ch, NULL, victim, TO_VICT );
    send_to_char( "Ok.\n\r", ch );
    return;
    }
}

void do_restoretime( CHAR_DATA *ch, char *argument )
{
  long int time_passed;
  int hour, minute;

  if ( !last_restore_all_time )
     ch_printf( ch, "There has been no restore all since reboot\n\r");
  else
     {
     time_passed = current_time - last_restore_all_time;
     hour = (int) ( time_passed / 3600 );
     minute = (int) ( ( time_passed - ( hour * 3600 ) ) / 60 );
     ch_printf( ch, "The  last restore all was %d hours and %d minutes ago.\n\r", 
                  hour, minute );
     }

  if ( !ch->pcdata )
    return;

  if ( !ch->pcdata->restore_time )
  {
    send_to_char( "You have never done a restore all.\n\r", ch );
    return;
  }

  time_passed = current_time - ch->pcdata->restore_time;
  hour = (int) ( time_passed / 3600 );
  minute = (int) ( ( time_passed - ( hour * 3600 ) ) / 60 );
  ch_printf( ch, "Your last restore all was %d hours and %d minutes ago.\n\r", 
                  hour, minute ); 
  return;
}

void do_freeze( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Freeze whom?\n\r", ch );
	return;
    }

   if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_FREEZE) )
    {
	REMOVE_BIT(victim->act, PLR_FREEZE);
	send_to_char( "You can play again.\n\r", victim );
	send_to_char( "FREEZE removed.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_FREEZE);
	send_to_char( "You can't do ANYthing!\n\r", victim );
	send_to_char( "FREEZE set.\n\r", ch );
    }

    save_char_obj( victim );

    return;
}



void do_log( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Log whom?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	if ( fLogAll )
	{
	    fLogAll = FALSE;
	    send_to_char( "Log ALL off.\n\r", ch );
	}
	else
	{
	    fLogAll = TRUE;
	    send_to_char( "Log ALL on.\n\r", ch );
	}
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    /*
     * No level check, gods can log anyone.
     */
    if ( IS_SET(victim->act, PLR_LOG) )
    {
	REMOVE_BIT(victim->act, PLR_LOG);
	send_to_char( "LOG removed.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_LOG);
	send_to_char( "LOG set.\n\r", ch );
    }

    return;
}


void do_litterbug( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Set litterbug flag on whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_LITTERBUG) )
    {
	REMOVE_BIT(victim->act, PLR_LITTERBUG);
	send_to_char( "You can drop items again.\n\r", victim );
	send_to_char( "LITTERBUG removed.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_LITTERBUG);
	send_to_char( "You a strange force prevents you from dropping any more items!\n\r", victim );
	send_to_char( "LITTERBUG set.\n\r", ch );
    }

    return;
}


void do_noemote( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Noemote whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_NO_EMOTE) )
    {
	REMOVE_BIT(victim->act, PLR_NO_EMOTE);
	send_to_char( "You can emote again.\n\r", victim );
	send_to_char( "NO_EMOTE removed.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_NO_EMOTE);
	send_to_char( "You can't emote!\n\r", victim );
	send_to_char( "NO_EMOTE set.\n\r", ch );
    }

    return;
}

void do_noecho( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
      {
    send_to_char( "noecho whom?\n\r", ch );
    return;
      }
    
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
      {
    send_to_char( "They aren't here\n\r", ch );
    return;
      }

    if ( IS_NPC(victim) )
      {
    send_to_char( "Mobs can't echo much\n\r", ch );
    return;
      }

    if ( !IS_IMMORTAL(victim) )
      {
    send_to_char( "Only on immortals\n\r", ch );
    return;
      }

    if ( get_trust(victim) >= get_trust(ch) )
      {
    send_to_char( "Hehe you're nuts\n\r", ch );
    return;
      }

    if ( victim == ch )
      {
    send_to_char( "Yeah right\n\r", ch );
    return;
      }
    
    if ( IS_SET(victim->pcdata->flags, PCFLAG_NO_ECHO) )
      {
    REMOVE_BIT( victim->pcdata->flags, PCFLAG_NO_ECHO );
    send_to_char( "You can echo again!\n\r", victim );
    send_to_char( "Done..\n\r", ch );
    return;
      }
    
    else
      {
    SET_BIT( victim->pcdata->flags, PCFLAG_NO_ECHO );
    send_to_char( "You can no longer echo!!\n\r", victim );
    send_to_char( "Done..\n\r", ch );
      }

  return;

}
        


void do_notell( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Notell whom?", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_NO_TELL) )
    {
	REMOVE_BIT(victim->act, PLR_NO_TELL);
	send_to_char( "You can tell again.\n\r", victim );
	send_to_char( "NO_TELL removed.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_NO_TELL);
	send_to_char( "You can't tell!\n\r", victim );
	send_to_char( "NO_TELL set.\n\r", ch );
    }

    return;
}


void do_notitle( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
 
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Notitle whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }
  
    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }
    
    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }
    
    if ( IS_SET(victim->pcdata->flags, PCFLAG_NOTITLE) )
    {
        REMOVE_BIT(victim->pcdata->flags, PCFLAG_NOTITLE);
        send_to_char( "You can set your own title again.\n\r", victim );
        send_to_char( "NOTITLE removed.\n\r", ch );
    }
    else
    {
        SET_BIT(victim->pcdata->flags, PCFLAG_NOTITLE);
        sprintf( buf, "the %s",
                title_table [victim->class] [victim->level]
                [victim->sex == SEX_FEMALE ? 1 : 0] );
        set_title( victim, buf );   
        send_to_char( "You can't set your own title!\n\r", victim );
        send_to_char( "NOTITLE set.\n\r", ch );
    }
    
    return;
}

void do_silence( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Silence whom?", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_SILENCE) )
    {
	send_to_char( "Player already silenced, use unsilence to remove.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_SILENCE);
	send_to_char( "You can't use channels!\n\r", victim );
	send_to_char( "SILENCE set.\n\r", ch );
    }

    return;
}

void do_unsilence( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Unsilence whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_SILENCE) )
    {
	REMOVE_BIT(victim->act, PLR_SILENCE);
	send_to_char( "You can use channels again.\n\r", victim );
	send_to_char( "SILENCE removed.\n\r", ch );
    }
    else
    {
	send_to_char( "That player is not silenced.\n\r", ch );
    }

    return;
}

void do_censor( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Censor whom?", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->pcdata->flags, PCFLAG_CENSORED) )
    {
	REMOVE_BIT(victim->pcdata->flags, PCFLAG_CENSORED);
	send_to_char( "You can once again use obscene language!\n\r", victim );
	send_to_char( "Victim no longer censored.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->pcdata->flags, PCFLAG_CENSORED);
	send_to_char( "You can no longer use obscene language!\n\r", victim );
	send_to_char( "Victim censored.\n\r", ch );
    }

    return;
}


void do_peace( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;

    act( AT_IMMORT, "$n booms, 'PEACE!'", ch, NULL, NULL, TO_ROOM );
    for ( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
    {
	if ( rch->fighting )
	{
	    stop_fighting( rch, TRUE );
	    do_sit( rch, "" );
	}
       
        /* Added by Narn, Nov 28/95 */
        stop_hating( rch );
        stop_hunting( rch );
        stop_fearing( rch );
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}



BAN_DATA *		first_ban;
BAN_DATA *		last_ban;

void save_banlist( void )
{
  BAN_DATA *pban;
  FILE *fp;

//  new_fclose( fpReserve );
  if ( !(fp = fopen( SYSTEM_DIR BAN_LIST, "w" )) )
  {
    bug( "Save_banlist: Cannot open " BAN_LIST, 0 );
    perror(BAN_LIST);
//    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }
  for ( pban = first_ban; pban; pban = pban->next )
    fprintf( fp, "%d %s~~%s~%s~\n", pban->level, pban->name,pban->ban_time,
		pban->ban_issuer );
  fprintf( fp, "-1\n" );
  new_fclose( fp );
//  fpReserve = fopen( NULL_FILE, "r" );
  return;
}
void do_ban( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    BAN_DATA *pban;
    int bnum;

    if ( IS_NPC(ch) )
	return;

    argument = one_argument( argument, arg );

    set_pager_color( AT_PLAIN, ch );
    if ( arg[0] == '\0' )
    {
	send_to_pager( "Banned sites:\n\r", ch );
	send_to_pager( "[ #] Issuer (Lv) Time Site\n\r", ch );
	send_to_pager( "----  ------------- ---- ------------------------ ---------------\n\r", ch );
	for ( pban = first_ban, bnum = 1; pban; pban = pban->next, bnum++ )
	    pager_printf(ch, "[%2d] [%-13s] (%2d) %-24s %s\n\r", bnum,
	            pban->ban_issuer, pban->level, pban->ban_time,
			pban->name);
	return;
    }
    
    /* People are gonna need .# instead of just # to ban by just last
       number in the site ip.                               -- Altrag */
    if ( is_number(arg) )
    {
      for ( pban = first_ban, bnum = 1; pban; pban = pban->next, bnum++ )
        if ( bnum == atoi(arg) )
          break;
      if ( !pban )
      {
        do_ban(ch, "");
        return;
      }
      argument = one_argument(argument, arg);
      if ( arg[0] == '\0' )
      {
        do_ban( ch, "help" );
        return;
      }
      if ( !str_cmp(arg, "level") )
      {
        argument = one_argument(argument, arg);
        if ( arg[0] == '\0' || !is_number(arg) )
        {
          do_ban( ch, "help" );
          return;
        }
        if ( atoi(arg) < 1 || atoi(arg) > LEVEL_HEAD_GOD )
        {
          ch_printf(ch, "Level range: 1 - %d.\n\r", LEVEL_HEAD_GOD);
          return;
        }
        pban->level = atoi(arg);
        send_to_char( "Ban level set.\n\r", ch );
      }
      else if ( !str_cmp(arg, "newban") )
      {
        pban->level = 1;
        send_to_char( "New characters banned.\n\r", ch );
      }
      else if ( !str_cmp(arg, "mortal") )
      {
        pban->level = LEVEL_AVATAR;
        send_to_char( "All mortals banned.\n\r", ch );
      }
      else if ( !str_cmp(arg, "total") )
      {
        pban->level = LEVEL_HYPERION;
        send_to_char( "Everyone banned.\n\r", ch );
      }
      else
      {
        do_ban(ch, "help");
        return;
      }
      save_banlist( );
      return;
    }
    
    if ( !str_cmp(arg, "help") )
    {
      send_to_char( "Syntax: ban <site address>\n\r", ch );
      send_to_char( "Syntax: ban <ban number> <level <lev>|newban|mortal|"
                    "total>\n\r", ch );
      return;
    }
    for ( pban = first_ban; pban; pban = pban->next )

    {
	if ( !str_cmp( arg, pban->name ) )
	{
	    send_to_char( "That site is already banned!\n\r", ch );
	    return;
	}
    }

    CREATE( pban, BAN_DATA, 1 );
    LINK( pban, first_ban, last_ban, next, prev );
    pban->name	= str_dup( arg );
    pban->level = LEVEL_AVATAR;
    sprintf(buf, "%24.24s", ctime(&current_time));
    pban->ban_time = str_dup( buf );
    /* Add the name of the person who issued the ban --GW */
    pban->ban_issuer = str_dup( ch->name );
    save_banlist( );
    send_to_char( "Ban created.  Mortals banned from site.\n\r", ch );
    return;
}




void do_allow( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    BAN_DATA *pban;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Remove which site from the ban list?\n\r", ch );
	return;
    }

    for ( pban = first_ban; pban; pban = pban->next )
    {
	if ( !str_cmp( arg, pban->name ) )
	{
	    UNLINK( pban, first_ban, last_ban, next, prev );
	    if ( pban->ban_time )
	      DISPOSE(pban->ban_time);
	    DISPOSE( pban->name );
	    DISPOSE( pban );
	    save_banlist( );
	    send_to_char( "Site no longer banned.\n\r", ch );
	    return;
	}
    }

    send_to_char( "Site is not banned.\n\r", ch );
    return;
}



void do_wizlock( CHAR_DATA *ch, char *argument )
{
    extern bool wizlock;
    wizlock = !wizlock;

    if ( wizlock )
	send_to_char( "Game wizlocked.\n\r", ch );
    else
	send_to_char( "Game un-wizlocked.\n\r", ch );

    return;
}


void do_noresolve( CHAR_DATA *ch, char *argument )
{
    sysdata.NO_NAME_RESOLVING = !sysdata.NO_NAME_RESOLVING;

    if ( sysdata.NO_NAME_RESOLVING )
	send_to_char( "Name resolving disabled.\n\r", ch );
    else
	send_to_char( "Name resolving enabled.\n\r", ch );

    return;
}


void do_users( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int count;
    char arg[MAX_INPUT_LENGTH];

    one_argument (argument, arg);
    count	= 0;
    buf[0]	= '\0';

    set_pager_color( AT_PLAIN, ch );
    sprintf(buf, 
"Desc|Con          |Idle      | Port | Player@HostIP                 ");
   strcat(buf, "\n\r");
   strcat(buf, "----+-------------+----------+------+-------------------------------");
   strcat(buf, "\n\r");
   send_to_pager(buf, ch);

    for ( d = first_descriptor; d; d = d->next )
    {
     int hours=0,mins=0,secs=0;
     char idle[MSL];

     idle[0] = '\0';

      if (arg[0] == '\0')
      {     
	if (  get_trust(ch) >= (LEVEL_HYPERION+2)
	||   (d->character && can_see( ch, d->character )) )
	{
	   /* Well i think its safe to say a for loop was not a great choice here :P Lets try this.. --GW*/
           secs = d->idle/3;
	   if ( secs > 60 )
	   {
            mins = (secs/60);
	    secs -= (mins*60);
	   }
	   if ( mins > 60 )
	   {
            hours = (mins/60);
	    mins -= (hours*60);
	   }

	  if ( hours > 0 )
		sprintf(idle,"%d:%2.2d Hrs",hours,mins == 0 ? 0 : mins );
	  else if ( mins > 0 )
		sprintf(idle,"%d:%2.2d Mns",mins,secs == 0 ? 0 : secs );
	  else
		sprintf(idle,"%d Sec",secs);
            
	    count++;
	    sprintf( buf,
	     " %3d| %-12s|%-10s|%6d| %-12s@%-16s ",
		d->descriptor,
		connection_names(d->connected),
		idle,
		d->port,
		d->character ? d->character->name : "(none)",
		d->host);
	    strcat(buf, "\n\r");
	    send_to_pager( buf, ch );
	}
      }
      else
      {
	if ( (get_trust(ch) >= (LEVEL_HYPERION+2)	||   (d->character && can_see( ch, d->character )) )
        &&   ( !str_prefix( arg, d->host ) 
	||   ( d->character && !str_prefix( arg, d->character->name ) ) ) )
	{
	   /* Well i think its safe to say a for loop was not a great choice here :P Lets try this.. --GW*/
           secs = d->idle/3;
	   if ( secs > 60 )
	   {
            mins = (secs/60);
	    secs -= (mins*60);
	   }
	   if ( mins > 60 )
	   {
            hours = (mins/60);
	    mins -= (hours*60);
	   }

	  if ( hours > 0 )
		sprintf(idle,"%d:%2.2d Hrs",hours,mins == 0 ? 0 : mins );
	  else if ( mins > 0 )
		sprintf(idle,"%d:%2.2d Mns",mins,secs == 0 ? 0 : secs );
	  else
		sprintf(idle,"%d Sec",secs);
            
	    count++;
	    sprintf( buf,
	     " %3d| %-12s|%-10s|%6d| %-12s@%-16s ",
		d->descriptor,
		connection_names(d->connected),
		idle,
		d->port,
		d->original  ? d->original->name  :
		d->character ? d->character->name : "(none)",
		d->host);
	    strcat(buf, "\n\r");
	    send_to_pager( buf, ch );
	}
      }
    }
    pager_printf( ch, "%d user%s.\n\r", count, count == 1 ? "" : "s" );
    if ( ch->level >= 59 )
	num_descriptors = count;

    return;
}



/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    bool mobsonly; 
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Force whom to do what?\n\r", ch );
	return;
    }

    mobsonly = get_trust( ch ) < sysdata.level_forcepc; 

    if ( !str_cmp( argument, "delete" ) )
	{
	send_to_char("No, ..... Idiot, you can't do that.\n\r",ch);
	return;
	}

    if ( !str_cmp( arg, "all" ) )
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

        if ( mobsonly )
        {
	  send_to_char( "Force whom to do what?\n\r", ch );
	  return;
        } 

	for ( vch = first_char; vch; vch = vch_next )
	{
	    vch_next = vch->next;

	    if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch ) )
	    {
		if ( ch->level < 58 )
		act( AT_IMMORT, "$n forces you to '$t'.", ch, argument, vch, TO_VICT );

		interpret( vch, argument );
	    }
	}
    }
    else
    {
	CHAR_DATA *victim;

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "Aye aye, right away!\n\r", ch );
	    return;
	}

	if ( ( get_trust( victim ) >= get_trust( ch ) ) 
          || ( mobsonly && !IS_NPC( victim ) ) )
	{
	    send_to_char( "Do it yourself!\n\r", ch );
	    return;
	}
	if ( ch->level < 58 )
	{
    act( AT_IMMORT, "$n forces you to '$t'.", ch, argument, victim, TO_VICT );
	}

	if ( IS_NPC(victim) && !IS_SET(victim->act, ACT_PROTOTYPE) && get_trust(ch) < 56 )
        {
         send_to_char("You may only force prototype mobiles.\n\r",ch);
         return;
        }

	interpret( victim, argument );
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}


void do_invis( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    sh_int level;
    char invis_buf[MAX_STRING_LENGTH];

    /*
    if ( IS_NPC(ch))
	return;
    */

    argument = one_argument( argument, arg );
    if ( arg && arg[0] != '\0' )
    {
	if ( !is_number( arg ) )
	{
	   send_to_char( "Usage:wizinvis | wizinvis <level>\n\r", ch );
	   return;
	}
	level = atoi( arg );
	if ( level < 2 || level > get_trust( ch ) )
	{
	    send_to_char( "Invalid level.\n\r", ch );
	    return;
	}

	if (!IS_NPC(ch))
        {
	  ch->pcdata->wizinvis = level;
	  ch_printf( ch, "Wizinvis level set to %d.\n\r", level );
        }
        
        if (IS_NPC(ch))       
        {
          ch->mobinvis = level;
          ch_printf( ch, "Mobinvis level set to %d.\n\r", level );
        }
	return;
    }
    
    if (!IS_NPC(ch))
    {
    if ( ch->pcdata->wizinvis < 2 )
      ch->pcdata->wizinvis = ch->level;
    }

    if (IS_NPC(ch))
    {
    if ( ch->mobinvis < 2 )
      ch->mobinvis = ch->level;
    }

    if ( IS_SET(ch->act, PLR_WIZINVIS) )
    {
	REMOVE_BIT(ch->act, PLR_WIZINVIS);
	act( AT_IMMORT, "$n slowly fades into view.", ch, NULL, NULL,
TO_ROOM );
	send_to_char( "Invisibility level set to 0.\n\r", ch );
    }
    else
    {
	SET_BIT(ch->act, PLR_WIZINVIS);
	act( AT_IMMORT, "$n slowly fades out of view.", ch, NULL, NULL,
TO_ROOM );
	sprintf(invis_buf,"Invisibility level set to %d.\n\r",
ch->pcdata->wizinvis );
        send_to_char(invis_buf,ch);
    }

    return;
}


void do_holylight( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( IS_SET(ch->act, PLR_HOLYLIGHT) )
    {
	REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode off.\n\r", ch );
    }
    else
    {
	SET_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode on.\n\r", ch );
    }

    return;
}

void do_rassign( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    sh_int  r_lo, r_hi;
    CHAR_DATA *victim;
    
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    r_lo = atoi( arg2 );  r_hi = atoi( arg3 );

    if ( arg1[0] == '\0' || r_lo < 0 || r_hi < 0 )
    {
	send_to_char( "Syntax: rassign <who> <low> <high>\n\r", ch );
	return;
    }
    if ( (victim = get_char_world( ch, arg1 )) == NULL )
    {
	send_to_char( "They don't seem to be around.\n\r", ch );
	return;
    }
    if ( IS_NPC( victim ) || ch->level <= 50 || !victim )
    {
	send_to_char( "They wouldn't know what to do with a room range.\n\r", ch );
	return;
    }
    if ( r_lo > r_hi )
    {
	send_to_char( "Unacceptable room range.\n\r", ch );
	return;
    }
    if ( r_lo == 0 )
       r_hi = 0;
    victim->pcdata->r_range_lo = r_lo;
    victim->pcdata->r_range_hi = r_hi;
    assign_area( victim );
    send_to_char( "Done.\n\r", ch );
    ch_printf( victim, "%s has assigned you the room vnum range %d - %d.\n\r",
		ch->name, r_lo, r_hi );
    assign_area( victim );	/* Put back by Thoric on 02/07/96 */
    if ( !victim->pcdata->area )
    {
	bug( "rassign: assign_area failed", 0 );
    	return;
    }
    if (r_lo == 0)				/* Scryn 8/12/95 */
    {
	REMOVE_BIT ( victim->pcdata->area->status, AREA_LOADED );
	SET_BIT( victim->pcdata->area->status, AREA_DELETED );
    }
    else
    {
        SET_BIT( victim->pcdata->area->status, AREA_LOADED );
	REMOVE_BIT( victim->pcdata->area->status, AREA_DELETED );
    }
    return;
}

void do_oassign( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    sh_int  o_lo, o_hi;
    CHAR_DATA *victim;
    
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    o_lo = atoi( arg2 );  o_hi = atoi( arg3 );

    if ( arg1[0] == '\0' || o_lo < 0 || o_hi < 0 )
    {
	send_to_char( "Syntax: oassign <who> <low> <high>\n\r", ch );
	return;
    }
    if ( (victim = get_char_world( ch, arg1 )) == NULL )
    {
	send_to_char( "They don't seem to be around.\n\r", ch );
	return;
    }
    if ( IS_NPC( victim ) || ch->level <= 50 )
    {
	send_to_char( "They wouldn't know what to do with an object range.\n\r", ch );
	return;
    }
    if ( o_lo > o_hi )
    {
	send_to_char( "Unacceptable object range.\n\r", ch );
	return;
    }
    victim->pcdata->o_range_lo = o_lo;
    victim->pcdata->o_range_hi = o_hi;
    assign_area( victim );
    send_to_char( "Done.\n\r", ch );
    ch_printf( victim, "%s has assigned you the object vnum range %d - %d.\n\r",
		ch->name, o_lo, o_hi );
    return;
}

void do_massign( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    sh_int  m_lo, m_hi;
    CHAR_DATA *victim;
    
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    m_lo = atoi( arg2 );  m_hi = atoi( arg3 );

    if ( arg1[0] == '\0' || m_lo < 0 || m_hi < 0 )
    {
	send_to_char( "Syntax: massign <who> <low> <high>\n\r", ch );
	return;
    }
    if ( (victim = get_char_world( ch, arg1 )) == NULL )
    {
	send_to_char( "They don't seem to be around.\n\r", ch );
	return;
    }
    if ( IS_NPC( victim ) || ch->level <= 50 )
    {
	send_to_char( "They wouldn't know what to do with a monster range.\n\r", ch );
	return;
    }
    if ( m_lo > m_hi )
    {
	send_to_char( "Unacceptable monster range.\n\r", ch );
	return;
    }
    victim->pcdata->m_range_lo = m_lo;
    victim->pcdata->m_range_hi = m_hi;
    assign_area( victim );
    send_to_char( "Done.\n\r", ch );
    ch_printf( victim, "%s has assigned you the monster vnum range %d - %d.\n\r",
		ch->name, m_lo, m_hi );
    return;
}

void do_cmdtable( CHAR_DATA *ch, char *argument )
{
    int hash, cnt;
    CMDTYPE *cmd;
 
    set_pager_color( AT_PLAIN, ch );
    send_to_pager("Commands and Number of Uses This Run\n\r", ch);
 
    for ( cnt = hash = 0; hash < 126; hash++ )
	for ( cmd = command_hash[hash]; cmd; cmd = cmd->next )
	{
	    if ((++cnt)%4)
		pager_printf(ch,"%-6.6s %4d\t",cmd->name,cmd->userec.num_uses);
	    else
		pager_printf(ch,"%-6.6s %4d\n\r", cmd->name,cmd->userec.num_uses );
	}
    return;
}

/*
 * Load up a player file
 */
void do_loadup( CHAR_DATA *ch, char *argument )
{
    char fname[1024];
    char name[256];
    struct stat fst;
    bool loaded;
    DESCRIPTOR_DATA *d=NULL;
    char buf[MAX_STRING_LENGTH];
    FILE *fp;

    one_argument( argument, name );
    if ( name[0] == '\0' )
    {
	send_to_char( "Usage: loadup <playername>\n\r", ch );
	return;
    }

    name[0] = UPPER(name[0]);

    sprintf( fname, "%s%c/%s", PLAYER_DIR, tolower(name[0]),
			capitalize( name ) );
    if ( stat( fname, &fst ) == -1 )
    {
    sprintf( fname, "%s%c/%s.gz", PLAYER_DIR, tolower(name[0]),
			capitalize(name) );
    }

    if ( stat( fname, &fst ) != -1 )
    {
      sprintf( buf, "Uncompressing Player Data for: %s (%dK)",name
        ,(int) fst.st_size/1024 );
        log_string_plus( buf, LOG_COMM, LEVEL_GOD );
        buf[0] = '\0';

    /* decompress if .gz file exists - Thx Alander */
    if ( ( fp = fopen( fname, "r" ) ) )
    {
        new_fclose( fp );
        sprintf( buf, "gunzip -f %s", fname );
        system( buf );
    sprintf( fname, "%s%c/%s", PLAYER_DIR, tolower(name[0]),
                        capitalize( name ) );
    }

	CREATE( d, DESCRIPTOR_DATA, 1 );
	d->next = NULL;
	d->prev = NULL;
	d->connected = CON_GET_NAME;
	d->outsize = 2000;
	CREATE( d->outbuf, char, d->outsize );
	
	loaded = load_char_obj( d, name, FALSE );
	add_char( d->character );
	char_to_room( d->character, ch->in_room );
	if ( get_trust(d->character) >= get_trust( ch ) )
	{
	   do_say( d->character, "Do *NOT* disturb me again!" );
	   send_to_char( "I think you'd better leave that player alone!\n\r", ch );
	   d->character->desc	= NULL;
	   return;	   
	}
	d->character->desc	= NULL;
	d->character		= NULL;	
	DISPOSE( d->outbuf );
	DISPOSE( d );
	ch_printf(ch, "Player %s loaded from room %d.\n\r", capitalize(name ),0 );
	sprintf(buf, "%s appears from nowhere, eyes glazed over.\n\r", capitalize( name ) );
        act( AT_IMMORT, buf, ch, NULL, NULL, TO_ROOM );
	send_to_char( "Done.\n\r", ch );
	return;
    }
    /* else no player file */
    send_to_char( "No such player.\n\r", ch );
    return;
}

void do_fixchar( CHAR_DATA *ch, char *argument )
{
    char name[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, name );
    if ( name[0] == '\0' )
    {
	send_to_char( "Usage: fixchar <playername>\n\r", ch );
	return;
    }
    victim = get_char_room( ch, name );
    if ( !victim )
    {
	send_to_char( "They're not here.\n\r", ch );
	return;
    }
    fix_char( victim );
/*  victim->armor	= 100;
    victim->mod_str	= 0;
    victim->mod_dex	= 0;
    victim->mod_wis	= 0;
    victim->mod_int	= 0;
    victim->mod_con	= 0;
    victim->mod_cha	= 0;
    victim->mod_lck	= 0;
    victim->damroll	= 0;
    victim->hitroll	= 0;
    victim->alignment	= URANGE( -1000, victim->alignment, 1000 );
    victim->saving_spell_staff = 0; */
    send_to_char( "Done.\n\r", ch );
}

void do_newbieset( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    CHAR_DATA *victim;
    ZONE_DATA *zone;
        
    argument = one_argument( argument, arg1 );
    argument = one_argument (argument, arg2);

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Syntax: nbag <char>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( ( victim->level < 1 ) || ( victim->level > 5 ) )
    {
     send_to_char( "Level of victim must be 1 to 5.\n\r", ch );
	return;
    }

     zone = find_zone(1);

     obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_VEST,1), 1,zone );
     obj_to_char(obj, victim);

     obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_SHIELD,1), 1,zone );
     obj_to_char(obj, victim);

     obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_CLOAK,1), 1,zone );
     obj_to_char(obj, victim);

     obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_HELMET,1), 1,zone );
     obj_to_char(obj, victim);

     obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_LEGGINGS,1),1,zone );
     obj_to_char(obj, victim);

     obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_BOOTS,1), 1,zone );
     obj_to_char(obj, victim);

     obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_GLOVES,1), 1,zone );
     obj_to_char(obj, victim);

     obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_CAPE,1), 1,zone );
     obj_to_char(obj, victim);

     obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_BELT,1), 1,zone );
     obj_to_char(obj, victim);

     obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_BRACER,1), 1,zone );
     obj_to_char(obj, victim);

     obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_BRACER,1), 1,zone );
     obj_to_char(obj, victim);

     obj = create_object( get_obj_index(25505,1), 1,zone );
     obj_to_char(obj, victim);

     if ( (victim->class == 0) || (victim->class == 2)
     || (victim->class == 4) || (victim->class == 7) 
     || (victim->class == 6) || (victim->class == 10) )
     {
     obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_DAGGER,1), 1,zone );
     obj_to_char(obj, victim);
     }
     else if ( (victim->class == 1) || (victim->class == 5) 
             || (victim->class == 11))
     {
     obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_MACE,1), 1,zone );
     obj_to_char(obj, victim);
     }
     else if ( (victim->class == 3) || (victim->class == 6) 
             || (victim->class == 8))
     {
     obj = create_object(get_obj_index(OBJ_VNUM_SCHOOL_SWORD,1), 1,zone );
     obj_to_char(obj, victim);
     }
  
     /* Added by Brittany, on Nov. 24, 1996. The object is the adventurer's 
          guide to the realms of despair, part of academy.are. */
     {
       OBJ_INDEX_DATA *obj_ind = get_obj_index( 10333,1 );
       if ( obj_ind != NULL )
       {
         obj = create_object( obj_ind, 1, zone );
         obj_to_char( obj, victim );
       }
     }

/* Added the burlap sack to the newbieset.  The sack is part of sgate.are
   called Spectral Gate.  Brittany */

     {

       OBJ_INDEX_DATA *obj_ind = get_obj_index( 123,1 );
       if ( obj_ind != NULL )
       {
         obj = create_object( obj_ind, 1, zone );
         obj_to_char( obj, victim );
       }
     }

    act( AT_IMMORT, "$n has equipped you with a newbieset.", ch, NULL, victim, TO_VICT);
    ch_printf( ch, "You have re-equipped %s.\n\r", victim->name );
    return;
}

void do_refit( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    CHAR_DATA *victim;
    ZONE_DATA *zone;
        
  victim = ch;

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( ( victim->level < 1 ) || ( victim->level > 5 ) )
    {
     send_to_char( "You must be level 1 to 5.\n\r", ch );
	return;
    }

     zone = find_zone(1);

     obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_VEST,1), 1,zone );
     obj_to_char(obj, victim);

     obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_SHIELD,1), 1,zone );
     obj_to_char(obj, victim);

     obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_CLOAK,1), 1,zone );
     obj_to_char(obj, victim);

     obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_HELMET,1), 1,zone );
     obj_to_char(obj, victim);

     obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_LEGGINGS,1),1,zone );
     obj_to_char(obj, victim);

     obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_BOOTS,1), 1,zone );
     obj_to_char(obj, victim);

     obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_GLOVES,1), 1,zone );
     obj_to_char(obj, victim);

     obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_CAPE,1), 1,zone );
     obj_to_char(obj, victim);

     obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_BELT,1), 1,zone );
     obj_to_char(obj, victim);

     obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_BRACER,1), 1,zone );
     obj_to_char(obj, victim);

     obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_BRACER,1), 1,zone );
     obj_to_char(obj, victim);

     obj = create_object( get_obj_index(25505,1), 1,zone );
     obj_to_char(obj, victim);

     if ( (victim->class == 0) || (victim->class == 2)
     || (victim->class == 4) || (victim->class == 7) )
     {
     obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_DAGGER,1), 1, zone );
     obj_to_char(obj, victim);
     }
     else if ( (victim->class == 1) || (victim->class == 5) )
     {
     obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_MACE,1), 1,zone );
     obj_to_char(obj, victim);
     }
     else if ( (victim->class == 3) || (victim->class == 6) )
     {
     obj = create_object(get_obj_index(OBJ_VNUM_SCHOOL_SWORD,1), 1,zone );
     obj_to_char(obj, victim);
     }
  
    send_to_char("You have been re-equipped\n\r", victim);
    return;
}

/*
 * Extract area names from "input" string and place result in "output" string
 * e.g. "aset joe.are sedit susan.are cset" --> "joe.are susan.are"
 * - Gorog
 */
void extract_area_names (char *inp, char *out)
{
char buf[MAX_INPUT_LENGTH], *pbuf=buf;
int  len;

*out='\0';
while (inp && *inp)
   {
   inp = one_argument(inp, buf);
   if ( (len=strlen(buf)) >= 5 && !strcmp(".are", pbuf+len-4) )
       {
       if (*out) strcat (out, " ");
       strcat (out, buf);
       }
   }
}

/*
 * Remove area names from "input" string and place result in "output" string
 * e.g. "aset joe.are sedit susan.are cset" --> "aset sedit cset"
 * - Gorog
 */
void remove_area_names (char *inp, char *out)
{
char buf[MAX_INPUT_LENGTH], *pbuf=buf;
int  len;

*out='\0';
while (inp && *inp)
   {
   inp = one_argument(inp, buf);
   if ( (len=strlen(buf)) < 5 || strcmp(".are", pbuf+len-4) )
       {
       if (*out) strcat (out, " ");
       strcat (out, buf);
       }
   }
}

/*
 * Allows members of the Area Council to add Area names to the bestow field.
 * Area names mus end with ".are" so that no commands can be bestowed.
 */
void do_bestowarea( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int  arg_len;

    argument = one_argument( argument, arg );

    if ( str_cmp( ch->pcdata->council_name, "Area Council" )
    &&   get_trust (ch) < LEVEL_COUNCIL )
       {
       send_to_char( "Sorry. You are not on the Area Council.\n\r", ch );
       return;
       }

    if ( !*arg )
        {
        send_to_char(
        "Syntax:\n\r"
        "bestowarea <victim> <filename>.are\n\r"
        "bestowarea <victim> none             removes bestowed areas\n\r"
        "bestowarea <victim> list             lists bestowed areas\n\r"
        "bestowarea <victim>                  lists bestowed areas\n\r", ch);
        return;
        }

    if ( !(victim = get_char_world( ch, arg )) )
        {
        send_to_char( "They aren't here.\n\r", ch );
        return;
        }

    if ( IS_NPC( victim ) )
        {
        send_to_char( "You can't give special abilities to a mob!\n\r", ch );
        return;
        }

    if ( get_trust(victim) < LEVEL_IMMORTAL )
        {
        send_to_char( "They aren't an immortal.\n\r", ch );
        return;
        }

    if (!victim->pcdata->bestowments)
       victim->pcdata->bestowments = str_dup("");

    if ( !*argument || !str_cmp (argument, "list") )
       {
       extract_area_names (victim->pcdata->bestowments, buf);
       ch_printf( ch, "Bestowed areas: %s\n\r", buf);
       return;
       }

    if ( !str_cmp (argument, "none") )
       {
       remove_area_names (victim->pcdata->bestowments, buf);
       DISPOSE( victim->pcdata->bestowments );
       victim->pcdata->bestowments = str_dup( buf );
       send_to_char( "Done.\n\r", ch);
       return;
       }

    arg_len = strlen(argument);
    if ( arg_len < 5 
    || argument[arg_len-4] != '.' || argument[arg_len-3] != 'a'
    || argument[arg_len-2] != 'r' || argument[arg_len-1] != 'e' )
    {
        send_to_char( "You can only bestow an area name\n\r", ch );
        send_to_char( "E.G. bestow joe sam.are\n\r", ch );
        return;
    }

    sprintf( buf, "%s %s", victim->pcdata->bestowments, argument );
    DISPOSE( victim->pcdata->bestowments );
    victim->pcdata->bestowments = str_dup( buf );
    ch_printf( victim, "%s has bestowed on you the area: %s\n\r", 
             ch->name, argument );
    send_to_char( "Done.\n\r", ch );
}

void do_bestow( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Bestow whom with what?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC( victim ) )
    {
	send_to_char( "You can't give special abilities to a mob!\n\r", ch );
	return;
    }

    if ( get_trust( victim ) > get_trust( ch ) )
    {
	send_to_char( "You aren't powerful enough...\n\r", ch );
	return;
    }

    if( ch == victim )
    {
	send_to_char( "In your dreams!\n\r", ch );
	return;
    }

    if (!victim->pcdata->bestowments)
      victim->pcdata->bestowments = str_dup("");

    if ( argument[0] == '\0' || !str_cmp( argument, "list" ) )
    {
        ch_printf( ch, "Current bestowed commands on %s: %s.\n\r",
                      victim->name, victim->pcdata->bestowments );
        return;
    }

    if ( !str_cmp( argument, "none" ) )
    {
        DISPOSE( victim->pcdata->bestowments );
	victim->pcdata->bestowments = str_dup("");
        ch_printf( ch, "Bestowments removed from %s.\n\r", victim->name );
        ch_printf( victim, "%s has removed your bestowed commands.\n\r", ch->name );
        return;
    }

    sprintf( buf, "%s %s", victim->pcdata->bestowments, argument );
    DISPOSE( victim->pcdata->bestowments );
    victim->pcdata->bestowments = str_dup( buf );

    if ( argument[0] != '-' )
    ch_printf( victim, "%s has bestowed on you the command(s): %s\n\r", 
             ch->name, argument );
    else
    ch_printf( victim, "%s has forbidden you from using the command: %s\n\r", 
             ch->name, argument );


    send_to_char( "Done.\n\r", ch );
}

struct tm *update_time ( struct tm *old_time )
{
   time_t time;

   time = mktime(old_time); 
   return localtime(&time);
}

void do_set_boot_time( CHAR_DATA *ch, char *argument)
{
   char arg[MAX_INPUT_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   bool check;
 
   check = FALSE;

   argument = one_argument(argument, arg);

    if ( arg[0] == '\0' )
    {
	send_to_char( "Syntax: setboot time {hour minute <day> <month> <year>}\n\r", ch);
	send_to_char( "        setboot manual {0/1}\n\r", ch);
	send_to_char( "        setboot default\n\r", ch); 
        ch_printf( ch, "Boot time is currently set to %s, manual bit is set to %d\n\r"
	,reboot_time, set_boot_time->manual );
	return;
    }

    if ( !str_cmp(arg, "time") )
    {
      struct tm *now_time;
      
      argument = one_argument(argument, arg);
      argument = one_argument(argument, arg1);
      if ( !*arg || !*arg1 || !is_number(arg) || !is_number(arg1) )
      {
	send_to_char("You must input a value for hour and minute.\n\r", ch);
 	return;
      }
      now_time = localtime(&current_time);

      if ( (now_time->tm_hour = atoi(arg)) < 0 || now_time->tm_hour > 23 )
      {
        send_to_char("Valid range for hour is 0 to 23.\n\r", ch);
        return;
      }
 
      if ( (now_time->tm_min = atoi(arg1)) < 0 || now_time->tm_min > 59 )
      {
        send_to_char("Valid range for minute is 0 to 59.\n\r", ch);
        return;
      }

      argument = one_argument(argument, arg);
      if ( *arg != '\0' && is_number(arg) )
      {
        if ( (now_time->tm_mday = atoi(arg)) < 1 || now_time->tm_mday > 31 )
        {
	  send_to_char("Valid range for day is 1 to 31.\n\r", ch);
	  return;
        }
        argument = one_argument(argument, arg);
        if ( *arg != '\0' && is_number(arg) )
        {
          if ( (now_time->tm_mon = atoi(arg)) < 1 || now_time->tm_mon > 12 )
          {
            send_to_char( "Valid range for month is 1 to 12.\n\r", ch );
            return;
          }
          now_time->tm_mon--;
          argument = one_argument(argument, arg);
          if ( (now_time->tm_year = atoi(arg)-1900) < 0 ||
                now_time->tm_year > 199 )
          {
            send_to_char( "Valid range for year is 1900 to 2099.\n\r", ch );
            return;
          }
        }
      }
      now_time->tm_sec = 0;
      if ( mktime(now_time) < current_time )
      {
        send_to_char( "You can't set a time previous to today!\n\r", ch );
        return;
      }
      if (set_boot_time->manual == 0)
 	set_boot_time->manual = 1;
      new_boot_time = update_time(now_time);
      new_boot_struct = *new_boot_time;
      new_boot_time = &new_boot_struct;
      reboot_check(mktime(new_boot_time));
      get_reboot_string();

      ch_printf(ch, "Boot time set to %s\n\r", reboot_time);
      check = TRUE;
    }  
    else if ( !str_cmp(arg, "manual") )
    {
      argument = one_argument(argument, arg1);
      if (arg1[0] == '\0')
      {
	send_to_char("Please enter a value for manual boot on/off\n\r", ch);
	return;
      }
      
      if ( !is_number(arg1))
      {
	send_to_char("Value for manual must be 0 (off) or 1 (on)\n\r", ch);
	return;
      }

      if (atoi(arg1) < 0 || atoi(arg1) > 1)
      {
	send_to_char("Value for manual must be 0 (off) or 1 (on)\n\r", ch);
	return;
      }
   
      set_boot_time->manual = atoi(arg1);
      ch_printf(ch, "Manual bit set to %s\n\r", arg1);
      check = TRUE;
      get_reboot_string();
      return;
    }

    else if (!str_cmp( arg, "default" ))
    {
      set_boot_time->manual = 0;
    /* Reinitialize new_boot_time */
      new_boot_time = localtime(&current_time);
      new_boot_time->tm_mday += 1;
      if (new_boot_time->tm_hour > 12)
      new_boot_time->tm_mday += 1; 
      new_boot_time->tm_hour = 6;
      new_boot_time->tm_min = 0;
      new_boot_time->tm_sec = 0;
      new_boot_time = update_time(new_boot_time);

      sysdata.DENY_NEW_PLAYERS = FALSE;

      send_to_char("Reboot time set back to normal.\n\r", ch);
      check = TRUE;
    }

    if (!check)
    {
      send_to_char("Invalid argument for setboot.\n\r", ch);
      return;
    }

    else
    {
      get_reboot_string();
      new_boot_time_t = mktime(new_boot_time);
    }
}
/* Online high level immortal command for displaying what the encryption
 * of a name/password would be, taking in 2 arguments - the name and the
 * password - can still only change the password if you have access to 
 * pfiles and the correct password
 */ 
void do_form_password( CHAR_DATA *ch, char *argument) 
{
   char arg[MAX_STRING_LENGTH];
  
   argument = one_argument(argument, arg);
   
   ch_printf(ch, "Those two arguments encrypted would result in: %s", 
   crypt(arg, argument)); 
   return;
}

/*
 * Purge a player file.  No more player.  -- Altrag
 */
void do_destro( CHAR_DATA *ch, char *argument )
{
  set_char_color( AT_RED, ch );
  send_to_char("If you want to destroy a character, spell it out!\n\r",ch);
  return;
}

/*
 * This could have other applications too.. move if needed. -- Altrag
 */
void close_area( AREA_DATA *pArea )
{
/*  extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];
  extern OBJ_INDEX_DATA   *obj_index_hash[MAX_KEY_HASH];
  extern MOB_INDEX_DATA   *mob_index_hash[MAX_KEY_HASH];*/
  CHAR_DATA *ech;
  CHAR_DATA *ech_next;
  ZONE_DATA *in_zone;
  OBJ_DATA *eobj;
  OBJ_DATA *eobj_next;
  int icnt;
  ROOM_INDEX_DATA *rid;
  ROOM_INDEX_DATA *rid_next;
  OBJ_INDEX_DATA *oid;
  OBJ_INDEX_DATA *oid_next;
  MOB_INDEX_DATA *mid;
  MOB_INDEX_DATA *mid_next;
  RESET_DATA *ereset;
  RESET_DATA *ereset_next;
  EXTRA_DESCR_DATA *eed;
  EXTRA_DESCR_DATA *eed_next;
  EXIT_DATA *exit;
  EXIT_DATA *exit_next;
  MPROG_ACT_LIST *mpact;
  MPROG_ACT_LIST *mpact_next;
  MPROG_DATA *mprog;
  MPROG_DATA *mprog_next;
  AFFECT_DATA *paf;
  AFFECT_DATA *paf_next;

in_zone = pArea->zone;
  
  for ( ech = first_char; ech; ech = ech_next )
  {
    ech_next = ech->next;
    
    if ( ech->fighting )
      stop_fighting( ech, TRUE );
    if ( IS_NPC(ech) )
    {
      /* if mob is in area, or part of area. */
      if ( URANGE(pArea->low_m_vnum, ech->pIndexData->vnum,
                  pArea->hi_m_vnum) == ech->pIndexData->vnum ||
          (ech->in_room && ech->in_room->area == pArea) )
        extract_char( ech, TRUE );
      continue;
    }
    if ( ech->in_room && ech->in_room->area == pArea )
      do_recall( ech, "" );
  }

  for ( ech = in_zone->first_mob; ech; ech = ech_next )
  {
    ech_next = ech->next;
    
    if ( ech->fighting )
      stop_fighting( ech, TRUE );
    if ( IS_NPC(ech) )
    {
      /* if mob is in area, or part of area. */
      if ( URANGE(pArea->low_m_vnum, ech->pIndexData->vnum,
                  pArea->hi_m_vnum) == ech->pIndexData->vnum ||
          (ech->in_room && ech->in_room->area == pArea) )
        extract_char( ech, TRUE );
      continue;
    }
    if ( ech->in_room && ech->in_room->area == pArea )
      do_recall( ech, "" );
  }

  for ( eobj = in_zone->first_obj; eobj; eobj = eobj_next )
  {
    eobj_next = eobj->next;
    /* if obj is in area, or part of area. */
    if ( URANGE(pArea->low_o_vnum, eobj->pIndexData->vnum,
                pArea->hi_o_vnum) == eobj->pIndexData->vnum ||
        (eobj->in_room && eobj->in_room->area == pArea) )
      extract_obj( eobj );
  }
  for ( icnt = 0; icnt < MAX_KEY_HASH; icnt++ )
  {
    for ( rid = in_zone->room_index_hash[icnt]; rid; rid = rid_next )
    {
      rid_next = rid->next;
      
      for ( exit = rid->first_exit; exit; exit = exit_next )
      {
        exit_next = exit->next;
        if ( rid->area == pArea || exit->to_room->area == pArea )
        {
          STRFREE( exit->keyword );
          STRFREE( exit->description );
          UNLINK( exit, rid->first_exit, rid->last_exit, next, prev );
          DISPOSE( exit );
        }
      }
      if ( rid->area != pArea )
        continue;
      STRFREE(rid->name);
      STRFREE(rid->description);
      if ( rid->first_person )
      {
        bug( "close_area: room with people #%d", rid->vnum );
        for ( ech = rid->first_person; ech; ech = ech_next )
        {
          ech_next = ech->next_in_room;
          if ( ech->fighting )
            stop_fighting( ech, TRUE );
          if ( IS_NPC(ech) )
            extract_char( ech, TRUE );
          else
            do_recall( ech, "" );
        }
      }
      if ( rid->first_content )
      {
        bug( "close_area: room with contents #%d", rid->vnum );
        for ( eobj = rid->first_content; eobj; eobj = eobj_next )
        {
          eobj_next = eobj->next_content;
          extract_obj( eobj );
        }
      }
      for ( eed = rid->first_extradesc; eed; eed = eed_next )
      {
        eed_next = eed->next;
        STRFREE( eed->keyword );
        STRFREE( eed->description );
        DISPOSE( eed );
      }
      for ( mpact = rid->mpact; mpact; mpact = mpact_next )
      {
        mpact_next = mpact->next;
        STRFREE( mpact->buf );
        DISPOSE( mpact );
      }
      for ( mprog = rid->mudprogs; mprog; mprog = mprog_next )
      {
        mprog_next = mprog->next;
        STRFREE( mprog->arglist );
        STRFREE( mprog->comlist );
        DISPOSE( mprog );
      }
      if ( rid == in_zone->room_index_hash[icnt] )
        in_zone->room_index_hash[icnt] = rid->next;
      else
      {
        ROOM_INDEX_DATA *trid;
        
        for ( trid = in_zone->room_index_hash[icnt]; trid; trid =trid->next )
          if ( trid->next == rid )
            break;
        if ( !trid )
          bug( "Close_area: rid not in hash list %d", rid->vnum );
        else
          trid->next = rid->next;
      }
      DISPOSE(rid);
    }
    
    for ( mid = in_zone->mob_index_hash[icnt]; mid; mid = mid_next )
    {
      mid_next = mid->next;
      
      if ( mid->vnum < pArea->low_m_vnum || mid->vnum > pArea->hi_m_vnum )
        continue;
      
      STRFREE( mid->player_name );
      STRFREE( mid->short_descr );
      STRFREE( mid->long_descr  );
      STRFREE( mid->description );
      if ( mid->pShop )
      {
        UNLINK( mid->pShop, first_shop, last_shop, next, prev );
        DISPOSE( mid->pShop );
      }
      if ( mid->rShop )
      {
        UNLINK( mid->rShop, first_repair, last_repair, next, prev );
        DISPOSE( mid->rShop );
      }
      for ( mprog = mid->mudprogs; mprog; mprog = mprog_next )
      {
        mprog_next = mprog->next;
        STRFREE(mprog->arglist);
        STRFREE(mprog->comlist);
        DISPOSE(mprog);
      }
      if ( mid == in_zone->mob_index_hash[icnt] )
        in_zone->mob_index_hash[icnt] = mid->next;
      else
      {
        MOB_INDEX_DATA *tmid;
        
        for ( tmid = in_zone->mob_index_hash[icnt]; tmid; tmid =tmid->next )
          if ( tmid->next == mid )
            break;
        if ( !tmid )
          bug( "Close_area: mid not in hash list %s", mid->vnum );
        else
          tmid->next = mid->next;
      }
      DISPOSE(mid);
    }
    
    for ( oid = in_zone->obj_index_hash[icnt]; oid; oid = oid_next )
    {
      oid_next = oid->next;
      
      if ( oid->vnum < pArea->low_o_vnum || oid->vnum > pArea->hi_o_vnum )
        continue;
      
      STRFREE(oid->name);
      STRFREE(oid->short_descr);
      STRFREE(oid->description);
      STRFREE(oid->action_desc);

      for ( eed = oid->first_extradesc; eed; eed = eed_next )
      {
        eed_next = eed->next;
        STRFREE(eed->keyword);
        STRFREE(eed->description);
        DISPOSE(eed);
      }
      for ( paf = oid->first_affect; paf; paf = paf_next )
      {
        paf_next = paf->next;
        DISPOSE(paf);
      }
      for ( mprog = oid->mudprogs; mprog; mprog = mprog_next )
      {
        mprog_next = mprog->next;
        STRFREE(mprog->arglist);
        STRFREE(mprog->comlist);
        DISPOSE(mprog);
      }
      if ( oid == in_zone->obj_index_hash[icnt] )
        in_zone->obj_index_hash[icnt] = oid->next;
      else
      {
        OBJ_INDEX_DATA *toid;
        
        for ( toid = in_zone->obj_index_hash[icnt]; toid; toid =toid->next )
          if ( toid->next == oid )
            break;
        if ( !toid )
          bug( "Close_area: oid not in hash list %s", oid->vnum );
        else
          toid->next = oid->next;
      }
      DISPOSE(oid);
    }
  }
  for ( ereset = pArea->first_reset; ereset; ereset = ereset_next )
  {
    ereset_next = ereset->next;
    DISPOSE(ereset);
  }
  DISPOSE(pArea->name);
  DISPOSE(pArea->filename);
  STRFREE(pArea->author);
  UNLINK( pArea, first_build, last_build, next, prev );
  UNLINK( pArea, first_asort, last_asort, next_sort, prev_sort );
  DISPOSE( pArea );
}

void do_destroy( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  struct stat fst;

  one_argument( argument, arg );
  if ( arg[0] == '\0' )
  {

      send_to_char( "Destroy what player file?\n\r", ch );
      return;
  }

  for ( victim = first_char; victim; victim = victim->next )
    if ( !IS_NPC(victim) && !str_cmp(victim->name, arg) )
      break;
  if ( !victim )
  {
    DESCRIPTOR_DATA *d;

    /* Make sure they aren't halfway logged in. */
    for ( d = first_descriptor; d; d = d->next )
      if ( (victim = d->character) && !IS_NPC(victim) &&
          !str_cmp(victim->name, arg) )
        break;
    if ( d )
      write_to_buffer( d, "\n\rYour Character has been Deleted.\n\r",0);
      close_socket( d, TRUE );
  }
  else
  {
    int x, y;
    send_to_char("\n\rYour Character hasbeen DELETED!\n\r",victim);
    quitting_char = victim;
    save_char_obj( victim );
    saving_char = NULL;
    extract_char( victim, TRUE );
    for ( x = 0; x < MAX_WEAR; x++ )
        for ( y = 0; y < MAX_LAYERS; y++ )
            save_equipment[x][y] = NULL;
  }

  sprintf( buf, "%s%c/%s", PLAYER_DIR, tolower(arg[0]),
          capitalize( arg ) );

    if ( stat( buf, &fst ) == -1 )
    {
    sprintf( buf, "%s%c/%s.gz", PLAYER_DIR, tolower(arg[0]),
                        capitalize(arg) );
    }

  sprintf( buf2, "%s%c/%s", BACKUP_DIR, tolower(arg[0]),
          capitalize( arg ) );

  if ( !rename( buf, buf2 ) )
  {
    /*AREA_DATA *pArea;*/

        sprintf( buf,"gzip -f %s%c/%s",BACKUP_DIR, tolower(ch->name[0]),
                                capitalize( ch->name ));
        system( buf );

    set_char_color( AT_RED, ch );
    send_to_char( "Player destroyed.  Pfile saved in backup directory.\n\r", ch);
    sprintf( buf, "%s%s", GOD_DIR, capitalize(arg) );
    if ( !remove( buf ) )
      send_to_char( "Player's immortal data destroyed.\n\r", ch );

    else if ( errno != ENOENT )
    {
      sprintf( buf2, "%s destroying %s", ch->name, buf );
      perror( buf2 );

    }

/*    sprintf( buf2, "%s.are", capitalize(arg) );
    for ( pArea = first_build; pArea; pArea = pArea->next )
      if ( !strcmp( pArea->filename, buf2 ) )
      {
        sprintf( buf, "%s%s", BUILD_DIR, buf2 );
        if ( IS_SET( pArea->status, AREA_LOADED ) )
          fold_area( pArea, buf, FALSE );
        close_area( pArea );
        sprintf( buf2, "%s.bak", buf );
        set_char_color( AT_RED, ch );  Log message changes colors
        if ( !rename( buf, buf2 ) )
          send_to_char( "Player's area data destroyed.  Area saved as
backup.\n$
        else if ( errno != ENOENT )
        {
          ch_printf( ch, "Unknown error #%d - %s (area data).  Report to
Thoric$

                  errno, strerror( errno ) );
          sprintf( buf2, "%s destroying %s", ch->name, buf );
          perror( buf2 );
        }
      }*/
  }
  else if ( errno == ENOENT )
  {
    set_char_color( AT_PLAIN, ch );
    send_to_char( "Player does not exist.\n\r", ch );
  }
  else
  {
    set_char_color( AT_WHITE, ch );
    ch_printf( ch, "Unknown error #%d - %s.  Report to Thoric.\n\r",
            errno, strerror( errno ) );
    sprintf( buf, "%s destroying %s", ch->name, arg );

    perror( buf );
  }
  return;
}



extern ROOM_INDEX_DATA *       room_index_hash[MAX_KEY_HASH]; /* db.c */


/* Super-AT command:

FOR ALL <action>
FOR MORTALS <action>
FOR GODS <action>
FOR MOBS <action>
FOR EVERYWHERE <action>


Executes action several times, either on ALL players (not including yourself),
MORTALS (including trusted characters), GODS (characters with level higher than
L_HERO), MOBS (Not recommended) or every room (not recommended either!)

If you insert a # in the action, it will be replaced by the name of the target.

If # is a part of the action, the action will be executed for every target
in game. If there is no #, the action will be executed for every room containg
at least one target, but only once per room. # cannot be used with FOR EVERY-
WHERE. # can be anywhere in the action.

Example: 

FOR ALL SMILE -> you will only smile once in a room with 2 players.
FOR ALL TWIDDLE # -> In a room with A and B, you will twiddle A then B.

Destroying the characters this command acts upon MAY cause it to fail. Try to
avoid something like FOR MOBS PURGE (although it actually works at my MUD).

FOR MOBS TRANS 3054 (transfer ALL the mobs to Midgaard temple) does NOT work
though :)

The command works by transporting the character to each of the rooms with 
target in them. Private rooms are not violated.

*/

/* Expand the name of a character into a string that identifies THAT
   character within a room. E.g. the second 'guard' -> 2. guard
*/   
const char * name_expand (CHAR_DATA *ch)
{
	int count = 1;
	CHAR_DATA *rch;
	char name[MAX_INPUT_LENGTH]; /*  HOPEFULLY no mob has a name longer than THAT */

	static char outbuf[MAX_INPUT_LENGTH];	
	
	if (!IS_NPC(ch))
		return ch->name;
		
	one_argument (ch->name, name); /* copy the first word into name */
	
	if (!name[0]) /* weird mob .. no keywords */
	{
		strcpy (outbuf, ""); /* Do not return NULL, just an empty buffer */
		return outbuf;
	}
	
	/* ->people changed to ->first_person -- TRI */	
	for (rch = ch->in_room->first_person; rch && (rch != ch);rch = 
	    rch->next_in_room)
		if (is_name (name, rch->name))
			count++;
			

	sprintf (outbuf, "%d.%s", count, name);
	return outbuf;
}


void do_for (CHAR_DATA *ch, char *argument)
{
	char range[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	bool fGods = FALSE, fMortals = FALSE, fMobs = FALSE, fEverywhere = FALSE, found;
	ROOM_INDEX_DATA *room, *old_room;
	CHAR_DATA *p, *p_prev;  /* p_next to p_prev -- TRI */
	int i;
	ZONE_DATA *in_zone;
	
	argument = one_argument (argument, range);
	
	if (!range[0] || !argument[0]) /* invalid usage? */
	{
		do_help (ch, "for");
		return;
	}
	
	if (!str_prefix("quit", argument))
	{
		send_to_char ("Are you trying to crash the MUD or something?\n\r",ch);
		return;
	}
	
	
	if (!str_cmp (range, "all"))
	{
		fMortals = TRUE;
		fGods = TRUE;
	}
	else if (!str_cmp (range, "gods"))
		fGods = TRUE;
	else if (!str_cmp (range, "mortals"))
		fMortals = TRUE;
	else if (!str_cmp (range, "mobs"))
		fMobs = TRUE;
	else if (!str_cmp (range, "everywhere"))
		fEverywhere = TRUE;
	else
		do_help (ch, "for"); /* show syntax */

	/* do not allow # to make it easier */		
	if (fEverywhere && strchr (argument, '#'))
	{
		send_to_char ("Cannot use FOR EVERYWHERE with the # thingie.\n\r",ch);
		return;
	}
		
	if (strchr (argument, '#')) /* replace # ? */
	{ 
		/* char_list - last_char, p_next - gch_prev -- TRI */
		for (p = last_char; p ; p = p_prev )
		{
			p_prev = p->prev;  /* TRI */
		/*	p_next = p->next; */ /* In case someone DOES try to AT MOBS SLAY # */
			found = FALSE;
			
			if (!(p->in_room) || room_is_private(p->in_room) || (p == ch))
				continue;
			
			if (IS_NPC(p) && fMobs)
				found = TRUE;
			else if (!IS_NPC(p) && p->level >= LEVEL_IMMORTAL && fGods)
				found = TRUE;
			else if (!IS_NPC(p) && p->level < LEVEL_IMMORTAL && fMortals)
				found = TRUE;

			/* It looks ugly to me.. but it works :) */				
			if (found) /* p is 'appropriate' */
			{
				char *pSource = argument; /* head of buffer to be parsed */
				char *pDest = buf; /* parse into this */
				
				while (*pSource)
				{
					if (*pSource == '#') /* Replace # with name of target */
					{
						const char *namebuf = name_expand (p);
						
						if (namebuf) /* in case there is no mob name ?? */
							while (*namebuf) /* copy name over */
								*(pDest++) = *(namebuf++);

						pSource++;
					}
					else
						*(pDest++) = *(pSource++);
				} /* while */
				*pDest = '\0'; /* Terminate */
				
				/* Execute */
				old_room = ch->in_room;
				char_from_room (ch);
				char_to_room (ch,p->in_room);
				interpret (ch, buf);
				char_from_room (ch);
				char_to_room (ch,old_room);
				
			} /* if found */
		} /* for every char */
	}
	else /* just for every room with the appropriate people in it */
	{
	for( in_zone=first_zone;in_zone;in_zone=in_zone->next)
	{
		for (i = 0; i < MAX_KEY_HASH; i++) /* run through all the buckets */
			for (room = in_zone->room_index_hash[i] ; room ;room = room->next)
			{
				found = FALSE;
				
				/* Anyone in here at all? */
				if (fEverywhere) /* Everywhere executes always */
					found = TRUE;
				else if (!room->first_person) /* Skip it if room is empty */
					continue;
				/* ->people changed to first_person -- TRI */
					
				/* Check if there is anyone here of the requried type */
				/* Stop as soon as a match is found or there are no more ppl in room */
				/* ->people to ->first_person -- TRI */
				for (p = room->first_person; p && !found; p = p->next_in_room)
				{

					if (p == ch) /* do not execute on oneself */
						continue;
						
					if (IS_NPC(p) && fMobs)
						found = TRUE;
					else if (!IS_NPC(p) && (p->level >= LEVEL_IMMORTAL) && fGods)
						found = TRUE;
					else if (!IS_NPC(p) && (p->level <= LEVEL_IMMORTAL) && fMortals)
						found = TRUE;
				} /* for everyone inside the room */
						
				if (found && !room_is_private(room)) /* Any of the required type here AND room not private? */
				{
					/* This may be ineffective. Consider moving character out of old_room
					   once at beginning of command then moving back at the end.
					   This however, is more safe?
					*/
				
					old_room = ch->in_room;
					char_from_room (ch);
					char_to_room (ch, room);
					interpret (ch, argument);
					char_from_room (ch);
					char_to_room (ch, old_room);
				} /* if found */
			} /* for every room in a bucket */
		} /* for in_zone */
	} /* if strchr */
} /* do_for */

void save_sysdata  args( ( SYSTEM_DATA sys ) );

void do_cset( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_STRING_LENGTH];
  sh_int level;

  set_char_color( AT_IMMORT, ch );

  if (argument[0] == '\0')
  {
    ch_printf(ch, "Authorization ( 1= on 0=off): %d Rent Ratio %d\n\r",
	    sysdata.WAIT_FOR_AUTH,sysdata.rent_ratio);
    ch_printf(ch, " ** Max HP: %d    Max Mana: %d    Max QP Award: %d\n\r",
	    sysdata.max_hp,sysdata.max_mana,sysdata.maxaward );
    ch_printf(ch, "Mail:\n\r  Read all mail: %d. Read mail for free: %d. Write mail for free: %d.\n\r",
	    sysdata.read_all_mail, sysdata.read_mail_free, sysdata.write_mail_free );
    ch_printf(ch, "  Take all mail: %d.\n\r",
	    sysdata.take_others_mail);
    ch_printf(ch, "  IMC board vnum: %d.\n\r",
	    sysdata.imc_mail_vnum);
    ch_printf(ch, "Channels:\n\r  Muse: %d. Think: %d. Log: %d. Build: %d.\n\r",
 	    sysdata.muse_level, sysdata.think_level, sysdata.log_level, 
	    sysdata.build_level);
    ch_printf(ch, "Building:\n\r  Prototype modification: %d.  Player msetting: %d.\n\r",
	    sysdata.level_modify_proto, sysdata.level_mset_player );
    ch_printf(ch, "Guilds:\n\r  Overseer: %s.  Advisor: %s.\n\r", 
            sysdata.guild_overseer, sysdata.guild_advisor );
    ch_printf(ch, "System Sounds: EnterSound: %s ConnectSound %s\n\r",
	    sysdata.entersound,sysdata.connectsound );
    ch_printf(ch, "Other:\n\r  Force on players: %d.  ", sysdata.level_forcepc);
    ch_printf(ch, "Private room override: %d.\n\r", sysdata.level_override_private);
    ch_printf(ch, "  Penalty to regular stun chance: %d.  ", sysdata.stun_regular );
    ch_printf(ch, "Penalty to stun plr vs. plr: %d.\n\r", sysdata.stun_plr_vs_plr );
    ch_printf(ch, "  Percent damage plr vs. plr: %3d.  ", sysdata.dam_plr_vs_plr );
    ch_printf(ch, "Percent damage plr vs. mob: %d.\n\r", sysdata.dam_plr_vs_mob );
    ch_printf(ch, "  Percent damage mob vs. plr: %3d.  ", sysdata.dam_mob_vs_plr );
    ch_printf(ch, "Percent damage mob vs. mob: %d.\n\r", sysdata.dam_mob_vs_mob );
    ch_printf(ch, "  Get object without take flag: %d.  ", sysdata.level_getobjnotake);
    ch_printf(ch, "Autosave frequency (minutes): %d.\n\r", sysdata.save_frequency );
    ch_printf(ch, "  Save flags: %s\n\r", flag_string( sysdata.save_flags, save_flag ) );
    return;
  }

  argument = one_argument( argument, arg );

  if (!str_cmp(arg, "help"))
  {
     do_help(ch, "controls");
     return;
  }

  if (!str_cmp(arg, "save"))
  {
     save_sysdata(sysdata);
     return;
  }

  if (!str_cmp(arg, "saveflag"))
  {
	int x = get_saveflag( argument );

	if ( x == -1 )
	    send_to_char( "Not a save flag.\n\r", ch );
	else
	{
	    TOGGLE_BIT( sysdata.save_flags, 1 << x );
	    send_to_char( "Ok.\n\r", ch );
	}
	return;
  }

  if (!str_prefix( arg, "entersound" ) )
  {
    STRFREE( sysdata.entersound );
    sysdata.entersound = str_dup( argument );
    send_to_char("Ok.\n\r", ch);      
    return;
  }
  if (!str_prefix( arg, "connectsound" ) )
  {
    STRFREE( sysdata.connectsound );
    sysdata.connectsound = str_dup( argument );
    send_to_char("Ok.\n\r", ch);      
    return;
  }
  if (!str_prefix( arg, "guild_overseer" ) )
  {
    STRFREE( sysdata.guild_overseer );
    sysdata.guild_overseer = str_dup( argument );
    send_to_char("Ok.\n\r", ch);      
    return;
  }
  if (!str_prefix( arg, "guild_advisor" ) )
  {
    STRFREE( sysdata.guild_advisor );
    sysdata.guild_advisor = str_dup( argument );
    send_to_char("Ok.\n\r", ch);      
    return;
  }

  level = (sh_int) atoi(argument);

  if (!str_prefix( arg, "savefrequency" ) )
  {
    sysdata.save_frequency = level;
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (!str_cmp(arg, "auth"))
  {
    if ((level > 1) || (level < 0))
     {
      send_to_char("Invalid Number, 1=on 0=off(0=AutoAuth)\n\r",ch);
      return;
     }

   sysdata.WAIT_FOR_AUTH = level;
   send_to_char("Ok.\n\r",ch);
   return;
  }

  if (!str_cmp(arg, "maxhp"))
  {
    if ((level > 30000) || (level <= 0))
     {
      send_to_char("Invalid Number, 1 - 30000\n\r",ch);
      return;
     }

   sysdata.max_hp = level;
   send_to_char("Ok.\n\r",ch);
   return;
  }

  if (!str_cmp(arg, "maxmana"))
  {
    if ((level > 30000) || (level <= 0))
     {
      send_to_char("Invalid Number, 1 - 30000\n\r",ch);
      return;
     }

   sysdata.max_mana = level;
   send_to_char("Ok.\n\r",ch);
   return;
  }

  if (!str_cmp(arg, "maxaward"))
  {
    if ((level > 30000) || (level <= 0))
     {
      send_to_char("Invalid Number, 1 - 30000\n\r",ch);
      return;
     }
    
    sysdata.maxaward = level;
    send_to_char("Ok.\n\r",ch);
    return;
  }

  if (!str_cmp(arg, "rent"))
  {
   sysdata.rent_ratio = level;
   send_to_char("Ok.\n\r",ch);
   return;
  }

  if (!str_cmp(arg, "plrmsetting"))
  {

   sysdata.level_mset_player = level;
   send_to_char("Ok.\n\r",ch);
   return;
  }

  if (!str_cmp(arg, "stun"))
  {
    sysdata.stun_regular = level;
    send_to_char("Ok.\n\r", ch);      
    return;
  }

  if (!str_cmp(arg, "stun_pvp"))
  {
    sysdata.stun_plr_vs_plr = level;
    send_to_char("Ok.\n\r", ch);      
    return;
  }

  if (!str_cmp(arg, "dam_pvp"))
  {
    sysdata.dam_plr_vs_plr = level;
    send_to_char("Ok.\n\r", ch);      
    return;
  }

  if (!str_cmp(arg, "get_notake"))
  {
    sysdata.level_getobjnotake = level;
    send_to_char("Ok.\n\r", ch);      
    return;
  }

  if (!str_cmp(arg, "dam_pvm"))
  {
    sysdata.dam_plr_vs_mob = level;
    send_to_char("Ok.\n\r", ch);      
    return;
  }

  if (!str_cmp(arg, "dam_mvp"))
  {
    sysdata.dam_mob_vs_plr = level;
    send_to_char("Ok.\n\r", ch);      
    return;
  }

  if (!str_cmp(arg, "dam_mvm"))
  {
    sysdata.dam_mob_vs_mob = level;
    send_to_char("Ok.\n\r", ch);      
    return;
  }

  if (level < 0 || level > MAX_LEVEL)
  {
    send_to_char("Invalid value for new control.\n\r", ch);
    return;
  }

  if (!str_cmp(arg, "read_all"))
  {
    sysdata.read_all_mail = level;
    send_to_char("Ok.\n\r", ch);      
    return;
  }

  if (!str_cmp(arg, "read_free"))
  {
    sysdata.read_mail_free = level;
    send_to_char("Ok.\n\r", ch);      
    return;
  }
  if (!str_cmp(arg, "write_free"))
  {
    sysdata.write_mail_free = level;
    send_to_char("Ok.\n\r", ch);      
    return;
  }
  if (!str_cmp(arg, "take_all"))
  {
    sysdata.take_others_mail = level;
    send_to_char("Ok.\n\r", ch);      
    return;
  }
  if (!str_cmp(arg, "muse"))
  {
    sysdata.muse_level = level;
    send_to_char("Ok.\n\r", ch);      
    return;
  }
  if (!str_cmp(arg, "think"))
  {
    sysdata.think_level = level;
    send_to_char("Ok.\n\r", ch);      
    return;
  }
  if (!str_cmp(arg, "log"))
  {
    sysdata.log_level = level;
    send_to_char("Ok.\n\r", ch);      
    return;
  }
  if (!str_cmp(arg, "build"))
  {
    sysdata.build_level = level;
    send_to_char("Ok.\n\r", ch);      
    return;
  }
  if (!str_cmp(arg, "proto_modify"))
  {
    sysdata.level_modify_proto = level;
    send_to_char("Ok.\n\r", ch);      
    return;
  }
  if (!str_cmp(arg, "override_private"))
  {
    sysdata.level_override_private = level;
    send_to_char("Ok.\n\r", ch);      
    return;
  }
  if (!str_cmp(arg, "forcepc"))
  {
    sysdata.level_forcepc = level;
    send_to_char("Ok.\n\r", ch);      
    return;
  }
  if (!str_cmp(arg, "mset_player"))
  {
    sysdata.level_mset_player = level;
    send_to_char("Ok.\n\r", ch);      
    return;
  }
  if (!str_cmp(arg, "imc_mail_vnum"))
  {
    sysdata.imc_mail_vnum = level;
    send_to_char("Ok.\n\r", ch);
    return;
  }
  else
  {
    send_to_char("Invalid argument.\n\r", ch);
    return;
  }
}

void get_reboot_string(void)
{
  sprintf(reboot_time, "%s", asctime(new_boot_time));
}


void do_orange( CHAR_DATA *ch, char *argument )
{
  send_to_char( "Function under construction.\n\r", ch );
  return;
}

void do_mrange( CHAR_DATA *ch, char *argument )
{
  send_to_char( "Function under construction.\n\r", ch );
  return;
}

void do_hell( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  sh_int time;
  bool h_d = FALSE;
  struct tm *tms;
  
  argument = one_argument(argument, arg);
  if ( !*arg )
  {
    send_to_char( "Hell who, and for how long?\n\r", ch );
    return;
  }
  if ( !(victim = get_char_world(ch, arg)) || IS_NPC(victim) )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }
  if ( IS_IMMORTAL(victim) )
  {
    send_to_char( "There is no point in helling an immortal.\n\r", ch );
    return;
  }
  if ( victim->pcdata->release_date != 0 )
  {
    ch_printf(ch, "They are already in hell until %24.24s, by %s.\n\r",
            ctime(&victim->pcdata->release_date), victim->pcdata->helled_by);
    return;
  }
  argument = one_argument(argument, arg);
  if ( !*arg || !is_number(arg) )
  {
    send_to_char( "Hell them for how long?\n\r", ch );
    return;
  }
  time = atoi(arg);
  if ( time <= 0 )
  {
    send_to_char( "You cannot hell for zero or negative time.\n\r", ch );
    return;
  }
  argument = one_argument(argument, arg);
  if ( !*arg || !str_prefix(arg, "hours") )
    h_d = TRUE;
  else if ( str_prefix(arg, "days") )
  {
    send_to_char( "Is that value in hours or days?\n\r", ch );
    return;
  }
  else if ( time > 30 )
  {
    send_to_char( "You may not hell a person for more than 30 days at a time.\n\r", ch );
    return;
  }
  tms = localtime(&current_time);
  if ( h_d )
    tms->tm_hour += time;
  else
    tms->tm_mday += time;
  victim->pcdata->release_date = mktime(tms);
  victim->pcdata->helled_by = STRALLOC(ch->name);
  ch_printf(ch, "%s will be released from hell at %24.24s.\n\r", victim->name,
          ctime(&victim->pcdata->release_date));
  act(AT_MAGIC, "$n disappears in a cloud of hellish light.", victim, NULL, ch, TO_NOTVICT);
  char_from_room(victim);
  char_to_room(victim, get_room_index(8,1));
  act(AT_MAGIC, "$n appears in a could of hellish light.", victim, NULL, ch, TO_NOTVICT);
  do_look(victim, "auto");
  ch_printf(victim, "The immortals are not pleased with your actions.\n\r"
          "You shall remain in hell for %d %s%s.\n\r", time,
          (h_d ? "hour" : "day"), (time == 1 ? "" : "s"));
  save_char_obj(victim);	/* used to save ch, fixed by Thoric 09/17/96 */
  return;
}

void do_unhell( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *location;
  
  argument = one_argument(argument, arg);
  if ( !*arg )
  {
    send_to_char( "Unhell whom..?\n\r", ch );
    return;
  }

//    location = ch->in_room;
//  ch->in_room = get_room_index(8,1);
     victim = get_char_room(ch, arg);

//    char_from_room(ch);
//    char_to_room(ch,location);            /* The case of unhell self, etc. */

  if ( !victim || IS_NPC(victim) || victim->in_room->vnum != 8 )
  {
    send_to_char( "No one like that is in hell.\n\r", ch );
    return;
  }
  if ( victim->pcdata->clan )
    location = get_room_index(victim->pcdata->clan->recall,1);
  else
    location = get_room_index(ROOM_VNUM_TEMPLE,1);
  if ( !location )
    location = ch->in_room;
  MOBtrigger = FALSE;
  act( AT_MAGIC, "$n disappears in a cloud of godly light.", victim, NULL, ch, TO_NOTVICT );
  char_from_room(victim);
  char_to_room(victim, location);
  send_to_char( "The gods have smiled on you and released you from hell early!\n\r", victim );
  do_look(victim, "auto");
  send_to_char( "They have been released.\n\r", ch );

  if ( victim->pcdata->helled_by )
  {
    if( str_cmp(ch->name, victim->pcdata->helled_by) )
      ch_printf(ch, "(You should probably write a note to %s, explaining the early release.)\n\r",
            victim->pcdata->helled_by);
    STRFREE(victim->pcdata->helled_by);
    victim->pcdata->helled_by = NULL;
  }

  MOBtrigger = FALSE;
  act( AT_MAGIC, "$n appears in a cloud of godly light.", victim, NULL, ch, TO_NOTVICT );
  victim->pcdata->release_date = 0;
  save_char_obj(victim);
  return;
}

/* Vnum search command by Swordbearer */
void do_vsearch( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    bool found = FALSE;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    int obj_counter = 1;
    int argi;
    ZONE_DATA *in_zone;
 
    one_argument( argument, arg );
 
    if( arg[0] == '\0' )
    {
        send_to_char( "Syntax:  vsearch <vnum>.\n\r", ch );
        return;
    }
 
    set_pager_color( AT_PLAIN, ch );
    argi=atoi(arg);
    if (argi<0 && argi>32681)
    {
	send_to_char( "Vnum out of range.\n\r", ch);
	return;
    }

for( in_zone = first_zone; in_zone; in_zone = in_zone->next )
{
    for ( obj = in_zone->first_obj; obj != NULL; obj = obj->next )
    {
	if ( !can_see_obj( ch, obj ) || !( argi == obj->pIndexData->vnum ))
	  continue;
 
	found = TRUE;
	for ( in_obj = obj; in_obj->in_obj != NULL;
	  in_obj = in_obj->in_obj );

	if ( in_obj->carried_by != NULL )
	  pager_printf( ch, "[%2] %s carried by %s.\n\r", 
		obj_counter,
		obj_short(obj),
		PERS( in_obj->carried_by, ch ) );
	else           
	  pager_printf( ch, "[%2d] [%-5d] %s in %s.\n\r", obj_counter,
		( ( in_obj->in_room ) ? in_obj->in_room->vnum : 0 ),
		obj_short(obj), ( in_obj->in_room == NULL ) ?
		"somewhere" : in_obj->in_room->name );

	obj_counter++;
    }
}
    if ( !found )
      send_to_char( "Nothing like that in hell, earth, or heaven.\n\r" , ch );

    return;
}

/* 
 * Simple function to let any imm make any player instantly sober.
 * Saw no need for level restrictions on this.
 * Written by Narn, Apr/96 
 */
void do_sober( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char arg1 [MAX_INPUT_LENGTH];

  smash_tilde( argument );
  argument = one_argument( argument, arg1 );
  if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( IS_NPC( victim ) )
  {
    send_to_char( "Not on mobs.\n\r", ch );
    return;    
  }

  if ( victim->pcdata ) 
    victim->pcdata->condition[COND_DRUNK] = 0;
  send_to_char( "Ok.\n\r", ch );
  send_to_char( "You feel sober again.\n\r", victim );
  return;    
}


/*
 * Free a social structure					-Thoric
 */
void free_social( SOCIALTYPE *social )
{
    if ( social->name )
      DISPOSE( social->name );
    if ( social->char_no_arg )
      DISPOSE( social->char_no_arg );
    if ( social->others_no_arg )
      DISPOSE( social->others_no_arg );
    if ( social->char_found )
      DISPOSE( social->char_found );
    if ( social->others_found )
      DISPOSE( social->others_found );
    if ( social->vict_found )
      DISPOSE( social->vict_found );
    if ( social->char_auto )
      DISPOSE( social->char_auto );
    if ( social->others_auto )
      DISPOSE( social->others_auto );
    DISPOSE( social );
}

/*
 * Remove a social from it's hash index				-Thoric
 */
void unlink_social( SOCIALTYPE *social )
{
    SOCIALTYPE *tmp, *tmp_next;
    int hash;

    if ( !social )
    {
	bug( "Unlink_social: NULL social", 0 );
	return;
    }

    if ( social->name[0] < 'a' || social->name[0] > 'z' )
	hash = 0;
    else
	hash = (social->name[0] - 'a') + 1;

    if ( social == (tmp=social_index[hash]) )
    {
	social_index[hash] = tmp->next;
	return;
    }
    for ( ; tmp; tmp = tmp_next )
    {
	tmp_next = tmp->next;
	if ( social == tmp_next )
	{
	    tmp->next = tmp_next->next;
	    return;
	}
    }
}

/*
 * Add a social to the social index table			-Thoric
 * Hashed and insert sorted
 */
void add_social( SOCIALTYPE *social )
{
    int hash, x;
    SOCIALTYPE *tmp, *prev;

    if ( !social )
    {
	bug( "Add_social: NULL social", 0 );
	return;
    }

    if ( !social->name )
    {
	bug( "Add_social: NULL social->name", 0 );
	return;
    }

    if ( !social->char_no_arg )
    {
	bug( "Add_social: NULL social->char_no_arg", 0 );
	return;
    }

    /* make sure the name is all lowercase */
    for ( x = 0; social->name[x] != '\0'; x++ )
	social->name[x] = LOWER(social->name[x]);

    if ( social->name[0] < 'a' || social->name[0] > 'z' )
	hash = 0;
    else
	hash = (social->name[0] - 'a') + 1;

    if ( (prev = tmp = social_index[hash]) == NULL )
    {
	social->next = social_index[hash];
	social_index[hash] = social;
	return;
    }

    for ( ; tmp; tmp = tmp->next )
    {
	if ( (x=strcmp(social->name, tmp->name)) == 0 )
	{
	    bug( "Add_social: trying to add duplicate name to bucket %d", hash );
	    free_social( social );
	    return;
	}
	else
	if ( x < 0 )
	{
	    if ( tmp == social_index[hash] )
	    {
		social->next = social_index[hash];
		social_index[hash] = social;
		return;
	    }
	    prev->next = social;
	    social->next = tmp;
	    return;
	}
	prev = tmp;
    }

    /* add to end */
    prev->next = social;
    social->next = NULL;
    return;
}

/*
 * Social editor/displayer/save/delete				-Thoric
 */
void do_sedit( CHAR_DATA *ch, char *argument )
{
    SOCIALTYPE *social;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    set_char_color( AT_SOCIAL, ch );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Syntax: sedit <social> [field]\n\r", ch );
	send_to_char( "Syntax: sedit <social> create\n\r", ch );
	if ( get_trust(ch) > LEVEL_GOD )
	    send_to_char( "Syntax: sedit <social> delete\n\r", ch );
	if ( get_trust(ch) > LEVEL_FATE )
	    send_to_char( "Syntax: sedit <save>\n\r", ch );
	send_to_char( "\n\rField being one of:\n\r", ch );
	send_to_char( "  cnoarg onoarg cfound ofound vfound cauto oauto\n\r", ch );
	return;
    }

    if ( get_trust(ch) > LEVEL_FATE && !str_cmp( arg1, "save" ) )
    {
	save_socials();
	send_to_char( "Saved.\n\r", ch );
	return;
    }

    social = find_social(ch, arg1, FALSE );

    if ( !str_cmp( arg2, "create" ) )
    {
	if ( social )
	{
	    send_to_char( "That social already exists!\n\r", ch );
	    return;
	}
	CREATE( social, SOCIALTYPE, 1 );
	social->name = str_dup( arg1 );
	sprintf( arg2, "You %s.", arg1 );
	social->char_no_arg = str_dup( arg2 );
	add_social( social );
	send_to_char( "Social added.\n\r", ch );
	return;
    }

    if ( !social )
    {
	send_to_char( "Social not found.\n\r", ch );
	return;
    }

    if ( arg2[0] == '\0' || !str_cmp( arg2, "show" ) )
    {
	ch_printf( ch, "Social: %s\n\r\n\rCNoArg: %s\n\r",
	    social->name,	social->char_no_arg );
	ch_printf( ch, "ONoArg: %s\n\rCFound: %s\n\rOFound: %s\n\r",
	    social->others_no_arg	? social->others_no_arg	: "(not set)",
	    social->char_found		? social->char_found	: "(not set)",
	    social->others_found	? social->others_found	: "(not set)" );
	ch_printf( ch, "VFound: %s\n\rCAuto : %s\n\rOAuto : %s\n\r",
	    social->vict_found	? social->vict_found	: "(not set)",
	    social->char_auto	? social->char_auto	: "(not set)",
	    social->others_auto	? social->others_auto	: "(not set)" );
	return;
    }

    if ( get_trust(ch) > LEVEL_GOD && !str_cmp( arg2, "delete" ) )
    {
	unlink_social( social );
	free_social( social );
	send_to_char( "Deleted.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "cnoarg" ) )
    {
	if ( argument[0] == '\0' || !str_cmp( argument, "clear" ) )
	{
	    send_to_char( "You cannot clear this field.  It must have a message.\n\r", ch );
	    return;
	}
	if ( social->char_no_arg )
	    DISPOSE( social->char_no_arg );
	social->char_no_arg = str_dup( argument );
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "onoarg" ) )
    {
	if ( social->others_no_arg )
	    DISPOSE( social->others_no_arg );
	if ( argument[0] != '\0' && str_cmp( argument, "clear" ) )
	    social->others_no_arg = str_dup( argument );
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "cfound" ) )
    {
	if ( social->char_found )
	    DISPOSE( social->char_found );
	if ( argument[0] != '\0' && str_cmp( argument, "clear" ) )
	    social->char_found = str_dup( argument );
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "ofound" ) )
    {
	if ( social->others_found )
	    DISPOSE( social->others_found );
	if ( argument[0] != '\0' && str_cmp( argument, "clear" ) )
	    social->others_found = str_dup( argument );
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "vfound" ) )
    {
	if ( social->vict_found )
	    DISPOSE( social->vict_found );
	if ( argument[0] != '\0' && str_cmp( argument, "clear" ) )
	    social->vict_found = str_dup( argument );
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "cauto" ) )
    {
	if ( social->char_auto )
	    DISPOSE( social->char_auto );
	if ( argument[0] != '\0' && str_cmp( argument, "clear" ) )
	    social->char_auto = str_dup( argument );
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "oauto" ) )
    {
	if ( social->others_auto )
	    DISPOSE( social->others_auto );
	if ( argument[0] != '\0' && str_cmp( argument, "clear" ) )
	    social->others_auto = str_dup( argument );
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( get_trust(ch) > LEVEL_GOD && !str_cmp( arg2, "name" ) )
    {
	bool relocate;

	one_argument( argument, arg1 );
	if ( arg1[0] == '\0' )
	{
	    send_to_char( "Cannot clear name field!\n\r", ch );
	    return;
	}
	if ( arg1[0] != social->name[0] )
	{
	    unlink_social( social );
	    relocate = TRUE;
	}
	else
	    relocate = FALSE;
	if ( social->name )
	    DISPOSE( social->name );
	social->name = str_dup( arg1 );
	if ( relocate )
	    add_social( social );
	send_to_char( "Done.\n\r", ch );
	return;
    }

    /* display usage message */
    do_sedit( ch, "" );
}

/*
 * Free a command structure					-Thoric
 */
void free_command( CMDTYPE *command )
{
    if ( command->name )
      DISPOSE( command->name );
    DISPOSE( command );
}

/*
 * Remove a command from it's hash index			-Thoric
 */
void unlink_command( CMDTYPE *command )
{
    CMDTYPE *tmp, *tmp_next;
    int hash;

    if ( !command )
    {
	bug( "Unlink_command NULL command", 0 );
	return;
    }

    hash = command->name[0]%126;

    if ( command == (tmp=command_hash[hash]) )
    {
	command_hash[hash] = tmp->next;
	return;
    }
    for ( ; tmp; tmp = tmp_next )
    {
	tmp_next = tmp->next;
	if ( command == tmp_next )
	{
	    tmp->next = tmp_next->next;
	    return;
	}
    }
}

/*
 * Add a command to the command hash table			-Thoric
 */
void add_command( CMDTYPE *command )
{
    int hash, x;
    CMDTYPE *tmp, *prev;

    if ( !command )
    {
	bug( "Add_command: NULL command", 0 );
	return;
    }

    if ( !command->name )
    {
	bug( "Add_command: NULL command->name", 0 );
	return;
    }

    if ( !command->do_fun )
    {
	bug( "Add_command: NULL command->do_fun", 0 );
	return;
    }

    /* make sure the name is all lowercase */
    for ( x = 0; command->name[x] != '\0'; x++ )
	command->name[x] = LOWER(command->name[x]);

    hash = command->name[0] % 126;

    if ( (prev = tmp = command_hash[hash]) == NULL )
    {
	command->next = command_hash[hash];
	command_hash[hash] = command;
	return;
    }

    /* add to the END of the list */
    for ( ; tmp; tmp = tmp->next )
	if ( !tmp->next )
	{
	    tmp->next = command;
	    command->next = NULL;
	}
    return;
}

/*
 * Command editor/displayer/save/delete				-Thoric
 */
void do_cedit( CHAR_DATA *ch, char *argument )
{
    CMDTYPE *command;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    set_char_color( AT_IMMORT, ch );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Syntax: cedit save\n\r", ch );
	if ( get_trust(ch) > LEVEL_COUNCIL )
	{
	    send_to_char( "Syntax: cedit <command> create [code]\n\r", ch );
	    send_to_char( "Syntax: cedit <command> delete\n\r", ch );
	    send_to_char( "Syntax: cedit <command> show\n\r", ch );
	    send_to_char( "Syntax: cedit <command> [field]\n\r", ch );
	    send_to_char( "\n\rField being one of:\n\r", ch );
	    send_to_char( "  level position log code\n\r", ch );
	}
	return;
    }

    if ( get_trust(ch) > LEVEL_COUNCIL && !str_cmp( arg1, "save" ) )
    {
	save_commands();
	send_to_char( "Saved.\n\r", ch );
	return;
    }

    command = find_command( arg1 );

    if ( get_trust(ch) > LEVEL_COUNCIL && !str_cmp( arg2, "create" ) )
    {
	if ( command )
	{
	    send_to_char( "That command already exists!\n\r", ch );
	    return;
	}
	CREATE( command, CMDTYPE, 1 );
	command->name = str_dup( arg1 );
	command->level = get_trust(ch);
	if ( *argument )
	  one_argument(argument, arg2);
	else
	  sprintf( arg2, "do_%s", arg1 );
	command->do_fun = skill_function( arg2 );
	add_command( command );
	send_to_char( "Command added.\n\r", ch );
	if ( command->do_fun == skill_notfound )
	  ch_printf( ch, "Code %s not found.  Set to no code.\n\r", arg2 );
	return;
    }

    if ( !command )
    {
	send_to_char( "Command not found.\n\r", ch );
	return;
    }
    else
    if ( command->level > get_trust(ch) )
    {
	send_to_char( "You cannot touch this command.\n\r", ch );
	return;
    }

    if ( arg2[0] == '\0' || !str_cmp( arg2, "show" ) )
    {
	ch_printf( ch, "Command:  %s\n\rLevel:    %d\n\rPosition: %d\n\rLog:      %d\n\rCode:     %s\n\r",
	    command->name, command->level, command->position, command->log,
	    skill_name(command->do_fun) );
	if ( command->userec.num_uses )
	  send_timer(&command->userec, ch);
	return;
    }

    if ( get_trust(ch) <= LEVEL_COUNCIL )
    {
	do_cedit( ch, "" );
	return;
    }

    if ( !str_cmp( arg2, "delete" ) )
    {
	unlink_command( command );
	free_command( command );
	send_to_char( "Deleted.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "code" ) )
    {
	DO_FUN *fun = skill_function( argument );
	
	if ( fun == skill_notfound )
	{
	    send_to_char( "Code not found.\n\r", ch );
	    return;
	}
	command->do_fun = fun;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "level" ) )
    {
	int level = atoi( argument );

	if ( level < 0 || level > get_trust(ch) )
	{
	    send_to_char( "Level out of range.\n\r", ch );
	    return;
	}
	command->level = level;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "log" ) )
    {
	int log = atoi( argument );

	if ( log < 0 || log > LOG_COMM )
	{
	    send_to_char( "Log out of range.\n\r", ch );
	    return;
	}
	command->log = log;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "position" ) )
    {
	int position = atoi( argument );

	if ( position < 0 || position > POS_DRAG )
	{
	    send_to_char( "Position out of range.\n\r", ch );
	    return;
	}
	command->position = position;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "name" ) )
    {
	bool relocate;

	one_argument( argument, arg1 );
	if ( arg1[0] == '\0' )
	{
	    send_to_char( "Cannot clear name field!\n\r", ch );
	    return;
	}
	if ( arg1[0] != command->name[0] )
	{
	    unlink_command( command );
	    relocate = TRUE;
	}
	else
	    relocate = FALSE;
	if ( command->name )
	    DISPOSE( command->name );
	command->name = str_dup( arg1 );
	if ( relocate )
	    add_command( command );
	send_to_char( "Done.\n\r", ch );
	return;
    }

    /* display usage message */
    do_cedit( ch, "" );
}

/*
 * Display class information					-Thoric
 */
void do_showclass( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    struct class_type *class;
    int cl, low, hi;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    set_char_color( AT_IMMORT, ch );
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Syntax: showclass <class> [level range]\n\r", ch );
	return;
    }
    if ( is_number(arg1) && (cl=atoi(arg1)) >= 0 && cl < MAX_CLASS )
	class = class_table[cl];
    else
    {
	class = NULL;
	for ( cl = 0; cl < MAX_CLASS && class_table[cl]; cl++ )
	    if ( !str_cmp(class_table[cl]->who_name, arg1) )
	    {
		class = class_table[cl];
		break;
	    }
    }
    if ( !class )
    {
	send_to_char( "No such class.\n\r", ch );
	return;
    }
    set_pager_color( AT_IMMORT, ch );
    pager_printf( ch, "Class: %s\n\rPrime Attribute: %-14s Weapon: %-5d  Guild:   %d\n\r"
		   "Max Skill Adept: %-3d            Thac0:  %-5d  Thac32:  %d\n\r"
		   "Hp Min / Hp Max: %d/%-2d           Mana:   %s    ExpBase: %d\n\r",
	class->who_name, affect_loc_name(class->attr_prime), class->weapon, class->guild,
	class->skill_adept, class->thac0_00, class->thac0_32,
	class->hp_min, class->hp_max, class->fMana ? "yes" : "no ", class->exp_base );
    if ( arg2[0] != '\0' )
    {
	int x, y, cnt;

	low = UMIN( 0, atoi(arg2) );
	hi  = URANGE( low, atoi(argument), MAX_LEVEL );
	for ( x = low; x <= hi; x++ )
	{
	    set_pager_color( AT_LBLUE, ch );
	    pager_printf( ch, "Male: %-30s Female: %s\n\r",
		title_table[cl][x][0], title_table[cl][x][1] );
	    cnt = 0;
	    set_pager_color( AT_BLUE, ch );
	    for ( y = gsn_first_spell; y < gsn_top_sn; y++ )
		if ( skill_table[y]->skill_level[cl] == x )
		{
		   pager_printf( ch, "  %-7s %-19s%3d     ",
			skill_tname[skill_table[y]->type],
			skill_table[y]->name, skill_table[y]->skill_adept[cl] );
		   if ( ++cnt % 2 == 0 )
			send_to_pager( "\n\r", ch );
		}
	    if ( cnt % 2 != 0 )
		send_to_pager( "\n\r", ch );
	    send_to_pager( "\n\r", ch );
	}
    }
}

/*
 * Edit class information					-Thoric
 */
void do_setclass( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    struct class_type *class;
    int cl;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    set_char_color( AT_IMMORT, ch );
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Syntax: setclass <class> <field> <value>\n\r",	ch );
	send_to_char( "\n\rField being one of:\n\r",			ch );
	send_to_char( "  name prime weapon guild thac0 thac32\n\r",	ch );
	send_to_char( "  hpmin hpmax mana expbase mtitle ftitle type\n\r", ch );
	return;
    }
    if ( is_number(arg1) && (cl=atoi(arg1)) >= 0 && cl < MAX_CLASS )
	class = class_table[cl];
    else
    {
	class = NULL;
	for ( cl = 0; cl < MAX_CLASS && class_table[cl]; cl++ )
	    if ( !str_cmp(class_table[cl]->who_name, arg1) )
	    {
		class = class_table[cl];
		break;
	    }
    }
    if ( !class )
    {
	send_to_char( "No such class.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "save" ) )
    {
	write_class_file( cl );
	send_to_char( "Saved.\n\r", ch );
	return;
    }
    if ( !str_cmp( arg2, "name" ) )
    {
	STRFREE(class->who_name );
	class->who_name = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	return;
    }
    if ( !str_cmp( arg2, "type" ) )
    {
	if ( !str_cmp(argument, "fighter" ) )
	class->type = 1;
	if ( !str_cmp(argument, "magic" ) )
	class->type = 0;
	if ( !str_cmp(argument, "exclude" ) )
	class->type = 2;
       
        return;
    }
    if ( !str_cmp( arg2, "prime" ) )
    {
	int x = get_atype( argument );

	if ( x < APPLY_STR || (x > APPLY_CON && x != APPLY_LCK) )
	    send_to_char( "Invalid prime attribute!\n\r", ch );
	else
	{
	    class->attr_prime = x;
	    send_to_char( "Done.\n\r", ch );
	}
	return;
    }
    if ( !str_cmp( arg2, "weapon" ) )
    {
	class->weapon = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	return;
    }
    if ( !str_cmp( arg2, "guild" ) )
    {
	class->guild = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	return;
    }
    if ( !str_cmp( arg2, "thac0" ) )
    {
	class->thac0_00 = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	return;
    }
    if ( !str_cmp( arg2, "thac32" ) )
    {
	class->thac0_32 = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	return;
    }
    if ( !str_cmp( arg2, "hpmin" ) )
    {
	class->hp_min = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	return;
    }
    if ( !str_cmp( arg2, "hpmax" ) )
    {
	class->hp_max = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	return;
    }
    if ( !str_cmp( arg2, "mana" ) )
    {
	if ( UPPER(argument[0]) == 'Y' )
	  class->fMana = TRUE;
	else
	  class->fMana = FALSE;
	send_to_char( "Done.\n\r", ch );
	return;
    }
    if ( !str_cmp( arg2, "expbase" ) )
    {
	class->exp_base = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	return;
    }
    if ( !str_cmp( arg2, "mtitle" ) )
    {
	char arg3[MAX_INPUT_LENGTH];
	int x;

	argument = one_argument( argument, arg3 );
	if ( arg3[0] == '\0' || argument[0] == '\0' )
	{
	    send_to_char( "Syntax: setclass <class> mtitle <level> <title>\n\r", ch );
	    return;
	}
	if ( (x=atoi(arg3)) < 0 || x > MAX_LEVEL )
	{
	    send_to_char( "Invalid level.\n\r", ch );
	    return;
	}
	DISPOSE( title_table[cl][x][0] );
	title_table[cl][x-1][0] = str_dup( argument );
    }
    if ( !str_cmp( arg2, "ftitle" ) )
    {
	char arg3[MAX_INPUT_LENGTH];
	int x;

	argument = one_argument( argument, arg3 );
	if ( arg3[0] == '\0' || argument[0] == '\0' )
	{
	    send_to_char( "Syntax: setclass <class> ftitle <level> <title>\n\r", ch );
	    return;
	}
	if ( (x=atoi(arg3)) < 0 || x > MAX_LEVEL )
	{
	    send_to_char( "Invalid level.\n\r", ch );
	    return;
	}
	DISPOSE( title_table[cl][x][1] );
	title_table[cl][x-1][1] = str_dup( argument );
    }
    do_setclass( ch, "" );
}

/*
 * quest point set - TRI
 * syntax is: qpset char give/take amount
 */

void do_qpset( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int amount;
  bool give = TRUE;

  if ( IS_NPC(ch) )
  {
    send_to_char( "Cannot qpset as an NPC.\n\r", ch );
    return;
  }

  if ( get_trust( ch ) < LEVEL_IMMORTAL ) 
  {
    send_to_char( "Huh?\n\r", ch );
    return;
  }
 else if ( str_cmp( ch->pcdata->council_name, "Quest Council" ) 
	&& ( get_trust( ch ) < LEVEL_GOD ) )
  {
    send_to_char( "You must be a member of Quest Council to give or remove qp from players.\n\r", ch );
    return;
  }

  set_char_color( AT_LOG, ch );  
  argument = one_argument( argument, arg );
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );
  amount = atoi( arg3 );

  if ( arg[0] == '\0' || arg2[0] == '\0' || amount <= 0 )
  {
    send_to_char( "Syntax: qpset <character> <give/take> <amount>\n\r", ch );
    send_to_char( "Amount must be a positive number greater than 0.\n\r", ch );
    return;
  }

  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
  {
    send_to_char( "There is no such player currently playing.\n\r", ch );
    return;
  }

  if ( IS_NPC( victim ) )
  {
    send_to_char( "Glory cannot be given to or taken from a "
	"mob.\n\r", ch );
    return;
  }

  if ( nifty_is_name_prefix( arg2, "give" ) )
    give = TRUE;
   else if ( nifty_is_name_prefix( arg2, "take" ) )
    give = FALSE;
   else
    {
      do_qpset( ch, "" );
      return;
    }

  if ( give )
  {
    victim->pcdata->quest_curr += amount;
    victim->pcdata->quest_accum += amount;
    ch_printf( victim, "Your glory has been increased by %d.\n\r", amount );
  }
 else
  {
    victim->pcdata->quest_curr -= amount;
    ch_printf( victim, "Your glory has been decreased by %d.\n\r", amount );
  }

  send_to_char( "Ok.\n\r", ch );
  return;
}
/*
 * Simple number loop command by Callinon
 * Syntax: loop <start> <finish> <command>
 * use a # sign to indicate the number of the loop
 */

void do_loop( CHAR_DATA *ch, char *argument )
{
    int a=0;
    int b=0;
    int c=0;
    int loopnum;
    char arg1[MSL];
    char arg2[MSL];
    char buf[MSL];
    bool use_1;
    bool use_2;
    char spare1[MSL], spare2[MSL];
    char buf0[MSL];

    arg1[0] = '\0';
    arg2[0] = '\0';
    buf[0] = '\0';
    buf0[0] = '\0';

    if( !argument || argument[0] == '\0' )
    {
	send_to_char( "Syntax: loop <start> <finish> <command>\n\r", ch );
	send_to_char( "Use a # sign to indicate the loop number\n\r", ch
);
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if( arg1 && arg1[0] != '\0' )
	a = atoi( arg1 );
    if( arg2 && arg2[0] != '\0' )
	b = atoi( arg2 );

    if( a )
	loopnum = a;

    if( a > b )
    {
	send_to_char( "Invalid range\n\r", ch );
	return;
    }

    if( a && b )
	if( argument && argument[0] != '\0' )
	{
		/* Lets try this -- GW */
		/* Do the Arg Scan first, only do it once. --GW */
		use_1 = TRUE;
		use_2 = FALSE;
		for( c=0; argument[c]; c++ )
		{
		  if ( argument[c] == '#' )
		  {
		   spare1[c] = '%';
		   spare1[c+1] = 'd';
		   spare1[c+2] = '\0';
		   use_1 = FALSE;
		   use_2 = TRUE;
		   continue;
		  }

		  if ( use_1 )
		   spare1[c] = argument[c];

		  if ( use_2 )
		   spare2[c-strlen(spare1)+1] = argument[c];

		}

		/* Reconstruct it */
		sprintf(buf0,"%s%s",spare1,spare2);

		/* Now do the Looping --GW */
		for ( a=a; a<=b; a++ )
		{
		   buf[0] = '\0';
		   sprintf(buf,buf0,a);
		   interpret( ch, buf );
		}

	send_to_char( "Done\n\r", ch );
	}
    
    return;
}



