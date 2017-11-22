 
/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.0 (C) 1994, 1995, 1996 by Derek Snider             |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh and Tricops  |~'~.VxvxV.~'~*
 * ------------------------------------------------------------------------ *
 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 * ------------------------------------------------------------------------ *
 *			      Regular update module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include <sys/resource.h>
#include <signal.h>
#include <unistd.h>


/*
 * Local functions.
 */
int	hit_gain	args( ( CHAR_DATA *ch ) );
int	mana_gain	args( ( CHAR_DATA *ch ) );
int	move_gain	args( ( CHAR_DATA *ch ) );
void    do_newbie	args( ( CHAR_DATA *ch, char *argument) );
void	mobile_update	args( ( void ) );
void	weather_update	args( ( void ) );
void	char_update	args( ( void ) );
void	obj_update	args( ( void ) );
void	aggr_update	args( ( void ) );
void	room_act_update	args( ( void ) );
void	obj_act_update	args( ( void ) );
void	char_check	args( ( void ) );
void    drunk_randoms	args( ( CHAR_DATA *ch ) );
void    halucinations	args( ( CHAR_DATA *ch ) );
void	subtract_times	args( ( struct timeval *etime,
				struct timeval *stime ) );
void    write_last_command  args( ( void ) );
void	mail_notice	args( ( CHAR_DATA *ch ) );
void    syspanic        args( ( void ) );
void    room_link_check args( ( CHAR_DATA *ch ) );
void dragon_abduct(CHAR_DATA *ch);
void dragon_update( void );
void inroom_update( void );
pid_t getpid(void);

/*
 * Global Variables
 */

CHAR_DATA *	gch_prev;
OBJ_DATA *	gobj_prev;
CHAR_DATA *	timechar;
TIMER *		KOMBAT_TIME;
TIMER *		CTF_TIME;

char * corpse_descs[] =
   { 
     "The corpse of %s is in the last stages of decay.", 
     "The corpse of %s is crawling with vermin.",
     "The corpse of %s fills the air with a foul stench.",
     "The corpse of %s is buzzing with flies.",
     "The corpse of %s lies here."
   };

extern int      top_exit;

/* DEADLOCK*/
#define BOOT_DB_ABORT_THRESHOLD                   20
#define RUNNING_ABORT_THRESHOLD                   8
#define ALARM_FREQUENCY                           20

/* Interval in pulses after which to abort */
int     abort_threshold = BOOT_DB_ABORT_THRESHOLD;
bool    disable_timer_abort = FALSE;
int     last_checkpoint;

/* find number of CPU seconds spent in user mode so far */
int get_user_seconds ()
{
        struct rusage rus;
        getrusage (RUSAGE_SELF, &rus);
        return rus.ru_utime.tv_sec;
  record_call("<get_user_seconds>");
}
 
/* Update the checkpoint */
void alarm_update ()
{
        last_checkpoint = get_user_seconds();
        if (abort_threshold == BOOT_DB_ABORT_THRESHOLD)
        {
                abort_threshold = RUNNING_ABORT_THRESHOLD;
                fprintf (stderr, "Used %d user CPU seconds.\n", 
last_checkpoint);
        }
record_call("<alarm_update>");
}
 
/* Set the virtual (CPU time) timer to the standard setting, 
ALARM_FREQUENCY */
void reset_itimer ()
{
        struct itimerval itimer;
        itimer.it_interval.tv_usec = 0; /* miliseconds */
        itimer.it_interval.tv_sec  = ALARM_FREQUENCY;
        itimer.it_value.tv_usec = 0;
        itimer.it_value.tv_sec = ALARM_FREQUENCY;
   record_call("<reset_itimer>"); 
        /* start the timer - in that many CPU seconds, alarm_handler will 
be called */
        if (setitimer (ITIMER_VIRTUAL, &itimer, NULL) < 0)
        {
                perror ("reset_itimer:setitimer");
                exit (1);
        }
}
 
const char * szFrozenMessage = "Alarm_handler: Not checkpointed recently, 
aborting!\n";
 
/* Signal handler for alarm - suggested for use in MUDs by Fusion */
void alarm_handler (int signo)
{
        int usage_now = get_user_seconds();
        char buf[MSL];
        record_call("<alarm_handler>");
 
	buf[0] = '\0';
 
        /* Has there gone abort_threshold CPU seconds without 
alarm_update? */
    if (!disable_timer_abort && (usage_now - last_checkpoint > 
abort_threshold ))
        {
                /* For the log file */
                log_string(szFrozenMessage);
 		write_last_command();
                signal (signo, SIG_DFL);
		/*syspanic();*/
	        sprintf(buf,"kill -SEGV %d",getpid());
	        system(buf);
//	        exit(11); /* Drop core, exit with Segmentation Fault --GW */ 
//               return;  /*this will create a better core file */
		
        }
 
        /* The timer resets to the values specified in it_interval 
automatically */
}
 
/* Install signal alarm handler */
void init_alarm_handler()
{
        struct sigaction sa;
     record_call("<init_alarm_handler>");
 
        sa.sa_handler = alarm_handler;
        sa.sa_flags = SA_RESTART; /* Restart interrupted system calls */
        sigemptyset(&sa.sa_mask);
 
        if (sigaction(SIGVTALRM, &sa, NULL) < 0) /* setup handler for 
virtual timer */
        {
                perror ("init_alarm_handler:sigaction");
                exit (1);
        }
        last_checkpoint = get_user_seconds();
        reset_itimer(); /* start timer */
}


 /*
 * Advancement stuff.
 */

/*
 * Make a char lose a level, takes away everything they had when they
 * gained it --GW
 */
void lose_level( CHAR_DATA *ch, int lvl, int classnum )
{
int hp_loss=0,mana_loss=0,move_loss=0;
//char buf[MSL];

//buf[0] = '\0';

if ( classnum == 1 )
{
    hp_loss      = con_app[get_curr_con(ch)].hitp + number_range(
                    class_table[ch->class]->hp_min,
                    class_table[ch->class]->hp_max );
    mana_loss    = class_table[ch->class]->fMana
                    ? number_range(2,(2*get_curr_int(ch)+get_curr_wis(ch))/8)
                    : 0;
    move_loss    = number_range( 5, (get_curr_con(ch)+get_curr_dex(ch))/4);

    hp_loss      = UMAX(  1, hp_loss   );
    mana_loss    = UMAX(  0, mana_loss );
    move_loss    = UMAX( 10, move_loss );
}
else if ( classnum == 2 )
{
    hp_loss      = con_app[get_curr_con(ch)].hitp + number_range(
                    class_table[ch->class2]->hp_min,
                    class_table[ch->class2]->hp_max );
    mana_loss    = class_table[ch->class2]->fMana
                    ? number_range(2,(2*get_curr_int(ch)+get_curr_wis(ch))/8)
                    : 0;
    move_loss    = number_range( 5, (get_curr_con(ch)+get_curr_dex(ch))/4);

    hp_loss      = UMAX(  1, hp_loss   );
    mana_loss    = UMAX(  0, mana_loss );
    move_loss    = UMAX( 10, move_loss );
}
else if ( classnum == 3 )
{
    hp_loss      = con_app[get_curr_con(ch)].hitp + number_range(
                    class_table[ch->advclass]->hp_min,
                    class_table[ch->advclass]->hp_max );
    mana_loss    = class_table[ch->advclass]->fMana
                    ? number_range(2,(2*get_curr_int(ch)+get_curr_wis(ch))/8)
                    : 0;
    move_loss    = number_range( 5, (get_curr_con(ch)+get_curr_dex(ch))/4);

    hp_loss      = UMAX(  1, hp_loss   );
    mana_loss    = UMAX(  0, mana_loss );
    move_loss    = UMAX( 10, move_loss );
}
else if ( classnum == 4 )
{
    hp_loss      = con_app[get_curr_con(ch)].hitp + number_range(
                    class_table[ch->advclass2]->hp_min,
                    class_table[ch->advclass2]->hp_max );
    mana_loss    = class_table[ch->advclass2]->fMana
                    ? number_range(2,(2*get_curr_int(ch)+get_curr_wis(ch))/8)
                    : 0;
    move_loss    = number_range( 5, (get_curr_con(ch)+get_curr_dex(ch))/4);

    hp_loss      = UMAX(  1, hp_loss   );
    mana_loss    = UMAX(  0, mana_loss );
    move_loss    = UMAX( 10, move_loss );
}

    /* bonus for deadlies */
/*    if(IS_PKILL(ch)){
        mana_loss = mana_loss + mana_loss*.3;
        move_loss = move_loss + move_loss*.3;
        hp_loss +=1;
    }*/

//sprintf(buf,"Lose_lev: Originals: H:%d MN: %d MV: %d",ch->max_hit,ch->max_mana,ch->max_move); 
//log_string(buf);

ch->max_hit -= hp_loss;
ch->max_mana -= mana_loss;
ch->max_move -= move_loss;

//sprintf(buf,"Lose_lev: %s: HP: %d MN: %d MV: %d",capitalize(ch->name),hp_loss,mana_loss, move_loss);
//log_string(buf);

//sprintf(buf,"Lose_lev: New: H: %d MN: %d MV: %d",ch->max_hit,ch->max_mana,ch->max_move);
//log_string(buf);

save_char_obj(ch);
return;
}

void advance_level( CHAR_DATA *ch, bool SHOW, int class )
{
    char buf[MAX_STRING_LENGTH];
    int add_hp=0;
    int add_mana=0;
    int add_move=0;
    int add_prac=0;
   record_call("<advance_level>");

/*	save_char_obj( ch );*/
/*    sprintf( buf, "the %s",
	title_table [ch->class] [ch->level] [ch->sex == SEX_FEMALE ? 1 : 0] );
    set_title( ch, buf );
*/
  if ( class==1 )
  {

    add_hp	= con_app[get_curr_con(ch)].hitp + number_range(
		    class_table[ch->class]->hp_min,
		    class_table[ch->class]->hp_max );
    add_mana	= class_table[ch->class]->fMana
		    ? number_range(2, (2*get_curr_int(ch)+get_curr_wis(ch))/8)
		    : 0;
    add_move	= number_range( 5, (get_curr_con(ch)+get_curr_dex(ch))/4 );
    add_prac	= wis_app[get_curr_wis(ch)].practice;

    add_hp	= UMAX(  1, add_hp   );
    add_mana	= UMAX(  0, add_mana );
    add_move	= UMAX( 10, add_move );
  }

  if ( class==2 )
  {

    add_hp	= con_app[get_curr_con(ch)].hitp + number_range(
		    class_table[ch->class2]->hp_min,
		    class_table[ch->class2]->hp_max );
    add_mana	= class_table[ch->class2]->fMana
		    ? number_range(2, (2*get_curr_int(ch)+get_curr_wis(ch))/8)
		    : 0;
    add_move	= number_range( 5, (get_curr_con(ch)+get_curr_dex(ch))/4 );
    add_prac	= wis_app[get_curr_wis(ch)].practice;

    add_hp	= UMAX(  1, add_hp   );
    add_mana	= UMAX(  0, add_mana );
    add_move	= UMAX( 10, add_move );
  }

  if ( class==3 )
  {

    add_hp	= (con_app[get_curr_con(ch)].hitp + number_range(
		    class_table[ch->advclass]->hp_min,
		    class_table[ch->advclass]->hp_max )/2);
    add_mana	= class_table[ch->advclass]->fMana
		    ? (number_range(2, (2*get_curr_int(ch)+get_curr_wis(ch))/8)/2)
		    : 0;
    add_move	= number_range( 5, (get_curr_con(ch)+get_curr_dex(ch))/4 );
    add_prac	= wis_app[get_curr_wis(ch)].practice;

    add_hp	= UMAX(  1, add_hp   );
    add_mana	= UMAX(  0, add_mana );
    add_move	= UMAX( 10, add_move );
  }

  if ( class==4 && ch->advclass2 > -1 )
  {

    add_hp	= (con_app[get_curr_con(ch)].hitp + number_range(
		    class_table[ch->advclass2]->hp_min,
		    class_table[ch->advclass2]->hp_max )/2);
    add_mana	= class_table[ch->advclass2]->fMana
		    ? (number_range(2, (2*get_curr_int(ch)+get_curr_wis(ch))/8)/2)
		    : 0;
    add_move	= number_range( 5, (get_curr_con(ch)+get_curr_dex(ch))/4 );
    add_prac	= wis_app[get_curr_wis(ch)].practice;

    add_hp	= UMAX(  1, add_hp   );
    add_mana	= UMAX(  0, add_mana );
    add_move	= UMAX( 10, add_move );
  }

  if ( class > 4 || class < 1 )
  {
	bug("Advance_Level: Bad ClassNumber: %d! Not Advancing!!\n",class);  
	return;
  }

    if ( !IS_NPC(ch) && ch->pcdata->nat_hp >= sysdata.max_hp && !IS_ADV_DUAL(ch) )
    add_hp = 1;
    UMIN( ch->max_hit 	+= add_hp, sysdata.max_hp);
    if ( !IS_NPC(ch) && ch->pcdata->nat_mana >= sysdata.max_mana && !IS_ADV_DUAL(ch) )
    add_mana = 1;
    UMIN( ch->max_mana	+= add_mana, sysdata.max_mana);

    ch->max_move	+= add_move;
    ch->practice	+= add_prac;
    if (!IS_ADVANCED(ch))
    ch->exp             -= exp_level( ch, ch->level);
    if (IS_ADVANCED(ch) && !IS_ADV_DUAL(ch) )
    ch->exp		-= exp_level( ch, ch->advlevel);
    if ( IS_ADV_DUAL(ch) )
    ch->exp		-= exp_level( ch, ch->advlevel2);

    if ( !IS_NPC(ch) )
	REMOVE_BIT( ch->act, PLR_BOUGHT_PET );

    if ( ch->level == LEVEL_AVATAR && ch->level2 == LEVEL_AVATAR
	 && !IS_ADVANCED(ch) )
    {
	DESCRIPTOR_DATA *d;

	sprintf( buf, "%s has reached the next level of power!", ch->name );
	for ( d = first_descriptor; d; d = d->next )
	   if ( d->connected == CON_PLAYING && d->character != ch )
	   {
		set_char_color( AT_IMMORT, d->character );
		send_to_char( buf,	d->character );
		send_to_char( "\n\r",	d->character );
	   }
	set_char_color( AT_WHITE, ch );
	do_help( ch, "M_ADVHERO_" );
    }

    if ( ch->level < LEVEL_IMMORTAL && SHOW )
    {
      if ( IS_VAMPIRE(ch) )
        sprintf( buf,
	  "Your gain is: %d/%d hp, %d/%d bp, %d/%d mv %d/%d prac.\n\r",
	  add_hp,	ch->max_hit,
	  1,	        get_bloodthirst(ch),
	  add_move,	ch->max_move,
	  add_prac,	ch->practice
	  );
      else
        sprintf( buf,
	  "Your gain is: %d/%d hp, %d/%d mana, %d/%d mv %d/%d prac.\n\r",
	  add_hp,	ch->max_hit,
 	  add_mana,	ch->max_mana,
	  add_move,	ch->max_move,
	  add_prac,	ch->practice
	  );
      set_char_color( AT_WHITE, ch );
      send_to_char( buf, ch );
    }
    return;
}   

/*
 * Advancement stuff.
 */
void advance_level2( CHAR_DATA *ch, bool SHOW )
{
    char buf[MAX_STRING_LENGTH];
    int add_hp;
    int add_mana;
    int add_move;
    int add_prac;
   record_call("<advance_level2>");

/*	save_char_obj( ch );*/
/*    sprintf( buf, "the %s",
	title_table [ch->class] [ch->level] [ch->sex == SEX_FEMALE ? 1 : 0] );
    set_title( ch, buf );
*/
    add_hp	= (con_app[get_curr_con(ch)].hitp + number_range(
		    class_table[ch->class2]->hp_min,
		    class_table[ch->class2]->hp_max)/2 );
    add_mana	= class_table[ch->class2]->fMana
		    ? (number_range(2, (2*get_curr_int(ch)+get_curr_wis(ch))/8)/2)
		    : 0;
    add_move	= (number_range( 5, (get_curr_con(ch)+get_curr_dex(ch))/4 )/2);
    add_prac	= wis_app[get_curr_wis(ch)].practice;

    add_hp	= UMAX(  1, add_hp   );
    add_mana	= UMAX(  0, add_mana );
    add_move	= UMAX( 10, add_move );

    if( !IS_NPC(ch) && ch->pcdata->nat_hp >= sysdata.max_hp )
    add_hp = 1;
    UMIN( ch->max_hit	+= add_hp, sysdata.max_hp);
    if( !IS_NPC(ch) && ch->pcdata->nat_mana >= sysdata.max_mana )
    add_mana = 1;
    UMIN(ch->max_mana	+= add_mana, sysdata.max_mana);

    ch->max_move	+= add_move/2;
    ch->practice	+= add_prac;
    ch->exp             -= exp_level( ch, ch->level2);

    if ( !IS_NPC(ch) )
	REMOVE_BIT( ch->act, PLR_BOUGHT_PET );

    if ( ch->level2 == LEVEL_AVATAR && ch->level == LEVEL_AVATAR)
    {
	DESCRIPTOR_DATA *d;

	sprintf( buf, "%s has reached the next level of power!", ch->name );
	for ( d = first_descriptor; d; d = d->next )
	   if ( d->connected == CON_PLAYING && d->character != ch )
	   {
		set_char_color( AT_IMMORT, d->character );
		send_to_char( buf,	d->character );
		send_to_char( "\n\r",	d->character );
	   }
	set_char_color( AT_WHITE, ch );
	do_help( ch, "M_ADVHERO_" );
    }
    if ( ch->level2 < LEVEL_IMMORTAL && SHOW )
    {
      if ( IS_VAMPIRE(ch) )
        sprintf( buf,
	  "Your gain is: %d/%d hp, %d/%d bp, %d/%d mv %d/%d prac.\n\r",
	  add_hp,	ch->max_hit,
	  1,	        get_bloodthirst(ch),
	  add_move,	ch->max_move,
	  add_prac,	ch->practice
	  );
      else
        sprintf( buf,
	  "Your gain is: %d/%d hp, %d/%d mana, %d/%d mv %d/%d prac.\n\r",
	  add_hp,	ch->max_hit,
 	  add_mana,	ch->max_mana,
	  add_move,	ch->max_move,
	  add_prac,	ch->practice
	  );
      set_char_color( AT_WHITE, ch );
      send_to_char( buf, ch );
    }
    return;
}   



void gain_exp( CHAR_DATA *ch, int gain, bool fGain )
{
    long modgain;
    char buf[MAX_STRING_LENGTH];
   record_call("<gain_exp>");

    modgain = gain;

/* Nah -- Not Nomore --GW	   
	if ( fGain )
	{
	if ( modgain <= 0) modgain = 0;
	if ( modgain > 450000 ) modgain = 450000;
	if ( modgain > 0 && ( get_timer(ch, TIMER_RECENTFIGHT) <= 0) )
	{ 
	buf[0] = '\0';    
        sprintf( buf, "You receive %ld extra experience points for your success.\n\r",modgain );
	send_to_char(buf, ch);
	}


     Deadly exp loss floor is exp floor of level 
    if(IS_PKILL(ch)&& modgain<0){
       if( ch->exp + modgain < exp_level(ch, ch->level)){
          modgain = exp_level(ch, ch->level) - ch->exp;
	}
	}

  if (ch->exp + modgain < 2099999999 ) 
    ch->exp += modgain;
}
*/

  if ( fGain )
    URANGE( 0, ch->exp += gain, 2100000000);
  else
    URANGE( 0, ch->exp -= gain, 2100000000);

  if (NOT_AUTHED(ch) && ch->exp >= exp_level(ch, ch->level+1))
    {
	send_to_char("You can not ascend to a higher level until you are authorized.\n\r", ch);
	ch->exp = (exp_level(ch, (ch->level+1)) - 1);
	return;
    }

    while ( IS_ADVANCED(ch) && !IS_ADV_DUAL(ch) && ch->exp >= exp_level(ch,ch->advlevel+1) && ch->advlevel < 50 )
    {
	set_char_color( AT_WHITE + AT_BLINK, ch );
	ch_printf( ch, "You have now obtained experience level %d!\n\r",ch->advlevel+1 );
	ch->advlevel += 1;
	advance_level( ch, TRUE, 3 );
	sprintf(buf,"%s is now level %d %s",QUICKLINK(ch->name),ch->advlevel,capitalize(npc_class[ch->advclass]));
	info(buf);
    }

    while ( IS_ADV_DUAL(ch) && ch->exp >= exp_level(ch,ch->advlevel2+1) && ch->advlevel2 < 50 )
    {
	set_char_color( AT_WHITE + AT_BLINK, ch );
	ch_printf( ch, "You have now obtained experience level %d!\n\r",ch->advlevel2+1 );
	ch->advlevel2 += 1;
	advance_level( ch, TRUE, 4 );
	sprintf(buf,"%s is now level %d %s",QUICKLINK(ch->name),ch->advlevel2,capitalize(npc_class[ch->advclass2]));
	info(buf);
    }
    return;

}


/*
 * Regeneration stuff.
 */

int hit_gain( CHAR_DATA *ch )
{
    int gain;
   record_call("<hit_gain>"); 

    if ( IS_NPC(ch) )
    {
	gain = ch->level * 3 / 2;
    }
    else
    {

	gain = UMIN( 5, ch->level );

	switch ( ch->position )
	{
	case POS_DEAD:	   return 0;
	case POS_MORTAL:   return -1;
	case POS_INCAP:    return -1;
	case POS_STUNNED:  return 1;
	case POS_SLEEPING: gain += get_curr_con(ch) * 1.5;	break;
	case POS_RESTING:  gain += get_curr_con(ch); 		break;
	}

        if ( IS_VAMPIRE(ch) ) {
            if ( ch->pcdata->condition[COND_BLOODTHIRST] <= 1 )
		gain /= 2;
	    else
	    if ( ch->pcdata->condition[COND_BLOODTHIRST] >= (8 + ch->level) )
                gain *= 2;
            if ( IS_OUTSIDE( ch ) )
  	    {
    	       switch(weather_info.sunlight)
    	       {
    	          case SUN_RISE:
    		  case SUN_SET:
      	            gain /= 2;
      	            break;
    		  case SUN_LIGHT:
      		    gain /= 4;
      		    break;
      	       }
      	    }
        }


	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL]   == 0 )
	    gain /= 2;

	if(!IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] == 0 )
	    gain /= 2;

 if ( ch->in_room->vnum == ch->pcdata->rentroom  && ch->gold > 0)
   gain *= 2;
 if ( IS_SET(ch->pcdata->flags, PCFLAG_TENNANT) && ch->gold > 0)
   ch->gold -= 100;

    }

    if ( IS_AFFECTED(ch, AFF_POISON) )
	gain /= 4;

    return UMIN(gain, ch->max_hit - ch->hit);
}



int mana_gain( CHAR_DATA *ch )
{
    int gain;
   record_call("<mana_gain>");

    if ( IS_NPC(ch) )
    {
	gain = ch->level;
    }
    else
    {
	gain = UMIN( 5, ch->level / 2 );

	if ( ch->position < POS_SLEEPING )
	  return 0;
	switch ( ch->position )
	{
	case POS_SLEEPING: gain += get_curr_int(ch) * 3;	break;
	case POS_RESTING:  gain += get_curr_int(ch) * 1.5;	break;
	}

	if ( ch->pcdata->condition[COND_FULL]   == 0 )
	    gain /= 2;

	if ( ch->pcdata->condition[COND_THIRST] == 0 )
	    gain /= 2;

    }

    if ( IS_AFFECTED( ch, AFF_POISON ) )
	gain /= 4;

    return UMIN(gain, ch->max_mana - ch->mana);
}



int move_gain( CHAR_DATA *ch )
{
    int gain;
   record_call("<move_gain>");

    if ( IS_NPC(ch) )
    {
	gain = ch->level;
    }
    else
    {
	gain = UMAX( 15, 2 * ch->level );

	switch ( ch->position )
	{
	case POS_DEAD:	   return 0;
	case POS_MORTAL:   return -1;
	case POS_INCAP:    return -1;
	case POS_STUNNED:  return 1;
	case POS_SLEEPING: gain += get_curr_dex(ch) * 2;	break;
	case POS_RESTING:  gain += get_curr_dex(ch);		break;
	}

        if ( IS_VAMPIRE(ch) ) {
            if ( ch->pcdata->condition[COND_BLOODTHIRST] <= 1 )
		gain /= 2;
	    else
	    if ( ch->pcdata->condition[COND_BLOODTHIRST] >= (8 + ch->level) )
                gain *= 2;
            if ( IS_OUTSIDE( ch ) )
  	    {
    	       switch(weather_info.sunlight)
    	       {
    	          case SUN_RISE:
    		  case SUN_SET:
      	            gain /= 2;
      	            break;
    		  case SUN_LIGHT:
      		    gain /= 4;
      		    break;
      	       }
      	    }
        }

	if ( ch->pcdata->condition[COND_FULL]   == 0 )
	    gain /= 2;

	if ( ch->pcdata->condition[COND_THIRST] == 0 )
	    gain /= 2;
    }

    if ( IS_AFFECTED(ch, AFF_POISON) )
	gain /= 4;

    return UMIN(gain, ch->max_move - ch->move);
}


void gain_condition( CHAR_DATA *ch, int iCond, int value )
{
    int condition;
    ch_ret retcode=rNONE;
   record_call("<gain_condition>");

    if ( value == 0 || IS_NPC(ch) || ch->level >= LEVEL_IMMORTAL || NOT_AUTHED(ch))
	return;

    condition				= ch->pcdata->condition[iCond];
    if ( iCond == COND_BLOODTHIRST )
      ch->pcdata->condition[iCond]    = URANGE( 0, condition + value,
                                                get_bloodthirst(ch) );
    else
      ch->pcdata->condition[iCond]    = URANGE( 0, condition + value, 48 );

    if ( ch->pcdata->condition[iCond] == 0 )
    {
	switch ( iCond )
	{
	case COND_FULL:
          if ( ch->level < LEVEL_AVATAR && ch->class != CLASS_VAMPIRE )
          {
            set_char_color( AT_HUNGRY, ch );
	    send_to_char( "You are STARVING!\n\r",  ch );
            act( AT_HUNGRY, "$n is starved half to death!", ch, NULL, NULL, TO_ROOM);
	    if ( !IS_PKILL(ch) || number_bits(1) == 0 )
		worsen_mental_state( ch, 1 );
	   if ( ch->hit > 2 )
	    retcode = damage(ch, ch, 1, TYPE_UNDEFINED);
	   else
	    retcode = rNONE;
          }
          break;

	case COND_THIRST:
          if ( ch->level < LEVEL_AVATAR && ch->class != CLASS_VAMPIRE )
          {
            set_char_color( AT_THIRSTY, ch );
	    send_to_char( "You are DYING of THIRST!\n\r", ch );
            act( AT_THIRSTY, "$n is dying of thirst!", ch, NULL, NULL, TO_ROOM);
	    worsen_mental_state( ch, IS_PKILL(ch) ? 1: 2 );
	   if ( ch->hit > 2 )
	    retcode = damage(ch, ch, 2, TYPE_UNDEFINED);
	   else
	    retcode = rNONE;
          }
          break;

        case COND_BLOODTHIRST:
          if ( ch->level < LEVEL_AVATAR )
          {
            set_char_color( AT_BLOOD, ch );
            send_to_char( "You are starved to feast on blood!\n\r", ch );
            act( AT_BLOOD, "$n is suffering from lack of blood!", ch,
                 NULL, NULL, TO_ROOM);
	    worsen_mental_state( ch, 2 );
	   if ( ch->hit > ( ch->max_hit / 20 ) )
	    retcode = damage(ch, ch, ch->max_hit / 20, TYPE_UNDEFINED);
	   else
	    retcode = rNONE;
          }
          break;
	case COND_DRUNK:
	    if ( condition != 0 ) {
                set_char_color( AT_SOBER, ch );
		send_to_char( "You are sober.\n\r", ch );
	    }
	    retcode = rNONE;
	    break;
	default:
	    bug( "Gain_condition: invalid condition type %d", iCond );
	    retcode = rNONE;
	    break;
	}
    }

    if ( retcode != rNONE )
      return;

    if ( ch->pcdata->condition[iCond] == 1 )
    {
	switch ( iCond )
	{
	case COND_FULL:
          if ( ch->level < LEVEL_AVATAR && ch->class != CLASS_VAMPIRE )
          {
            set_char_color( AT_HUNGRY, ch );
	    send_to_char( "You are really hungry.\n\r",  ch );
            act( AT_HUNGRY, "You can hear $n's stomach growling.", ch, NULL, NULL, TO_ROOM);
	    if ( number_bits(1) == 0 )
		worsen_mental_state( ch, 1 );
          } 
	  break;

	case COND_THIRST:
          if ( ch->level < LEVEL_AVATAR && ch->class != CLASS_VAMPIRE )
          {
            set_char_color( AT_THIRSTY, ch );
	    send_to_char( "You are really thirsty.\n\r", ch );
	    worsen_mental_state( ch, 1 );
	    act( AT_THIRSTY, "$n looks a little parched.", ch, NULL, NULL, TO_ROOM);
          } 
	  break;

        case COND_BLOODTHIRST:
          if ( ch->level < LEVEL_AVATAR )
          {
            set_char_color( AT_BLOOD, ch );
            send_to_char( "You have a growing need to feast on blood!\n\r", ch );
            act( AT_BLOOD, "$n gets a strange look in $s eyes...", ch,
                 NULL, NULL, TO_ROOM);
	    worsen_mental_state( ch, 1 );
          }
          break;
	case COND_DRUNK:
	    if ( condition != 0 ) {
                set_char_color( AT_SOBER, ch );
		send_to_char( "You are feeling a little less light headed.\n\r", ch );
            }
	    break;
	}
    }


    if ( ch->pcdata->condition[iCond] == 2 )
    {
	switch ( iCond )
	{
	case COND_FULL:
          if ( ch->level < LEVEL_AVATAR && ch->class != CLASS_VAMPIRE )
          {
            set_char_color( AT_HUNGRY, ch );
	    send_to_char( "You are hungry.\n\r",  ch );
          } 
	  break;

	case COND_THIRST:
          if ( ch->level < LEVEL_AVATAR && ch->class != CLASS_VAMPIRE )
          {
            set_char_color( AT_THIRSTY, ch );
	    send_to_char( "You are thirsty.\n\r", ch );
          } 
	  break;

        case COND_BLOODTHIRST:
          if ( ch->level < LEVEL_AVATAR )
          {
            set_char_color( AT_BLOOD, ch );
            send_to_char( "You feel an urgent need for blood.\n\r", ch );
          }  
          break;
	}
    }

    if ( ch->pcdata->condition[iCond] == 3 )
    {
	switch ( iCond )
	{
	case COND_FULL:
          if ( ch->level < LEVEL_AVATAR && ch->class != CLASS_VAMPIRE )
          {
            set_char_color( AT_HUNGRY, ch );
	    send_to_char( "You are a mite peckish.\n\r",  ch );
          } 
	  break;

	case COND_THIRST:
          if ( ch->level < LEVEL_AVATAR && ch->class != CLASS_VAMPIRE )
          {
            set_char_color( AT_THIRSTY, ch );
	    send_to_char( "You could use a sip of something refreshing.\n\r", ch );
          } 
	  break;

        case COND_BLOODTHIRST:
          if ( ch->level < LEVEL_AVATAR )
          {
            set_char_color( AT_BLOOD, ch );
            send_to_char( "You feel an aching in your fangs.\n\r", ch );
          }
          break;
	}
    }
    return;
}

/* check and see if a char is in the list --GW */
CHAR_DATA *get_char_list( CHAR_DATA *ch )
{
CHAR_DATA *vch;

if ( !ch || IS_NPC(ch) )
return NULL;

for ( vch = first_char; vch; vch = vch->next ) 
{
if ( !IS_NPC(vch) && !str_cmp( vch->name, ch->name ) )
return vch;
}

return NULL;
}


/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Mud cpu time.
 */
void mobile_update( void )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *ch;
    EXIT_DATA *pexit;
    int door;
    ch_ret     retcode;
    ZONE_DATA *in_zone;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *test_char;

    retcode = rNONE;
   record_call("<mobile_update>");

    /* Examine all mobs. */
  
    for ( ch = last_char; ch; ch = gch_prev )
    {
	set_cur_char( ch );
	if ( ch == first_char && ch->prev )
	{
	    bug( "mobile_update: first_char->prev != NULL... fixed", 0 );
	    ch->prev = NULL;
	}
	  
	gch_prev = ch->prev;

/*---->*/   if ( !gch_prev )
            break;
	
	if ( gch_prev && gch_prev->next != ch )
	{
	    sprintf( buf, "FATAL: Mobile_update: %s->prev->next doesn't point to ch.",
		ch->name );
	    bug( buf, 0 );	    
	    bug( "Short-cutting here", 0 );
	    gch_prev = NULL;
	    ch->prev = NULL;
	}


	if ( !IS_NPC(ch) && ch->in_room )
	{

         if ( ch->desc ) /* only if the character is connected --GW */
         {
	   for ( d = first_descriptor; d; d = d->next )
           {
		test_char = NULL;
                if ( (  d->connected == CON_PLAYING || d->connected == CON_EDITING )
                && ( ( test_char = get_char_list(d->character) )==NULL ) )
 		{
		bug("Char Found NOT IN LIST!! (%s)!",d->character->name);
		bug("Linking...",0);	
		add_char(d->character);
		}
	    }
	}
	    drunk_randoms(ch);
	    halucinations(ch);

    }

  }

for( in_zone = first_zone; in_zone; in_zone = in_zone->next )
{
    for ( ch = in_zone->last_mob; ch; ch = gch_prev )
    {
	set_cur_char( ch );
	if ( ch == in_zone->first_mob && ch->prev )
	{
	    bug( "mobile_update: first_char->prev != NULL... fixed", 0 );
	    ch->prev = NULL;
	}
	  
	gch_prev = ch->prev;

/*---->*/   if ( !gch_prev )
            break;
	
	if ( gch_prev && gch_prev->next != ch )
	{
	    sprintf( buf, "FATAL: Mobile_update: %s->prev->next doesn't point to ch.",
		ch->name );
	    bug( buf, 0 );	    
	    bug( "Short-cutting here", 0 );
	    gch_prev = NULL;
	    ch->prev = NULL;
	}


	if ( !IS_NPC(ch))
	{
	    drunk_randoms(ch);
	    halucinations(ch);
	    continue;
	}

	if ( !ch->in_room 
	||   IS_AFFECTED(ch, AFF_CHARM)
	||   IS_AFFECTED(ch, AFF_PARALYSIS)) 
	    continue;

/* Clean up 'animated corpses' that are not charmed' - Scryn */


       if (  ch->pIndexData->vnum == 5 && !IS_AFFECTED(ch,AFF_CHARM) )
	{
	  if(ch->in_room->first_person)
	    act(AT_MAGIC, "$n returns to the dust from whence $e came.", ch, NULL, NULL, TO_ROOM);
          
   	  if(IS_NPC(ch)) 
	    extract_char(ch, TRUE);
	  continue;
	} 

        /* Make Sure the Smart Mobs stay smart =) --GW */
	smart_update(ch,who_fighting(ch));

	if ( !IS_SET( ch->act, ACT_RUNNING )
	&&   !IS_SET( ch->act, ACT_SENTINEL )
	&&   !ch->fighting && ch->hunting )
	{
	  WAIT_STATE( ch, 2 * PULSE_VIOLENCE );
	  /* Commented out temporarily to avoid spam - Scryn 
	  sprintf( buf, "%s hunting %s from %s.", ch->name,
	  	ch->hunting->name,
		ch->in_room->name );
	  log_string( buf ); */
	  hunt_victim( ch );
	  continue;
	}  

	/* Examine call for special procedure */
	if ( !IS_SET( ch->act, ACT_RUNNING )
	&&    ch->spec_fun )
	{
	    if ( (*ch->spec_fun) ( ch ) )
		continue;
	    if ( char_died(ch) )
		continue;
	}

	/* Check for mudprogram script on mob */
	if ( IS_SET( ch->pIndexData->progtypes, SCRIPT_PROG ))
	{
	    mprog_script_trigger( ch );
	    continue;
	}

	if ( ch != cur_char )
	{
	    bug( "Mobile_update: ch != cur_char after spec_fun", 0 );
	    continue;
	}

	/* That's all for sleeping / busy monster */
	if ( ch->position != POS_STANDING )
	    continue;

	if ( IS_SET(ch->act, ACT_MOUNTED ) )
	{
	    if ( IS_SET(ch->act, ACT_AGGRESSIVE) )
		do_emote( ch, "snarls and growls." );
	    continue;
	}

	if ( IS_SET(ch->in_room->room_flags, ROOM_SAFE )
	&&   IS_SET(ch->act, ACT_AGGRESSIVE) )
	    do_emote( ch, "glares around and snarls." );


	/* MOBprogram random trigger */
	if ( ch->in_room->area->nplayer > 0 )
	{
	    mprog_random_trigger( ch );
	    if ( char_died(ch) )
		continue;
	    if ( ch->position < POS_STANDING )
	        continue;
	}

        /* MOBprogram hour trigger: do something for an hour */
        mprog_hour_trigger(ch);

	if ( char_died(ch) )
	  continue;

	rprog_hour_trigger(ch);
	if ( char_died(ch) )
	  continue;

	if ( ch->position < POS_STANDING )
	  continue;

	/* Scavenge */
	if ( IS_SET(ch->act, ACT_SCAVENGER)
	&&   ch->in_room->first_content
	&&   number_bits( 2 ) == 0 )
	{
	    OBJ_DATA *obj;
	    OBJ_DATA *obj_best;
	    int max;

	    max         = 1;
	    obj_best    = NULL;
	    for ( obj = ch->in_room->first_content; obj; obj = obj->next_content )
	    {
		if ( CAN_WEAR(obj, ITEM_TAKE) && obj->cost > max 
		&& !IS_OBJ_STAT( obj, ITEM_BURRIED ) )
		{
		    obj_best    = obj;
		    max         = obj->cost;
		}
	    }

	    if ( obj_best )
	    {
		obj_from_room( obj_best );
		obj_to_char( obj_best, ch );
		act( AT_ACTION, "$n gets $p.", ch, obj_best, NULL, TO_ROOM );
	    }
	}

	/* Wander */
	if ( !IS_SET(ch->act, ACT_RUNNING)
	&&   !IS_SET(ch->act, ACT_SENTINEL)
	&&   !IS_SET(ch->act, ACT_PROTOTYPE)
	&& ( door = number_bits( 5 ) ) <= 9
	&& ( pexit = get_exit(ch->in_room, door) ) != NULL
	&&   pexit->to_room
	&&   !IS_SET(pexit->exit_info, EX_CLOSED)
	&&   !IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB)
	&&   !IS_SET(pexit->to_room->room_flags, ROOM_DEATH)
	&& ( !IS_SET(ch->act, ACT_STAY_AREA)
	||   pexit->to_room->area == ch->in_room->area ) )
	{
	    retcode = move_char( ch, pexit, 0, FALSE );
						/* If ch changes position due
						to it's or someother mob's
						movement via MOBProgs,
						continue - Kahn */
	    if ( char_died(ch) )
	      continue;
	    if ( retcode != rNONE || IS_SET(ch->act, ACT_SENTINEL)
	    ||    ch->position < POS_STANDING )
	        continue;
	}

	/* Flee */
	if ( ch->hit < ch->max_hit / 2
	&& ( door = number_bits( 4 ) ) <= 9
	&& ( pexit = get_exit(ch->in_room,door) ) != NULL
	&&   pexit->to_room
	&&   !IS_SET(pexit->exit_info, EX_CLOSED)
	&&   !IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB) )
	{
	    CHAR_DATA *rch;
	    bool found;

	    found = FALSE;
	    for ( rch  = ch->in_room->first_person;
		  rch;
		  rch  = rch->next_in_room )
	    {
		if ( is_fearing(ch, rch) )
		{
		    switch( number_bits(2) )
		    {
			case 0:
			  sprintf( buf, "Get away from me, %s!", rch->name );
			  break;
			case 1:
			  sprintf( buf, "Leave me be, %s!", rch->name );
			  break;
			case 2:
			  sprintf( buf, "%s is trying to kill me!  Help!", rch->name );
			  break;
			case 3:
			  sprintf( buf, "Someone save me from %s!", rch->name );
			  break;
		    }
		    do_yell( ch, buf );
		    found = TRUE;
		    break;
		}
	    }
	    if ( found )
		retcode = move_char( ch, pexit, 0, FALSE );
	}
    }
}
    return;
}



/*
 * Update the weather.
 */
void weather_update( void )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int diff;
    sh_int AT_TEMP = AT_PLAIN;
   record_call("<weather_update>");

    buf[0] = '\0';

    switch ( ++time_info.hour )
    {

    case  5:
	weather_info.sunlight = SUN_LIGHT;
	strcat( buf, "Another day has begun." );
        AT_TEMP = AT_YELLOW;
	break;

    case  6:
	weather_info.sunlight = SUN_RISE;
	strcat( buf, "A stunning sun rise begins in the east." );
        AT_TEMP = AT_ORANGE;
	break;

    case 12:
        weather_info.sunlight = SUN_LIGHT;
        strcat( buf, "The sun sits directly overhead." ); 
        AT_TEMP = AT_YELLOW;
   	break;
	
    case 19:
	weather_info.sunlight = SUN_SET;
	strcat( buf, "Dusk has begun." );
        AT_TEMP = AT_BLOOD;
  	break;

    case 20:
	weather_info.sunlight = SUN_DARK;
	strcat( buf, "Your surroundings darken, as night begins." );
        AT_TEMP = AT_DGREY;
	break;

    case 24:
	time_info.hour = 0;
	time_info.day++;
	break;
    }

    if ( time_info.day   >= 30 )
    {
	time_info.day = 0;
	time_info.month++;
    }

    if ( time_info.month >= 17 )
    {
	time_info.month = 0;
	time_info.year++;
    }

    if ( buf[0] != '\0' )
    {
	for ( d = first_descriptor; d; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   IS_OUTSIDE(d->character)
	    &&   IS_AWAKE(d->character) )
		act( AT_TEMP, buf, d->character, 0, 0, TO_CHAR );
	}
        buf[0] = '\0';
    }
    /*
     * Weather change.
     */
    if ( time_info.month >= 9 && time_info.month <= 16 )
	diff = weather_info.mmhg >  985 ? -2 : 2;
    else
	diff = weather_info.mmhg > 1015 ? -2 : 2;

    weather_info.change   += diff * dice(1, 4) + dice(2, 6) - dice(2, 6);
    weather_info.change    = UMAX(weather_info.change, -12);
    weather_info.change    = UMIN(weather_info.change,  12);

    weather_info.mmhg += weather_info.change;
    weather_info.mmhg  = UMAX(weather_info.mmhg,  960);
    weather_info.mmhg  = UMIN(weather_info.mmhg, 1040);

    AT_TEMP = AT_GREY;
    switch ( weather_info.sky )
    {
    default: 
	bug( "Weather_update: bad sky %d.", weather_info.sky );
	weather_info.sky = SKY_CLOUDLESS;
	break;

    case SKY_CLOUDLESS:
	if ( weather_info.mmhg <  990
	|| ( weather_info.mmhg < 1010 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The sky begins to cloud over." );
	    weather_info.sky = SKY_CLOUDY;
            AT_TEMP = AT_GREY;
	}
	break;

    case SKY_CLOUDY:
	if ( weather_info.mmhg <  970
	|| ( weather_info.mmhg <  990 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "With a crackle of thunder, it begins to rain."
);
	    weather_info.sky = SKY_RAINING;
            AT_TEMP = AT_BLUE;
	}

	if ( weather_info.mmhg > 1030 && number_bits( 2 ) == 0 )
	{
	    strcat( buf, "The sky begins to clear." );
	    weather_info.sky = SKY_CLOUDLESS;
            AT_TEMP = AT_WHITE;
	}
	break;

    case SKY_RAINING:
	if ( weather_info.mmhg <  970 && number_bits( 2 ) == 0 )
	{
	    strcat( buf, "The ground shakes breifly, as thunder crackles."
);
	    weather_info.sky = SKY_LIGHTNING;
	    AT_TEMP = AT_YELLOW;
	}

	if ( weather_info.mmhg > 1030
	|| ( weather_info.mmhg > 1010 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The rain stopped." );
	    weather_info.sky = SKY_CLOUDY;
	    AT_TEMP = AT_WHITE;
	}
	break;

    case SKY_LIGHTNING:
	if ( weather_info.mmhg > 1010
	|| ( weather_info.mmhg >  990 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The lightning has stopped." );
	    weather_info.sky = SKY_RAINING;
            AT_TEMP = AT_GREY;
	    break;
	}
	break;
    }

    if ( buf[0] != '\0' )
    {
	for ( d = first_descriptor; d; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   IS_OUTSIDE(d->character)
	    &&   IS_AWAKE(d->character) )
		act( AT_TEMP, buf, d->character, 0, 0, TO_CHAR );
	}
    }

    return;
}



/*
 * Update all chars, including mobs.
 * This function is performance sensitive.
 */
void char_update( void )
{   
    CHAR_DATA *ch;
    CHAR_DATA *ch_save;
    sh_int save_count = 0;
    int rannum;
    ZONE_DATA *in_zone;
    ROOM_INDEX_DATA *room;
    CHAR_DATA *kch;
    char buf[MSL];
   record_call("<char_update>");

    ch_save	= NULL;

if ( IS_SET( kombat->bits, KOMBAT_ON ) && IS_SET( kombat->bits, KOMBAT_ACCEPT ) )
{
if ( !KOMBAT_TIME->count )
{
echo_to_all(AT_PLAIN,"&R<&BMortal Kombat&R> &BKombat is Klosed.",ECHOTAR_ALL);
REMOVE_BIT( kombat->bits, KOMBAT_ACCEPT );

for ( kch = first_char; kch; kch = kch->next )
{
if ( !IS_NPC(kch) && IS_SET( kch->pcdata->flags, PCFLAG_KOMBAT ) )
{
send_to_char("You march out to BATTLE! BANZAI!!\n\r",kch);
char_from_room(kch);
char_to_room( kch, get_room_index( 21000,1) );
do_look(kch,"auto");
}
}
}
else
switch( KOMBAT_TIME->count )
{
case 6:
echo_to_all(AT_PLAIN,"&R<&BMortal Kombat&R> &BFirst Warning, type '&RKombat&B' to enter.",ECHOTAR_ALL);
KOMBAT_TIME->count--;
break;

case 4:
echo_to_all(AT_PLAIN,"&R<&BMortal Kombat&R> &BSecond Warning, type '&RKombat&B' to enter.",ECHOTAR_ALL);
KOMBAT_TIME->count--;
break;

case 2:
echo_to_all(AT_PLAIN,"&R<&BMortal Kombat&R> LAST Warning&B, type '&RKombat&B' to enter.",ECHOTAR_ALL);
KOMBAT_TIME->count--;
break;

default:
KOMBAT_TIME->count--;
break;
}
}

/* CTF */
if ( CTF_ACTIVE && CTF_ACCEPTING )
{
if ( !CTF_TIME->count )
{
echo_to_all(AT_PLAIN,"&R[&WCTF&R] &WCTF has Begun.",ECHOTAR_ALL);
CTF_ACCEPTING=FALSE;

for ( kch = first_char; kch; kch = kch->next )
{
if ( !IS_NPC(kch) && IS_SET( kch->pcdata->flagstwo, MOREPC_CTF ) )
{
char_from_room(kch);

if ( kch->pcdata->team == TEAM_RED )
char_to_room( kch, get_room_index( CTF_RED_BASE,1) );
if ( kch->pcdata->team == TEAM_BLUE )
char_to_room( kch, get_room_index( CTF_BLUE_BASE,1) );

do_look(kch,"auto");
send_to_char("You march out to BATTLE! BANZAI!!\n\r",kch);
}
}
}
else
switch( CTF_TIME->count )
{
case 6:
echo_to_all(AT_PLAIN,"&R[&WCTF&R] &WFirst Warning, type '&RCTF&W' to enter.",ECHOTAR_ALL);
CTF_TIME->count--;
break;

case 4:
echo_to_all(AT_PLAIN,"&R[&WCTF&R] &WSecond Warning, type '&RCTF&W' to enter.",ECHOTAR_ALL);
CTF_TIME->count--;
break;

case 2:
echo_to_all(AT_PLAIN,"&R[&WCTF&R] &RLAST Warning&W, type '&RCTF&W' to enter.",ECHOTAR_ALL);
CTF_TIME->count--;
break;

default:
CTF_TIME->count--;
break;
}
}

    for ( ch = last_char; ch; ch = gch_prev )
    {

 /*----->*/   if ( !ch )
                break;

	if ( ch == first_char && ch->prev )
	{
	    bug( "char_update: first_char->prev != NULL... fixed", 0 );
	    ch->prev = NULL;
	}
	gch_prev = ch->prev;
	set_cur_char( ch );
	if ( gch_prev && gch_prev->next != ch )
	{
	    bug( "char_update: ch->prev->next != ch", 0 );
	    return;
	}

	if( ch->in_room == NULL )
	{
/*	  bug("UPDATE_CHAR: Null ch->in_room(%s)!*FATAL*",capitalize(ch->name));*/
	  continue;
	}
	if( ( room=get_room_index(ch->in_room->vnum,1))==NULL)
	{
	  bug("UPDATE_CHAR: Char in NULL room!(%s)*FATAL*",capitalize(ch->name));
	}

	rannum = dice(1,10);
        if ((rannum >= 5) && (!IS_NPC(ch)) && (ch->level <= 5) && (IS_SET(ch->pcdata->flags, PCFLAG_NEWBIEHELP)))
	{
	do_newbie(ch,"show");
	}

	mail_notice( ch );

	room_link_check( ch );

	if ( ch->stun_rounds > 0 && !ch->fighting )	
	ch->stun_rounds = 0;

	/*
	 * Die on Tick for Sever --GW
	 */
     if ( !IS_NPC(ch) )
     {
        if(IS_SET(ch->pcdata->flags,PCFLAG_SEVERED))
        {
            REMOVE_BIT(ch->pcdata->flags,PCFLAG_SEVERED);
            act(AT_CARNAGE,"With a last gasp of breath, $n dies due to massive lower body trauma.",ch,NULL,NULL,TO_ROOM);
            send_to_char("Your injuries prove too much, and you die.\n\r",ch);
            raw_kill(ch,ch);
            continue;
        }  /*Be sure to define raw_kill up top of update.c*/
     } 

	if ( !IS_NPC(ch) && IS_SET( ch->pcdata->flagstwo, MOREPC_ALIENS ) )
	char_alien_update( ch );

        /*
	 *  Do a room_prog rand check right off the bat
	 *   if ch disappears (rprog might wax npc's), continue
	 */
	if(!IS_NPC(ch))
	    rprog_random_trigger( ch );

	if( char_died(ch) )
	    continue;

	if( char_died(ch) )
	    continue;


	rprog_time_trigger(ch);

	if( char_died(ch) )
	    continue;

	/* Character Update times .. --GW */
	if ( ( !ch->desc || ch->desc->connected == CON_PLAYING ) && ch->level > 2 )	
	{
	  if ( (current_time - ch->pcdata->update_time) >= 60 )
	  {
	   ch->timer++;
	   ch->pcdata->update_time=current_time;
	  }
	}
	     
	/*
	 * See if player should be auto-saved.
	 */
	if ( !IS_NPC(ch)
	&& ( !ch->desc || ch->desc->connected == CON_PLAYING )
	&&    ch->level >= 2
	&&    current_time - ch->pcdata->save_time > (sysdata.save_frequency*60) )
	    ch_save	= ch;
	else
	    ch_save	= NULL;

	if ( ch->position >= POS_STUNNED )
	{
	    if ( ch->hit  < ch->max_hit )
		ch->hit  += hit_gain(ch);

	    if ( ch->mana < ch->max_mana )
		ch->mana += mana_gain(ch);

	    if ( ch->move < ch->max_move )
		ch->move += move_gain(ch);
	}

	if ( ch->position == POS_STUNNED )
	    update_pos( ch );

	if ( !IS_NPC(ch) && ch->level < LEVEL_IMMORTAL )
	{
	    OBJ_DATA *obj;

	    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
	    &&   obj->item_type == ITEM_LIGHT
	    &&   obj->value[2] > 0 )
	    {
		if ( --obj->value[2] == 0 && ch->in_room )
		{
		    ch->in_room->light -= obj->count;
		    act( AT_ACTION, "$p goes out.", ch, obj, NULL, TO_ROOM );
		    act( AT_ACTION, "$p goes out.", ch, obj, NULL, TO_CHAR );
		    if ( obj->serial == cur_obj )
		      global_objcode = rOBJ_EXPIRED;
		    extract_obj( obj );
		}
	    }

	    if ( ch->pcdata->condition[COND_DRUNK] > 8 )
		worsen_mental_state( ch, ch->pcdata->condition[COND_DRUNK]/8 );
	    if ( ch->pcdata->condition[COND_FULL] > 1 )
	    {
		switch( ch->position )
		{
		    case POS_SLEEPING:  better_mental_state( ch, 4 );	break;
		    case POS_RESTING:   better_mental_state( ch, 3 );	break;
		    case POS_SITTING:
		    case POS_MOUNTED:   better_mental_state( ch, 2 );	break;
		    case POS_STANDING:  better_mental_state( ch, 1 );	break;
		    case POS_FIGHTING:
			if ( number_bits(2) == 0 )
			    better_mental_state( ch, 1 );
			break;
		}
	    }
	    if ( ch->pcdata->condition[COND_THIRST] > 1 )
	    {
		switch( ch->position )
		{
		    case POS_SLEEPING:  better_mental_state( ch, 5 );	break;
		    case POS_RESTING:   better_mental_state( ch, 3 );	break;
		    case POS_SITTING:
		    case POS_MOUNTED:   better_mental_state( ch, 2 );	break;
		    case POS_STANDING:  better_mental_state( ch, 1 );	break;
		    case POS_FIGHTING:
			if ( number_bits(2) == 0 )
			    better_mental_state( ch, 1 );
			break;
		}
	    }
	    gain_condition( ch, COND_DRUNK,  -1 );
	    gain_condition( ch, COND_FULL,   -1 );
            if ( ch->class == CLASS_VAMPIRE && ch->level >= 10 )
	    {
		if ( time_info.hour < 21 && time_info.hour > 5 )
		  gain_condition( ch, COND_BLOODTHIRST, -1 );
	    }
	    if ( ch->in_room )
	      switch( ch->in_room->sector_type )
	      {
		default:
		    gain_condition( ch, COND_THIRST, -1 );  break;
		case SECT_DESERT:
		    gain_condition( ch, COND_THIRST, -2 );  break;
		case SECT_UNDERWATER:
		case SECT_OCEANFLOOR:
		    if ( number_bits(1) == 0 )
			gain_condition( ch, COND_THIRST, -1 );  break;
	      }

	}


	/* Hot Rooms --GW */
	if ( char_died(ch) )
	continue;

	if ( !IS_NPC(ch) && ch->in_room && !IS_IMMORTAL(ch) &&
		IS_SET(ch->in_room->room_flags, ROOM_HOT ) ) 
	{
		send_to_char("&RYou are Burning up!&w\n\r",ch);
		damage( ch, ch, 5, TYPE_UNDEFINED );
	}

	/* Cold Rooms --GW */
	if ( char_died(ch) )
	continue;

	if ( !IS_NPC(ch) && ch->in_room && !IS_IMMORTAL(ch) &&
		IS_SET(ch->in_room->room_flags, ROOM_COLD ) ) 
	{
		send_to_char("&CYou shiver, as the cold chills you to the Bone!&w\n\r",ch);
		damage( ch, ch, 5, TYPE_UNDEFINED );
	}
	
	if ( !char_died(ch) )
	{
	    /*
	     * Careful with the damages here,
	     *   MUST NOT refer to ch after damage taken,
	     *   as it may be lethal damage (on NPC).
	     */
	    if ( IS_AFFECTED(ch, AFF_POISON) )
	    {
		act( AT_POISON, "$n shivers and suffers.", ch, NULL, NULL, TO_ROOM );
		act( AT_POISON, "You shiver and suffer.", ch, NULL, NULL, TO_CHAR );
		ch->mental_state = URANGE( 20, ch->mental_state
				 + (IS_PKILL(ch) ? 3 : 4), 100 );
		damage( ch, ch, 6, gsn_poison );
	    if ( char_died(ch) )
		continue;
	    }
	    else
	    if ( ch->position == POS_INCAP )
		damage( ch, ch, 1, TYPE_UNDEFINED );
	    if ( char_died(ch) )
		continue;
	    if ( ch->position == POS_MORTAL )
		damage( ch, ch, 4, TYPE_UNDEFINED );
	    if ( char_died(ch) )
		continue;
	    if ( ch->mental_state >= 30 && !IS_NPC(ch))
		switch( (ch->mental_state+5) / 10 )
		{
		    case  3:
		    	send_to_char( "You feel feverish.\n\r", ch );
			act( AT_ACTION, "$n looks kind of out of it.", ch, NULL, NULL, TO_ROOM );
		    	break;
		    case  4:
		    	send_to_char( "You do not feel well at all.\n\r", ch );
			act( AT_ACTION, "$n doesn't look too good.", ch, NULL, NULL, TO_ROOM );
		    	break;
		    case  5:
		    	send_to_char( "You need help!\n\r", ch );
			act( AT_ACTION, "$n looks like $e could use your help.", ch, NULL, NULL, TO_ROOM );
		    	break;
		    case  6:
		    	send_to_char( "Seekest thou a cleric.\n\r", ch );
			act( AT_ACTION, "Someone should fetch a healer for $n.", ch, NULL, NULL, TO_ROOM );
		    	break;
		    case  7:
		    	send_to_char( "You feel reality slipping away...\n\r", ch );
			act( AT_ACTION, "$n doesn't appear to be aware of what's going on.", ch, NULL, NULL, TO_ROOM );
		    	break;
		    case  8:
		    	send_to_char( "You begin to understand... everything.\n\r", ch );
			act( AT_ACTION, "$n starts ranting like a madman!", ch, NULL, NULL, TO_ROOM );
		    	break;
		    case  9:
		    	send_to_char( "You are ONE with the universe.\n\r", ch );
			act( AT_ACTION, "$n is ranting on about 'the answer', 'ONE' and other mumbo-jumbo...", ch, NULL, NULL, TO_ROOM );
		    	break;
		    case 10:
		    	send_to_char( "You feel the end is near.\n\r", ch );
			act( AT_ACTION, "$n is muttering and ranting in tongues...", ch, NULL, NULL, TO_ROOM );
		    	break;
		}
	    if ( ch->mental_state <= -100 )
		switch( (abs(ch->mental_state)+5) / 10 )
		{
		    case  10:
			if ( ch->position > POS_SLEEPING )
			{
			   if ( (ch->position == POS_STANDING
			   ||    ch->position < POS_FIGHTING)
			   &&    number_percent()+10 < abs(ch->mental_state) )
				do_sleep( ch, "" );
			   else
				send_to_char( "You're barely conscious.\n\r", ch );
			}
			break;
		    case   9:
			if ( ch->position > POS_SLEEPING )
			{
			   if ( (ch->position == POS_STANDING
			   ||    ch->position < POS_FIGHTING)
			   &&   (number_percent()+20) < abs(ch->mental_state) )
				do_sleep( ch, "" );
			   else
				send_to_char( "You can barely keep your eyes open.\n\r", ch );
			}
			break;
		    case   8:
			if ( ch->position > POS_SLEEPING )
			{
			   if ( ch->position < POS_SITTING
			   &&  (number_percent()+30) < abs(ch->mental_state) )
				do_sleep( ch, "" );
			   else
				send_to_char( "You're extremely drowsy.\n\r", ch );
			}
			break;
		    case   7:
			if ( ch->position > POS_RESTING )
			   send_to_char( "You feel very unmotivated.\n\r", ch );
			break;
		    case   6:
			if ( ch->position > POS_RESTING )
			   send_to_char( "You feel sedated.\n\r", ch );
			break;
		    case   5:
			if ( ch->position > POS_RESTING )
			   send_to_char( "You feel sleepy.\n\r", ch );
			break;
		    case   4:
			if ( ch->position > POS_RESTING )
			   send_to_char( "You feel tired.\n\r", ch );
			break;
		    case   3:
			if ( ch->position > POS_RESTING )
			   send_to_char( "You could use a rest.\n\r", ch );
			break;
		}
	
	    /* 25 Minute warning --GW */
	    if ( ch->timer == 20 )
	    {
	      send_to_char("\a\aYou have been idle 20 Mins. You will be logged out if you\n\r",ch);
	      send_to_char("stay idle for another 5 minutes.\n\r",ch);
	    }

	    if ( ch->timer == 25 )
	    {
	        sprintf(buf,"%s is idle to long, disconnecting...",ch->name);
	        log_string(buf);
		do_quit2( ch, "" );
	    }
	    else
	    if ( ch == ch_save && IS_SET( sysdata.save_flags, SV_AUTO )
	    &&   ++save_count < 10 )	/* save max of 10 per tick */
		save_char_obj( ch );
	}
    }

/* Mobs here --GW */
for( in_zone = first_zone; in_zone; in_zone = in_zone->next )
{
    for ( ch = in_zone->last_mob; ch; ch = gch_prev )
    {

 /*----->*/   if ( !ch )
                break;


	if ( !IS_NPC(ch) )
	bug("PC Character in NPC Loop! Bad Bad!! (%s)",ch->name);

	if ( ch == in_zone->first_mob && ch->prev )
	{
	    bug( "char_update: first_char->prev != NULL... fixed", 0 );
	    ch->prev = NULL;
	}
	gch_prev = ch->prev;
	set_cur_char( ch );
	if ( gch_prev && gch_prev->next != ch )
	{
	    bug( "char_update: ch->prev->next != ch", 0 );
	    return;
	}

	if ( ch->stun_rounds > 0 && !ch->fighting )	
	ch->stun_rounds = 0;

     /*
      * Die on Tick for Sever --GW
      */

     if ( IS_NPC(ch) )
     {
        if(IS_SET(ch->acttwo,ACT2_SEVERED))
        {
            REMOVE_BIT(ch->acttwo,ACT2_SEVERED);
            act(AT_CARNAGE,"With a last gasp of breath, $n dies due to massive\nlower body trauma.",ch,NULL,NULL,TO_ROOM);
            raw_kill(ch,ch);
            continue;
        }  /*Be sure to define raw_kill up top of update.c*/
     }
 
        /*
	 *  Do a room_prog rand check right off the bat
	 *   if ch disappears (rprog might wax npc's), continue
	 */
	if(!IS_NPC(ch))
	    rprog_random_trigger( ch );

	if( char_died(ch) )
	    continue;

	if(IS_NPC(ch))
	    mprog_time_trigger(ch);   

	if( char_died(ch) )
	    continue;


	rprog_time_trigger(ch);

	if( char_died(ch) )
	    continue;

	if ( ch->position >= POS_STUNNED )
	{
	    if ( ch->hit  < ch->max_hit )
		ch->hit  += hit_gain(ch);

	    if ( ch->mana < ch->max_mana )
		ch->mana += mana_gain(ch);

	    if ( ch->move < ch->max_move )
		ch->move += move_gain(ch);
	}

	if ( ch->position == POS_STUNNED )
	    update_pos( ch );

	if ( !char_died(ch) )
	{
	    /*
	     * Careful with the damages here,
	     *   MUST NOT refer to ch after damage taken,
	     *   as it may be lethal damage (on NPC).
	     */
	    if ( IS_AFFECTED(ch, AFF_POISON) )
	    {
		act( AT_POISON, "$n shivers and suffers.", ch, NULL, NULL, TO_ROOM );
		act( AT_POISON, "You shiver and suffer.", ch, NULL, NULL, TO_CHAR );
		ch->mental_state = URANGE( 20, ch->mental_state
				 + (IS_PKILL(ch) ? 3 : 4), 100 );
		damage( ch, ch, 6, gsn_poison );
	    }
	    else
	    if ( ch->position == POS_INCAP )
		damage( ch, ch, 1, TYPE_UNDEFINED );
	    else
	    if ( ch->position == POS_MORTAL )
		damage( ch, ch, 4, TYPE_UNDEFINED );
	    if ( char_died(ch) )
     		continue;
      }
    }
}
    return;
}



/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update( void )
{   
    OBJ_DATA *obj;
    sh_int AT_TEMP;
    ZONE_DATA *in_zone;
   record_call("<obj_update>");

for( in_zone = first_zone; in_zone; in_zone = in_zone->next )
{
    for ( obj = in_zone->last_obj; obj; obj = gobj_prev )
    {
	CHAR_DATA *rch;
	char *message;

	if ( obj == in_zone->first_obj && obj->prev )
	{
	    bug( "obj_update: first_object->prev != NULL... fixed", 0 );
	    obj->prev = NULL;
	}
	gobj_prev = obj->prev;
	if ( gobj_prev && gobj_prev->next != obj )
	{
	    bug( "obj_update: obj->prev->next != obj", 0 );
	    return;
	}
	set_cur_obj( obj );
	if ( obj->carried_by )
	  oprog_random_trigger( obj ); 
	else
	if( obj->in_room && obj->in_room->area->nplayer > 0 )
	  oprog_random_trigger( obj ); 

        if( obj_extracted(obj) )
	  continue;

	if ( obj->item_type == ITEM_PIPE )
	{
	    if ( IS_SET( obj->value[3], PIPE_LIT ) )
	    {
		if ( --obj->value[1] <= 0 )
		{
		  obj->value[1] = 0;
		  REMOVE_BIT( obj->value[3], PIPE_LIT );
		}
		else
		if ( IS_SET( obj->value[3], PIPE_HOT ) )
		  REMOVE_BIT( obj->value[3], PIPE_HOT );
		else
		{
		  if ( IS_SET( obj->value[3], PIPE_GOINGOUT ) )
		  {
		    REMOVE_BIT( obj->value[3], PIPE_LIT );
		    REMOVE_BIT( obj->value[3], PIPE_GOINGOUT );
		  }
		  else
		    SET_BIT( obj->value[3], PIPE_GOINGOUT );
		}
		if ( !IS_SET( obj->value[3], PIPE_LIT ) )
		  SET_BIT( obj->value[3], PIPE_FULLOFASH );
	    }
	    else
	      REMOVE_BIT( obj->value[3], PIPE_HOT );
	}


/* Corpse decay (npc corpses decay at 8 times the rate of pc corpses) - Narn */

        if ( obj->item_type == ITEM_CORPSE_PC || obj->item_type == ITEM_CORPSE_NPC )
        {
          sh_int timerfrac = UMAX(1, obj->timer - 1);
          if ( obj->item_type == ITEM_CORPSE_PC )
	    timerfrac = (int)(obj->timer / 8 + 1);

	  if ( obj->timer > 0 && obj->value[2] > timerfrac )
	  {
            char buf[MAX_STRING_LENGTH];
            char name[MAX_STRING_LENGTH];
            char *bufptr;
            bufptr = one_argument( obj->short_descr, name ); 
            bufptr = one_argument( bufptr, name ); 
            bufptr = one_argument( bufptr, name ); 

	    separate_obj(obj);
            obj->value[2] = timerfrac; 
            sprintf( buf, corpse_descs[ UMIN( timerfrac - 1, 4 ) ], 
                          capitalize( bufptr ) ); 

            STRFREE( obj->description );
            obj->description = STRALLOC( buf ); 
          }  
        }
       
	/* don't let inventory decay */
	if ( IS_OBJ_STAT(obj, ITEM_INVENTORY) )
	  continue;

	if ( ( obj->timer <= 0 || --obj->timer > 0 ) )
	  continue;
	
	/* if we get this far, object's timer has expired. */
        /* support for underwater sectors added --GW*/ 
         AT_TEMP = AT_PLAIN;
	 switch ( obj->item_type )
	 {
	 default:
	   message = "$p mysteriously vanishes.";
           AT_TEMP = AT_PLAIN;
	   break;
	 case ITEM_PORTAL:
	   message = "$p winks out of existence.";
           remove_portal(obj);
	   obj->item_type = ITEM_TRASH;		/* so extract_obj	 */
           AT_TEMP = AT_MAGIC;			/* doesn't remove_portal */
	   break;
	 case ITEM_FOUNTAIN:
	   if ( obj->in_room && ( obj->in_room->sector_type == 8 ) )
	   {
	     message = "$p is overtaken by the saltwater around it";
	     AT_TEMP = AT_BLUE;
	     break;
	   }
	   message = "$p dries up.";
           AT_TEMP = AT_BLUE;
	   break;
	 case ITEM_CORPSE_NPC:
	   if ( obj->in_room && ( obj->in_room->sector_type == 8 ) )
	   {
	     message = "$p is carried off into the currents of the sea..";
	     AT_TEMP = AT_OBJECT;
	     break;
	   }
	   message = "$p decays into dust and blows away.";
           AT_TEMP = AT_OBJECT;
	   break;
	 case ITEM_CORPSE_PC:
	   if ( obj->in_room && ( obj->in_room->sector_type == 8 ) )
	   {
	     message = "$p fills up with water and disappears into the depths...";
	     AT_TEMP = AT_MAGIC;
	     break;
	   }
	   message = "$p is sucked into a swirling vortex of colors...";
           AT_TEMP = AT_MAGIC;
	   break;
	 case ITEM_FOOD:
	   if ( obj->in_room && ( obj->in_room->sector_type == 8 ) )
	   {
	     message = "$p is devoured by a school of fish..";
	     AT_TEMP = AT_HUNGRY;
	     break;
	   }
	   message = "$p is devoured by a swarm of maggots.";
           AT_TEMP = AT_HUNGRY;
	   break;
         case ITEM_BLOOD:
	   if ( obj->in_room && ( obj->in_room->sector_type == 8 ) )
	   {
	     message = "$p clouds the water and fades from view";
	     AT_TEMP = AT_BLOOD;
	     break;
	   }
           message = "$p slowly seeps into the ground.";
           AT_TEMP = AT_BLOOD;
           break;
         case ITEM_BLOODSTAIN:
	   if ( obj->in_room && ( obj->in_room->sector_type == 8 ) )
	   {
	     message = "$p diloutes and is carried off into the currents.";
	     AT_TEMP = AT_BLOOD;
	     break;
	   }
           message = "$p dries up into flakes and blows away.";
           AT_TEMP = AT_BLOOD;
	   break;
         case ITEM_SCRAPS:
	   if ( obj->in_room && ( obj->in_room->sector_type == 8 ) )
	   {
	     message = "$p slowly sinks to the bottom of the sea..";
	     AT_TEMP = AT_OBJECT;
	     break;
	   }
           message = "$p crumbles and decays into nothing.";
           AT_TEMP = AT_OBJECT;
	   break;
	 case ITEM_FIRE:
	   if ( obj->in_room && ( obj->in_room->sector_type == 8 ) )
	   {
	     message = "$p is finally extinguished by the water surrounding it.";
	     AT_TEMP = AT_FIRE;
	     break;
	   }
	   if (obj->in_room)
	     --obj->in_room->light;
	   message = "$p burns out.";
	   AT_TEMP = AT_FIRE;
	 }

	if ( obj->carried_by )
	{
	    act( AT_TEMP, message, obj->carried_by, obj, NULL, TO_CHAR );
	}
	else if ( obj->in_room
	&&      ( rch = obj->in_room->first_person ) != NULL
	/*&&	!IS_OBJ_STAT( obj, ITEM_BURRIED )*/ )
	{
	    act( AT_TEMP, message, rch, obj, NULL, TO_ROOM );
            act( AT_TEMP, message, rch, obj, NULL, TO_CHAR );
	}

	if ( obj->serial == cur_obj )
	  global_objcode = rOBJ_EXPIRED;

       /* Objects inside shouldnt be destroyed, make them fall on the floor, or
          into the characters inventory */
        if ( obj->first_content )
        {
 	  OBJ_DATA *inside_obj, *next_inside;

          for( inside_obj=obj->first_content; inside_obj; inside_obj = next_inside )
          {
            next_inside = inside_obj->next_content;
            obj_from_obj( inside_obj );

	    if ( obj->carried_by )
              obj_to_char( inside_obj, obj->carried_by );
            else
              obj_to_room( inside_obj, obj->in_room );
          }
        }

	extract_obj( obj );
    }
}
    return;
}


/*
 * Function to check important stuff happening to a player
 * This function should take about 5% of mud cpu time
 */
void char_check( void )
{
    CHAR_DATA *ch, *ch_next, *test_char;
    DESCRIPTOR_DATA *d;
    OBJ_DATA *obj;
    EXIT_DATA *pexit;
    static int cnt = 0;
    int door, retcode;
    ZONE_DATA *in_zone;
   record_call("<char_check>");

    cnt = (cnt+1) % 2;

/* Want to Verify the list is all right first --GW */
         for ( d = first_descriptor; d; d = d->next )
         {
                test_char = NULL;
                if ( (  d->connected == CON_PLAYING || d->connected == CON_EDITING )
                && ( ( test_char = get_char_list(d->character) )==NULL ) )
                {
                bug("Char Found NOT IN LIST!! (%s)!",d->character->name);
                bug("Linking...",0);
                add_char(d->character);
                }
        }

    for ( ch = first_char; ch; ch = ch_next )
    {

	if ( !ch)
	continue;
	
        if (!ch->in_room)
        char_to_room( ch, get_room_index(ROOM_VNUM_ALTAR,1));

	set_cur_char(ch);
	ch_next = ch->next;

	will_fall(ch, 0);


	if ( char_died( ch ) )
	  continue;

	if ( IS_NPC( ch ) )
	{
	    if ( cnt != 0 )
		continue;

	    /* running mobs	-Thoric */
	    if ( IS_SET(ch->act, ACT_RUNNING) )
	    {
		if ( !IS_SET( ch->act, ACT_SENTINEL )
		&&   !ch->fighting && ch->hunting )
		{
		    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );
		    hunt_victim( ch );
		    continue;
		}

		if ( ch->spec_fun )
		{
		    if ( (*ch->spec_fun) ( ch ) )
			continue;
		    if ( char_died(ch) )
			continue;
		}

		if ( !IS_SET(ch->act, ACT_SENTINEL)
		&&   !IS_SET(ch->act, ACT_PROTOTYPE)
		&& ( door = number_bits( 4 ) ) <= 9
		&& ( pexit = get_exit(ch->in_room, door) ) != NULL
		&&   pexit->to_room
		&&   !IS_SET(pexit->exit_info, EX_CLOSED)
		&&   !IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB)
		&&   !IS_SET(pexit->to_room->room_flags, ROOM_DEATH)
		&& ( !IS_SET(ch->act, ACT_STAY_AREA)
		||   pexit->to_room->area == ch->in_room->area ) )
		{
		    retcode = move_char( ch, pexit, 0, FALSE );
		    if ( char_died(ch) )
			continue;
		    if ( retcode != rNONE || IS_SET(ch->act, ACT_SENTINEL)
		    ||    ch->position < POS_STANDING )
			continue;
		}
	    }
	    continue;
	}
	else
	{
	    if ( ch && ch->mount && ch->in_room && ch->mount->in_room
	    &&   ch->in_room != ch->mount->in_room )
	    {
		REMOVE_BIT( ch->mount->act, ACT_MOUNTED );
		ch->mount = NULL;
		ch->position = POS_STANDING;
		send_to_char( "No longer upon your mount, you fall to the ground...\n\r",ch);
                send_to_char("OUCH!\n\r", ch );
	    }

	    if ( ( ch->in_room && ch->in_room->sector_type == SECT_UNDERWATER )
	    || ( ch->in_room && ch->in_room->sector_type == SECT_OCEANFLOOR ) )
	    {
		if ( !IS_AFFECTED( ch, AFF_AQUA_BREATH ) )
		{
		    if ( ch->level < LEVEL_IMMORTAL )
		    {
			int dam;

        		/* Changed level of damage at Brittany's request. -- Narn */	
			dam = number_range( ch->max_hit / 100, ch->max_hit / 50 );
			dam = UMAX( 1, dam );
			if ( number_bits(3) == 0 )
			  send_to_char( "You cough and choke as you try to breathe water!\n\r", ch );
			damage( ch, ch, dam, TYPE_UNDEFINED );
		    }
		}
	    }
	
	    if ( char_died( ch ) )
		continue; 

	    if ( ch->in_room
	    && (( ch->in_room->sector_type == SECT_WATER_NOSWIM )
	    ||  ( ch->in_room->sector_type == SECT_WATER_SWIM ) ) )
	    {
		if ( !IS_AFFECTED( ch, AFF_FLYING )
		&& !IS_AFFECTED( ch, AFF_FLOATING ) 
		&& !IS_AFFECTED( ch, AFF_AQUA_BREATH )
		&& !ch->mount )
		{
		    for ( obj = ch->first_carrying; obj; obj = obj->next_content )
		      if ( obj->item_type == ITEM_BOAT ) break;

		    if ( !obj )
		    {
			if ( ch->level < LEVEL_IMMORTAL )
			{
			    int mov;
			    int dam;

			    if ( ch->move > 0 )
			    {
				mov = number_range( ch->max_move / 20, ch->max_move / 5 );
				mov = UMAX( 1, mov );

				if ( ch->move - mov < 0 )
				    ch->move = 0;
				else
				    ch->move -= mov;
			    }
			    else
			    {
				dam = number_range( ch->max_hit / 20, ch->max_hit / 5 );
				dam = UMAX( 1, dam );

				if ( number_bits(3) == 0 )
				   send_to_char( "Struggling with exhaustion, you choke on a mouthful of water.\n\r", ch );
				damage( ch, ch, dam, TYPE_UNDEFINED );
			    }
			}
		    }
		}
	    }

	    /* beat up on link dead players */


	if ( !IS_NPC(ch) && ch && ch->level >= 2)
	{	
	    if ( !ch->desc )
	    {
		CHAR_DATA *wch, *wch_next;
	   

		for ( wch = ch->in_room->first_person; wch; wch = wch_next )
		{
		    if (!wch)
        		break;

		    wch_next	= wch->next_in_room;

		    if ( (ch && wch) && ( !IS_NPC(wch)
		    ||   wch->fighting 
		    ||   IS_AFFECTED(wch, AFF_CHARM)
		    ||   !IS_AWAKE(wch)
		    || ( IS_SET(wch->act, ACT_WIMPY) && IS_AWAKE(ch) )
		    ||   !can_see( wch, ch ) ) )
			continue;

		    if ( ch && is_hating( wch, ch ) )
		    {
			found_prey( wch, ch );
			continue;
		    }

		    if ( wch && ( !IS_SET(wch->act, ACT_AGGRESSIVE)
		    ||    IS_SET(wch->act, ACT_MOUNTED)
		    ||    IS_SET(wch->in_room->room_flags, ROOM_SAFE ) ) )
			continue;
		    global_retcode = multi_hit( wch, ch, TYPE_UNDEFINED );
		}
	    }
	}
    }
}

for( in_zone = first_zone; in_zone; in_zone = in_zone->next )
{
    for ( ch = in_zone->first_mob; ch; ch = ch_next )
    {

	if ( !ch)
	continue;

	set_cur_char(ch);
	ch_next = ch->next;

	if ( !ch->in_room )
	continue;

	will_fall(ch, 0);


	if ( char_died( ch ) )
	  continue;

	if ( IS_NPC( ch ) )
	{
	    if ( cnt != 0 )
		continue;

	    /* running mobs	-Thoric */
	    if ( IS_SET(ch->act, ACT_RUNNING) )
	    {
		if ( !IS_SET( ch->act, ACT_SENTINEL )
		&&   !ch->fighting && ch->hunting )
		{
		    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );
		    hunt_victim( ch );
		    continue;
		}

		if ( ch->spec_fun )
		{
		    if ( (*ch->spec_fun) ( ch ) )
			continue;
		    if ( char_died(ch) )
			continue;
		}

		if ( !IS_SET(ch->act, ACT_SENTINEL)
		&&   !IS_SET(ch->act, ACT_PROTOTYPE)
		&& ( door = number_bits( 4 ) ) <= 9
		&& ( pexit = get_exit(ch->in_room, door) ) != NULL
		&&   pexit->to_room
		&&   !IS_SET(pexit->exit_info, EX_CLOSED)
		&&   !IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB)
		&&   !IS_SET(pexit->to_room->room_flags, ROOM_DEATH)
		&& ( !IS_SET(ch->act, ACT_STAY_AREA)
		||   pexit->to_room->area == ch->in_room->area ) )
		{
		    retcode = move_char( ch, pexit, 0, FALSE );
		    if ( char_died(ch) )
			continue;
		    if ( retcode != rNONE || IS_SET(ch->act, ACT_SENTINEL)
		    ||    ch->position < POS_STANDING )
			continue;
		}
	    }
	    continue;
	}
	else
	{
	    if ( ch && ch->mount && ch->in_room && ch->mount->in_room
	    &&   ch->in_room != ch->mount->in_room )
	    {
		REMOVE_BIT( ch->mount->act, ACT_MOUNTED );
		ch->mount = NULL;
		ch->position = POS_STANDING;
		send_to_char( "No longer upon your mount, you fall to the ground...\n\r",ch);
                send_to_char("OUCH!\n\r", ch );
	    }

	    if ( ( ch->in_room && ch->in_room->sector_type == SECT_UNDERWATER )
	    || ( ch->in_room && ch->in_room->sector_type == SECT_OCEANFLOOR ) )
	    {
		if ( !IS_AFFECTED( ch, AFF_AQUA_BREATH ) )
		{
		    if ( ch->level < LEVEL_IMMORTAL )
		    {
			int dam;

        		/* Changed level of damage at Brittany's request. -- Narn */	
			dam = number_range( ch->max_hit / 100, ch->max_hit / 50 );
			dam = UMAX( 1, dam );
			if ( number_bits(3) == 0 )
			  send_to_char( "You cough and choke as you try to breathe water!\n\r", ch );
			damage( ch, ch, dam, TYPE_UNDEFINED );
		    }
		}
	    }
	
	    if ( char_died( ch ) )
		continue; 

	    if ( ch->in_room
	    && (( ch->in_room->sector_type == SECT_WATER_NOSWIM )
	    ||  ( ch->in_room->sector_type == SECT_WATER_SWIM ) ) )
	    {
		if ( !IS_AFFECTED( ch, AFF_FLYING )
		&& !IS_AFFECTED( ch, AFF_FLOATING ) 
		&& !IS_AFFECTED( ch, AFF_AQUA_BREATH )
		&& !ch->mount )
		{
		    for ( obj = ch->first_carrying; obj; obj = obj->next_content )
		      if ( obj->item_type == ITEM_BOAT ) break;

		    if ( !obj )
		    {
			if ( ch->level < LEVEL_IMMORTAL )
			{
			    int mov;
			    int dam;

			    if ( ch->move > 0 )
			    {
				mov = number_range( ch->max_move / 20, ch->max_move / 5 );
				mov = UMAX( 1, mov );

				if ( ch->move - mov < 0 )
				    ch->move = 0;
				else
				    ch->move -= mov;
			    }
			    else
			    {
				dam = number_range( ch->max_hit / 20, ch->max_hit / 5 );
				dam = UMAX( 1, dam );

				if ( number_bits(3) == 0 )
				   send_to_char( "Struggling with exhaustion, you choke on a mouthful of water.\n\r", ch );
				damage( ch, ch, dam, TYPE_UNDEFINED );
			    }
			}
		    }
		}
	    }

	    /* beat up on link dead players */


	if ( !IS_NPC(ch) && ch && ch->level >= 2)
	{	
	    if ( !ch->desc )
	    {
		CHAR_DATA *wch, *wch_next;
	   

		for ( wch = ch->in_room->first_person; wch; wch = wch_next )
		{
		    if (!wch)
        		break;

		    wch_next	= wch->next_in_room;

		    if ( (ch && wch) && ( !IS_NPC(wch)
		    ||   wch->fighting 
		    ||   IS_AFFECTED(wch, AFF_CHARM)
		    ||   !IS_AWAKE(wch)
		    || ( IS_SET(wch->act, ACT_WIMPY) && IS_AWAKE(ch) )
		    ||   !can_see( wch, ch ) ) )
			continue;

		    if ( ch && is_hating( wch, ch ) )
		    {
			found_prey( wch, ch );
			continue;
		    }

		    if ( wch && ( !IS_SET(wch->act, ACT_AGGRESSIVE)
		    ||    IS_SET(wch->act, ACT_MOUNTED)
		    ||    IS_SET(wch->in_room->room_flags, ROOM_SAFE ) ) )
			continue;
		    global_retcode = multi_hit( wch, ch, TYPE_UNDEFINED );
		}
	    }
	}
    }
}
}
}

/*
 * Aggress.
 *
 * for each descriptor
 *     for each mob in room
 *         aggress on some random PC
 *
 * This function should take 5% to 10% of ALL mud cpu time.
 * Unfortunately, checking on each PC move is too tricky,
 *   because we don't the mob to just attack the first PC
 *   who leads the party into the room.
 *
 */
void aggr_update( void )
{
    CHAR_DATA *wch;
    CHAR_DATA *ch, *next_ch;
    struct act_prog_data *apdtmp;
    record_call("<aggr_update>");

#ifdef UNDEFD
  /*
   *  GRUNT!  To do
   *
   */
        if ( IS_NPC( wch ) && wch->mpactnum > 0
	    && wch->in_room->area->nplayer > 0 )
        {
            MPROG_ACT_LIST * tmp_act, *tmp2_act;
	    for ( tmp_act = wch->mpact; tmp_act;
		 tmp_act = tmp_act->next )
	    {
                 oprog_wordlist_check( tmp_act->buf,wch, tmp_act->ch,
				      tmp_act->obj, tmp_act->vo, ACT_PROG );
                 DISPOSE( tmp_act->buf );
            }
	    for ( tmp_act = wch->mpact; tmp_act; tmp_act = tmp2_act )
	    {
                 tmp2_act = tmp_act->next;
                 DISPOSE( tmp_act );
            }
            wch->mpactnum = 0;
            wch->mpact    = NULL;
        }
#endif

    /* check mobprog act queue */
    while ( (apdtmp = mob_act_list) != NULL )
    {
	wch = mob_act_list->vo;
	if ( !char_died(wch) && wch->mpactnum > 0 )
	{
	    MPROG_ACT_LIST * tmp_act;

	    while ( (tmp_act = wch->mpact) != NULL )
	    {
		if ( tmp_act->obj && obj_extracted(tmp_act->obj) )
		  tmp_act->obj = NULL;
		if ( tmp_act->ch && !char_died(tmp_act->ch) )
		  mprog_wordlist_check( tmp_act->buf, wch, tmp_act->ch,
					tmp_act->obj, tmp_act->vo, ACT_PROG );
		wch->mpact = tmp_act->next;
		DISPOSE(tmp_act->buf);
		DISPOSE(tmp_act);
	    }
	    wch->mpactnum = 0;
	    wch->mpact    = NULL;
        }
	mob_act_list = apdtmp->next;
	DISPOSE( apdtmp );
    }

for ( ch = first_char; ch; ch = next_ch )
{
      CHAR_DATA *aggnpc, *next_aggnpc;
      CHAR_DATA *attpc, *next_attpc;
      CHAR_DATA *attackme=NULL, *last_char=NULL;
      int rand=0;
      bool attacked=FALSE;
      bool pc_found=FALSE;

next_ch = ch->next;

if ( !ch->in_room )
{
  bug("AGG_UPDATE: Null ch->room (%s) - Keep an EYE!",ch->name);
  continue;
}

       for ( aggnpc = ch->in_room->first_person; aggnpc; aggnpc = next_aggnpc )
       {
         next_aggnpc = aggnpc->next_in_room;


            if ( ch != aggnpc && is_hating( ch, aggnpc ) )
            {
                found_prey( ch, aggnpc );
                continue;
            }

         /*Are We in a Safe Room? */
         if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
           break;

         /* Check for Aggro NPC's First, if there isnt any, donot bother*/
        if ( IS_NPC(aggnpc) &&                         /* Mob? */
              IS_SET(aggnpc->act, ACT_AGGRESSIVE) &&    /* Aggro? */
              IS_AWAKE(aggnpc) &&                       /* Awake? */
	      aggnpc->position >= POS_STANDING &&	/* Position Check!*/
	      !aggnpc->fighting &&		   /* We are not fighting!*/
              !IS_AFFECTED(aggnpc,AFF_CHARM) )     /* Not Charmed?*/
         {
            for( attpc = aggnpc->in_room->first_person; attpc; attpc = next_attpc )
            {
              next_attpc = attpc->next_in_room;

                if ( !IS_NPC(attpc) &&          /* Not a Mob */
                     IS_AWAKE(attpc) &&         /* We are Awake */
                     !IS_IMMORTAL(attpc) &&     /* Not a God */
                     attpc->desc &&             /* We are Linked to the Mud.. */
                     !IS_AFFECTED(attpc, AFF_CHARM) && /* PC Not Charmed  */
                     can_see(aggnpc,attpc) )         /* We Can see the PC? */
                   {
                        pc_found = TRUE;
                        last_char = attpc;
                        if ( ( rand = number_range( 0, 3 ) )==2 ) /* Attack Randomly */
                        {
                        attacked = TRUE;
                        attackme = attpc;
                        }
                   }/* if */
            }/* for */

                        /* DONOT Continue if there is No Pc's
                         * In the Room! --GW
                         */
                        if ( !pc_found )
                           break;

                        /*
                         * Attack Sequence
                         */

                        /* Ok .. lets make SURE We attack Something here..*/
                        if ( !attacked )
                           attackme = last_char;

            if ( IS_SET(aggnpc->attacks,ATCK_BACKSTAB ) )
            {
                OBJ_DATA *obj;

                if ( !aggnpc->mount
                && (obj = get_eq_char( aggnpc, WEAR_WIELD )) != NULL
                && obj->value[3] == 11
                && !attackme->fighting
                && attackme->hit >= attackme->max_hit )
                {
                    WAIT_STATE( aggnpc, skill_table[gsn_backstab]->beats);
                    if ( !IS_AWAKE(attackme)
                    ||   number_percent( )+5 < aggnpc->level )
                    {
                        global_retcode = multi_hit( aggnpc, attackme, gsn_backstab );
                        continue;
                    }
                    else
                    {
                        global_retcode = damage( aggnpc, attackme, 0, gsn_backstab );
                        continue;
                    }
                } /* if */
             }/* if */
            global_retcode = multi_hit( aggnpc, attackme, TYPE_UNDEFINED);
   } /* for */
  } /*if */
} 
    return;
}

/* From interp.c */
bool check_social  args( ( CHAR_DATA *ch, char *command, char *argument ) );

/*
 * drunk randoms	- Tricops
 * (Made part of mobile_update	-Thoric)
 */
void drunk_randoms( CHAR_DATA *ch )
{
    CHAR_DATA *rvch = NULL;
    CHAR_DATA *vch;
    sh_int drunk;
    sh_int position;
   record_call("<drunk_randoms>");

    if (!ch)
	return;
   
    if (!IS_NPC(ch) && !ch->desc )
	return;

    if (!IS_NPC(ch) && (ch->desc->connected && ch->desc->connected != CON_PLAYING) )
	return;

    if ( IS_NPC( ch ) || ch->pcdata->condition[COND_DRUNK] <= 0 )
	return;

    if ( number_percent() < 30 )
	return;

    drunk = ch->pcdata->condition[COND_DRUNK];
    position = ch->position;
    ch->position = POS_STANDING;

    if ( number_percent() < (2*drunk / 20) )
	check_social( ch, "burp", "" );
    else
    if ( number_percent() < (2*drunk / 20) )
	check_social( ch, "hiccup", "" );
    else
    if ( number_percent() < (2*drunk / 20) )
	check_social( ch, "drool", "" );
    else
    if ( number_percent() < (2*drunk / 20) )
	check_social( ch, "fart", "" );
    else
    if ( drunk > (10+(get_curr_con(ch)/5))
    &&   number_percent() < ( 2 * drunk / 18 ) )
    {
	for ( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
	    if ( number_percent() < 10 )
		rvch = vch;
	check_social( ch, "puke", (rvch ? rvch->name : "") );
    }

    ch->position = position;
    return;
}

void halucinations( CHAR_DATA *ch )
{
record_call("<halucinations>");
    if ( ch->mental_state >= 30 && number_bits(5 - (ch->mental_state >= 50) - (ch->mental_state >= 75)) == 0 )
    {
	char *t;

	switch( number_range( 1, UMIN(20, (ch->mental_state+5) / 5)) )
	{
	    default:
	    case  1: t = "You feel very restless... you can't sit still.\n\r";		break;
	    case  2: t = "You're tingling all over.\n\r";				break;
	    case  3: t = "Your skin is crawling.\n\r";					break;
	    case  4: t = "You suddenly feel that something is terribly wrong.\n\r";	break;
	    case  5: t = "Those damn little fairies keep laughing at you!\n\r";		break;
	    case  6: t = "You can hear your mother crying...\n\r";			break;
	    case  7: t = "Have you been here before, or not?  You're not sure...\n\r";	break;
	    case  8: t = "Painful childhood memories flash through your mind.\n\r";	break;
	    case  9: t = "You hear someone call your name in the distance...\n\r";	break;
	    case 10: t = "Your head is pulsating... you can't think straight.\n\r";	break;
	    case 11: t = "The ground... seems to be squirming...\n\r";			break;
	    case 12: t = "You're not quite sure what is real anymore.\n\r";		break;
	    case 13: t = "It's all a dream... or is it?\n\r";				break;
	    case 14: t = "They're coming to get you... coming to take you away...\n\r";	break;
	    case 15: t = "You begin to feel all powerful!\n\r";				break;
	    case 16: t = "You're light as air... the heavens are yours for the taking.\n\r";	break;
	    case 17: t = "Your whole life flashes by... and your future...\n\r";	break;
	    case 18: t = "You are everywhere and everything... you know all and are all!\n\r";	break;
	    case 19: t = "You feel immortal!\n\r";					break;
	    case 20: t = "Ahh... the power of a Supreme Entity... what to do...\n\r";	break;
	}
	send_to_char( t, ch );
    }
    return;
}

void tele_update( void )
{
    TELEPORT_DATA *tele, *tele_next;

    if ( !first_teleport )
      return;
    
    for ( tele = first_teleport; tele; tele = tele_next )
    {
	tele_next = tele->next;
	if ( --tele->timer <= 0 )
	{
	    if ( tele->room->first_person )
	    {
		if ( IS_SET( tele->room->room_flags, ROOM_TELESHOWDESC ) )
		  teleport( tele->room->first_person, tele->room->tele_vnum,
			TELE_SHOWDESC | TELE_TRANSALL );
		else
		  teleport( tele->room->first_person, tele->room->tele_vnum,
			TELE_TRANSALL );
   record_call("tele_update>");

	    }
	    UNLINK( tele, first_teleport, last_teleport, next, prev );
	    DISPOSE( tele );
	}
    }
}

#if FALSE
/* 
 * Write all outstanding authorization requests to Log channel - Gorog
 */ 
void auth_update( void ) 
{ 
  CHAR_DATA *victim; 
  DESCRIPTOR_DATA *d; 
  char log_buf [MAX_INPUT_LENGTH];
  bool first_time = TRUE;         /* so titles are only done once */
   record_call("<auth_update>>");

  for ( d = first_descriptor; d; d = d->next ) 
      {
      victim = d->character;
      if ( victim && IS_WAITING_FOR_AUTH(victim) )
         {
         if ( first_time )
            {
            first_time = FALSE;
            strcpy (log_buf, "Pending authorizations:" ); 
            log_string( log_buf ); 
            to_channel( log_buf, CHANNEL_MONITOR, "[ ** ", 1);
            }
         sprintf( log_buf, " %s@%s new %s %s", victim->name,
            victim->desc->host, race_table[victim->race].race_name, 
            class_table[victim->class]->who_name ); 
         log_string( log_buf ); 
         to_channel( log_buf, CHANNEL_MONITOR, "[ ** ", 1);
         }
      }
} 
#endif

void auth_update( void ) 
{ 
    CHAR_DATA *victim; 
    DESCRIPTOR_DATA *d; 
    char buf [MAX_INPUT_LENGTH], log_buf [MAX_INPUT_LENGTH];
    bool found_hit = FALSE;         /* was at least one found? */
   record_call("<auth_update>");
    strcpy (log_buf, "Pending authorizations:\n\r" );
    for ( d = first_descriptor; d; d = d->next ) 
    {
	if ( (victim = d->character) && IS_WAITING_FOR_AUTH(victim) )
	{
	    found_hit = TRUE;
	    sprintf( buf, " %s@%s new %s %s\n\r", victim->name,
		victim->desc->host, race_table[victim->race]->race_name, 
		class_table[victim->class]->who_name );
	    strcat (log_buf, buf);
	}
    }
    if (found_hit)
    {
	log_string( log_buf ); 
	to_channel( log_buf, CHANNEL_MONITOR, "[ ** ", 1);
    }
} 

/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */
void update_handler( void )
{
    static  int     pulse_area;
    static  int     pulse_mobile;
    static  int     pulse_violence;
    static  int     pulse_point;
    static  int	    pulse_second;
    struct timeval stime;
    struct timeval etime;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *test_char;
    record_call("<update_handler>");

    if ( timechar )
    {
      set_char_color(AT_PLAIN, timechar);
      send_to_char( "Starting update timer.\n\r", timechar );
      gettimeofday(&stime, NULL);
    }
    
      inroom_update();

    if ( --pulse_area     <= 0 )
    {
	pulse_area	= number_range( PULSE_AREA / 2, 3 * PULSE_AREA / 2 );
	area_update	( );
    /* Quest updating --GW*/
	quest_update();
    }

    if ( --pulse_mobile   <= 0 )
    {
	pulse_mobile	= PULSE_MOBILE;
	mobile_update  ( );
    }

    if ( --pulse_violence <= 0 )
    {
	pulse_violence	= PULSE_VIOLENCE;
	violence_update	( );
    }

    if ( --pulse_point    <= 0 )
    {
	pulse_point     = number_range( PULSE_TICK * 0.75, PULSE_TICK * 1.25 );

        auth_update     ( );			/* Gorog */
	weather_update	( );
	char_update	( );
	obj_update	( );
	clear_vrooms	( );			/* remove virtual rooms */
    }

    if ( --pulse_second   <= 0 )
    {
	pulse_second	= PULSE_PER_SECOND;
	char_check( );
	reboot_check(0);
    }

    if ( auction->item && --auction->pulse <= 0 )
    {                                                  
	auction->pulse = PULSE_AUCTION;                     
	auction_update( );
    }

	/* Aliens --GW */
	aliens_update( );

        /* Dragon Abductions */
        if ( DRAGON_DROP )
	dragon_update( );

         for ( d = first_descriptor; d; d = d->next )
         {
                test_char = NULL;
                if ( (  d->connected == CON_PLAYING || d->connected == CON_EDITING )
                && ( ( test_char = get_char_list(d->character) )==NULL ) )
                {
                bug("Char Found NOT IN LIST!! (%s)!",d->character->name);
                bug("Linking...",0);
                add_char(d->character);
                }
        }


        aggr_update( );/* Attempting to do this better --GW */
    tele_update( );
    /* Still need it cause it checks the MPROG_ACT Queue */
    obj_act_update ( );
    room_act_update( );
    clean_obj_queue();		/* dispose of extracted objects */
    clean_char_queue();		/* dispose of dead mobs/quitting chars */
    if ( timechar )
    {
      gettimeofday(&etime, NULL);
      set_char_color(AT_PLAIN, timechar);
      send_to_char( "Update timing complete.\n\r", timechar );
      subtract_times(&etime, &stime);
      ch_printf( timechar, "Timing took %d.%06d seconds.\n\r",
          etime.tv_sec, etime.tv_usec );
      timechar = NULL;
    }
    tail_chain( );
    return;
}


void remove_portal( OBJ_DATA *portal )
{
    ROOM_INDEX_DATA *fromRoom, *toRoom;
    CHAR_DATA *ch;
    EXIT_DATA *pexit;
    bool found;
   record_call("<remove_portal>");
    if ( !portal )
    {
	bug( "remove_portal: portal is NULL", 0 );
	return;
    }

    fromRoom = portal->in_room;
    found = FALSE;
    if ( !fromRoom )
    {
	bug( "remove_portal: portal->in_room is NULL", 0 );
	return;
    }

    for ( pexit = fromRoom->first_exit; pexit; pexit = pexit->next )
	if ( IS_SET( pexit->exit_info, EX_PORTAL ) )
	{
	    found = TRUE;
	    break;
	}

    if ( !found )
    {
	bug( "remove_portal: portal not found in room %d!", fromRoom->vnum );
	return;
    }

    if ( pexit->vdir != DIR_PORTAL )
	bug( "remove_portal: exit in dir %d != DIR_PORTAL", pexit->vdir );

    if ( ( toRoom = pexit->to_room ) == NULL )
      bug( "remove_portal: toRoom is NULL", 0 );
 
    extract_exit( fromRoom, pexit );
    /* rendunancy */
    /* send a message to fromRoom */
    /* ch = fromRoom->first_person; */
    /* if(ch!=NULL) */
    /* act( AT_PLAIN, "A magical portal below winks from existence.", ch, NULL, NULL, TO_ROOM ); */

    /* send a message to toRoom */
    if ( toRoom && (ch = toRoom->first_person) != NULL )
      act( AT_PLAIN, "A magical portal above winks from existence.", ch, NULL, NULL, TO_ROOM );

    /* remove the portal obj: looks better to let update_obj do this */
    /* extract_obj(portal);  */

    return;
}

void reboot_check( time_t reset )
{
  static char *tmsg[] =
  { "Warning: Auto-Warmboot in 1 Minute!",
    "Warning: Auto-Warmboot in 2 Minutes!",
    "Warning: Auto-Warmboot in 3 Minutes!",
    "Warning: Auto-Warmboot in 4 Minutes!",
    "Warning: Auto-Warmboot in 5 Minutes!",
    "Warning: Auto-Warmboot in 10 Minutes!",
    "Warning: Auto-Warmboot in 15 Minutes!",
    "Warning: Auto-Warmboot in 30 Minutes!"
  };
  static const int times[] = { 60, 120, 180, 240, 300, 600, 900, 1800 };
  static const int timesize =
      UMIN(sizeof(times)/sizeof(*times), sizeof(tmsg)/sizeof(*tmsg));
  char buf[MAX_STRING_LENGTH];
  static int trun;
  static bool init;
  
  if ( !init || reset >= current_time )
  {
    for ( trun = timesize-1; trun >= 0; trun-- )
      if ( reset >= current_time+times[trun] )
        break;
    init = TRUE;
    return;
  }
  
  if ( (current_time % 1800) == 0 )
  {
    sprintf(buf, "%.24s: %d players", ctime(&current_time), num_descriptors);
    append_to_file(USAGE_FILE, buf);
  }
  
  if ( new_boot_time_t - boot_time < 60*60*18 &&
      !set_boot_time->manual )
    return;
  
  if ( new_boot_time_t <= current_time )
  {
    CHAR_DATA *vch;
    
    if ( auction->item )
    {
      sprintf(buf, "Sale of %s has been stopped by mud.",
          auction->item->short_descr);
      talk_auction(buf,TRUE);
      obj_to_char(auction->item, auction->seller);
      auction->item = NULL;
      REMOVE_BIT(auction->flags, AUCTION_HOARD_AUC);
      if ( auction->buyer && auction->buyer != auction->seller )
      {
        auction->buyer->gold += auction->bet;
        send_to_char("Your money has been returned.\n\r", auction->buyer);
      }
    }
    
    log_string("Automatic Warm-boot");
    for ( vch = first_char; vch; vch = vch->next )
      if ( !IS_NPC(vch) )
        save_char_obj(vch);

    do_copyover(NULL,"");
    return;
  }
  
  if ( trun != -1 && new_boot_time_t - current_time <= times[trun] )
  {
    echo_to_all(AT_YELLOW, tmsg[trun], ECHOTAR_ALL);
    if ( trun <= 5 )
      sysdata.DENY_NEW_PLAYERS = TRUE;
    --trun;
   record_call("<reboot_check>");

    return;
  }
  return;
}
  
#if 0
void reboot_check( char *arg )
{
    char buf[MAX_STRING_LENGTH];
    extern bool mud_down;
    /*struct tm *timestruct;
    int timecheck;*/
    CHAR_DATA *vch;
   record_call("<reboot_check>");
    /*Bools to show which pre-boot echoes we've done. */
    static bool thirty  = FALSE;
    static bool fifteen = FALSE;
    static bool ten     = FALSE;
    static bool five    = FALSE;
    static bool four    = FALSE;
    static bool three   = FALSE;
    static bool two     = FALSE;
    static bool one     = FALSE;

    /* This function can be called by do_setboot when the reboot time
       is being manually set to reset all the bools. */
    if ( !str_cmp( arg, "reset" ) )
    {
      thirty  = FALSE;
      fifteen = FALSE;
      ten     = FALSE;
      five    = FALSE;
      four    = FALSE;
      three   = FALSE;
      two     = FALSE;
      one     = FALSE;
      return;
    }

    /* If the mud has been up less than 18 hours and the boot time 
       wasn't set manually, forget it. */ 
/* Usage monitor */

if ((current_time % 1800) == 0)
{
  sprintf(buf, "%s: %d players", ctime(&current_time), num_descriptors);  
  append_to_file(USAGE_FILE, buf);
}

/* Change by Scryn - if mud has not been up 18 hours at boot time - still 
 * allow for warnings even if not up 18 hours 
 */
    if ( new_boot_time_t - boot_time < 60*60*18 
         && set_boot_time->manual == 0 )
    {
      return;
    }
/*
    timestruct = localtime( &current_time);

    if ( timestruct->tm_hour == set_boot_time->hour        
         && timestruct->tm_min  == set_boot_time->min )*/
    if ( new_boot_time_t <= current_time )
    {
       /* Return auction item to seller */
       if (auction->item != NULL)
       {
        sprintf (buf,"Sale of %s has been stopped by mud.",
                 auction->item->short_descr);
        talk_auction (buf);
        obj_to_char (auction->item, auction->seller);
        auction->item = NULL;
        if (auction->buyer != NULL && auction->seller != auction->buyer) /* return money to the buyer */
        {
            auction->buyer->gold += auction->bet;
            send_to_char ("Your money has been returned.\n\r",auction->buyer);
        }
       }      

       sprintf( buf, "You are forced from these realms by a strong magical presence" ); 
       echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
       sprintf( buf, "as life here is reconstructed." );
       echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );

       /* Save all characters before booting. */
       for ( vch = first_char; vch; vch = vch->next )
       {
         if ( !IS_NPC( vch ) )
           save_char_obj( vch );
       }
       mud_down = TRUE;
    }

  /* How many minutes to the scheduled boot? */
/*  timecheck = ( set_boot_time->hour * 60 + set_boot_time->min )
              - ( timestruct->tm_hour * 60 + timestruct->tm_min );

  if ( timecheck > 30  || timecheck < 0 ) return;

  if ( timecheck <= 1 ) */
  if ( new_boot_time_t - current_time <= 60 )
  {
    if ( one == FALSE )
    {
	sprintf( buf, "You feel the ground shake as the end comes near!" );
	echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
	one = TRUE;
	sysdata.DENY_NEW_PLAYERS = TRUE;
    }
    return;   
  }

/*  if ( timecheck == 2 )*/
  if ( new_boot_time_t - current_time <= 120 )
  {
    if ( two == FALSE )
    {
	sprintf( buf, "Lightning crackles in the sky above!" );
	echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
	two = TRUE;
	sysdata.DENY_NEW_PLAYERS = TRUE;
    }
    return;   
  }

/*  if ( timecheck == 3 )*/ 
  if (new_boot_time_t - current_time <= 180 )
  {
    if ( three == FALSE )
    {
	sprintf( buf, "Crashes of thunder sound across the land!" );
	echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
	three = TRUE;
	sysdata.DENY_NEW_PLAYERS = TRUE;
    }
    return;   
  }

/*  if ( timecheck == 4 )*/
  if( new_boot_time_t - current_time <= 240 )
  {
    if ( four == FALSE )
    {
	sprintf( buf, "The sky has suddenly turned midnight black." );
	echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
	four = TRUE;
	sysdata.DENY_NEW_PLAYERS = TRUE;
    }
    return;   
  }

/*  if ( timecheck == 5 )*/
  if( new_boot_time_t - current_time <= 300 )
  {
    if ( five == FALSE )
    {
	sprintf( buf, "You notice the life forms around you slowly dwindling away." );
	echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
	five = TRUE;
	sysdata.DENY_NEW_PLAYERS = TRUE;
    }
    return;   
  }

/*  if ( timecheck == 10 )*/
  if( new_boot_time_t - current_time <= 600 )
  {
    if ( ten == FALSE )
    {
	sprintf( buf, "The seas across the realm have turned frigid." );
	echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
	ten = TRUE;
    }
    return;   
  }

/*  if ( timecheck == 15 )*/
  if( new_boot_time_t - current_time <= 900 )
  {
    if ( fifteen == FALSE )
    {
	sprintf( buf, "The aura of magic which once surrounded the realms seems slightly unstable." );
	echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
	fifteen = TRUE;
    }
    return;   
  }

/*  if ( timecheck == 30 )*/
  if( new_boot_time_t - current_time <= 1800 )
  { 
    if ( thirty == FALSE )
    {
	sprintf( buf, "You sense a change in the magical forces surrounding you." );
	echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
	thirty = TRUE;
    }
    return;   
  }

  return;
}
#endif

/* the auction update*/

void auction_update (void)
{
    int tax, pay;
    char buf[MAX_STRING_LENGTH];
   record_call("<auction_update>");

    switch (++auction->going) /* increase the going state */
    {
	case 1 : /* going once */
	case 2 : /* going twice */
	    if (auction->bet > auction->starting)
	    {
	        sprintf(buf,"%s &Wfor &Y%s&W gold coins&W going %s",auction->item->short_descr,
		num_comma(auction->bet),((auction->going == 1) ? "once" : "twice"));
	        talk_auction (buf,TRUE);
	    }
	    else
	    {
		sprintf(buf, "No bidders yet on %s.",auction->item->short_descr);
	        talk_auction (buf,TRUE);
		sprintf(buf,"The minimum bid is &Y%s&W gold coin%s&W",
		  num_comma(auction->bet),(auction->bet > 1) ? "s&W." : "&W." );	
	        talk_auction (buf,FALSE);
	    }
	    break;

	case 3 : /* SOLD! */
	    if (!auction->buyer && auction->bet)
	    {
		bug( "Auction code reached SOLD, with NULL buyer, but %d gold bid", auction->bet );
		auction->bet = 0;
	    }
	    if (auction->bet > 0 && auction->buyer != auction->seller)
	    {
		sprintf (buf, "%s &BSOLD &Wto &P%s &Wfor &Y%s&W gold coins.",
			auction->item->short_descr,
			IS_NPC(auction->buyer) ? auction->buyer->short_descr : auction->buyer->name,
			num_comma(auction->bet));
		talk_auction(buf,TRUE);

		act(AT_ACTION, "The auctioneer materializes before you, and hands you $p.",
			auction->buyer, auction->item, NULL, TO_CHAR);
		act(AT_ACTION, "The auctioneer materializes before $n, and hands $m $p.",
			auction->buyer, auction->item, NULL, TO_ROOM);

		if ( (auction->buyer->carry_weight 
		+     get_obj_weight( auction->item ))
		>     can_carry_w( auction->buyer ) )
		{
		    act( AT_PLAIN, "$p is too heavy for you to carry with your current inventory.", auction->buyer, auction->item, NULL, TO_CHAR );
    		    act( AT_PLAIN, "$n is carrying too much to also carry $p, and $e drops it.", auction->buyer, auction->item, NULL, TO_ROOM );
		    obj_to_room( auction->item, auction->buyer->in_room );
		}
		else
		    obj_to_char( auction->item, auction->buyer );
	        pay = (int)auction->bet * 0.9;
		tax = (int)auction->bet * 0.1;

		/* Dont boost econ if not in a room .. --GW */
	        if ( auction->seller->in_room )
		boost_economy( auction->seller->in_room->area, tax );

                auction->seller->real_gold += pay; /* give him the money, tax 10 % */
		sprintf(buf, "The auctioneer pays you %s gold, charging an auction fee of %s.\n\r", num_comma(pay), num_comma(tax));
		send_to_char(buf, auction->seller);

		/* Hoarder Stuff --GW */
                if ( IS_HOARDER(auction->seller) )
	  	   hoarder_check(auction->seller,auction->item,HCHECK_AUCTION);

                auction->item = NULL; /* reset item */
		REMOVE_BIT(auction->flags, AUCTION_HOARD_AUC);
		if ( IS_SET( sysdata.save_flags, SV_AUCTION ) )
		{
		    save_char_obj( auction->buyer );
		    save_char_obj( auction->seller );
		}
		
            }
            else /* not sold */
            {
                sprintf(buf, "&WNobody wants %s.",auction->item->short_descr);
                talk_auction(buf,TRUE);
		sprintf(buf, "&WLets get a new item up for bids.");
	        talk_auction(buf,FALSE);

		if ( IS_SET(auction->flags, AUCTION_HOARD_AUC) )
		{
		  sprintf(buf,"&WObject was auctioned by a Hoarder. Object &RDestroyed.");
		  talk_auction(buf,FALSE);
		  extract_obj(auction->item);
		  auction->item = NULL;
		  REMOVE_BIT(auction->flags, AUCTION_HOARD_AUC);
		  return;
	        }

                act (AT_ACTION, "The auctioneer appears before you to return $p to you.",
                      auction->seller,auction->item,NULL,TO_CHAR);
                act (AT_ACTION, "The auctioneer appears before $n to return $p to $m.",
                      auction->seller,auction->item,NULL,TO_ROOM);
		if ( (auction->seller->carry_weight
		+     get_obj_weight( auction->item ))
		>     can_carry_w( auction->seller ) )
		{
		    act( AT_PLAIN, "You drop $p as it is just too much to carry"
			" with everything else you're carrying.", auction->seller,
			auction->item, NULL, TO_CHAR );
		    act( AT_PLAIN, "$n drops $p as it is too much extra weight"
			" for $m with everything else.", auction->seller,
			auction->item, NULL, TO_ROOM );
		    obj_to_room( auction->item, auction->seller->in_room );
		}
		else
		    obj_to_char (auction->item,auction->seller);
		tax = (int)auction->item->cost * 0.05;
		boost_economy( auction->seller->in_room->area, tax );
		sprintf(buf, "The auctioneer charges you an auction fee of %s.\n\r", num_comma(tax) );
		send_to_char(buf, auction->seller);
		if ((auction->seller->gold - tax) < 0)
		  auction->seller->gold = 0;
		else
		  auction->seller->gold -= tax;
		if ( IS_SET( sysdata.save_flags, SV_AUCTION ) )
		    save_char_obj( auction->seller );
	    } /* else */
	    auction->item = NULL; /* clear auction */
	    REMOVE_BIT(auction->flags, AUCTION_HOARD_AUC);
    } /* switch */
} /* func */

void subtract_times(struct timeval *etime, struct timeval *stime)
{
   record_call("<subtract_times>");
  etime->tv_sec -= stime->tv_sec;
  etime->tv_usec -= stime->tv_usec;
  while ( etime->tv_usec < 0 )
  {
    etime->tv_usec += 1000000;
    etime->tv_sec--;
  }
  return;
}

void room_link_check( CHAR_DATA *ch )
{
ROOM_INDEX_DATA *room=NULL;
CHAR_DATA *test=NULL,*next_test=NULL;
bool link=FALSE;
char buf[MSL];

if ( !ch->in_room )
  return;

/* Cant find it at all */
room = ch->in_room;

if ( room && room->first_person && room->last_person )
{
   for( test = room->first_person; test; test = next_test )
   {
      next_test = test->next_in_room;
      if ( !str_cmp(test->name,ch->name) )
	  link=TRUE;
   }
}

if ( !link )
{
 sprintf(buf,"ROOM-LINK-CHECK: %s not in roomlist, adding.(V:%d)",ch->name,ch->in_room->vnum);
 log_string_plus(buf,LOG_NORMAL,ch->level);

if ( !IS_NPC(ch) )
{
 sprintf(buf,"ROOM-LINK-CHECK: Last command from character was %s",ch->pcdata->lastcom[0] ? 
	ch->pcdata->lastcom[0] : "None!");
 log_string_plus(buf,LOG_NORMAL,ch->level);
}
else
 log_string_plus("ROOM-LINK-CHECK: NPC!",LOG_NORMAL,51);
 
 LINK( ch, ch->in_room->first_person, ch->in_room->last_person,
              next_in_room, prev_in_room );
}

return;
}

void dragon_update( void )
{
  int num;
  CHAR_DATA *ch, *next_ch;
  char buf[MSL], hourbuf[MSL], ampmbuf[MSL];
  int hour;
  extern bool DRAGONS_RAN;
  extern bool FORCE_DRAGON_RUN;

/* Toss some poor loser.. --GW */
sprintf( hourbuf,"%d",(time_info.hour % 12 == 0) ? 12 : time_info.hour % 12);
sprintf( ampmbuf,"%s",time_info.hour >= 12 ? "pm" : "am");
hour = atoi(hourbuf);

if ( hour == 11 && !str_cmp( ampmbuf, "pm" ) && DRAGONS_RAN == TRUE )
{
DRAGONS_RAN = FALSE;
/* Warning Blurb .. added per request --GW */
log_string("System Activating Global Mode: Dragon Drop");
echo_to_all(AT_PLAIN,"&RYou see the &wShadow&R of &wDragon Wings&R fall over the &YMoon&R..",ECHOTAR_ALL);
echo_to_all(AT_PLAIN,"\n\r",ECHOTAR_ALL);
echo_to_all(AT_PLAIN,"&wDragons &RSCREECH &wfrom above .. &RRUN!",ECHOTAR_ALL);
} 
 
if ( ( hour == 12 && !str_cmp( ampmbuf, "am" ) && !DRAGONS_RAN ) ||
	FORCE_DRAGON_RUN )
{  
DRAGONS_RAN = TRUE;

   for( ch = first_char; ch; ch = next_ch )
   {
    next_ch = ch->next;
  
   if( (num=number_range(1,2))==2)
    if ( !IS_NPC(ch) && !IS_IMMORTAL(ch) && str_cmp(ch->name, "ancientone") &&
	(ch->in_room && !IS_SET(ch->in_room->room_flags, ROOM_INDOORS))
	&& IS_SET(ch->in_room->area->flags, AFLAG_TOWN))
    {
    sprintf(buf,
    "&wA Large &zShadow &wlooms over &Ryou..\r"
    "&wSuddenly, a &RScreaching Dragon&w swoops down and grabs &R%s!\r"
    "&w%s &Rscreams in Pain, &wwhile being dragged away by the screaching &zBeast!\r",
	capitalize(ch->name),capitalize(ch->name));
    act(AT_PLAIN,buf,ch,NULL,ch,TO_NOTVICT);

    sprintf(buf,
    "&wA Large &zShadow &wlooms over &Ryou..\r"
    "&wSuddenly, a &RScreaching Dragon&w swoops down and grabs &RYou!\r"
    "&wYou &Rscream in Pain, &wwhile being dragged away by the screaching &zBeast!\r");
    act(AT_PLAIN,buf,ch,NULL,NULL,TO_CHAR);
    dragon_abduct(ch);
    return;
   }
 }        
}
return;
}

void dragon_abduct(CHAR_DATA *ch)
{
bool looking;
ROOM_INDEX_DATA *location;
int num=0;
char buf[MSL];

  ch->hit = ch->max_hit/2;
  ch->mana = ch->max_mana/2;
  ch->move = ch->max_move/2;

looking = TRUE;
while( looking )
{
  num = number_range( 100, 32737 );
  if ( (location = get_room_index(num,1))!=NULL &&
         !IS_SET(location->room_flags, ROOM_DEATH) &&
	 IS_SET(location->area->flags, AFLAG_DRAGON_DROP) &&
         (location->level < get_trust(ch)))
  {
    send_to_char("&RWith a Screech, the Dragon Drops you .. somewhere!\n\r",ch);
    char_from_room(ch);
    char_to_room(ch,location);
    reset_area(ch->in_room->area);
    looking = FALSE;
  }
  
}

sprintf(buf,"Dragon Drop: Dropped %s in %s!",capitalize(ch->name),location->name);
log_string(buf);
return;
}

/* Redoes all room links every 5 ticks --GW */
void inroom_update( void )
{
ZONE_DATA *zone=NULL;
CHAR_DATA *ch,*next_ch=NULL;
ROOM_INDEX_DATA *RoomIdx;
int hash=0;
extern bool skip_locker;

zone=find_zone(1);
skip_locker = TRUE;
/* take all from rooms */

/*Players*/
for ( ch=first_char;ch;ch=next_ch )
{
  next_ch=ch->next;

  if ( !ch->last_to_room )
  {
   bug("In_Room_Update: Last_to_room is NULL (%s)",ch->name);
   continue;
  }

  char_from_room(ch);
  ch->in_room=NULL;
  ch->next_in_room=NULL;
}
/*Mobiles*/
for ( ch=zone->first_mob;ch;ch=next_ch )
{
  next_ch=ch->next;

  if ( !ch->last_to_room )
  {
   bug("In_Room_Update: Last_to_room is NULL (%s)",ch->name);
   continue;
  }

  char_from_room(ch);
  ch->in_room=NULL;
  ch->next_in_room=NULL;
}

/*Clean rooms */
for(hash=0;hash < MAX_KEY_HASH; hash++)
  for( RoomIdx = zone->room_index_hash[hash];RoomIdx;RoomIdx=RoomIdx->next )
  { 
   RoomIdx->first_person = NULL;
   RoomIdx->last_person = NULL;
  }
/*put everyone back*/

/* Players */
for ( ch=first_char;ch;ch=next_ch )
{
  next_ch=ch->next;

  if ( !ch->last_to_room )
  {
   bug("In_Room_Update: Last_to_room is NULL (%s)",ch->name);
   continue;
  }

  char_to_room(ch,ch->last_to_room);
}

/*Mobs*/
for ( ch=zone->first_mob;ch;ch=next_ch )
{
  next_ch=ch->next;

  if ( !ch->last_to_room )
  {
   bug("In_Room_Update: Last_to_room is NULL (%s)",ch->name);
   continue;
  }

  char_to_room(ch,ch->last_to_room);
}

//fprintf(stderr,"Completed.\n");
skip_locker = FALSE;
/* Done */
return;
}

