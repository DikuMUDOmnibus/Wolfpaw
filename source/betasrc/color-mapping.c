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
 *			Player Color Mapping Module			    *
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

char *colormap_parse( CHAR_DATA *ch, int map_type, bool advanced, char *speaker, char *string );
void create_map(CHAR_DATA *ch,int map_type,int color_num,bool string,char *txt);
void free_char_colormaps( CHAR_DATA *ch );
void load_char_cmap( CHAR_DATA *ch, char *argument );
void save_char_cmaps( FILE *fp, CHAR_DATA *ch );
int get_map_type( char *name, bool immortal );
int get_color_num( char *color );
bool colorstringcheck( int map_type );
void setup_char_color_maps( CHAR_DATA *ch );
void set_cmap_bv( CHAR_DATA *ch, int map_type );
void remove_cmap_bv( CHAR_DATA *ch, int map_type );
void delete_char_color_map( CHAR_DATA *ch, int map_type );
bool colormap_check( CHAR_DATA *ch, int map_type );
bool advanced_map_name_check( char *argument );
char *advanced_map_convert( char *argument );
int get_cmap_bv_by_type( int map_type );
CMAP_DATA *find_char_map_by_type(CHAR_DATA *ch, int map_type);
char *find_real_color_string(CMAP_DATA *map);
int get_channel_cmap_num(int chan);
CHAR_DATA *get_char args( ( char *name ) );
int get_real_act_color_num(int orig);
char *get_map_name_by_num( int num );
char *get_color_word_by_num( int num );

/*
 * Main Control Function, lets the player configure everything --GW
 * Syntax:
 * Color on <-- Activate Color, same as the ANSI command did at one point
 * Color off <-- DeActivate Color, again same as the ANSI command did
 * Color map <map name> <color or string> <-- Map Something
 */
void do_color( CHAR_DATA *ch, char *argument )
{
char arg1[MIL];
char arg2[MIL];

argument = one_argument( argument, arg1 );
argument = one_argument( argument, arg2 );

if ( IS_NPC(ch) )
   return;

/* 
 * Activate Color 
 */
if ( !str_cmp( arg1, "on" ) )
{
   SET_BIT( ch->act, PLR_ANSI );
   send_to_char("Color Activated.\n\r",ch);
   return;
}

/*
 * Deactivate Color
 */
if ( !str_cmp( arg1, "off" ) )
{
    REMOVE_BIT( ch->act, PLR_ANSI );
    send_to_char("Color Deactivated.\n\r",ch);
    return;
}

/*
 * Map Something to another color, or set of colors
 * Includes support for basic things, ie: room descriptions
 * aswell as advanced mapping of Channel Names, and colors.
 */
if ( !str_cmp( arg1, "map" ) )
{
int map_type=0;
bool string=FALSE;
char color_string[MSL];
int color_num=0;
bool immortal=FALSE;

if ( IS_IMMORTAL(ch) )
immortal=TRUE;

/* What Color are we mapping? */
map_type=get_map_type(arg2,immortal);

/*Invalid?*/
if ( map_type==0 )
{
send_to_char("That is not a Map name!\n\r",ch);
return;
}

if ( !str_cmp( argument, "off" ) )
{
remove_cmap_bv(ch,map_type);
delete_char_color_map(ch,map_type);
send_to_char("Color Remap Deleted.\n\r",ch);
return;
}

/* Color Number, or a string? */
if ( (color_num=get_color_num(argument))==0 )
{
string=TRUE;
sprintf(color_string,"%s",argument);
}

/* Are we supposed to use a COLOR or a string here? */
if ( string && colorstringcheck(map_type)==FALSE )
{
send_to_char("You must use a valid Color!\n\r",ch);
return;
}

/* Check to see if its a FIRST time map, or a REmapping */
if ( colormap_check(ch, map_type)==TRUE )
{
  delete_char_color_map(ch,map_type);
  remove_cmap_bv(ch, map_type);
  send_to_char("Already Remapped, Old mapping Deleted.\n\r",ch);
}

/* handle simple mapping, single colors only */
if ( !string )
create_map( ch, map_type, color_num, FALSE, NULL );

/* Advanced Maps */
if ( string )
{
  if ( advanced_map_name_check(color_string)==FALSE )
  {
    send_to_char("The map string must contain an @@ sign\n\r",ch);
    send_to_char("representing the name of the character speaking.\n\r",ch);
    return;
  }
create_map( ch, map_type, 0, TRUE, color_string );
}

set_cmap_bv(ch,map_type);
send_to_char("Color Remapped.\n\r",ch);
return;
}

/* If we got this far, someone needs some assitance....*/
send_to_char("Syntax: Color <on/off/map> <map name> <off/color name/string>\n\r",ch);
send_to_char("------\n\r\n\r",ch);
send_to_char("Map Name being one of:\n\r",ch);
send_to_char("rumor tell play quest war music ask flame shout yell\n\r",ch);
send_to_char("clan council avatar tells roomdesc roomname socials exits obj mob\n\r",ch);
send_to_char("FOR DM's: immtalk imp olympus newbiechat think\n\r",ch);
send_to_char("-------\n\r\n\r",ch);
send_to_char("Color Names being one of:\n\r",ch);
send_to_char("Black Blood Green Orange DBlue Purple Cyan Grey DGrey LRed\n\r",ch);
send_to_char("LGreen Yellow Blue LBlue White\n\r",ch);
/* Not Done yet .. do it later..--GW
send_to_char("For Background Colors:\n\r",ch);
send_to_char("^Black ^Green ^Blue ^Cyan ^Red ^Orange ^Purple ^Grey\n\r",ch);
send_to_char("-------\n\r",ch);
send_to_char("NOTE: When setting background and regular colors, the BACKGROUND\n\r",ch);
send_to_char("Color MUST come AFTER the regular!\n\r",ch);
*/
send_to_char("For details see HELP COLOR, and also, for setting strings of text\n\r",ch);
send_to_char("Please use the regular color codes available in HELP PCOLORS\n\r",ch);
return;
}

/*
 * Parses the string, and sends back whats needed, this is a MAIN function
 * as everything is sent here to be looked at, if the player has mapped
 * this to a string (for channels) it will return that string, if the
 * player has only specified a color (room names, room descriptions)
 * it will return the currect color for that group of text --GW
 */
char *colormap_parse( CHAR_DATA *ch, int map_type, bool advanced, char *speaker,char *string )
{
CMAP_DATA *map;
char *color_string;
char buf[MSL];
char *pnt;
CHAR_DATA *vch;
char *speak=NULL;

map=find_char_map_by_type(ch, map_type);

/* make Sure can_see is obeyed! --GW */
if (advanced && speaker)
{
 vch=get_char(speaker);
 if ( !can_see( ch, vch ) )
   speak=STRALLOC("Someone");
  else
   speak=speaker;
}

if (!advanced)
{
  color_string=find_real_color_string(map);
  sprintf( buf, "%s%s&w", color_string,string );
}

if (advanced)
{
 if (speaker)
  sprintf(buf,map->remap_string,speak);
 else
  sprintf(buf,map->remap_string,"");
}

pnt = STRALLOC(buf);
return pnt;
}

/*
 * Create a Map in the players data Structure, and set it up --GW 
 */
void create_map(CHAR_DATA *ch,int map_type,int color_num,bool string,char *txt )
{
CMAP_DATA *map=NULL;

if ( !ch->pcdata->color_maps )
   setup_char_color_maps(ch);

CREATE( map, CMAP_DATA, 1 );

if ( string )
{
  txt=advanced_map_convert(txt);
  map->remap=1;
  map->remap_string=STRALLOC(txt);
}

map->mapped_color=color_num;
map->map_type=map_type;

LINK( map, ch->pcdata->color_maps->first_map,ch->pcdata->color_maps->last_map, next, prev );
return;
}

/*
 * Free a Chars Color Maps --GW
 */
void free_char_colormaps( CHAR_DATA *ch )
{
CMAP_DATA *map, *map_next;

for ( map=ch->pcdata->color_maps->first_map; map; map = map_next )
{
  map_next = map->next;
  
  if ( map->remap_string )
    STRFREE( map->remap_string );

  UNLINK( map, ch->pcdata->color_maps->first_map,ch->pcdata->color_maps->last_map, next, prev );
  DISPOSE(map);
}

DISPOSE(ch->pcdata->color_maps);
return;
}

/*
 * Loads a Color Map --GW
 */
void load_char_cmap( CHAR_DATA *ch, char *argument )
{
char arg1[MIL];
char arg2[MIL];
char arg3[MIL];
sh_int remap=0;
int mapped_color=0;
int map_type=0;
char *txt;
bool string=FALSE;
int cnt=0;

argument = one_argument( argument, arg1 ); /* Remap */
argument = one_argument( argument, arg2 ); /* Mapped_color */
argument = one_argument( argument, arg3 ); /* map_type */
/* Left over is Remap String */

/* Setup Vars */
remap=atoi(arg1);
mapped_color=atoi(arg2);
map_type=atoi(arg3);
for( cnt=0; argument[cnt]; cnt++ )
{
 if ( argument[cnt] && argument[cnt+1] && argument[cnt+2] )
   if ( argument[cnt] == '*' && argument[cnt+1] == '!' && argument[cnt+2] == '*' )
     argument[cnt] = '\0';
}
txt=argument;

if (mapped_color==0)
string=TRUE;

create_map(ch,map_type,mapped_color,string,txt);
return;
}

/*
 * Saves all the Characters Color Maps --GW 
 */
void save_char_cmaps( FILE *fp, CHAR_DATA *ch )
{
CMAP_DATA *map, *map_next;

if ( !ch->pcdata->color_maps )
   setup_char_color_maps(ch);

for ( map=ch->pcdata->color_maps->first_map; map; map = map_next )
{
  map_next=map->next;
  fprintf(fp,"CMAP %d %d %d %s*!*\n",map->remap, map->mapped_color,
	map->map_type, map->remap_string);
}

return;
}

/*
 * Get a Map Type from the Keyword for it --GW
 */
int get_map_type( char *name, bool immortal )
{
int map_num=0;

switch( tolower(name[0]) )
{
  /* Auction, Ask, Avatar */
  case 'a':
//     if ( !str_cmp( name,"auction" ) )
//	map_num=CMAP_AUCTION;
     if ( !str_cmp( name,"ask" ) )
	map_num=CMAP_ASK;
     if ( !str_cmp( name,"avatar" ) )
	map_num=CMAP_AVATAR;
     break;

  /* Clan, Council */
  case 'c':
     if ( !str_cmp( name,"clan" ) )
	map_num=CMAP_CLAN;
     if ( !str_cmp( name,"council") )
	map_num=CMAP_COUNCIL;
     break;

  /* Exits */
  case 'e':
      if ( !str_cmp( name, "exits" ) )
        map_num=CMAP_EXITS;
     break;

  /* Flame */
  case 'f':
      if ( !str_cmp( name, "flame" ) )
	map_num=CMAP_FLAME;
     break;

  /* Info */
//  case 'i':
//      if ( !str_cmp( name, "info" ) )
//	map_num=CMAP_INFO;
//     break;

  /* Mob, Music */
  case 'm':
     if ( !str_cmp( name,"mob" ) )
	map_num=CMAP_MOB;
     if ( !str_cmp( name,"music" ) )
	map_num=CMAP_MUSIC;
     break;

  /* Obj */
  case 'o':
      if ( !str_cmp( name, "obj" ) )
	map_num=CMAP_OBJ;
     break;

  /* Play */
  case 'p':
      if ( !str_cmp( name, "play" ) )
	map_num=CMAP_PLAY;
     break;

  /* Quest */
  case 'q':
      if ( !str_cmp( name, "quest" ) )
	map_num=CMAP_QUEST;
     break;

  /* Rumor, RoomDesc, RoomName */
  case 'r':
     if ( !str_cmp( name,"rumor" ) )
	map_num=CMAP_RUMOR;
     if ( !str_cmp( name,"roomdesc" ) )
	map_num=CMAP_ROOMDESC;
     if ( !str_cmp( name,"roomname" ) )
	map_num=CMAP_ROOMNAME;
     break;

  /* Shout, Socials */
  case 's':
     if ( !str_cmp( name,"shout" ) )
	map_num=CMAP_SHOUT;
     if ( !str_cmp( name,"socials" ) )
	map_num=CMAP_SOCIALS;
     break;

  /* Tells */
  case 't':
      if ( !str_cmp( name, "tells" ) )
	map_num=CMAP_TELLS;
     break;

  /* War */
  case 'w':
      if ( !str_cmp( name, "war" ) )
	map_num=CMAP_WAR;
     break;

  /* Yell */
  case 'y':
      if ( !str_cmp( name, "yell" ) )
	map_num=CMAP_YELL;
     break;
}

/* Gods Only */
if ( immortal && map_num == 0 )
{
  switch( tolower(name[0]) )
  {
    /* Immtalk, Imp */
    case 'i':
     if ( !str_cmp( name,"immtalk" ) )
	map_num=CMAP_IMMTALK;
     if ( !str_cmp( name,"imp" ) )
	map_num=CMAP_IMP;
     break;

    /* Olympus */
    case 'o':
      if ( !str_cmp( name, "olympus" ) )
	map_num=CMAP_OLYMPUS;
     break;

    /* NewbieChat */
    case 'n':
      if ( !str_cmp( name, "newbiechat" ) )
	map_num=CMAP_NEWBIECHAT;
     break;

    /* Think */
    case 't':
      if ( !str_cmp( name, "think" ) )
	map_num=CMAP_THINK;
     break;
  }
}

return map_num;
}

/*
 * Get the Color Number from a name --GW
 */
int get_color_num( char *color )
{
int cnum=0;

switch( tolower(color[0]) )
{
  case '^':
	if ( !str_cmp( color, "^black" ) )
	   cnum=CMCOLOR__BLACK;
	if ( !str_cmp( color, "^green" ) )
	   cnum=CMCOLOR__GREEN;
	if ( !str_cmp( color, "^blue" ) )
	   cnum=CMCOLOR__BLUE;
	if ( !str_cmp( color, "^cyan" ) )
	   cnum=CMCOLOR__CYAN;
	if ( !str_cmp( color, "^red" ) )
	   cnum=CMCOLOR__RED;
	if ( !str_cmp( color, "^orange" ) )
	   cnum=CMCOLOR__ORANGE;
	if ( !str_cmp( color, "^purple" ) )
	   cnum=CMCOLOR__PURPLE;
	if ( !str_cmp( color, "^grey" ) )
	   cnum=CMCOLOR__GREY;
	break;

  default:
	if ( !str_cmp( color, "black" ) )
	   cnum=CMCOLOR_BLACK;
	if ( !str_cmp( color, "blood" ) )
	   cnum=CMCOLOR_BLOOD;
	if ( !str_cmp( color, "green" ) )
	   cnum=CMCOLOR_GREEN;
	if ( !str_cmp( color, "orange" ) )
	   cnum=CMCOLOR_BROWN;
	if ( !str_cmp( color, "dblue" ) )
	   cnum=CMCOLOR_DBLUE;
	if ( !str_cmp( color, "purple" ) )
	   cnum=CMCOLOR_PURPLE;
	if ( !str_cmp( color, "cyan" ) )
	   cnum=CMCOLOR_CYAN;
	if ( !str_cmp( color, "grey" ) )
	   cnum=CMCOLOR_GREY;
	if ( !str_cmp( color, "dgrey" ) )
	   cnum=CMCOLOR_DGREY;
	if ( !str_cmp( color, "lred" ) )
	   cnum=CMCOLOR_LRED;
	if ( !str_cmp( color, "lgreen" ) )
	   cnum=CMCOLOR_LGREEN;
	if ( !str_cmp( color, "yellow" ) )
	   cnum=CMCOLOR_YELLOW;
	if ( !str_cmp( color, "blue" ) )
	   cnum=CMCOLOR_BLUE;
	if ( !str_cmp( color, "lblue" ) )
	   cnum=CMCOLOR_LBLUE;
	if ( !str_cmp( color, "white" ) )
	   cnum=CMCOLOR_WHITE;
        break;
}

return cnum;
}

/*
 * Check to see if this map can be a string, or only a color --GW
 * False if its only a color
 * True is its a string.
 */
bool colorstringcheck( int map_type )
{
if ( map_type > CMAP_AVATAR && map_type < CMAP_IMMTALK )
return FALSE;  

return TRUE;
}

/*
 * Initilize a Characters Color Map Settings --GW
 */
void setup_char_color_maps( CHAR_DATA *ch )
{
CREATE( ch->pcdata->color_maps, COLORMAP_DATA, 1 );
ch->pcdata->color_maps->first_map=NULL;
ch->pcdata->color_maps->last_map=NULL;
return;
}

void set_cmap_bv( CHAR_DATA *ch, int map_type )
{
int bv=0;

bv = get_cmap_bv_by_type(map_type);

if ( bv > 0 )
SET_BIT( ch->pcdata->colormap_settings, bv );

return;
}

void remove_cmap_bv( CHAR_DATA *ch, int map_type )
{
int bv=0;

bv = get_cmap_bv_by_type(map_type);

if ( bv > 0 )
REMOVE_BIT( ch->pcdata->colormap_settings, bv );

return;
}

void delete_char_color_map( CHAR_DATA *ch, int map_type )
{
CMAP_DATA *map, *map_next;

for ( map=ch->pcdata->color_maps->first_map; map; map = map_next )
{
  map_next = map->next;

  if ( map->map_type == map_type )
  {
    if ( map->remap_string )
      STRFREE( map->remap_string );

    UNLINK( map,ch->pcdata->color_maps->first_map,ch->pcdata->color_maps->last_map, next, prev );
    DISPOSE(map);
  }

}

return;
}

/*
 * Checks to see if the character has this mapped to another color setting
 * --GW
 */
bool colormap_check( CHAR_DATA *ch, int map_type )
{
int bv=0;

if ( IS_NPC(ch) )
return FALSE;

bv = get_cmap_bv_by_type(map_type);

if ( IS_SET( ch->pcdata->colormap_settings, bv ) )
return TRUE;

return FALSE;
}

/*
 * Checks the map string for the @@ characters --GW
 */
bool advanced_map_name_check( char *argument )
{
int str=0;

    for ( str = 0; argument[str] != '\0'; str++ )
    {
    if ( argument[str] == '%' )
    return FALSE;
    }
    
    for ( str = 0; argument[str] != '\0'; str++ )
    {    
    if ( argument[str] == '@' && argument[str+1] && argument[str+1] =='@' )
       return TRUE;  
    }  
  
return FALSE;
}

char *advanced_map_convert( char *argument )
{
int str=0;

    for ( str = 0; argument[str] != '\0'; str++ )
    {    
      if ( argument[str] == '@' && argument[str+1] && argument[str+1] =='@' )
      {
         argument[str] = '%';
         argument[str+1] = 's';
      }  
    }
 
return argument;
}


int get_cmap_bv_by_type( int map_type )
{
int bv=0;

switch( map_type )
{
   case CMAP_RUMOR:
    bv=BV01;
    break;
   case CMAP_TELL:
    bv=BV02;
    break;
   case CMAP_AUCTION:
    bv=BV03;
    break;
   case CMAP_PLAY:
    bv=BV04;
    break;
   case CMAP_INFO:
    bv=BV05;
    break;
   case CMAP_QUEST:
    bv=BV06;
    break;
   case CMAP_WAR:
    bv=BV07;
    break;
   case CMAP_MUSIC:
    bv=BV08;
    break;
   case CMAP_ASK:
    bv=BV09;
    break;
   case CMAP_FLAME:
    bv=BV10;
    break;
   case CMAP_SHOUT:
    bv=BV11;
    break;
   case CMAP_YELL:
    bv=BV12;
    break;
   case CMAP_CLAN:
    bv=BV13;
    break;
   case CMAP_COUNCIL:
    bv=BV14;
    break;
   case CMAP_AVATAR:
    bv=BV15;
    break;
   case CMAP_TELLS:
    bv=BV16;
    break;
   case CMAP_ROOMDESC:
    bv=BV17;
    break;
   case CMAP_ROOMNAME:
    bv=BV18;
    break;
   case CMAP_SOCIALS:
    bv=BV19;
    break;
   case CMAP_EXITS:
    bv=BV20;
    break;
   case CMAP_OBJ:
    bv=BV21;
    break;
   case CMAP_MOB:
    bv=BV22;
    break;
   case CMAP_IMMTALK:
    bv=BV23;
    break;
   case CMAP_IMP:
    bv=BV24;
    break;
   case CMAP_OLYMPUS:
    bv=BV25;
    break;
   case CMAP_NEWBIECHAT:
    bv=BV26;
    break;
   case CMAP_THINK:
    bv=BV27;
    break;
}

return bv;
}

CMAP_DATA *find_char_map_by_type(CHAR_DATA *ch, int map_type)
{
CMAP_DATA *map, *map_next;

for ( map=ch->pcdata->color_maps->first_map; map; map = map_next )
{
  map_next = map->next;

  if ( map->map_type == map_type )
  return map;

}

return NULL;
}

char *find_real_color_string(CMAP_DATA *map)
{
char color[5];
char *pnt;

switch(map->mapped_color)
{
  case CMCOLOR_BLACK:
    strcpy(color,"&x");
    break;
  case CMCOLOR_BLOOD:
    strcpy(color,"&r");
    break;
  case CMCOLOR_GREEN:
    strcpy(color,"&g");
    break;
  case CMCOLOR_BROWN:
    strcpy(color,"&O");
    break;
  case CMCOLOR_DBLUE:
    strcpy(color,"&b");
    break;
  case CMCOLOR_PURPLE:
    strcpy(color,"&p");
    break;
  case CMCOLOR_CYAN:
    strcpy(color,"&c");
    break;
  case CMCOLOR_GREY:
    strcpy(color,"&w");
    break;
  case CMCOLOR_DGREY:
    strcpy(color,"&z");
    break;
  case CMCOLOR_LRED:
    strcpy(color,"&R");
    break;
  case CMCOLOR_LGREEN:
    strcpy(color,"&G");
    break;
  case CMCOLOR_YELLOW:
    strcpy(color,"&Y");
    break;
  case CMCOLOR_BLUE:
    strcpy(color,"&B");
    break;
  case CMCOLOR_LBLUE:
    strcpy(color,"&C");
    break;
  case CMCOLOR_WHITE:
    strcpy(color,"&W");
    break;
  case CMCOLOR__BLACK:
    strcpy(color,"^x");
    break;
  case CMCOLOR__GREEN:
    strcpy(color,"^g");
    break;
  case CMCOLOR__BLUE:
    strcpy(color,"^b");
    break;
  case CMCOLOR__CYAN:
    strcpy(color,"^c");
    break;
  case CMCOLOR__RED:
    strcpy(color,"^r");
    break;
  case CMCOLOR__ORANGE:
    strcpy(color,"^O");
    break;
  case CMCOLOR__PURPLE:
    strcpy(color,"^p");
    break;
  case CMCOLOR__GREY:
    strcpy(color,"^w");
    break;
}
pnt=STRALLOC(color);

return pnt;
}

int get_real_act_color_num(int orig)
{
int color=0;

switch(orig)
{
  case CMCOLOR_BLACK:
    color=AT_BLACK;
    break;
  case CMCOLOR_BLOOD:
    color=AT_BLOOD;
    break;
  case CMCOLOR_GREEN:
    color=AT_GREEN;
    break;
  case CMCOLOR_BROWN:
    color=AT_ORANGE;
    break;
  case CMCOLOR_DBLUE:
    color=AT_DBLUE;
    break;
  case CMCOLOR_PURPLE:
    color=AT_PURPLE;
    break;
  case CMCOLOR_CYAN:
    color=AT_CYAN;
    break;
  case CMCOLOR_GREY:
    color=AT_GREY;
    break;
  case CMCOLOR_DGREY:
    color=AT_DGREY;
    break;
  case CMCOLOR_LRED:
    color=AT_RED;
    break;
  case CMCOLOR_LGREEN:
    color=AT_GREEN;
    break;
  case CMCOLOR_YELLOW:
    color=AT_YELLOW;
    break;
  case CMCOLOR_BLUE:
    color=AT_BLUE;
    break;
  case CMCOLOR_LBLUE:
    color=AT_LBLUE;
    break;
  case CMCOLOR_WHITE:
    color=AT_WHITE;
    break;
  case CMCOLOR__BLACK:
    color=AT_BLACK;
    break;
  case CMCOLOR__GREEN:
    color=AT_GREEN;
    break;
  case CMCOLOR__BLUE:
    color=AT_BLUE;
    break;
  case CMCOLOR__CYAN:
    color=AT_CYAN;
    break;
  case CMCOLOR__RED:
    color=AT_RED;
    break;
  case CMCOLOR__ORANGE:
    color=AT_ORANGE;
    break;
  case CMCOLOR__PURPLE:
    color=AT_PURPLE;
    break;
  case CMCOLOR__GREY:
    color=AT_GREY;
    break;
}

return color;
}

int get_channel_cmap_num(int chan)
{
int cmap=0;

switch( chan )
{
  case CHANNEL_CHAT:
     cmap=CMAP_RUMOR;
     break;
  case CHANNEL_TELLS:
     cmap=CMAP_TELL;
     break;
  case CHANNEL_AUCTION:
     cmap=CMAP_AUCTION;
     break;
  case CHANNEL_IC:
     cmap=CMAP_PLAY;
     break;
  case CHANNEL_INFO:
     cmap=CMAP_INFO;
     break;
  case CHANNEL_QUEST:
     cmap=CMAP_QUEST;
     break;
  case CHANNEL_WARTALK:
     cmap=CMAP_WAR;
     break;
  case CHANNEL_MUSIC:
     cmap=CMAP_MUSIC;
     break;
  case CHANNEL_ASK:
     cmap=CMAP_ASK;
     break;
  case CHANNEL_FLAME:
     cmap=CMAP_FLAME;
     break;
  case CHANNEL_SHOUT:
     cmap=CMAP_SHOUT;
     break;
  case CHANNEL_YELL:
     cmap=CMAP_YELL;
     break;
  case CHANNEL_CLAN:
     cmap=CMAP_CLAN;
     break;
  case CHANNEL_COUNCIL:
     cmap=CMAP_COUNCIL;
     break;
  case CHANNEL_AVTALK:
     cmap=CMAP_AVATAR;
     break;
  case CHANNEL_IMMTALK:
     cmap=CMAP_IMMTALK;
     break;
  case CHANNEL_HIGHGOD:  
     cmap=CMAP_IMP;
     break;
  case CHANNEL_OLYMPUS:
     cmap=CMAP_OLYMPUS;
     break;
  case CHANNEL_NEWBIE:
     cmap=CMAP_NEWBIECHAT;
     break;
  case CHANNEL_HIGH:
     cmap=CMAP_THINK;
     break;
}

return cmap;
}

void do_colors( CHAR_DATA *ch, char *argument )
{
CMAP_DATA *map, *map_next;
int cnt=0;
char buf[MSL];
char *color;

if ( IS_NPC(ch) )
return;

send_to_char("Color Settings\n\r",ch);
send_to_char("==============\n\r",ch);

ch_printf( ch, "Color is %s.\n\r", IS_SET( ch->act, PLR_ANSI ) ? "Active" : "Disabled" );
for( map = ch->pcdata->color_maps->first_map; map; map=map_next )
{
   map_next=map->next;
  if ( map->mapped_color > 0 )
  {
    cnt++;
    color = get_color_word_by_num(map->mapped_color);
    sprintf(buf, "%-11s:  %s\n\r",get_map_name_by_num(map->remap),
	color);
    send_to_char(buf,ch);
  }
  else
  {  
    cnt++;
    sprintf(buf, "%-11s:  %s\n\r",get_map_name_by_num(map->remap),
	map->remap_string);
    send_to_char(buf,ch);
  }
}

if ( cnt > 0 )
  ch_printf( ch, "%d Color Maps Found.\n\r",cnt);
else
  send_to_char("No Color Maps Found.\n\r",ch);

return;
}

char *get_color_word_by_num( int num )
{
char *color=NULL;

switch(num)
{
 case CMCOLOR_BLACK:
	color = STRALLOC("Black");
	break;
 case CMCOLOR_BLOOD:
	color = STRALLOC("Blood");
	break;
 case CMCOLOR_GREEN:
	color = STRALLOC("Green");
	break;
 case CMCOLOR_BROWN:
	color = STRALLOC("Orange");
	break;
 case CMCOLOR_DBLUE:
	color = STRALLOC("Dark Blue");
	break;
 case CMCOLOR_PURPLE:
	color = STRALLOC("Purple");
	break;
 case CMCOLOR_CYAN:
	color = STRALLOC("Cyan");
	break;
 case CMCOLOR_GREY:
	color = STRALLOC("Grey");
	break;
 case CMCOLOR_DGREY:
	color = STRALLOC("Dark Grey");
	break;
 case CMCOLOR_LRED:
	color = STRALLOC("Light Red");
	break;
 case CMCOLOR_LGREEN:
	color = STRALLOC("Light Green");
	break;
 case CMCOLOR_YELLOW:
	color = STRALLOC("Yellow");
	break;
 case CMCOLOR_BLUE:
	color = STRALLOC("Blue");
	break;
 case CMCOLOR_LBLUE:
	color = STRALLOC("Light Blue");
	break;
 case CMCOLOR_WHITE:
	color = STRALLOC("White");
	break;
}

return color;
}

char *get_map_name_by_num( int num )
{
char *name=NULL;

switch(num)
{
  case CMAP_RUMOR:
	name = STRALLOC("Rumor");
	break;
  case CMAP_TELL:
	name = STRALLOC("Tells");
	break;
  case CMAP_AUCTION:
	name = STRALLOC("Auction");
	break;
  case CMAP_PLAY:
	name = STRALLOC("Play");
	break;
  case CMAP_INFO:
	name = STRALLOC("Info");
	break;
  case CMAP_QUEST:
	name = STRALLOC("Quest");
	break;
  case CMAP_WAR:
	name = STRALLOC("War");
	break;
  case CMAP_MUSIC:
	name = STRALLOC("Music");
	break;
  case CMAP_ASK:
	name = STRALLOC("Ask");
	break;
  case CMAP_FLAME:
	name = STRALLOC("Flame");
	break;
  case CMAP_SHOUT:
	name = STRALLOC("Shout");
	break;
  case CMAP_YELL:
	name = STRALLOC("Yell");
	break;
  case CMAP_CLAN:
	name = STRALLOC("Clan");
	break;
  case CMAP_COUNCIL:
	name = STRALLOC("Council");
	break;
  case CMAP_AVATAR:
	name = STRALLOC("Avatar");
	break;
  case CMAP_TELLS:
	name = STRALLOC("Tells");
	break;
  case CMAP_ROOMDESC:
	name = STRALLOC("RoomDesc");
	break;
  case CMAP_ROOMNAME:
	name = STRALLOC("RoomName");
	break;
  case CMAP_SOCIALS:
	name = STRALLOC("Socials");
	break;
  case CMAP_EXITS:
	name = STRALLOC("Exits");
	break;
  case CMAP_OBJ:
	name = STRALLOC("Obj");
	break;
  case CMAP_MOB:
	name = STRALLOC("Mob");
	break;
  case CMAP_IMMTALK:
	name = STRALLOC("Immtalk");
	break;
  case CMAP_IMP:
	name = STRALLOC("Imp");
	break;
  case CMAP_OLYMPUS:
	name = STRALLOC("Olympus");
	break;
  case CMAP_NEWBIECHAT:
	name = STRALLOC("Newbiechat");
	break;
  case CMAP_THINK:
	name = STRALLOC("Think");
	break;
}

return name;
}

