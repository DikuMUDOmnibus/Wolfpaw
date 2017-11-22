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
 *   		     Glory Point Prize Administration Module		    *
 *--------------------------------------------------------------------------*/
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

CHAR_DATA *find_glory_salesman( ROOM_INDEX_DATA *room );
bool limit_check( int prize_number, CHAR_DATA *ch );
OBJ_DATA *get_pin( CHAR_DATA *victim );
OBJ_DATA *make_pin( CHAR_DATA *ch );
sh_int find_affect_bivector( char *name, sh_int type );
void affect_obj_perm( OBJ_DATA *pin, CHAR_DATA *ch, CHAR_DATA *salesman, sh_int biv, sh_int type );
void lower_obj_weight( CHAR_DATA *ch, CHAR_DATA *salesman,char *name );
void rename_obj( CHAR_DATA *ch, char *name, CHAR_DATA *salesman, char *new_name );
void affect_armor_class( OBJ_DATA *pin, CHAR_DATA *ch, CHAR_DATA *salesman, sh_int ac );
void affect_con( CHAR_DATA *ch, CHAR_DATA *salesman, sh_int con );
void set_obj_bit( char *name, CHAR_DATA *ch, CHAR_DATA *salesman, char *bit );
void skill_to_100( CHAR_DATA *ch, CHAR_DATA *salesman, char *skill );
OBJ_DATA *find_obj_by_name( char *name, CHAR_DATA *ch );
bool cost_check( CHAR_DATA *ch, sh_int prize_num );
void charge_glory( CHAR_DATA *ch, sh_int prize_num );

/*
 * Some prelimnary stuff, Defining prizes prices etc.. --GW
 */

/* Vnum of the Prize Pin */
#define PRIZE_PIN_VNUM	63

/* Prize Types */
#define PRIZE_MINOR_AFFECT		0
#define PRIZE_MAJOR_AFFECT		1
#define PRIZE_RENAME			2
#define PRIZE_10_AC			3
#define PRIZE_1_CON			4
#define PRIZE_INDESTRUCTABLE_ITEM	5
#define PRIZE_MINOR_RESIST		6
#define PRIZE_MAJOR_RESIST		7
#define PRIZE_SKILL_100			8
#define PRIZE_LOWERED_WEIGHT		9

/* Reward Types */
#define MINOR_AFF	1
#define MAJOR_AFF	2
#define MINOR_RES	3
#define MAJOR_RES	4

/* Bivector Numbers */
#define BIV_AQUA_BREATH		1
#define BIV_DETECT_INVIS	2
#define BIV_DETECT_HIDDEN	3
#define BIV_DETECT_EVIL		4
#define BIV_FLOAT		5
#define BIV_PROTECTION		6
#define BIV_SCRYING		7
#define BIV_INVIS		8
#define BIV_HIDE		9
#define BIV_SNEAK		10
#define BIV_TRUE_SIGHT		11
#define BIV_SANCTUARY		12
#define BIV_FIRESHIELD		13
#define BIV_ICESHIELD		14
#define BIV_SHOCKSHIELD		15
#define BIV_PASS_DOOR		16
#define BIV_FLYING		17
#define BIV_FIRE		18
#define BIV_COLD		19
#define BIV_ELECTRICITY		20
#define BIV_ACID		21
#define BIV_POISON		22
#define BIV_SLEEP		23
#define BIV_NONMAGIC		24
#define BIV_MAGIC		25
#define BIV_ENERGY		26

/*
 * Glory Status Settings -- For Restringing --GW
 */
#define GLORY_STATUS_NONE		0
#define GLORY_STATUS_KEYWORD		1
#define GLORY_STATUS_KEYWORD_CONFIRM	2
#define GLORY_STATUS_SHORT_DESC		3
#define GLORY_STATUS_SHORT_DESC_CONFIRM	4
#define GLORY_STATUS_LONG_DESC		5
#define GLORY_STATUS_LONG_DESC_CONFIRM	6

/* Base Structure for glory_table */
struct glory_prize_data
{
char *name;
long price;
bool gold;
char *details;
char *comments;
};

/* Defines for glory_table */
const struct glory_prize_data glory_table[] =
{
{ "Minor Affect", 500, FALSE,
  "AquaBreath, DetectInvis, DetectHidden, DetectEvil, Float, Protection",NULL },
{ "Major Affect", 3000, FALSE,
  "Scrying, Invis, Hide, Sneak, TrueSight, Sanctuary, FireShield, IceShield\n\r, ShockShield, PassDoor, Fly",
  "4 Affects Maximum, Minor or Major" },
{ "Rename", 200, FALSE, NULL, "Curse Words WILL CAUSE A CHARACTER PURGE!" },
{ "-10 AC", 300, FALSE, NULL, NULL },
{ "1 Con Point", 30000000, TRUE, NULL, NULL },
{ "Indestructable Item", 2000, FALSE, NULL, NULL },
{ "Minor Resist", 500, FALSE, 
  "Fire, Cold, Electricity, Acid, Poison, Sleep" },
{ "Major Resist", 3000, FALSE,
  "NonMagic, Magic, Energy", "4 Maximum, Minor or Major" },
{ "Skill to 100%",1000, FALSE, NULL, "5 Maximum" },
{ "Lowered Weight",50, FALSE, "No less then 1 Pound" },
{ NULL, 0, FALSE, NULL, NULL }
};


/*
 * Main Input Handler -- Lets a player pick what they want --GW
 * Format: prize <list/buy> <prize #> <obj name> <value/desc>
 */
void do_prize( CHAR_DATA *ch, char *argument )
{
char arg1[MIL];
char arg2[MIL];
char arg3[MIL];
int prize_num=0;
CHAR_DATA *salesman;
OBJ_DATA *pin;
sh_int bivector=0;

argument = one_argument( argument, arg1 ); /* List/Buy */
argument = one_argument( argument, arg2 ); /* Prize # */
argument = one_argument( argument, arg3 ); /* obj_name */

if ( arg1[0] == '\0' )
{
send_to_char("Format: prize <list/buy> <prize #> <obj name/argument> <value/desc>\n\r",ch);
return;
}

if ( !( salesman = find_glory_salesman( ch->in_room ) ) )
{
send_to_char("I see no Glory Salesman here....\n\r",ch);
return;
}

/* Prize List */
if ( !str_cmp( arg1, "list" ) )
{

send_to_char("The Glory Store -- Price List\n\r",ch);
send_to_char("=============================\n\r\n\r",ch);

for( prize_num=0; glory_table[prize_num].name != NULL; prize_num++ )
{
ch_printf(ch, "Prize Number: [%d]\n\r", prize_num+1 );
ch_printf(ch, "Name:    [%s]\n\r", glory_table[prize_num].name );
ch_printf(ch, "Price:   [%ld %s]\n\r", glory_table[prize_num].price,
	glory_table[prize_num].gold ? "Gold Coins" : "Glory Points" );
ch_printf(ch, "Details:\n\r%s\n\r",glory_table[prize_num].details ? 
	glory_table[prize_num].details : "None." );
ch_printf(ch, "Comments:%s\n\r", glory_table[prize_num].comments ?
	glory_table[prize_num].comments : "None." );
send_to_char("---------------------------------------------------------------------\n\r",ch);
}

return;
}

/* Buy Prize */
if ( !str_cmp( arg1, "buy" ) )
{

if ( arg2[0] == '\0' || !is_number(arg2) )
{
send_to_char("Format: prize <list/buy> <prize #> <obj name/argument> <value/desc>\n\r",ch);
return;
}

prize_num = atoi(arg2);

if ( !cost_check( ch, prize_num-1 ) )
{
   do_say(salesman, "But, you don't have enough Glory!");
   return;
}

switch( prize_num-1 )
{
	/* Minor Affects */
	/* Arg3: Affect Name */
	case PRIZE_MINOR_AFFECT:
	if ( arg3[0] == '\0' )
	{
	   send_to_char("Format: prize <list/buy> <prize #> <obj name/argument> <value/desc>\n\r",ch);
	   return;
	}
	pin=get_pin(ch);
	if ( !pin )
	{
	   do_say(salesman, "Oh, I see you have no prize Pin!");
	   do_say(salesman, "Now your wearing one!.");
	   do_say(salesman, "Remember: It is KEYED to your character, it cannot be transfered!");
	   pin=make_pin(ch);
	}
	if ( limit_check(prize_num-1,ch) == TRUE )
	{
	   do_say(salesman, "Sorry, You have to many of those already.");
	   break;
	}
	bivector = find_affect_bivector( arg3, MINOR_AFF );
	affect_obj_perm( pin, ch, salesman, bivector, MINOR_AFF );
	break;

	/* Major Affects */
	/* Arg3: affect name */
	case PRIZE_MAJOR_AFFECT:
	if ( arg3[0] == '\0' )
	{
	   send_to_char("Format: prize <list/buy> <prize #> <obj name/argument> <value/desc>\n\r",ch);
	   return;
	}
	pin=get_pin(ch);
	if ( !pin )
	{
	   do_say(salesman, "Oh, I see you have no prize Pin!");
	   do_say(salesman, "There is now one in your inventory.");
	   do_say(salesman, "Remember: It is KEYED to your character, it cannot be transfered!");
	   pin=make_pin(ch);
	}
	if ( limit_check(prize_num-1,ch) == TRUE )
	{
	   do_say(salesman, "Sorry, You have to many of those already.");
	   break;
	}
	bivector = find_affect_bivector( arg3, MAJOR_AFF );
	affect_obj_perm( pin, ch, salesman, bivector, MAJOR_AFF );
	break;

	/* Rename */
	/* Arg3: orig obj name */
  	/* Argument: new obj name */
	case PRIZE_RENAME:
	if ( arg3[0] == '\0' || !argument  )
	{
	   send_to_char("Format: prize <list/buy> <prize #> <obj name/argument> <value/desc>\n\r",ch);
	   return;
	}
	rename_obj( ch, arg3, salesman, argument );
	break;

	/* -10 AC */
	/* No Arguments */
	case PRIZE_10_AC:
	pin=get_pin(ch);
	if ( !pin )
	{
	   do_say(salesman, "Oh, I see you have no prize Pin!");
	   do_say(salesman, "There is now one in your inventory.");
	   do_say(salesman, "Remember: It is KEYED to your character, it cannot be transfered!");
	   pin=make_pin(ch);
	}
	affect_armor_class( pin, ch, salesman, 10 );
	break;

	/* 1 Con Point */
	/* No Arguments */
	case PRIZE_1_CON:
	affect_con( ch, salesman, 1 );
	break;

	/* Indestructable Item */
	/* Arg3: object name */
	case PRIZE_INDESTRUCTABLE_ITEM:
	if ( arg3[0] == '\0' )
	{
	   send_to_char("Format: prize <list/buy> <prize #> <obj name/argument> <value/desc>\n\r",ch);
	   return;
	}
	set_obj_bit( arg3, ch, salesman, "no_scrap" );
	break;

	/* Minor Resist */
	/* Arg3: resist name */
	case PRIZE_MINOR_RESIST:
	if ( arg3[0] == '\0' )
	{
	   send_to_char("Format: prize <list/buy> <prize #> <obj name/argument> <value/desc>\n\r",ch);
	   return;
	}
	pin=get_pin(ch);
	if ( !pin )
	{
	   do_say(salesman, "Oh, I see you have no prize Pin!");
	   do_say(salesman, "There is now one in your inventory.");
	   do_say(salesman, "Remember: It is KEYED to your character, it cannot be transfered!");
	   pin=make_pin(ch);
	}
	if ( limit_check(prize_num-1,ch) == TRUE )
	{
	   do_say(salesman, "Sorry, You have to many of those already.");
	   break;
	}
	bivector = find_affect_bivector( arg3, MINOR_RES );
	affect_obj_perm( pin, ch, salesman, bivector, MINOR_RES );
	break;

	/* Major Resist */
	/* Arg3: resist name */
	case PRIZE_MAJOR_RESIST:
	if ( arg3[0] == '\0' )
	{
	   send_to_char("Format: prize <list/buy> <prize #> <obj name/argument> <value/desc>\n\r",ch);
	   return;
	}
	pin=get_pin(ch);
	if ( !pin )
	{
	   do_say(salesman, "Oh, I see you have no prize Pin!");
	   do_say(salesman, "There is now one in your inventory.");
	   do_say(salesman, "Remember: It is KEYED to your character, it cannot be transfered!");
	   pin=make_pin(ch);
	}
	if ( limit_check(prize_num-1,ch) == TRUE )
	{
	   do_say(salesman, "Sorry, You have to many of those already.");
	   break;
	}
	bivector = find_affect_bivector( arg3, MAJOR_RES );
	affect_obj_perm( pin, ch, salesman, bivector, MAJOR_RES );
	break;

	/* Skill to 100% */
	/* arg3: skill name */
	case PRIZE_SKILL_100:
	if ( arg3[0] == '\0' )
	{
	   send_to_char("Format: prize <list/buy> <prize #> <obj name/argument> <value/desc>\n\r",ch);
	   return;
	}
	if ( limit_check(prize_num-1,ch) == TRUE )
	{
	   do_say(salesman, "Sorry, You have to many of those already.");
	   break;
	}
	skill_to_100( ch, salesman, arg3 );
	break;

	/* Lowered Weight */
	/* arg3: obj name */
	case PRIZE_LOWERED_WEIGHT:
	if ( arg3[0] == '\0' )
	{
	   send_to_char("Format: prize <list/buy> <prize #> <obj name/argument> <value/desc>\n\r",ch);
	   return;
	}
	lower_obj_weight( ch, salesman, arg3 );
	break;

	/* Wrong #....Dail again..*/
	default:
	send_to_char("Invalid Prize Number, type: prize list\n\r",ch);
	break;
}

return;
}

send_to_char("Format: prize <list/buy> <prize #> <obj name> <value/desc>\n\r",ch);
return;
}

/*
 * Find the Salesman in the room --GW
 */
CHAR_DATA *find_glory_salesman( ROOM_INDEX_DATA *room )
{
CHAR_DATA *man, *man_next;
bool found=FALSE;

for ( man = room->first_person; man; man = man_next )
{
    man_next = man->next_in_room;

    if ( !IS_NPC(man) )
	continue;

    if ( man->spec_fun == spec_lookup( "spec_glory_salesman" ) ) 
    { 
	found = TRUE;
	break;
    }
}

if( found )
return man;

return NULL;
}

/*
 * Make Sure the Character is UNDER Limit for the Prize
 * Returns:
 * TRUE : OVER or AT Limit
 * FALSE : UNDER limit
 * --Greywolf
 */
bool limit_check( int prize_number, CHAR_DATA *ch )
{

/* Major and Minor Affects */
if ( prize_number == PRIZE_MINOR_AFFECT || prize_number == PRIZE_MAJOR_AFFECT )
{
	if ( ( ch->pcdata->minoraffects + ch->pcdata->majoraffects )+1 > 4 )
	return TRUE;
}

/* Major and Minor Resists */
if ( prize_number == PRIZE_MINOR_RESIST || prize_number == PRIZE_MAJOR_RESIST )
{
	if ( ( ch->pcdata->minorresist + ch->pcdata->majorresist)+1 > 4 )
	return TRUE;
}

/* Spells to 100 */
if ( prize_number == PRIZE_SKILL_100 )
{
	if ( (ch->pcdata->skill100s+1) > 5 )
	return TRUE;
}


return FALSE;
}

/*
 * Get data for the Pin off the character --GW
 * Null if none found
 */
OBJ_DATA *get_pin( CHAR_DATA *victim )
{
OBJ_DATA *obj;

   if ( ( obj = get_obj_carry( victim, "prizepin" ) ) == NULL )
    {
        if ( ( obj = get_obj_wear( victim, "prizepin" ) ) == NULL )
        {
        return NULL;
        }
    }

return obj;
}

/*
 * Create the Pin, and put it on the character --GW 
 */
OBJ_DATA *make_pin( CHAR_DATA *ch )
{
OBJ_DATA *pin;
OBJ_DATA *old_pin;
ZONE_DATA *zone;

zone = find_zone(1);

if ( !( pin = create_object( get_obj_index( PRIZE_PIN_VNUM,1), 1, zone ) ) )
{
bug("Glory_Store: Make Pin: Unable to Create Prize Pin!");
return NULL;
}

obj_to_char( pin, ch );

old_pin = get_eq_char( ch, WEAR_INSIG );

if ( old_pin )
unequip_char( ch, old_pin );

equip_char( ch, pin, WEAR_INSIG );
return pin;
}

sh_int find_affect_bivector( char *name, sh_int type )
{
int biv=0;

switch( type )
{
 case MINOR_AFF:
 if ( !str_cmp( name, "aquabreath" ) ) { biv=BIV_AQUA_BREATH; break; }
 else if ( !str_cmp( name, "detectinvis" ) ) { biv=BIV_DETECT_INVIS; break; }
 else if ( !str_cmp( name, "detecthidden" ) ) { biv=BIV_DETECT_HIDDEN; break; }
 else if ( !str_cmp( name, "detectevil" ) ) { biv=BIV_DETECT_EVIL; break; }
 else if ( !str_cmp( name, "float" ) ) { biv=BIV_FLOAT; break; }
 else if ( !str_cmp( name, "protection" ) ) { biv=BIV_PROTECTION; break; }
 biv=-1;
 break;

 case MAJOR_AFF:
 if ( !str_cmp( name, "scrying" ) ) { biv=BIV_SCRYING; break; }
 else if ( !str_cmp( name, "invisable" ) ) { biv=BIV_INVIS; break; }
 else if ( !str_cmp( name, "hide" ) ) { biv=BIV_HIDE; break; }
 else if ( !str_cmp( name, "sneak" ) ) { biv=BIV_SNEAK; break; }
 else if ( !str_cmp( name, "truesight" ) ) { biv=BIV_TRUE_SIGHT; break; }
 else if ( !str_cmp( name, "sanctuary" ) ) { biv=BIV_SANCTUARY; break; }
 else if ( !str_cmp( name, "fireshield" ) ) {biv=BIV_FIRESHIELD; break; }
 else if ( !str_cmp( name, "iceshield" ) ) { biv=BIV_ICESHIELD; break; }
 else if ( !str_cmp( name, "shockshield" ) ) { biv=BIV_SHOCKSHIELD; break; }
 else if ( !str_cmp( name, "passdoor" ) ) { biv=BIV_PASS_DOOR; break; }
 else if ( !str_cmp( name, "flying" ) ) { biv=BIV_FLYING; break; }
 biv=-1;
 break;
 
 case MINOR_RES:
 if ( !str_cmp( name, "fire" ) ) { biv=BIV_FIRE; break; }
 else if ( !str_cmp( name, "cold" ) ) { biv=BIV_COLD; break; }
 else if ( !str_cmp( name, "electricity" ) ) { biv=BIV_ELECTRICITY; break; }
 else if ( !str_cmp( name, "acid" ) ) { biv=BIV_ACID; break; }
 else if ( !str_cmp( name, "poison" ) ) { biv=BIV_POISON; break; }
 else if ( !str_cmp( name, "sleep" ) ) { biv=BIV_SLEEP; break; }
 biv=-1;
 break;

 case MAJOR_RES:
 if ( !str_cmp( name, "nonmagic" ) ) { biv=BIV_NONMAGIC; break; }
 else if ( !str_cmp( name, "magic" ) ) { biv=BIV_MAGIC; break; }
 else if ( !str_cmp( name, "energy" ) ) { biv=BIV_ENERGY; break; }
 biv=-1;
 break;

 default:
 bug("Glory_Store: Bad Type!");
 break;

}

return biv;
}

void affect_obj_perm( OBJ_DATA *pin, CHAR_DATA *ch, CHAR_DATA *salesman, sh_int biv, sh_int type )
{
int value=-1,bitv=0;
char buf[MSL];
sh_int loc;
AFFECT_DATA *paf=NULL;
extern int  top_affect;
bool res=FALSE;
bool aff=FALSE;

if ( biv == -1 )
{
  do_say(salesman,"I know no such Affect, or Resistance!");
  return;
}

switch( type )
{
  case MINOR_AFF:
    switch( biv )
    {
      case BIV_AQUA_BREATH:
      strcpy(buf,"aqua_breath");
      break;
      case BIV_DETECT_INVIS:
      strcpy(buf,"detect_invis");
      break;
      case BIV_DETECT_HIDDEN:
      strcpy(buf,"detect_hidden");
      break;
      case BIV_DETECT_EVIL:
      strcpy(buf,"detect_evil");
      break;
      case BIV_FLOAT:
      strcpy(buf,"floating");
      break;
      case BIV_PROTECTION:
      strcpy(buf,"protect");
      break;
      default:
      bug("Glory_Store: Affect_Char_Perm: BAD BiV!");
      break;
    }
  break;
  case MAJOR_AFF:
    switch( biv )
    {
      case BIV_SCRYING:
      strcpy(buf,"scrying");
      break;
      case BIV_INVIS:
      strcpy(buf,"invisible");
      break;
      case BIV_HIDE:
      strcpy(buf,"hide");
      break;
      case BIV_SNEAK:
      strcpy(buf,"sneak");
      break;
      case BIV_TRUE_SIGHT:
      strcpy(buf,"truesight");
      break;
      case BIV_SANCTUARY:
      strcpy(buf,"sanctuary");
      break;
      case BIV_FIRESHIELD:
      strcpy(buf,"fireshield");
      break;
      case BIV_ICESHIELD:
      strcpy(buf,"iceshield");
      break;
      case BIV_SHOCKSHIELD:
      strcpy(buf,"shockshield");
      break;
      case BIV_PASS_DOOR:
      strcpy(buf,"pass_door");
      break;
      case BIV_FLYING:
      strcpy(buf,"flying");
      break;
      default:
      bug("Glory_Store: Affect_Char_Perm: BAD BiV!");
      break;
    }
  break;
  case MINOR_RES:
    switch( biv )
    {
      case BIV_FIRE:
      strcpy( buf, "fire" );
      break;
      case BIV_COLD:
      strcpy( buf, "cold" );
      break;
      case BIV_ELECTRICITY:
      strcpy( buf, "electricity" );
      break;
      case BIV_ACID:
      strcpy( buf, "acid" );
      break;
      case BIV_POISON:
      strcpy( buf, "poison" );
      break;
      case BIV_SLEEP:      
      strcpy( buf, "sleep" );
      break;
      default:
      bug("Glory_Store: Affect_Char_Perm: BAD BiV!");
      break;
    }
  break;
  case MAJOR_RES:
    switch( biv )
    {
      case BIV_NONMAGIC:
      strcpy( buf, "nonmagic" );
      break;
      case BIV_MAGIC:
      strcpy( buf, "magic" );
      break;
      case BIV_ENERGY:
      strcpy( buf, "energy" );
      break;
      default:
      bug("Glory_Store: Affect_Char_Perm: BAD BiV!");
      break;
    }
  break;
  default:
  bug("Glory_Store: Affect_char_perm: Bad type!(%d)",type);
  break;
}

if ( type == MINOR_AFF || type == MAJOR_AFF )
{
loc = get_atype( "affected" );
value = get_aflag(buf);
aff=TRUE;
}
else
{
value = get_risflag( buf );
loc = get_atype( "resistant" );
res=TRUE;
}

if ( value > -1 )
{

if ( ( type == MINOR_AFF || type == MAJOR_AFF ) &&
       xIS_SET(ch->affected_by, value ) )
{
do_say(salesman,"But you already have that affect on you!");
do_say(salesman,"Try Dispeling first."); 
return;
}

if ( ( type == MINOR_RES || type == MAJOR_RES ) &&
       IS_SET(ch->resistant, value ) )
{
do_say(salesman,"But you already have that resist on you!");
do_say(salesman,"Try Dispeling first"); 
return;
}
}

SET_BIT( bitv, 1 << value );
value = bitv;

if ( value==-1 || !bitv )
{
bug("Glory_Store: INVALID BITVECTOR!",0);
do_say(salesman, "An Error has Occured, please contact Greywolf with complete Details on what you did.");
return;
}

CREATE( paf, AFFECT_DATA, 1 );
paf->type 	= -1;
paf->duration	= -1;
paf->location	= loc;
paf->modifier   = value;
xCLEAR_BITS(paf->bitvector);
paf->next	= NULL;
LINK( paf, pin->first_affect, pin->last_affect, next, prev );
++top_affect;

switch( type )
{
  case MINOR_AFF:
  ch->pcdata->minoraffects++;
  do_say(salesman,"Minor Affect Added =)" );
  charge_glory( ch, PRIZE_MINOR_AFFECT );
  break;
  case MAJOR_AFF:
  ch->pcdata->majoraffects++;
  do_say(salesman,"Major Affect Added =)" );
  charge_glory( ch, PRIZE_MAJOR_AFFECT );
  break;
  case MINOR_RES:
  ch->pcdata->minorresist++;
  do_say(salesman,"Minor Resist Added =)" );
  charge_glory( ch, PRIZE_MINOR_RESIST );
  break;
  case MAJOR_RES:
  ch->pcdata->majorresist++;
  do_say(salesman,"Major Resist Added =)" );
  charge_glory( ch, PRIZE_MAJOR_RESIST );
  break;
  default:
  bug("Glory_Store: affect_obj_perm: BAD TYPE!", 0);
  break;
}

save_char_obj(ch);
return;
}

/* 
 * Lower the Weight of an object --GW
 */
void lower_obj_weight( CHAR_DATA *ch, CHAR_DATA *salesman,char *name )
{
OBJ_DATA *obj;

if ( !( obj=find_obj_by_name( name, ch ) ) )
{
do_say(salesman, "You don't seem to have an object like that!" );
return;
}

if ( obj->weight-1 < 1 )
{
do_say(salesman, "That object cannot get any lighter!\n\r");
return;
}

obj->weight--;
do_say(salesman, "The object's weight has been altered.");
charge_glory( ch, PRIZE_LOWERED_WEIGHT );
return;
}

/*
 * Restring an Object --GW
 */
void rename_obj( CHAR_DATA *ch, char *name, CHAR_DATA *salesman, char *new_name )
{
OBJ_DATA *obj;
char say_buf[MSL];

if ( !( obj=find_obj_by_name( name, ch ) ) )
{
do_say(salesman, "You don't seem to have an object like that!" );
return;
}

separate_obj(obj);

if ( IS_SET( obj->pIndexData->extra_flags, ITEM_NO_RESTRING ) )
{
do_say(salesman, "You Cannot Restring that! The Gods Forbid it!" );
return;
}

switch( ch->pcdata->glory_status )
{
    /* Start the process, take the glory off the top */
    case GLORY_STATUS_NONE:
	charge_glory( ch, PRIZE_RENAME );
	do_say(salesman,"We will now set the KEYWORDS For your Object.");
	sprintf(say_buf,"%s, %s", capitalize(ch->name),
		"Please Type: prize buy 3 <obj name> <keywords>");
	do_say(salesman,say_buf);
	do_say(salesman,"Seperate the words with 1 space!");
	ch->pcdata->glory_status = GLORY_STATUS_KEYWORD;
	break;
    /* Set the Keywords, and send the confirm */
    case GLORY_STATUS_KEYWORD:
	obj->name = STRALLOC(new_name);
	sprintf(say_buf,"Object Keywords: %s",obj->name);
	do_say(salesman, say_buf);
	do_say(salesman,"If this is Correct, type:");
	do_say(salesman,"prize buy 3 <objname> yes");
	do_say(salesman,"If it is wrong, start at this step again");
	ch->pcdata->glory_status = GLORY_STATUS_KEYWORD_CONFIRM;
	break;
    /* Confirm the Keywords, reset them if they are not correct */
    case GLORY_STATUS_KEYWORD_CONFIRM:
	if ( !str_cmp( new_name, "yes" ) )
	{
	   do_say(salesman,"Keywords Confirmed, Continuing.");
	   do_say(salesman,"We will now Change the Short Description of the Object.");
	   do_say(salesman,"This is the name you see when it is Worn, or in your Inventory.");
	   do_say(salesman,"Please Type: prize buy 3 <objname> <desc>");
	   ch->pcdata->glory_status = GLORY_STATUS_SHORT_DESC;
	   break;
	}
	obj->name = STRALLOC(new_name);
	sprintf(say_buf,"Object Keywords: %s",obj->name);
	do_say(salesman,say_buf);
	do_say(salesman,"If this is Correct, type:");
	do_say(salesman,"prize buy 3 <objname> yes");
	do_say(salesman,"If it is wrong, start at this step again");
	ch->pcdata->glory_status = GLORY_STATUS_KEYWORD_CONFIRM;
	break;
    /* Set the Short Desc, send to Confirm */
    case GLORY_STATUS_SHORT_DESC:	
	obj->short_descr = STRALLOC(new_name);
	sprintf(say_buf,"Object Short Desc: %s",obj->short_descr);
	do_say(salesman,say_buf);
	do_say(salesman,"If this is Correct, type:");
	do_say(salesman,"prize buy 3 <objname> yes");
	do_say(salesman,"If it is wrong, start at this step again");
	ch->pcdata->glory_status = GLORY_STATUS_SHORT_DESC_CONFIRM;
	break;
    /* Confirm the Short Desc */
    case GLORY_STATUS_SHORT_DESC_CONFIRM:	
	if ( !str_cmp( new_name, "yes" ) )
	{
	   do_say(salesman,"Short Desc Confirmed, Continuing.");
	   do_say(salesman,"We will now Change the Long Description of the Object.");
	   do_say(salesman,"This is the name you see when it is on the ground.");
	   do_say(salesman,"Please Type: prize buy 3 <objname> <desc>");
	   ch->pcdata->glory_status = GLORY_STATUS_LONG_DESC;
	   break;
	}
	obj->short_descr = STRALLOC(new_name);
	sprintf(say_buf,"Object Short Desc: %s",obj->short_descr);
	do_say(salesman,say_buf);
	do_say(salesman,"If this is Correct, type:");
	do_say(salesman,"prize buy 3 <objname> yes");
	do_say(salesman,"If it is wrong, start at this step again");
	ch->pcdata->glory_status = GLORY_STATUS_SHORT_DESC_CONFIRM;
	break;
    /* Set the Long Desc, send to Confirm */
    case GLORY_STATUS_LONG_DESC:	
	obj->description = STRALLOC(new_name);
	sprintf(say_buf,"Object Long Desc: %s",obj->description);
	do_say(salesman,say_buf);
	do_say(salesman,"If this is Correct, type:");
	do_say(salesman,"prize buy 3 <objname> yes");
	do_say(salesman,"If it is wrong, start at this step again");
	ch->pcdata->glory_status = GLORY_STATUS_LONG_DESC_CONFIRM;
	break;
    /* Confirm the Long Desc */
    case GLORY_STATUS_LONG_DESC_CONFIRM:	
	if ( !str_cmp( new_name, "yes" ) )
	{
	   do_say(salesman,"Long Desc Confirmed.");
	   do_say(salesman,"Object Rename Complete.");
	   ch->pcdata->glory_status = GLORY_STATUS_NONE;
	   save_char_obj(ch);
	   break;
	}
	obj->description = STRALLOC(new_name);
	sprintf(say_buf,"Object Long Desc: %s",obj->description);
	do_say(salesman,say_buf);
	do_say(salesman,"If this is Correct, type:");
	do_say(salesman,"prize buy 3 <objname> yes");
	do_say(salesman,"If it is wrong, start at this step again");
	ch->pcdata->glory_status = GLORY_STATUS_LONG_DESC_CONFIRM;
	break;
}

return;
}

/*
 * Add a Armor Class Affect to an Object --GW
 */
void affect_armor_class( OBJ_DATA *pin, CHAR_DATA *ch, CHAR_DATA *salesman, sh_int ac )
{

if ( pin->value[0]+ac >= 30000 )
{
do_say(salesman, "I'm sorry, I cannot modify your ac any further");
return;
}

pin->value[0] += ac;

/*
AFFECT_DATA *paf;
extern int top_affect;

CREATE( paf, AFFECT_DATA, 1 );
paf->type               = -1;
paf->duration           = -1;
paf->location           = APPLY_AC;
paf->modifier           = ac;
paf->bitvector          = 0;
paf->next               = NULL;

LINK( paf, pin->first_affect, pin->last_affect, next, prev );
++top_affect;
*/

do_say(salesman, "A -10 AC Affect has been added to your Prize Pin!" );
unequip_char( ch, pin );
equip_char( ch, pin, WEAR_INSIG );
charge_glory( ch, PRIZE_10_AC );
save_char_obj(ch);
return;
}

/*
 * Set a Players Constitution --GW
 */
void affect_con( CHAR_DATA *ch, CHAR_DATA *salesman, sh_int con )
{

if ( ch->perm_con+1 >= 25 )
{
do_say( salesman, "Your Constitution is already at Maximum!" );
return;
}

ch->perm_con++;
do_say(salesman,"Your constitution has been modified." );
charge_glory( ch, PRIZE_1_CON );
return;
}

/*
 * Set a Bit on an Object --GW
 */
void set_obj_bit( char *name, CHAR_DATA *ch, CHAR_DATA *salesman, char* bit )
{
OBJ_DATA *obj;
bool FOUND;

if ( !( obj=find_obj_by_name( name, ch ) ) )
{
do_say(salesman, "You don't seem to have an object like that!" );
return;
}

FOUND = FALSE;

if ( !str_cmp( bit, "no_scrap" ) )
{
SET_BIT( obj->second_flags, ITEM2_NO_SCRAP );
FOUND = TRUE;
}

if ( FOUND )
do_say( salesman, "That object is now Indestructable!" );
else
do_say( salesman, "An Error has occured, contact Greywolf with Details!");

save_char_obj(ch);
charge_glory( ch, PRIZE_INDESTRUCTABLE_ITEM );
return;
}

/*
 * set a skill to 100% --GW
 */
void skill_to_100( CHAR_DATA *ch, CHAR_DATA *salesman, char *skill )
{
int sn=0;

sn = skill_lookup(skill);

if ( sn <= 0 )
{
do_say( salesman, "There is no such skill known to me." );
return;
}

/* Make sure they HAVE the skill first --GW */
if ( !CAN_USE_SK(ch, skill_table[sn]) && !CAN_USE_RACE_SK(ch, skill_table[sn] ) )
{
  do_say(salesman, "You don't know that Skill!" );
  return;
}

ch->pcdata->learned[sn] = 100;
ch->pcdata->skill100s++;
do_say( salesman, "Skill/Spell set to 100%");
charge_glory( ch, PRIZE_SKILL_100 );
return;
}

/*
 * Grab the data for an object specified by name off a character --GW
 */
OBJ_DATA *find_obj_by_name( char *name, CHAR_DATA *ch )
{
OBJ_DATA *obj;

   if ( ( obj = get_obj_carry( ch, name ) ) == NULL )
   {
        if ( ( obj = get_obj_wear( ch, name ) ) == NULL )
        {
        return NULL;
        }
   }

return obj;
}

/*
 * Checks if they have enuf glory --GW
 * True if they do
 */
bool cost_check( CHAR_DATA *ch, sh_int prize_num )
{

if ( glory_table[prize_num].gold == FALSE )
{
  if ( ch->pcdata->quest_curr < glory_table[prize_num].price )
  return FALSE;
}

if ( glory_table[prize_num].gold == TRUE )
{
  if ( ch->gold < glory_table[prize_num].price )
  return FALSE;
}
    
return TRUE;
}

void charge_glory( CHAR_DATA *ch, sh_int prize_num )
{
if ( glory_table[prize_num].gold == FALSE )
ch->pcdata->quest_curr -= glory_table[prize_num].price;
else
ch->gold -= glory_table[prize_num].price;
return;
}

