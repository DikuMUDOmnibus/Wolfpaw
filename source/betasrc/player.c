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
 * 		Commands for personal player settings/statictics	    *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

/*
 *  Locals
 */
char *tiny_affect_loc_name(int location);
char *  get_class2       args( (CHAR_DATA *ch) );    
char *  get_advclass       args( (CHAR_DATA *ch) );    
char *  get_advclass2      args( (CHAR_DATA *ch) );    
char *  get_ego_text	 args( ( CHAR_DATA *ch ) );
sh_int	total_affects	args( ( CHAR_DATA *ch ) );
sh_int	total_resists	args( ( CHAR_DATA *ch ) );
sh_int  get_ac_type_char args( ( CHAR_DATA *ch ) );
bool check_new_char_pass( char *argument );
char *get_damage_value_obj( OBJ_DATA *obj );
int GET_HRDR_CAP( CHAR_DATA *ch);
int GET_AC_CAP( CHAR_DATA *ch);
int GET_MAC_CAP( CHAR_DATA *ch);
int GET_TOTALAC_CAP( CHAR_DATA *ch);
void add_secure_name( char *plrname, SECURE_DATA *secure, char *type );

void do_gold(CHAR_DATA * ch, char *argument)
{
   set_char_color( AT_GOLD, ch );
   ch_printf( ch,  "You have %s Copper pieces.\n\r", num_comma(ch->gold) );
   ch_printf( ch,  "You have %s Silver pieces.\n\r", num_comma(ch->silver) );
   ch_printf( ch,  "You have %s Gold pieces.\n\r", num_comma(ch->real_gold) );
   return;
}

void do_finger( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char * buf1;
  char * buf2;
  char * buf3;
  char * buf4;
  CHAR_DATA *victim;
  WIZENT *wiz, *wiz_next;
  extern WIZENT *first_wiz;

  buf[0] = '\0';
  if(IS_NPC(ch))
     return;

  if ( argument[0] == '\0' )
  {
      send_to_char("Finger whom?\n\r", ch );
      return;
  }

  victim = get_char_world(ch, argument);
  if ( ( victim == NULL ) || (!victim) )
  {
      read_finger( ch, argument );
      return;
  }
  if ( !can_see( ch, victim ) )
  {
      send_to_char("They aren't here.\n\r", ch );
      return;
  }

  if  ( IS_NPC( victim ) )
  {
      send_to_char("Not on NPC's.\n\r", ch );
      return;
  }

  /* No fingering those above your lvl --GW */
  for( wiz = first_wiz; wiz; wiz = wiz_next )
  {
  
   wiz_next = wiz->next;
    if ( !str_cmp( wiz->name, victim->name ) && 
        wiz->level > ch->level )
    {
      ch_printf( ch, "Information on %s is beyond your grasp!\n\r",capitalize(victim->name));
      return;
    }
  }

buf1 = STRALLOC(get_class(victim));
buf2 = STRALLOC(get_class2(victim));
buf3 = STRALLOC(get_advclass(victim));
buf4 = STRALLOC(get_advclass2(victim));

buf1 = STRALLOC(capitalize(buf1));
buf2 = STRALLOC(capitalize(buf2));
buf3 = STRALLOC(capitalize(buf3));
buf4 = STRALLOC(capitalize(buf4));

  send_to_char("          Finger Info\n\r", ch);
  send_to_char("          -----------\n\r", ch);
  ch_printf(ch, "&CName: &W%-12s\n\r", victim->name);
  ch_printf(ch, "&CMud Age: &W%2d\n\r", get_age( victim ));
  ch_printf(ch, "&CLevel: &W[&R%d&W/&R%d&W/&R%d&W/&R%d&W] [1/2/3/4]\n\r",
		victim->level, victim->level2, victim->advlevel,
		victim->advlevel2 );             
  ch_printf(ch, "&CSex: &W%s\n\r",
                victim->sex == SEX_MALE   ? "Male"   :
                victim->sex == SEX_FEMALE ? "Female" : "Neutral" );
  ch_printf(ch, "&CClasses: &W[&R%s&W/&R%s&W/&R%s&W/&R%s&W]\n\r",
       QUICKLINK(buf1),QUICKLINK(buf2),
			QUICKLINK(buf3), QUICKLINK(buf4));
  ch_printf(ch, "&CRace: &W%s\n\r",
     capitalize(npc_race[victim->race]) );
  ch_printf(ch, "&CTitle: &W%s\n\r", victim->pcdata->title );
  ch_printf(ch, "&CLast on: &W%s\n\r", (char *) ctime( &victim->pcdata->logon ) );
  return;

}

/* New New score command by Callinon :P */
void do_newscore(CHAR_DATA *ch, char *argument)
{
    char	buf[MAX_STRING_LENGTH];
    struct class_type *class;
    int		cl;
    
    cl    = ch->class;
    class = class_table[cl];
    
    if( IS_NPC(ch) )
    {
	do_oldscore(ch, argument);
	return;
    }
    
    ch_printf( ch, "&CScore sheet for &c%s&g\n\r", ch->name );
    ch_printf( ch, "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n\r",0);
    ch_printf( ch, "&CClass 1: &B%-15s          &CLevel: &B%-2d\n\r",
	capitalize(npc_class[ch->class]), ch->level );
    if( ch->class2 != -1 )
    ch_printf( ch, "&CClass 2: &B%-15s 	        &CLevel: &B%-2d\n\r",
	capitalize(npc_class[ch->class2]), ch->level2 );
    if( ch->advclass != -1 )
    ch_printf( ch, "&CClass 3: &R%-15s          &CLevel: &R%-2d\n\r",
	capitalize(npc_class[ch->advclass]), ch->advlevel );
    if ( ch->advclass2 != -1 )
    ch_printf( ch, "&CClass 4: &R%-15s	        &CLevel: &R%-2d\n\r",
	capitalize(npc_class[ch->advclass2]), ch->advlevel2 );
    ch_printf( ch, "&CRace   : &g%-15s          &CAge : &g%-3d\n\r",
	capitalize(get_race(ch)), get_age(ch) );
    ch_printf( ch, "&CExp    : &c%-11s\n\r", num_comma(ch->exp));
    if ( !IS_ADVANCED(ch) )
    {
    if ( ch->level < 50 )
    ch_printf( ch, "&CExp to level %d %s: &c%-11s\n\r",
	ch->level+1, capitalize(npc_class[ch->class]),num_comma(exp_level(ch,ch->level+1) - ch->exp) );
    if ( ch->class2 > -1 && ch->level2 > 0 && ch->level2 < 50 )
    ch_printf( ch, "&CExp to level %d %s: &c%-11s\n\r",
	ch->level2+1,capitalize(npc_class[ch->class2]),num_comma(exp_level2(ch,ch->level2+1)- ch->exp) );
    }
    if ( IS_ADVANCED(ch) && ch->advlevel < 50 )
    {
    ch_printf( ch, "&CExp to level %d %s: &c%-11s\n\r",
	ch->advlevel+1,capitalize(npc_class[ch->advclass]),num_comma(exp_level(ch,ch->advlevel+1) - ch->exp) );
    }
    if ( IS_ADV_DUAL(ch) && ch->advlevel2 < 50 )
    {
    ch_printf( ch, "&CExp to level %d %s: &c%-11s\n\r",
	ch->advlevel2+1,capitalize(npc_class[ch->advclass2]),num_comma(exp_level(ch,ch->advlevel2+1) - ch->exp) );
    }
    ch_printf( ch, "&g*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n\r",0);

    /* What Type We are.. */
    if ( get_ac_type_char(ch) == 0 )
    sprintf(buf,"Fighter");
    else if ( get_ac_type_char(ch) == 1 )
    sprintf(buf,"Magic User");
    else
    sprintf(buf,"Rogue Conjurer");
    ch_printf( ch,"&CYour Class Combination Depicts you as being a &c%s.\n\r",buf);
		

    if( GET_HITROLL(ch) < 10 )
	sprintf( buf, "You can't swat a fly" );
    else if( GET_HITROLL(ch) <= 30 )
	sprintf( buf, "You can hit the side of a castle" );
    else if( GET_HITROLL(ch) <= 60 )
	sprintf( buf, "You should be able to graze a tree" );
    else if( GET_HITROLL(ch) <= 100 )
	sprintf( buf, "You should be an archer" );
    else if( GET_HITROLL(ch) <= 300 )
	sprintf( buf, "Impressive" );
    else
	sprintf( buf, "Is there anything you CAN'T hit?" );

    ch_printf( ch, "&CHR :      &c%-5d&C(&c%d&C)/&c%-5d &C(&r%s&C)\n\r",
	GET_HITROLL(ch),ch->pcdata->hit_bonus,GET_HRDR_CAP(ch), buf );
    
    if( GET_DAMROLL(ch) < 10 )
	sprintf( buf, "Not planning on killing a roach are you?" );
    else if( GET_DAMROLL(ch) <= 30 )
	sprintf( buf, "Beastly Fidos beware!" );
    else if( GET_DAMROLL(ch) <= 60 )
	sprintf( buf, "Have you whacked a guard today?" );
    else if( GET_DAMROLL(ch) <= 100 )
	sprintf( buf, "Impressive" );
    else if( GET_DAMROLL(ch) <= 300 )
	sprintf( buf, "Please don't hurt me!" );
    else
	sprintf( buf, "You'd make a good TANK!" );

    ch_printf( ch, "&CDR :      &c%-5d&C(&c%d&C)/&c%-5d &C(&r%s&C)\n\r",
	GET_DAMROLL(ch),ch->pcdata->dam_bonus,GET_HRDR_CAP(ch),buf );

    if( GET_EQAC(ch) >= 101 )
	sprintf( buf, "Air hurts doesn't it" );
    else if( GET_EQAC(ch) >= 80 )
	sprintf( buf, "Watch out for rampaging dust" );
    else if( GET_EQAC(ch) >= 55 )
	sprintf( buf, "Naked eh?" );
    else if( GET_EQAC(ch) >= 40 )
	sprintf( buf, "There's a big HOLE in your armor" );
    else if( GET_EQAC(ch) >= 20 )
	sprintf( buf, "Leaves and a Loincloth are REALLY the best you can do?" );
    else if( GET_EQAC(ch) >= 10 )
	sprintf( buf, "It's better then nothing" );
    else if( GET_EQAC(ch) >= 0  )
	sprintf( buf, "Fine for light adventure" );
    else if( GET_EQAC(ch) >= -10)
	sprintf( buf, "HA that dust doesn't stand a chance now!" );
    else if( GET_EQAC(ch) >= -20)
	sprintf( buf, "Adequete" );
    else if( GET_EQAC(ch) >= -40)
	sprintf( buf, "Not too bad, could be better" );
    else if( GET_EQAC(ch) >= -55)
	sprintf( buf, "Ahh you're getting into the good stuff now eh" );
    else if( GET_EQAC(ch) >= -80)
	sprintf( buf, "Excellently crafted" );
    else if( GET_EQAC(ch) >= -101)
	sprintf( buf, "Now you're talkin" );
    else
	sprintf( buf, "You could probibly stop a cannonball" );
    ch_printf( ch, "&CEQ-AC :   &c%-5d&C(&c%d&C)/&c%-5d &C(&r%s&C)\n\r",
	GET_EQAC(ch), ch->pcdata->ac_bonus, GET_AC_CAP(ch), buf );

    if( GET_MAC(ch) >= 101 )
	sprintf( buf, "Air hurts doesn't it" );
    else if( GET_MAC(ch) >= 80 )
	sprintf( buf, "Watch out for rampaging dust" );
    else if( GET_MAC(ch) >= 55 )
	sprintf( buf, "Naked eh?" );
    else if( GET_MAC(ch) >= 40 )
	sprintf( buf, "There's a big HOLE in your armor" );
    else if( GET_MAC(ch) >= 20 )
	sprintf( buf, "Leaves and a Loincloth are REALLY the best you can do?" );
    else if( GET_MAC(ch) >= 10 )
	sprintf( buf, "It's better then nothing" );
    else if( GET_MAC(ch) >= 0  )
	sprintf( buf, "Fine for light adventure" );
    else if( GET_MAC(ch) >= -10)
	sprintf( buf, "HA that dust doesn't stand a chance now!" );
    else if( GET_MAC(ch) >= -20)
	sprintf( buf, "Adequete" );
    else if( GET_MAC(ch) >= -40)
	sprintf( buf, "Not too bad, could be better" );
    else if( GET_MAC(ch) >= -55)
	sprintf( buf, "Ahh you're getting into the good stuff now eh" );
    else if( GET_MAC(ch) >= -80)
	sprintf( buf, "Excellently crafted" );
    else if( GET_MAC(ch) >= -101)
	sprintf( buf, "Now you're talkin" );
    else
	sprintf( buf, "You could probibly stop a cannonball" );
    ch_printf( ch, "&CMAGIC AC :&c%-5d&C(&c%d&C)/&c%-5d &C(&r%s&C)\n\r",
	GET_MAC(ch), ch->pcdata->mac_bonus,GET_MAC_CAP(ch), buf );

    /* Total AC */
    ch_printf( ch, "&CTOTAL AC :&c%-5d&C(&c%d&C)/&c%-5d\n\r", GET_AC(ch),
	ch->pcdata->mac_bonus + ch->pcdata->ac_bonus,GET_TOTALAC_CAP(ch) );

    /* Added Align and Some other things --GW */
    if (ch->alignment > 900)
        sprintf(buf, "Devout");
    else if (ch->alignment > 700)
        sprintf(buf, "Noble");
    else if (ch->alignment > 350)
        sprintf(buf, "Honorable");
    else if (ch->alignment > 100)
        sprintf(buf, "Worthy");
    else if (ch->alignment > -100)
        sprintf(buf, "Neutral");
    else if (ch->alignment > -350)
        sprintf(buf, "Base");
    else if (ch->alignment > -700)
        sprintf(buf, "Evil");
    else if (ch->alignment > -900)
        sprintf(buf, "Ignoble");
    else
        sprintf(buf, "Fiendish");

    ch_printf( ch, "&CAlign :   &c%d &C(&r%s&C)\n\r",
        ch->alignment, buf);
    ch_printf( ch, "&g*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n\r",0);
    ch_printf( ch, "&CHP   : &c%-5d&C/&c%-5d     &CWeight: &c%4.4d\n\r",
	ch->hit, ch->max_hit, ch->carry_weight );
    ch_printf( ch, "&C%s: &c%-5d&C/&c%-5d &CMax Weight: &c%4.4d\n\r",
	IS_VAMPIRE(ch) ? "BLOOD" : "MP   ",
	IS_VAMPIRE(ch) ? ch->pcdata->condition[COND_BLOODTHIRST] : ch->mana,
	IS_VAMPIRE(ch) ? get_bloodthirst(ch) : ch->max_mana, can_carry_w(ch) );
    ch_printf( ch, "&CMOVE : &c%-5d&C/&c%-5d\n\r",
	ch->move, ch->max_move );
    ch_printf( ch, "&CPOWER: &c%-5d\n\r",
	get_power(ch) );
    ch_printf( ch, "&CCopper:  &c%s      &CBank:  &c%s\n\r",
	num_comma(ch->gold), num_comma(ch->pcdata->bank) );
    ch_printf( ch, "&CSilver:  &c%s      &CBank:  &c%s\n\r",
	num_comma(ch->silver), num_comma(ch->pcdata->silver_bank) );
    ch_printf( ch, "&CGold:    &c%s      &CBank:  &c%s\n\r",
	num_comma(ch->real_gold), num_comma(ch->pcdata->gold_bank) );
    ch_printf( ch, "&CGlory: &c%4.4d(%4.4d) \n\r",
        ch->pcdata->quest_curr, ch->pcdata->quest_accum );
    ch_printf( ch, "&CYour EGO &R%s\n\r",
	get_ego_text(ch));

    ch_printf( ch, "&CYour PK Safety flag is &c%s\n\r",
	IS_SET(ch->pcdata->flagstwo, MOREPC_SAFETY ) ? "Active" : "Disabled" );

    if( ch->pcdata->clan && ch->pcdata->clan->clan_type != CLAN_ORDER && ch->pcdata->clan->clan_type != CLAN_GUILD )
    {
    ch_printf( ch, "&g*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n\r",0);
    ch_printf( ch, "&CCLAN: %-6s           &CLEVEL: &c%-2d\n\r",
	ch->pcdata->clan->whoname, ch->pcdata->clevel );
    }
    if( ch->pcdata->clan && ch->pcdata->clan->clan_type == CLAN_ORDER )
    {
    ch_printf( ch, "&g*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n\r",0);
    ch_printf( ch, "&CORDER: %-6s        &CLEVEL: &c%-2d\n\r",
	ch->pcdata->clan->whoname, ch->pcdata->clevel );
    }
    if( ch->pcdata->guild_name != NULL &&
	str_cmp(ch->pcdata->guild_name,"(null)"))
    {
    ch_printf( ch, "&g*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n\r",0);
    ch_printf( ch, "&CGUILD: %s	       &CLEVEL: &c%-2d\n\r",
	ch->pcdata->guild_name, ch->pcdata->guildlevel );
    }
    if( ch->pcdata->deity )
    {
    ch_printf( ch, "&g*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n\r",0);
    ch_printf( ch, "&CDEITY: &c%-20s    &CFAVOR: &c%-4d\n\r",
	ch->pcdata->deity->name, ch->pcdata->favor );
    }
} 

/*
 * Attribute Command by Greywolf
 */
void do_attribute( CHAR_DATA *ch, char *argument )
{
    int iLang;
    struct class_type *class;
    int		cl;

if ( IS_NPC(ch) )
   return;
    
    cl    = ch->class;
    class = class_table[cl];

    ch_printf( ch, "&CAttributes for: &c%s\n\r", capitalize(ch->name));
    ch_printf( ch, "&g*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n\r",0);
    ch_printf( ch, "&CSTR:&c%-2d &C(&c%-2d&C)           &R%s\n\r",
	get_curr_str(ch), ch->perm_str, class->attr_prime == APPLY_STR ? "PRIME" : "" );
    ch_printf( ch, "&CDEX:&c%-2d &C(&c%-2d&C)           &R%s\n\r",
	get_curr_dex(ch), ch->perm_dex, class->attr_prime == APPLY_DEX ? "PRIME" : "" );
    ch_printf( ch, "&CINT:&c%-2d &C(&c%-2d&C)           &R%s\n\r",
	get_curr_int(ch), ch->perm_int, class->attr_prime == APPLY_INT ? "PRIME" : "" );
    ch_printf( ch, "&CWIS:&c%-2d &C(&c%-2d&C)           &R%s\n\r",
	get_curr_wis(ch), ch->perm_wis, class->attr_prime == APPLY_WIS ? "PRIME" : "" );
    ch_printf( ch, "&CCON:&c%-2d &C(&c%-2d&C)           &R%s\n\r",
	get_curr_con(ch), ch->perm_con, class->attr_prime == APPLY_CON ? "PRIME" : "" );
    ch_printf( ch, "&CCHA:&c%-2d &C(&c%-2d&C)		&R%s\n\r",
	get_curr_cha(ch), ch->perm_cha, class->attr_prime == APPLY_CHA ? "PRIME" : "" );
    ch_printf( ch, "&CLCK:&c%-2d &C(&c%-2d&C)		&R%s\n\r",
	get_curr_lck(ch), ch->perm_lck, class->attr_prime == APPLY_LCK ? "PRIME" : "" );
    ch_printf( ch, "&g*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n\r",0);
    if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
	send_to_char("&rYou are drunk.\n\r", ch);
    if (!IS_NPC(ch) && ch->level < LEVEL_AVATAR && ch->pcdata->condition[COND_THIRST] == 0)
	send_to_char("&rYou are in danger of dehydrating.\n\r", ch);
    if (!IS_NPC(ch) && ch->level < LEVEL_AVATAR && ch->pcdata->condition[COND_FULL] == 0)
	send_to_char("&rYou are starving to death.\n\r", ch);
    if ( ch->position != POS_SLEEPING )
	switch( ch->mental_state / 10 )
	{
	    default:   send_to_char( "&cYou're completely messed up!\n\r",ch );	break;
	    case -10:  send_to_char( "&cYou're barely conscious.\n\r", ch);	break;
	    case  -9:  send_to_char( "&cYou can barely keep your eyes open.\n\r", ch );	break;
	    case  -8:  send_to_char( "&cYou're extremely drowsy.\n\r", ch);	break;
	    case  -7:  send_to_char( "&cYou feel very unmotivated.\n\r",ch );	break;
	    case  -6:  send_to_char( "&cYou feel sedated.\n\r", ch ); break;
	    case  -5:  send_to_char( "&cYou feel sleepy.\n\r", ch ); break;
	    case  -4:  send_to_char( "&cYou feel tired.\n\r", ch ); break;
	    case  -3:  send_to_char( "&cYou could use a rest.\n\r", ch ); break;
	    case  -2:  send_to_char( "&cYou feel a little under the weather.\n\r", ch );	break;
	    case  -1:  send_to_char( "&cYou feel fine.\n\r", ch ); break;
	    case   0:  send_to_char( "&cYou feel great.\n\r", ch ); break;
	    case   1:  send_to_char( "&cYou feel energetic.\n\r", ch ); break;
	    case   2:  send_to_char( "&cYour mind is racing.\n\r", ch ); break;
	    case   3:  send_to_char( "&cYou can't think straight.\n\r", ch );	break;
	    case   4:  send_to_char( "&cYour mind is going 100 miles an hour.\n\r", ch );	break;
	    case   5:  send_to_char( "&cYou're high as a kite.\n\r", ch );break;
	    case   6:  send_to_char( "&cYour mind and body are slipping apart.\n\r", ch );	break;
	    case   7:  send_to_char( "&cReality is slipping away.\n\r", ch );	break;
	    case   8:  send_to_char( "&cYou have no idea what is real, and what is not.\n\r", ch );	break;
	    case   9:  send_to_char( "&cYou feel immortal.\n\r", ch );break;
	    case  10:  send_to_char( "&cYou are an Implementor.\n\r", ch); break;
	}
    else
    if ( ch->mental_state >45 )
	send_to_char( "&cYour sleep is filled with strange and vivid dreams.\n\r", ch );
    else
    if ( ch->mental_state >25 )
	send_to_char( "&cYour sleep is uneasy.\n\r", ch );
    else
    if ( ch->mental_state <-35 )
	send_to_char( "&cYou are deep in a much needed sleep.\n\r", ch );
    else
    if ( ch->mental_state <-25 )
	send_to_char( "&cYou are in deep slumber.\n\r", ch );
    send_to_char("&CLanguages:&c ", ch );
    for ( iLang = 0; lang_array[iLang] != LANG_UNKNOWN; iLang++ )
	if ( knows_language( ch, lang_array[iLang], ch )
	||  (IS_NPC(ch) && ch->speaks == 0) )
	{
	    if ( lang_array[iLang] & ch->speaking
	    ||  (IS_NPC(ch) && !ch->speaking) )
		set_char_color( AT_RED, ch );
	    send_to_char( lang_names[iLang], ch );
	    set_char_color( AT_PLAIN, ch );
	    send_to_char( " ", ch );
	}
    send_to_char( "\n\r", ch );
    if ( ch->pcdata->bounty > 0 )
	ch_printf( ch, "&RBOUNTY: &c%d\n\r", ch->pcdata->bounty );

    if ( ch->pcdata->bestowments && ch->pcdata->bestowments[0] != '\0' )
	ch_printf( ch, "&CYou are bestowed with the command(s): &c%s.\n\r", 
		ch->pcdata->bestowments );

    if ( CAN_PKILL( ch ) )
    {
        ch_printf( ch,"&g*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n\r",0);
	ch_printf(ch, "&CPKILL DATA:  &RPkills &C(&c%3.3d&C)     &RIllegal Pkills &C(&c%3.3d&C)     &RPdeaths &C(&c%3.3d&C)\n\r",
		ch->pcdata->pkills, ch->pcdata->illegal_pk, ch->pcdata->pdeaths );
    }
    if (ch->pcdata->deity)
    {
        ch_printf( ch,"&g*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n\r",0);
	ch_printf(ch, "&CDeity:  &c%-20s  &CFavor: &c%d\n\r", ch->pcdata->deity->name, ch->pcdata->favor );
    }
        ch_printf( ch,"&g*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n\r",0);
	ch_printf(ch, "&RCHALLENGE STATS: &CWins &c%d &CLoses: &c%d &CChampionships Won: &c%d\n",
	          ch->pcdata->awins,ch->pcdata->alosses,ch->pcdata->pkchamps );
    if ( ch->pcdata->gladiator != NULL )
    {
        ch_printf( ch,"&g*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n\r",0);
	ch_printf(ch,"&RGAMBLE STATS: &CCharacter: &c%s  &RBet: &c%d\n",ch->pcdata->gladiator,ch->pcdata->plr_wager);
    } 
        ch_printf( ch,"&g*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n\r",0);
	ch_printf(ch, "&RKOMBAT STATS:   &CKombats Entered: &c%d\n",ch->pcdata->num_kombats );
        ch_printf(ch, "              &CKombatants Killed: &c%d\n",ch->pcdata->num_kombatants);
	ch_printf(ch, "                    &CKombats Won: &c%d\n",ch->pcdata->num_kombats_won );

        /* Glory-Store Stuff --GW */
        ch_printf( ch,"&g*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n\r",0);
	ch_printf( ch, "&RGLORY STORE STATS:  &CSkills to 100%: &c%d\n",ch->pcdata->skill100s);
        ch_printf( ch, "	            &CAffects:        &c%d\n",total_affects(ch));
        ch_printf( ch, "    	            &CResistances:    &c%d\n",total_resists(ch));


    if (IS_IMMORTAL(ch))
    {
        ch_printf(ch,"&g*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*&C\n\r",0);

	ch_printf(ch, "IMMORTAL DATA:  Wizinvis [%s]  Wizlevel (%d)\n\r",
		IS_SET(ch->act, PLR_WIZINVIS) ? "X" : " ", ch->pcdata->wizinvis );
        ch_printf(ch, "Incog [%s] Incog level (%d)\n\r",
                IS_SET(ch->pcdata->flagstwo, MOREPC_INCOG) ? "X" : "
",ch->pcdata->incog_level ); /* Incog data added by Callinon */
	ch_printf(ch, "Bamfin:  %s\n\r",(ch->pcdata->bamfin[0] != '\0')
		? ch->pcdata->bamfin : "?? appears in a swirling mist.");
	ch_printf(ch, "Bamfout: %s\n\r",(ch->pcdata->bamfout[0] != '\0')
		? ch->pcdata->bamfout : "?? leaves in a swirling mist.");
	ch_printf(ch, "Creation Security: %d\n\r",ch->pcdata->cperm );
	
	/* Area Loaded info - Scryn 8/11*/
	if (ch->pcdata->area)
	{
	    ch_printf(ch, "Vnums:   Room (%-5.5d - %-5.5d)   Object (%-5.5d - %-5.5d)   Mob (%-5.5d - %-5.5d)\n\r",
		ch->pcdata->area->low_r_vnum, ch->pcdata->area->hi_r_vnum,
		ch->pcdata->area->low_o_vnum, ch->pcdata->area->hi_o_vnum,
		ch->pcdata->area->low_m_vnum, ch->pcdata->area->hi_m_vnum);
	    ch_printf(ch, "Area Loaded [%s]\n\r", (IS_SET(ch->pcdata->area->status, AREA_LOADED)) ? "yes" : "no");
	}
    }

return;
}

/*
 * New score command by Haus
 */
void do_score(CHAR_DATA * ch, char *argument)
{
    char            buf[MAX_STRING_LENGTH];
    AFFECT_DATA    *paf;
    int iLang;

    if (IS_NPC(ch))
    {
	do_oldscore(ch, argument);
	return;
    }
    set_char_color(AT_SCORE, ch);

    ch_printf(ch, "\n\rScore for %s.\n\r", ch->pcdata->title);

    send_to_char("----------------------------------------------------------------------------\n\r", ch);
	if( ch->level == 50 )
ch_printf(ch, "LEVEL: &R%-2d&C/%-2d         Race : %-8.10s        Played: %dhours\n\r", ch->level,ch->level2, capitalize(get_race(ch)), (get_age(ch) - 17) *2);

        else if( ch->level2 && ch->level2 == 50 )
ch_printf(ch, "LEVEL: %-2d/&R%-2d&C         Race : %-8.10s        Played: %dhours\n\r",ch->level,ch->level2, capitalize(get_race(ch)), (get_age(ch) - 17) *2);

	else if( ch->level == 50 && ch->level2 && ch->level2 == 50 )
ch_printf(ch, "LEVEL: &R%-2d/&R%-2d&C        Race : %-8.10s        Played: %d hours\n\r", ch->level,ch->level2, capitalize(get_race(ch)), (get_age(ch) - 17) *2);

	else
	ch_printf(ch, "LEVEL: %-2d/%-2d         Race : %-8.10s        Played: %d hours\n\r",
	ch->level,ch->level2, capitalize(get_race(ch)), (get_age(ch) - 17) * 2);

	if( IS_ADVANCED(ch))
	{
         if( ch->advlevel == 50 ) {
            ch_printf(ch, "ADVANCED LEVEL:&R%-2d&C     ADVANCED CLASS: %-11.11s\n\r",
	       ch->advlevel, capitalize(get_advclass(ch))); }
         else {
           ch_printf(ch, "ADVANCED LEVEL: %-2d     ADVANCED CLASS: %-11.11s\n\r",
		ch->advlevel, capitalize(get_advclass(ch))); }
	}
             ch_printf(ch, "YEARS: %-6d      Class: %-11.11s       Log In: %s\n\r",
		get_age(ch), capitalize(get_class(ch)),ctime(&(ch->pcdata->logon)) );
   if ( ch->class2 != -1)
    ch_printf(ch, "                 Class: %-11.11s\n\r",
	capitalize(get_class2(ch)) );  
   
  if (ch->level >= 5
    ||  IS_PKILL( ch ) )
    {
	ch_printf(ch, "STR  : %2.2d(%2.2d)    HitRoll: %-4d              Saved:  %s\r",
		get_curr_str(ch), ch->perm_str, GET_HITROLL(ch),ch->pcdata->save_time ? ctime(&(ch->pcdata->save_time)) : "no save this session\n");

	ch_printf(ch, "INT  : %2.2d(%2.2d)    DamRoll: %-4d              Time:   %s\r",
		get_curr_int(ch), ch->perm_int, GET_DAMROLL(ch), ctime(&current_time) );
    }
    else
    {
	ch_printf(ch, "STR  : %2.2d(%2.2d)                               Saved:  %s\r",
		get_curr_str(ch), ch->perm_str, ch->pcdata->save_time ? ctime(&(ch->pcdata->save_time)) : "no\n" );

	ch_printf(ch, "INT  : %2.2d(%2.2d)                               Time:   %s\r",
		get_curr_int(ch), ch->perm_int, ctime(&current_time) );
    }

    if (GET_AC(ch) >= 101)
	sprintf(buf, "the rags of a beggar");
    else if (GET_AC(ch) >= 80)
	sprintf(buf, "improper for adventure");
    else if (GET_AC(ch) >= 55)
	sprintf(buf, "shabby and threadbare");
    else if (GET_AC(ch) >= 40)
	sprintf(buf, "of poor quality");
    else if (GET_AC(ch) >= 20)
	sprintf(buf, "scant protection");
    else if (GET_AC(ch) >= 10)
	sprintf(buf, "that of a knave");
    else if (GET_AC(ch) >= 0)
	sprintf(buf, "moderately crafted");
    else if (GET_AC(ch) >= -10)
	sprintf(buf, "well crafted");
    else if (GET_AC(ch) >= -20)
	sprintf(buf, "the envy of squires");
    else if (GET_AC(ch) >= -40)
	sprintf(buf, "excellently crafted");
    else if (GET_AC(ch) >= -60)
	sprintf(buf, "the envy of knights");
    else if (GET_AC(ch) >= -80)
	sprintf(buf, "the envy of barons");
    else if (GET_AC(ch) >= -100)
	sprintf(buf, "the envy of dukes");
    else if (GET_AC(ch) >= -200)
	sprintf(buf, "the envy of emperors");
    else
	sprintf(buf, "that of an avatar");
    if (ch->level > 24)
	ch_printf(ch, "WIS  : %2.2d(%2.2d)      Armor: %4.4d, %s\n\r",
		get_curr_wis(ch), ch->perm_wis, GET_AC(ch), buf);
    else
	ch_printf(ch, "WIS  : %2.2d(%2.2d)      Armor: %s \n\r",
		get_curr_wis(ch), ch->perm_wis, buf);


    if (ch->alignment > 900)
	sprintf(buf, "devout");
    else if (ch->alignment > 700)
	sprintf(buf, "noble");
    else if (ch->alignment > 350)
	sprintf(buf, "honorable");
    else if (ch->alignment > 100)
	sprintf(buf, "worthy");
    else if (ch->alignment > -100)
	sprintf(buf, "neutral");
    else if (ch->alignment > -350)
	sprintf(buf, "base");
    else if (ch->alignment > -700)
	sprintf(buf, "evil");
    else if (ch->alignment > -900)
	sprintf(buf, "ignoble");
    else
	sprintf(buf, "fiendish");
    if (ch->level < 10)
	ch_printf(ch, "DEX  : %2.2d(%2.2d)      Align: %-20.20s    Items: %5.5d   (max %5.5d)\n\r",
		get_curr_dex(ch), ch->perm_dex, buf, ch->carry_number, can_carry_n(ch));
    else
	ch_printf(ch, "DEX  : %2.2d(%2.2d)      Align: %+4.4d, %-14.14s   Items: %5.5d   (max %5.5d)\n\r",
		get_curr_dex(ch), ch->perm_dex, ch->alignment, buf, ch->carry_number, can_carry_n(ch));

    switch (ch->position)
    {
	case POS_DEAD:
		sprintf(buf, "slowly decomposing");
		break;
	case POS_MORTAL:
		sprintf(buf, "mortally wounded");
		break;
	case POS_INCAP:
		sprintf(buf, "incapacitated");
		break;
	case POS_STUNNED:
		sprintf(buf, "stunned");
		break;
	case POS_SLEEPING:
		sprintf(buf, "sleeping");
		break;
	case POS_RESTING:
		sprintf(buf, "resting");
		break;
	case POS_STANDING:
		sprintf(buf, "standing");
		break;
	case POS_FIGHTING:
		sprintf(buf, "fighting");
		break;
	case POS_MOUNTED:
		sprintf(buf, "mounted");
		break;
        case POS_SITTING:
		sprintf(buf, "sitting");
		break;
    }
    ch_printf(ch, "CON  : %2.2d(%2.2d)      Pos'n: %-21.21s  Weight: %5.5d (max %7.7d)\n\r",
	get_curr_con(ch), ch->perm_con, buf, ch->carry_weight, can_carry_w(ch));

    ch_printf(ch, "CHA  : %2.2d(%2.2d)      Wimpy: %d \n\r",
	get_curr_cha(ch), ch->perm_cha, ch->wimpy);

    ch_printf(ch, "LCK  : %2.2d(%2.2d) \n\r",
	get_curr_lck(ch), ch->perm_lck);

    ch_printf(ch, "Glory: %4.4d(%4.4d) \n\r",
	ch->pcdata->quest_curr, ch->pcdata->quest_accum );

    ch_printf(ch, "PRACT: %3.3d         Hitpoints: %-5d of %5d   Pager: (%c) %3d    AutoExit(%c)\n\r",
	ch->practice, ch->hit, ch->max_hit,
	IS_SET(ch->pcdata->flags, PCFLAG_PAGERON) ? 'X' : ' ',
	ch->pcdata->pagerlen, IS_SET(ch->act, PLR_AUTOEXIT) ? 'X' : ' ');

    if (IS_VAMPIRE(ch))
	ch_printf(ch, "XP   : %-9d       Blood: %-5d of %5d   MKills:  %-5.5d    AutoLoot(%c)\n\r",
		ch->exp, ch->pcdata->condition[COND_BLOODTHIRST], get_bloodthirst(ch) , ch->pcdata->mkills,
		IS_SET(ch->act, PLR_AUTOLOOT) ? 'X' : ' ');
    else
	ch_printf(ch, "XP   : %-9d        Mana: %-5d of %5d   MKills:  %-5.5d    AutoLoot(%c)\n\r",
		ch->exp, ch->mana, ch->max_mana, ch->pcdata->mkills, IS_SET(ch->act, PLR_AUTOLOOT) ? 'X' : ' ');

    ch_printf(ch, "GOLD : %-10d       Move: %-5d of %5d   Mdeaths: %-5.5d    AutoSac (%c)\n\r",
	ch->gold, ch->move, ch->max_move, ch->pcdata->mdeaths, IS_SET(ch->act, PLR_AUTOSAC) ? 'X' : ' ');

    if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
	send_to_char("You are drunk.\n\r", ch);
    if (!IS_NPC(ch) && ch->level < LEVEL_AVATAR && ch->pcdata->condition[COND_THIRST] == 0)
	send_to_char("You are in danger of dehydrating.\n\r", ch);
    if (!IS_NPC(ch) && ch->level < LEVEL_AVATAR && ch->pcdata->condition[COND_FULL] == 0)
	send_to_char("You are starving to death.\n\r", ch);
    if ( ch->position != POS_SLEEPING )
	switch( ch->mental_state / 10 )
	{
	    default:   send_to_char( "You're completely messed up!\n\r", ch );	break;
	    case -10:  send_to_char( "You're barely conscious.\n\r", ch );	break;
	    case  -9:  send_to_char( "You can barely keep your eyes open.\n\r", ch );	break;
	    case  -8:  send_to_char( "You're extremely drowsy.\n\r", ch );	break;
	    case  -7:  send_to_char( "You feel very unmotivated.\n\r", ch );	break;
	    case  -6:  send_to_char( "You feel sedated.\n\r", ch );		break;
	    case  -5:  send_to_char( "You feel sleepy.\n\r", ch );		break;
	    case  -4:  send_to_char( "You feel tired.\n\r", ch );		break;
	    case  -3:  send_to_char( "You could use a rest.\n\r", ch );		break;
	    case  -2:  send_to_char( "You feel a little under the weather.\n\r", ch );	break;
	    case  -1:  send_to_char( "You feel fine.\n\r", ch );		break;
	    case   0:  send_to_char( "You feel great.\n\r", ch );		break;
	    case   1:  send_to_char( "You feel energetic.\n\r", ch );	break;
	    case   2:  send_to_char( "Your mind is racing.\n\r", ch );	break;
	    case   3:  send_to_char( "You can't think straight.\n\r", ch );	break;
	    case   4:  send_to_char( "Your mind is going 100 miles an hour.\n\r", ch );	break;
	    case   5:  send_to_char( "You're high as a kite.\n\r", ch );	break;
	    case   6:  send_to_char( "Your mind and body are slipping apart.\n\r", ch );	break;
	    case   7:  send_to_char( "Reality is slipping away.\n\r", ch );	break;
	    case   8:  send_to_char( "You have no idea what is real, and what is not.\n\r", ch );	break;
	    case   9:  send_to_char( "You feel immortal.\n\r", ch );	break;
	    case  10:  send_to_char( "You are an Implementor.\n\r", ch ); break;
	}
    else
    if ( ch->mental_state >45 )
	send_to_char( "Your sleep is filled with strange and vivid dreams.\n\r", ch );
    else
    if ( ch->mental_state >25 )
	send_to_char( "Your sleep is uneasy.\n\r", ch );
    else
    if ( ch->mental_state <-35 )
	send_to_char( "You are deep in a much needed sleep.\n\r", ch );
    else
    if ( ch->mental_state <-25 )
	send_to_char( "You are in deep slumber.\n\r", ch );
    send_to_char("Languages: ", ch );
    for ( iLang = 0; lang_array[iLang] != LANG_UNKNOWN; iLang++ )
	if ( knows_language( ch, lang_array[iLang], ch )
	||  (IS_NPC(ch) && ch->speaks == 0) )
	{
	    if ( lang_array[iLang] & ch->speaking
	    ||  (IS_NPC(ch) && !ch->speaking) )
		set_char_color( AT_RED, ch );
	    send_to_char( lang_names[iLang], ch );
	    send_to_char( " ", ch );
	    set_char_color( AT_SCORE, ch );
	}
    send_to_char( "\n\r", ch );
    if ( ch->pcdata->bounty > 0 )
	ch_printf( ch, "BOUNTY: %d\n\r", ch->pcdata->bounty );

    if ( ch->pcdata->bestowments && ch->pcdata->bestowments[0] != '\0' )
	ch_printf( ch, "You are bestowed with the command(s): %s.\n\r", 
		ch->pcdata->bestowments );

    if ( CAN_PKILL( ch ) )
    {
	send_to_char( "----------------------------------------------------------------------------\n\r", ch);
	ch_printf(ch, "PKILL DATA:  Pkills (%3.3d)     Illegal Pkills (%3.3d)     Pdeaths (%3.3d)\n\r",
		ch->pcdata->pkills, ch->pcdata->illegal_pk, ch->pcdata->pdeaths );
    }
    if (ch->pcdata->clan && ch->pcdata->clan->clan_type != CLAN_ORDER  && ch->pcdata->clan->clan_type != CLAN_GUILD )
    {
	send_to_char( "----------------------------------------------------------------------------\n\r", ch);
	ch_printf(ch, " CLAN STATS: %-8s  Level: %-4d  Clan Pkills: %-4d Clan Pdeaths:  %d\n\r",
		ch->pcdata->clan->name,ch->pcdata->clevel,ch->pcdata->clan->pkills, ch->pcdata->clan->pdeaths) ;
    }
    if (ch->pcdata->deity)
    {
	send_to_char( "----------------------------------------------------------------------------\n\r", ch);
	ch_printf(ch, "Deity:  %-20s  Favor: %d\n\r", ch->pcdata->deity->name, ch->pcdata->favor );
    }
    if (ch->pcdata->clan && ch->pcdata->clan->clan_type == CLAN_ORDER )
    {
        send_to_char( "----------------------------------------------------------------------------\n\r", ch);
	ch_printf(ch, "Order:  %-8s  Level: %-4d   Order Mkills:  %-6d Order MDeaths: %-6d\n\r",
		ch->pcdata->clan->name,ch->pcdata->clevel,ch->pcdata->clan->mkills, ch->pcdata->clan->mdeaths);
    }
    if (ch->pcdata->guild && ch->pcdata->guild->clan_type == CLAN_GUILD )
    {
        send_to_char( "----------------------------------------------------------------------------\n\r", ch);
        ch_printf(ch, "Guild:  %-20s  Level: %d\n\r",
                ch->pcdata->guild->name, ch->pcdata->guildlevel);
    }
    	send_to_char( "----------------------------------------------------------------------------\n\r", ch);
	ch_printf(ch, "CHALLENGE STATS: Wins %d Loses: %d  Championships Won: %d\n",
	          ch->pcdata->awins,ch->pcdata->alosses,ch->pcdata->pkchamps );
    if ( ch->pcdata->gladiator != NULL )
    {
	send_to_char( "----------------------------------------------------------------------------\n\r", ch);
	ch_printf(ch,"GAMBLE STATS: %s  Bet: %d\n",ch->pcdata->gladiator,ch->pcdata->plr_wager);
    } 
	send_to_char( "----------------------------------------------------------------------------\n\r", ch);
	ch_printf(ch, "KOMBAT STATS:   Kombats Entered: %d\n",ch->pcdata->num_kombats );
        ch_printf(ch, "              Kombatants Killed: %d\n",ch->pcdata->num_kombatants);
	ch_printf(ch, "                    Kombats Won: %d\n",ch->pcdata->num_kombats_won );

    if (IS_IMMORTAL(ch))
    {
	send_to_char( "----------------------------------------------------------------------------\n\r", ch);

	ch_printf(ch, "IMMORTAL DATA:  Wizinvis [%s]  Wizlevel (%d)\n\r",
		IS_SET(ch->act, PLR_WIZINVIS) ? "X" : " ", ch->pcdata->wizinvis );
        ch_printf(ch, "Incog [%s] Incog level (%d)\n\r",
                IS_SET(ch->pcdata->flagstwo, MOREPC_INCOG) ? "X" : " ",
ch->pcdata->incog_level ); /* Incog data added by Callinon */


	ch_printf(ch, "Bamfin:  %s\n\r",(ch->pcdata->bamfin[0] != '\0')
		? ch->pcdata->bamfin : "?? appears in a swirling mist.");
	ch_printf(ch, "Bamfout: %s\n\r",(ch->pcdata->bamfout[0] != '\0')
		? ch->pcdata->bamfout : "?? leaves in a swirling mist.");
	ch_printf(ch, "Creation Security: %d\n\r",ch->pcdata->cperm );
	
	/* Area Loaded info - Scryn 8/11*/
	if (ch->pcdata->area)
	{
	    ch_printf(ch, "Vnums:   Room (%-5.5d - %-5.5d)   Object (%-5.5d - %-5.5d)   Mob (%-5.5d - %-5.5d)\n\r",
		ch->pcdata->area->low_r_vnum, ch->pcdata->area->hi_r_vnum,
		ch->pcdata->area->low_o_vnum, ch->pcdata->area->hi_o_vnum,
		ch->pcdata->area->low_m_vnum, ch->pcdata->area->hi_m_vnum);
	    ch_printf(ch, "Area Loaded [%s]\n\r", (IS_SET(ch->pcdata->area->status, AREA_LOADED)) ? "yes" : "no");
	}
    }
    if (ch->first_affect)
    {
	int i;
	SKILLTYPE *sktmp;

	i = 0;
	send_to_char( "----------------------------------------------------------------------------\n\r", ch);
	send_to_char("AFFECT DATA:                            ", ch);
	for (paf = ch->first_affect; paf; paf = paf->next)
	{
	    if ( (sktmp=get_skilltype(paf->type)) == NULL )
		continue;
	    if (ch->level < 20)
	    {
		ch_printf(ch, "[%-34.34s]    ", sktmp->name);
		if (i == 0)
		   i = 2;
		if ((++i % 3) == 0)
		   send_to_char("\n\r", ch);
	     }
	     if (ch->level >= 20)
	     {
		if (paf->modifier == 0)
		    ch_printf(ch, "[%-24.24s;%5d rds]    ",
			sktmp->name,
			paf->duration);
		else
		if (paf->modifier > 999)
		    ch_printf(ch, "[%-15.15s; %7.7s;%5d rds]    ",
			sktmp->name,
			tiny_affect_loc_name(paf->location),
			paf->duration);
		else
		    ch_printf(ch, "[%-11.11s;%+-3.3d %7.7s;%5d rds]    ",
			sktmp->name,
			paf->modifier,
			tiny_affect_loc_name(paf->location),
			paf->duration);
		if (i == 0)
		    i = 1;
		if ((++i % 2) == 0)
		    send_to_char("\n\r", ch);
	    }
	}
    }
    send_to_char("\n\r", ch);
    return;
}

/*
 * Reroll a Character
 */
void reroll_char( CHAR_DATA *ch )
{

/*
advanced = FALSE;
dual = FALSE;
single = FALSE;

ch->max_hit = 10;
ch->max_mana = 10;
ch->max_move = 10;
ch->exp = 0;

if ( !IS_ADVANCED(ch) && ch->class2 == -1 )
{
ch->level = 2;
single = TRUE;
}
else if ( !IS_ADVANCED(ch) && ch->class2 > -1 )
{
ch->level = 2;
ch->level2 = 1;
dual = TRUE;
}
else
{
ch->advlevel = 1;
advanced = TRUE;
}

if ( single )
{
for ( cnt = 1; cnt < ch->level; cnt++ )
advance_level(ch,);
}
if ( dual )
{
for ( cnt = 1; cnt < ch->level; cnt++ )
advance_level(ch);

for ( cnt = 1; cnt < ch->level2; cnt++ )
advance_level2(ch);
}
if ( advanced )
{
for ( cnt = 1; cnt < ch->level; cnt++ )
advance_level(ch);

for ( cnt = 1; cnt < ch->level2; cnt++ )
advance_level2(ch);

for ( cnt = 1; cnt < ch->advlevel; cnt++ )
advance_level(ch);
}
*/ 
return;
}

/*
 * Return ascii name of an affect location.
 */
char           *
tiny_affect_loc_name(int location)
{
	switch (location) {
	case APPLY_NONE:		return "NIL";
	case APPLY_STR:			return " STR  ";
	case APPLY_DEX:			return " DEX  ";
	case APPLY_INT:			return " INT  ";
	case APPLY_WIS:			return " WIS  ";
	case APPLY_CON:			return " CON  ";
	case APPLY_CHA:			return " CHA  ";
	case APPLY_LCK:			return " LCK  ";
	case APPLY_SEX:			return " SEX  ";
	case APPLY_CLASS:		return " CLASS";
	case APPLY_LEVEL:		return " LVL  ";
	case APPLY_AGE:			return " AGE  ";
	case APPLY_MANA:		return " MANA ";
	case APPLY_HIT:			return " HV   ";
	case APPLY_MOVE:		return " MOVE ";
	case APPLY_GOLD:		return " GOLD ";
	case APPLY_EXP:			return " EXP  ";
	case APPLY_AC:			return " AC   ";
	case APPLY_HITROLL:		return " HITRL";
	case APPLY_EGO:			return " EGO  ";
	case APPLY_DAMROLL:		return " DAMRL";
	case APPLY_SAVING_POISON:	return "SV POI";
	case APPLY_SAVING_ROD:		return "SV ROD";
	case APPLY_SAVING_PARA:		return "SV PARA";
	case APPLY_SAVING_BREATH:	return "SV BRTH";
	case APPLY_SAVING_SPELL:	return "SV SPLL";
	case APPLY_HEIGHT:		return "HEIGHT";
	case APPLY_WEIGHT:		return "WEIGHT";
	case APPLY_AFFECT:		return "AFF BY";
	case APPLY_RESISTANT:		return "RESIST";
	case APPLY_IMMUNE:		return "IMMUNE";
	case APPLY_SUSCEPTIBLE:		return "SUSCEPT";
	case APPLY_WEAPONSPELL:		return " WEAPON";
	case APPLY_BACKSTAB:		return "BACKSTB";
	case APPLY_PICK:		return " PICK  ";
	case APPLY_TRACK:		return " TRACK ";
	case APPLY_STEAL:		return " STEAL ";
	case APPLY_SNEAK:		return " SNEAK ";
	case APPLY_HIDE:		return " HIDE  ";
	case APPLY_PALM:		return " PALM  ";
	case APPLY_DETRAP:		return " DETRAP";
	case APPLY_DODGE:		return " DODGE ";
	case APPLY_PEEK:		return " PEEK  ";
	case APPLY_SCAN:		return " SCAN  ";
	case APPLY_GOUGE:		return " GOUGE ";
	case APPLY_SEARCH:		return " SEARCH";
	case APPLY_MOUNT:		return " MOUNT ";
	case APPLY_DISARM:		return " DISARM";
	case APPLY_KICK:		return " KICK  ";
	case APPLY_PARRY:		return " PARRY ";
	case APPLY_BASH:		return " BASH  ";
	case APPLY_STUN:		return " STUN  ";
	case APPLY_PUNCH:		return " PUNCH ";
	case APPLY_CLIMB:		return " CLIMB ";
	case APPLY_GRIP:		return " GRIP  ";
	case APPLY_SCRIBE:		return " SCRIBE";
	case APPLY_BREW:		return " BREW  ";
	case APPLY_WEARSPELL:		return " WEAR  ";
	case APPLY_REMOVESPELL:		return " REMOVE";
	case APPLY_EMOTION:		return "EMOTION";
	case APPLY_MENTALSTATE:		return " MENTAL";
	case APPLY_STRIPSN:		return " DISPEL";
	case APPLY_REMOVE:		return " REMOVE";
	case APPLY_DIG:			return " DIG   ";
	case APPLY_FULL:		return " HUNGER";
	case APPLY_THIRST:		return " THIRST";
	case APPLY_DRUNK:		return " DRUNK ";
	case APPLY_BLOOD:		return " BLOOD ";
	case APPLY_ALIGN:		return " ALIGN ";
	case APPLY_ROUND_STUN:		return " STUN ";
	}

	bug("Affect_location_name: unknown location %d.", location);
	return "()";
}

char * get_class(CHAR_DATA *ch)
{
    if ( ch->class < MAX_NPC_CLASS && ch->class >= 0)
	return ( npc_class[ch->class] );
    return ("Unknown");
}
char * get_class2(CHAR_DATA *ch)
{
	if ( ch->class2 == -1 )
	return ("None");
    if ( ch->class2 < MAX_NPC_CLASS && ch->class2 >= 0)
	return ( npc_class[ch->class2] );
    return ("Unknown");
}

char * get_advclass(CHAR_DATA *ch)
{
	if ( ch->advclass < 1 )
	return ("None");
    if ( ch->advclass < MAX_NPC_CLASS && ch->advclass >= 0)
	return ( npc_class[ch->advclass] );
    return ("Unknown");
}

char * get_advclass2(CHAR_DATA *ch)
{
	if ( ch->advclass2 < 1 )
	return ("None");
    if ( ch->advclass2 < MAX_NPC_CLASS && ch->advclass2 >= 0)
	return ( npc_class[ch->advclass2] );
    return ("Unknown");
}


char *
get_race( CHAR_DATA *ch)
{
    if ( ch->race < MAX_NPC_RACE && ch->race >= 0)
	return ( npc_race[ch->race] );
    return ("Unknown");
}

void do_oldscore( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA *paf;
    SKILLTYPE   *skill;

    if ( IS_AFFECTED(ch, AFF_POSSESS) )
    {   
       send_to_char("You can't do that in your current state of mind!\n\r", ch);
       return;
    }

    set_char_color( AT_SCORE, ch );
    ch_printf( ch,
	"You are %s, level %d, %d years old (%d hours).\n\r",
	IS_NPC(ch) ? "" : ch->pcdata->title,
	ch->level,
	get_age(ch),
	(get_age(ch) - 17) * 2 );

    if ( get_trust( ch ) != ch->level )
	ch_printf( ch, "You are trusted at level %d.\n\r",
	    get_trust( ch ) );

    if ( IS_SET(ch->act, ACT_MOBINVIS) )
      ch_printf( ch, "You are mobinvis at level %d.\n\r",
            ch->mobinvis);

    if ( IS_VAMPIRE(ch) )
      ch_printf( ch,
	"You have %d/%d hit, %d/%d blood level, %d/%d movement, %d practices.\n\r",
	ch->hit,  ch->max_hit,
	ch->pcdata->condition[COND_BLOODTHIRST], get_bloodthirst(ch),
	ch->move, ch->max_move,
	ch->practice );
    else
      ch_printf( ch,
	"You have %d/%d hit, %d/%d mana, %d/%d movement, %d practices.\n\r",
	ch->hit,  ch->max_hit,
	ch->mana, ch->max_mana,
	ch->move, ch->max_move,
	ch->practice );

    ch_printf( ch,
	"You are carrying %d/%d items with weight %d/%d kg.\n\r",
	ch->carry_number, can_carry_n(ch),
	ch->carry_weight, can_carry_w(ch) );

/*  if ( ch->level >= 5 ) */
    ch_printf( ch,
	"Str: %d  Int: %d  Wis: %d  Dex: %d  Con: %d  Cha: %d  Lck: %d.\n\r",
	get_curr_str(ch),
	get_curr_int(ch),
	get_curr_wis(ch),
	get_curr_dex(ch),
	get_curr_con(ch),
	get_curr_cha(ch),
	get_curr_lck(ch) );

    ch_printf( ch,
	"You have scored %d exp, and have %d gold coins.\n\r",
	ch->exp,  ch->gold );

    if ( !IS_NPC(ch) )
    ch_printf( ch,
	"You have achieved %d glory during your life, and currently have %d.\n\r",
	ch->pcdata->quest_accum, ch->pcdata->quest_curr );

    ch_printf( ch,
	"Autoexit: %s   Autoloot: %s   Autosac: %s   Autogold: %s\n\r",
	(!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT)) ? "yes" : "no",
	(!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOLOOT)) ? "yes" : "no",
	(!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOSAC) ) ? "yes" : "no",
  	(!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOGOLD)) ? "yes" : "no" );

    ch_printf( ch, "Wimpy set to %d hit points.\n\r", ch->wimpy );

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]   > 10 )
	send_to_char( "You are drunk.\n\r",   ch );
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] ==  0 )
	send_to_char( "You are thirsty.\n\r", ch );
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL]   ==  0 )
	send_to_char( "You are hungry.\n\r",  ch );

    switch( ch->mental_state / 10 )
    {
        default:   send_to_char( "You're completely messed up!\n\r", ch ); break;
        case -10:  send_to_char( "You're barely conscious.\n\r", ch ); break;
        case  -9:  send_to_char( "You can barely keep your eyes open.\n\r", ch ); break;
        case  -8:  send_to_char( "You're extremely drowsy.\n\r", ch ); break;
        case  -7:  send_to_char( "You feel very unmotivated.\n\r", ch ); break;
        case  -6:  send_to_char( "You feel sedated.\n\r", ch ); break;
        case  -5:  send_to_char( "You feel sleepy.\n\r", ch ); break;
        case  -4:  send_to_char( "You feel tired.\n\r", ch ); break;
        case  -3:  send_to_char( "You could use a rest.\n\r", ch ); break;
        case  -2:  send_to_char( "You feel a little under the weather.\n\r", ch ); break;
        case  -1:  send_to_char( "You feel fine.\n\r", ch ); break;
        case   0:  send_to_char( "You feel great.\n\r", ch ); break;
        case   1:  send_to_char( "You feel energetic.\n\r", ch ); break;
        case   2:  send_to_char( "Your mind is racing.\n\r", ch ); break;
        case   3:  send_to_char( "You can't think straight.\n\r", ch ); break;
        case   4:  send_to_char( "Your mind is going 100 miles an hour.\n\r", ch ); break;
        case   5:  send_to_char( "You're high as a kite.\n\r", ch ); break;
        case   6:  send_to_char( "Your mind and body are slipping appart.\n\r", ch ); break;
        case   7:  send_to_char( "Reality is slipping away.\n\r", ch ); break;
        case   8:  send_to_char( "You have no idea what is real, and what is not.\n\r", ch ); break;
        case   9:  send_to_char( "You feel immortal.\n\r", ch ); break;
        case  10:  send_to_char( "You are an Implementor.\n\r", ch ); break;
    }

    switch ( ch->position )
    {
    case POS_DEAD:
	send_to_char( "You are DEAD!!\n\r",		ch );
	break;
    case POS_MORTAL:
	send_to_char( "You are mortally wounded.\n\r",	ch );
	break;
    case POS_INCAP:
	send_to_char( "You are incapacitated.\n\r",	ch );
	break;
    case POS_STUNNED:
	send_to_char( "You are stunned.\n\r",		ch );
	break;
    case POS_SLEEPING:
	send_to_char( "You are sleeping.\n\r",		ch );
	break;
    case POS_RESTING:
	send_to_char( "You are resting.\n\r",		ch );
	break;
    case POS_STANDING:
	send_to_char( "You are standing.\n\r",		ch );
	break;
    case POS_FIGHTING:
	send_to_char( "You are fighting.\n\r",		ch );
	break;
    case POS_MOUNTED:
	send_to_char( "Mounted.\n\r",			ch );
	break;
    case POS_SHOVE:
	send_to_char( "Being shoved.\n\r",		ch );
	break;
    case POS_DRAG:
	send_to_char( "Being dragged.\n\r",		ch );
	break;
    }

    if ( ch->level >= 25 )
	ch_printf( ch, "AC: %d.  ", GET_AC(ch) );

    send_to_char( "You are ", ch );
	 if ( GET_AC(ch) >=  101 ) send_to_char( "WORSE than naked!\n\r", ch );
    else if ( GET_AC(ch) >=   80 ) send_to_char( "naked.\n\r",            ch );
    else if ( GET_AC(ch) >=   60 ) send_to_char( "wearing clothes.\n\r",  ch );
    else if ( GET_AC(ch) >=   40 ) send_to_char( "slightly armored.\n\r", ch );
    else if ( GET_AC(ch) >=   20 ) send_to_char( "somewhat armored.\n\r", ch );
    else if ( GET_AC(ch) >=    0 ) send_to_char( "armored.\n\r",          ch );
    else if ( GET_AC(ch) >= - 20 ) send_to_char( "well armored.\n\r",     ch );
    else if ( GET_AC(ch) >= - 40 ) send_to_char( "strongly armored.\n\r", ch );
    else if ( GET_AC(ch) >= - 60 ) send_to_char( "heavily armored.\n\r",  ch );
    else if ( GET_AC(ch) >= - 80 ) send_to_char( "superbly armored.\n\r", ch );
    else if ( GET_AC(ch) >= -100 ) send_to_char( "divinely armored.\n\r", ch );
    else                           send_to_char( "invincible!\n\r",       ch );

    if ( ch->level >= 15
    ||   IS_PKILL( ch ) )
	ch_printf( ch, "Hitroll: %d  Damroll: %d.\n\r",
	    GET_HITROLL(ch), GET_DAMROLL(ch) );

    if ( ch->level >= 10 )
	ch_printf( ch, "Alignment: %d.  ", ch->alignment );

    send_to_char( "You are ", ch );
	 if ( ch->alignment >  900 ) send_to_char( "angelic.\n\r", ch );
    else if ( ch->alignment >  700 ) send_to_char( "saintly.\n\r", ch );
    else if ( ch->alignment >  350 ) send_to_char( "good.\n\r",    ch );
    else if ( ch->alignment >  100 ) send_to_char( "kind.\n\r",    ch );
    else if ( ch->alignment > -100 ) send_to_char( "neutral.\n\r", ch );
    else if ( ch->alignment > -350 ) send_to_char( "mean.\n\r",    ch );
    else if ( ch->alignment > -700 ) send_to_char( "evil.\n\r",    ch );
    else if ( ch->alignment > -900 ) send_to_char( "demonic.\n\r", ch );
    else                             send_to_char( "satanic.\n\r", ch );

    if ( ch->first_affect )
    {
	send_to_char( "You are affected by:\n\r", ch );
	for ( paf = ch->first_affect; paf; paf = paf->next )
	    if ( (skill=get_skilltype(paf->type)) != NULL )
	{
	    ch_printf( ch, "Spell: '%s'", skill->name );

	    if ( ch->level >= 20 )
		ch_printf( ch,
		    " modifies %s by %d for %d rounds",
		    affect_loc_name( paf->location ),
		    paf->modifier,
		    paf->duration );

	    send_to_char( ".\n\r", ch );
	}
    }

    if ( !IS_NPC( ch ) && IS_IMMORTAL( ch ) )
    {
	ch_printf( ch, "WizInvis level: %d   WizInvis is %s\n\r",
			ch->pcdata->wizinvis,
			IS_SET( ch->act, PLR_WIZINVIS ) ? "ON" : "OFF" );
	if ( ch->pcdata->r_range_lo && ch->pcdata->r_range_hi )
	  ch_printf( ch, "Room Range: %d - %d\n\r", ch->pcdata->r_range_lo,
					 	   ch->pcdata->r_range_hi	);
	if ( ch->pcdata->o_range_lo && ch->pcdata->o_range_hi )
	  ch_printf( ch, "Obj Range : %d - %d\n\r", ch->pcdata->o_range_lo,
	  					   ch->pcdata->o_range_hi	);
	if ( ch->pcdata->m_range_lo && ch->pcdata->m_range_hi )
	  ch_printf( ch, "Mob Range : %d - %d\n\r", ch->pcdata->m_range_lo,
	  					   ch->pcdata->m_range_hi	);
    }

    return;
}

/*								-Thoric
 * Display your current exp, level, and surrounding level exp requirements
 */
void do_level( CHAR_DATA *ch, char *argument )
{
    char buf [MAX_STRING_LENGTH];
    int x, lowlvl=0, hilvl=0;

   if (!IS_ADVANCED(ch))
	{
    if ( ch->level == 1 )
      lowlvl = 1;
    else
      lowlvl = UMAX( 2, ch->level - 5 );
    hilvl = URANGE( ch->level, ch->level + 5, MAX_LEVEL );
	}

	if (IS_ADVANCED(ch))
	{
    if ( ch->level == 1 )
      lowlvl = 1;
    else
      lowlvl = UMAX( 2, ch->advlevel - 5 );
    hilvl = URANGE( ch->advlevel, ch->advlevel + 5, MAX_LEVEL );
	}
    set_char_color( AT_SCORE, ch );
    ch_printf( ch, "Experience required levels %d to %d:\n\r", lowlvl, hilvl );
    sprintf( buf, " exp (You have %ld)", ch->exp );
    for ( x = lowlvl; x <= hilvl; x++ )
	ch_printf( ch, " %3d) %11d%s\n\r", x, exp_level( ch, x ),
		(x == ch->level) ? buf : " exp" );
}
void do_level2( CHAR_DATA *ch, char *argument )
{
    char buf [MAX_STRING_LENGTH];
    int x, lowlvl, hilvl;
    x = 0;
    hilvl = 0;
    lowlvl = 0;

   if( ch->class2 == -1 )
	return;
    if ( ch->level2 == 1 )
      lowlvl = 1;
    else
      lowlvl = UMAX( 2, ch->level2 - 5 );
    hilvl = URANGE( ch->level2, ch->level2 + 5, MAX_LEVEL );
    set_char_color( AT_SCORE, ch );
    ch_printf( ch, "Experience required levels %d to %d:\n\r", lowlvl, hilvl );
    sprintf( buf, " exp (You have %ld)", ch->exp );
    for ( x = lowlvl; x <= hilvl; x++ )
	ch_printf( ch, " %3d) %11d%s\n\r", x, exp_level2( ch, x ),
		(x == ch->level2) ? buf : " exp" );
}


void do_affected ( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    SKILLTYPE *skill;
 
    if ( IS_NPC(ch) )
        return;

    argument = one_argument( argument, arg );

    if ( !ch->first_affect )
    {
        set_char_color( AT_SCORE, ch );
        send_to_char( "\n\rNo cantrip or skill affects you.\n\r", ch );
    }
    else
    {
	send_to_char( "\n\r", ch );
        for (paf = ch->first_affect; paf; paf = paf->next)
	    if ( (skill=get_skilltype(paf->type)) != NULL )
        {
            set_char_color( AT_BLUE, ch );
            send_to_char( "Affected:  ", ch );
            set_char_color( AT_SCORE, ch );
                if (paf->duration < 25 ) set_char_color( AT_WHITE, ch );
                if (paf->duration < 6  ) set_char_color( AT_WHITE + AT_BLINK, ch );
                ch_printf( ch, "(%5d)   ", paf->duration );
            ch_printf( ch, "%-18s\n\r", skill->name );
        }
    }

        set_char_color( AT_BLUE, ch );
        send_to_char( "\n\rEQUIPMENT AFFECTS:\n\r", ch );
	set_char_color( AT_SCORE, ch );
	ch_printf( ch, "%s\n\r", affect_bit_name( &ch->affected_by ) );

            send_to_char( "\n\r", ch );
            if ( ch->resistant > 0 )
	    {
		set_char_color ( AT_BLUE, ch );
                send_to_char( "RESISTANTCIES:  ", ch );
                set_char_color( AT_SCORE, ch );
                ch_printf( ch, "%s\n\r", flag_string(ch->resistant, ris_flags) );
	    }
            if ( ch->immune > 0 )
	    {
                set_char_color( AT_BLUE, ch );
                send_to_char( "IMMUNITIES:   ", ch);
                set_char_color( AT_SCORE, ch );
                ch_printf( ch, "%s\n\r", flag_string(ch->immune, ris_flags) );
	    }
            if ( ch->susceptible > 0 )
	    {
                set_char_color( AT_BLUE, ch );
                send_to_char( "SUSCEPTS:     ", ch );
		set_char_color( AT_SCORE, ch );
                ch_printf( ch, "%s\n\r", flag_string(ch->susceptible, ris_flags) );
	    }
    return;
}

void do_inventory( CHAR_DATA *ch, char *argument )
{
    set_char_color( AT_RED, ch );
    send_to_char( "You are carrying:\n\r", ch );
    show_list_to_char( ch->first_carrying, ch, TRUE, TRUE );
    return;
}


void do_equipment( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    int iWear;
    bool found;

    set_char_color( AT_RED, ch );
    send_to_char( "You are using:\n\r", ch );
    found = FALSE;
    set_char_color( AT_GREY, ch );
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	for ( obj = ch->first_carrying; obj; obj = obj->next_content )
	   if ( obj->wear_loc == iWear )
	   {
		send_to_char( where_name[iWear], ch );
		ch_printf(ch,"%s &W(&R%-10s&W)&w",obj->value[4] > 0 ? "&RACIDED&W" : "",
			get_damage_value_obj(obj) );
		if ( can_see_obj( ch, obj ) )
		{
		    send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
		    send_to_char( "\n\r", ch );
		}
		else
		    send_to_char( "something.\n\r", ch );
		found = TRUE;
	   }
    }

    if ( !found )
	send_to_char( "Nothing.\n\r", ch );

    return;
}



void set_title( CHAR_DATA *ch, char *title )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
    {
	bug( "Set_title: NPC.", 0 );
	return;
    }

    if ( isalpha(title[0]) || isdigit(title[0]) )
    {
	buf[0] = ' ';
	strcpy( buf+1, title );
    }
    else
	strcpy( buf, title );

    STRFREE( ch->pcdata->title );
    ch->pcdata->title = STRALLOC( buf );
    return;
}



  void do_title( CHAR_DATA *ch, char *argument )
  {
     char title_buf[MAX_STRING_LENGTH];
     int str = 0;
     bool is_ok;
     char name[MSL];
     bool crasher;

     is_ok = FALSE;
     name[0]='\0';
     title_buf[0] = '\0';

      if ( IS_NPC(ch) )
  	return;
 
      if ( ch->level < 4 )
      {
  	send_to_char( "Sorry... you must be at least level 4 to do that.\n\r", ch );
  	return;
      }
  
     if ( IS_SET( ch->pcdata->flags, PCFLAG_NOTITLE ))
     {
         send_to_char( "The Gods prohibit you from changing your title.\n\r", ch );
         return;
     }

        if ( IS_SET( ch->pcdata->flagstwo, MOREPC_NAME_SET ) )
	sprintf(name,"%s %s",ch->pcdata->name,QUICKLINK(ch->name));
  	else
	sprintf(name,"%s",QUICKLINK(ch->name));


      if ( argument[0] == '\0' )
      {
         sprintf( title_buf, "%s the lost %s of the Ancients.", name, capitalize(npc_class[ch->class]));
         set_title( ch, title_buf );
   	send_to_char( "\n\rOriginal title Reset.\n\r", ch );
  	return;
      }
  
    if ( strlen(argument) > 70 )
	argument[70] = '\0';


    /* fix the % in title crash bug.. GW */
    crasher = FALSE;
    for ( str = 0; argument[str] != '\0'; str++ )
    {
    if ( argument[str] == '%' )
    {
       crasher = TRUE;
       break;
    }
    }

    if ( crasher == TRUE )
    {
	send_to_char("Titles cannot contian the '%' symbol.\n\r",ch);
        return;
    }

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

    if( is_ok == TRUE )
    {

    sprintf( title_buf,argument,name);

       smash_tilde( title_buf );
       set_title( ch, title_buf );
       send_to_char( "Ok.\n\r", ch );
   }

    if ( is_ok == FALSE && ch->level < 59 )
    {
    send_to_char("You must have an @@ sign in your title, indicating your name.\n\r",ch);
    }

    if ( is_ok == FALSE && ch->level == 59 )
    {
           smash_tilde( argument );
       set_title( ch, argument );
       send_to_char( "Ok.\n\r", ch );
    }

    return;
}


void do_homepage( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
	return;

    if ( ch->level < 5 )
    {
	send_to_char( "Sorry... you must be at least level 5 to do that.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	if ( !ch->pcdata->homepage )
	  ch->pcdata->homepage = str_dup( "" );
	ch_printf( ch, "Your homepage is: %s\n\r",
		show_tilde( ch->pcdata->homepage ) );
	return;
    }

    if ( !str_cmp( argument, "clear" ) )
    {
	if ( ch->pcdata->homepage )
	  DISPOSE(ch->pcdata->homepage);
	ch->pcdata->homepage = str_dup("");
	send_to_char( "Homepage cleared.\n\r", ch );
	return;
    }

    if ( strstr( argument, "://" ) )
	strcpy( buf, argument );
    else
	sprintf( buf, "http://%s", argument );
    if ( strlen(buf) > 70 )
	buf[70] = '\0';

    hide_tilde( buf );
    if ( ch->pcdata->homepage )
      DISPOSE(ch->pcdata->homepage);
    ch->pcdata->homepage = str_dup(buf);
    send_to_char( "Homepage set.\n\r", ch );
}



/*
 * Set your personal description				-Thoric
 */
void do_description( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC( ch ) )
    {
	send_to_char( "Monsters are too dumb to do that!\n\r", ch );
	return;	  
    }

    if ( !ch->desc )
    {
	bug( "do_description: no descriptor", 0 );
	return;
    }

    switch( ch->substate )
    {
	default:
	   bug( "do_description: illegal substate", 0 );
	   return;

	case SUB_RESTRICTED:
	   send_to_char( "You cannot use this command from within another command.\n\r", ch );
	   return;

	case SUB_NONE:
	   ch->substate = SUB_PERSONAL_DESC;
	   ch->dest_buf = ch;
	   start_editing( ch, ch->description );
	   return;

	case SUB_PERSONAL_DESC:
	   STRFREE( ch->description );
	   ch->description = copy_buffer( ch );
	   stop_editing( ch );
	   return;	
    }
}

/* Ripped off do_description for whois bio's -- Scryn*/
void do_bio( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC( ch ) )
    {
	send_to_char( "Mobs can't set bio's!\n\r", ch );
	return;	  
    }

    if ( !ch->desc )
    {
	bug( "do_bio: no descriptor", 0 );
	return;
    }

    switch( ch->substate )
    {
	default:
	   bug( "do_bio: illegal substate", 0 );
	   return;
	  	   
	case SUB_RESTRICTED:
	   send_to_char( "You cannot use this command from within another command.\n\r", ch );
	   return;

	case SUB_NONE:
	   ch->substate = SUB_PERSONAL_BIO;
	   ch->dest_buf = ch;
	   start_editing( ch, ch->pcdata->bio );
	   return;

	case SUB_PERSONAL_BIO:
	   STRFREE( ch->pcdata->bio );
	   ch->pcdata->bio = copy_buffer( ch );
	   stop_editing( ch );
	   return;	
    }
}



void do_report( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];

    if ( IS_AFFECTED(ch, AFF_POSSESS) )
    {   
       send_to_char("You can't do that in your current state of mind!\n\r", ch);
       return;
    }

    if ( IS_VAMPIRE(ch) )
      ch_printf( ch,
	"You report: %d/%d hp %d/%d blood %d/%d mv %d xp.\n\r",
	ch->hit,  ch->max_hit,
	ch->pcdata->condition[COND_BLOODTHIRST], get_bloodthirst(ch),
	ch->move, ch->max_move,
	ch->exp   );
    else
      ch_printf( ch,
	"You report: %d/%d hp %d/%d mana %d/%d mv %d xp.\n\r",
	ch->hit,  ch->max_hit,
	ch->mana, ch->max_mana,
	ch->move, ch->max_move,
	ch->exp   );

    if ( IS_VAMPIRE(ch) )
      sprintf( buf, "$n reports: %d/%d hp %d/%d blood %d/%d mv %ld xp.\n\r",
	ch->hit,  ch->max_hit,
	ch->pcdata->condition[COND_BLOODTHIRST], get_bloodthirst(ch),
	ch->move, ch->max_move,
	ch->exp   );
    else
      sprintf( buf, "$n reports: %d/%d hp %d/%d mana %d/%d mv %ld xp.",
	ch->hit,  ch->max_hit,
	ch->mana, ch->max_mana,
	ch->move, ch->max_move,
	ch->exp   );

    act( AT_REPORT, buf, ch, NULL, NULL, TO_ROOM );

    return;
}

void do_prompt( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  
  if ( IS_NPC(ch) )
  {
    send_to_char( "NPC's can't change their prompt..\n\r", ch );
    return;
  }
  smash_tilde( argument );
  one_argument( argument, arg );
  if ( !*arg )
  {
    send_to_char( "Set prompt to what? (try help prompt)\n\r", ch );
    return;
  }
  if (ch->pcdata->prompt)
    STRFREE(ch->pcdata->prompt);

  if ( strlen(argument) > 128 )
    argument[128] = '\0';

  /* Can add a list of pre-set prompts here if wanted.. perhaps
     'prompt 1' brings up a different, pre-set prompt */
  if ( !str_cmp(arg, "default") )
    ch->pcdata->prompt = STRALLOC("");
  else
    ch->pcdata->prompt = STRALLOC(argument);
  send_to_char( "Ok.\n\r", ch );
  return;
}

/*
* Ok I rewrote alotof this section for Magical AC stuff.. so these comments
* are all way off .. still leave it alone please --GW
*/
/* AC, HITROLL, and DAMROLL Stuff --GW */
/* Addition: Made it so that Hitroll/Damroll Max is 200, AC Max is -1000 --GW*/
/* Changed Aug. 06/98,
   Non-Adv Limits: -1000ac/150 Dam/ 150 Hit
   Adv (single Class): -2000ac/250 Dam/ 250 Hit
   Adv Dual: -2500 Ac/300 Dam/ 300 Hit

Leave it ALONE!

--GW
*/
/*
 * Get a chars AC .. replaces the #define in mud.h .. to complex for
 * a header file now --GW
 */
int GET_AC( CHAR_DATA *ch )
{
return (UMAX((GET_MAC(ch) + GET_EQAC(ch)), -3000 ));
}

/*
 * Get a Chars Magical AC
 */
int GET_MAC( CHAR_DATA *ch )
{
int bonus =0;

if ( IS_NPC(ch) )
bonus -= (ch->level * 1);

/*Pure Fighters*/
if ( IS_NPC(ch) || get_ac_type_char(ch) == AC_TYPE_PURE_FIGHTER )
{
  if ( !IS_NPC(ch) && !IS_ADVANCED(ch) )
    return UMAX((ch->magical_armor + ( IS_AWAKE(ch) ? dex_app[get_curr_dex(ch)].defensive : 0 ) + VAMP_AC(ch)),-150) - add_share_mac(ch)  + (!IS_NPC(ch) ? ch->pcdata->ac_bonus : 0);
  if ( !IS_NPC(ch) && IS_ADVANCED(ch) && !IS_ADV_DUAL(ch) ) 
    return UMAX((ch->magical_armor + ( IS_AWAKE(ch) ? dex_app[get_curr_dex(ch)].defensive : 0 ) + VAMP_AC(ch)), -300) - add_share_mac(ch) + (!IS_NPC(ch) ? ch->pcdata->ac_bonus : 0);
  if ( IS_NPC(ch) )
    return (UMAX((ch->magical_armor + ( IS_AWAKE(ch) ? dex_app[get_curr_dex(ch)].defensive : 0 ) + VAMP_AC(ch)),-600) - add_share_mac(ch) + bonus);
  if ( IS_ADV_DUAL(ch) )
    return (UMAX((ch->magical_armor + ( IS_AWAKE(ch) ? dex_app[get_curr_dex(ch)].defensive : 0 ) + VAMP_AC(ch)),-600) - add_share_mac(ch) + bonus) + (!IS_NPC(ch) ? ch->pcdata->ac_bonus : 0);
}

/* Halfers */
if ( get_ac_type_char(ch) == AC_TYPE_HALFLING )
{
  if ( !IS_ADVANCED(ch) )
    return UMAX((ch->magical_armor + ( IS_AWAKE(ch) ? dex_app[get_curr_dex(ch)].defensive : 0 ) + VAMP_AC(ch)),-225) - add_share_mac(ch) + (!IS_NPC(ch) ? ch->pcdata->ac_bonus : 0);
  if ( IS_ADVANCED(ch) && !IS_ADV_DUAL(ch) ) 
    return UMAX((ch->magical_armor + ( IS_AWAKE(ch) ? dex_app[get_curr_dex(ch)].defensive : 0 ) + VAMP_AC(ch)), -400) - add_share_mac(ch)  + (!IS_NPC(ch) ? ch->pcdata->ac_bonus : 0);
  if ( IS_ADV_DUAL(ch) )
    return UMAX((ch->magical_armor + ( IS_AWAKE(ch) ? dex_app[get_curr_dex(ch)].defensive : 0 ) + VAMP_AC(ch)),-700) - add_share_mac(ch)  + (!IS_NPC(ch) ? ch->pcdata->ac_bonus : 0);
 }

/* Pure Magic Users */
if ( get_ac_type_char(ch) == AC_TYPE_PURE_MAGIC_USER )
{
  if ( !IS_ADVANCED(ch) )
    return UMAX((ch->magical_armor + ( IS_AWAKE(ch) ? dex_app[get_curr_dex(ch)].defensive : 0 ) + VAMP_AC(ch)),-500) - add_share_mac(ch)  + (!IS_NPC(ch) ? ch->pcdata->ac_bonus : 0);
  if ( IS_ADVANCED(ch) && !IS_ADV_DUAL(ch) ) 
    return UMAX((ch->magical_armor + ( IS_AWAKE(ch) ? dex_app[get_curr_dex(ch)].defensive : 0 ) + VAMP_AC(ch)), -1000) - add_share_mac(ch)  + (!IS_NPC(ch) ? ch->pcdata->ac_bonus : 0);
  if ( IS_ADV_DUAL(ch) )
    return UMAX((ch->magical_armor + ( IS_AWAKE(ch) ? dex_app[get_curr_dex(ch)].defensive : 0 ) + VAMP_AC(ch)),-1500) - add_share_mac(ch)  + (!IS_NPC(ch) ? ch->pcdata->ac_bonus : 0);
 }

return 0;
}

/*
 * Get a Chars EQ ac --GW
 */
int GET_EQAC( CHAR_DATA *ch )
{
int bonus=0;

/* Pure Fighter */
if ( IS_NPC(ch) || get_ac_type_char(ch) == AC_TYPE_PURE_FIGHTER )
{
  if ( !IS_NPC(ch) && !IS_ADVANCED(ch) )
    return UMAX((ch->armor + ( IS_AWAKE(ch) ? dex_app[get_curr_dex(ch)].defensive : 0 ) + VAMP_AC(ch)),-500) - add_share_ac(ch)  + (!IS_NPC(ch) ? ch->pcdata->mac_bonus : 0);
  if ( !IS_NPC(ch) && IS_ADVANCED(ch) && !IS_ADV_DUAL(ch) ) 
    return UMAX((ch->armor + ( IS_AWAKE(ch) ? dex_app[get_curr_dex(ch)].defensive : 0 ) + VAMP_AC(ch)), -1000) - add_share_ac(ch)  + (!IS_NPC(ch) ? ch->pcdata->mac_bonus : 0);
  if ( IS_NPC(ch) )
    return (UMAX((ch->armor + ( IS_AWAKE(ch) ? dex_app[get_curr_dex(ch)].defensive : 0 ) + VAMP_AC(ch)),-1500) - add_share_ac(ch) + bonus );
  if ( IS_ADV_DUAL(ch) )
    return (UMAX((ch->armor + ( IS_AWAKE(ch) ? dex_app[get_curr_dex(ch)].defensive : 0 ) + VAMP_AC(ch)),-1500) - add_share_ac(ch) + bonus ) + (!IS_NPC(ch) ? ch->pcdata->mac_bonus : 0);
}

/* Halfer*/
if ( get_ac_type_char(ch) == AC_TYPE_HALFLING )
{
  if ( !IS_ADVANCED(ch) )
    return UMAX((ch->armor + ( IS_AWAKE(ch) ? dex_app[get_curr_dex(ch)].defensive : 0 ) + VAMP_AC(ch)),-350) - add_share_ac(ch) + (!IS_NPC(ch) ? ch->pcdata->mac_bonus : 0);
  if ( IS_ADVANCED(ch) && !IS_ADV_DUAL(ch) ) 
    return UMAX((ch->armor + ( IS_AWAKE(ch) ? dex_app[get_curr_dex(ch)].defensive : 0 ) + VAMP_AC(ch)), -600) - add_share_ac(ch) + (!IS_NPC(ch) ? ch->pcdata->mac_bonus : 0);
  if ( IS_ADV_DUAL(ch) )
    return UMAX((ch->armor + ( IS_AWAKE(ch) ? dex_app[get_curr_dex(ch)].defensive : 0 ) + VAMP_AC(ch)),-950) - add_share_ac(ch) + (!IS_NPC(ch) ? ch->pcdata->mac_bonus : 0);
}

/* Pure Magic User */
if ( get_ac_type_char(ch) == AC_TYPE_PURE_MAGIC_USER )
{
  if ( !IS_ADVANCED(ch) )
    return UMAX((ch->armor + ( IS_AWAKE(ch) ? dex_app[get_curr_dex(ch)].defensive : 0 ) + VAMP_AC(ch)),-250) - add_share_ac(ch) + (!IS_NPC(ch) ? ch->pcdata->mac_bonus : 0);
  if ( IS_ADVANCED(ch) && !IS_ADV_DUAL(ch) ) 
    return UMAX((ch->armor + ( IS_AWAKE(ch) ? dex_app[get_curr_dex(ch)].defensive : 0 ) + VAMP_AC(ch)), -500) - add_share_ac(ch) + (!IS_NPC(ch) ? ch->pcdata->mac_bonus : 0);
  if ( IS_ADV_DUAL(ch) )
    return UMAX((ch->armor + ( IS_AWAKE(ch) ? dex_app[get_curr_dex(ch)].defensive : 0 ) + VAMP_AC(ch)),-700) - add_share_ac(ch) + (!IS_NPC(ch) ? ch->pcdata->mac_bonus : 0);
}

return 0;
}

int GET_HRDR_CAP( CHAR_DATA *ch )
{
/* Fighter */
if ( get_ac_type_char(ch) == AC_TYPE_PURE_FIGHTER )
{
  if ( !IS_ADVANCED(ch) )
	return 150;
  if ( IS_ADVANCED(ch) && !IS_ADV_DUAL(ch) )
	return 250;
  if ( IS_ADV_DUAL(ch) )
	return 350;
}

/* Rogue */
if ( get_ac_type_char(ch) == AC_TYPE_HALFLING )
{
  if ( !IS_ADVANCED(ch) )
	return 100;
  if ( IS_ADVANCED(ch) && !IS_ADV_DUAL(ch) )
	return 200;
  if ( IS_ADV_DUAL(ch) )
	return 300;
}

/* Magic User */
if ( get_ac_type_char(ch) == AC_TYPE_PURE_MAGIC_USER )
{
  if ( !IS_ADVANCED(ch) )
	return 75;
  if ( IS_ADVANCED(ch) && !IS_ADV_DUAL(ch) )
	return 175;
  if ( IS_ADV_DUAL(ch) )
	return 275;
}

return 0;
}

int GET_AC_CAP( CHAR_DATA *ch )
{
/* Fighter */
if ( get_ac_type_char(ch) == AC_TYPE_PURE_FIGHTER )
{
  if ( !IS_ADVANCED(ch) )
	return -500;
  if ( IS_ADVANCED(ch) && !IS_ADV_DUAL(ch) )
	return -1000;
  if ( IS_ADV_DUAL(ch) )
	return -1500;
}

/* Rogue */
if ( get_ac_type_char(ch) == AC_TYPE_HALFLING )
{
  if ( !IS_ADVANCED(ch) )
	return -350;
  if ( IS_ADVANCED(ch) && !IS_ADV_DUAL(ch) )
	return -600;
  if ( IS_ADV_DUAL(ch) )
	return -950;
}

/* Magic User */
if ( get_ac_type_char(ch) == AC_TYPE_PURE_MAGIC_USER )
{
  if ( !IS_ADVANCED(ch) )
	return -250;
  if ( IS_ADVANCED(ch) && !IS_ADV_DUAL(ch) )
	return -500;
  if ( IS_ADV_DUAL(ch) )
	return -700;
}

return 0;
}

int GET_MAC_CAP( CHAR_DATA *ch )
{
/* Fighter */
if ( get_ac_type_char(ch) == AC_TYPE_PURE_FIGHTER )
{
  if ( !IS_ADVANCED(ch) )
	return -150;
  if ( IS_ADVANCED(ch) && !IS_ADV_DUAL(ch) )
	return -250;
  if ( IS_ADV_DUAL(ch) )
	return -350;
}

/* Rogue */
if ( get_ac_type_char(ch) == AC_TYPE_HALFLING )
{
  if ( !IS_ADVANCED(ch) )
	return -225;
  if ( IS_ADVANCED(ch) && !IS_ADV_DUAL(ch) )
	return -400;
  if ( IS_ADV_DUAL(ch) )
	return -700;
}

/* Magic User */
if ( get_ac_type_char(ch) == AC_TYPE_PURE_MAGIC_USER )
{
  if ( !IS_ADVANCED(ch) )
	return -500;
  if ( IS_ADVANCED(ch) && !IS_ADV_DUAL(ch) )
	return -1000;
  if ( IS_ADV_DUAL(ch) )
	return -1500;
}

return 0;
}

int GET_TOTALAC_CAP( CHAR_DATA *ch )
{
  return -3000;
}

/*
 * Get a chars HITROLL .. replaces the #define in mud.h .. to complex for
 * a header file now --GW
 */
int GET_HITROLL( CHAR_DATA *ch )
{
int bonus = 0;

/* Fighter */
if ( get_ac_type_char(ch) == AC_TYPE_PURE_FIGHTER || IS_NPC(ch) )
{
  if( !IS_NPC(ch) && !IS_ADVANCED(ch) )                                          
    return UMIN((ch->hitroll+str_app[get_curr_str(ch)].tohit+(2-(abs(ch->mental_state)/10))),150) + add_share_hitndam(ch) + (!IS_NPC(ch) ? ch->pcdata->hit_bonus : 0);
  if (!IS_NPC(ch) && IS_ADVANCED(ch) && !IS_ADV_DUAL(ch) )
    return UMIN((ch->hitroll+str_app[get_curr_str(ch)].tohit+(2-(abs(ch->mental_state)/10))),250) + add_share_hitndam(ch) + (!IS_NPC(ch) ? ch->pcdata->hit_bonus : 0);
  if (IS_NPC(ch) )
    return (bonus + UMIN((ch->hitroll+str_app[get_curr_str(ch)].tohit+(2-(abs(ch->mental_state)/10))),350) + add_share_hitndam(ch));
  if (IS_ADV_DUAL(ch))
    return (bonus + UMIN((ch->hitroll+str_app[get_curr_str(ch)].tohit+(2-(abs(ch->mental_state)/10))),350) + add_share_hitndam(ch)) + (!IS_NPC(ch) ? ch->pcdata->hit_bonus : 0);
 }

/* Rouge */
if ( get_ac_type_char(ch) == AC_TYPE_HALFLING )
{
  if( !IS_ADVANCED(ch) )                                          
    return UMIN((ch->hitroll+str_app[get_curr_str(ch)].tohit+(2-(abs(ch->mental_state)/10))),100) + add_share_hitndam(ch) + (!IS_NPC(ch) ? ch->pcdata->hit_bonus : 0);
  if (IS_ADVANCED(ch) && !IS_ADV_DUAL(ch) )
    return UMIN((ch->hitroll+str_app[get_curr_str(ch)].tohit+(2-(abs(ch->mental_state)/10))),200) + add_share_hitndam(ch) + (!IS_NPC(ch) ? ch->pcdata->hit_bonus : 0);
  if (IS_ADV_DUAL(ch))
    return UMIN((ch->hitroll+str_app[get_curr_str(ch)].tohit+(2-(abs(ch->mental_state)/10))),300) + add_share_hitndam(ch) + (!IS_NPC(ch) ? ch->pcdata->hit_bonus : 0);
}

/* Magic user */
if ( get_ac_type_char(ch) == AC_TYPE_PURE_MAGIC_USER )
{
  if( !IS_ADVANCED(ch) )                                          
    return UMIN((ch->hitroll+str_app[get_curr_str(ch)].tohit+(2-(abs(ch->mental_state)/10))),75) + add_share_hitndam(ch) + (!IS_NPC(ch) ? ch->pcdata->hit_bonus : 0);
  if (IS_ADVANCED(ch) && !IS_ADV_DUAL(ch) )
    return UMIN((ch->hitroll+str_app[get_curr_str(ch)].tohit+(2-(abs(ch->mental_state)/10))),175) + add_share_hitndam(ch) + (!IS_NPC(ch) ? ch->pcdata->hit_bonus : 0);
  if (IS_ADV_DUAL(ch))
    return UMIN((ch->hitroll+str_app[get_curr_str(ch)].tohit+(2-(abs(ch->mental_state)/10))),275) + add_share_hitndam(ch) + (!IS_NPC(ch) ? ch->pcdata->hit_bonus : 0);
}

return -1;
}

/*
 * Get a chars DAMROLL .. replaces the #define in mud.h .. to complex for
 * a header file now --GW
 */
int GET_DAMROLL( CHAR_DATA *ch )
{
/* Fighter */
if ( get_ac_type_char(ch) == AC_TYPE_PURE_FIGHTER || IS_NPC(ch) )
{
  if ( !IS_NPC(ch) && !IS_ADVANCED(ch) )
    return UMIN((ch->damroll+str_app[get_curr_str(ch)].todam+((ch->mental_state > 5 && ch->mental_state < 15) ? 1 :0)),150)+ add_share_hitndam(ch) + (!IS_NPC(ch) ? ch->pcdata->dam_bonus : 0);
  if ( !IS_NPC(ch) && IS_ADVANCED(ch) && !IS_ADV_DUAL(ch) )
    return UMIN((ch->damroll+str_app[get_curr_str(ch)].todam+((ch->mental_state > 5 && ch->mental_state < 15) ? 1 :0)),250)+ add_share_hitndam(ch) + (!IS_NPC(ch) ? ch->pcdata->dam_bonus : 0);
  if (IS_NPC(ch))
    return (UMIN((ch->damroll+str_app[get_curr_str(ch)].todam+((ch->mental_state > 5 && ch->mental_state < 15) ? 1 :0)),350)+ add_share_hitndam(ch));
  if (IS_ADV_DUAL(ch) )
    return (UMIN((ch->damroll+str_app[get_curr_str(ch)].todam+((ch->mental_state > 5 && ch->mental_state < 15) ? 1 :0)),350)+ add_share_hitndam(ch)) + (!IS_NPC(ch) ? ch->pcdata->dam_bonus : 0);
}

/* Halfling */
if ( get_ac_type_char(ch) == AC_TYPE_HALFLING )
{
  if ( !IS_ADVANCED(ch) )
    return UMIN((ch->damroll+str_app[get_curr_str(ch)].todam+((ch->mental_state > 5 && ch->mental_state < 15) ? 1 :0)),100)+ add_share_hitndam(ch) + (!IS_NPC(ch) ? ch->pcdata->dam_bonus : 0);
  if ( IS_ADVANCED(ch) && !IS_ADV_DUAL(ch) )
    return UMIN((ch->damroll+str_app[get_curr_str(ch)].todam+((ch->mental_state > 5 && ch->mental_state < 15) ? 1 :0)),200)+ add_share_hitndam(ch) + (!IS_NPC(ch) ? ch->pcdata->dam_bonus : 0);
  if (IS_ADV_DUAL(ch) )
    return UMIN((ch->damroll+str_app[get_curr_str(ch)].todam+((ch->mental_state > 5 && ch->mental_state < 15) ? 1 :0)),300)+ add_share_hitndam(ch) + (!IS_NPC(ch) ? ch->pcdata->dam_bonus : 0);
}

/* Magic User */
if ( get_ac_type_char(ch) == AC_TYPE_PURE_MAGIC_USER )
{
  if ( !IS_ADVANCED(ch) )
    return UMIN((ch->damroll+str_app[get_curr_str(ch)].todam+((ch->mental_state > 5 && ch->mental_state < 15) ? 1 :0)),75)+ add_share_hitndam(ch) + (!IS_NPC(ch) ? ch->pcdata->dam_bonus : 0);
  if ( IS_ADVANCED(ch) && !IS_ADV_DUAL(ch) )
    return UMIN((ch->damroll+str_app[get_curr_str(ch)].todam+((ch->mental_state > 5 && ch->mental_state < 15) ? 1 :0)),175)+ add_share_hitndam(ch) + (!IS_NPC(ch) ? ch->pcdata->dam_bonus : 0);
  if (IS_ADV_DUAL(ch) )
    return UMIN((ch->damroll+str_app[get_curr_str(ch)].todam+((ch->mental_state > 5 && ch->mental_state < 15) ? 1 :0)),275)+ add_share_hitndam(ch) + (!IS_NPC(ch) ? ch->pcdata->dam_bonus : 0);
}

return -1;
}

char *number_pad( sh_int number )
{
char buf[MSL];
char *buf2;

if ( number < 10 )
sprintf(buf,"0%d",number);
else
sprintf(buf,"%d",number);

/*
switch(number)
{
case 0:
sprintf(buf,"%s","00");
break;
case 1:
sprintf(buf,"%s","01");
break;
case 2:
sprintf(buf,"%s","02");
break;
case 3:
sprintf(buf,"%s","03");
break;
case 4:
sprintf(buf,"%s","04");
break;
case 5:
sprintf(buf,"%s","05");
break;
case 6:
sprintf(buf,"%s","06");
break;
case 7:
sprintf(buf,"%s","07");
break;
case 8:
sprintf(buf,"%s","08");
break;
case 9:
sprintf(buf,"%s","09");
break;
default:
sprintf(buf,"%d",number);
break;
}
*/

buf2 = STRALLOC(buf);

return buf2;
}

sh_int total_affects( CHAR_DATA *ch )
{
sh_int total=0;

total = (((int)ch->pcdata->minoraffects) + ((int)ch->pcdata->majoraffects));

return total;
}

sh_int total_resists( CHAR_DATA *ch )
{
sh_int total=0;

total = (((int)ch->pcdata->minorresist) + ((int)ch->pcdata->majorresist));

return total;
}

/*
 * Are you.. a Mage.. a Fighter.. or a Halfling? --GW
 */
sh_int get_ac_type_char( CHAR_DATA *ch )
{
bool fighter=FALSE;
bool magic_user=FALSE;

switch(class_table[ch->class]->type)
{
   case CLASS_TYPE_MAGIC:
   magic_user=TRUE;
   break;
   case CLASS_TYPE_FIGHTER:
   fighter=TRUE;
   break;
   case CLASS_TYPE_EXCLUDE:
   break;
   default:
   bug("BAD CLASS TYPE (%s)",class_table[ch->class]->who_name);
   break;
}

if ( ch->class2 > -1 )
{
switch(class_table[ch->class2]->type)
{
   case CLASS_TYPE_MAGIC:
   magic_user=TRUE;
   break;
   case CLASS_TYPE_FIGHTER:
   fighter=TRUE;
   break;
   case CLASS_TYPE_EXCLUDE:
   break;
   default:
   bug("BAD CLASS TYPE (%s)",class_table[ch->class2]->who_name);
   break;
}
}

if ( IS_ADVANCED(ch) )
{
switch(class_table[ch->advclass]->type)
{
   case CLASS_TYPE_MAGIC:
   magic_user=TRUE;
   break;
   case CLASS_TYPE_FIGHTER:
   fighter=TRUE;
   break;
   case CLASS_TYPE_EXCLUDE:
   break;
   default:
   bug("BAD CLASS TYPE (%s)",class_table[ch->advclass]->who_name);
   break;
}
}

if ( IS_ADV_DUAL(ch) )
{
switch(class_table[ch->class]->type)
{
   case CLASS_TYPE_MAGIC:
   magic_user=TRUE;
   break;
   case CLASS_TYPE_FIGHTER:
   fighter=TRUE;
   break;
   case CLASS_TYPE_EXCLUDE:
   break;
   default:
   bug("BAD CLASS TYPE (%s)",class_table[ch->class]->who_name);
   break;
}
}

/* Fighter */
if ( fighter && !magic_user )
return AC_TYPE_PURE_FIGHTER;

/* Halfling */
if ( fighter && magic_user )
return AC_TYPE_HALFLING;

/* Magic User */
if ( magic_user && !fighter )
return AC_TYPE_PURE_MAGIC_USER;

return -1;
}

/*
 * 'Special Moves' for PURE Classers --GW 
 */
void do_special( CHAR_DATA *ch, char *argument )
{
int ac_type=-1;
char buf[MSL];
char arg[MIL];
CHAR_DATA *victim;
int cost;

one_argument( argument, arg );

if ( IS_SET(ch->in_room->room_flags, ROOM_SAFE) )
{
  send_to_char("You feel to good to do that here...\n\r",ch);
  return;
}

if ( ch->fighting && ch->fighting->who )
victim=ch->fighting->who;
else
if( ( victim=get_char_room( ch, arg ) )==NULL)
{
  send_to_char("I don't see that person here....\n\r",ch);
  return;
}

if ( victim==ch)
{
send_to_char("Now thats Rather Stupid!!!\n\r",ch);
return;
}

/* No Mobs doin this shit .. thank you! --GW */
if ( IS_NPC(ch) )
return;

if ( !check_pk_ok(victim,ch))
  return;


if ( !IS_IMMORTAL(ch)) ac_type=get_ac_type_char(ch);
else ac_type=number_range(0,2);

switch(ac_type)
{
   case AC_TYPE_PURE_FIGHTER:
   cost = ch->max_move/2;
   if( ch->move < cost )
   {
      send_to_char("You don't have enough movement!\n\r",ch);
      return;
   }
   sprintf(buf,
   "\n\r\n\r"
   "&B$n suddenly gets a strange look in $s eyes, as $s raises $s sword high in the air!\n"
   "$n screams to all the forgotten warriors of the past to help $m!"
   "\n\r"
   "&B$n's blade turns to &RFLAMES&B as $e brings it down with &RIncedable SPEED and FORCE"
   "&Bto crash directly in the center of $N's Skull!\n\r\n\r");
   act(AT_PLAIN,buf,ch,NULL,victim,TO_ROOM);
   sprintf(buf,
   "\n\r\n\r"
   "&BYou suddenly get a strange look in your eyes, as raise your sword high in the air!\n"
   "You scream to all the forgotten warriors of the past to help You!"
   "\n\r"
   "&BYour blade turns to &RFLAMES&B as you bring it down with &RIncedable SPEED and FORCE"
   "&Bto crash directly in the center of $N's Skull!\n\r\n\r");
   act(AT_PLAIN,buf,ch,NULL,victim,TO_CHAR);
   sprintf(buf,
   "\n\r\n\r"
   "&BYou suddenly see $N get a strange look in $S eyes, as $E raises $S sword high in the air!\n"
   "$N scream's to all the forgotten warriors of the past to help $M!"
   "\n\r"
   "&B$N's blade turns to &RFLAMES&B as $E brings it down with &RIncedable SPEED and FORCE"
   "&Bto crash directly in the center of &RYOUR Skull!\n\r\n\r");
   act(AT_PLAIN,buf,ch,NULL,victim,TO_VICT);
   ch->move -= cost;
   damage( ch, victim, (ch->level * 10), TYPE_UNDEFINED );
   break;   
   case AC_TYPE_PURE_MAGIC_USER:
   /* Oops.. need to accomodate vampires so they dont get infinate casting --GW*/
   cost = ( !IS_VAMPIRE(ch) ? ch->max_mana/4 : get_bloodthirst(ch)/4);

   if ( !IS_NPC(ch) && !IS_VAMPIRE(ch) && ch->mana < cost )
   {
     send_to_char("You don't have enough mana!\n\r",ch); 
     return;
   }
   if ( !IS_NPC(ch) && IS_VAMPIRE(ch) && ch->pcdata->condition[COND_BLOODTHIRST] < cost )
   {
     send_to_char("You don't have enough blood power!\n\r",ch);
     return;
   }

   sprintf(buf,
   "\n\r\n\r"
   "&B$n's entire body begins to glow brightly as $e slowly rises above the battle..\n\r"
   "$n begins to chant and $s eyes &Rglow red&B as $e summons the powers of all $s\n\r"
   "teachers from the far reaches of the &Runiverse!\n\r"
   "&RMolten Rock Shoots from the Ground and Engulf's $N!"
   "\n\r\n\r");
   act(AT_PLAIN,buf,ch,NULL,victim,TO_ROOM);   
   sprintf(buf,
   "\n\r\n\r"
   "&BYour entire body begins to glow brightly as you slowly rise above the battle..\n\r"
   "You begin to chant and your eyes &Rglow red&B as you summon the powers of all your\n\r"
   "teachers from the far reaches of the &Runiverse!\n\r"
   "&RMolten Rock Shoots from the Ground and Engulf's $N!"
   "\n\r\n\r");
   act(AT_PLAIN,buf,ch,NULL,victim,TO_CHAR);   
   sprintf(buf,
   "\n\r\n\r"
   "&B$N's entire body begins to glow brightly as $E slowly rises above the battle..\n\r"
   "$N begins to chant and $S eyes &Rglow red&B as $E summons the powers of all $S\n\r"
   "teachers from the far reaches of the &Runiverse!\n\r"
   "&RMolten Rock Shoots from the Ground and Engulf's YOU!"
   "\n\r\n\r");
   act(AT_PLAIN,buf,ch,NULL,victim,TO_VICT);   
   /* Accomodate Vampires.... --GW */
   if ( !IS_VAMPIRE(ch) )
	ch->mana -= cost;
   else
        ch->pcdata->condition[COND_BLOODTHIRST] -= cost;

   damage(ch,victim,(ch->level * 20),TYPE_UNDEFINED);
   break;
   default:
   send_to_char("Rogue's Don't have Special Moves! They Are not Educated Enough!\n\r",ch);    
   break;
}

return;
}

void do_ic( CHAR_DATA *ch, char *argument )
{

if ( IS_NPC(ch) )
return;

if ( IS_SET( ch->pcdata->flagstwo, MOREPC_IC ) )
{
  send_to_char( "But you are already In Character!\n\r",ch);
  return;
}

SET_BIT( ch->pcdata->flagstwo, MOREPC_IC );
send_to_char("You are now In Character.\n\r",ch);
act( AT_MAGIC,"$n is now In Character.",ch,NULL,NULL,TO_ROOM);
return;
}

void do_ooc( CHAR_DATA *ch, char *argument )
{

if ( !IS_SET( ch->pcdata->flagstwo, MOREPC_IC ) )
{
  send_to_char( "But you are already Out of Character!\n\r",ch);
  return;
}

REMOVE_BIT( ch->pcdata->flagstwo, MOREPC_IC );
send_to_char("You are now Out of Character.\n\r",ch);
act( AT_MAGIC,"$n is now Out of Character.",ch,NULL,NULL,TO_ROOM);
return;
}

void do_changes( CHAR_DATA *ch, char *argument )
{
show_file(ch, "../system/CHANGES");
return;
}

bool check_new_char_pass( char *argument )
{
char email[MSL];
char pass[MSL];
APPROVE_DATA *approve, *approve_next;

argument = one_argument( argument, email );
argument = one_argument( argument, pass );

if ( email[0] == '\0' || pass[0] == '\0' )
return FALSE;

for( approve = sysdata.first_approve; approve; approve = approve_next )
{
  approve_next = approve->next;

  if ( !approve->email || !approve->password )
  {
    bug("Approve Entry with no Email or Password!! (Line: %s %s)",email,pass);
    continue;
  }

  if ( !str_cmp(approve->email, email ) && 
       !str_cmp(approve->password, pass ) )
     return TRUE;

}

return FALSE;     
}

/*
 * Couple of Macro's --GW
 */
int GET_MAX_LEVEL( CHAR_DATA *ch )
{
  if ( IS_NPC(ch) )
    return ch->level;

  if ( !IS_ADVANCED(ch) )
	return ch->level;

  if ( !IS_ADV_DUAL(ch) )
	return ch->advlevel;

  return ch->advlevel2;
}

char *GET_MAX_CLASS(CHAR_DATA *ch)
{
  if ( IS_NPC(ch) )
    return "Mob";

  if ( !IS_ADVANCED(ch) )
	return class_table[ch->class]->who_name;

  if ( !IS_ADV_DUAL(ch) )
	return class_table[ch->advclass]->who_name;

return class_table[ch->advclass2]->who_name;
}

char *get_damage_value_obj( OBJ_DATA *obj )
{
  int pcnt=0;

  if ( obj->value[0] <= 0 )
   return "N/A";

if ( obj->item_type == ITEM_ARMOR && obj->value[1] < 1 )
  return "Unknown";
else if ( obj->item_type == ITEM_ARMOR )
  pcnt = obj->value[0] * 100 / obj->value[1];
else if ( obj->item_type == ITEM_WEAPON )
  pcnt = obj->value[0] * 100 / INIT_WEAPON_CONDITION;
else
  return "N/A";

  if ( pcnt >= 100 )
    return "Excellent";
  else if ( pcnt >= 90 )
    return "Scratched";
  else if ( pcnt >= 80 )
    return "Chipped up";
  else if ( pcnt >= 70 )
    return "Well Used";
  else if ( pcnt >= 60 )
    return "Worn";
  else if ( pcnt >= 50 )
    return "Neglected";
  else if ( pcnt >= 40 )
    return "Rusted";
  else if ( pcnt >= 30 )
    return "Horrid";
  else if ( pcnt >= 20 )
    return "Worthless";
  else if ( pcnt >= 10 )
    return "Wrecked";
  else
    return "Destroyed";
}

/* Security --GW */

bool HIMP_CHECK( CHAR_DATA *ch )
{
SECURE_DATA *tmp, *next_tmp;

if ( IS_NPC(ch) )
return FALSE;

for( tmp=first_secure; tmp; tmp = next_tmp )
{
  next_tmp=tmp->next;

  if ( tmp->type == SECURE_TYPE_HIMP )
  {
   SECURE_NAME *tmp2, *next_tmp2;

     for( tmp2 = tmp->first_name; tmp2; tmp2 = next_tmp2 )
     {
       next_tmp2 = tmp2->next;
       if ( !str_cmp(ch->name, tmp2->name) )
       return TRUE;
     }
  }
}

return FALSE;
}

bool IMP_CHECK( CHAR_DATA *ch )
{
SECURE_DATA *tmp, *next_tmp;

if ( IS_NPC(ch) )
return FALSE;

for( tmp=first_secure; tmp; tmp = next_tmp )
{
  next_tmp=tmp->next;

  if ( tmp->type == SECURE_TYPE_IMP )
  {
   SECURE_NAME *tmp2, *next_tmp2;

     for( tmp2 = tmp->first_name; tmp2; tmp2 = next_tmp2 )
     {
       next_tmp2 = tmp2->next;
       if ( !str_cmp(ch->name, tmp2->name) )
       return TRUE;
     }
  }
}

return FALSE;
}

bool OWNER_CHECK( CHAR_DATA *ch )
{
SECURE_DATA *tmp, *next_tmp;

if ( IS_NPC(ch) )
return FALSE;

for( tmp=first_secure; tmp; tmp = next_tmp )
{
  next_tmp=tmp->next;

  if ( tmp->type == SECURE_TYPE_OWNER )
  {
   SECURE_NAME *tmp2, *next_tmp2;

     for( tmp2 = tmp->first_name; tmp2; tmp2 = next_tmp2 )
     {
       next_tmp2 = tmp2->next;
       if ( !str_cmp(ch->name, tmp2->name) )
       return TRUE;
     }
  }
}

return FALSE;
}

bool BPORT_IMP_CHECK( CHAR_DATA *ch)
{
SECURE_DATA *tmp, *next_tmp;

if ( IS_NPC(ch) )
return FALSE;

for( tmp=first_secure; tmp; tmp = next_tmp )
{
  next_tmp=tmp->next;

  if ( tmp->type == SECURE_TYPE_BPORT_IMP )
  {
   SECURE_NAME *tmp2, *next_tmp2;

     for( tmp2 = tmp->first_name; tmp2; tmp2 = next_tmp2 )
     {
       next_tmp2 = tmp2->next;
       if ( !str_cmp(ch->name, tmp2->name) )
       return TRUE;
     }
  }
}

return FALSE;
}

bool BPORT_HIMP_CHECK( CHAR_DATA *ch)
{
SECURE_DATA *tmp, *next_tmp;

if ( IS_NPC(ch) )
return FALSE;

for( tmp=first_secure; tmp; tmp = next_tmp )
{
  next_tmp=tmp->next;

  if ( tmp->type == SECURE_TYPE_BPORT_HIMP )
  {
   SECURE_NAME *tmp2, *next_tmp2;

     for( tmp2 = tmp->first_name; tmp2; tmp2 = next_tmp2 )
     {
       next_tmp2 = tmp2->next;
       if ( !str_cmp(ch->name, tmp2->name) )
       return TRUE;
     }
  }
}

return FALSE;
}

void load_imps( void )
{
SECURE_DATA *secure_himp=NULL;
SECURE_DATA *secure_imp=NULL;
SECURE_DATA *secure_owner=NULL;
SECURE_DATA *secure_bport_imp=NULL;
SECURE_DATA *secure_bport_himp=NULL;
FILE *fp=NULL;
char buf[MSL];

/* Initilization */
CREATE( secure_himp, SECURE_DATA, 1 );
CREATE( secure_imp, SECURE_DATA, 1 );
CREATE( secure_owner, SECURE_DATA, 1 );
CREATE( secure_bport_imp, SECURE_DATA, 1 );
CREATE( secure_bport_himp, SECURE_DATA, 1 );
secure_himp->first_name = NULL;
secure_himp->last_name = NULL;
secure_himp->type = SECURE_TYPE_HIMP;
secure_imp->first_name = NULL;
secure_imp->last_name = NULL;
secure_imp->type = SECURE_TYPE_IMP;
secure_owner->first_name = NULL;
secure_owner->last_name = NULL;
secure_owner->type = SECURE_TYPE_OWNER;
secure_bport_imp->first_name = NULL;
secure_bport_imp->last_name = NULL;
secure_bport_imp->type = SECURE_TYPE_BPORT_IMP;
secure_bport_himp->first_name = NULL;
secure_bport_himp->last_name = NULL;
secure_bport_himp->type = SECURE_TYPE_BPORT_HIMP;

/* Load the Info */

/* HIMP */
sprintf(buf,"%s%s", SECURE_DIR, HIMP_FILE );
if ( !(fp=fopen(buf,"r")))
{
  log_string("WME-SECURE: Unable to open HIMP File!");
  log_string("WME-SECURE: MUD Shutting down.");
  exit(0);
}

for( ; ; )
{
char letter;
char *name;

letter=fread_letter(fp);

if ( letter == '#' )
break;

name=fread_word(fp);
add_secure_name(name,secure_himp,"HIMP");
}
new_fclose(fp);
LINK(secure_himp,first_secure,last_secure,next,prev);

/* IMP */
sprintf(buf,"%s%s", SECURE_DIR, IMP_FILE );
if ( !(fp=fopen(buf,"r")))
{
  log_string("WME-SECURE: Unable to open IMP File!");
  log_string("WME-SECURE: MUD Shutting down.");
  exit(0);
}

for( ; ; )
{
char letter;
char *name;

letter=fread_letter(fp);

if ( letter == '#' )
break;

name=fread_word(fp);
add_secure_name(name,secure_imp,"IMP");
}
new_fclose(fp);
LINK(secure_imp,first_secure,last_secure,next,prev);

/* OWNER */
sprintf(buf,"%s%s", SECURE_DIR, OWNER_FILE );
if ( !(fp=fopen(buf,"r")))
{
  log_string("WME-SECURE: Unable to open OWNER File!");
  log_string("WME-SECURE: MUD Shutting down.");
  exit(0);
}

for( ; ; )
{
char letter;
char *name;

letter=fread_letter(fp);

if ( letter == '#' )
break;

name=fread_word(fp);
add_secure_name(name,secure_owner,"OWNER");
}
new_fclose(fp);
LINK(secure_owner,first_secure,last_secure,next,prev);

/* BPORT_IMP */
sprintf(buf,"%s%s", SECURE_DIR, BPORT_IMP_FILE );
if ( !(fp=fopen(buf,"r")))
{
  log_string("WME-SECURE: Unable to open BPORT_IMP File!");
  log_string("WME-SECURE: MUD Shutting down.");
  exit(0);
}

for( ; ; )
{
char letter;
char *name;

letter=fread_letter(fp);

if ( letter == '#' )
break;

name=fread_word(fp);
add_secure_name(name,secure_bport_imp,"BPORT IMP");
}
new_fclose(fp);
LINK(secure_bport_imp,first_secure,last_secure,next,prev);

/* BPORT_HIMP */
sprintf(buf,"%s%s", SECURE_DIR, BPORT_HIMP_FILE );
if ( !(fp=fopen(buf,"r")))
{
  log_string("WME-SECURE: Unable to open HIMP File!");
  log_string("WME-SECURE: MUD Shutting down.");
  exit(0);
}

for( ; ; )
{
char letter;
char *name;

letter=fread_letter(fp);

if ( letter == '#' )
break;

name=fread_word(fp);
add_secure_name(name,secure_bport_himp,"BPORT HIMP");
}
new_fclose(fp);
LINK(secure_bport_himp,first_secure,last_secure,next,prev);

log_string("WME-SECURE: Loading Complete, Done.");
}

void init_security( void )
{
first_secure = NULL;
last_secure = NULL;
log_string("Security Features Initialized.\n\r");
return;
}

void add_secure_name( char *plrname, SECURE_DATA *secure, char *type )
{
SECURE_NAME *name=NULL;
char buf[MSL];

CREATE(name, SECURE_NAME, 1);
name->name = STRALLOC(plrname);
LINK(name,secure->first_name,secure->last_name,next,prev);
sprintf(buf,"WME-SECURE: Adding %s: %s",type,capitalize(plrname));
log_string(buf);
return;
}
