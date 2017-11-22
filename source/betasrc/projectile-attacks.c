/*
 * Projectile code taken from SMAUG 1.4
 */
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "mud.h"

char *rev_exit( sh_int vdir );
char *myobj( OBJ_DATA *obj );

/*
 * Basically the same guts as do_scan() from above (please keep them in
 * sync) used to find the victim we're firing at.	-Thoric
 */
CHAR_DATA *scan_for_victim( CHAR_DATA *ch, EXIT_DATA *pexit, char *name )
{
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *was_in_room;
    sh_int dist, dir;
    sh_int max_dist = 10;

    if ( IS_AFFECTED(ch, AFF_BLIND) || !pexit )
	return NULL;
 
    was_in_room = ch->in_room;
    if ( IS_VAMPIRE(ch) && time_info.hour < 21 && time_info.hour > 5 )
	max_dist = 1;

    if ( ch->level < 50 ) --max_dist;
    if ( ch->level < 40 ) --max_dist;
    if ( ch->level < 30 ) --max_dist;

    for ( dist = 1; dist <= max_dist; )
    {
	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	    break;

	if ( room_is_private( pexit->to_room )
	&&   ch->level < LEVEL_GOD )
	    break;

	char_from_room( ch );
	char_to_room( ch, pexit->to_room );    

	if ( (victim=get_char_room(ch, name)) != NULL )
	{
	    char_from_room(ch);
	    char_to_room(ch, was_in_room);
	    return victim;
	}

	switch( ch->in_room->sector_type )
	{
	    default: dist++; break;
	    case SECT_AIR:
		if ( number_percent() < 80 ) dist++; break;
	    case SECT_INSIDE:
	    case SECT_FIELD:
	    case SECT_UNDERGROUND:
		dist++; break;
	    case SECT_FOREST:
	    case SECT_CITY:
	    case SECT_DESERT:
	    case SECT_HILLS:
		dist += 2; break;
	    case SECT_WATER_SWIM:
	    case SECT_WATER_NOSWIM:
		dist += 3; break;
	    case SECT_MOUNTAIN:
	    case SECT_UNDERWATER:
	    case SECT_OCEANFLOOR:
		dist += 4; break;
	}

	if ( dist >= max_dist )
	    break;

	dir = pexit->vdir;
	if ( (pexit=get_exit(ch->in_room, dir)) == NULL )
	    break;
    }
  
    char_from_room(ch);
    char_to_room(ch, was_in_room);

    return NULL;
}

/*
 * Search inventory for an appropriate projectile to fire.
 * Also search open quivers.					-Thoric
 */
OBJ_DATA *find_projectile( CHAR_DATA *ch, int type )
{
    OBJ_DATA *obj, *obj2;

    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
    {
	if ( can_see_obj(ch, obj) )
	{
	    if ( obj->item_type == ITEM_QUIVER && !IS_SET(obj->value[1], CONT_CLOSED) )
	    {
		for ( obj2 = obj->last_content; obj2; obj2 = obj2->prev_content )
		{
		    if ( obj2->item_type == ITEM_PROJECTILE
		    &&   obj2->value[3] == type )
			return obj2;
		}
	    }
	    if ( obj->item_type == ITEM_PROJECTILE && obj->value[3] == type )
		return obj;
	}
    }

    return NULL;
}


ch_ret spell_attack( int, int, CHAR_DATA *, void * );

/*
 * Perform the actual attack on a victim			-Thoric
 */
ch_ret ranged_got_target( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *weapon,
	OBJ_DATA *projectile, sh_int dist, sh_int dt, char *stxt, sh_int color )
{
    if ( IS_SET(ch->in_room->room_flags, ROOM_SAFE) )
    {
	/* safe room, bubye projectile */
	if ( projectile )
	{
	    ch_printf(ch, "Your %s is blasted from existance by a godly presense.",
		myobj(projectile) );
	    act( color, "A godly presence smites $p!", ch, projectile, NULL, TO_ROOM );
	    extract_obj(projectile);
	}
	else
	{
	    ch_printf(ch, "Your %s is blasted from existance by a godly presense.",
		stxt );
	    act( color, "A godly presence smites $t!", ch, aoran(stxt), NULL, TO_ROOM );
	}
	return rNONE;
    }

    if ( IS_NPC(victim) && IS_SET(victim->act, ACT_SENTINEL)
    &&   ch->in_room != victim->in_room )
    {
	/*
	 * letsee, if they're high enough.. attack back with fireballs
	 * long distance or maybe some minions... go herne! heh..
	 *
	 * For now, just always miss if not in same room  -Thoric
	 */

	if ( projectile )
	{
	    learn_from_failure( ch, gsn_missile_weapons );

	    /* 50% chance of projectile getting lost */
	    if ( number_percent() < 25 )
		extract_obj(projectile);
	    else
	    {
	 	if ( projectile->in_obj )
		    obj_from_obj(projectile);
	 	if ( projectile->carried_by )
		    obj_from_char(projectile);
	 	obj_to_room(projectile, victim->in_room);
	    }
	}
	return damage( ch, victim, 0, dt );
    }

    if ( number_percent() > 30 || (projectile && weapon) )
    {
	if ( projectile )
	    global_retcode = projectile_hit(ch, victim, weapon, projectile, dist );
	else
	    global_retcode = spell_attack( dt, ch->level, ch, victim );
    }
    else
    {
	learn_from_failure( ch, gsn_missile_weapons );
	global_retcode = damage( ch, victim, 0, dt );

	if ( projectile )
	{
	    /* 50% chance of getting lost */
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
	}
    }
    return global_retcode;
}

/*
 * Generic use ranged attack function			-Thoric & Tricops
 */
ch_ret ranged_attack( CHAR_DATA *ch, char *argument, OBJ_DATA *weapon,
		      OBJ_DATA *projectile, sh_int dt, sh_int range )
{
    CHAR_DATA *victim, *vch;
    EXIT_DATA *pexit;
    ROOM_INDEX_DATA *was_in_room;
    char arg[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char temp[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    SKILLTYPE *skill = NULL;
    sh_int dir = -1, dist = 0, color = AT_GREY;
    char *dtxt = "somewhere";
    char *stxt = "burst of energy";
    int count;


    if ( argument && argument[0] != '\0' && argument[0] == '\''){
      one_argument( argument, temp );
      argument = temp;
    }

    argument = one_argument(argument, arg);
    argument = one_argument(argument, arg1);

    if ( arg[0] == '\0' )
    {
	send_to_char( "Where?  At who?\n\r", ch );
	return rNONE;
    }

    victim = NULL;

    /* get an exit or a victim */
    if ( (pexit = find_door(ch, arg, TRUE)) == NULL )
    {
	if ( (victim=get_char_room(ch, arg)) == NULL )
	{
	    send_to_char( "Aim in what direction?\n\r", ch );
	    return rNONE;
	}
	else
	{
	    if ( who_fighting(ch) == victim )
	    {
		send_to_char( "They are too close to release that type of attack!\n\r", ch );
		return rNONE;
	    }
	    if ( !IS_NPC(ch) && !IS_NPC(victim) )
	    {
	    	send_to_char("Pkill like a real pkiller.\n\r", ch );
		return rNONE;
	    }
	}
    }
    else
	dir = pexit->vdir;

    /* check for ranged attacks from private rooms, etc */
    if ( !victim )
    {
	if ( IS_SET(ch->in_room->room_flags, ROOM_PRIVATE)
	||   IS_SET(ch->in_room->room_flags, ROOM_SOLITARY) )
	{
	    send_to_char( "You cannot perform a ranged attack from a private room.\n\r", ch );
	    return rNONE;
	}
	if ( ch->in_room->tunnel > 0 )
	{
	    count = 0;
	    for ( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
		++count;
	    if ( count >= ch->in_room->tunnel )
	    {
		send_to_char( "This room is too cramped to perform such an attack.\n\r", ch );
		return rNONE;
	    }
	}
    }

    if ( IS_VALID_SN(dt) )
	skill = skill_table[dt];

    if ( pexit && !pexit->to_room )
    {
	send_to_char( "Are you expecting to fire through a wall!?\n\r", ch );
	return rNONE;
    }

    /* Check for obstruction */
    if ( pexit && IS_SET(pexit->exit_info, EX_CLOSED) )
    {
	if ( IS_SET(pexit->exit_info, EX_SECRET)
	||   IS_SET(pexit->exit_info, EX_DIG) )
	    send_to_char( "Are you expecting to fire through a wall!?\n\r", ch );
	else
	    send_to_char( "Are you expecting to fire through a door!?\n\r", ch );
	return rNONE;
    }

    vch = NULL;
    if ( pexit && arg1[0] != '\0' )
    {
	if ( (vch=scan_for_victim(ch, pexit, arg1)) == NULL )
	{
	    send_to_char( "You cannot see your target.\n\r", ch );
	    return rNONE;
	}

	/* don't allow attacks on mobs stuck in another area?
	if ( IS_NPC(vch) && xIS_SET(vch->act, ACT_STAY_AREA)
	&&   ch->in_room->area != vch->in_room->area) )
	{
	}
	*/
	/*don't allow attacks on mobs that are in a no-missile room --Shaddai */
/*	if ( IS_SET(vch->in_room->room_flags, ROOM_NOMISSILE) )
	{
	    send_to_char( "You can't get a clean shot off.\n\r", ch );
	    return rNONE;
	}*/
	if ( !IS_NPC(ch) && !IS_NPC(vch) )
	{
	    send_to_char("Pkill like a real pkiller.\n\r", ch );
	    return rNONE;
	}

	/* can't properly target someone heavily in battle */
	if ( vch->num_fighting > max_fight(vch) )
	{
	    send_to_char( "There is too much activity there for you to get a clear shot.\n\r", ch );
	    return rNONE;
	}
    }
    if ( vch ) {
    if ( !IS_NPC( vch ) && !IS_NPC( ch ) &&
	 IS_SET(ch->act, PLR_NICE ) )
    {
	send_to_char( "Your too nice to do that!\n\r", ch );
	return rNONE;
    }
    if ( vch && is_safe(ch, vch) )
	    return rNONE;
    }
    was_in_room = ch->in_room;

    if ( projectile )
    {
	separate_obj(projectile);
	if ( pexit )
	{
	    if ( weapon )
	    {
		act( AT_GREY, "You fire $p $T.", ch, projectile, dir_name[dir], TO_CHAR );
		act( AT_GREY, "$n fires $p $T.", ch, projectile, dir_name[dir], TO_ROOM );
	    }
	    else
	    {
		act( AT_GREY, "You throw $p $T.", ch, projectile, dir_name[dir], TO_CHAR );
		act( AT_GREY, "$n throw $p $T.", ch, projectile, dir_name[dir], TO_ROOM );
	    }
	}
	else
	{
	    if ( weapon )
	    {
		act( AT_GREY, "You fire $p at $N.", ch, projectile, victim, TO_CHAR );
		act( AT_GREY, "$n fires $p at $N.", ch, projectile, victim, TO_NOTVICT );
		act( AT_GREY, "$n fires $p at you!", ch, projectile, victim, TO_VICT );
	    }
	    else
	    {
		act( AT_GREY, "You throw $p at $N.", ch, projectile, victim, TO_CHAR );
		act( AT_GREY, "$n throws $p at $N.", ch, projectile, victim, TO_NOTVICT );
		act( AT_GREY, "$n throws $p at you!", ch, projectile, victim, TO_VICT );
	    }
	}
    }
    else
    if ( skill )
    {
	if ( skill->noun_damage && skill->noun_damage[0] != '\0' )
	    stxt = skill->noun_damage;
	else
	    stxt = skill->name;
	/* a plain "spell" flying around seems boring */
	if ( !str_cmp(stxt, "spell") )
	    stxt = "magical burst of energy";
	if ( skill->type == SKILL_SPELL )
	{
	    color = AT_MAGIC;
	    if ( pexit )
	    {
		act( AT_MAGIC, "You release $t $T.", ch, aoran(stxt), dir_name[dir], TO_CHAR );
		act( AT_MAGIC, "$n releases $s $t $T.", ch, stxt, dir_name[dir], TO_ROOM );
	    }
	    else
	    {
		act( AT_MAGIC, "You release $t at $N.", ch, aoran(stxt), victim, TO_CHAR );
		act( AT_MAGIC, "$n releases $s $t at $N.", ch, stxt, victim, TO_NOTVICT );
		act( AT_MAGIC, "$n releases $s $t at you!", ch, stxt, victim, TO_VICT );
	    }
	}
    }
    else
    {
	bug( "Ranged_attack: no projectile, no skill dt %d", dt );
	return rNONE;
    }
  
    /* victim in same room */
    if ( victim )
    {
//    	check_illegal_pk( ch, victim );
//	check_attacker( ch, victim );

	if ( !check_pk_ok(victim,ch) )
	return rNONE;

	return ranged_got_target( ch, victim, weapon, projectile,
		0, dt, stxt, color );
    }

    /* assign scanned victim */
    victim = vch;

    /* reverse direction text from move_char */
    dtxt = rev_exit(pexit->vdir);

    while ( dist <= range )
    {
	char_from_room(ch);
	char_to_room(ch, pexit->to_room);

	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	{
	    /* whadoyahknow, the door's closed */
	    if ( projectile )
		sprintf(buf,"You see your %s pierce a door in the distance to the %s.", 
		    myobj(projectile), dir_name[dir] );
	    else
		sprintf(buf, "You see your %s hit a door in the distance to the %s.",
		    stxt, dir_name[dir] );
	    act( color, buf, ch, NULL, NULL, TO_CHAR );
	    if ( projectile )
	    {
		sprintf(buf,"$p flies in from %s and implants itself solidly in the %sern door.",
		    dtxt, dir_name[dir] );
		act( color, buf, ch, projectile, NULL, TO_ROOM );
	    }
	    else
	    {
		sprintf(buf, "%s flies in from %s and implants itself solidly in the %sern door.",
		    aoran(stxt), dtxt, dir_name[dir] );
		buf[0] = UPPER(buf[0]);
		act( color, buf, ch, NULL, NULL, TO_ROOM );
	    }
	    break; 
	}


	/* no victim? pick a random one */
	if ( !victim )
	{
	    for ( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
	    {
		if ( ((IS_NPC(ch) && !IS_NPC(vch))
		||   (!IS_NPC(ch) &&  IS_NPC(vch)))
		&&    number_bits(1) == 0 )
		{
		    victim = vch;
		    break;
		}
	    }
	    if ( victim && is_safe(ch, victim) )
	    {
	        char_from_room(ch);
	        char_to_room(ch, was_in_room);
		return rNONE;
	    }
	}

	/* In the same room as our victim? */
	if ( victim && ch->in_room == victim->in_room )
	{
	    if ( projectile )
		act( color, "$p flies in from $T.", ch, projectile, dtxt, TO_ROOM );
	    else
		act( color, "$t flies in from $T.", ch, aoran(stxt), dtxt, TO_ROOM );

	    /* get back before the action starts */
	    char_from_room(ch);
	    char_to_room(ch, was_in_room);

//	    check_illegal_pk( ch, victim );
//	    check_attacker( ch, victim );

	if ( !check_pk_ok(victim,ch) )
	   return rNONE;
	
	    return ranged_got_target( ch, victim, weapon, projectile,
					dist, dt, stxt, color );
 	}

	if ( dist == range ) 
	{
	    if ( projectile )
	    {
		act( color, "Your $t falls harmlessly to the ground to the $T.", 
		    ch, myobj(projectile), dir_name[dir], TO_CHAR );
		act( color, "$p flies in from $T and falls harmlessly to the ground here.",
		    ch, projectile, dtxt, TO_ROOM );
		if ( projectile->in_obj )
		    obj_from_obj(projectile);
		if ( projectile->carried_by )
		    obj_from_char(projectile);
		obj_to_room(projectile, ch->in_room);
	    }
	    else
	    {
		act( color, "Your $t fizzles out harmlessly to the $T.", ch, stxt, dir_name[dir], TO_CHAR );
		act( color, "$t flies in from $T and fizzles out harmlessly.",
		    ch, aoran(stxt), dtxt, TO_ROOM );
	    }
	    break;
	}

	if ( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
	{
	    if ( projectile )
	    {
		act( color, "Your $t hits a wall and bounces harmlessly to the ground to the $T.", 
		    ch, myobj(projectile), dir_name[dir], TO_CHAR );
		act( color, "$p strikes the $Tsern wall and falls harmlessly to the ground.",
		    ch, projectile, dir_name[dir], TO_ROOM );
		if ( projectile->in_obj )
		    obj_from_obj(projectile);
		if ( projectile->carried_by )
		    obj_from_char(projectile);
		obj_to_room(projectile, ch->in_room);
	    }
	    else
	    {
		act( color, "Your $t harmlessly hits a wall to the $T.", 
		    ch, stxt, dir_name[dir], TO_CHAR );
		act( color, "$t strikes the $Tsern wall and falls harmlessly to the ground.",
		    ch, aoran(stxt), dir_name[dir], TO_ROOM );
	    }
	    break;
	}
	if ( projectile )
	    act( color, "$p flies in from $T.", ch, projectile, dtxt, TO_ROOM );
	else
	    act( color, "$t flies in from $T.", ch, aoran(stxt), dtxt, TO_ROOM );
	dist++;
    }

    char_from_room( ch );
    char_to_room( ch, was_in_room );

    return rNONE;
}

/*
 * Fire <direction> <target>
 *
 * Fire a projectile from a missile weapon (bow, crossbow, etc)
 *
 * Design by Thoric, coding by Thoric and Tricops.
 *
 * Support code (see projectile_hit(), quiver support, other changes to
 * fight.c, etc by Thoric.
 */
void do_fire( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *arrow;
    OBJ_DATA *bow;
    sh_int max_dist;

    if ( argument[0] == '\0' || !str_cmp(argument, " ") )
    {
	send_to_char( "Fire where at who?\n\r", ch );
	return;
    }

    if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
	set_char_color( AT_MAGIC, ch );
	send_to_char( "A magical force prevents you from attacking.\n\r", ch );
	return;
    }

    /*
     * Find the projectile weapon
     */
    if ( (bow=get_eq_char(ch, WEAR_MISSILE_WIELD)) != NULL )
	if ( !(bow->item_type == ITEM_MISSILE_WEAPON) )
	    bow = NULL;

    if ( !bow )
    {
	send_to_char( "You are not wielding a missile weapon.\n\r", ch );
	return;
    }

    /* modify maximum distance based on bow-type and ch's class/str/etc */
    max_dist = URANGE( 1, bow->value[4], 10 );

    if ( (arrow=find_projectile(ch, bow->value[3])) == NULL )
    {
	char *msg = "You have nothing to fire...\n\r";

	switch( bow->value[3] )
	{
	    case DAM_BOLT:	msg = "You have no bolts...\n\r";	break;
	    case DAM_ARROW:	msg = "You have no arrows...\n\r";	break;
	    case DAM_DART:	msg = "You have no darts...\n\r";	break;
	    case DAM_STONE:	msg = "You have no slingstones...\n\r";	break;
	    case DAM_PEA:	msg = "You have no peas...\n\r";	break;
	}
	send_to_char( msg, ch );
	return;
    }

    /* handle the ranged attack */
    ranged_attack( ch, argument, bow, arrow, TYPE_HIT + arrow->value[3], max_dist );

    return;
}

/*
 * Attempt to fire at a victim.
 * Returns FALSE if no attempt was made
 */
bool mob_fire( CHAR_DATA *ch, char *name )
{
    OBJ_DATA *arrow;
    OBJ_DATA *bow;
    sh_int max_dist;

    if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
	return FALSE;

    if ( (bow=get_eq_char(ch, WEAR_MISSILE_WIELD)) != NULL )
	if ( !(bow->item_type == ITEM_MISSILE_WEAPON) )
	    bow = NULL;

    if ( !bow )
	return FALSE;

    /* modify maximum distance based on bow-type and ch's class/str/etc */
    max_dist = URANGE( 1, bow->value[4], 10 );

    if ( (arrow=find_projectile(ch, bow->value[3])) == NULL )
	return FALSE;

    ranged_attack( ch, name, bow, arrow, TYPE_HIT + arrow->value[3], max_dist );

    return TRUE;
}

/*------------------------------------------------------------ 
 *  Fighting Styles - haus
 */
/*
void do_style( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    int percent; 

    if ( IS_NPC(ch) )
	return;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	ch_printf_color( ch, "&wAdopt which fighting style?  (current:  %s&w)\n\r",
          ch->style == STYLE_BERSERK    ? "&Rberserk"    :
          ch->style == STYLE_AGGRESSIVE ? "&Raggressive" :
          ch->style == STYLE_DEFENSIVE  ? "&Ydefensive"  :
          ch->style == STYLE_EVASIVE    ? "&Yevasive"    :
                                          "standard"     );
	return;
    }

    if( !str_prefix(arg, "evasive") ){
      if( ch->level < skill_table[gsn_style_evasive]->skill_level[ch->class])
      {
         send_to_char( "You have not yet learned enough to fight evasively.\n\r",ch);
         return;
      }
      WAIT_STATE( ch, skill_table[gsn_style_evasive]->beats );
      if( number_percent() < LEARNED(ch, gsn_style_evasive) ){
           success 
	  if(ch->fighting){
             ch->position = POS_EVASIVE;
             learn_from_success(ch,gsn_style_evasive);
	     if ( IS_PKILL( ch ) )
		act( AT_ACTION, "$n falls back into an evasive stance.",
                  ch, NULL, NULL, TO_ROOM );
          }
          ch->style = STYLE_EVASIVE;
          send_to_char( "You adopt an evasive fighting style.\n\r",ch);
          return;
      } else {
           failure  
          send_to_char( "You nearly trip in a lame attempt to adopt an evasive fighting style.\n\r",ch);
	  return;
      }
    } else if( !str_prefix(arg, "defensive")){
      if( ch->level < skill_table[gsn_style_defensive]->skill_level[ch->class])
      {
         send_to_char( "You have not yet learned enough to fight defensively.\n\r",ch);
         return;
      }
      WAIT_STATE( ch, skill_table[gsn_style_defensive]->beats );
      if( number_percent() < LEARNED(ch, gsn_style_defensive) ){
           success 
	  if(ch->fighting) {
             ch->position = POS_DEFENSIVE;
             learn_from_success(ch,gsn_style_defensive);
	     if ( IS_PKILL( ch ) )
		act( AT_ACTION, "$n moves into a defensive posture.",
                  ch, NULL, NULL, TO_ROOM );
          }
          ch->style = STYLE_DEFENSIVE;
          send_to_char( "You adopt a defensive fighting style.\n\r",ch);
          return;
      } else {
           failure  
          send_to_char( "You nearly trip in a lame attempt to adopt a defensive fighting style.\n\r",ch);
	  return;
      }
    } else if( !str_prefix(arg,"standard")){
      if( ch->level < skill_table[gsn_style_standard]->skill_level[ch->class])
      {
         send_to_char( "You have not yet learned enough to fight in the standard style.\n\r",ch);
         return;
      }
      WAIT_STATE( ch, skill_table[gsn_style_standard]->beats );
      if( number_percent() < LEARNED(ch, gsn_style_standard) ){
           success 
	  if(ch->fighting) {
             ch->position = POS_FIGHTING;
             learn_from_success(ch,gsn_style_standard);
	      if ( IS_PKILL( ch ) )
		act( AT_ACTION, "$n switches to a standard fighting style.",
                  ch, NULL, NULL, TO_ROOM );
          }
          ch->style = STYLE_FIGHTING;
          send_to_char( "You adopt a standard fighting style.\n\r",ch);
          return;
      } else {
           failure  
          send_to_char( "You nearly trip in a lame attempt to adopt a standard fighting style.\n\r",ch);
	  return;
      }
    } else if( !str_prefix(arg,"aggressive")){
      if( ch->level < skill_table[gsn_style_aggressive]->skill_level[ch->class])
      {
         send_to_char( "You have not yet learned enough to fight aggressively.\n\r",ch);
         return;
      }
      WAIT_STATE( ch, skill_table[gsn_style_aggressive]->beats );
      if( number_percent() < LEARNED(ch, gsn_style_aggressive) ){
           success 
	  if(ch->fighting) {
             ch->position = POS_AGGRESSIVE;
             learn_from_success(ch,gsn_style_aggressive);
	      if ( IS_PKILL( ch ) )
		act( AT_ACTION, "$n assumes an aggressive stance.",
                  ch, NULL, NULL, TO_ROOM );
          }
          ch->style = STYLE_AGGRESSIVE;
          send_to_char( "You adopt an aggressive fighting style.\n\r",ch);
          return;
      } else {
           failure  
          send_to_char( "You nearly trip in a lame attempt to adopt an aggressive fighting style.\n\r",ch);
	  return;
      }
    } else if( !str_prefix(arg,"berserk")){
      if( ch->level < skill_table[gsn_style_berserk]->skill_level[ch->class])
      {
         send_to_char( "You have not yet learned enough to fight as a berserker.\n\r",ch);
         return;
      }
      WAIT_STATE( ch, skill_table[gsn_style_berserk]->beats );
      if( number_percent() < LEARNED(ch, gsn_style_berserk) ){
           success 
	  if(ch->fighting) {
             ch->position = POS_BERSERK;
             learn_from_success(ch,gsn_style_berserk);
	      if ( IS_PKILL( ch ) )
		act( AT_ACTION, "$n enters a wildly aggressive style.",
                  ch, NULL, NULL, TO_ROOM );
          }
          ch->style = STYLE_BERSERK;
          send_to_char( "You adopt a berserk fighting style.\n\r",ch);
          return;
      } else {
           failure  
          send_to_char( "You nearly trip in a lame attempt to adopt a berserk fighting style.\n\r",ch);
	  return;
      }
    }

    send_to_char( "Adopt which fighting style?\n\r",ch);

    return;
}
*/

char *rev_exit( sh_int vdir )
{
    switch( vdir )
    {
        default: return "somewhere";
        case 0:  return "the south";
        case 1:  return "the west";
        case 2:  return "the north";
        case 3:  return "the east";
        case 4:  return "below";
        case 5:  return "above";
        case 6:  return "the southwest";
        case 7:  return "the southeast";
        case 8:  return "the northwest";
        case 9:  return "the northeast";
    }

    return "<???>";
}

/*
 * Function to strip off the "a" or "an" or "the" or "some" from an object's
 * short description for the purpose of using it in a sentence sent to
 * the owner of the object.  (Ie: an object with the short description
 * "a long dark blade" would return "long dark blade" for use in a sentence
 * like "Your long dark blade".  The object name isn't always appropriate
 * since it contains keywords that may not look proper.         -Thoric
 */
char *myobj( OBJ_DATA *obj )
{
    if ( !str_prefix("a ", obj->short_descr) )
        return obj->short_descr + 2;
    if ( !str_prefix("an ", obj->short_descr) )
        return obj->short_descr + 3;
    if ( !str_prefix("the ", obj->short_descr) )
        return obj->short_descr + 4;
    if ( !str_prefix("some ", obj->short_descr) )
        return obj->short_descr + 5;
    return obj->short_descr;
}

