/*--------------------------------------------------------------------------*
 *                         ** WolfPaw 1.0 **                                *
 *--------------------------------------------------------------------------*
 *               WolfPaw 1.0 (c) 1997,1998 by Dale Corse                    *
 *--------------------------------------------------------------------------*
 *            The WolfPaw Coding Team is headed by: Greywolf                *
 *  With the Assitance from: Callinon, Dhamon, Sentra, Wyverns, Altrag      *
 *  Scryn, Thoric, Justice, Tricops and Mask.                               *
 *--------------------------------------------------------------------------*
 *		Special Quest & Game  Commands Module			    *
 *--------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "mud.h"


extern int	top_zone;
int get_kombatflag		args( ( char *flag ) );
void setup_ctf( void );

#define MIL MAX_INPUT_LENGTH

/*
 * Scatter a Mob or Object over a specified Area
 * Coded for New Years Quest, Dec 31, 1997 by Greywolf.
 * Whipped it up in 10 Mins, if that, could use improvment
 */
void do_scatter( CHAR_DATA *ch, char *argument )
{
ROOM_INDEX_DATA *pRoomIndex;
ROOM_INDEX_DATA *was_in_room;
int omzone,szone,hivnum,lowvnum,vnum,number,count=0;
char arg1[MIL];
char arg2[MIL];
char arg3[MIL];
char arg4[MIL];
char arg5[MIL];
char arg6[MIL];
char arg7[MIL];
char buf[MSL];

argument = one_argument( argument, arg1 );
argument = one_argument( argument, arg2 );
argument = one_argument( argument, arg3 );
argument = one_argument( argument, arg4 );
argument = one_argument( argument, arg5 );
argument = one_argument( argument, arg6 );
argument = one_argument( argument, arg7 );

if( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' || arg4[0] == '\0'
    || arg5[0] == '\0' || arg6[0] == '\0' || arg7[0] == '\0' )
{
send_to_char("Syntax: scatter <obj/mob> <vnum> <mob/obj zone> <low vnum> <hi vnum> <scatter zone> <# of items/mobs>\n\e",ch);
return;
}

if( !is_number(arg2) || !is_number(arg3) || !is_number(arg4)
    || !is_number(arg5) || !is_number(arg6) || !is_number(arg7) )
{
send_to_char("Syntax: scatter <obj/mob> <vnum> <mob/obj zone> <low vnum> <hi vnum> <scatter zone> <# of items/mobs>\n\e",ch);
return;
}

vnum = atoi(arg2);
omzone = atoi(arg3);
lowvnum = atoi(arg4);
hivnum = atoi(arg5);
szone = atoi(arg6);
number = atoi(arg7);

if ( ( vnum<=0 || vnum>32681 ) || (omzone<1 || omzone>top_zone) ||
     ( lowvnum>32681 || lowvnum<=0 || lowvnum>=hivnum ) ||
     ( hivnum>32681 || hivnum<=0 || hivnum<=lowvnum ) ||
     ( szone<1 || szone>top_zone ) || (number>200 || number<=0) )
{
send_to_char("One or more of the arguments is invalid.\n\r",ch);
return;
}

if( !str_cmp( arg1, "mob" ) )
{
for (count=0;count<number;count++)
{
pRoomIndex = NULL;
pRoomIndex = get_room_index( number_range( lowvnum, hivnum ),szone );

if(pRoomIndex == NULL )
{
count--;
continue;
}

if( pRoomIndex != NULL )
{
was_in_room = ch->in_room;
char_from_room(ch);
char_to_room( ch, pRoomIndex );
buf[0] = '\0';
sprintf(buf,"%d %d",vnum,omzone);
do_minvoke(ch,buf);
char_from_room(ch);
char_to_room(ch,was_in_room);
}
}
}

if( !str_cmp( arg1, "obj" ) )
{
do_drop(ch,"all");
for (count=0;count<number;count++)
{
pRoomIndex = NULL;
pRoomIndex = get_room_index( number_range( lowvnum, hivnum ),szone );

if(pRoomIndex == NULL )
{
count--;
continue;
}

if( pRoomIndex != NULL )
{
was_in_room = ch->in_room;
char_from_room(ch);
char_to_room( ch, pRoomIndex );
buf[0] = '\0';
sprintf(buf,"%d %d",vnum,omzone);
do_oinvoke(ch,buf);
do_drop(ch,"all");
char_from_room(ch);
char_to_room(ch,was_in_room);
}
}
}

ch_printf(ch,"%d %s's scattered over zone %d (%d to %d).\n\r",
	  count,arg1,szone,lowvnum,hivnum);
return;
}

/*
 * Kombat Game --GW
 */
void do_kombat( CHAR_DATA *ch, char *argument )
{
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
char arg3[MAX_INPUT_LENGTH];
char arg4[MAX_INPUT_LENGTH];
char buf[MSL];
int low_num,hi_num,value;
ROOM_INDEX_DATA *location;

argument = one_argument( argument, arg1 );
argument = one_argument( argument, arg2 );
argument = one_argument( argument, arg3 );
argument = one_argument( argument, arg4 );

if ( !kombat )
{
bug("do_Kombat: Kombat is NULL!",0);
return;
}

if ( IS_NPC(ch) )
return;

if ( !IS_IMMORTAL(ch) )
{
if ( IS_SET( ch->pcdata->flags,PCFLAG_KOMBAT) )
{
send_to_char("But you are already in a Kombat!!\n\r",ch);
return;
}
if ( !IS_SET( kombat->bits, KOMBAT_ON ) )
{
send_to_char("There is not currently a Kombat running.\n\r",ch);
return;
}
if ( !IS_SET( kombat->bits, KOMBAT_ACCEPT ) )
{
send_to_char("New Kombatants are not being accepted, try next time.\n\r",ch);
return;
}
if ( IS_ADVANCED(ch) && !IS_SET( kombat->bits, KOMBAT_ADVANCED ) )
{
send_to_char("You cannot enter this Kombat.\n\r",ch);
return;
}
if ( ( !IS_ADVANCED(ch) && IS_SET( kombat->bits, KOMBAT_ADVANCED ) )
	&& !IS_SET( kombat->bits, KOMBAT_DUAL ) )
{
send_to_char("You cannot enter this Kombat.\n\r",ch);
return;
}
if ( IS_ADVANCED(ch) && ( ch->advlevel > kombat->hi_lev ) )
{
send_to_char("You cannot enter this Kombat.\n\r",ch);
return;
}
if ( IS_ADVANCED(ch) && !IS_SET( kombat->bits, KOMBAT_DUAL ) &&
	( ch->advlevel < kombat->low_lev ) )
{
send_to_char("You cannot enter this Kombat.\n\r",ch);
return;
}
if ( !IS_ADVANCED(ch) && ( ch->level > kombat->hi_lev ) )
{
send_to_char("You cannot enter this Kombat.\n\r",ch);
return;
}
if ( !IS_ADVANCED(ch) && !IS_SET( kombat->bits, KOMBAT_ADVANCED ) &&
	( ch->level < kombat->low_lev ) )
{
send_to_char("You cannot enter this Kombat.\n\r",ch);
return;
}

SET_BIT( ch->pcdata->flags, PCFLAG_KOMBAT );
sprintf(buf,"&R<&BMortal Kombat&R>&B &R%s &Bhas &REntered &Bthe &RKombat!",
	capitalize(ch->name));
echo_to_all(AT_PLAIN,buf,ECHOTAR_ALL);
send_to_char("&BYou are transported to the &RKombat Arena!&W",ch);
char_from_room( ch );
location = get_room_index( 21440, 1 );
char_to_room( ch, location );
do_look(ch,"auto");
act(AT_MAGIC,"$n slowly fades into view..",ch,NULL,NULL,TO_ROOM);
ch->pcdata->num_kombats++;
do_save(ch,"");
return;
}

if ( ch->level < LEVEL_COUNCIL )
{
send_to_char("Immortals Cannot Enter Kombat!\n\r",ch);
return;
}

if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
{
send_to_char( "Syntax: kombat <low_lev> <hi_lev> <advanced/dual/both> <flags>\n\r",ch);

send_to_char( "\n\rKombat Statistics:\n\r",ch);

ch_printf( ch,"Active: %s\n",IS_SET( kombat->bits, KOMBAT_ON ) ? "Yes" : "No" );

if( IS_SET( kombat->bits, KOMBAT_ON ) )
{
ch_printf( ch,"Accepting: %s  Minutes Left: %d\n",
		IS_SET( kombat->bits, KOMBAT_ACCEPT ) ? "Yes" : "No",
		KOMBAT_TIME->count ? (KOMBAT_TIME->count/2) : 0 );
}

ch_printf( ch,"Flags: %s\n\r",flag_string(kombat->bits,kombat_flags));
return;
}

low_num = atoi(arg1);
hi_num = atoi(arg2);

if ( ( low_num > 50 || low_num < 1 ) || ( hi_num > 50 || hi_num < 1 ) )
{
  send_to_char("Invalid Level.\n\r",ch);
  do_kombat(ch,"");
  return;
}


if ( !str_cmp( arg3, "dual" ) )
{
SET_BIT( kombat->bits, KOMBAT_DUAL );
}
else if ( !str_cmp( arg3, "advanced" ) )
{
SET_BIT( kombat->bits, KOMBAT_ADVANCED );
}
else if ( !str_cmp( arg3, "both" ) )
{
SET_BIT( kombat->bits, KOMBAT_DUAL );
SET_BIT( kombat->bits, KOMBAT_ADVANCED );
}
else
{
send_to_char( "Invalid Option.\n\r",ch);
do_kombat(ch,"");
return;
}

if( arg4[0] != '\0' )
{
value = get_kombatflag(arg4);

if( value < 0 || value > 31 )
{
ch_printf( ch, "Unknown flag: %s", arg3 );
return;
}
else
TOGGLE_BIT( kombat->bits, 1 << value );
}

if ( !KOMBAT_TIME )
CREATE( KOMBAT_TIME, TIMER, 1 );

KOMBAT_TIME->count = 8;
KOMBAT_TIME->type = TIMER_KOMBAT;
KOMBAT_TIME->value = 0;
kombat->hi_lev = hi_num;
kombat->low_lev = low_num;
SET_BIT( kombat->bits, KOMBAT_ON );
SET_BIT( kombat->bits, KOMBAT_ACCEPT );
sprintf(buf,"&R<&BMortal Kombat&R> &R%d %s &Bto &R%d %s&B,type '&RKombat&B' to Enter.",
	    kombat->low_lev,
	    IS_SET( kombat->bits, KOMBAT_DUAL) ? "Dual" : "Advanced",
	    kombat->hi_lev,
            IS_SET( kombat->bits, KOMBAT_ADVANCED) ? "Advanced" : "Dual" );
echo_to_all(AT_PLAIN,buf,ECHOTAR_ALL);
send_to_char("Kombat Opened!.\n\r",ch);
return;
}

int number_kombat( void )
{
CHAR_DATA *search;
int cnt=0;

for( search = first_char; search; search = search->next )
{
if ( !IS_NPC(search) && IS_SET(search->pcdata->flags, PCFLAG_KOMBAT ) )
cnt++;
}
return cnt;
}
 

/* Begin CTF Functions --GW */

/* Globals for CTF */
char *CTFLOWSTR;
char *CTFHIGHSTR;
bool CTF_ACCEPTING;
bool QUAD_CTF_HIGH;
bool ADV_CTF_HIGH;
bool DUAL_CTF_HIGH;
bool QUAD_CTF_LOW;
bool ADV_CTF_LOW;
bool DUAL_CTF_LOW;
bool CTF_ACTIVE;
int  CTF_LOW_LEV;
int  CTF_HIGH_LEV;
int  CTF_RED_FRAGS;
int  CTF_BLUE_FRAGS;
int  CTF_RED_CAPS;
int  CTF_BLUE_CAPS;
int  CTF_FRAG_LIMIT;
int  CTF_CAPTURE_LIMIT;
char *CTF_CARRY_TEMP_B;
char *CTF_CARRY_TEMP_R;
int  CTF_RED;
int  CTF_BLUE;

/* Bootup Setup of the CTF Values */
void setup_ctf( void )
{
  CTF_ACCEPTING = FALSE;
  QUAD_CTF_HIGH = FALSE;
  ADV_CTF_HIGH = FALSE;
  DUAL_CTF_HIGH = FALSE;
  QUAD_CTF_LOW = FALSE;
  ADV_CTF_LOW = FALSE;
  DUAL_CTF_LOW = FALSE;
  CTF_LOW_LEV = 0;
  CTF_HIGH_LEV = 0;
  CTF_ACTIVE = FALSE;
  CTFHIGHSTR = NULL;
  CTFLOWSTR = NULL;
  CTF_BLUE_FRAGS = 0;
  CTF_RED_FRAGS = 0;
  CTF_CAPTURE_LIMIT=0;
  CTF_FRAG_LIMIT=0;
  CTF_RED_CAPS=0;
  CTF_BLUE_CAPS=0;
  CTF_CARRY_TEMP_B=NULL;
  CTF_CARRY_TEMP_R=NULL;
  CTF_RED=0;
  CTF_BLUE=0;
return;
}

/* Announce on the CTF Channel */
void ctf_channel( char *argument )
{
char buf[MSL];

sprintf(buf,"&w&W&R[&WCTF&R]&W %s",argument);
echo_to_all(AT_PLAIN,buf,ECHOTAR_ALL);
return;
}

/* CTF Control */
/* Syntax: ctf <start/stop> <lowest level> <D/A/Q> <highest level> <D/A/Q> <#> <fraglimit/caplimit>*/
void do_ctf( CHAR_DATA *ch, char *argument )
{
char arg[MSL];
char arg2[MSL];
char arg3[MSL];
char arg4[MSL];
char arg5[MSL];
char arg6[MSL];
char arg7[MSL];
int tm=0;
extern TIMER *         CTF_TIME;

argument = one_argument( argument, arg );
argument = one_argument( argument, arg2 );
argument = one_argument( argument, arg3 );
argument = one_argument( argument, arg4 );
argument = one_argument( argument, arg5 );
argument = one_argument( argument, arg6 );
argument = one_argument( argument, arg7 );

if ( IS_SET(ch->pcdata->flagstwo, MOREPC_CTF) )
{
  CHAR_DATA *v, *v_next;

  send_to_char("Players in the CTF Quest:\n\r",ch);
  send_to_char("=========================\n\r",ch);

  for( v=first_char; v; v=v_next )
  {
    v_next = v->next;  
   
    if ( IS_SET(v->pcdata->flagstwo, MOREPC_CTF ) )
      ch_printf(ch, "%s\n\r",capitalize(v->name));
  }
 return;
}
else if ( !IS_IMMORTAL(ch) || (IS_IMMORTAL(ch) && get_trust(ch) < LEVEL_GOD) )
{
char buf[MSL];

  if ( !CTF_ACCEPTING )
  {
    send_to_char("There is currently No CTF Quests accepting players.\n\r",ch);
    return;
  }
  else /* Were Accepting */
  {
    bool allow;

     if ( IS_IMMORTAL(ch) )
     {
       send_to_char("Immortals Cannot Join a CTF Quest!\n\r",ch);
       return;
     }
     
     /* Are we in the level Range? */
     /* We start out allowing - and disallow if these checks say otherwise */
     allow=TRUE;

     /* Lowest lev */
     if ( QUAD_CTF_LOW )
     {
        if ( !IS_ADV_DUAL(ch) )
	allow=FALSE;
        else if ( ch->advlevel2 < CTF_LOW_LEV )
        allow=FALSE;
     }

     if ( ADV_CTF_LOW )
     {
        if ( !IS_ADVANCED(ch) )
	allow=FALSE;
        else if ( ch->advlevel < CTF_LOW_LEV )
        allow=FALSE;
     }

     if ( DUAL_CTF_LOW )
     {
        if ( ch->level < CTF_LOW_LEV )
        allow=FALSE;
     }

     /* Highest Lev */
     if ( QUAD_CTF_HIGH )
        if ( ch->advlevel2 > CTF_HIGH_LEV )
        allow=FALSE;
     if ( ADV_CTF_HIGH )
     {
        if ( IS_ADV_DUAL(ch) ) {
	allow=FALSE; }
        else if ( ch->advlevel > CTF_HIGH_LEV )
        { allow=FALSE; }
     }
     if ( DUAL_CTF_HIGH ) {
        if ( IS_ADVANCED(ch) ) {
	allow=FALSE; }
        else if ( ch->level > CTF_HIGH_LEV ) {
        allow=FALSE; }
     }
     if ( !allow )
     {
       send_to_char("You are not allowed to join this CTF! Sorry!\n\r",ch); 
       return;
     }

     /* Let them in! */
     SET_BIT(ch->pcdata->flagstwo, MOREPC_CTF);
     sprintf(buf,"%s has joined the CTF Quest.",capitalize(ch->name));
     ctf_channel(buf);

     /* Assign Team */
     if ( CTF_RED== 0 )
     tm=TEAM_RED;
     if ( CTF_BLUE== 0 )
     tm=TEAM_BLUE;
     if ( CTF_BLUE > CTF_RED )
     tm=TEAM_RED;
     if ( CTF_RED > CTF_BLUE )
     tm=TEAM_BLUE;
     if ( CTF_RED == CTF_BLUE )
     tm=TEAM_RED;

     if ( tm == TEAM_RED )
     CTF_RED++;
     if ( tm == TEAM_BLUE )
     CTF_BLUE++;

     ch->pcdata->team=tm;
     sprintf(buf,"%s assigned to the %s team!",capitalize(ch->name),tm == TEAM_RED ? "&RRED&W" : "&BBLUE&W" );
     ctf_channel(buf);

     send_to_char("you make your way to the CTF Arena.\n\r",ch);
     char_from_room(ch);
     char_to_room(ch,get_room_index(21440,1));
     do_look(ch,"auto");
     act(AT_PLAIN,"$n slowly fades into view..",ch,NULL,NULL,TO_ROOM);
     return;
   } /* else */
  } /* else if */
  else /* God Functions */
  {
   char *lowstr=NULL;
   char *highstr=NULL;
   char buf[MSL];
   CHAR_DATA *bluemob,*redmob=NULL;
   OBJ_DATA *blueflag,*redflag=NULL;
    
      if ( arg2[0] == '\0' || arg3[0] == '\0' || arg4[0] == '\0' || arg5[0] == '\0'  || arg6[0] == '\0' || arg7[0] == '\0' )
      {
        send_to_char("Syntax: ctf <start/stop> <lowlevel> <D/A/Q> <highlevel> <D/A/Q> <#> <fraglimit/caplimit>\n\r",ch);
        return;
      } /* if */

    if ( !str_cmp( arg, "start" ) )
    {
      if ( arg2[0] == '\0' || arg3[0] == '\0' || arg4[0] == '\0' || arg5[0] == '\0'  || arg6[0] == '\0' || arg7[0] == '\0' )
      {
        send_to_char("Syntax: ctf <start/stop> <lowlevel> <D/A/Q> <highlevel> <D/A/Q> <#> <fraglimit/caplimit>\n\r",ch);
        return;
      } /* if */

      if ( CTF_ACCEPTING || CTF_ACTIVE )
      {
       send_to_char("Stop the other one first!\n\r",ch);
       return;
      } /* if */

      setup_ctf();
      CTF_LOW_LEV = atoi(arg2);
      CTF_HIGH_LEV = atoi(arg4);
      switch( arg3[0] )
      {
        case 'q':
	case 'Q': QUAD_CTF_LOW = TRUE; lowstr = STRALLOC("Quad");  break;
        case 'a':
	case 'A': ADV_CTF_LOW = TRUE;  lowstr = STRALLOC("Advanced"); break;
        case 'd':
	case 'D': DUAL_CTF_LOW = TRUE; lowstr = STRALLOC("Dual Class"); break;
	default:
	  send_to_char("Invalid Level Type! Try again!!",ch);
	  return;
      } /* switch */
      switch( arg5[0] )
      {
 	case 'q':
	case 'Q': QUAD_CTF_HIGH = TRUE; highstr = STRALLOC("Quad"); break;
	case 'a':
	case 'A': ADV_CTF_HIGH = TRUE; highstr = STRALLOC("Advanced"); break;
	case 'd':
	case 'D': DUAL_CTF_HIGH = TRUE; highstr = STRALLOC("Dual Class"); break;
	default:
	  send_to_char("Invalid Level Type! Try again!!",ch);
	  return;
      } /* switch */


     if ( !str_cmp( arg7,"fraglimit" ) )
       CTF_FRAG_LIMIT = atoi(arg6);
     else if ( !str_cmp( arg7,"capturelimit" ) )
       CTF_CAPTURE_LIMIT = atoi(arg6);
     else
     {
       send_to_char("You MUST Specify ethier a frag limit, or a capture limit. Try again.\n\r",ch);
       return;
     }

      CTFHIGHSTR = highstr;
      CTFLOWSTR = lowstr;
      
      /* put the guards in the rooms */
      bluemob=create_mobile(get_mob_index(CTF_BLUE_GUARD,1),find_zone(1));
      redmob=create_mobile(get_mob_index(CTF_RED_GUARD,1),find_zone(1));
      char_to_room(bluemob,get_room_index(CTF_BLUE_BASE,1));
      char_to_room(redmob,get_room_index(CTF_RED_BASE,1));

      /* Flags */
      blueflag=create_object(get_obj_index(CTF_BLUE_FLAG,1),50,find_zone(1));
      redflag=create_object(get_obj_index(CTF_RED_FLAG,1),50,find_zone(1));
      obj_to_room(blueflag,get_room_index(CTF_BLUE_BASE,1));
      obj_to_room(redflag,get_room_index(CTF_RED_BASE,1));

      if ( !CTF_TIME )
      CREATE( CTF_TIME, TIMER, 1 );

      CTF_TIME->count = 8;
      CTF_TIME->type = TIMER_CTF;
      CTF_TIME->value = 0;

      CTF_ACTIVE = TRUE;
      CTF_ACCEPTING = TRUE;
      ctf_channel("CTF is Open!");
      sprintf(buf,"Game &R%s&W set to &R%d&W", CTF_FRAG_LIMIT > 0 ? "Frag Limit" : "Capture Limit",
        CTF_FRAG_LIMIT > 0 ? CTF_FRAG_LIMIT : CTF_CAPTURE_LIMIT );
      ctf_channel(buf);
      sprintf(buf,"Level %d %s, to level %d %s Characters, may type CTF to Join!",
	CTF_LOW_LEV,
	CTFLOWSTR,
	CTF_HIGH_LEV,
	CTFHIGHSTR);
      ctf_channel(buf);
      ctf_channel("Joining Closed in 3 minutes.");
      return;
    } /* if */

    if ( !str_cmp( arg, "stop" ) )
    {
     CHAR_DATA *v, *v_next;

     ctf_channel("The CTF Quest has been Canceled.");
     for( v=first_char; v; v = v_next )
     {
	v_next = v->next;
	if ( IS_SET(v->pcdata->flagstwo, MOREPC_CTF) )
        {
	  stop_fighting(v,TRUE);
          send_to_char("You are transefed back to Mithras.\n\r",v);
	  REMOVE_BIT(v->pcdata->flagstwo, MOREPC_CTF);
	  char_from_room(v);
	  char_to_room(v,get_room_index(30602,1));
	} /* if */
      } /* for */
    /* purge the rooms and shit --GW */
    interpret(ch,"at 21321 purge");
    interpret(ch,"at 21307 purge");

    } /* if */
     return;

    send_to_char("Syntax: ctf <start/stop> <lowlevel> <D/A/Q> <highlevel> <D/A/Q>\n\r",ch);
    return;
  } /* else */

return;
}

/* Grabber - Person's name grabbing the flag */
/* flag - TEAM_BLUE or TEAM_RED - Whos flag is in question here */
/* type - 1 = Dropped in base 2 = Grabbed it off the ground 3 = Carrier Killed */
void flag_control( CHAR_DATA *ch, int flag, int type )
{
char buf[MSL];
char *grabber=NULL;

buf[0] = '\0';
grabber = STRALLOC(capitalize(ch->name));

   if ( flag == TEAM_RED )
   {
    if ( type == 1 ) /* Dropped in Base (killed there) */
    {
      sprintf(buf,"The &RRed&W Flag Yells 'Hey! &R%s&W &YDropped&W me like a &Rbad lay&W!",grabber);
      ctf_channel(buf);
      ch->pcdata->team = TEAM_BLUE;
    }
    if ( type == 2 ) /* Grabbed the Flag */
    {
      sprintf(buf,"The &RRed&W Flag Yells 'Hey! &R%s&W &YGrabbed&W my &RPole&W!",grabber);
      CTF_CARRY_TEMP_R=STRALLOC(capitalize(grabber));
      ch->pcdata->team = RED_FLAG_CARRIER;
      ctf_channel(buf);
    }
    if ( type == 3 ) /* Dropped Outside base (killed somewhere) */
    {
      sprintf(buf,"The &RRed&W Flag Yells '&RDing Dong &Wthe &RDork &Wis &RDead&W! I'm Running &RHome&W to Momma!");
      ctf_channel(buf);
      ch->pcdata->team = TEAM_BLUE;
    }
  }

   if ( flag == TEAM_BLUE )
   {
    if ( type == 1 ) /* Dropped in Base (killed there) */
    {
      sprintf(buf,"The &BBlue&W Flag Yells 'Hey! &B%s&W &YDropped&W me like a &Bbad lay&W!",grabber);
      ctf_channel(buf);
      ch->pcdata->team = TEAM_RED;
    }
    if ( type == 2 ) /* Grabbed the Flag */
    {
      sprintf(buf,"The &BBlue&W Flag Yells 'Hey! &B%s&W &YGrabbed&W my &BPole&W!",grabber);
      CTF_CARRY_TEMP_B=STRALLOC(capitalize(ch->name));
      ch->pcdata->team = BLUE_FLAG_CARRIER;
      ctf_channel(buf);
    }
    if ( type == 3 ) /* Dropped Outside base (killed somewhere) */
    {
      sprintf(buf,"The &BBlue&W Flag Yells '&BDing Dong&W The &BDork&W is &BDead&W! I'm Running &BHome&W to Momma!");
      ctf_channel(buf);
      ch->pcdata->team = TEAM_RED;
    }

   }
}
/* End CTF Stuff --GW */
