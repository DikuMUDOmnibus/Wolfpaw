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
 *			    Battle & death module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

extern char		lastplayercmd[MAX_INPUT_LENGTH];
extern CHAR_DATA *	gch_prev;

bool death_handler	      args( ( CHAR_DATA *ch, CHAR_DATA *victim, bool npcvict ) );
void maim_update( CHAR_DATA *ch, CHAR_DATA *victim );

/*
 * Local functions.
 */
void	dam_message	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
			    int dt ) );
void	death_cry	args( ( CHAR_DATA *ch ) );
void	group_gain	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
int	xp_compute	args( ( CHAR_DATA *gch, CHAR_DATA *victim ) );
int	align_compute	args( ( CHAR_DATA *gch, CHAR_DATA *victim ) );
ch_ret	one_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
int	obj_hitroll	args( ( OBJ_DATA *obj ) );
bool    remove_obj      args( ( CHAR_DATA *ch, int iWear, bool fReplace ));
int	number_kombat   args( ( void ) );
bool	dual_flip = FALSE;
bool    fight_in_progress; /* for pk arena scheduling */
void    quest_death_check args( ( CHAR_DATA *ch, CHAR_DATA *victim ) ); 
void    ego_update args( ( CHAR_DATA *ch, CHAR_DATA *victim, bool gain ) );
bool    can_flee_dir( CHAR_DATA *ch, int door );

/*
 * Check to see if weapon is poisoned.
 */
bool is_wielding_poisoned( CHAR_DATA *ch )
{
         OBJ_DATA *obj;

         if ( ( obj = get_eq_char( ch, WEAR_WIELD ) 	)
         &&   (IS_SET( obj->extra_flags, ITEM_POISONED) )	)
                  return TRUE;

         return FALSE;

}

/*
 * hunting, hating and fearing code				-Thoric
 */
bool is_hunting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( !ch->hunting || ch->hunting->who != victim )
      return FALSE;
    
    return TRUE;    
}

bool is_hating( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( !ch->hating || ch->hating->who != victim )
      return FALSE;
    
    return TRUE;    
}

bool is_fearing( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( !ch->fearing || ch->fearing->who != victim )
      return FALSE;
    
    return TRUE;    
}

void stop_hunting( CHAR_DATA *ch )
{
    if ( ch->hunting )
    {
	STRFREE( ch->hunting->name );
	DISPOSE( ch->hunting );
	ch->hunting = NULL;
    }
    return;
}

void stop_hating( CHAR_DATA *ch )
{
    if ( ch->hating )
    {
	STRFREE( ch->hating->name );
	DISPOSE( ch->hating );
	ch->hating = NULL;
    }
    return;
}

void stop_fearing( CHAR_DATA *ch )
{
    if ( ch->fearing )
    {
	STRFREE( ch->fearing->name );
	DISPOSE( ch->fearing );
	ch->fearing = NULL;
    }
    return;
}

void start_hunting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->hunting )
      stop_hunting( ch );

    CREATE( ch->hunting, HHF_DATA, 1 );
    ch->hunting->name = QUICKLINK( victim->name );
    ch->hunting->who  = victim;
    return;
}

void start_hating( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->hating )
      stop_hating( ch );

    CREATE( ch->hating, HHF_DATA, 1 );
    ch->hating->name = QUICKLINK( victim->name );
    ch->hating->who  = victim;
    return;
}

void start_fearing( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->fearing )
      stop_fearing( ch );

    CREATE( ch->fearing, HHF_DATA, 1 );
    ch->fearing->name = QUICKLINK( victim->name );
    ch->fearing->who  = victim;
    return;
}

/*
 * Get the current armor class for a vampire based on time of day
 */
sh_int VAMP_AC( CHAR_DATA * ch )
{
  if ( IS_VAMPIRE( ch ) || IS_OUTSIDE( ch ) )
  {
    switch(weather_info.sunlight)
    {
    case SUN_DARK:
      return -8;
    case SUN_RISE:
      return 5;
    case SUN_LIGHT:
      return 10;
    case SUN_SET:
      return 2;
    default:
      return 0;
    }
  }
  else
    return 0;
}

int max_fight( CHAR_DATA *ch )
{
    return 8;
}

/*
 * Control the fights going on.
 * Called periodically by update_handler.
 * Many hours spent fixing bugs in here by Thoric, as noted by residual
 * debugging checks.  If you never get any of these error messages again
 * in your logs... then you can comment out some of the checks without
 * worry.
 */
void violence_update( void )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *ch;
    CHAR_DATA *lst_ch;
    CHAR_DATA *victim;
    CHAR_DATA *rch, *rch_next;
    AFFECT_DATA *paf, *paf_next;
    TIMER	*timer, *timer_next;
    ch_ret     retcode;
    int	       x, attacktype=0, cnt;
    SKILLTYPE	*skill;
    ZONE_DATA *in_zone;

    lst_ch = NULL;
    for ( ch = last_char; ch; lst_ch = ch, ch = gch_prev )
    {
	set_cur_char( ch );

	if ( ch == first_char && ch->prev )
	{
	   bug( "ERROR: first_char->prev != NULL, fixing...", 0 );
	   ch->prev = NULL;
	}

	gch_prev	= ch->prev;

	if ( gch_prev && gch_prev->next != ch )
	{
	    sprintf( buf, "FATAL: violence_update: %s->prev->next doesn't point to ch.",
		ch->name );
	    bug( buf, 0 );	    
	    bug( "Short-cutting here", 0 );
	    ch->prev = NULL;
	    gch_prev = NULL;
	}

	/*
	 * See if we got a pointer to someone who recently died...
	 * if so, either the pointer is bad... or it's a player who
	 * "died", and is back at the healer...
	 * Since he/she's in the char_list, it's likely to be the later...
	 * and should not already be in another fight already
	 */
	if ( char_died(ch) )
	    continue;

	/*
	 * See if we got a pointer to some bad looking data...
	 */
	if ( !ch->in_room || !ch->name )
	{
	    log_string( "violence_update: bad ch record!  (Shortcutting.)" );
	    sprintf( buf, "ch: %d  ch->in_room: %d  ch->prev: %d  ch->next: %d",
	    	(int) ch, (int) ch->in_room, (int) ch->prev, (int) ch->next );
	    log_string( buf );
	    log_string( lastplayercmd );
	    if ( lst_ch )
	      sprintf( buf, "lst_ch: %d  lst_ch->prev: %d  lst_ch->next: %d",
	      		(int) lst_ch, (int) lst_ch->prev, (int) lst_ch->next );
	    else
	      strcpy( buf, "lst_ch: NULL" );
	    log_string( buf );
	    gch_prev = NULL;
	    continue;
	}


	for ( timer = ch->first_timer; timer; timer = timer_next )
	{
	    timer_next = timer->next;
	    if ( --timer->count <= 0 )
	    {
		if ( timer->type == TIMER_DO_FUN )
		{
		    int tempsub;

		    tempsub = ch->substate;
		    ch->substate = timer->value;
		    (timer->do_fun)( ch, "" );
		    if ( char_died(ch) )
		      break;
		    ch->substate = tempsub;
		}
		extract_timer( ch, timer );
	    }
	}

	if ( char_died(ch) )
	  continue;

	/*
	 * We need spells that have shorter durations than an hour.
	 * So a melee round sounds good to me... -Thoric
	 */
	for ( paf = ch->first_affect; paf; paf = paf_next )
	{
	      paf_next	= paf->next;
	      if ( paf->duration > 0 )
		paf->duration--;
	      else
	      if ( paf->duration < 0 )
		;
	      else
	      {
		  if ( !paf_next
		  ||    paf_next->type != paf->type
		  ||    paf_next->duration > 0 )
		  {
		      skill = get_skilltype(paf->type);
		      if ( paf->type > 0 && skill && skill->msg_off )
		      {
                          set_char_color( AT_WEAROFF, ch );
			  send_to_char( skill->msg_off, ch );
			  send_to_char( "\n\r", ch );
		      }
		  }
		  if (paf->type == gsn_possess)
	          {
	            ch->desc->character       = ch->desc->original;
    	            ch->desc->original        = NULL;
    		    ch->desc->character->desc = ch->desc;
   	            ch->desc->character->switched = NULL;
    		    ch->desc                  = NULL;
		  }
		  affect_remove( ch, paf );
	      }
	}
	
	if (( victim = who_fighting( ch ) ) == NULL
	||   IS_AFFECTED( ch, AFF_PARALYSIS ) )
	    continue;

        retcode = rNONE;

	if ( IS_SET(ch->in_room->room_flags, ROOM_SAFE ) )
	{
	   sprintf( buf, "violence_update: %s fighting %s in a SAFE room.",
	   	ch->name, victim->name );
	   log_string( buf );
	   stop_fighting( ch, TRUE );
	}
	else
	if ( IS_AWAKE(ch) && ch->in_room == victim->in_room )
	    retcode = multi_hit( ch, victim, TYPE_UNDEFINED );
	else
	    stop_fighting( ch, FALSE );

	if ( char_died(ch) )
	    continue;

	if ( retcode == rCHAR_DIED
	|| ( victim = who_fighting( ch ) ) == NULL )
	    continue;

	/*
	 *  Mob triggers
	 */
	rprog_rfight_trigger( ch );
	if ( char_died(ch) )
	    continue;
	mprog_hitprcnt_trigger( ch, victim );
	if ( char_died(ch) )
	    continue;
	mprog_fight_trigger( ch, victim );
	if ( char_died(ch) )
	    continue;

	/*
	 * Maimers --GW
	 */
	 maim_update( ch, victim );
         if ( char_died(ch) )
	     continue;

	/*
	 * NPC special attack flags				-Thoric
	 */
	if ( IS_NPC(ch) )
	{
	  cnt = 0;
	  if ( ch->attacks )
	    for ( ;; )
	    {
	      if ( cnt++ > 10 )
	      {
		attacktype = 0;
		break;
	      }
	      x = number_range( 7, 31 );
	      attacktype = 1 << x;
	      if ( IS_SET( ch->attacks, attacktype ) )
	        break;
	    }

	  if ( 30 + (ch->level/4) < number_percent( ) )
	    attacktype = 0;
	  switch( attacktype )
	  {
	  case ATCK_BASH:
	    do_bash( ch, "" );
	    retcode = global_retcode;
	    break;
	  case ATCK_STUN:
	    do_stun( ch, "" ); 
	    retcode = global_retcode;
	    break;
	  case ATCK_GOUGE:
	    do_gouge( ch, "" );
	    retcode = global_retcode;
	    break;
	  case ATCK_FEED:
	    do_feed( ch, "" );
	    retcode = global_retcode;
	    break;
	  case ATCK_DRAIN:
	    retcode = spell_energy_drain( skill_lookup( "energy drain" ), ch->level, ch, victim );
	    break;
	  case ATCK_FIREBREATH:
	    retcode = spell_fire_breath( skill_lookup( "fire breath" ), ch->level, ch, victim );
	    break;
	  case ATCK_FROSTBREATH:
	    retcode = spell_frost_breath( skill_lookup( "frost breath" ), ch->level, ch, victim );
	    break;
	  case ATCK_ACIDBREATH:
	    retcode = spell_acid_breath( skill_lookup( "acid breath" ), ch->level, ch, victim );
	    break;
	  case ATCK_LIGHTNBREATH:
	    retcode = spell_lightning_breath( skill_lookup( "lightning breath" ), ch->level, ch, victim );
	    break;
	  case ATCK_GASBREATH:
	    retcode = spell_gas_breath( skill_lookup( "gas breath" ), ch->level, ch, victim );
	    break;
	  case ATCK_SPIRALBLAST:         
            retcode = spell_spiral_blast( skill_lookup( "spiral blast" ),
                        ch->level, ch, victim );
	    break;
	   case ATCK_POISON:
	    retcode = spell_poison( gsn_poison, ch->level, ch, victim );
	    break;
	  case ATCK_NASTYPOISON:
	    /*
	    retcode = spell_nasty_poison( skill_lookup( "nasty poison" ), ch->level, ch, victim );
	     */
	    break;
	  case ATCK_GAZE:
	    /*
	    retcode = spell_gaze( skill_lookup( "gaze" ), ch->level, ch, victim );
	     */
	    break;
	  case ATCK_BLINDNESS:
	    retcode = spell_blindness( gsn_blindness, ch->level, ch, victim );
	    break;
	  case ATCK_CAUSESERIOUS:
	    retcode = spell_cause_serious( skill_lookup( "cause serious" ), ch->level, ch, victim );
	    break;
	  case ATCK_EARTHQUAKE:
	    retcode = spell_earthquake( skill_lookup( "earthquake" ), ch->level, ch, victim );
	    break;
	  case ATCK_CAUSECRITICAL:
	    retcode = spell_cause_critical( skill_lookup( "cause critical" ), ch->level, ch, victim );
	    break;
	  case ATCK_CURSE:
	    retcode = spell_curse( skill_lookup( "curse" ), ch->level, ch, victim );
	    break;
	  case ATCK_FLAMESTRIKE:
	    retcode = spell_flamestrike( skill_lookup( "flamestrike" ), ch->level, ch, victim );
	    break;
	  case ATCK_HARM:
	    retcode = spell_harm( skill_lookup( "harm" ), ch->level, ch, victim );
	    break;
	  case ATCK_FIREBALL:
	    retcode = spell_fireball( skill_lookup( "fireball" ), ch->level, ch, victim );
	    break;
	  case ATCK_COLORSPRAY:
	    retcode = spell_colour_spray( skill_lookup( "colour spray" ), ch->level, ch, victim );
	    break;
	  case ATCK_WEAKEN:
	    retcode = spell_weaken( skill_lookup( "weaken" ), ch->level, ch, victim );
	    break;
	  }
	  if ( retcode == rCHAR_DIED || (char_died(ch)) )
	    continue;

	  /*
	   * NPC special defense flags				-Thoric
	   */
	  cnt = 0;
	  if ( ch->defenses )
	    for ( ;; )
	    {
	      if ( cnt++ > 10 )
	      {
		attacktype = 0;
		break;
	      }
	      x = number_range( 2, 18 );
	      attacktype = 1 << x;
	      if ( IS_SET( ch->defenses, attacktype ) )
	        break;
	    }
	  if ( 50 + (ch->level/4) < number_percent( ) )
	    attacktype = 0;
	  switch( attacktype )
	  {
	    case DFND_CURELIGHT:
	      act( AT_MAGIC, "$n mutters a few incantations...and looks a little better.", ch, NULL, NULL, TO_ROOM );
	      retcode = spell_smaug( skill_lookup( "cure light" ), ch->level, ch, ch );
	      break;
	    case DFND_CURESERIOUS:
	      act( AT_MAGIC, "$n mutters a few incantations...and looks a bit better.", ch, NULL, NULL, TO_ROOM );
	      retcode = spell_smaug( skill_lookup( "cure serious" ), ch->level, ch, ch );
	      break;
	    case DFND_CURECRITICAL:
	      act( AT_MAGIC, "$n mutters a few incantations...and looks a bit healthier.", ch, NULL, NULL, TO_ROOM );
	      retcode = spell_smaug( skill_lookup( "cure critical" ), ch->level, ch, ch );
	      break;
	    case DFND_DISPELMAGIC:
	      act( AT_MAGIC, "$n mutters a few incantations...and waves $s arms about.", ch, NULL, NULL, TO_ROOM );
	      retcode = spell_dispel_magic( skill_lookup( "dispel magic" ), ch->level, ch, victim );
	      break;
	    case DFND_DISPELEVIL:
	      act( AT_MAGIC, "$n mutters a few incantations...and waves $s arms about.", ch, NULL, NULL, TO_ROOM );
	      retcode = spell_dispel_evil( skill_lookup( "dispel evil" ), ch->level, ch, victim );
	      break;
	    case DFND_SANCTUARY:
	      if ( !IS_AFFECTED(victim, AFF_SANCTUARY) )
	      {
		act( AT_MAGIC, "$n mutters a few incantations...", ch, NULL, NULL, TO_ROOM );
		retcode = spell_smaug( skill_lookup( "sanctuary" ), ch->level, ch, ch );
	      }
	      else
	        retcode = rNONE;
	      break;
	  }
	  if ( retcode == rCHAR_DIED || (char_died(ch)) )
	    continue;
	}

	/*
	 * Fun for the whole family!
	 */
	for ( rch = ch->in_room->first_person; rch; rch = rch_next )
	{
	    rch_next = rch->next_in_room;

	    if ( IS_AWAKE(rch) && !rch->fighting )
	    {
		/*
		 * PC's auto-assist others in their group.
		 */
		if ( !IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) )
		{
		    if ( ( !IS_NPC(rch) || IS_AFFECTED(rch, AFF_CHARM) )
		    &&   is_same_group(ch, rch) )
			multi_hit( rch, victim, TYPE_UNDEFINED );
		    continue;
		}

		/*
		 * NPC's assist NPC's of same type or 12.5% chance regardless.
		 */
		if ( IS_NPC(rch) && !IS_AFFECTED(rch, AFF_CHARM)
		&&  !IS_SET(rch->act, ACT_NOASSIST) )
		{
		    if ( char_died(ch) )
			break;
		    if ( rch->pIndexData == ch->pIndexData
		    ||   number_bits( 3 ) == 0 )
		    {
			CHAR_DATA *vch;
			CHAR_DATA *target;
			int number;

			target = NULL;
			number = 0;			for ( vch = ch->in_room->first_person; vch; vch = vch->next )
			{
			    if ( can_see( rch, vch )
			    &&   is_same_group( vch, victim )
			    &&   number_range( 0, number ) == 0 )
			    {
				target = vch;
				number++;
			    }
			}

			if ( target )
			    multi_hit( rch, target, TYPE_UNDEFINED );
		    }
		}
	    }
	}
    }

for( in_zone = first_zone; in_zone; in_zone = in_zone->next )
{
    for ( ch = in_zone->last_mob; ch; lst_ch = ch, ch = gch_prev )
    {
	set_cur_char( ch );

	if ( ch == in_zone->first_mob && ch->prev )
	{
	   bug( "ERROR: first_char->prev != NULL, fixing...", 0 );
	   ch->prev = NULL;
	}

	gch_prev	= ch->prev;

	if ( gch_prev && gch_prev->next != ch )
	{
	    sprintf( buf, "FATAL: violence_update: %s->prev->next doesn't point to ch.",
		ch->name );
	    bug( buf, 0 );	    
	    bug( "Short-cutting here", 0 );
	    ch->prev = NULL;
	    gch_prev = NULL;
	}

	/*
	 * See if we got a pointer to someone who recently died...
	 * if so, either the pointer is bad... or it's a player who
	 * "died", and is back at the healer...
	 * Since he/she's in the char_list, it's likely to be the later...
	 * and should not already be in another fight already
	 */
	if ( char_died(ch) )
	    continue;

	/*
	 * See if we got a pointer to some bad looking data...
	 */
	if ( !ch->in_room || !ch->name )
	{
	    log_string( "violence_update: bad ch record!  (Shortcutting.)" );
	    sprintf( buf, "ch: %d  ch->in_room: %d  ch->prev: %d  ch->next: %d",
	    	(int) ch, (int) ch->in_room, (int) ch->prev, (int) ch->next );
	    log_string( buf );
	    log_string( lastplayercmd );
	    if ( lst_ch )
	      sprintf( buf, "lst_ch: %d  lst_ch->prev: %d  lst_ch->next: %d",
	      		(int) lst_ch, (int) lst_ch->prev, (int) lst_ch->next );
	    else
	      strcpy( buf, "lst_ch: NULL" );
	    log_string( buf );
	    gch_prev = NULL;
	    continue;
	}


	for ( timer = ch->first_timer; timer; timer = timer_next )
	{
	    timer_next = timer->next;
	    if ( --timer->count <= 0 )
	    {
		if ( timer->type == TIMER_DO_FUN )
		{
		    int tempsub;

		    tempsub = ch->substate;
		    ch->substate = timer->value;
		    (timer->do_fun)( ch, "" );
		    if ( char_died(ch) )
		      break;
		    ch->substate = tempsub;
		}
		extract_timer( ch, timer );
	    }
	}

	if ( char_died(ch) )
	  continue;

	/*
	 * We need spells that have shorter durations than an hour.
	 * So a melee round sounds good to me... -Thoric
	 */
	for ( paf = ch->first_affect; paf; paf = paf_next )
	{
	      paf_next	= paf->next;
	      if ( paf->duration > 0 )
		paf->duration--;
	      else
	      if ( paf->duration < 0 )
		;
	      else
	      {
		  if ( !paf_next
		  ||    paf_next->type != paf->type
		  ||    paf_next->duration > 0 )
		  {
		      skill = get_skilltype(paf->type);
		      if ( paf->type > 0 && skill && skill->msg_off )
		      {
                          set_char_color( AT_WEAROFF, ch );
			  send_to_char( skill->msg_off, ch );
			  send_to_char( "\n\r", ch );
		      }
		  }
		  if (paf->type == gsn_possess)
	          {
	            ch->desc->character       = ch->desc->original;
    	            ch->desc->original        = NULL;
    		    ch->desc->character->desc = ch->desc;
   	            ch->desc->character->switched = NULL;
    		    ch->desc                  = NULL;
		  }
		  affect_remove( ch, paf );
	      }
	}
	
	if (( victim = who_fighting( ch ) ) == NULL
	||   IS_AFFECTED( ch, AFF_PARALYSIS ) )
	    continue;

        retcode = rNONE;

	if ( IS_SET(ch->in_room->room_flags, ROOM_SAFE ) )
	{
	   sprintf( buf, "violence_update: %s fighting %s in a SAFE room.",
	   	ch->name, victim->name );
	   log_string( buf );
	   stop_fighting( ch, TRUE );
	}
	else
	if ( IS_AWAKE(ch) && ch->in_room == victim->in_room )
	    retcode = multi_hit( ch, victim, TYPE_UNDEFINED );
	else
	    stop_fighting( ch, FALSE );

	if ( char_died(ch) )
	    continue;

	if ( retcode == rCHAR_DIED
	|| ( victim = who_fighting( ch ) ) == NULL )
	    continue;

    if ( ch->stun_rounds < 1 )
    {
	/*
	 *  Mob triggers
	 */
	rprog_rfight_trigger( ch );
	if ( char_died(ch) )
	    continue;
	mprog_hitprcnt_trigger( ch, victim );
	if ( char_died(ch) )
	    continue;
	mprog_fight_trigger( ch, victim );
	if ( char_died(ch) )
	    continue;

	/* Smart Mobbies -- YIKES! --GW */
	smart_update( ch, victim ); 

	/*
	 * Maimers --GW
	 */
	 maim_update( ch, victim );
         if ( char_died(ch) )
	     continue;

	/*
	 * NPC special attack flags				-Thoric
	 */
	if ( IS_NPC(ch) )
	{
	  cnt = 0;
	  if ( ch->attacks )
	    for ( ;; )
	    {
	      if ( cnt++ > 10 )
	      {
		attacktype = 0;
		break;
	      }
	      x = number_range( 7, 31 );
	      attacktype = 1 << x;
	      if ( IS_SET( ch->attacks, attacktype ) )
	        break;
	    }

	  if ( 30 + (ch->level/4) < number_percent( ) )
	    attacktype = 0;
	  switch( attacktype )
	  {
	  case ATCK_BASH:
	    do_bash( ch, "" );
	    retcode = global_retcode;
	    break;
	  case ATCK_STUN:
	    do_stun( ch, "" ); 
	    retcode = global_retcode;
	    break;
	  case ATCK_GOUGE:
	    do_gouge( ch, "" );
	    retcode = global_retcode;
	    break;
	  case ATCK_FEED:
	    do_feed( ch, "" );
	    retcode = global_retcode;
	    break;
	  case ATCK_DRAIN:
	    retcode = spell_energy_drain( skill_lookup( "energy drain" ), ch->level, ch, victim );
	    break;
	  case ATCK_FIREBREATH:
	    retcode = spell_fire_breath( skill_lookup( "fire breath" ), ch->level, ch, victim );
	    break;
	  case ATCK_FROSTBREATH:
	    retcode = spell_frost_breath( skill_lookup( "frost breath" ), ch->level, ch, victim );
	    break;
	  case ATCK_ACIDBREATH:
	    retcode = spell_acid_breath( skill_lookup( "acid breath" ), ch->level, ch, victim );
	    break;
	  case ATCK_LIGHTNBREATH:
	    retcode = spell_lightning_breath( skill_lookup( "lightning breath" ), ch->level, ch, victim );
	    break;
	  case ATCK_GASBREATH:
	    retcode = spell_gas_breath( skill_lookup( "gas breath" ), ch->level, ch, victim );
	    break;
	  case ATCK_SPIRALBLAST:         
            retcode = spell_spiral_blast( skill_lookup( "spiral blast" ),
                        ch->level, ch, victim );
	    break;
	   case ATCK_POISON:
	    retcode = spell_poison( gsn_poison, ch->level, ch, victim );
	    break;
	  case ATCK_NASTYPOISON:
	    /*
	    retcode = spell_nasty_poison( skill_lookup( "nasty poison" ), ch->level, ch, victim );
	     */
	    break;
	  case ATCK_GAZE:
	    /*
	    retcode = spell_gaze( skill_lookup( "gaze" ), ch->level, ch, victim );
	     */
	    break;
	  case ATCK_BLINDNESS:
	    retcode = spell_blindness( gsn_blindness, ch->level, ch, victim );
	    break;
	  case ATCK_CAUSESERIOUS:
	    retcode = spell_cause_serious( skill_lookup( "cause serious" ), ch->level, ch, victim );
	    break;
	  case ATCK_EARTHQUAKE:
	    retcode = spell_earthquake( skill_lookup( "earthquake" ), ch->level, ch, victim );
	    break;
	  case ATCK_CAUSECRITICAL:
	    retcode = spell_cause_critical( skill_lookup( "cause critical" ), ch->level, ch, victim );
	    break;
	  case ATCK_CURSE:
	    retcode = spell_curse( skill_lookup( "curse" ), ch->level, ch, victim );
	    break;
	  case ATCK_FLAMESTRIKE:
	    retcode = spell_flamestrike( skill_lookup( "flamestrike" ), ch->level, ch, victim );
	    break;
	  case ATCK_HARM:
	    retcode = spell_harm( skill_lookup( "harm" ), ch->level, ch, victim );
	    break;
	  case ATCK_FIREBALL:
	    retcode = spell_fireball( skill_lookup( "fireball" ), ch->level, ch, victim );
	    break;
	  case ATCK_COLORSPRAY:
	    retcode = spell_colour_spray( skill_lookup( "colour spray" ), ch->level, ch, victim );
	    break;
	  case ATCK_WEAKEN:
	    retcode = spell_weaken( skill_lookup( "weaken" ), ch->level, ch, victim );
	    break;
	  }
	  if ( retcode == rCHAR_DIED || (char_died(ch)) )
	    continue;

	  /*
	   * NPC special defense flags				-Thoric
	   */
	  cnt = 0;
	  if ( ch->defenses )
	    for ( ;; )
	    {
	      if ( cnt++ > 10 )
	      {
		attacktype = 0;
		break;
	      }
	      x = number_range( 2, 18 );
	      attacktype = 1 << x;
	      if ( IS_SET( ch->defenses, attacktype ) )
	        break;
	    }
	  if ( 50 + (ch->level/4) < number_percent( ) )
	    attacktype = 0;
	  switch( attacktype )
	  {
	    case DFND_CURELIGHT:
	      act( AT_MAGIC, "$n mutters a few incantations...and looks a little better.", ch, NULL, NULL, TO_ROOM );
	      retcode = spell_smaug( skill_lookup( "cure light" ), ch->level, ch, ch );
	      break;
	    case DFND_CURESERIOUS:
	      act( AT_MAGIC, "$n mutters a few incantations...and looks a bit better.", ch, NULL, NULL, TO_ROOM );
	      retcode = spell_smaug( skill_lookup( "cure serious" ), ch->level, ch, ch );
	      break;
	    case DFND_CURECRITICAL:
	      act( AT_MAGIC, "$n mutters a few incantations...and looks a bit healthier.", ch, NULL, NULL, TO_ROOM );
	      retcode = spell_smaug( skill_lookup( "cure critical" ), ch->level, ch, ch );
	      break;
	    case DFND_DISPELMAGIC:
	      act( AT_MAGIC, "$n mutters a few incantations...and waves $s arms about.", ch, NULL, NULL, TO_ROOM );
	      retcode = spell_dispel_magic( skill_lookup( "dispel magic" ), ch->level, ch, victim );
	      break;
	    case DFND_DISPELEVIL:
	      act( AT_MAGIC, "$n mutters a few incantations...and waves $s arms about.", ch, NULL, NULL, TO_ROOM );
	      retcode = spell_dispel_evil( skill_lookup( "dispel evil" ), ch->level, ch, victim );
	      break;
	    case DFND_SANCTUARY:
	      if ( !IS_AFFECTED(victim, AFF_SANCTUARY) )
	      {
		act( AT_MAGIC, "$n mutters a few incantations...", ch, NULL, NULL, TO_ROOM );
		retcode = spell_smaug( skill_lookup( "sanctuary" ), ch->level, ch, ch );
	      }
	      else
	        retcode = rNONE;
	      break;
	  }
	  if ( retcode == rCHAR_DIED || (char_died(ch)) )
	    continue;
	}

      }

	/*
	 * Fun for the whole family!
	 */
	for ( rch = ch->in_room->first_person; rch; rch = rch_next )
	{
	    rch_next = rch->next_in_room;

	    if ( IS_AWAKE(rch) && !rch->fighting )
	    {
		/*
		 * PC's auto-assist others in their group.
		 */
		if ( !IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) )
		{
		    if ( ( !IS_NPC(rch) || IS_AFFECTED(rch, AFF_CHARM) )
		    &&   is_same_group(ch, rch) )
			multi_hit( rch, victim, TYPE_UNDEFINED );
		    continue;
		}

		/*
		 * NPC's assist NPC's of same type or 12.5% chance regardless.
		 */
		if ( IS_NPC(rch) && !IS_AFFECTED(rch, AFF_CHARM)
		&&  !IS_SET(rch->act, ACT_NOASSIST) )
		{
		    if ( char_died(ch) )
			break;
		    if ( rch->pIndexData == ch->pIndexData
		    ||   number_bits( 3 ) == 0 )
		    {
			CHAR_DATA *vch;
			CHAR_DATA *target;
			int number;

			target = NULL;
			number = 0;			for ( vch = ch->in_room->first_person; vch; vch = vch->next )
			{
			    if ( can_see( rch, vch )
			    &&   is_same_group( vch, victim )
			    &&   number_range( 0, number ) == 0 )
			    {
				target = vch;
				number++;
			    }
			}

			if ( target )
			    multi_hit( rch, target, TYPE_UNDEFINED );
		    }
		}
	    }
	}
    }
}
    return;
}



/*
 * Do one group of attacks.
 */
ch_ret multi_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    int     chance;
    int	    dual_bonus;
    ch_ret  retcode=rNONE;
   
    if ( !ch || !victim )
	return rNONE;

	arrest(ch);
	if ( check_warrents(ch)==TRUE)
	return rNONE;

    if ( IS_AFFECTED(victim,AFF_REFLECT_DAMAGE) )
      act(AT_RED,"$n's chest glows, as some damage reflects!",victim,NULL,NULL,TO_ROOM);


    /* add timer if player is attacking another player */
    if ( !IS_NPC(ch) && !IS_NPC(victim) )
      add_timer( ch, TIMER_RECENTFIGHT, 20, NULL, 0 );

    if ( !IS_NPC(ch) && IS_SET( ch->act, PLR_NICE ) && !IS_NPC( victim ) )
      return rNONE;

    if ( ch->stun_rounds > 0 )
    {
	ch->stun_rounds--;
	return rNONE;
    }

     if ( char_died(victim) )
	return rCHAR_DIED;

   if ( ch && victim && ch->in_room && victim->in_room && 
        ( ch->in_room == victim->in_room ) )
    {
    if ( (retcode = one_hit( ch, victim, dt )) != rNONE )
      return retcode;
    }

    if ( who_fighting( ch ) != victim || dt == gsn_backstab || 
         dt == gsn_circle || dt == gsn_assassinate )
	return rNONE;
	
    /* Very high chance of hitting compared to chance of going berserk */
    /* 40% or higher is always hit.. don't learn anything here though. */
    /* -- Altrag */
    chance = IS_NPC(ch) ? 100 : (ch->pcdata->learned[gsn_berserk]*5/2);

    if ( char_died(victim) )
	return rCHAR_DIED;

    if ( IS_AFFECTED(ch, AFF_BERSERK) && number_percent() < chance )
      if ( (retcode = one_hit( ch, victim, dt )) != rNONE ||
            who_fighting( ch ) != victim )
        return retcode;

    if ( char_died(victim) )
	return rCHAR_DIED;

    if ( get_eq_char( ch, WEAR_DUAL_WIELD ) )
    {
      dual_bonus = IS_NPC(ch) ? (ch->level / 10) : (ch->pcdata->learned[gsn_dual_wield] / 10);
      chance = IS_NPC(ch) ? ch->level : ch->pcdata->learned[gsn_dual_wield];
      if ( number_percent( ) < chance )
      {
	learn_from_success( ch, gsn_dual_wield );
	retcode = one_hit( ch, victim, dt );
	if ( retcode != rNONE || who_fighting( ch ) != victim )
	    return retcode;
      }
      else
	learn_from_failure( ch, gsn_dual_wield );
    }
    else
      dual_bonus = 0;

    if ( ch->move < 10 )
      dual_bonus = -20;

    /*
     * NPC predetermined number of attacks			-Thoric
     */
    if ( IS_NPC(ch) && ch->numattacks > 0 )
    {
	for ( chance = 0; chance <= ch->numattacks; chance++ )
	{
          if ( char_died(victim) )
    	    return rCHAR_DIED;

	   retcode = one_hit( ch, victim, dt );
	   if ( retcode != rNONE || who_fighting( ch ) != victim )
	     return retcode;
	}
	return retcode;
    }

    chance = IS_NPC(ch) ? ch->level
	   : (int) ((ch->pcdata->learned[gsn_second_attack]+dual_bonus)/1.5);
    if ( number_percent( ) < chance )
    {
       if ( char_died(victim) )
    	   return rCHAR_DIED;

	learn_from_success( ch, gsn_second_attack );
	retcode = one_hit( ch, victim, dt );
	if ( retcode != rNONE || who_fighting( ch ) != victim )
	    return retcode;
    }
    else
	learn_from_failure( ch, gsn_second_attack );

    chance = IS_NPC(ch) ? ch->level
	   : (int) ((ch->pcdata->learned[gsn_third_attack]+(dual_bonus*1.5))/2);
    if ( number_percent( ) < chance )
    {
        if ( char_died(victim) )
  	    return rCHAR_DIED;

	learn_from_success( ch, gsn_third_attack );
	retcode = one_hit( ch, victim, dt );
	if ( retcode != rNONE || who_fighting( ch ) != victim )
	    return retcode;
    }
    else
	learn_from_failure( ch, gsn_third_attack );

    chance = IS_NPC(ch) ? ch->level
	   : (int) ((ch->pcdata->learned[gsn_fourth_attack]+(dual_bonus*2))/3);
    if ( number_percent( ) < chance )
    {
          if ( char_died(victim) )
	       return rCHAR_DIED;

	learn_from_success( ch, gsn_fourth_attack );
	retcode = one_hit( ch, victim, dt );
	if ( retcode != rNONE || who_fighting( ch ) != victim )
	    return retcode;
    }
    else
	learn_from_failure( ch, gsn_fourth_attack );

    chance = IS_NPC(ch) ? ch->level
	   : (int) ((ch->pcdata->learned[gsn_fifth_attack]+(dual_bonus*3))/4);
    if ( number_percent( ) < chance )
    {
         if ( char_died(victim) )
  	    return rCHAR_DIED;

	learn_from_success( ch, gsn_fifth_attack );
	retcode = one_hit( ch, victim, dt );
	if ( retcode != rNONE || who_fighting( ch ) != victim )
	    return retcode;
    }
    else
	learn_from_failure( ch, gsn_fifth_attack );

   chance = IS_NPC(ch) ? ch->level
	  : (int) ((ch->pcdata->learned[gsn_sixth_attack]+(dual_bonus*4))/5);
   if ( number_percent( ) < chance )
   {
         if ( char_died(victim) )
	     return rCHAR_DIED;

	learn_from_success(ch, gsn_sixth_attack );
	retcode = one_hit(ch, victim, dt );
	if( retcode != rNONE || who_fighting( ch ) != victim )
	   return retcode;
   }
   else
	learn_from_failure(ch, gsn_sixth_attack );

   chance = IS_NPC(ch) ? ch->level
	  : (int) ((ch->pcdata->learned[gsn_seventh_attack]+(dual_bonus*5))/6);
   if ( number_percent( ) < chance )
   {
        if ( char_died(victim) )
	    return rCHAR_DIED;

	learn_from_success(ch, gsn_seventh_attack );
	retcode = one_hit(ch, victim, dt );
	if( retcode != rNONE || who_fighting( ch ) != victim )
	   return retcode;
   }
   else
	learn_from_failure(ch, gsn_seventh_attack );


    retcode = rNONE;

    if ( char_died(victim) )
	return rCHAR_DIED;

    chance = IS_NPC(ch) ? (int) (ch->level / 2) : 0;
    if ( number_percent( ) < chance )
	retcode = one_hit( ch, victim, dt );

    if ( retcode == rNONE )
    {
	int move;

	if ( !IS_AFFECTED(ch, AFF_FLYING)
	&&   !IS_AFFECTED(ch, AFF_FLOATING) )
	  move = encumbrance( ch, movement_loss[UMIN(SECT_MAX-1, ch->in_room->sector_type)] );
	else
	  move = encumbrance( ch, 1 );
	if ( ch->move )
	  ch->move = UMAX( 0, ch->move - move );
    }

    return retcode;
}


/*
 * Weapon types, haus
 */
int weapon_prof_bonus_check( CHAR_DATA *ch, OBJ_DATA *wield, int *gsn_ptr )
{
    int bonus;

    bonus = 0;	*gsn_ptr = -1;
    if ( !IS_NPC(ch) && ch->level > 5 && wield )   
    {
	switch(wield->value[3])
	{
	   default:	*gsn_ptr = -1;			break;
           case 0:      *gsn_ptr = gsn_pugilism;        break;
           case 1:	*gsn_ptr = gsn_long_blades;	break;
           case 2:	*gsn_ptr = gsn_short_blades;	break;
           case 3:      *gsn_ptr = gsn_short_blades;    break;
           case 4:	*gsn_ptr = gsn_flexible_arms;	break;
           case 5:	*gsn_ptr = gsn_talonous_arms;	break;
	   case 6:	*gsn_ptr = gsn_pugilism;	break;
           case 7:      *gsn_ptr = gsn_bludgeons;       break;
           case 8:	*gsn_ptr = gsn_bludgeons;	break;
	   case 10:     *gsn_ptr = gsn_short_blades;    break;
           case 11:     *gsn_ptr = gsn_short_blades;    break;
	   case 12:     *gsn_ptr = gsn_bludgeons;       break;
	   case 13:	
	   case 14:	
	   case 15:	
	   case 16:	
	   case 17:	*gsn_ptr = gsn_missile_weapons; break;
	}
	if ( *gsn_ptr != -1 )
	  bonus = (int) ((ch->pcdata->learned[*gsn_ptr] -50)/10);

       /* Reduce weapon bonuses for misaligned clannies.
       if ( IS_CLANNED(ch) )
       {
          bonus = bonus / 
          ( 1 + abs( ch->alignment - ch->pcdata->clan->alignment ) / 1000 );
       }*/

	if ( IS_DEVOTED( ch ) )
	{
	   bonus = bonus - abs( ch->pcdata->favor ) / -100 ;
	}

    }
    return bonus;
}

/*
 * Calculate the tohit bonus on the object and return RIS values.
 * -- Altrag
 */
int obj_hitroll( OBJ_DATA *obj )
{
	int tohit = 0;
	AFFECT_DATA *paf;
	
	for ( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
		if ( paf->location == APPLY_HITROLL )
			tohit += paf->modifier;
	for ( paf = obj->first_affect; paf; paf = paf->next )
		if ( paf->location == APPLY_HITROLL )
			tohit += paf->modifier;
	return tohit;
}

/*
 * Offensive shield level modifier
 */
sh_int off_shld_lvl( CHAR_DATA *ch, CHAR_DATA *victim )
{
    sh_int lvl;

    if ( !IS_NPC(ch) )		/* players get much less effect */
    {
	lvl = UMAX( 1, (ch->level - 10) / 2 );
	if ( number_percent() + (victim->level - lvl) < 35 )
	  return lvl;
	else
	  return 0;
    }
    else
    {
	lvl = ch->level / 2;
	if ( number_percent() + (victim->level - lvl) < 70 )
	  return lvl;
	else
	  return 0;
    }
}

/*
 * Hit one guy once.
 */
ch_ret one_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    OBJ_DATA *wield;
    int victim_ac;
    int thac0;
    int thac0_00;
    int thac0_32;
    int plusris;
    long dam, x;
    int diceroll;
    int attacktype, cnt;
    int	prof_bonus;
    int	prof_gsn;
    ch_ret retcode=rNONE;

    if ( !ch || !victim )
	return rVICT_DIED;

    if ( char_died(victim) )
	return rCHAR_DIED;

    /*
     * Can't beat a dead char!
     * Guard against weird room-leavings.
     */
    if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return rVICT_DIED;

    /*
     * Rooms seem to get fucked in fights - fix it --GW
     */
    if ( ch->in_room )
      ch->was_in_room = ch->in_room;
    if ( victim->in_room )
      victim->was_in_room = victim->in_room;
    if ( !ch->in_room && ch->was_in_room )
      char_to_room(ch,ch->was_in_room);
    if ( !victim->in_room && victim->was_in_room )
      char_to_room(victim,victim->was_in_room);

    /*
     * New Stun Code --GW
     */
    if ( ch->stun_rounds > 0 )
	return rNONE;

    /*
     * Figure out the weapon doing the damage			-Thoric
     */
    if ( (wield = get_eq_char( ch, WEAR_DUAL_WIELD )) != NULL )
    {
       if ( dual_flip == FALSE )
       {
	 dual_flip = TRUE;
	 wield = get_eq_char( ch, WEAR_WIELD );
       }
       else
	 dual_flip = FALSE;
    }
    else
      wield = get_eq_char( ch, WEAR_WIELD );

    prof_bonus = weapon_prof_bonus_check( ch, wield, &prof_gsn );

    if ( ch->fighting		/* make sure fight is already started */
    &&   dt == TYPE_UNDEFINED
    &&   IS_NPC(ch)
    &&   ch->attacks != 0 )
    {
	cnt = 0;
	for ( ;; )
	{
	   x = number_range( 0, 6 );
	   attacktype = 1 << x;
	   if ( IS_SET( ch->attacks, attacktype ) )
	     break;
	   if ( cnt++ > 16 )
	   {
	     attacktype = 0;
	     break;
	   }
	}
	if ( attacktype == ATCK_BACKSTAB )
	  attacktype = 0;
	if ( wield && number_percent( ) > 25 )
	  attacktype = 0;
	switch ( attacktype )
	{
	  default:
	    break;
	  case ATCK_BITE:
	    do_bite( ch, "" );
	    retcode = global_retcode;
	    break;
	  case ATCK_CLAWS:
	    do_claw( ch, "" );
	    retcode = global_retcode;
	    break;
	  case ATCK_TAIL:
	    do_tail( ch, "" );
	    retcode = global_retcode;
	    break;
	  case ATCK_STING:
	    do_sting( ch, "" );
	    retcode = global_retcode;
	    break;
	  case ATCK_PUNCH:
	    do_punch( ch, "" );
	    retcode = global_retcode;
	    break;
	  case ATCK_KICK:
	    do_kick( ch, "" );
	    retcode = global_retcode;
	    break;
	  case ATCK_TRIP:
	    attacktype = 0;
	    break;
	}
	if ( attacktype )
	  return retcode;
    }

    if ( dt == TYPE_UNDEFINED )
    {
	dt = TYPE_HIT;
	if ( wield && wield->item_type == ITEM_WEAPON )
	    dt += wield->value[3];
    }

    /*
     * Calculate to-hit-armor-class-0 versus armor.
     */
    if ( ch->class == -1 )
	return 0;

    if ( IS_NPC(ch) )
    {
	thac0_00 = ch->mobthac0;
	thac0_32 =  0;
    }
    else
    {
	thac0_00 = class_table[ch->class]->thac0_00;
	thac0_32 = class_table[ch->class]->thac0_32;
    }
    thac0     = interpolate( ch->level, thac0_00, thac0_32 ) - GET_HITROLL(ch);
    victim_ac = UMAX( -19, (int) (GET_AC(victim) / 10) );

    /* if you can't see what's coming... */
    if ( wield && !can_see_obj( victim, wield) )
	victim_ac += 1;
    if ( !can_see( ch, victim ) )
	victim_ac -= 4;

    /*
     * "learning" between combatients.  Takes the intelligence difference,
     * and multiplies by the times killed to make up a learning bonus
     * given to whoever is more intelligent		-Thoric
     */
    if ( ch->fighting && ch->fighting->who == victim )
    {
	sh_int times = ch->fighting->timeskilled;

	if ( times )
	{
	    sh_int intdiff = get_curr_int(ch) - get_curr_int(victim);

	    if ( intdiff != 0 )
		victim_ac += (intdiff*times)/10;
	}
    }

    /* Weapon proficiency bonus */
    victim_ac += prof_bonus;

    /*
     * The moment of excitement!
     */
    while ( ( diceroll = number_bits( 5 ) ) >= 20 )
	;

    if ( diceroll == 0
    || ( diceroll != 19 && diceroll < thac0 - victim_ac ) )
    {
	/* Miss. */
	if ( prof_gsn != -1 )
	  learn_from_failure( ch, prof_gsn );
	damage( ch, victim, 0, dt );
	tail_chain( );
	return rNONE;
    }

    /*
     * Hit.
     * Calc damage.
     */

    /* Thoric's Dice job re-fixed by Greywolf, thanks to G. Moody. --GW */

       dam = number_range( ch->barenumdie, ch->baresizedie * ch->barenumdie ) + ch->damplus;
       dam += number_range( ch->perm_str, ( ch->level * 2.4 ));

	if ( wield )
	dam += number_range(wield->value[1],wield->value[2]);

    /*
     * Bonuses.
     */
    dam += GET_DAMROLL(ch);

    if ( prof_bonus )
      dam += prof_bonus / 4;

    if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_enhanced_damage] > 0 )
    {
	dam += (int) (dam * ch->pcdata->learned[gsn_enhanced_damage] / 120);
	learn_from_success( ch, gsn_enhanced_damage );
    }

    if ( !IS_AWAKE(victim) )
	dam *= 2;
    if ( dt == gsn_backstab )
	dam *= (2 + URANGE( 2, ch->level - (victim->level/4), 30 ) / 8);
    if ( dt == gsn_assassinate )
	dam *= (4 + URANGE( 2, ch->level - (victim->level/2), 30) / 4 );

/* This comment-keyword is here for easy jumping -- Cal  (CIRCLE) */
/* It's a little silly I know but hey it's original :P */
    if ( dt == gsn_circle )
 	dam *= (2 + URANGE( 2, ch->level - (victim->level/4), 30 ) / 16); 

    plusris = 0;

	 wield = get_eq_char( ch, WEAR_WIELD );

    if ( wield && !IS_SET(wield->magic_flags, ITEM_DISSOLVE_IMM) )
    {
       
      if ( IS_SET( wield->extra_flags, ITEM_MAGIC ) )
        dam = ris_damage( victim, dam, RIS_MAGIC );
      else
        dam = ris_damage( victim, dam, RIS_NONMAGIC );

      /*
       * Handle PLUS1 - PLUS6 ris bits vs. weapon hitroll	-Thoric
       */
      plusris = obj_hitroll( wield );
    }
    else
      dam = ris_damage( victim, dam, RIS_NONMAGIC );

    /* check for RIS_PLUSx 					-Thoric */
  if ( (wield && !IS_SET(wield->magic_flags, ITEM_DISSOLVE_IMM)) || !wield )
  {
    if ( dam )
    {
	int x, res, imm, sus, mod;

	if ( plusris )
	   plusris = RIS_PLUS1 << UMIN(plusris, 7);

	/* initialize values to handle a zero plusris */
	imm = res = -1;  sus = 1;

	/* find high ris */
	for ( x = RIS_PLUS1; x <= RIS_PLUS6; x <<= 1 )
	{
	   if ( IS_SET( victim->immune, x ) )
		imm = x;
	   if ( IS_SET( victim->resistant, x ) )
		res = x;
	   if ( IS_SET( victim->susceptible, x ) )
		sus = x;
	}
	mod = 10;
	if ( imm >= plusris )
	  mod -= 10;
	if ( res >= plusris )
	  mod -= 2;
	if ( sus <= plusris )
	  mod += 2;

	/* check if immune */
	/* Cackle.. opps.. I guess immune isnt 100% anymore eh... GW */
	if ( (x=number_range(1,100)) > 75 )
	{
  	  if ( mod <= 0 )
	    dam = -1;
	  if ( mod != 10 )
	    dam = (dam * mod) / 10;
	}
    }
}
    if ( prof_gsn != -1 )
    {
      if ( dam > 0 )
        learn_from_success( ch, prof_gsn );
      else
        learn_from_failure( ch, prof_gsn );
    }

if ( (wield && !IS_SET(wield->magic_flags, ITEM_DISSOLVE_IMM) ) || !wield )
{
    /* immune to damage */
    if ( dam == -1 )
    {
	if ( dt >= 0 && dt < top_sn )
	{
	    SKILLTYPE *skill = skill_table[dt];
	    bool found = FALSE;

	    if ( skill->imm_char && skill->imm_char[0] != '\0' )
	    {
		act( AT_HIT, skill->imm_char, ch, NULL, victim, TO_CHAR );
		found = TRUE;
	    }
	    if ( skill->imm_vict && skill->imm_vict[0] != '\0' )
	    {
		act( AT_HITME, skill->imm_vict, ch, NULL, victim, TO_VICT );
		found = TRUE;
	    }
	    if ( skill->imm_room && skill->imm_room[0] != '\0' )
	    {
		act( AT_ACTION, skill->imm_room, ch, NULL, victim, TO_NOTVICT );
		found = TRUE;
	    }
	    if ( found )
	      return rNONE;
	}
	dam = 0;
    }
}
    if ( !ch )
        return rNONE;
    if ( !victim )
        return rNONE;

/* Damage Cap --GW */
/* Player vs. Player Cap */
if ( !IS_NPC(ch) && !IS_NPC(victim) && dam > 1500 )
	dam = 1500;
/* Mob vs. Player */
if ( IS_NPC(ch) && !IS_NPC(victim) && dam > 4500 )
        dam = 4500;
/* Player vs. Mob */
if ( !IS_NPC(ch) && IS_NPC(victim) && dam > 2000 )
        dam = 2000;

    if ( (retcode = damage( ch, victim, dam, dt )) != rNONE )
      return retcode;
    if ( char_died(ch) )
      return rCHAR_DIED;
    if ( char_died(victim) )
      return rVICT_DIED;

    retcode = rNONE;
    if ( dam == 0 )
      return retcode;

/* weapon spells	-Thoric */
    if ( wield && ((x=number_range(1,100))>50) /* 50% fail chance..--GW */
    &&  (!IS_SET(wield->magic_flags, ITEM_DISSOLVE_IMM) && !IS_SET(victim->immune, RIS_MAGIC))
    &&  !IS_SET(victim->in_room->room_flags, ROOM_NO_MAGIC) )
    {
	AFFECT_DATA *aff;
	
	if ( retcode != rNONE || char_died(ch) || char_died(victim) )
		return rCHAR_DIED;
	for ( aff = wield->pIndexData->first_affect; aff; aff = aff->next )
	   if ( aff->location == APPLY_WEAPONSPELL
	   &&   IS_VALID_SN(aff->modifier)
	   &&   !char_died(ch)
	   &&   !char_died(victim)
	   &&   skill_table[aff->modifier]->spell_fun )
		retcode = (*skill_table[aff->modifier]->spell_fun) (aff->modifier, (ch->level+3)/3, ch, victim );
	if ( retcode != rNONE || char_died(ch) || char_died(victim) )
		return retcode;
	for ( aff = wield->first_affect; aff; aff = aff->next )
	   if ( aff->location == APPLY_WEAPONSPELL
	   &&   IS_VALID_SN(aff->modifier)
	   &&   !char_died(ch)
	   &&   !char_died(victim)
	   &&   skill_table[aff->modifier]->spell_fun )
		retcode = (*skill_table[aff->modifier]->spell_fun) (aff->modifier, (ch->level+3)/3, ch, victim );
	if ( retcode != rNONE || char_died(ch) || char_died(victim) )
		return retcode;
    }

    /*
     * magic shields that retaliate				-Thoric
     */
    if ( IS_AFFECTED( victim, AFF_FIRESHIELD )
    &&  !IS_AFFECTED( ch, AFF_FIRESHIELD ) )
	retcode = spell_fireball( gsn_fireball, off_shld_lvl(victim, ch), victim, ch );
    if ( retcode != rNONE || char_died(ch) || char_died(victim) )
      return retcode;

    if ( IS_AFFECTED( victim, AFF_ICESHIELD )
    &&  !IS_AFFECTED( ch, AFF_ICESHIELD ) )
         retcode = spell_chill_touch( gsn_chill_touch, off_shld_lvl(victim, ch), victim, ch );
    if ( retcode != rNONE || char_died(ch) || char_died(victim) )
      return retcode;

    if ( IS_AFFECTED( victim, AFF_SHOCKSHIELD )
    &&  !IS_AFFECTED( ch, AFF_SHOCKSHIELD ) )
	retcode = spell_lightning_bolt( gsn_lightning_bolt, off_shld_lvl(victim, ch), victim, ch );
    if ( retcode != rNONE || char_died(ch) || char_died(victim) )
      return retcode;

    tail_chain( );
    return retcode;
}

/*
 * Calculate damage based on resistances, immunities and suceptibilities
 *					-Thoric
 */
sh_int ris_damage( CHAR_DATA *ch, long dam, int ris )
{
   sh_int modifier;
   int rand=0;

   modifier = 10;
   if ( IS_SET(ch->immune, ris ) )
     modifier -= 10;
   if ( IS_SET(ch->resistant, ris ) )
     modifier -= 2;
   if ( IS_SET(ch->susceptible, ris ) )
     modifier += 2;

/* Failure.. it is NOT an Immunity!--GW*/
if ( modifier < 0 && ch->fighting && (rand=number_range(1,100))<(ch->fighting->who->level/4))
  return dam;

   if ( modifier <= 0 )
     return -1;
   if ( modifier == 10 )
     return dam;
   return (dam * modifier) / 10;
}


/*
 * Inflict damage from a hit.
 */
ch_ret damage( CHAR_DATA *ch, CHAR_DATA *victim, long dam, int dt )
{
    char buf1[MAX_STRING_LENGTH];
    sh_int dameq;
    sh_int maxdam;
    bool npcvict;
    bool loot;
    long xp_gain;
    OBJ_DATA *damobj;
    OBJ_DATA *obj;
    ch_ret retcode;
    sh_int dampmod;
    CHAR_DATA *gch;
    bool death;
    bool selfdeath;
    int init_gold, new_gold, gold_diff;
    extern sh_int dam_no_msg;

    death = FALSE;
    selfdeath = FALSE;

    retcode = rNONE;

    if ( !ch )
    {
	bug( "Damage: null ch!", 0 );
	return rERROR;
    }
    if ( !victim )
    {
	bug( "Damage: null victim!", 0 );
	return rVICT_DIED;
    }

    if ( victim->position == POS_DEAD )
	return rVICT_DIED;

    npcvict = IS_NPC(victim);

    if ( ch->stun_rounds > 0 )
     return rNONE;

    /*
     * Check damage types for RIS				-Thoric
     */
    if ( dam && dt != TYPE_UNDEFINED )
    {
	if ( IS_FIRE(dt) )
	  dam = ris_damage(victim, dam, RIS_FIRE);
	else
	if ( IS_COLD(dt) )
	  dam = ris_damage(victim, dam, RIS_COLD);
	else
	if ( IS_ACID(dt) )
	  dam = ris_damage(victim, dam, RIS_ACID);
	else
	if ( IS_ELECTRICITY(dt) )
	  dam = ris_damage(victim, dam, RIS_ELECTRICITY);
	else
	if ( IS_ENERGY(dt) )
	  dam = ris_damage(victim, dam, RIS_ENERGY);
	else
	if ( IS_DRAIN(dt) )
	  dam = ris_damage(victim, dam, RIS_DRAIN);
	else
	if ( dt == gsn_poison || IS_POISON(dt) )
	  dam = ris_damage(victim, dam, RIS_POISON);
	else
	if ( dt == (TYPE_HIT + 7) || dt == (TYPE_HIT + 8) )
	  dam = ris_damage(victim, dam, RIS_BLUNT);
	else
	if ( dt == (TYPE_HIT + 2) || dt == (TYPE_HIT + 11)
	||   dt == (TYPE_HIT + 10) )
	  dam = ris_damage(victim, dam, RIS_PIERCE);
	else
	if ( dt == (TYPE_HIT + 1) || dt == (TYPE_HIT + 3)
	||   dt == (TYPE_HIT + 4) || dt == (TYPE_HIT + 5) )
	  dam = ris_damage(victim, dam, RIS_SLASH);

	if ( dam == -1 )
	{
	    if ( dt >= 0 && dt < top_sn )
	    {
		bool found = FALSE;
		SKILLTYPE *skill = skill_table[dt];


		if ( skill->imm_char && skill->imm_char[0] != '\0' )
		{
		   act( AT_HIT, skill->imm_char, ch, NULL, victim, TO_CHAR );
		   found = TRUE;
		}
		if ( skill->imm_vict && skill->imm_vict[0] != '\0' )
		{
		   act( AT_HITME, skill->imm_vict, ch, NULL, victim, TO_VICT );
		   found = TRUE;
		}
		if ( skill->imm_room && skill->imm_room[0] != '\0' )
		{
		   act( AT_ACTION, skill->imm_room, ch, NULL, victim, TO_NOTVICT );
		   found = TRUE;
		}
		if ( found )
		   return rNONE;
	    }
	    dam = 0;
	}
    }

    if ( dam && npcvict && ch != victim )
    {
	if ( !IS_SET( victim->act, ACT_SENTINEL ) )
 	{
	   if ( victim->hunting )
	   {
	     if ( victim->hunting->who != ch )
	     {
		STRFREE( victim->hunting->name );
		victim->hunting->name = QUICKLINK( ch->name );
		victim->hunting->who  = ch;
	     }
           }
	   else
	     start_hunting( victim, ch );
	}

      if ( victim->hating )
      {
	if ( victim->hating->who != ch )
	{
	   if (victim && victim->hating && victim->hating->name )
	   STRFREE( victim->hating->name );
	   victim->hating->name = QUICKLINK( ch->name );
	   victim->hating->who  = ch;
	}
      }
      else
	start_hating( victim, ch );
    }

    /*
     * Stop up any residual loopholes.
     */
    maxdam = ch->level * 30;
    if ( dt == gsn_backstab )
      maxdam = ch->level * 80;
    if ( dt == gsn_assassinate )
      maxdam = ch->level * 80;
    if ( ( dam > maxdam) &&(ch->level2 != 50 ))
	dam = maxdam;

    if ( victim != ch )
    {
	/*
	 * Certain attacks are forbidden.
	 * Most other attacks are returned.
	 */
         if ( !ch )
	    return rNONE;

	 if ( !victim )
	    return rNONE;

	if (ch && victim && is_safe( ch, victim ) )
	    return rNONE;


	if ( victim->position > POS_STUNNED )
	{
	    if ( !victim->fighting )
		set_fighting( victim, ch );
	    if ( victim->fighting )
		victim->position = POS_FIGHTING;
	}

	if ( victim->position > POS_STUNNED )
	{
	    if ( !ch->fighting )
		set_fighting( ch, victim );

	    /*
	     * If victim is charmed, ch might attack victim's master.
	     */
	    if ( IS_NPC(ch)
	    &&   npcvict
	    &&   IS_AFFECTED(victim, AFF_CHARM)
	    &&   victim->master
	    &&   victim->master->in_room == ch->in_room
	    &&   number_bits( 3 ) == 0 )
	    {
		stop_fighting( ch, FALSE );
		retcode = multi_hit( ch, victim->master, TYPE_UNDEFINED );
		return retcode;
	    }
	}


	/*
	 * More charm stuff.
	 */
	if ( victim->master && victim->master == ch )
	    stop_follower( victim );

        /* Pkill stuff.  If a deadly attacks another deadly or is attacked by one,
         * then ungroup any nondealies.  Disabled untill I can figure out the right
         * way to do it.
         */
         
/*	{
	  sh_int anopc = 0;  * # of (non-pkill) pc in a (ch) *
	  sh_int bnopc = 0;  * # of (non-pkill) pc in b (victim) *
	  CHAR_DATA *lch;   * leader ch *

	  * count the # of non-pkill pc in a ( not including == ch ) *
	  for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
	    if ( is_same_group( ch, gch ) && !IS_NPC( gch )
		&& !IS_PKILL( gch ) && ( ch != gch ) ) anopc++;

	  * count the # of non-pkill pc in b ( not including == victim ) *
	  for ( gch = victim->in_room->first_person; gch; gch = gch->next_in_room )
	    if ( is_same_group( victim, gch ) && !IS_NPC( gch )
		&& !IS_PKILL( gch ) && ( victim != gch ) ) bnopc++;


	  * only consider disbanding if both groups have 1(+) non-pk pc *
	  if ( ( bnopc > 0 ) && ( anopc > 0 ) )
	  {
	    * look at group a through ch's leader first *
	    lch = ch->leader ? ch->leader : ch;
	    if ( lch != ch )
	    {
	      * stop following leader if it isn't pk *
	      if ( !IS_NPC(lch) && !IS_PKILL( lch ) )
		stop_follower( ch );
	       else
		{
		  * disband non-pk members from lch's group if it is pk *
		  for ( gch = ch->in_room->first_person; gch; 
			gch = gch->next_in_room )
		  {
		    if ( is_same_group( lch, gch )
		      && ( lch != gch )
		      && !IS_NPC(gch) && !IS_PKILL( gch ) )
		      stop_follower( gch );
		  }
		}
	    } 
	   else
	    for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
	    {
	      * ch is leader - disband non-pks from group *
	      if ( is_same_group( ch, gch )
		&& ( ch != gch )
	        && ( !IS_PKILL( gch ) && !IS_NPC( gch ) ) )
	        stop_follower( gch );
	    }

	    * time to look at the victims group through its leader *
	    lch = victim->leader ? victim->leader : victim;

	    if ( lch != victim )
	    {
	      * if leader isn't deadly, stop following lch *
	      if ( !IS_PKILL( lch ) && !IS_NPC( lch ) )
	        stop_follower( victim );
	       else
		{
		  * lch is pk, disband non-pk's from group *
	          for ( gch = victim->in_room->first_person; gch; 
			gch = gch->next_in_room )
	    	  {
	     	    if ( is_same_group( lch, gch )
		      && ( lch != gch )
	              && ( !IS_PKILL( gch ) && !IS_NPC( gch ) ) )
	              stop_follower( gch );
	          }
		}
	    }
	   else
	    {
	      * victim is leader of group - disband non-pks *
	      for ( gch = victim->in_room->first_person; gch;
		 gch = gch->next_in_room )
	      {
		if ( is_same_group( victim, gch )
		  && ( victim != gch )
 		  && !IS_PKILL( gch ) && !IS_NPC( gch ) )
		  stop_follower( gch );
	      }
	    }
	  }
	}*/

	{
	  sh_int anopc = 0;  /* # of (non-pkill) pc in a (ch) */
	  sh_int bnopc = 0;  /* # of (non-pkill) pc in b (victim) */

	  /* count the # of non-pkill pc in a ( not including == ch ) */
	  for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
	    if ( is_same_group( ch, gch ) && !IS_NPC( gch )
		&& !IS_PKILL( gch ) && ( ch != gch ) ) anopc++;

	  /* count the # of non-pkill pc in b ( not including == victim)*/
	  for ( gch = victim->in_room->first_person; gch; gch = gch->next_in_room )
	    if ( is_same_group( victim, gch ) && !IS_NPC( gch )
		&& !IS_PKILL( gch ) && ( victim != gch ) ) bnopc++;


	  /* only consider disbanding if both groups have 1(+) non-pk pc,
	     or when one participant is pc, and the other group has 1(+)
	     pk pc's (in the case that participant is only pk pc in group)*/
	  if ( ( bnopc > 0 && anopc > 0 )
	    || ( bnopc > 0 && !IS_NPC(ch) )
	    || ( anopc > 0 && !IS_NPC(victim) ) )
	  {
	    /* Disband from same group first */
	    if ( is_same_group(ch, victim) )
	    {
	      /* Messages to char and master handled in stop_follower */
	      act( AT_ACTION, "$n disbands from $N's group.",
	           (ch->leader == victim) ? victim : ch, NULL,
	           (ch->leader == victim) ? victim->master : ch->master,
	           TO_NOTVICT );
	      if ( ch->leader == victim )
	        stop_follower(victim);
	      else
	        stop_follower(ch);
	    }
	    /* if leader isnt pkill, leave the group and disband ch */
	    if ( ch->leader != NULL && !IS_NPC(ch->leader) &&
	        !IS_PKILL(ch->leader) )
	    {
	      act( AT_ACTION, "$n disbands from $N's group.", ch, NULL,
	           ch->master, TO_NOTVICT );
	      stop_follower( ch );
	    }
	    else
	    {
	      for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
	        if ( is_same_group(gch, ch) && !IS_NPC( gch ) &&
	             !IS_PKILL( gch ) && gch != ch )
	        {
	          act( AT_ACTION, "$n disbands from $N's group.", ch, NULL,
	               gch->master, TO_NOTVICT );
	          stop_follower( gch );
	        }
	    }
	    /* if leader isnt pkill, leave the group and disband victim */
	    if ( victim->leader != NULL && !IS_NPC(victim->leader) &&
	        !IS_PKILL(victim->leader) )
	    {
	      act( AT_ACTION, "$n disbands from $N's group.", victim, NULL,
	           victim->master, TO_NOTVICT );
	      stop_follower( victim );
	    }
	    else
	    {
	      for ( gch = victim->in_room->first_person; gch; gch = gch->next_in_room )
	        if ( is_same_group(gch, victim) && !IS_NPC( gch ) &&
	             !IS_PKILL( gch ) && gch != victim )
	        {
	          act( AT_ACTION, "$n disbands from $N's group.", gch, NULL,
	               gch->master, TO_NOTVICT );
	          stop_follower( gch );
	        }
	    }
	  }
	}
	    
/*         for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
         {
           if ( is_same_group( ch, gch ) 
                && ( IS_PKILL( ch ) != IS_PKILL( gch ) ) )
           {
             stop_follower( ch );
             stop_follower( gch );
           }
         }
                
         for ( gch = victim->in_room->first_person; gch; gch = gch->next_in_room )
         {
           if ( is_same_group( victim, gch ) 
                && ( IS_PKILL( victim ) != IS_PKILL( gch ) ) )
           {
             stop_follower( victim ); 
             stop_follower( gch );
           }
         }
*/
	/*
	 * Inviso attacks ... not.
	 */
	if ( IS_AFFECTED(ch, AFF_INVISIBLE) || IS_SET(ch->act, ACT_MOBINVIS) )
	{
	    affect_strip( ch, gsn_invis );
	    affect_strip( ch, gsn_mass_invis );
	    xREMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
	    REMOVE_BIT( ch->act, ACT_MOBINVIS );	   
	act( AT_MAGIC, "$n fades into existence.", ch, NULL, NULL, TO_ROOM );
	}

	/* Take away Hide */
	if ( IS_AFFECTED(ch, AFF_HIDE) )
	     xREMOVE_BIT(ch->affected_by, AFF_HIDE);     

	/*
	 * Damage modifiers.
	 */
	if ( IS_AFFECTED(victim, AFF_SANCTUARY) )
	    dam /= 2;

	if ( IS_AFFECTED(victim, AFF_PROTECT) && IS_EVIL(ch) )
	    dam -= (int) (dam / 4);

	/*
	 * Check for disarm, trip, parry, and dodge.
	 */
	if ( dt >= TYPE_HIT && victim->stun_rounds < 1 )
	{
	    if ( IS_NPC(ch)
	    &&   IS_SET( ch->attacks, DFND_DISARM )
	    &&   ch->level > 9
	    &&   number_percent( ) < ch->level / 2 )
		disarm( ch, victim );

	    if ( IS_NPC(ch)
	    &&   IS_SET( ch->attacks, ATCK_TRIP )
	    &&   ch->level > 5
	    &&   number_percent( ) < ch->level / 2 )
		trip( ch, victim );

	    if ( check_parry( ch, victim ) )
		return rNONE;
	    if ( check_dodge( ch, victim ) )
		return rNONE;
	}

    /*
     * Check control panel settings and modify damage
     */
    if ( IS_NPC(ch) )
    {
	if ( npcvict )
	  dampmod = sysdata.dam_mob_vs_mob;
	else
	  dampmod = sysdata.dam_mob_vs_plr;
    }
    else
    {
	if ( npcvict )
	  dampmod = sysdata.dam_plr_vs_mob;
	else
	  dampmod = sysdata.dam_plr_vs_plr;
    }
    if ( dampmod > 0 )
      dam = ( dam * dampmod ) / 100;

     /* Make stunned Mobiles/Characters take 1/4 the damage, being thay cannot
      * defend themselves, and are dying to fast --GW
      */
	if ( victim->stun_rounds > 0 )
	   dam = dam/4;

	dam_message( ch, victim, dam, dt );
    }


    /*
     * Code to handle equipment getting damaged, and also support  -Thoric
     * bonuses/penalties for having or not having equipment where hit
     */
    if (dam > 10 && dt != TYPE_UNDEFINED)
    {
	/* get a random body eq part */
	dameq  = number_range(WEAR_LIGHT, WEAR_EYES);
	damobj = get_eq_char(victim, dameq);
	if ( damobj )
	{
	  if ( dam > get_obj_resistance(damobj) )
	  {
	     set_cur_obj(damobj);
	     damage_obj(damobj);
	  }
	  dam -= 5;  /* add a bonus for having something to block the blow */
	}
	else
	  dam += 5;  /* add penalty for bare skin! */
    }


    /* Reflect Damage Stuff --GW */
  if ( IS_AFFECTED(victim, AFF_REFLECT_DAMAGE) )
  {
   CHAR_DATA *dam_char, *dam_next;

    for( dam_char = victim->in_room->first_person; dam_char; dam_char = dam_next )
    {
      dam_next = dam_char->next_in_room;

      /* Don't hit myself.. */
      if ( dam_char == victim || IS_NPC(ch) || dam==0 )
	continue;

      dam_no_msg = 1;
      /* Inflict 1/4th the damage done on each character (yes it hurts!) */
      damage(victim,ch,(dam/4),TYPE_UNDEFINED);
      dam_no_msg = 0;
    }
   dam = dam/4;
  }

    /*
     * Hurt the victim.
     * Inform the victim of his new state.
     */

    if ( dam > 1500 || dam < 0 )
     dam = 1500;

    URANGE(0,victim->hit -= dam,victim->max_hit+5);


    /*
     * Get experience based on % of damage done			-Thoric
     */
    if ( dam && ch != victim
    &&  !IS_NPC(ch) && ch->fighting && ch->fighting->xp )
    {
	if ( ch->fighting->who == victim )
	  xp_gain = URANGE(0, (int) (ch->fighting->xp * dam) / victim->max_hit, 2100000000);
	else
	  xp_gain = URANGE(0, (int) (xp_compute( ch, victim ) * 1.90 * dam) / victim->max_hit, 2100000000);

	if ( IS_NPC(ch->fighting->who) && IS_SET(ch->fighting->who->act,ACT_CREATE_LIFE_MOB))
	xp_gain=0;

	if ( !IS_NPC(ch) )
	ch->pcdata->battle_xp += xp_gain;
	
	gain_exp( ch, xp_gain, TRUE );
    }

    if (!IS_NPC(victim)
    && ( !in_arena(victim) && chaos != 1 ) 
    && !IS_SET(victim->pcdata->flags, PCFLAG_KOMBAT )
    && victim->level >= LEVEL_IMMORTAL
    &&   victim->hit < 1 )
       victim->hit = 1;

    /* Make sure newbies dont die */

    if (!IS_NPC(victim) && NOT_AUTHED(victim) && victim->hit < 1)
	victim->hit = 1;

    if ( dam > 0 && dt > TYPE_HIT
    && !IS_AFFECTED( victim, AFF_POISON )
    &&  is_wielding_poisoned( ch )
    && !IS_SET( victim->immune, RIS_POISON )
    && !saves_poison_death( ch->level, victim ) )
    {
	AFFECT_DATA af;

	af.type      = gsn_poison;
	af.duration  = 20;
	af.location  = APPLY_STR;
	af.modifier  = -2;
	af.bitvector = meb(AFF_POISON);
	affect_join( victim, &af );
	ch->mental_state = URANGE( 20, ch->mental_state + 2, 100 );
    }

    /*
     * Vampire self preservation				-Thoric
     */
    if ( IS_VAMPIRE(victim) )
    {
      if ( dam >= (victim->max_hit / 10) )	/* get hit hard, lose blood */
	gain_condition(victim, COND_BLOODTHIRST, -1 - (victim->level / 20));
      if ( victim->hit <= (victim->max_hit / 8)
      && victim->pcdata->condition[COND_BLOODTHIRST]>5 )
      {
	gain_condition(victim, COND_BLOODTHIRST,
		-URANGE(3, victim->level / 10, 8) );
	victim->hit += URANGE( 4, (victim->max_hit / 30), 15);
	set_char_color(AT_BLOOD, victim);
	send_to_char("You howl with rage as the beast within stirs!\n\r", victim);
      }
    }

    if ( !npcvict
    &&   get_trust(victim) >= LEVEL_IMMORTAL
    &&	 get_trust(ch)	   >= LEVEL_IMMORTAL
    &&   victim->hit < 1 )
	victim->hit = 1;
    update_pos( victim );

    switch( victim->position )
    {
    case POS_MORTAL:
	act( AT_DYING, "$n is mortally wounded, and will die soon, if not aided.",
	    victim, NULL, NULL, TO_ROOM );
	act( AT_DANGER, "You are mortally wounded, and will die soon, if not aided.",
	victim, NULL, NULL, TO_CHAR );
	break;

    case POS_INCAP:
	act( AT_DYING, "$n is incapacitated and will slowly die, if not aided.",
	    victim, NULL, NULL, TO_ROOM );
	act( AT_DANGER, "You are incapacitated and will slowly die, if not aided.",
	victim, NULL, NULL, TO_CHAR );
	break;

    case POS_STUNNED:
        if ( !IS_AFFECTED( victim, AFF_PARALYSIS ) )
        {
	  act( AT_ACTION, "$n is stunned, but will probably recover.",
	    victim, NULL, NULL, TO_ROOM );
	  act( AT_HURT, "You are stunned, but will probably recover.",
	    victim, NULL, NULL, TO_CHAR );
	}
	break;

    case POS_DEAD:
	if ( dt >= 0 && dt < top_sn )
	{
	    SKILLTYPE *skill = skill_table[dt];

	    if ( skill->die_char && skill->die_char[0] != '\0' )
	      act( AT_DEAD, skill->die_char, ch, NULL, victim, TO_CHAR );
	    if ( skill->die_vict && skill->die_vict[0] != '\0' )
	      act( AT_DEAD, skill->die_vict, ch, NULL, victim, TO_VICT );
	    if ( skill->die_room && skill->die_room[0] != '\0' )
	      act( AT_DEAD, skill->die_room, ch, NULL, victim, TO_NOTVICT );
	}
        stop_fighting(victim,TRUE);
	   if ( !IS_NPC(ch) && IS_SET( ch->pcdata->flags, PCFLAG_OLD_DEATH ) )
    		grim_reaper(victim,TRUE);
   		else
    		grim_reaper(victim,FALSE);

/*        if ( !victim->in_room )
	{
	   if ( victim->in_room )
	     char_from_room(victim);

	   char_to_room(victim,victim->was_in_room);
	}

        if ( !ch->in_room )
	{
	   if ( ch->in_room )
	     char_from_room(ch);

	   char_to_room(ch,ch->was_in_room);
	}
*/
	break;

    default:
	if ( dam > victim->max_hit / 4 )
	{
	   act( AT_HURT, "That really did HURT!", victim, 0, 0, TO_CHAR );
	   if ( number_bits(3) == 0 )
		worsen_mental_state( ch, 1 );
	}
	if ( victim->hit < victim->max_hit / 4 )

	{
	   act( AT_DANGER, "You wish that your wounds would stop BLEEDING so much!",
		victim, 0, 0, TO_CHAR );
	   if ( number_bits(2) == 0 )
		worsen_mental_state( ch, 1 );
	}
	break;
    }

    /*
     * Sleep spells and extremely wounded folks.
     */
    /*if ( *//*!IS_AWAKE(victim) Nah let them die! --GW */		/*letsmake NPC's not slaughter PC's */
    /*&&   !IS_AFFECTED( victim, AFF_PARALYSIS ) )
    {*/
/*	if ( victim->fighting
	&&   victim->fighting->who->hunting
	&&   victim->fighting->who->hunting->who == victim )
	   stop_hunting( victim->fighting->who );

	if ( victim->fighting
	&&   victim->fighting->who->hating
	&&   victim->fighting->who->hating->who == victim )
	   stop_hating( victim->fighting->who );

	if (!npcvict && IS_NPC(ch))
	  stop_fighting( victim, TRUE );
	else
	  stop_fighting( victim, FALSE );*/
/*    }*/

    /* Added this cause the mobile seems to still be fightin the char and vice
       versa after the attack is done, for ranged attacks  --GW */
    if ( ch->in_room != victim->in_room )
    stop_fighting(ch,TRUE);
 
    /*
     * Payoff for killing things.
     * Was to much crap here, in death.c now --GW
     */
     if ( victim->position == POS_DEAD )
     {
	group_gain( ch, victim );

	/*
	 * Check to see if thats there Quest Mob! --GW
         */
	quest_death_check(ch,victim);

	/* 
         * Fix up the killers ego --GW
         */
	ego_update(ch,victim, TRUE);


 
        if ( !npcvict )
        {

        if ( ( death_handler( ch, victim, npcvict ))==TRUE )
	{
        return TRUE;
	}
     
   	    /*
	     * Dying penalty:
	     * 1/2 way back to previous level.
	     */
	    if ( victim->exp > exp_level(victim, victim->level) )
	     victim->exp -= (victim->exp/2);
            do_help(victim,"_DIEMSG_");

	    /*
	     * New penalty... go back to the beginning of current level.
	     victim->exp = exp_level( victim, victim->level );
	     */
      }
	else
	if ( !IS_NPC(ch) )		/* keep track of mob vnum killed */
	    add_kill( ch, victim );

	loot = legal_loot( ch, victim );

	set_cur_char(victim);
	raw_kill( ch, victim );
	victim = NULL;

	if ( !IS_NPC(ch) && loot )
	{
	   /* Autogold by Scryn 8/12 */
	    if ( IS_SET(ch->act, PLR_AUTOGOLD) && npcvict )
	    {
		init_gold = ch->gold;
		do_get( ch, "coins corpse" );
		do_get( ch, "pieces corpse" );
		new_gold = ch->gold;
		gold_diff = (new_gold - init_gold);
		if (gold_diff > 0)
                {
                  sprintf(buf1,"%d",gold_diff);
		  do_split( ch, buf1 );
		} 
	    }
	    if ( IS_SET(ch->act, PLR_AUTOLOOT) && npcvict )
		do_get( ch, "all corpse" );
	    else
		do_look( ch, "in corpse" );


	    if ( IS_SET(ch->act, PLR_AUTOSAC) && npcvict )
	    {
		      if ( ms_find_obj(ch) )
        		return rVICT_DIED;
	   if ( ch->in_room )
	   {
	    obj = get_obj_list_rev( ch, "corpse", ch->in_room->last_content );
	    
    	       if ( !obj )
               {
                  return rVICT_DIED;
               }
		if ( obj->item_type == ITEM_CORPSE_NPC ) 
		do_sacrifice( ch, "corpse" );
	    }
	    }
	   }	

    	if ( IS_SET( sysdata.save_flags, SV_KILL ) )
	   save_char_obj( ch );
		return rVICT_DIED;
    }

    if ( victim == ch )
	return rNONE;

    /*
     * Take care of link dead people.
     */
    if ( !npcvict && !victim->desc  
    && !IS_SET( victim->pcdata->flags, PCFLAG_NORECALL ) )
    {
	if ( number_range( 0, victim->wait ) == 0)
	{
	    do_recall( victim, "" );
	    return rNONE;
	}
    }

    /*
     * Wimp out?
     */
    if ( npcvict && dam > 0 )
    {
	if ( ( IS_SET(victim->act, ACT_WIMPY) && number_bits( 1 ) == 0
	&&   victim->hit < victim->max_hit / 2 )
	||   ( IS_AFFECTED(victim, AFF_CHARM) && victim->master
	&&     victim->master->in_room != victim->in_room ) )
	{
	    start_fearing( victim, ch );
	    stop_hunting( victim );
	    do_flee( victim, "" );
	}
    }

    if ( !npcvict
    &&   victim->hit > 0
    &&   victim->hit <= victim->wimpy
    &&   victim->wait == 0 )
	do_flee( victim, "" );
    else
    if ( !npcvict && IS_SET( victim->act, PLR_FLEE ) )
	do_flee( victim, "" );

    tail_chain( );
    return rNONE;
}



bool is_safe( CHAR_DATA *ch, CHAR_DATA *victim )
{
//    int victclan;
//    int chclan;

    /* safety checks -- GW */
    if ( ch == NULL )
	return FALSE;

    if ( victim == NULL )
	return FALSE;	

    /* Thx Josh! */
    if ( who_fighting( ch ) == ch )
	return FALSE;

/* ARENA room checking */
        if( in_arena( ch )
        && in_arena(victim) )
        return FALSE;

/*CHAOS On? */	
	if( chaos == 1)
	return FALSE;

/* In a Club Maze? */
	if ( ch->in_room && ch->in_room->area &&
		IS_SET( ch->in_room->area->flags, AFLAG_CLUB_HALL ) )
	return FALSE;

/* in Kombat? */
	if( !IS_NPC(ch) && IS_SET(ch->pcdata->flags,PCFLAG_KOMBAT) &&
	  ( ch->in_room->vnum != 21440 ))
	return FALSE;

/* in CTF? */
	if( !IS_NPC(ch) && IS_SET(ch->pcdata->flagstwo,MOREPC_CTF) &&
	  ( ch->in_room->vnum != 21440 ))
	return FALSE;

/* Dragon? */
	if( !IS_NPC(ch) && !IS_NPC(victim) && victim->advclass == 14 )
	return FALSE;

//    victclan = victim->pcdata->clan;
//    chclan = ch->pcdata->clan;

//	not no .. but FUCK NO! leave it alone. --GW
//    if ( victim->pcdata->clan != ch->pcdata->clan )
//      return FALSE;


    if ( !victim->in_room )
     char_to_room(victim, ch->in_room);

    if ( victim && victim->in_room )
    {
    if ( IS_SET( victim->in_room->room_flags, ROOM_SAFE ) )
    {
	set_char_color( AT_MAGIC, ch );
	send_to_char( "A magical force prevents you from attacking.\n\r", ch );
	return TRUE;
    }
    }

    if ( ch->level > LEVEL_IMMORTAL )
        return FALSE;

    

    if( !IS_NPC( ch ) && !IS_NPC( victim )
       && ch != victim 
       && IS_SET( victim->in_room->area->flags, AFLAG_NOPKILL ) ) 
    {
        set_char_color( AT_IMMORT, ch );
        send_to_char( "The gods have forbidden player killing in this area.\n\r", ch );
        return TRUE;
    }
 
    if ( IS_NPC(ch) || IS_NPC(victim) )
	return FALSE;

	if (!IS_NPC(ch)) 
    {
	if( !IS_NPC(ch) && IS_SET(ch->pcdata->flags, PCFLAG_ASSASSIN))
	return FALSE;

	if( !IS_NPC(ch) && IS_SET(victim->pcdata->flags, PCFLAG_ASSASSIN))
	return FALSE;
	}

    if ( ch->level < 5 )
    {
	set_char_color( AT_WHITE, ch );	
	send_to_char( "You are not yet ready, needing age or experience, if not both. \n\r", ch );
	return TRUE;
    }

    if ( victim->level < 5 )
    {
	set_char_color( AT_WHITE, ch );
	send_to_char( "They are yet too young to die.\n\r", ch );
	return TRUE;
    }

    if ( get_timer(victim, TIMER_PKILLED) > 0 )
    {
	set_char_color( AT_GREEN, ch );
        send_to_char( "That character has died within the last 5 minutes.\n\r", ch);
        return TRUE;
    }
  
    if ( get_timer(ch, TIMER_PKILLED) > 0 )
    {
	set_char_color( AT_GREEN, ch );
        send_to_char( "You have been killed within the last 5 minutes.\n\r", ch );
        return TRUE;
    }

    return FALSE;
}

/*
 * just verify that a corpse looting is legal
 */
bool legal_loot( CHAR_DATA *ch, CHAR_DATA *victim )
{
    /* anyone can loot mobs */
    if ( IS_NPC(victim) )
      return TRUE;
    /* non-charmed mobs can loot anything */
    if ( IS_NPC(ch) && !ch->master )
      return TRUE;
    /* members of different clans can loot too! -Thoric */
    if ( !IS_NPC(ch) && !IS_NPC(victim)
    &&    IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) 
    &&    IS_SET( victim->pcdata->flags, PCFLAG_DEADLY ) ) 
	return TRUE;
    return FALSE;
}

/*
 * See if an attack justifies a KILLER flag.
 */
void check_killer( CHAR_DATA *ch, CHAR_DATA *victim )
{
    /*
     * NPC's are fair game.
     */
    if ( IS_NPC(victim) )
    {
	if ( !IS_NPC( ch ) )
	{
	  int level_ratio;
	  level_ratio = URANGE( 1, ch->level / victim->level, 50);
	  if ( ch->pcdata->clan )
	    ch->pcdata->clan->mkills++;
	  ch->pcdata->mkills++;
	  ch->in_room->area->mkills++;
	  if ( ch->pcdata->deity ) 
	  {
	    if ( victim->race == ch->pcdata->deity->npcrace )
	      adjust_favor( ch, 3, level_ratio );
	    else
	      if ( victim->race == ch->pcdata->deity->npcfoe )
		adjust_favor( ch, 17, level_ratio );
	      else
                adjust_favor( ch, 2, level_ratio );
	  }
	}
	return;
    }


    /*
     * Any character in the arena is ok to kill.
     */

    if ( in_arena( ch ) )
      return;

     /*
      * Chaos is on, kill away:)
      */

     if ( chaos == 1 )
      return;

     /*
      * In a club HALL or MAZE?
      */
     if ( ch->in_room && ch->in_room->area &&
		IS_SET( ch->in_room->area->flags, AFLAG_CLUB_HALL ) )
     return;


     /*
      * Mortal Kombat
      */
     if ( ( !IS_NPC(ch) && !IS_NPC(victim) ) &&
	    IS_SET( ch->pcdata->flags, PCFLAG_KOMBAT ) )
       return;

     /* CTF */
     if ( ( !IS_NPC(ch) && !IS_NPC(victim) ) &&
	    IS_SET( ch->pcdata->flagstwo, MOREPC_CTF ) )
       return;


     /*
      * Dragons
      */
     if ( !IS_NPC(ch) && !IS_NPC(victim) )
     {
      if ( IS_ADVANCED(ch) && IS_ADVANCED(victim) )
      {
	if ( ch->advclass == 14 && victim->advclass == 14 )
	{
	   if( ch->advlevel <= (victim->advlevel+10) )
   	     return;
	}
      }
     }
    /*
     * So are killers and thieves.
     */
    if ( IS_SET(victim->act, PLR_KILLER)
    || IS_SET(victim->pcdata->flags, PCFLAG_ASSASSIN)
    || IS_SET(victim->act, PLR_THIEF) )
    {
	if ( !IS_NPC( ch ) )
	{
	  if ( ch->pcdata->clan )
	    ch->pcdata->clan->pkills++;
	  ch->in_room->area->pkills++;
	}
	return;
    }

    if ( ( ch->level > (victim->level+10) ) || 
	   !IS_SET( victim->pcdata->flags, PCFLAG_DEADLY)
	|| (IS_ADVANCED(ch) && !IS_ADVANCED(victim)) )
    {
    set_char_color( AT_WHITE, ch );
    send_to_char( "A strange feeling grows deep inside you, and a tingle goes up your spine...\n\r", ch );
    set_char_color( AT_IMMORT, ch );
    send_to_char( "A deep voice booms inside your head, 'Thou shall now be known as a deadly murderer!!!'\n\r", ch );
    set_char_color( AT_WHITE, ch );
    send_to_char( "You feel as if your soul has been revealed for all to see.\n\r", ch );
    SET_BIT(ch->act, PLR_KILLER);
    if ( IS_SET( ch->act, PLR_ATTACKER) )
      REMOVE_BIT(ch->act, PLR_ATTACKER);
    save_char_obj( ch );
    return;
    }

    /* clan checks					-Thoric */
    if ( ( !IS_NPC(ch) && !IS_NPC(victim) )
    &&    (IS_SET( ch->pcdata->flags, PCFLAG_DEADLY )
    &&    IS_SET( victim->pcdata->flags, PCFLAG_DEADLY ))
    &&    ( ( ( IS_ADVANCED(ch) && IS_ADVANCED(victim))
    &&	  ( ch->advlevel <= (victim->advlevel+10) ) )
    ||    ( ( !IS_ADVANCED(ch) && !IS_ADVANCED(victim) )
    &&	  ( ch->level <= (victim->advlevel+10) ) ) ) )
    {
      
      /* not of same clan? Go ahead and kill!!! */
      if   (!IS_NPC(ch) && ( ch->pcdata->clan && victim->pcdata->clan)
      &&   (ch->pcdata->clan->clan_type != CLAN_NOKILL
      &&   victim->pcdata->clan->clan_type != CLAN_NOKILL)
      &&   (ch->pcdata->clan != victim->pcdata->clan ) ) 

      {
	if ( ch->pcdata->clan ) ch->pcdata->clan->pkills++;
	  ch->hit = ch->max_hit;
	  ch->mana = ch->max_mana;
	  ch->move = ch->max_move;
	if ( ch->pcdata )
	  ch->pcdata->condition[COND_BLOODTHIRST] = get_bloodthirst(ch);
	update_pos(victim);
	if ( victim != ch )
	{
	  act( AT_MAGIC, "Bolts of blue energy rise from the corpse, seeping into $n.", ch, victim->name, NULL, TO_ROOM );
	  act( AT_MAGIC, "Bolts of blue energy rise from the corpse, seeping into you.", ch, victim->name, NULL, TO_CHAR ); 
	}
	if ( victim->pcdata->clan )
	  victim->pcdata->clan->pdeaths++;
	victim->pcdata->pdeaths++;   
	adjust_favor( victim, 11, 1 );
	adjust_favor( ch, 2, 1 );
	add_timer( victim, TIMER_PKILLED, 115, NULL, 0 );
	WAIT_STATE( victim, 3 * PULSE_VIOLENCE );
	/* SET_BIT(victim->act, PLR_PK); */
	return; 
      }
	return;
    }


    /*
     * Charm-o-rama.
     */
    if ( IS_AFFECTED(ch, AFF_CHARM) )
    {
	if ( !ch->master )
	{
	    char buf[MAX_STRING_LENGTH];

	    sprintf( buf, "Check_killer: %s bad AFF_CHARM",
		IS_NPC(ch) ? ch->short_descr : ch->name );
	    bug( buf, 0 );
	    affect_strip( ch, gsn_charm_person );
	    xREMOVE_BIT( ch->affected_by, AFF_CHARM );
	    return;
	}

	/* stop_follower( ch ); */
//	if ( ch->master )
//	  check_killer( ch->master, victim );
	return;
    }

    /*
     * NPC's are cool of course (as long as not charmed).
     * Hitting yourself is cool too (bleeding).
     * So is being immortal (Alander's idea).
     * And current killers stay as they are.
     */
    if ( IS_NPC(ch) )
    {
      if ( !IS_NPC(victim) )
      {
	int level_ratio;
        if ( victim->pcdata->clan )
          victim->pcdata->clan->mdeaths++;
        victim->pcdata->mdeaths++;
	victim->in_room->area->mdeaths++;
	level_ratio = URANGE( 1, ch->level / victim->level, 50 );
	if ( victim->pcdata->deity )
	{
	  if ( ch->race == victim->pcdata->deity->npcrace )
	    adjust_favor( victim, 12, level_ratio );
	  else
	    if ( ch->race == victim->pcdata->deity->npcfoe )
		adjust_favor( victim, 15, level_ratio );
	    else
	        adjust_favor( victim, 11, level_ratio );
	}
      }
      return;
    }

    if ( ch == victim || ch->level >= LEVEL_IMMORTAL )
      return;
      
    if ( !IS_NPC(ch) )
    {
      if ( ch->pcdata->clan )
        ch->pcdata->clan->illegal_pk++;
      ch->pcdata->illegal_pk++;
      ch->in_room->area->illegal_pk++;
    }
    if ( !IS_NPC(victim) )
    {
      if ( victim->pcdata->clan )
        victim->pcdata->clan->pdeaths++;
      victim->pcdata->pdeaths++;
      victim->in_room->area->pdeaths++;
    }

    if ( IS_SET(ch->act, PLR_KILLER) )
      return;

    set_char_color( AT_WHITE, ch );
    send_to_char( "A strange feeling grows deep inside you, and a tingle goes up your spine...\n\r", ch );
    set_char_color( AT_IMMORT, ch );
    send_to_char( "A deep voice booms inside your head, 'Thou shall now be known as a deadly murderer!!!'\n\r", ch );
    set_char_color( AT_WHITE, ch );
    send_to_char( "You feel as if your soul has been revealed for all to see.\n\r", ch );
    SET_BIT(ch->act, PLR_KILLER);
    if ( IS_SET( ch->act, PLR_ATTACKER) )
      REMOVE_BIT(ch->act, PLR_ATTACKER);
    save_char_obj( ch );
    return;
}

/*
 * See if an attack justifies a ATTACKER flag.
 */
void check_attacker( CHAR_DATA *ch, CHAR_DATA *victim )
{

/* 
 * Made some changes to this function Apr 6/96 to reduce the prolifiration
 * of attacker flags in the realms. -Narn
 */
    /*
     * NPC's are fair game.
     * So are killers and thieves.
     */
    if ( IS_NPC(victim)
    ||   IS_SET(victim->act, PLR_KILLER)
    ||   IS_SET(victim->act, PLR_THIEF) )
	return;

    /* deadly char check */
    if ( !IS_NPC(ch) && !IS_NPC(victim)
         && CAN_PKILL( ch ) && CAN_PKILL( victim ) ) 
    {
      if ( !(ch->pcdata->clan && victim->pcdata->clan 
           && ch->pcdata->clan == victim->pcdata->clan ) )
        return;
    }

    /*
     * Charm-o-rama.
     */
    if ( IS_AFFECTED(ch, AFF_CHARM) )
    {
	if ( !ch->master )
	{
	    char buf[MAX_STRING_LENGTH];

	    sprintf( buf, "Check_attacker: %s bad AFF_CHARM",
		IS_NPC(ch) ? ch->short_descr : ch->name );
	    bug( buf, 0 );
	    affect_strip( ch, gsn_charm_person );
	    xREMOVE_BIT( ch->affected_by, AFF_CHARM );
	    return;
	}

        /* Won't have charmed mobs fighting give the master an attacker 
           flag.  The killer flag stays in, and I'll put something in 
           do_murder. -Narn */
	/*SET_BIT(ch->master->act, PLR_ATTACKER);*/
	/* stop_follower( ch ); */
	return;
    }

    /*
     * NPC's are cool of course (as long as not charmed).
     * Hitting yourself is cool too (bleeding).
     * So is being immortal (Alander's idea).
     * And current killers stay as they are.
     */
    if ( IS_NPC(ch)
    ||   ch == victim
    ||   ch->level >= LEVEL_IMMORTAL
    ||   IS_SET(ch->act, PLR_ATTACKER)
    ||   IS_SET(ch->act, PLR_KILLER) )
	return;

    SET_BIT(ch->act, PLR_ATTACKER);
    save_char_obj( ch );
    return;
}


/*
 * Set position of a victim.
 */
void update_pos( CHAR_DATA *victim )
{
    if ( !victim )
    {
      bug( "update_pos: null victim", 0 );
      return;
    }

    if ( victim->hit > 0 )
    {
	if ( victim->position <= POS_STUNNED )
	  victim->position = POS_STANDING;
	if ( (IS_AFFECTED( victim, AFF_PARALYSIS )))
	  victim->position = POS_STUNNED;
	return;
    }

    if ( IS_NPC(victim) || victim->hit <= -11 )
    {
	if ( victim->mount )
	{
	  act( AT_ACTION, "$n falls from $N.",
		victim, NULL, victim->mount, TO_ROOM );
	  REMOVE_BIT( victim->mount->act, ACT_MOUNTED );
	  victim->mount = NULL;
	}
	victim->position = POS_DEAD;
	return;
    }

	 if ( victim->hit <= -6 ) victim->position = POS_MORTAL;
    else if ( victim->hit <= -3 ) victim->position = POS_INCAP;
    else                          victim->position = POS_STUNNED;

    if (( victim->position > POS_STUNNED
    && (  IS_AFFECTED( victim, AFF_PARALYSIS) ) ) )
      victim->position = POS_STUNNED;

    if ( victim->mount )
    {
	act( AT_ACTION, "$n falls unconscious from $N.",
		victim, NULL, victim->mount, TO_ROOM );
	REMOVE_BIT( victim->mount->act, ACT_MOUNTED );
	victim->mount = NULL;
    }
    return;
}


/*
 * Start fights.
 */
void set_fighting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    FIGHT_DATA *fight=NULL;

    if ( ch->fighting )
    {
	char buf[MAX_STRING_LENGTH];

	sprintf( buf, "Set_fighting: %s -> %s (already fighting %s)",
		ch->name, victim->name, ch->fighting->who->name );
	bug( buf, 0 );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_SLEEP) )
      affect_strip( ch, gsn_sleep );

    /* Limit attackers -Thoric */
    if ( victim->num_fighting > max_fight(victim) )
    {
	send_to_char( "There are too many people fighting for you to join in.\n\r", ch );
	return;
    }

    CREATE( fight, FIGHT_DATA, 1 );
    fight->who	 = victim;
    fight->xp	 = (int) xp_compute( ch, victim ) * 0.85;
    fight->align = align_compute( ch, victim );
    if ( !IS_NPC(ch) && IS_NPC(victim) )
      fight->timeskilled = times_killed(ch, victim);
    ch->num_fighting = 1;
    ch->fighting = fight;
    ch->position = POS_FIGHTING;
    victim->num_fighting++;
/*    if ( victim->switched && IS_AFFECTED(victim->switched, AFF_POSSESS))
    {
	send_to_char( "You are disturbed!\n\r", victim->switched );
	do_return( victim->switched, "" );
    }*/
    return;
}


CHAR_DATA *who_fighting( CHAR_DATA *ch )
{
char bug_buf[MAX_STRING_LENGTH];

    if ( !ch )
    {
	bug( "who_fighting: null ch", 0 );
	return NULL;
    }

    if ( !ch->fighting )
      return NULL;

    if ( !ch->fighting->who )
	{
	sprintf(bug_buf,"who_fighting: NULL ch->fighting->who (%s)",
		ch->name );
	return NULL;
	}

    return ch->fighting->who;
}

void free_fight( CHAR_DATA *ch )
{
   if ( !ch )
   {
	bug( "Free_fight: null ch!", 0 );
	return;
   }
   if ( ch->fighting )
   {
     if ( !char_died(ch->fighting->who) )
       --ch->fighting->who->num_fighting;
     DISPOSE( ch->fighting );
   }
   ch->fighting = NULL;
   if ( ch->mount )
     ch->position = POS_MOUNTED;
   else
     ch->position = POS_STANDING;
   /* Berserk wears off after combat. -- Altrag */
   if ( IS_AFFECTED(ch, AFF_BERSERK) )
   {
     affect_strip(ch, gsn_berserk);
     set_char_color(AT_WEAROFF, ch);
     send_to_char(skill_table[gsn_berserk]->msg_off, ch);
     send_to_char("\n\r", ch);
   }
   return;
}


/*
 * Stop fights.
 */
void stop_fighting( CHAR_DATA *ch, bool fBoth )
{
    CHAR_DATA *fch;
    ZONE_DATA *in_zone;

    if( !ch )
	return;

    free_fight( ch );
    update_pos( ch );

    if ( !fBoth )   /* major short cut here by Thoric */
      return;

    for ( fch = first_char; fch; fch = fch->next )
    {
	if ( who_fighting( fch ) == ch )
	{
	    free_fight( fch );
	    update_pos( fch );
	}
    }
if ( ch->in_room && ch->in_room->area && ch->in_room->area->zone )
in_zone=find_zone(ch->in_room->area->zone->number);
else
in_zone = find_zone(1);

    for ( fch = in_zone->first_mob; fch; fch = fch->next )
    {
	if ( who_fighting( fch ) == ch )
	{
	    free_fight( fch );
	    update_pos( fch );
	}
    }

    return;
}

void group_gain( CHAR_DATA *ch, CHAR_DATA *victim )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *gch;
    CHAR_DATA *lch;
    long xp;
    int members;

    /*
     * Monsters don't get kill xp's or alignment changes.
     * Dying of mortal wounds or poison doesn't give xp to anyone!
     */
    if ( !ch )
     return;

    if ( !victim) 
     return;

    if ( IS_NPC(ch) || victim == ch || war == 1 || (IS_NPC(ch) && IS_SET(ch->pcdata->flags, PCFLAG_KOMBAT) ) )
	return;

    members = 0;
    for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
    {
	if ( gch && ch && is_same_group( gch, ch ) )
	    members++;
    }

    if ( members == 0 )
    {
	bug( "Group_gain: members.", members );
	members = 1;
    }

    lch = ch->leader ? ch->leader : ch;

    for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
    {
/*	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
-- used for zap .. no longer used --GW
*/
	if ( !is_same_group( gch, ch ) )
	    continue;

	if ( gch->level - lch->level >  10 )
	{
	    send_to_char( "You are too high for this group.\n\r", gch );
	    continue;
	}

	if ( gch->level - lch->level < -10 )
	{
	    send_to_char( "You are too low for this group.\n\r", gch );
	    continue;
	}

	xp = (int) (xp_compute( gch, victim ) * 0.3 ) / members;
	if ( !gch->fighting )
	  xp /= 2;

	if ( IS_NPC(victim) && IS_SET(victim->acttwo, ACT_CREATE_LIFE_MOB))
	xp = 0;

	gch->alignment = align_compute( gch, victim );
	buf[0] = '\0';
	sprintf( buf, "You receive %s experience points for the Kill.\n\r", num_comma(xp) );
	send_to_char( buf, gch );

      /* New 'statistical' display of EXP Gain per kill --GW */
      if ( !IS_NPC(ch) )
      {
        sprintf( buf, "You received a total of %s experiance points for Damage Inflicted.\n\r",num_comma(ch->pcdata->battle_xp));
	send_to_char( buf, gch );
	sprintf( buf, "Total Experiance Gained: %s\n\r",num_comma(ch->pcdata->battle_xp + xp));
        send_to_char( buf, gch );
	ch->pcdata->battle_xp = 0;
       }
	buf[0] = '\0';
	gain_exp( gch, xp, TRUE );

/* this sucks .. remove --GW */
/*	for ( obj = ch->first_carrying; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( obj->wear_loc == WEAR_NONE )
		continue;

	    if ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)    )
	    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)    )
	    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch) ) )
	    {
		act( AT_MAGIC, "You are zapped by $p.", ch, obj, NULL, TO_CHAR );
		act( AT_MAGIC, "$n is zapped by $p.",   ch, obj, NULL, TO_ROOM );
	        
                remove_obj ( ch, obj->wear_loc, TRUE );
		oprog_zap_trigger(ch, obj);  
		if ( char_died(ch) )
		  return;
	    }
	}
*/
   if (!IS_NPC(gch) && IS_SET(gch->pcdata->flags,PCFLAG_QUESTING )
        && IS_NPC(victim))
        {
            if (gch->pcdata->questmob == victim->pIndexData->vnum)
            {
               send_to_char("You have almost completed your quest!\n\r",gch);
        send_to_char(
        "Return to the questmaster before your time runs of time.\n\r",gch);
                ch->pcdata->questmob = -1;
            }
        }


    }

    return;
}


int align_compute( CHAR_DATA *gch, CHAR_DATA *victim )
{
    int align, newalign;

    align = gch->alignment - victim->alignment;

    if ( align >  500 )
	newalign  = UMIN( gch->alignment + (align-500)/4,  1000 );
    else
    if ( align < -500 )
	newalign  = UMAX( gch->alignment + (align+500)/4, -1000 );
    else
	newalign  = gch->alignment - (int) (gch->alignment / 4);

    return newalign;
}


/*
 * Calculate how much XP gch should gain for killing victim
 * Lots of redesigning for new exp system by Thoric
 */
int xp_compute( CHAR_DATA *gch, CHAR_DATA *victim )
{
    int align;
    int xp;
    int xp_ratio;
    int gchlev = gch->level;

    xp	  = (get_exp_worth( victim )
    	  *  URANGE( 0, (victim->level - gchlev) + 25, 13 )) / 5.9;

	if ( doubleexp == 1)
	xp = xp*2;

    align = gch->alignment - victim->alignment;

    /* bonus for attacking opposite alignment */
    if ( align >  990 || align < -990 )
	xp = (xp*5) >> 2;
    else
    /* penalty for good attacking same alignment */
    if ( gch->alignment > 300 && align < 250 )
	xp = (xp*3) >> 2;

    xp = number_range( (xp*3) >> 2, (xp*5) >> 2 );

    /* get 1/4 exp for players					-Thoric */
    if ( !IS_NPC( victim ) )
      xp /= 4;
    else
    /* reduce exp for killing the same mob repeatedly		-Thoric */
    if ( !IS_NPC( gch ) )
    {
	int times = times_killed( gch, victim );

	if ( times >= 20 )
	   xp = 0;
	else
	if ( times )
	{
	   xp = (xp * (20-times)) / 20;
	   if ( times > 15 )
	     xp /= 3;
	   else
	   if ( times > 10 )
	     xp >>= 1;
	}

    }

    /*
     * semi-intelligent experienced player vs. novice player xp gain
     * "bell curve"ish xp mod by Thoric
     * based on time played vs. level
     */
    if ( !IS_NPC( gch ) && gchlev > 5 )
    {
       xp_ratio = (int) gch->pcdata->played / gchlev;
       if ( xp_ratio > 20000 )
         xp = (xp*5) >> 2;
       else
       if ( xp_ratio < 16000 )
         xp = (xp*3) >> 2;
       else
       if ( xp_ratio < 10000 )
         xp >>= 1;
       else
       if ( xp_ratio < 5000 )
         xp >>= 2;
       else
       if ( xp_ratio < 3500 )
         xp >>= 3;
       else
       if ( xp_ratio < 2000 )
         xp >>= 4;
    }

    /*
     * Level based experience gain cap.  Cannot get more experience for
     * a kill than the amount for your current experience level   -Thoric
     */

    /* NO Exp for Ghosts!!!!!! --GW */
    if ( IS_AFFECTED( victim, AFF_GHOST ) )
    xp = 0;

    return URANGE(0, xp, exp_level( gch, gchlev ));
}


/*
 * Revamped by Thoric to be more realistic
 */
void dam_message( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt )
{
    char buf1[256], buf2[256], buf3[256];
    char *vs;
    char *vp;
    char *attack;
    char punct;
    sh_int dampc;
    struct skill_type *skill = NULL;
    bool gcflag = FALSE;
    bool gvflag = FALSE; 
    extern sh_int dam_no_msg;

    if ( !ch || !victim )
	return;

    vs = NULL;
    vp = NULL;
    attack = NULL;

    if ( dam_no_msg == 1 )
      return;    

    if ( ! dam )
      dampc = 0;
    else
      dampc = URANGE( 0,( (dam * 1000) / victim->max_hit) +
              ( 50 - ((victim->hit * 50) / victim->max_hit) ),32000);

	 if ( dampc  <	  0 ) { vs = "HELP"; vp = "HELPS"; }
    else if ( dam   ==    0 ) { vs = "miss";	vp = "misses";}
    else if ( dampc <=    5 ) { vs = "barely hit";vp = "barely hits";}
    else if ( dampc <=   10 ) { vs = "scratch";	vp = "scratches";	}
    else if ( dampc <=   20 ) { vs = "scrape";	vp = "scrapes";		}
    else if ( dampc <=   30 ) { vs = "graze";	vp = "grazes";		}
    else if ( dampc <=   40 ) { vs = "bruise";	vp = "bruises";		}
    else if ( dampc <=   50 ) { vs = "hit";	vp = "hits";		}
    else if ( dampc <=   60 ) { vs = "hurt";	vp = "hurts";		}
    else if ( dampc <=   75 ) { vs = "wound";	vp = "wounds";	}
    else if ( dampc <=   80 ) { vs = "lacerate";	vp = "lacerates";		}
    else if ( dampc <=   90 ) { vs = "harm";    vp = "harms";		}
    else if ( dampc <=  125 ) { vs = "ravage";	vp = "ravages";	}
    else if ( dampc <=  150 ) { vs = "demolish"; vp = "demolishes";	}
    else if ( dampc <=  200 ) { vs = "cripple";	vp = "cripples";		}
    else if ( dampc <=  300 ) { vs = "MANGLE";vp = "MANGLES";	}
    else if ( dampc <=  400 ) { vs = "DISMEMBER";vp = "DISMEMBERS";	}
    else if ( dampc <=  500 ) { vs = "== SLAUGHTER ==";  vp = "== SLAUGHTERS ==";	}
    else if ( dampc <=  600 ) { vs = "**  LIQUEFY **"; vp = "** LIQUEFIES **";	}
    else if ( dampc <=  750 ) { vs = "|| DISINTEGRATE ||";vp = "|| DISINTEGRATES ||";	}
    else if ( dampc <=  990 ) { vs = "<== TERMINATE ==>";
			        vp = "<== TERMINATES ==>";			}
    else                      { vs = "do *** TERRIBLE THINGS *** to";
			        vp = "does *** TERRIBLE THINGS *** to";		}

    punct   = '\0';

    punct   = (dampc <= 30) ? '.' : '!';

    if ( dam < 0 )
	dam = 0;

    if ( dam == 0 && (!IS_NPC(ch) && 
       (IS_SET(ch->pcdata->flags, PCFLAG_GAG)))) gcflag = TRUE;

    if ( dam == 0 && (!IS_NPC(victim) &&
       (IS_SET(victim->pcdata->flags, PCFLAG_GAG)))) gvflag = TRUE;

    if ( dt >=0 && dt < top_sn )
	skill = skill_table[dt];
    if ( dt == TYPE_HIT )
    {
        buf1[0] = '\0';
        buf2[0] = '\0';
        buf3[0] = '\0';

	sprintf( buf1, "&G$n &R%s&G $N%c&W",  vp, punct );
	sprintf( buf2, "&GYou &R%s&G $N%c&W", vs, punct );
	sprintf( buf3, "&G$n &R%s&G you%c&W", vp, punct );
    }
    else
    if ( dt > TYPE_HIT && is_wielding_poisoned( ch ) )
    {
	if ( dt < TYPE_HIT + sizeof(attack_table)/sizeof(attack_table[0]) )
	    attack	= attack_table[dt - TYPE_HIT];
	else
	{
	    dt  = TYPE_HIT;
	    attack  = attack_table[0];
        }

        buf1[0] = '\0';
        buf3[0] = '\0';
        buf2[0] = '\0';
	sprintf( buf1, "&G$n's &Rpoisoned %s&G %s $N%c&W", attack, vp,punct);
	sprintf( buf2, "&GYour &Rpoisoned %s&G %s $N%c&W", attack, vp,punct);
	sprintf( buf3, "&G$n's &Rpoisoned %s&G %s you%c&W", attack, vp,punct); 
    }
    else
    {
	if ( skill )
	{
	    attack	= skill->noun_damage;
	    if ( dam == 0 )
	    {
		bool found = FALSE;

		if ( skill->miss_char && skill->miss_char[0] != '\0' )
		{
		   act( AT_HIT, skill->miss_char, ch, NULL, victim,TO_CHAR);
		   found = TRUE;
		}
		if ( skill->miss_vict && skill->miss_vict[0] != '\0' )
		{
		   act( AT_HITME, skill->miss_vict, ch, NULL, victim,TO_VICT );
		   found = TRUE;
		}
		if ( skill->miss_room && skill->miss_room[0] != '\0' )
		{
		   act( AT_ACTION, skill->miss_room, ch, NULL, victim, TO_NOTVICT );
		   found = TRUE;
		}
		if ( found )	/* miss message already sent */
		  return;
	    }
	    else
	    {
		if ( skill->hit_char && skill->hit_char[0] != '\0' )
		  act( AT_HIT, skill->hit_char, ch, NULL, victim, TO_CHAR );
		if ( skill->hit_vict && skill->hit_vict[0] != '\0' )
		  act( AT_HITME, skill->hit_vict, ch, NULL, victim, TO_VICT );
		if ( skill->hit_room && skill->hit_room[0] != '\0' )
		  act( AT_ACTION, skill->hit_room, ch, NULL, victim, TO_NOTVICT );
	    }
	}
	else if ( dt >= TYPE_HIT
	&& dt < TYPE_HIT + sizeof(attack_table)/sizeof(attack_table[0]) )
	    attack	= attack_table[dt - TYPE_HIT];
	else
	{
	    dt  = TYPE_HIT;
	    attack  = attack_table[0];
	}

	buf1[0] = '\0';
	buf2[0] = '\0';
	buf3[0] = '\0';


	sprintf( buf1, "&G$n's %s &R%s&G $N%c&W",  attack, vp, punct );
	sprintf( buf2, "&GYour %s &R%s&G $N%c&W",  attack, vp, punct );
	sprintf( buf3, "&G$n's %s &R%s&G you%c&W", attack, vp, punct );
    }

    act( AT_PLAIN, buf1, ch, NULL, victim, TO_NOTVICT );
    if (!gcflag)  act( AT_PLAIN, buf2, ch, NULL, victim, TO_CHAR );
    if (!gvflag) act( AT_PLAIN, buf3, ch, NULL, victim, TO_VICT );

    return;
}


void do_kill( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Kill whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch == NULL )
    return;

    if ( victim == NULL )
    return;

    if ( !IS_NPC(victim) )
    {
	if ( !IS_SET(victim->act, PLR_KILLER)
	&&   !IS_SET(victim->act, PLR_THIEF) )
	{
	    send_to_char( "You must MURDER a player.\n\r", ch );
	    return;
	}

    }

   /*
    *
    else
    {
	if ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL )
	{
	    send_to_char( "You must MURDER a charmed creature.\n\r", ch );
	    return;
	}
    }
    *
    */

    if ( victim == ch )
    {
	send_to_char( "You hit yourself.  Ouch!\n\r", ch );
	multi_hit( ch, ch, TYPE_UNDEFINED );
	return;
    }

    if ( is_safe( ch, victim ) )
	return;

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
    act( AT_PLAIN, "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "You do the best you can!\n\r", ch );
	return;
    }

    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
//    check_killer( ch, victim );
//    check_attacker( ch, victim );
   if ( IS_IMMORTAL(ch) && IS_NPC(victim) && (get_trust(ch) < 56 ) )
   {
     send_to_char("You should not be killing mobiles!\n\r",ch);
     return;
   }

    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}



void do_murde( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to MURDER, spell it out.\n\r", ch );
    return;
}



void do_murder( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Murder whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Suicide is a mortal sin.\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim ) )
	return;

    if ( IS_AFFECTED(ch, AFF_CHARM) ) 
    {
      if ( ch->master == victim )
      {
        act( AT_PLAIN, "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
	return;
      }
      else
      {
        if ( ch->master )
          SET_BIT(ch->master->act, PLR_ATTACKER);
      }
    }

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "You do the best you can!\n\r", ch );
	return;
    }

    if ( !IS_NPC( victim ) && IS_SET( ch->act, PLR_NICE ) )
    {
      send_to_char( "You feel too nice to do that!\n\r", ch );
      return;
    }
/*
    if ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_PK ) )
*/

if ( !check_pk_ok(victim,ch)) 
  return;


    if (!IS_NPC (victim))
       {
       sprintf( log_buf, "%s: murder %s.", ch->name, victim->name );
       log_string_plus( log_buf, LOG_NORMAL, ch->level );
       }

    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    sprintf( buf, "Help!  I am being attacked by %s!", 
                   IS_NPC( ch ) ? ch->short_descr : ch->name );
    if ( IS_PKILL(victim) )
	do_wartalk( victim, buf );
    else
	do_yell( victim, buf );

/* Replaced these with New check --GW */
//    check_killer( ch, victim );
//    check_illegal_pk( ch, victim );
//    check_attacker( ch, victim );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}

bool in_arena( CHAR_DATA *ch )
{

if ( !ch || !ch->in_room )
return FALSE;

if ( ch->in_room && ( ch->in_room->vnum < 30 || ch->in_room->vnum > 44 ) )
  return FALSE;

return TRUE;
}

bool check_illegal_pk( CHAR_DATA *ch, CHAR_DATA *victim )
{


  if (!IS_NPC (victim) && !IS_NPC(ch))
  {
	if (ch == victim)
	return FALSE;

     if ( !IS_NPC(ch) && IS_ADVANCED(ch) && ch->advclass == 14 
     && !IS_NPC(victim) && IS_ADVANCED(victim) && victim->advclass == 14
     && ch->advlevel <= (victim->advlevel+10) )
	return FALSE;

    if ( IS_SET( victim->pcdata->flags, PCFLAG_DEADLY ) 
    && IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) 
    && (ch->level > (victim->level+10) ) )
      return FALSE;

    if ( IS_SET( victim->pcdata->flags, PCFLAG_ASSASSIN ) ||
	 IS_SET( ch->pcdata->flags, PCFLAG_ASSASSIN) )
	return FALSE;

     if ( in_arena( ch ) && in_arena(victim) )
	return FALSE;

    if (chaos == 1)
	return FALSE;

/* In a Club Maze? */
	if ( ch->in_room && ch->in_room->area &&
		IS_SET( ch->in_room->area->flags, AFLAG_CLUB_HALL ) )
	return FALSE;


    if ( IS_SET(ch->pcdata->flags, PCFLAG_KOMBAT ) ) 
	return FALSE;

    if ( IS_SET(ch->pcdata->flagstwo, MOREPC_CTF ) ) 
	return FALSE;

      sprintf( log_buf, "%s performing illegal pkill on %s at %d",
      (IS_NPC(ch) ? ch->short_descr : ch->name),
      victim->name,
      ch->in_room->vnum );
      log_string( log_buf );
      to_channel( log_buf, CHANNEL_MONITOR, "[ ** ", LEVEL_IMMORTAL );
      return TRUE;
  }
  return FALSE;
}


void do_flee( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in=NULL;
    char buf[MAX_STRING_LENGTH];
    sh_int door;
    EXIT_DATA *pexit;
    bool succeded;
    sh_int attempt;

    if ( !who_fighting( ch ) )
    {
	if ( ch->position == POS_FIGHTING )
	{
	  if ( ch->mount )
	    ch->position = POS_MOUNTED;
	  else
	    ch->position = POS_STANDING;
	}
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    if ( ch->move <= 0 )
    {
	send_to_char( "You're too exhausted to flee from combat!\n\r", ch );
	return;
    }

    /* No fleeing while stunned. - Narn */
    /* Ya .. --GW */
    if ( ch->position < POS_FIGHTING || ch->stun_rounds > 0 )
    {
       send_to_char("your to stunned to do that!",ch);
	return; 
    }
    succeded = FALSE;
    was_in = ch->in_room;
    for ( attempt = 0; attempt < 6; attempt++ )
    {

	door = number_door( );
	if ( ( pexit = get_exit(was_in, door) ) == NULL 
	|| ( IS_SET(pexit->exit_info, EX_CLOSED)
	&&   !IS_AFFECTED( ch, AFF_PASS_DOOR ) )
	|| ( !IS_NPC(ch) && IS_SET( ch->pcdata->flagstwo, MOREPC_ALIENS ) &&
	      IS_SET( pexit->to_room->room_flags, ROOM_DEATH ) )
	|| ( IS_NPC(ch)
	&&   IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB) ) )
	    continue;

	if ( !can_flee_dir(ch, door) )
	  continue;

	succeded = TRUE; 
        affect_strip ( ch, gsn_sneak );
        xREMOVE_BIT( ch->affected_by, AFF_SNEAK );
	if ( ch->mount && ch->mount->fighting )
	    stop_fighting( ch->mount, TRUE );
	stop_fighting( ch, TRUE );
	move_char( ch, get_exit( ch->in_room, door ), 0, FALSE );
	now_in = ch->in_room;
	break;
        }

        if ( succeded == TRUE )
	{ 
	char_from_room(ch);
        char_to_room(ch, was_in );
//	ch->in_room = was_in;
	act( AT_FLEE, "$n flees head over heels!", ch, NULL, NULL, TO_ROOM );
        char_from_room(ch);
        char_to_room(ch, now_in);
//	ch->in_room = now_in;
	act( AT_FLEE, "$n glances around for signs of pursuit.", ch, NULL, NULL, TO_ROOM );

	if ( !IS_NPC(ch) && !in_arena(ch) && (war == 0))
	{
	    CHAR_DATA *wf = who_fighting(ch);
	    int level_ratio=0;	    
	    sprintf(buf, "You flee from combat!");
	    act( AT_FLEE, buf, ch, NULL, NULL, TO_CHAR );
	    if ( wf ) level_ratio = URANGE( 1, wf->level / ch->level, 50 );
	    if ( ch->pcdata->deity )
	    {
	      if ( wf && wf->race == ch->pcdata->deity->npcrace )
		adjust_favor( ch, 1, level_ratio );
   	      else
		if ( wf && wf->race == ch->pcdata->deity->npcfoe )
		  adjust_favor( ch, 16, level_ratio );
		else
		  adjust_favor( ch, 0, level_ratio );
	    }
	}
    
	stop_fighting( ch, TRUE );
	return;
   }    
    if ( !IS_NPC(ch) && !in_arena(ch) && (war == 0 ) )
    {
    sprintf(buf, "You attempt to flee from combat! And &RFAIL!&w\n\r");
    send_to_char( buf, ch );
    }
    else
    {
    send_to_char("You Failed to Flee!!\n\r",ch);
    }

    return;
}



void do_sla( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to SLAY, spell it out.\n\r", ch );
    return;
}



void do_slay( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

    argument = one_argument( argument, arg );
    one_argument( argument, arg2 );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Slay whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "Suicide is a mortal sin.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) && get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "immolate" ) )
    {
      act( AT_FIRE, "Your fireball turns $N into a blazing inferno.",  ch, NULL, victim, TO_CHAR    );
      act( AT_FIRE, "$n releases a searing fireball in your direction.", ch, NULL, victim, TO_VICT    );
      act( AT_FIRE, "$n points at $N, who bursts into a flaming inferno.",  ch, NULL, victim, TO_NOTVICT );
    }

    else if ( !str_cmp( arg2, "shatter" ) )
    {
      act( AT_LBLUE, "You freeze $N with a glance and shatter the frozen corpse into tiny shards.",  ch, NULL, victim, TO_CHAR    );
      act( AT_LBLUE, "$n freezes you with a glance and shatters your frozen body into tiny shards.", ch, NULL, victim, TO_VICT    );
      act( AT_LBLUE, "$n freezes $N with a glance and shatters the frozen body into tiny shards.",  ch, NULL, victim, TO_NOTVICT );
    }

    else if ( !str_cmp( arg2, "demon" ) )
    {
      act( AT_IMMORT, "You gesture, and a slavering demon appears.  With a horrible grin, the",  ch, NULL, victim, TO_CHAR );
      act( AT_IMMORT, "foul creature turns on $N, who screams in panic before being eaten alive.",  ch, NULL, victim, TO_CHAR );
      act( AT_IMMORT, "$n gestures, and a slavering demon appears.  The foul creature turns on",  ch, NULL, victim, TO_VICT );
      act( AT_IMMORT, "you with a horrible grin.   You scream in panic before being eaten alive.",  ch, NULL, victim, TO_VICT );
      act( AT_IMMORT, "$n gestures, and a slavering demon appears.  With a horrible grin, the",  ch, NULL, victim, TO_NOTVICT );
      act( AT_IMMORT, "foul creature turns on $N, who screams in panic before being eaten alive.",  ch, NULL, victim, TO_NOTVICT );
    }

    else if ( !str_cmp( arg2, "kombat" ) )
    {
    sprintf(buf,"THE SKIES TURN RED AS %s SCREAMS -- MORTAL KOMBAT!!!!!
In the distance you hear %s scream in pain, while being burned alive!\n\r",
		capitalize(ch->name),QUICKLINK(victim->name));
    echo_to_all(AT_CARNAGE,buf,ECHOTAR_ALL);
     }

    else if ( !str_cmp( arg2, "pinball") && get_trust(ch) >= 58)
    {
    sprintf(buf, "
\a\aThe sound of pinball machines fills the Realms! %s slowly
appears in the center of the Sky, directly above %s, holding a giant Pinball
Machine! Suddenly, %s drops the Pinball Machine, you hear a SPLAT as it
hits %s at Terminal Volocity!\n\r\n\r
You hear the sound of a Free Game in the Distance!\n\r",
QUICKLINK(ch->name),
QUICKLINK(victim->name),
QUICKLINK(ch->name),
QUICKLINK(victim->name));
echo_to_all(AT_CARNAGE,buf,ECHOTAR_ALL);
}
    
    else if ( !str_cmp( arg2, "flush") && get_trust(ch) >= LEVEL_GOD)
    {
       sprintf(buf, "
\a\aYou hear the slow sound of rushing water, and suddenly see a bolt
of entense energy slam into %s, paralyzing them.  Then
%s creates a giant toilet and drowns %s! You hear %s's
helpless gurgles for help.\n\r
You hear the faint sound of flushing in the distance...\n\r",
QUICKLINK(victim->name),
QUICKLINK(ch->name),
QUICKLINK(victim->name),
QUICKLINK(victim->name));
echo_to_all(AT_CARNAGE,buf,ECHOTAR_ALL);
}

    else if ( !str_cmp( arg2, "squish") && get_trust(ch) >= LEVEL_GOD )
    {
       sprintf(buf, "
\a\aSuddenly there is a dull rumble throughout the land. A bright beam of
intense light sent by %s encases %s. A giant hand then
appears and plummets  down to earth squashing %s with a
thunderous boom which is hear all over the land . The hills echo with 
%s's screams of pain.\n\r
The hand moves away revealing a smear of blood and flesh.\n\r",
QUICKLINK(ch->name),
QUICKLINK(victim->name),
QUICKLINK(victim->name),
QUICKLINK(victim->name));
echo_to_all(AT_CARNAGE,buf,ECHOTAR_ALL);
}
    else if ( !str_cmp( arg2, "soul" ) && get_trust(ch) >= LEVEL_GOD )
    {
       sprintf(buf,"
\a\aThe earth shakes violently! Suddenly %s raises from the ground up
to the center of the sky! In a large ball of flame demons seem to shoot
from all directions as %s points at %s and screams -- BANISH!!
%s is ripped limb from limb! then suddenly goes limp as the head
demon punches its claw through thier chest and rips out thier very SOUL!\n\r
%s's lifeless body hits the ground with a bone shattering thud....\n\r",
QUICKLINK(victim->name),
QUICKLINK(ch->name),
QUICKLINK(victim->name),
QUICKLINK(victim->name),
QUICKLINK(victim->name));

echo_to_all(AT_BLOOD,buf,ECHOTAR_ALL);
}

    else if ( !str_cmp( arg2, "pounce" ) && get_trust(ch) >= LEVEL_FATE )
    {
      act( AT_BLOOD, "Leaping upon $N with bared fangs, you tear open $S throat and toss the corpse to the ground...",  ch, NULL, victim, TO_CHAR );
      act( AT_BLOOD, "In a heartbeat, $n rips $s fangs through your throat!  Your blood sprays and pours to the ground as your life ends...", ch, NULL, victim, TO_VICT );
      act( AT_BLOOD, "Leaping suddenly, $n sinks $s fangs into $N's throat.  As blood sprays and gushes to the ground, $n tosses $N's dying body away.",  ch, NULL, victim, TO_NOTVICT );
    }
 
    else if ( !str_cmp( arg2, "slit" ) && get_trust(ch) >= LEVEL_FATE )
    {
      act( AT_BLOOD, "You calmly slit $N's throat.", ch, NULL, victim, TO_CHAR );
      act( AT_BLOOD, "$n reaches out with a clawed finger and calmly slits your throat.", ch, NULL, victim, TO_VICT );
      act( AT_BLOOD, "$n calmly slits $N's throat.", ch, NULL, victim, TO_NOTVICT );
    }

    else
    {
      act( AT_IMMORT, "You slay $N in cold blood!",  ch, NULL, victim, TO_CHAR    );
      act( AT_IMMORT, "$n slays you in cold blood!", ch, NULL, victim, TO_VICT    );
      act( AT_IMMORT, "$n slays $N in cold blood!",  ch, NULL, victim, TO_NOTVICT );
    }

    victim->died_in_room = NULL;
    victim->died_in_room = victim->in_room;
    set_cur_char(victim);
    raw_kill( ch, victim );
    return;
}

/*
 * Hit one guy with a projectile.
 * Handles use of missile weapons (wield = missile weapon)
 * or thrown items/weapons
 */
ch_ret projectile_hit( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield,
                       OBJ_DATA *projectile, sh_int dist )
{
    int victim_ac;
    int thac0;
    int thac0_00;
    int thac0_32;
    int plusris;
    int dam;
    int diceroll;
    int prof_bonus;
    int prof_gsn = -1;
    int proj_bonus;
    int dt;
    ch_ret retcode;

    if ( !projectile )
        return rNONE;

    if ( projectile->item_type == ITEM_PROJECTILE
    ||   projectile->item_type == ITEM_WEAPON )
    {
        dt = TYPE_HIT + projectile->value[3];
        proj_bonus = number_range(projectile->value[1], projectile->value[2]);
    }
    else
    {
        dt = TYPE_UNDEFINED;
        proj_bonus = number_range(1, URANGE(2, get_obj_weight(projectile), 100) );
    }

    /*
     * Can't beat a dead char!
     */
    if ( victim->position == POS_DEAD || char_died(victim) )
    {
        extract_obj(projectile);
        return rVICT_DIED;
    }

    if ( wield )
        prof_bonus = weapon_prof_bonus_check( ch, wield, &prof_gsn );
    else
        prof_bonus = 0;

    if ( dt == TYPE_UNDEFINED )
    {
        dt = TYPE_HIT;
        if ( wield && wield->item_type == ITEM_MISSILE_WEAPON )
            dt += wield->value[3];
    }

    /*
     * Calculate to-hit-armor-class-0 versus armor.
     */
    if ( IS_NPC(ch) )
    {
        thac0_00 = ch->mobthac0;
        thac0_32 =  0;
    }
    else
    {
        thac0_00 = class_table[ch->class]->thac0_00;
        thac0_32 = class_table[ch->class]->thac0_32;
    }
    thac0     = interpolate( ch->level, thac0_00, thac0_32 ) - GET_HITROLL(ch) + (dist*2);
    victim_ac = UMAX( -30, (int) (GET_AC(victim) / 20) );

    /* if you can't see what's coming... */
    if ( !can_see_obj( victim, projectile) )
        victim_ac += 1;
    if ( !can_see( ch, victim ) )
        victim_ac -= 4;

    /* Weapon proficiency bonus */
    victim_ac += prof_bonus;

    /*
     * The moment of excitement!
     */
    while ( ( diceroll = number_bits( 5 ) ) >= 20 )
        ;

    if ( diceroll == 0
    || ( diceroll != 19 && diceroll < thac0 - victim_ac ) )
    {
        /* Miss. */
        if ( prof_gsn != -1 )
            learn_from_failure( ch, prof_gsn );

        /* Do something with the projectile */
        if ( number_percent() < 50 )
            extract_obj(projectile);
        else
        {
            if ( projectile->in_obj )
                obj_from_obj(projectile);
            if ( projectile->carried_by )
                obj_from_char(projectile);
            obj_to_room(projectile, victim->in_room);
        }
        damage( ch, victim, 0, dt );
        tail_chain( );
        return rNONE;
    }

    /*
     * Hit.
     * Calc damage.
     */
    if ( !wield )       /* dice formula fixed by Thoric */
        dam = proj_bonus;
    else
        dam = number_range(wield->value[1], wield->value[2]) + (proj_bonus / 10);

    /*
     * Bonuses.
     */
    dam += GET_DAMROLL(ch);

    if ( prof_bonus )
        dam += prof_bonus / 4;

    /*
     * Calculate Damage Modifiers from Victim's Fighting Style
     */
/*    if ( victim->position == POS_BERSERK )
       dam = 1.2 * dam;
    else if ( victim->position == POS_AGGRESSIVE )
       dam = 1.1 * dam;
    else if ( victim->position == POS_DEFENSIVE )
       dam = .85 * dam;
    else if ( victim->position == POS_EVASIVE )
       dam = .8 * dam;*/

    if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_enhanced_damage] > 0 )
    {
        dam += (int) (dam * LEARNED(ch, gsn_enhanced_damage) / 120);
        learn_from_success( ch, gsn_enhanced_damage );
    }

    if ( !IS_AWAKE(victim) )
        dam *= 2;

    if ( dam <= 0 )
        dam = 1;

    plusris = 0;

    if ( IS_OBJ_STAT(projectile, ITEM_MAGIC) )
        dam = ris_damage( victim, dam, RIS_MAGIC );
    else
        dam = ris_damage( victim, dam, RIS_NONMAGIC );

    /*
     * Handle PLUS1 - PLUS6 ris bits vs. weapon hitroll -Thoric
     */
    if ( wield )
        plusris = obj_hitroll( wield );

    /* check for RIS_PLUSx                                      -Thoric */
    if ( dam )
    {
        int x, res, imm, sus, mod;

        if ( plusris )
           plusris = RIS_PLUS1 << UMIN(plusris, 7);

        /* initialize values to handle a zero plusris */
        imm = res = -1;  sus = 1;

        /* find high ris */
        for ( x = RIS_PLUS1; x <= RIS_PLUS6; x <<= 1 )
        {
           if ( IS_SET( victim->immune, x ) )
                imm = x;
           if ( IS_SET( victim->resistant, x ) )
                res = x;
           if ( IS_SET( victim->susceptible, x ) )
                sus = x;
        }
        mod = 10;
        if ( imm >= plusris )
          mod -= 10;
        if ( res >= plusris )
          mod -= 2;
        if ( sus <= plusris )
          mod += 2;

        /* check if immune */
        if ( mod <= 0 )
          dam = -1;
        if ( mod != 10 )
          dam = (dam * mod) / 10;
    }

    if ( prof_gsn != -1 )
    {
      if ( dam > 0 )
        learn_from_success( ch, prof_gsn );
      else
        learn_from_failure( ch, prof_gsn );
    }

    /* immune to damage */
    if ( dam == -1 )
    {
        if ( dt >= 0 && dt < top_sn )
        {
            SKILLTYPE *skill = skill_table[dt];
            bool found = FALSE;

            if ( skill->imm_char && skill->imm_char[0] != '\0' )
            {
                act( AT_HIT, skill->imm_char, ch, NULL, victim, TO_CHAR );
                found = TRUE;
            }
            if ( skill->imm_vict && skill->imm_vict[0] != '\0' )
            {
                act( AT_HITME, skill->imm_vict, ch, NULL, victim, TO_VICT );
                found = TRUE;
            }
            if ( skill->imm_room && skill->imm_room[0] != '\0' )
            {
                act( AT_ACTION, skill->imm_room, ch, NULL, victim, TO_NOTVICT );
                found = TRUE;
            }
            if ( found )
            {
                if ( number_percent() < 50 )
                    extract_obj(projectile);
                else
                {
                    if ( projectile->in_obj )
                        obj_from_obj(projectile);
                    if ( projectile->carried_by )
                        obj_from_char(projectile);
                    obj_to_room(projectile, victim->in_room);
                }
                return rNONE;
            }
        }
        dam = 0;
    }
    if ( (retcode = damage( ch, victim, dam, dt )) != rNONE )
    {
        extract_obj(projectile);
        return retcode;
    }
    if ( char_died(ch) )
    {
        extract_obj(projectile);
        return rCHAR_DIED;
    }
    if ( char_died(victim) )
    {
        extract_obj(projectile);
        return rVICT_DIED;
    }

    retcode = rNONE;
    if ( dam == 0 )
    {
        if ( number_percent() < 50 )
            extract_obj(projectile);
        else
        {
            if ( projectile->in_obj )
                obj_from_obj(projectile);
            if ( projectile->carried_by )
                obj_from_char(projectile);
            obj_to_room(projectile, victim->in_room);
        }
        return retcode;
    }

/* weapon spells        -Thoric */
    if ( wield
    &&  !IS_SET(victim->immune, RIS_MAGIC)
    &&  !IS_SET(victim->in_room->room_flags, ROOM_NO_MAGIC) )
    {
        AFFECT_DATA *aff;

        for ( aff = wield->pIndexData->first_affect; aff; aff = aff->next )
           if ( aff->location == APPLY_WEAPONSPELL
           &&   IS_VALID_SN(aff->modifier)
           &&   skill_table[aff->modifier]->spell_fun )
                retcode = (*skill_table[aff->modifier]->spell_fun) ( aff->modifier, (wield->level+3)/3, ch, victim );
        if ( retcode != rNONE || char_died(ch) || char_died(victim) )
        {
            extract_obj(projectile);
            return retcode;
        }
        for ( aff = wield->first_affect; aff; aff = aff->next )
           if ( aff->location == APPLY_WEAPONSPELL
           &&   IS_VALID_SN(aff->modifier)
           &&   skill_table[aff->modifier]->spell_fun )
                retcode = (*skill_table[aff->modifier]->spell_fun) ( aff->modifier, (wield->level+3)/3, ch, victim );
        if ( retcode != rNONE || char_died(ch) || char_died(victim) )
        {
            extract_obj(projectile);
            return retcode;
        }
    }
    extract_obj(projectile);

    tail_chain( );
    return retcode;
}


/* Can we flee that way? --GW */
bool can_flee_dir( CHAR_DATA *ch, int door )
{
CHAR_DATA *mob, *next_mob=NULL;
bool blocked, mob_found;

blocked=FALSE;
mob_found=FALSE;

/* Find a Mob with a block flag */
for( mob = ch->in_room->first_person; mob; mob=next_mob)
{
  next_mob=mob->next_in_room;

  if ( IS_NPC(mob) &&
       (IS_SET(mob->acttwo, ACT2_BNORTH) ||
        IS_SET(mob->acttwo, ACT2_BSOUTH) ||
        IS_SET(mob->acttwo, ACT2_BEAST) ||
        IS_SET(mob->acttwo, ACT2_BWEST) ||
        IS_SET(mob->acttwo, ACT2_BUP) ||
        IS_SET(mob->acttwo, ACT2_BDOWN)))
  {
    mob_found=TRUE;
    break;
  }
}

/* No mob? Kick us.. */
if ( !mob_found )
return TRUE;

/* Does it block the Dir im trying to flee!!?! */
switch(door)
{
  case DIR_NORTH:
     if ( IS_SET(mob->acttwo, ACT2_BNORTH ) )
     {
	blocked=TRUE;
	break;
     }
     else
	break;
  case DIR_SOUTH:
     if ( IS_SET(mob->acttwo, ACT2_BSOUTH ) )
     {
	blocked=TRUE;
	break;
     }
     else
	break;
  case DIR_EAST:
     if ( IS_SET(mob->acttwo, ACT2_BEAST ) )
     {
	blocked=TRUE;
	break;
     }
     else
	break;
  case DIR_WEST:
     if ( IS_SET(mob->acttwo, ACT2_BWEST ) )
     {
	blocked=TRUE;
	break;
     }
     else
	break;
  case DIR_UP:
     if ( IS_SET(mob->acttwo, ACT2_BUP ) )
     {
	blocked=TRUE;
	break;
     }
     else
	break;
  case DIR_DOWN:
     if ( IS_SET(mob->acttwo, ACT2_BDOWN ) )
     {
	blocked=TRUE;
	break;
     }
     else
	break;
}

/* Yep - tell them to fuck off.. */
if (blocked)
{
char buf[MSL];

buf[0]='\0';

  sprintf(buf,"HAHA! You can't flee from me, %s!",capitalize(ch->name));
  do_say(mob,buf);
  return FALSE;
}

/* Guess not.. run like a child! hehe */
return TRUE;
}

