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
 *		     Character saving and loading module		    *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include "mud.h"

/*
 * Increment with every major format change.
 */
#define SAVEVERSION	6

/*
 * Corpse Loader stuff   --GW
 */

#define LCORPSE_DIR     "../loadedcorpses/"
int cloaded;
bool CR;
ROOM_INDEX_DATA *croom;
char * get_class2    args( ( CHAR_DATA *ch ) );
char * get_advclass  args( ( CHAR_DATA *ch ) );
char * get_advclass2  args( ( CHAR_DATA *ch ) );
void load_char_cmap( CHAR_DATA *ch, char *argument );
void save_char_cmaps( FILE *fp, CHAR_DATA *ch );

/*
 * Array to keep track of equipment temporarily.		-Thoric
 */
OBJ_DATA *save_equipment[MAX_WEAR][8];
CHAR_DATA *quitting_char, *loading_char, *saving_char;

int file_ver;
CHAR_DATA *corpse_ch;

/*
 * Externals
 */
void fwrite_comments( CHAR_DATA *ch, FILE *fp );
void fread_comment( CHAR_DATA *ch, FILE *fp );

/*
 * Array of containers read for proper re-nesting of objects.
 */
static	OBJ_DATA *	rgObjNest	[MAX_NEST];

/*
 * Local functions.
 */
void	fwrite_char	args( ( CHAR_DATA *ch, FILE *fp ) );
void	fread_char	args( ( CHAR_DATA *ch, FILE *fp, bool preload) );
void	write_corpses	args( ( CHAR_DATA *ch, char *name, bool write_2 ) );
void    write_locker    args( ( CHAR_DATA *ch, char *name ) );

void read_finger( CHAR_DATA *ch, char *argument )
{
  FILE *fp;
  char buf [ MAX_STRING_LENGTH ];
  char fingload [ MAX_INPUT_LENGTH ];
  struct stat fst;
  WIZENT *wiz;
  extern WIZENT *first_wiz;

  buf[0] = '\0';
  record_call("<read_finger>");

  while( isspace(*argument) )
    argument++;

  /* This names the player finger file Joe.F in the player's directory.
   * If you want a different extension for your finger files,
   * then just change the "F" to whatever extension you want.
   * BE SURE if you change the extension here, to change it in the
   * save_finger function
   */
  sprintf( fingload, "%s%c/%s.F", FINGER_DIR, tolower(argument[0]),
                     capitalize( argument ) );

    if ( stat( fingload, &fst ) == -1 )
    {
      ch_printf(ch, "No Finger Data Found for: '%s'.\n\r", argument );
      return;
    }

//  new_fclose(fpReserve);
  if ( !( fp = fopen ( fingload, "r" ) ) )
  {
    sprintf( buf, "Load_finger_Info: fopen %s ", argument );
    bug( buf, 0 );
    perror( fingload );
    ch_printf(ch, "No Finger Data Found for: '%s'.\n\r", argument );
    return;
  }
  else
  {
    for( wiz = first_wiz; wiz; wiz = wiz->next )
    {
      if ( !str_cmp( wiz->name, argument ) && wiz->level > get_trust(ch) )
      {
        ch_printf( ch, "Information on %s is beyond your grasp!\n\r",
		capitalize(wiz->name) );
        new_fclose( fp );
        return;
      }
     }
     fread_finger ( ch, fp );
    }

  new_fclose( fp );
//  fpReserve = fopen( NULL_FILE, "r" );
  return;
}

void fread_finger ( CHAR_DATA *ch, FILE *fp )
{
  char *finger;
  char  buf[MAX_STRING_LENGTH];

  buf[0] = '\0';
  record_call("<fread_finger>");

  send_to_char("          Finger Info\n\r",ch);
  send_to_char("          -----------\n\r\n\r",ch);
  finger = fread_string( fp );
  ch_printf(ch, "%s\n\r", finger);
  return;
}

void fwrite_finger( CHAR_DATA *ch, FILE *fp )
{
char * buf1;
char * buf2;
char * buf3;
char * buf4;

buf1 = STRALLOC(get_class(ch));
buf2 = STRALLOC(get_class2(ch));
buf3 = STRALLOC(get_advclass(ch));
buf4 = STRALLOC(get_advclass2(ch));
buf1 = STRALLOC(capitalize(buf1));
buf2 = STRALLOC(capitalize(buf2));
buf3 = STRALLOC(capitalize(buf3));
buf4 = STRALLOC(capitalize(buf4));
record_call("<fwrite_finger>");

  fprintf( fp, "&CName: &W%s\n",                ch->name        );
  fprintf( fp, "&CMud Age: &W%-13d\n\r",            get_age( ch )   );
  fprintf( fp, "&CLevels: &W[&R%d&W/&R%d&W/&R%d&W/&R%d&W] [1/2/3/4]\n",ch->level,ch->level2,ch->advlevel, ch->advlevel2 );
  fprintf( fp, "&CSex: &W%s\n",   ch->sex == SEX_MALE ? "Male" :
                        ch->sex == SEX_FEMALE ? "Female" : "Neutral" );
  fprintf( fp, "&CClasses: &W[&R%s&W/&R%s&W/&R%s&W/&R%s&W]\n",
    QUICKLINK(buf1),QUICKLINK(buf2),
	QUICKLINK(buf3), QUICKLINK(buf4)); 

  fprintf( fp, "&CRace: &W%s\n",        capitalize(get_race(ch)) );
  fprintf( fp, "&CTitle: &W%s\n",                       ch->pcdata->title);
  fprintf( fp, "&CLast On: &W%s~\n", (char * ) ctime(&ch->pcdata->logon) );
  return;
}

void save_finger( CHAR_DATA *ch )
{
    FILE *fp=NULL;
    char buf [MAX_STRING_LENGTH];
    char fng [MAX_INPUT_LENGTH];
    record_call("<save_finger>");

    if( IS_NPC(ch) || ch->level < 2 )
        return;
    if ( ch->desc && ch->desc->original )
        ch = ch->desc->original;
  /* This names the player finger file Joe.F in the player's directory.
   * If you want a different extension for your finger files,
   * then just change the "F" to whatever extension you want.
   * BE SURE if you change the extension here, to change it in the
   * read_finger function
   */
    sprintf( fng, "%s%c/%s.F", FINGER_DIR, tolower(ch->name[0]),
                     capitalize( ch->name ) );
//    new_fclose(fpReserve);
    if((fp = fopen(fng, "w"))==NULL )
    {
        sprintf( buf, "Save_finger: fopen %s ", ch->name);
        bug( buf, 0 );
        perror(fng);
        return;
    }
    else
    {
        fwrite_finger( ch, fp );
        new_fclose(fp);
    }
//    fpReserve = fopen( NULL_FILE, "r" );
    return;
}
 

/*
 * Un-equip character before saving to ensure proper	-Thoric
 * stats are saved in case of changes to or removal of EQ
 */
void de_equip_char( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int x,y;
    record_call("<de_equip_char>");

    if ( !ch )
    {
	bug("DE_EQUIP_CHAR: NULL CH!",0);
	return;
    }

    for ( x = 0; x < MAX_WEAR; x++ )
	for ( y = 0; y < MAX_LAYERS; y++ )
	    save_equipment[x][y] = NULL;
    for ( obj = ch->first_carrying; obj; obj = obj->next_content )
	if ( obj->wear_loc > -1 && obj->wear_loc < MAX_WEAR )
	{
		for ( x = 0; x < MAX_LAYERS; x++ )
		   if ( !save_equipment[obj->wear_loc][x] )
		   {
			save_equipment[obj->wear_loc][x] = obj;
			break;

		if ( x == MAX_LAYERS )
		{
		    sprintf( buf, "%s had on more than %d layers of clothing in one location (%d): %s",
			ch->name, MAX_LAYERS, obj->wear_loc, obj->name );
		    bug( buf, 0 );
		}
	    }
/*	    else
	    {
	       sprintf( buf, "%s had on %s:  ch->level = %d ",
		ch->name, obj->name,
	       	ch->level);
	       bug( buf, 0 );
	    }-- Spam.. --GW*/
	    unequip_char(ch, obj);
	} 
}

/*
 * Re-equip character					-Thoric
 */
void re_equip_char( CHAR_DATA *ch )
{
    int x,y;
    record_call("<re_equip_char>");

    for ( x = 0; x < MAX_WEAR; x++ )
	for ( y = 0; y < MAX_LAYERS; y++ )
	   if ( save_equipment[x][y] != NULL )
	   {
		if ( quitting_char != ch )
		   equip_char(ch, save_equipment[x][y], x);
		save_equipment[x][y] = NULL;
	   }
	   else
		break;
}


/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void save_char_obj( CHAR_DATA *ch )
{
    char strsave[MSL];
    char strback[MSL];
    char buf[MSL];
    FILE *fp=NULL;
    int no_limit=FALSE;

    record_call("<save_char_obj>");
    if ( !ch )
    {
	bug( "Save_char_obj: null ch!", 0 );
	return;
    }

    if ( IS_NPC(ch) || ch->level < 2 )
	return;

    saving_char = ch;
    /* save pc's clan's data while we're at it to keep the data in sync */
    if ( !IS_NPC(ch) && ch->pcdata->clan )
	save_clan( ch->pcdata->clan );

    /* save deity's data to keep it in sync -ren */ 
/*    if ( !IS_NPC(ch) && ch->pcdata->deity )
	save_deity( ch->pcdata->deity );
*/
/*    if ( ch->desc && ch->desc->original )
	ch = ch->desc->original;*/

    /* Save Offline Data --GW */
//    save_offline_data(ch);

    /* Topten Stuff */
    topten_check(ch);

    de_equip_char( ch );
    
    /* Keep the Chars NATURAL Stats .. --GW */
    ch->pcdata->nat_hp = ch->max_hit;
    ch->pcdata->nat_mana = ch->max_mana;

    ch->pcdata->save_time = current_time;
    sprintf( strsave, "%s%c/%s", PLAYER_DIR, tolower(ch->name[0]),
				 capitalize( ch->name ) );

    /*
     * Auto-backup pfile (can cause lag with high disk access situtations
     */
    if ( IS_SET( sysdata.save_flags, SV_BACKUP ) )
    {
	sprintf( strback, "%s%c/%s", BACKUP_DIR, tolower(ch->name[0]),
				 capitalize( ch->name ) );
	rename( strsave, strback );
        sprintf( buf,"gzip -f %s%c/%s",BACKUP_DIR, tolower(ch->name[0]),
				capitalize( ch->name ));
	system( buf );
    }

    /*
     * Save immortal stats, level & vnums for wizlist		-Thoric
     * and do_vnums command
     *
     * Also save the player flags so we the wizlist builder can see
     * who is a guest and who is retired.
     */
    if ( ch->level >= LEVEL_IMMORTAL )
    {
      fp=NULL;
      sprintf( strback, "%s%s", "../gods/", capitalize( ch->name ) );
      if ( ( fp = fopen( strback, "w" ) ) == NULL )
      {
	bug( "Save_god_level: fopen", 0 );
	perror( strsave );
      }
      else
      {
	fprintf( fp, "Level        %d\n", ch->level );
	fprintf( fp, "Pcflags      %d\n", ch->pcdata->flags );
	if ( ch->pcdata->r_range_lo && ch->pcdata->r_range_hi )
	  fprintf( fp, "RoomRange    %d %d\n", ch->pcdata->r_range_lo,
	  				       ch->pcdata->r_range_hi	);
	if ( ch->pcdata->o_range_lo && ch->pcdata->o_range_hi )
	  fprintf( fp, "ObjRange     %d %d\n", ch->pcdata->o_range_lo,
	  				       ch->pcdata->o_range_hi	);
	if ( ch->pcdata->m_range_lo && ch->pcdata->m_range_hi )
	  fprintf( fp, "MobRange     %d %d\n", ch->pcdata->m_range_lo,
	  				       ch->pcdata->m_range_hi	);
	new_fclose( fp );
      }
    }

    fp=NULL;
    if ( ( fp = fopen( strsave, "w" ) ) == NULL )
    {
	bug( "Save_char_obj: fopen", 0 );
	perror( strsave );
    }
    else
    {
	fwrite_char( ch, fp );
	if ( ch->pcdata->comments )                 /* comments */
	  fwrite_comments( ch, fp );        /* comments */

	/* Write Personal Socials --GW */
	save_char_socials(fp,ch);

	fprintf( fp, "#END\n" );
	new_fclose( fp );

	/* Imp/Comm/Council Morts ie: loaded eq */
	if ( IS_SET(ch->pcdata->flagstwo,MOREPC_NO_LIMIT_CNT) )
	 no_limit=TRUE;
	/* Immortals dont affect limits */
	if ( IS_IMMORTAL(ch) )
	no_limit=2;
	
	/* New Obj Loader/Saver --GW */
	  sprintf(buf,"%s%c/%s",PLAYER_OBJ_DIR,tolower(ch->name[0]),
		capitalize(ch->name));
	  fp=NULL;
	  if ( ( fp=fopen( buf, "w" ) )!=NULL )
	  {
	if ( ch->first_carrying )
	    fwrite_obj( ch, ch->last_carrying, fp, 0, OS_CARRY, no_limit );

	    fprintf( fp,"#END\n");
	    new_fclose(fp);
	  }
    }

    re_equip_char( ch );

    write_corpses(ch, NULL, FALSE);
    quitting_char = NULL;
    saving_char   = NULL;
    return;
}



/*
 * Write the char.
 */
void fwrite_char( CHAR_DATA *ch, FILE *fp )
{
    AFFECT_DATA *paf;
    int sn, track;
    int pos;
    SKILLTYPE *skill=NULL;
    record_call("<fwrite_char>");

    fprintf( fp, "#%s\n", IS_NPC(ch) ? "MOB" : "PLAYER"		);
    fprintf( fp, "Name         %s~\n",	ch->name		);
    fprintf( fp, "Name2	       %s~\n",  ch->pcdata->name	);
    fprintf( fp, "Version      %d\n",   SAVEVERSION		);
    if ( ch->short_descr && ch->short_descr[0] != '\0' )
      fprintf( fp, "ShortDescr   %s~\n",	ch->short_descr	);
    if ( ch->long_descr && ch->long_descr[0] != '\0' )
      fprintf( fp, "LongDescr    %s~\n",	ch->long_descr	);
    if ( ch->description && ch->description[0] != '\0' )
      fprintf( fp, "Description  %s~\n",	ch->description	);
    fprintf( fp, "Sex          %d\n",	ch->sex			);
    fprintf( fp, "Bank	       %ld\n",   ch->pcdata->bank	);
    fprintf( fp, "SilverBank   %ld\n",   ch->pcdata->silver_bank	);
    fprintf( fp, "GoldBank     %ld\n",   ch->pcdata->gold_bank	);
    fprintf( fp, "Bounty       %d\n",   ch->pcdata->bounty	);
    fprintf( fp, "Class        %d\n",	ch->class		);
    fprintf( fp, "Class2       %d\n",   ch->class2		);
    fprintf( fp, "Advlevel     %d\n",   ch->advlevel		);
    fprintf( fp, "Advlevel2    %d\n",   ch->advlevel2		);
    fprintf( fp, "Advclass     %d\n",   ch->advclass		);
    fprintf( fp, "Advclass2    %d\n",   ch->advclass2		);
    fprintf( fp, "Clevel       %d\n",   ch->pcdata->clevel	);
    fprintf( fp, "cperm	       %d\n",   ch->pcdata->cperm	);
    fprintf( fp, "ChallChar    %s~\n",  ch->pcdata->challenge_char_name );
    fprintf( fp, "imc	       %d\n",   ch->pcdata->imc		);
    fprintf( fp, "Race         %d\n",	ch->race		);
    fprintf( fp, "Languages    %d %d\n", ch->speaks, ch->speaking );
    fprintf( fp, "Level        %d\n",	ch->level		);
    fprintf( fp, "Level2       %d\n",   ch->level2		);
	if ( ch->pcdata->trust )
      fprintf( fp, "Trust        %d\n",	ch->pcdata->trust	);
    fprintf( fp, "Played       %d\n",
	ch->pcdata->played + (int) (current_time - ch->pcdata->logon)	);

    if ( ch->in_room )
    {
    fprintf( fp, "Room         %d\n",
	    ch->in_room->vnum );
    }

    fprintf( fp, "StartRoom    %d\n", ch->pcdata->start_room );
    fprintf( fp, "InvisStart   %d\n", ch->pcdata->invis_start );
    fprintf( fp, "HpManaMove   %d %d %d %d %d %d\n",
	ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move );
    fprintf( fp, "Nat_HP       %d\n", ch->pcdata->nat_hp	);
    fprintf( fp, "Nat_MANA     %d\n", ch->pcdata->nat_mana	);
    fprintf( fp, "Old_Hp       %d\n", ch->pcdata->old_hp	);
    fprintf( fp, "Old_Mana     %d\n", ch->pcdata->old_mana	);
    fprintf( fp, "Old_Move     %d\n", ch->pcdata->old_move	);
    fprintf( fp, "Old_Exp      %d\n", ch->pcdata->old_exp	);
    fprintf( fp, "Clssdropped  %d\n", ch->pcdata->clssdropped	);

    fprintf( fp, "Copper       %ld\n",	ch->gold		);
    fprintf( fp, "Silver       %ld\n",	ch->silver		);
    fprintf( fp, "RGold        %ld\n",	ch->real_gold		);

    fprintf( fp, "Exp          %ld\n",	ch->exp			);
    if ( ch->act )
      fprintf( fp, "Act          %d\n", ch->act			);
    if ( !xIS_EMPTY(ch->affected_by) )
      fprintf( fp, "AffectedBy   %s\n",	print_bitvector(&ch->affected_by));
    fprintf( fp, "Position     %d\n",
        ch->position == POS_FIGHTING ? POS_STANDING : ch->position );

    fprintf( fp, "Practice     %d\n",	ch->practice		);
    fprintf( fp, "SavingThrows %d %d %d %d %d\n",
    		  ch->saving_poison_death,
		  ch->saving_wand,
    		  ch->saving_para_petri,
    		  ch->saving_breath,
    		  ch->saving_spell_staff			);
    fprintf( fp, "Alignment    %d\n",	ch->alignment		);
    fprintf( fp, "Favor	       %d\n",	ch->pcdata->favor	);
    fprintf( fp, "Glory        %d\n",   ch->pcdata->quest_curr  );
    fprintf( fp, "MGlory       %d\n",   ch->pcdata->quest_accum );
    fprintf( fp, "MinorAff     %d\n",	ch->pcdata->minoraffects );
    fprintf( fp, "MajorAff     %d\n",	ch->pcdata->majoraffects );
    fprintf( fp, "MinorRes     %d\n",	ch->pcdata->minorresist );
    fprintf( fp, "MajorRes     %d\n",	ch->pcdata->majorresist );
    fprintf( fp, "Skill100s    %d\n",	ch->pcdata->skill100s );
    fprintf( fp, "GloryStatus  %d\n",	ch->pcdata->glory_status );
    fprintf( fp, "Stock1       %d\n",   ch->pcdata->shares[1] );
    fprintf( fp, "Stock2       %d\n",   ch->pcdata->shares[2] );
    fprintf( fp, "Stock3       %d\n",   ch->pcdata->shares[3] );
    fprintf( fp, "Guildlevel   %d\n",	ch->pcdata->guildlevel	);
    if (ch->pcdata->nextquest != 0)
        fprintf( fp, "QuestNext   %d\n",  ch->pcdata->nextquest   );
    else if (ch->pcdata->countdown != 0)
        fprintf( fp, "QuestNext   %d\n",  10              );
    fprintf( fp, "QuestPoints  %d\n",   ch->pcdata->questpoints );
    fprintf( fp, "Countdown    %d\n",   ch->pcdata->countdown );
    fprintf( fp, "QuestObj     %d\n",   ch->pcdata->questobj );
    fprintf( fp, "QuestMob     %d\n",   ch->pcdata->questmob );
    fprintf( fp, "QuestInRoom  %d\n",	ch->pcdata->questinroom );
    fprintf( fp, "Hitroll      %d\n",	ch->hitroll		);
    fprintf( fp, "Damroll      %d\n",	ch->damroll		);
    fprintf( fp, "Ego	       %d\n",   ch->pcdata->ego		);
    fprintf( fp, "Armor        %d\n",	ch->armor		);
    fprintf( fp, "MARating     %d\n",	ch->pcdata->martial_arts_rating	);
    fprintf( fp, "MArmor       %d\n",	ch->magical_armor	);
    fprintf( fp, "awins	       %d\n",   ch->pcdata->awins	);
    fprintf( fp, "alosses      %d\n",   ch->pcdata->alosses	);
    fprintf( fp, "plr_wager    %d\n",   ch->pcdata->plr_wager   );
    fprintf( fp, "Rroom	       %d\n",   ch->pcdata->rentroom	);
    fprintf( fp, "incog	       %d\n",   ch->pcdata->incog_level );
    fprintf( fp, "NumKombats   %d\n",   ch->pcdata->num_kombats );
    fprintf( fp, "NumKombatants %d\n",  ch->pcdata->num_kombatants );
    fprintf( fp, "NumKombatsWon %d\n",  ch->pcdata->num_kombats_won );
    fprintf( fp, "PkChamps     %d\n",   ch->pcdata->pkchamps    );

    /* Mail forwarding fix --GW */
    if ( ch->pcdata->mail_forward == NULL || str_cmp(ch->pcdata->mail_forward, "(null)" ) )
    fprintf( fp, "MForward	%s~\n",	ch->pcdata->mail_forward);

    if ( ch->wimpy )
      fprintf( fp, "Wimpy        %d\n",	ch->wimpy		);
    if ( ch->pcdata->deaf )
      fprintf( fp, "Deaf         %d\n",	ch->pcdata->deaf	);
    if ( ch->resistant )
      fprintf( fp, "Resistant    %d\n",	ch->resistant		);
    if ( ch->immune )
      fprintf( fp, "Immune       %d\n",	ch->immune		);
    if ( ch->susceptible )
      fprintf( fp, "Susceptible  %d\n",	ch->susceptible		);
    if ( ch->pcdata && ch->pcdata->outcast_time )
      fprintf( fp, "Outcast_time %ld\n",ch->pcdata->outcast_time );
    if ( ch->pcdata && ch->pcdata->restore_time )
      fprintf( fp, "Restore_time %ld\n",ch->pcdata->restore_time );
    if ( ch->mental_state != -10 )
      fprintf( fp, "Mentalstate  %d\n",	ch->mental_state	);

    if ( IS_NPC(ch) )
    {
	fprintf( fp, "Vnum         %d\n",	ch->pIndexData->vnum	);
        fprintf( fp, "Mobinvis     %d\n",	ch->mobinvis		);
    }
    else
    {
        fprintf( fp, "Email        %s~\n",      ch->pcdata->email       );
	fprintf( fp, "Password     %s~\n",	ch->pcdata->pwd		);
	if ( ch->pcdata->bamfin && ch->pcdata->bamfin[0] != '\0' )
	  fprintf( fp, "Bamfin       %s~\n",	ch->pcdata->bamfin	);
	if ( ch->pcdata->bamfout && ch->pcdata->bamfout[0] != '\0' )
	  fprintf( fp, "Bamfout      %s~\n",	ch->pcdata->bamfout	);
	if ( ch->pcdata->rank && ch->pcdata->rank[0] != '\0' )
	  fprintf( fp, "Rank         %s~\n",	ch->pcdata->rank	);
	if ( ch->pcdata->bestowments && ch->pcdata->bestowments[0] != '\0' )
	  fprintf( fp, "Bestowments  %s~\n", 	ch->pcdata->bestowments );
	fprintf( fp, "Title        %s~\n",	ch->pcdata->title	);
	if ( ch->pcdata->homepage && ch->pcdata->homepage[0] != '\0' )
	  fprintf( fp, "Homepage     %s~\n",	ch->pcdata->homepage	);
	if ( ch->pcdata->bio && ch->pcdata->bio[0] != '\0' )
	  fprintf( fp, "Bio          %s~\n",	ch->pcdata->bio 	);
	if ( ch->pcdata->authed_by && ch->pcdata->authed_by[0] != '\0' )
	  fprintf( fp, "AuthedBy     %s~\n",	ch->pcdata->authed_by	);
	if ( ch->pcdata->min_snoop )
	  fprintf( fp, "Minsnoop     %d\n",	ch->pcdata->min_snoop	);
	if ( ch->pcdata->prompt && *ch->pcdata->prompt )
	  fprintf( fp, "Prompt       %s~\n",	ch->pcdata->prompt	);
	if ( ch->pcdata->pagerlen != 24 )
	  fprintf( fp, "Pagerlen     %d\n",	ch->pcdata->pagerlen	);
	if ( IS_IMMORTAL( ch ) )
	{
	  fprintf( fp, "WizInvis     %d\n", ch->pcdata->wizinvis );
	  if ( ch->pcdata->r_range_lo && ch->pcdata->r_range_hi )
	    fprintf( fp, "RoomRange    %d %d\n", ch->pcdata->r_range_lo,
	  					 ch->pcdata->r_range_hi	);
	  if ( ch->pcdata->o_range_lo && ch->pcdata->o_range_hi )
	    fprintf( fp, "ObjRange     %d %d\n", ch->pcdata->o_range_lo,
	  					 ch->pcdata->o_range_hi	);
	  if ( ch->pcdata->m_range_lo && ch->pcdata->m_range_hi )
	    fprintf( fp, "MobRange     %d %d\n", ch->pcdata->m_range_lo,
	  					 ch->pcdata->m_range_hi	);
	}
	if ( ch->pcdata->council)
	  fprintf( fp, "Council      %s~\n", 	ch->pcdata->council_name );
        if ( ch->pcdata->deity_name && ch->pcdata->deity_name[0] != '\0' )
	  fprintf( fp, "Deity	     %s~\n",	ch->pcdata->deity_name	 );
	if ( ch->pcdata->clan_name && ch->pcdata->clan_name[0] != '\0' )
	  fprintf( fp, "Clan         %s~\n",	ch->pcdata->clan_name	);
	if ( ch->pcdata->guild_name && ch->pcdata->guild_name[0] != '\0' )
	  fprintf( fp, "Guild         %s~\n",	ch->pcdata->guild_name	);
        fprintf( fp, "Flags        %d\n",	ch->pcdata->flags	);
        fprintf( fp, "Flags2       %d\n",	ch->pcdata->flagstwo);
        if ( ch->pcdata->release_date > current_time )
            fprintf( fp, "Helled       %d %s~\n",
        	(int)ch->pcdata->release_date, ch->pcdata->helled_by );
	if ( ch->pcdata->pkills )
	  fprintf( fp, "PKills       %d\n",	ch->pcdata->pkills	);
	if ( ch->pcdata->pdeaths )
	  fprintf( fp, "PDeaths      %d\n",	ch->pcdata->pdeaths	);
 	if ( get_timer( ch , TIMER_PKILLED)       
         && ( get_timer( ch , TIMER_PKILLED) > 0 ) )
         fprintf( fp, "PTimer       %d\n",     get_timer(ch, TIMER_PKILLED));
        fprintf( fp, "MKills       %d\n",	ch->pcdata->mkills	);
	fprintf( fp, "MDeaths      %d\n",	ch->pcdata->mdeaths	);
	if ( ch->pcdata->illegal_pk )
	  fprintf( fp, "IllegalPK    %d\n",	ch->pcdata->illegal_pk	);
	fprintf( fp, "AttrPerm     %d %d %d %d %d %d %d\n",
	    ch->perm_str,
	    ch->perm_int,
	    ch->perm_wis,
	    ch->perm_dex,
	    ch->perm_con,
	    ch->perm_cha,
	    ch->perm_lck );

	fprintf( fp, "AttrMod      %d %d %d %d %d %d %d\n",
	    ch->mod_str, 
	    ch->mod_int, 
	    ch->mod_wis,
	    ch->mod_dex, 
	    ch->mod_con,
	    ch->mod_cha, 
	    ch->mod_lck );

	fprintf( fp, "Condition    %d %d %d %d\n",
	    ch->pcdata->condition[0],
	    ch->pcdata->condition[1],
	    ch->pcdata->condition[2],
	    ch->pcdata->condition[3] );
	if ( ch->desc && ch->desc->host )
            fprintf( fp, "Site         %s\n", ch->desc->host );
	else
            fprintf( fp, "Site         (Link-Dead)\n" );

	fprintf(fp,"CSettings	%d\n",ch->pcdata->colormap_settings);
	/* Write Cmaps --GW */
	save_char_cmaps(fp,ch);

	/* Write Site locks */
	save_hosts(ch,fp);

        /* write alias */
        for (pos = 0; pos < MAX_ALIAS; pos++)
        {
            if (ch->pcdata->alias[pos] == NULL
            ||  ch->pcdata->alias_sub[pos] == NULL)
                break;

            fprintf(fp,"Alias %s %s~\n",ch->pcdata->alias[pos],
                    ch->pcdata->alias_sub[pos]);
        }

        /* write ignores */
        for (pos = 0; pos < MAX_IGNORE; pos++)
        {
            if (ch->pcdata->ignore[pos] == NULL )
                break;

            fprintf(fp,"Ignore %s~\n",ch->pcdata->ignore[pos]);
        }

	
	for ( sn = 1; sn < top_sn; sn++ )
	{
	    if ( skill_table[sn]->name && ch->pcdata->learned[sn] > 0 &&
		 ( CAN_USE_SK( ch, skill_table[sn] ) || 
	           CAN_USE_RACE_SK( ch, skill_table[sn] ) ) )
		{

		  /* Fix for Mysterious 185% skill bug --GW */
		  if ( ch->pcdata->learned[sn] > 100 )
			ch->pcdata->learned[sn] = 80;

		switch( skill_table[sn]->type )
		{
		    default:
			fprintf( fp, "Skill        %d '%s'\n",
			  ch->pcdata->learned[sn], skill_table[sn]->name );
			break;
		    case SKILL_SPELL:
			fprintf( fp, "Spell        %d '%s'\n",
			  ch->pcdata->learned[sn], skill_table[sn]->name );
			break;
		    case SKILL_WEAPON:
			fprintf( fp, "Weapon       %d '%s'\n",
			  ch->pcdata->learned[sn], skill_table[sn]->name );
			break;
		    case SKILL_TONGUE:
			fprintf( fp, "Tongue       %d '%s'\n",
			  ch->pcdata->learned[sn], skill_table[sn]->name );
			break;
		}
	   }
	}
    }

    for ( paf = ch->first_affect; paf; paf = paf->next )
    {
	if ( paf->type >= 0 && (skill=get_skilltype(paf->type)) == NULL )
	    continue;

	if ( paf->type >= 0 && paf->type < TYPE_PERSONAL )
	  fprintf( fp, "AffectData   '%s' %3d %3d %3d %10s\n",
	    skill->name,
	    paf->duration,
	    paf->modifier,
	    paf->location,
	    print_bitvector(&paf->bitvector)
	    );
	else
	  fprintf( fp, "Affect       %3d %3d %3d %3d %10s\n",
	    paf->type,
	    paf->duration,
	    paf->modifier,
	    paf->location,
	    print_bitvector(&paf->bitvector)
	    );
    }

    track = URANGE( 2, ((ch->level+3) * MAX_KILLTRACK)/LEVEL_AVATAR, MAX_KILLTRACK );
    for ( sn = 0; sn < track; sn++ )
    {
	if ( ch->pcdata->killed[sn].vnum == 0 )
	  break;
	fprintf( fp, "Killed       %d %d\n",
		ch->pcdata->killed[sn].vnum,
		ch->pcdata->killed[sn].count );
    }

    fprintf( fp, "End\n\n" );
    return;
}



/*
 * Write an object and its contents.
 */
void fwrite_obj( CHAR_DATA *ch, OBJ_DATA *obj, FILE *fp, int iNest,
		 sh_int os_type, sh_int no_limit )
{
    EXTRA_DESCR_DATA *ed;
    AFFECT_DATA *paf;
    sh_int wear, wear_loc, x;
    bool NO_SAVE=FALSE;
    bool SAVEROOM=FALSE;

    record_call("<fwrite_obj>");

    if ( iNest >= MAX_NEST )
    {
	bug( "fwrite_obj: iNest hit MAX_NEST %d", iNest );
	return;
    }

    /*
     * Slick recursion to write lists backwards,
     *   so loading them will load in forwards order.
     */
    if ( obj->prev_content && os_type != OS_CORPSE && os_type != OS_LOCKER
	&& os_type != OS_SAVEROOM )
	fwrite_obj( ch, obj->prev_content, fp, iNest, OS_CARRY, no_limit );
    if ( obj->prev_content && os_type == OS_SAVEROOM )
	fwrite_obj( ch, obj->prev_content, fp, iNest, OS_SAVEROOM, FALSE );


    /*
     * Catch deleted objects					-Thoric
     */
    if ( obj_extracted(obj) )
	return;

    /*
     * Do NOT save prototype items!				-Thoric
     */
    if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
	return;

    /*
     * No Saving of LIFE_PROTECTION objects --GW
     */
    if ( obj->item_type == ITEM_LIFE_PROTECTION )
	return;

    /*
     * Nope - no keys save.. --GW
     */
    if ( obj->item_type == ITEM_KEY )
	return;

    /*
     * Save Rooms Dont Save Certain Things! --GW
     */
     if ( os_type == OS_SAVEROOM )
     {
	SAVEROOM=TRUE;
	switch( obj->item_type )
        {
	   case ITEM_KEY:	
	     NO_SAVE = TRUE;
	     break;
	   case ITEM_TRASH:	
	     NO_SAVE = TRUE;
	     break;
	   case ITEM_FOOD:	
	     NO_SAVE = TRUE;
	     break;
	   case ITEM_TREASURE:	
	     NO_SAVE = TRUE;
	     break;
	   case ITEM_CORPSE_NPC:	
	     NO_SAVE = TRUE;
	     break;
	   case ITEM_CORPSE_PC:	
	     NO_SAVE = TRUE;
	     break;
	   case ITEM_SCRAPS:	
	     NO_SAVE = TRUE;
	     break;
	   case ITEM_INSIG:	
	     NO_SAVE = TRUE;
	     break;
	   default:	
	     break;
	}

	if ( IS_SET( obj->second_flags, ITEM2_NOSAVE ) )
	NO_SAVE = TRUE;

	if ( NO_SAVE )
	return;
     }

    /* Redone to support Lockers, GW */
    /* SaveRoom Support added --GW */

    if ( os_type == OS_CORPSE )
	fprintf( fp, "#CORPSE\n" );
    else if ( os_type == OS_LOCKER )
	fprintf( fp, "#LOCKER\n" );
     else
	fprintf( fp, "#OBJECT\n" );

     if ( os_type == OS_SAVEROOM )
	   os_type = OS_CARRY;

    /* Imp/Comm/Council Morts ie: loaded eq */
    if ( no_limit == TRUE )
	    fprintf( fp, "ImpChar       YES!~\n");
    if ( no_limit == 2 )
	    fprintf( fp, "Immortal	YES!~\n");

    if ( iNest )
	fprintf( fp, "Nest         %d\n",	iNest		     );
    if ( obj->count > 1 )
	fprintf( fp, "Count        %d\n",	obj->count	     );
    if ( QUICKMATCH( obj->name, obj->pIndexData->name ) == 0 )
	fprintf( fp, "Name         %s~\n",	obj->name	     );
    if ( QUICKMATCH( obj->short_descr, obj->pIndexData->short_descr ) == 0 )
	fprintf( fp, "ShortDescr   %s~\n",	obj->short_descr     );
    if ( QUICKMATCH( obj->description, obj->pIndexData->description ) == 0 )
	fprintf( fp, "Description  %s~\n",	obj->description     );
    if ( QUICKMATCH( obj->action_desc, obj->pIndexData->action_desc ) == 0 )
	fprintf( fp, "ActionDesc   %s~\n",	obj->action_desc     );
    if ( obj->spare_desc )	
	fprintf( fp, "SpareDesc	   %s~\n",	obj->spare_desc	     );

    if ( obj->loaded_by && str_cmp( obj->loaded_by, "(null)"))
        fprintf( fp, "LoadedBy     %s~\n",   obj->loaded_by		     );
    if ( obj->modified_by && str_cmp( obj->modified_by, "(null)"))
        fprintf( fp, "ModifiedBy   %s~\n",   obj->modified_by	     );
    if ( obj->modified_date && str_cmp( obj->modified_date, "(null)"))
        fprintf( fp, "ModifiedDate %s~\n",   obj->modified_date	     );

    fprintf( fp, "Vnum         %d\n",	obj->pIndexData->vnum	     );

    if ( ( os_type == OS_CORPSE || os_type == OS_LOCKER ) && obj->in_room )
      fprintf( fp, "Room         %d\n",   obj->in_room->vnum         );
    if ( obj->extra_flags != obj->pIndexData->extra_flags )
	fprintf( fp, "ExtraFlags   %d\n",	obj->extra_flags     );
    if ( obj->second_flags != obj->pIndexData->second_flags)
	fprintf( fp, "ExtraFlags2   %d\n",	obj->second_flags     );
    if ( obj->magic_flags != obj->pIndexData->magic_flags)
	fprintf( fp, "MagicFlags   %d\n",	obj->magic_flags     );
    if ( obj->wear_flags != obj->pIndexData->wear_flags )
	fprintf( fp, "WearFlags    %d\n",	obj->wear_flags	     );
    wear_loc = -1;
    for ( wear = 0; wear < MAX_WEAR; wear++ )
	for ( x = 0; x < MAX_LAYERS; x++ )
	   if ( obj == save_equipment[wear][x] )
	   {
		wear_loc = wear;
		break;
	   }
	   else
	   if ( !save_equipment[wear][x] )
		break;
    if ( wear_loc != -1 )
	fprintf( fp, "WearLoc      %d\n",	wear_loc	     );
    if ( obj->item_type != obj->pIndexData->item_type )
	fprintf( fp, "ItemType     %d\n",	obj->item_type	     );
    if ( obj->weight != obj->pIndexData->weight )
      fprintf( fp, "Weight       %d\n",	obj->weight		     );
    if ( obj->level )
      fprintf( fp, "Level        %d\n",	obj->level		     );
    if ( obj->timer )
      fprintf( fp, "Timer        %d\n",	obj->timer		     );
    if ( obj->cost != obj->pIndexData->cost )
      fprintf( fp, "Cost         %d\n",	obj->cost		     );

    /* Upstatted EQ */
    if ( obj->dam_bonus > 0 )
      fprintf( fp, "BonusDam	 %d\n", obj->dam_bonus		     );
    if ( obj->hit_bonus > 0 )
      fprintf( fp, "BonusHit	 %d\n", obj->hit_bonus		     );
    if ( obj->ac_bonus > 0 )
      fprintf( fp, "BonusAC	 %d\n", obj->ac_bonus		     );
    if ( obj->mac_bonus > 0 )
      fprintf( fp, "BonusMAC	 %d\n", obj->mac_bonus		     );

    if ( obj->value[0] || obj->value[1] || obj->value[2]
    ||   obj->value[3] || obj->value[4] || obj->value[5] )
      fprintf( fp, "Values       %d %d %d %d %d %d\n",
	obj->value[0], obj->value[1], obj->value[2],
	obj->value[3], obj->value[4], obj->value[5]     );

    switch ( obj->item_type )
    {
    case ITEM_PILL: /* was down there with staff and wand, wrongly - Scryn */
    case ITEM_POTION:
    case ITEM_SCROLL:
	if ( IS_VALID_SN(obj->value[1]) )
	    fprintf( fp, "Spell 1      '%s'\n",
		skill_table[obj->value[1]]->name );

	if ( IS_VALID_SN(obj->value[2]) )
	    fprintf( fp, "Spell 2      '%s'\n",
		skill_table[obj->value[2]]->name );

	if ( IS_VALID_SN(obj->value[3]) )
	    fprintf( fp, "Spell 3      '%s'\n",
		skill_table[obj->value[3]]->name );

	break;

    case ITEM_STAFF:
    case ITEM_WAND:
	if ( IS_VALID_SN(obj->value[3]) )
	    fprintf( fp, "Spell 3      '%s'\n",
		skill_table[obj->value[3]]->name );

	break;
    case ITEM_SALVE:
	if ( IS_VALID_SN(obj->value[4]) )
	    fprintf( fp, "Spell 4      '%s'\n",
		skill_table[obj->value[4]]->name );

	if ( IS_VALID_SN(obj->value[5]) )
	    fprintf( fp, "Spell 5      '%s'\n",
		skill_table[obj->value[5]]->name );
	break;
    }

    for ( paf = obj->first_affect; paf; paf = paf->next )
    {
	/*
	 * Save extra object affects				-Thoric
	 */
	if ( paf->type < 0 || paf->type >= top_sn )
	{
	  fprintf( fp, "Affect       %d %d %d %d %s\n",
	    paf->type,
	    paf->duration,
	     ((paf->location == APPLY_WEAPONSPELL
	    || paf->location == APPLY_WEARSPELL
	    || paf->location == APPLY_REMOVESPELL
	    || paf->location == APPLY_STRIPSN)
	    && IS_VALID_SN(paf->modifier))
	    ? skill_table[paf->modifier]->slot : paf->modifier,
	    paf->location,
	    print_bitvector(&paf->bitvector)
	    );
	}
	else
	  fprintf( fp, "AffectData   '%s' %d %d %d %s\n",
	    skill_table[paf->type]->name,
	    paf->duration,
	     ((paf->location == APPLY_WEAPONSPELL
	    || paf->location == APPLY_WEARSPELL
	    || paf->location == APPLY_REMOVESPELL
	    || paf->location == APPLY_STRIPSN)
	    && IS_VALID_SN(paf->modifier))
	    ? skill_table[paf->modifier]->slot : paf->modifier,
	    paf->location,
	    print_bitvector(&paf->bitvector)
	    );
    }

    for ( ed = obj->first_extradesc; ed; ed = ed->next )
	fprintf( fp, "ExtraDescr   %s~ %s~\n",
	    ed->keyword, ed->description );

    fprintf( fp, "End\n\n" );

    if ( obj->first_content && !SAVEROOM )
	fwrite_obj( ch, obj->last_content, fp, iNest + 1, OS_CARRY, no_limit );
    else if ( obj->first_content && SAVEROOM )
	fwrite_obj( ch, obj->last_content, fp, iNest + 1, OS_SAVEROOM, FALSE );
    return;
}



/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj( DESCRIPTOR_DATA *d, char *name, bool preload )
{
    char strsave[MAX_INPUT_LENGTH];
    CHAR_DATA *ch=NULL;
    FILE *fp;
    bool found;
    bool compressed;
    struct stat fst;
    int i, x;
    extern FILE *fpArea;
    extern char strArea[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];

    CREATE( ch, CHAR_DATA, 1 );
    record_call("<load_char_obj>");

    for ( x = 0; x < MAX_WEAR; x++ )
	for ( i = 0; i < MAX_LAYERS; i++ )
	    save_equipment[x][i] = NULL;
    clear_char( ch );
    loading_char = ch;

    CREATE( ch->pcdata, PC_DATA, 1 );
    d->character			= ch;
    ch->desc				= d;
    ch->name				= STRALLOC( name );
    ch->act				= PLR_BLANK
					| PLR_COMBINE
					| PLR_PROMPT;
    ch->perm_str			= 13;
    ch->perm_int			= 13;
    ch->perm_wis			= 13;
    ch->perm_dex			= 13;
    ch->perm_con			= 13;
    ch->perm_cha			= 13;
    ch->perm_lck			= 13;
    ch->pcdata->condition[COND_THIRST]	= 48;
    ch->pcdata->condition[COND_FULL]	= 48;
    ch->pcdata->condition[COND_BLOODTHIRST] = 10;
    ch->pcdata->played			= 0;
    ch->pcdata->logon			= current_time;
    ch->pcdata->wizinvis		= 0;
    ch->mental_state			= -10;
    ch->mobinvis			= 0;
    for(i = 0; i < MAX_SKILL; i++)
        ch->pcdata->learned[i]		= 0;
    ch->pcdata->release_date		= 0;
    ch->pcdata->helled_by		= NULL;
    ch->saving_poison_death 		= 0;
    ch->saving_wand			= 0;
    ch->saving_para_petri		= 0;
    ch->saving_breath			= 0;
    ch->saving_spell_staff		= 0;
    ch->pcdata->comments                = NULL;    /* comments */
    ch->pcdata->pagerlen		= 24;
    found = FALSE;
    compressed = FALSE;
    strsave[0] = '\0';
    sprintf( strsave, "%s%c/%s", PLAYER_DIR, tolower(name[0]),
			capitalize( name ) );

    if ( stat( strsave, &fst ) == -1 )
    {
    strsave[0] = '\0';
    compressed = TRUE;
    sprintf( strsave, "%s%c/%s.gz", PLAYER_DIR, tolower(name[0]),
			capitalize( name ) );
    }

    if ( stat( strsave, &fst ) != -1 )
    {
	if ( compressed )
      {
      sprintf( buf, "Uncompressing Player Data for: %s (%dK)",ch->name
	,(int) fst.st_size/1024 );
	log_string_plus( buf, LOG_COMM, 59 );
	buf[0] = '\0';

    /* decompress if .gz file exists - Thx Alander */
    fp=NULL;
    if ( ( fp = fopen( strsave, "r" ) ) )
    {
        new_fclose( fp );
        sprintf( buf, "gunzip -f %s", strsave );
        system( buf );
    strsave[0] = '\0';
    sprintf( strsave, "%s%c/%s", PLAYER_DIR, tolower(name[0]),
			capitalize( name ) );
    }

      }	

      if ( fst.st_size == 0 )
      {
      sprintf( buf, "Uncompressing BACKUP Player Data for: %s (%dK)",ch->name
	,(int) fst.st_size/1024 );
	log_string_plus( buf, LOG_COMM, LEVEL_GOD );
	buf[0] = '\0';

	sprintf( strsave, "%s%c/%s", BACKUP_DIR, tolower(name[0]),
			capitalize( name ) );
    if ( stat( strsave, &fst ) == -1 )
    {
    sprintf( strsave, "%s%c/%s.gz", BACKUP_DIR, tolower(name[0]),
			capitalize( name ) );
    }
	send_to_char( "Restoring your backup player file...", ch );
    /* decompress if .gz file exists - Thx Alander */
    fp=NULL;
    if ( ( fp = fopen( strsave, "r" ) ) )
    {
        new_fclose( fp );
        sprintf( buf, "gunzip -f %s", strsave );
        system( buf );
    sprintf( strsave, "%s%c/%s", BACKUP_DIR, tolower(name[0]),
			capitalize( name ) );
    }
      }
      else
      {
	sprintf( buf, "%s player data for: %s (%dK)",
	  preload ? "Preloading" : "Loading", ch->name,
	  (int) fst.st_size/1024 );
	log_string_plus( buf, LOG_COMM, 59 );
      }
    }
    /* else no player file */
    fp=NULL;
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	int iNest;

	for ( iNest = 0; iNest < MAX_NEST; iNest++ )
	    rgObjNest[iNest] = NULL;

	found = TRUE;
	/* Cheat so that bug will show line #'s -- Altrag */
	fpArea = fp;
	strcpy(strArea, strsave);
	for ( ; ; )
	{
	    char letter;
	    char *word=NULL;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
		bug( "Load_char_obj: # not found. (%c)", letter );
		bug( name, 0 );
		break;
	    }

	    word = fread_word( fp );
	    if ( !str_cmp( word, "PLAYER" ) )
	    {
		fread_char ( ch, fp, preload );
		if ( preload )
		  break;
	    }
	    /* Changing this Around to Suit the new Object Loader --GW*/
	    else if ( file_ver < 5 )
	    {
	    if ( !str_cmp( word, "OBJECT" ) )	/* Objects	*/
	    {
		load_limited_toggle(FALSE);
		fread_obj  ( ch, fp, OS_CARRY );
		load_limited_toggle(TRUE);
	    }
	    }
	    else
	    if ( !str_cmp( word, "COMMENT") )
		fread_comment(ch, fp );		/* Comments	*/
	    else
	    if ( !str_cmp( word, "PSOCIAL" ) ) /* Psocials */
		load_char_social(fp,ch);
	    else
	    if ( !str_cmp( word, "END"    ) )	/* Done		*/
		break;
	    else
	    {
		bug( "Load_char_obj: bad section.", 0 );
		bug( name, 0 );
		break;
	    }
	}
	new_fclose( fp );
	fpArea = NULL;
	strcpy(strArea, "$");

	/* New Object Loader --GW */
	if ( file_ver > 4 && !preload )
	{
	   sprintf(buf,"%s%c/%s", PLAYER_OBJ_DIR, tolower(ch->name[0]),
		capitalize(ch->name));
    if ( stat( buf, &fst ) != -1 )
    {
     fp=NULL;
     if ( ( fp=fopen( buf, "r" ) )!=NULL)
     {
	strcpy(strArea, buf);
	sprintf( buf,"Loading Objects for: %s (%dK) (V%d)",capitalize(ch->name),
		(int) fst.st_size/1024, file_ver );
	log_string_plus( buf,LOG_COMM, 59 ); 
        fpArea = fp;
	for ( ; ; )
	{
	    char letter;
	    char *word=NULL;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
		bug( "Load_char_obj: # not found.", 0 );
		bug( name, 0 );
		break;
	    }

	    word = fread_word( fp );
	    if ( !str_cmp( word, "OBJECT" ) )
	    {
		load_limited_toggle(FALSE);
		fread_obj  ( ch, fp, OS_CARRY );
		load_limited_toggle(TRUE);
	    }
	    else if ( !str_cmp( word, "END" ) )
		break;
	    else
	    {
		bug("Fread_Player: Bad Obj File. (%s)", word);
	        bug( name, 0);
	        break;
	    }
	   }
 		new_fclose(fp);
	        fpArea = NULL;
		strcpy(strArea, "$" );
	   }     
	 }
	}

    }

    
    if ( !found )
    {
	ch->short_descr			= STRALLOC( "" );
	ch->long_descr			= STRALLOC( "" );
	ch->description			= STRALLOC( "" );
	ch->pcdata->editor		= NULL;
	ch->pcdata->clan_name		= STRALLOC( "" );
	ch->pcdata->clan		= NULL;
	ch->pcdata->guild_name		= STRALLOC( "" );
	ch->pcdata->guild		= NULL;
	ch->pcdata->council_name 	= STRALLOC( "" );
	ch->pcdata->council 		= NULL;
  	ch->pcdata->deity_name		= STRALLOC( "" );
	ch->pcdata->deity		= NULL;
	ch->pcdata->pwd			= str_dup( "" );
	ch->pcdata->bamfin		= str_dup( "" );
	ch->pcdata->bamfout		= str_dup( "" );
	ch->pcdata->rank		= str_dup( "" );
	ch->pcdata->bestowments		= str_dup( "" );
	ch->pcdata->title		= STRALLOC( "" );
	ch->pcdata->homepage		= str_dup( "" );
	ch->pcdata->bio 		= STRALLOC( "" );
	ch->pcdata->authed_by		= STRALLOC( "" );
	ch->pcdata->prompt		= STRALLOC( "" );
	ch->pcdata->r_range_lo		= 0;
	ch->pcdata->r_range_hi		= 0;
	ch->pcdata->m_range_lo		= 0;
	ch->pcdata->m_range_hi		= 0;
	ch->pcdata->o_range_lo		= 0;
	ch->pcdata->o_range_hi		= 0;
	ch->pcdata->wizinvis		= 0;
    }
    else
    {
	load_char_offline_data(ch);
	if ( !ch->pcdata->clan_name )
	{
	  ch->pcdata->clan_name	= STRALLOC( "" );
	  ch->pcdata->clan	= NULL;
	}
	if ( !ch->pcdata->clan_name )
	{
	  ch->pcdata->guild_name	= STRALLOC( "" );
	  ch->pcdata->guild	= NULL;
	}
	if ( !ch->pcdata->council_name )
	{
	  ch->pcdata->council_name = STRALLOC( "" );
	  ch->pcdata->council 	= NULL;
	}
	if ( !ch->pcdata->deity_name )
	{
	  ch->pcdata->deity_name = STRALLOC( "" );
	  ch->pcdata->deity	 = NULL;
	}
        if ( !ch->pcdata->bio )
          ch->pcdata->bio	 = STRALLOC( "" );

	if ( !ch->pcdata->authed_by )
	  ch->pcdata->authed_by	 = STRALLOC( "" );

	if ( !IS_NPC( ch ) && IS_IMMORTAL( ch ) && !preload )
	{
	  if ( ch->pcdata->wizinvis < 2 )
	    ch->pcdata->wizinvis = ch->level;
 	  assign_area( ch );
	}
	if ( file_ver > 1 )
	{
	  for ( i = 0; i < MAX_WEAR; i++ ) {
	    for ( x = 0; x < MAX_LAYERS; x++ ) {
		if ( save_equipment[i][x] )
		{
		    equip_char( ch, save_equipment[i][x], i );
		    save_equipment[i][x] = NULL;
		}
		else {
		    break; }
	}}}
	/* Must be done *AFTER* eq is worn because of wis/int modifiers */

/*	if ( !IS_IMMORTAL(ch) )
		REMOVE_BIT(ch->speaks, LANG_COMMON | race_table[ch->race].language);
	if ( countlangs(ch->speaks) < (ch->level / 10) && !IS_IMMORTAL(ch) )
	{
		int prct = 5 + (get_curr_int(ch) / 6) + (get_curr_wis(ch) / 7);

		do
		{
			int iLang;
			int lang = 1;
			int need = (ch->level / 10) - countlangs(ch->speaks);
			int prac = 2 - (get_curr_cha(ch) / 17) * (70 / prct) * need;
				
			if ( ch->practice >= prac )
				break;
			
			for ( iLang = 1; lang_array[iLang] != LANG_UNKNOWN; iLang++ )
				if ( number_range( 1, iLang ) == 1 )
					lang = iLang;
			if ( (iLang = bsearch_skill_exact( lang_names[lang], gsn_first_tongue, gsn_top_sn-1  )) < 0 )
				continue;
			if ( ch->pcdata->learned[iLang] > 0 )
				continue;
			SET_BIT(ch->speaks, lang_array[lang]);
			ch->pcdata->learned[iLang] = 70;
			ch->speaks &= VALID_LANGS;
			REMOVE_BIT(ch->speaks,
					   LANG_COMMON | race_table[ch->race].language);
		}
	}*/
    }
    loading_char = NULL;
    return found;
}



/*
 * Read in a char.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

void fread_char( CHAR_DATA *ch, FILE *fp, bool preload )
{
    char buf[MAX_STRING_LENGTH];
    char *line;
    char *word=NULL;
    int x1, x2, x3, x4, x5, x6, x7;
    int count = 0,count2 = 0;
    sh_int killcnt;
    bool fMatch=FALSE;

    file_ver = 0;
    killcnt = 0;
    record_call("<fread_char>");

if ( !preload )
{
   /* Setup Maps */
   setup_char_color_maps(ch);
   /* Setup Socials */
   setup_char_socials(ch);
   /* Setup Host lock */
   host_setup(ch);
}

    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    KEY( "Advclass",    ch->advclass,	        fread_number( fp ) );
	    KEY( "Advlevel",    ch->advlevel,		fread_number( fp ) );
	    KEY( "Advclass2",   ch->advclass2,	        fread_number( fp ) );
	    KEY( "Advlevel2",   ch->advlevel2,		fread_number( fp ) );
	    KEY( "alosses",	ch->pcdata->alosses,	fread_number( fp ) );
	    KEY( "awins",	ch->pcdata->awins,	fread_number( fp ) );
	    KEY( "Act",		ch->act,		fread_number( fp ) );
	    KEY("AffectedBy", ch->affected_by,	fread_bitvector(fp));
	    KEY( "Alignment",	ch->alignment,		fread_number( fp ) );
	    KEY( "Armor",	ch->armor,		fread_number( fp ) );

  /*          if (!str_cmp( word, "Alias"))
            {
                if (count >= MAX_ALIAS)
                {
                    fread_to_eol(fp);
                    fMatch = TRUE;
                    break;
                }

                ch->pcdata->alias[count]        = STRALLOC(fread_string(fp));
                ch->pcdata->alias_sub[count]    = STRALLOC(fread_string(fp));
                count++;
                fMatch = TRUE;
                break;
            }
*/
            if (!str_cmp( word, "Alias"))
            {
                if (count >= MAX_ALIAS)
                {
                    fread_to_eol(fp);
                    fMatch = TRUE;
                    break;
                }

                ch->pcdata->alias[count]        = STRALLOC(fread_word(fp)); 
                ch->pcdata->alias_sub[count]    = STRALLOC(fread_string(fp));
                count++;
                fMatch = TRUE;
                break;
            }

	    if ( !str_cmp( word, "Affect" ) || !str_cmp( word, "AffectData" ) )
	    {
		AFFECT_DATA *paf=NULL;

		if ( preload )
		{
		    fMatch = TRUE;
		    fread_to_eol( fp );
		    break;
		}
		CREATE( paf, AFFECT_DATA, 1 );
		if ( !str_cmp( word, "Affect" ) )
		{
		    paf->type	= fread_number( fp );
		}
		else
		{
		    int sn;
		    char *sname = fread_word(fp);

		    if ( (sn=skill_lookup(sname)) < 0 )
		    {
			if ( (sn=herb_lookup(sname)) < 0 )
			    bug( "Fread_char: unknown skill.", 0 );
			else
			    sn += TYPE_HERB;
		    }
		    paf->type = sn;
		}

		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->bitvector	= fread_bitvector( fp );
		LINK(paf, ch->first_affect, ch->last_affect, next, prev );
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "AttrMod"  ) )
	    {
		line = fread_line( fp );
		x1=x2=x3=x4=x5=x6=x7=13;
		sscanf( line, "%d %d %d %d %d %d %d",
		      &x1, &x2, &x3, &x4, &x5, &x6, &x7 );
		ch->mod_str = x1;
		ch->mod_int = x2;
		ch->mod_wis = x3;
		ch->mod_dex = x4;
		ch->mod_con = x5;
		ch->mod_cha = x6;
		ch->mod_lck = x7;
		if (!x7)
		ch->mod_lck = 0;
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "AttrPerm" ) )
	    {
		line = fread_line( fp );
		x1=x2=x3=x4=x5=x6=x7=0;
		sscanf( line, "%d %d %d %d %d %d %d",
		      &x1, &x2, &x3, &x4, &x5, &x6, &x7 );
		ch->perm_str = x1;
		ch->perm_int = x2;
		ch->perm_wis = x3;
		ch->perm_dex = x4;
		ch->perm_con = x5;
		ch->perm_cha = x6;
		ch->perm_lck = x7;
		if (!x7 || x7 == 0)
		  ch->perm_lck = 13;
		fMatch = TRUE;
		break;
	    }
	    KEY( "AuthedBy",	ch->pcdata->authed_by,	fread_string( fp ) );
	    break;

	case 'B':
	    KEY( "BonusDam",	ch->pcdata->dam_bonus,		fread_number(fp)  );
	    KEY( "BonusHit",	ch->pcdata->hit_bonus,		fread_number(fp)  );
	    KEY( "BonusAC",	ch->pcdata->ac_bonus,		fread_number(fp)  );
	    KEY( "BonusMAC",	ch->pcdata->mac_bonus,		fread_number(fp)  );
	    KEY( "Bamfin",	ch->pcdata->bamfin,	fread_string_nohash( fp ) );
	    KEY( "Bamfout",	ch->pcdata->bamfout,	fread_string_nohash( fp ) );
	    KEY( "Bestowments", ch->pcdata->bestowments, fread_string_nohash( fp ) );
	    KEY( "Bio",		ch->pcdata->bio,	fread_string( fp ) );
	    KEY( "Bank",	ch->pcdata->bank,		fread_number( fp ) );
	    KEY( "Bounty",	ch->pcdata->bounty,		fread_number( fp ) );
	    break;

	case 'C':
	    if ( !str_cmp( word, "Clan" ) )
	    {
		ch->pcdata->clan_name = fread_string( fp );

		if ( !preload
		&&   ch->pcdata->clan_name[0] != '\0'
		&& ( ch->pcdata->clan = get_clan( ch->pcdata->clan_name )) == NULL )
		{
		  sprintf( buf, "Warning: the organization %s no longer exists, and therefore you no longer\n\rbelong to that organization.\n\r",
		           ch->pcdata->clan_name );
		  send_to_char( buf, ch );
		  STRFREE( ch->pcdata->clan_name );
		  ch->pcdata->clan_name = STRALLOC( "" );
		}
		fMatch = TRUE;
		break;
	    }
	
	    if ( !str_cmp( word, "CMAP" ) )
	    {
		load_char_cmap(ch,fread_line(fp));
	        fMatch=TRUE;
		break;
	    }	   
	    KEY( "CSettings",	ch->pcdata->colormap_settings, fread_number( fp ) );
	    KEY( "Class",	ch->class,		fread_number( fp ) );
	    KEY( "Class2",	ch->class2,		fread_number( fp ) );
	    KEY( "Clssdropped", ch->pcdata->clssdropped,fread_number( fp ) );
	    KEY( "cperm",	ch->pcdata->cperm,	fread_number( fp ) );
            KEY( "Clevel",	ch->pcdata->clevel,	fread_number( fp ) );
 	    KEY( "ChallChar",   ch->pcdata->challenge_char_name, fread_string( fp ) );
	    KEY( "Countdown",   ch->pcdata->countdown,  fread_number( fp ) );
	    KEY( "Copper",	ch->gold,		fread_number( fp ) );
	    if ( !str_cmp( word, "Condition" ) )
	    {
		line = fread_line( fp );
		sscanf( line, "%d %d %d %d",
		      &x1, &x2, &x3, &x4 );
		ch->pcdata->condition[0] = x1;
		ch->pcdata->condition[1] = x2;
		ch->pcdata->condition[2] = x3;
		ch->pcdata->condition[3] = x4;
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Council" ) )
	    {
		ch->pcdata->council_name = fread_string( fp );
		if ( !preload
		&&   ch->pcdata->council_name[0] != '\0'
		&& ( ch->pcdata->council = get_council( ch->pcdata->council_name )) == NULL )
		{
		  sprintf( buf, "Warning: the council %s no longer exists, and herefore you no longer\n\rbelong to a council.\n\r",
		           ch->pcdata->council_name );
		  send_to_char( buf, ch );
		  STRFREE( ch->pcdata->council_name );
		  ch->pcdata->council_name = STRALLOC( "" );
		}
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'D':
	    KEY( "Damroll",	ch->damroll,		fread_number( fp ) );
	    KEY( "Deaf",	ch->pcdata->deaf,	fread_number( fp ) );
	    if ( !str_cmp( word, "Deity" ) )
            {
                ch->pcdata->deity_name = fread_string( fp );
 
                if ( !preload
                &&   ch->pcdata->deity_name[0] != '\0'
                && ( ch->pcdata->deity = get_deity( ch->pcdata->deity_name )) == NULL )
                {
                  sprintf( buf, "Warning: the deity %s no longer exists.\n\r",
                           ch->pcdata->deity_name );
                  send_to_char( buf, ch );
                  STRFREE( ch->pcdata->deity_name );
                  ch->pcdata->deity_name = STRALLOC( "" );
		  ch->pcdata->favor = 0;
                }
                fMatch = TRUE;
                break;
            }
	    KEY( "Description",	ch->description,	fread_string( fp ) );
	    break;

	/* 'E' was moved to after 'S' */
        case 'F':
	    KEY( "Favor",	ch->pcdata->favor,	fread_number( fp ) );
	    KEY( "Flags",	ch->pcdata->flags,	fread_number( fp ) );
	    KEY( "Flags2",	ch->pcdata->flagstwo,	fread_number( fp ) );
            break;

	case 'G':
	    KEY( "GoldBank", ch->pcdata->gold_bank,fread_number(fp));
	    KEY( "GloryStatus", ch->pcdata->glory_status,fread_number(fp));
	    KEY( "Guildlevel",  ch->pcdata->guildlevel,	 fread_number( fp ) );
	    if ( !str_cmp( word, "Guild" ) )
	    {
		ch->pcdata->guild_name = fread_string( fp );

		if ( !preload
		&&   ch->pcdata->guild_name[0] != '\0'
		&& ( ch->pcdata->guild = get_clan( ch->pcdata->guild_name)) == NULL )
		{
		  sprintf( buf, "Warning: the organization %s no longer exists, and therefore you no longer\n\rbelong to that organization.\n\r",
		           ch->pcdata->guild_name );
		  send_to_char( buf, ch );
		  STRFREE( ch->pcdata->guild_name );
		  ch->pcdata->guild_name = STRALLOC( "" );
		}
		fMatch = TRUE;
		break;
	    }

	    KEY( "Glory",       ch->pcdata->quest_curr, fread_number( fp ) );
	    KEY( "Gold",	ch->gold,		fread_number( fp ) );
            break;

	case 'H':
	    if ( !str_cmp(word, "Helled") )
	    {
	      ch->pcdata->release_date = fread_number(fp);
	      ch->pcdata->helled_by = fread_string(fp);
	      if ( ch->pcdata->release_date < current_time )
	      {
	        STRFREE(ch->pcdata->helled_by);
	        ch->pcdata->helled_by = NULL;
	        ch->pcdata->release_date = 0;
	      }
	      fMatch = TRUE;
	      break;
	    }

	    KEY( "Hitroll",	ch->hitroll,		fread_number( fp ) );
	    KEY( "Homepage",	ch->pcdata->homepage,	fread_string_nohash( fp ) );

	    if ( !str_cmp( word, "HpManaMove" ) )
	    {
		ch->hit		= fread_number( fp );
		ch->max_hit	= fread_number( fp );
		ch->mana	= fread_number( fp );
		ch->max_mana	= fread_number( fp );
		ch->move	= fread_number( fp );
		ch->max_move	= fread_number( fp );
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'I':
	    KEY( "InvisStart",	ch->pcdata->invis_start,fread_number( fp ) );
	    KEY( "IllegalPK",	ch->pcdata->illegal_pk,	fread_number( fp ) );
	    KEY( "Immune",	ch->immune,		fread_number( fp ) );
	    KEY( "imc",		ch->pcdata->imc,	fread_number( fp ) );
	    KEY( "incog",	ch->pcdata->incog_level,fread_number( fp ) );

            if (!str_cmp( word, "Ignore"))
            {
                if (count2 >= MAX_IGNORE)
                {
                    fread_to_eol(fp);
                    fMatch = TRUE;
                    break;
                }

                ch->pcdata->ignore[count2]        = STRALLOC(fread_string(fp)); 
                count2++;
                fMatch = TRUE;
                break;
            }
	    break;

	case 'K':
	    if ( !str_cmp( word, "Killed" ) )
	    {
		fMatch = TRUE;
		if ( killcnt >= MAX_KILLTRACK )
		    bug( "fread_char: killcnt (%d) >= MAX_KILLTRACK", killcnt );
		else
		{
		    ch->pcdata->killed[killcnt].vnum    = fread_number( fp );
		    ch->pcdata->killed[killcnt++].count = fread_number( fp );
		}
	    }
	    break;

	case 'L':
	    KEY( "Level",	ch->level,		fread_number( fp ) );
	    KEY( "Level2",	ch->level2,		fread_number( fp ) );
	    KEY( "LongDescr",	ch->long_descr,		fread_string( fp ) );
	    if ( !str_cmp( word, "Languages" ) )
	    {
	    	ch->speaks = fread_number( fp );
	    	ch->speaking = fread_number( fp );
	    	fMatch = TRUE;
	    }
	    break;

	case 'M':
	    KEY( "MARating",	ch->pcdata->martial_arts_rating, fread_number(fp) );
	    KEY( "MArmor",	ch->magical_armor,	fread_number( fp ) );
	    KEY( "MDeaths",	ch->pcdata->mdeaths,	fread_number( fp ) );
	    KEY( "Mentalstate", ch->mental_state,	fread_number( fp ) );
	    KEY( "MGlory",      ch->pcdata->quest_accum,fread_number( fp ) );
	    KEY( "Minsnoop",	ch->pcdata->min_snoop,	fread_number( fp ) );
	    KEY( "MKills",	ch->pcdata->mkills,	fread_number( fp ) );
	    KEY( "Mobinvis",	ch->mobinvis,		fread_number( fp ) );
	    KEY( "MinorAff",    ch->pcdata->minoraffects, fread_number( fp ) );
	    KEY( "MajorAff",    ch->pcdata->majoraffects, fread_number( fp ) );
	    KEY( "MinorRes",    ch->pcdata->minorresist, fread_number( fp ) );
	    KEY( "MajorRes",    ch->pcdata->majorresist, fread_number( fp ) );
	    KEY( "MForward",	ch->pcdata->mail_forward, fread_string( fp ) );
	    if ( !str_cmp( word, "MobRange" ) )
	    {
		ch->pcdata->m_range_lo = fread_number( fp );
		ch->pcdata->m_range_hi = fread_number( fp );
		fMatch = TRUE;
	    }
	    break;

	case 'N':
	    KEY( "Nat_HP",	ch->pcdata->nat_hp,	fread_number(fp));
	    KEY( "Nat_Mana",	ch->pcdata->nat_mana,	fread_number(fp));
	    KEY( "NumKombats",	ch->pcdata->num_kombats, fread_number(fp));
	    KEY( "NumKombatants",ch->pcdata->num_kombatants,fread_number(fp));
	    KEY( "NumKombatsWon",ch->pcdata->num_kombats_won,fread_number(fp));
	    KEY( "Name2", 	ch->pcdata->name, fread_string(fp));
	    if ( !str_cmp( word, "Name" ) )
	    {
		/*
		 * Name already set externally.
		 */
		fread_to_eol( fp );
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'O':
	    KEY( "Old_Hp",	ch->pcdata->old_hp,	fread_number( fp ) );
	    KEY( "Old_Mana",	ch->pcdata->old_mana,	fread_number( fp ) );
	    KEY( "Old_Move",	ch->pcdata->old_move,	fread_number( fp ) );
	    KEY( "Old_Exp",	ch->pcdata->old_exp,	fread_number( fp ) );
	    KEY( "Outcast_time", ch->pcdata->outcast_time, fread_number( fp ) );
	    if ( !str_cmp( word, "ObjRange" ) )
	    {
		ch->pcdata->o_range_lo = fread_number( fp );
		ch->pcdata->o_range_hi = fread_number( fp );
		fMatch = TRUE;
	    }
	    break;

	case 'P':
	    KEY( "PkChamps",    ch->pcdata->pkchamps,   fread_number( fp ) );
	    KEY( "plr_wager",   ch->pcdata->plr_wager,  fread_number( fp ) );
	    KEY( "Pagerlen",	ch->pcdata->pagerlen,	fread_number( fp ) );
	    KEY( "Password",	ch->pcdata->pwd,	fread_string_nohash( fp ) );
	    KEY( "PDeaths",	ch->pcdata->pdeaths,	fread_number( fp ) );
	    KEY( "PKills",	ch->pcdata->pkills,	fread_number( fp ) );
	    KEY( "Played",	ch->pcdata->played,	fread_number( fp ) );
	    KEY( "Position",	ch->position,		fread_number( fp ) );
	    KEY( "Practice",	ch->practice,		fread_number( fp ) );
	    KEY( "Prompt",	ch->pcdata->prompt,	fread_string( fp ) );
	    if (!str_cmp ( word, "PTimer" ) )
	    {
		add_timer( ch , TIMER_PKILLED, fread_number(fp), NULL, 0 );	
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'Q':
	    KEY( "QuestMob",    ch->pcdata->questmob,     fread_number( fp ) );
	    KEY( "QuestObj",    ch->pcdata->questobj,	  fread_number( fp ) );
	    KEY( "Questpoints", ch->pcdata->questpoints,  fread_number( fp ) );
            KEY( "QuestNext",   ch->pcdata->nextquest,    fread_number( fp ));    
	    KEY( "QuestInRoom", ch->pcdata->questinroom,  fread_number( fp ) );
	    break;

	case 'R':
	    KEY( "RGold",	ch->real_gold,		fread_number( fp ) );
	    KEY( "Race",        ch->race,		fread_number( fp ) );
	    KEY( "Rank",        ch->pcdata->rank,	fread_string_nohash( fp ) );
	    KEY( "Resistant",	ch->resistant,		fread_number( fp ) );
	    KEY( "Restore_time",ch->pcdata->restore_time, fread_number( fp ) );
	    KEY( "Rroom",	ch->pcdata->rentroom,	fread_number( fp ) );
	    if ( !str_cmp( word, "Room" ) )
	    {
		ch->pcdata->warm_to_room= fread_number( fp );
		fMatch = TRUE;
		break;
	    }
	    if ( !str_cmp( word, "RoomRange" ) )
	    {
		ch->pcdata->r_range_lo = fread_number( fp );
		ch->pcdata->r_range_hi = fread_number( fp );
		fMatch = TRUE;
	    }
	    break;

	case 'S':
	    KEY( "SilverBank",	ch->pcdata->silver_bank,fread_number( fp ) );
	    KEY( "Silver",	ch->silver,		fread_number( fp ) );
	    KEY( "StartRoom",	ch->pcdata->start_room, fread_number( fp ) );
	    KEY( "Sex",		ch->sex,		fread_number( fp ) );
	    KEY( "ShortDescr",	ch->short_descr,	fread_string( fp ) );
	    KEY( "Susceptible",	ch->susceptible,	fread_number( fp ) );
	    KEY( "Skill100s",	ch->pcdata->skill100s,	fread_number( fp ) );
	    KEY( "Stock1",ch->pcdata->shares[1],	fread_number( fp ) );
	    KEY( "Stock2",ch->pcdata->shares[2],	fread_number( fp ) );
	    KEY( "Stock3",ch->pcdata->shares[3],	fread_number( fp ) );
	    if ( !str_cmp( word, "SavingThrow" ) )
	    {
		ch->saving_wand 	= fread_number( fp );
		ch->saving_poison_death = ch->saving_wand;
		ch->saving_para_petri 	= ch->saving_wand;
		ch->saving_breath 	= ch->saving_wand;
		ch->saving_spell_staff 	= ch->saving_wand;
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "SavingThrows" ) )
	    {
		ch->saving_poison_death = fread_number( fp );
		ch->saving_wand 	= fread_number( fp );
		ch->saving_para_petri 	= fread_number( fp );
		ch->saving_breath 	= fread_number( fp );
		ch->saving_spell_staff 	= fread_number( fp );
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "SLOCK" ) )
	    {
	    char *host;
		host = fread_word(fp);
	        host[strlen(host)-1] = '\0';
		load_host(ch,host);
	        fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Site" ) )
	    {
		if ( !preload )
		{
		  sprintf( buf, "Last connected from: %s\n\r", fread_word( fp ) );
		  send_to_char( buf, ch );
		}
		else
		  fread_to_eol( fp );
		fMatch = TRUE;
		if ( preload )
		  word = "End";
		else
		  break;
	    }

	    if ( !str_cmp( word, "Skill" ) )
	    {
		int sn;
		int value;

		if ( preload )
		{
		  fMatch = TRUE;
		  word = "End";
		  break;
		}
		else
		{
		  value = fread_number( fp );
		  if ( file_ver < 3 )
		    sn = skill_lookup( fread_word( fp ) );
		  else
		    sn = bsearch_skill_exact( fread_word( fp ), gsn_first_skill, gsn_first_weapon-1 );


		  if ( sn < 0 )
		  {
		    bug( "Fread_char: unknown skill.", 0 );
		    fMatch = TRUE;
		    break;
		  }
		  else
		    ch->pcdata->learned[sn] = value;		    
	
		  if ( ch->level < LEVEL_IMMORTAL )
		    { 
		      if ( !CAN_USE_SK( ch, skill_table[sn] ) && 
			   !CAN_USE_RACE_SK(ch,skill_table[sn] ) )
		      {
		        ch->pcdata->learned[sn] = 0;
		        ch->practice++;
		      }
		    }

		  fMatch = TRUE;
		  break;
		  }
	      }

	    if ( !str_cmp( word, "Spell" ) )
	    {
		int sn;
		int value;

		if ( preload )
		{
		  fMatch = TRUE;
		  word = "End";
		  break;
		}
		else
		{
		  value = fread_number( fp );

		  sn = bsearch_skill_exact( fread_word( fp ), gsn_first_spell, gsn_first_skill-1 );

		   if ( sn < 0 )
		   {
		     bug( "Fread_char: unknown spell.", 0 );
	 	     fMatch = TRUE;
		     break;
		   }
		   else
		     ch->pcdata->learned[sn] = value;

		    if ( ch->level < LEVEL_IMMORTAL )
		    {
		      if ( !CAN_USE_SK( ch, skill_table[sn] ) &&
			   !CAN_USE_RACE_SK(ch,skill_table[sn]) )
			{
			    ch->pcdata->learned[sn] = 0;
			    ch->practice++;
			}
		   }

		  fMatch = TRUE;
		  break;
		}
	    }
	    if ( str_cmp( word, "End" ) )
		break;

	case 'E':
	    KEY( "Ego",		ch->pcdata->ego,	fread_number( fp ) );
	    if ( !str_cmp( word, "End" ) )
	    {
		if (!ch->short_descr)
		  ch->short_descr	= STRALLOC( "" );
		if (!ch->long_descr)
		  ch->long_descr	= STRALLOC( "" );
		if (!ch->description)
		  ch->description	= STRALLOC( "" );
		if (!ch->pcdata->pwd)
		  ch->pcdata->pwd	= str_dup( "" );
		if (!ch->pcdata->bamfin)
		  ch->pcdata->bamfin	= str_dup( "" );
		if (!ch->pcdata->bamfout)
		  ch->pcdata->bamfout	= str_dup( "" );
		if (!ch->pcdata->bio)
		  ch->pcdata->bio	= STRALLOC( "" );
		if (!ch->pcdata->rank)
		  ch->pcdata->rank	= str_dup( "" );
		if (!ch->pcdata->bestowments)
		  ch->pcdata->bestowments = str_dup( "" );
		if (!ch->pcdata->title)
		  ch->pcdata->title	= STRALLOC( "" );
		if (!ch->pcdata->homepage)
		  ch->pcdata->homepage	= str_dup( "" );
		if (!ch->pcdata->authed_by)
		  ch->pcdata->authed_by = STRALLOC( "" );
		if (!ch->pcdata->prompt )
		  ch->pcdata->prompt	= STRALLOC( "" );
		ch->pcdata->editor		= NULL;
		killcnt = URANGE( 2, ((ch->level+3) * MAX_KILLTRACK)/LEVEL_AVATAR, MAX_KILLTRACK );
		if ( killcnt < MAX_KILLTRACK )
		  ch->pcdata->killed[killcnt].vnum = 0;

		/* no good for newbies at all */
		if ( !IS_IMMORTAL( ch ) && !ch->speaking )
			ch->speaking = LANG_COMMON;
		/*	ch->speaking = race_table[ch->race].language; */
		if ( IS_IMMORTAL( ch ) )
		{
			ch->speaks = ~0;
			if ( ch->speaking == 0 )
				ch->speaking = ~0;
		}
		if ( !ch->pcdata->prompt )
		  ch->pcdata->prompt = STRALLOC("");
		return;
	    }
	    KEY( "Email",       ch->pcdata->email,      fread_string( fp ) );
	    KEY( "Exp",		ch->exp,		fread_number( fp ) );
	    if( !str_cmp( word, "Exp2" ) )	
	    {
	    fread_number( fp ); /* takes errors out of the log */
	    break; /* no longer used */
	    }
	   break;

	case 'T':
	    if ( !str_cmp( word, "Tongue" ) )
	    {
		int sn;
		int value;

		if ( preload )
		{
		  fMatch = TRUE;
		  word = "End";
		  break;
		}
		else
		{
		  value = fread_number( fp );

		  sn = bsearch_skill_exact( fread_word( fp ), gsn_first_tongue, gsn_top_sn-1 );


		  if ( sn < 0 )
		  {
		    bug( "Fread_char: unknown tongue.", 0 );
		    fMatch = TRUE;
		    break;
		  }
			else
		    ch->pcdata->learned[sn] = value;

		    if (( ch->level < LEVEL_IMMORTAL ) &&
		      !CAN_USE_SK( ch, skill_table[sn] ) &&
			   !CAN_USE_RACE_SK(ch,skill_table[sn]))
			{
			    ch->pcdata->learned[sn] = 0;
			    ch->practice++;
			}

		  fMatch = TRUE;
		break;
	       }
	    }

	    KEY( "Trust", ch->pcdata->trust, fread_number( fp ) );

	    if ( !str_cmp( word, "Title" ) )
	    {
		ch->pcdata->title = fread_string( fp );
		if ( isalpha(ch->pcdata->title[0])
		||   isdigit(ch->pcdata->title[0]) )
		{
		    sprintf( buf, " %s", ch->pcdata->title );
		    if ( ch->pcdata->title )
		      STRFREE( ch->pcdata->title );
		    ch->pcdata->title = STRALLOC( buf );
		}
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'V':
	    if ( !str_cmp( word, "Vnum" ) )
	    {
		ch->pIndexData = get_mob_index( fread_number( fp ),1 );
		fMatch = TRUE;
		break;
	    }
	    KEY( "Version",	file_ver,		fread_number( fp ) );
	    break;

	case 'W':
	    if ( !str_cmp( word, "Weapon" ) )
	    {
		int sn;
		int value;

		if ( preload )
		{
		  word = "End";
	          fMatch = TRUE;
		  break;
		}
		else
		{
		  value = fread_number( fp );

		  sn = bsearch_skill_exact( fread_word( fp ), gsn_first_weapon, gsn_first_tongue-1 );


		   if ( sn < 0 )
		   {
		    bug( "Fread_char: unknown weapon.", 0 );
		    fMatch = TRUE;
		    break;
		   }
			else
		    ch->pcdata->learned[sn] = value; 

		    if ( ch->level < LEVEL_IMMORTAL &&
		      !CAN_USE_SK( ch, skill_table[sn] ) &&
			   !CAN_USE_RACE_SK(ch,skill_table[sn]))
			{
			    ch->pcdata->learned[sn] = 0;
			    ch->practice++;
			}

	       fMatch = TRUE;
		break;
   	     }
	   }
	    KEY( "Wimpy",	ch->wimpy,		fread_number( fp ) );
	    KEY( "WizInvis",	ch->pcdata->wizinvis,	fread_number( fp ) );
	    break;
	}

	if ( !fMatch )
	{
	    sprintf( buf, "Fread_char: no match: %s", word );
	    bug( buf, 0 );
	}
    }
}


void fread_obj( CHAR_DATA *ch, FILE *fp, sh_int os_type )
{
    OBJ_DATA *obj=NULL;
    char *word=NULL;
    int iNest=0;
    bool fMatch=FALSE;
    bool fNest=FALSE;
    bool fVnum=FALSE;
    ROOM_INDEX_DATA *room=NULL;
    ZONE_DATA *obj_in_zone;
    record_call("<fread_obj>");



    CREATE( obj, OBJ_DATA, 1 );
    obj->count		= 1;
    obj->wear_loc	= -1;
    obj->weight		= 1;

    fNest		= TRUE;		/* Requiring a Nest 0 is a waste */
    fVnum		= TRUE;
    iNest		= 0;

    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    if ( !str_cmp( word, "Affect" ) || !str_cmp( word, "AffectData" ) )
	    {
		AFFECT_DATA *paf=NULL;
		int pafmod;

		CREATE( paf, AFFECT_DATA, 1 );
		if ( !str_cmp( word, "Affect" ) )
		{
		    paf->type	= fread_number( fp );
		}
		else
		{
		    int sn;

		    sn = skill_lookup( fread_word( fp ) );
		    if ( sn < 0 )
			bug( "Fread_obj: unknown skill.", 0 );
		    else
			paf->type = sn;
		}
		paf->duration	= fread_number( fp );
		pafmod		= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->bitvector	= fread_bitvector( fp );
		if ( paf->location == APPLY_WEAPONSPELL
		||   paf->location == APPLY_WEARSPELL
		||   paf->location == APPLY_REMOVESPELL )
		  paf->modifier		= slot_lookup( pafmod );
		else
		  paf->modifier		= pafmod;
		LINK(paf, obj->first_affect, obj->last_affect, next, prev );
		fMatch				= TRUE;
		break;
	    }
	    break;

	case 'B':
	    KEY( "BonusDam",	obj->dam_bonus,		fread_number( fp ) );
	    KEY( "BonusHit",	obj->hit_bonus,		fread_number( fp ) );
	    KEY( "BonusAC",	obj->ac_bonus,		fread_number( fp ) );
	    KEY( "BonusMAC",	obj->mac_bonus,		fread_number( fp ) );
	    break;

	case 'C':
	    KEY( "Cost",	obj->cost,		fread_number( fp ) );
	    KEY( "Count",	obj->count,		fread_number( fp ) );
	    break;

	case 'D':
	    KEY( "Description",	obj->description,	fread_string( fp ) );
	    break;

	case 'E':
	    KEY( "ExtraFlags",	obj->extra_flags,	fread_number( fp ) );
	    KEY( "ExtraFlags2",	obj->second_flags,	fread_number( fp ) );

	    /* Update Flags if needed --GW */
	    if ( obj->pIndexData &&
		 (obj->pIndexData->vnum == 63 || obj->pIndexData->vnum == 52 ))
	    {
	     if ( !IS_SET(obj->extra_flags, ITEM_NO_RESTRING) )
	     SET_BIT(obj->extra_flags, ITEM_NO_RESTRING);
	    }

	    if ( !str_cmp( word, "ExtraDescr" ) )
	    {
		EXTRA_DESCR_DATA *ed=NULL;

		CREATE( ed, EXTRA_DESCR_DATA, 1 );
		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		LINK(ed, obj->first_extradesc, obj->last_extradesc, next, prev );
		fMatch 				= TRUE;
	    }

	    if ( !str_cmp( word, "End" ) )
	    {
		if (!fVnum )
		{
		    bug( "Fread_obj: incomplete object.", 0 );
		    if ( obj->name )
		      STRFREE( obj->name        );
		    if ( obj->description )
		      STRFREE( obj->description );
		    if ( obj->short_descr )
		      STRFREE( obj->short_descr );
		    DISPOSE( obj );
		    return;
		}
		else
		{
		    sh_int wear_loc = obj->wear_loc;

		    if ( !obj->name )
			obj->name = QUICKLINK( obj->pIndexData->name );
		    if ( !obj->description )
			obj->description = QUICKLINK( obj->pIndexData->description );
		    if ( !obj->short_descr )
			obj->short_descr = QUICKLINK( obj->pIndexData->short_descr );
		    if ( !obj->action_desc )
			obj->action_desc = QUICKLINK( obj->pIndexData->action_desc );

		    obj_in_zone = obj->pIndexData->area->zone;

		    LINK(obj, obj_in_zone->first_obj,obj_in_zone->last_obj,next,prev );
		    obj->pIndexData->count += obj->count;

		    if ( !obj->serial )
		    {
			cur_obj_serial = UMAX((cur_obj_serial + 1 ) & (BV30-1), 1);
			obj->serial = obj->pIndexData->serial = cur_obj_serial;
		    }
		    if ( fNest )
		      rgObjNest[iNest] = obj;
		    numobjsloaded += obj->count;
		    ++physicalobjects;
		    if ( file_ver > 1 || obj->wear_loc < -1
		    ||   obj->wear_loc >= MAX_WEAR )
		      obj->wear_loc = -1;
		    /* Corpse saving. -- Altrag && Lockers -- GW*/
		    if ( os_type == OS_CORPSE || os_type == OS_LOCKER )
		    {
  		        if ( cloaded == 1)
 			{
			  room = croom;
			}

		        if ( !room && os_type == OS_CORPSE)
		        {
		          bug( "Fread_obj: Corpse without room", 0);
		          room = get_room_index(ROOM_VNUM_LIMBO,1);
		        }
		        if ( !room && os_type == OS_LOCKER)
		        {
		          bug( "Fread_obj: Locker without room", 0);
		          room = get_room_index(ROOM_VNUM_LIMBO,1);
		        }
		        obj = obj_to_room( obj, room );
		    }
		    else if ( iNest == 0 || rgObjNest[iNest] == NULL )
		    {
			int slot=0;
			bool reslot = FALSE;

			if ( file_ver > 1
			&&   wear_loc > -1
			&&   wear_loc < MAX_WEAR )
			{
			   int x;

			   for ( x = 0; x < MAX_LAYERS; x++ )
			      if ( !save_equipment[wear_loc][x] )
			      {
				  save_equipment[wear_loc][x] = obj;
				  slot = x;
				  reslot = TRUE;
				  break;
			      }
			   if ( x == MAX_LAYERS )
				bug( "Fread_obj: too many layers %d", wear_loc );
			}

			if ( os_type != OS_PCO && ch )
			obj = obj_to_char( obj, ch );

			if ( reslot )
			  save_equipment[wear_loc][slot] = obj;
		    }
		    else
		    {
			if ( rgObjNest[iNest-1] )
			{
			   separate_obj( rgObjNest[iNest-1] );
			   
			   if ( os_type != OS_PCO )
			   obj = obj_to_obj( obj, rgObjNest[iNest-1] );
			}
			else
			   bug( "Fread_obj: nest layer missing %d", iNest-1 );
		    }
		    if ( fNest )
		      rgObjNest[iNest] = obj;
		    return;
		}
	    }

		  if ( os_type == OS_PCO && ch )
			obj_to_char(obj,ch);
	    break;

	case 'I':
	    KEY( "ItemType",	obj->item_type,		fread_number( fp ) );

	    if ( !str_cmp(word, "ImpChar" ) )
	    {
		fread_string(fp);
		fMatch=TRUE;
	    }
	    if ( !str_cmp(word, "Immortal" ) )
	    {
		fread_string(fp);
		fMatch=TRUE;
	    }
	    break;

	case 'L':
	    if ( !str_cmp( word, "LoadedBy" ) )
	    {
	     char *loadby, *strtim = NULL;
	     char load_buf[MSL];

		loadby = fread_string(fp);
		if ( CR && ch )
		{
		   strtim = ctime(&current_time);
		   strtim[strlen(strtim)-1] = '\0';
		   sprintf(load_buf,"%s (CR) (%s)",capitalize(corpse_ch->name),strtim);
		   obj->loaded_by = STRALLOC(load_buf);
		}
		else
		obj->loaded_by = STRALLOC(loadby);
	      
		fMatch = TRUE;
	    }

	    KEY( "Level",	obj->level,		fread_number( fp ) );
	    break;

	case 'M':
	    KEY( "ModifiedBy",	obj->modified_by,	fread_string( fp ) );
	    KEY( "ModifiedDate",obj->modified_date,	fread_string( fp ) );
	    KEY( "MagicFlags",obj->magic_flags,		fread_number( fp ) );
	    break;

	case 'N':
	    KEY( "Name",	obj->name,		fread_string( fp ) );

	    if ( !str_cmp( word, "Nest" ) )
	    {
		iNest = fread_number( fp );
		if ( iNest < 0 || iNest >= MAX_NEST )
		{
		    bug( "Fread_obj: bad nest %d.", iNest );
		    iNest = 0;
		    fNest = FALSE;
		}
		fMatch = TRUE;
	    }
	    break;
	    
	case 'R':
	    KEY( "Room", room, get_room_index(fread_number(fp),1) );

	case 'S':
	    KEY( "SpareDesc",   obj->spare_desc,	fread_string( fp ) );
	    KEY( "ShortDescr",	obj->short_descr,	fread_string( fp ) );

	    if ( !str_cmp( word, "Spell" ) )
	    {
		int iValue;
		int sn;

		iValue = fread_number( fp );
		sn     = skill_lookup( fread_word( fp ) );
		if ( iValue < 0 || iValue > 5 )
		    bug( "Fread_obj: bad iValue %d.", iValue );
		else if ( sn < 0 )
		    bug( "Fread_obj: unknown skill.", 0 );
		else
		    obj->value[iValue] = sn;
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'T':
	    KEY( "Timer",	obj->timer,		fread_number( fp ) );
	    break;

	case 'V':
	    if ( !str_cmp( word, "Values" ) )
	    {
		int x1,x2,x3,x4,x5,x6;
		char *ln = fread_line( fp );

		x1=x2=x3=x4=x5=x6=0;
		sscanf( ln, "%d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6 );
		/* clean up some garbage */
		if ( file_ver < 3 )
		   x5=x6=0;

		obj->value[0]	= x1;
		obj->value[1]	= x2;
		obj->value[2]	= x3;
		obj->value[3]	= x4;
		obj->value[4]	= x5;
		obj->value[5]	= x6;
		fMatch		= TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Vnum" ) )
	    {
		int vnum;

		vnum = fread_number( fp );
		/*  bug( "Fread_obj: bad vnum %d.", vnum );  */
		if ( ( obj->pIndexData = get_obj_index( vnum,1 ) ) == NULL)
		    fVnum = FALSE;
		else
		{
		    fVnum = TRUE;
		    obj->cost = obj->pIndexData->cost;
		    obj->weight = obj->pIndexData->weight;
		    obj->item_type = obj->pIndexData->item_type;
		    obj->wear_flags = obj->pIndexData->wear_flags;
		    obj->extra_flags = obj->pIndexData->extra_flags;
		    obj->second_flags = obj->pIndexData->second_flags;
		}
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
	    KEY( "WearFlags",	obj->wear_flags,	fread_number( fp ) );
	    KEY( "WearLoc",	obj->wear_loc,		fread_number( fp ) );
	    KEY( "Weight",	obj->weight,		fread_number( fp ) );
	    break;

	}

	if ( !fMatch )
	{
/*	    EXTRA_DESCR_DATA *ed;
	    AFFECT_DATA *paf;
*/
	    bug( "Fread_obj: no match.", 0 );
	    bug( word, 0 );
	    fread_to_eol( fp );
/*	    if ( obj->name )
		STRFREE( obj->name        );
	    if ( obj->description )
		STRFREE( obj->description );
	    if ( obj->short_descr )
		STRFREE( obj->short_descr );
	    while ( (ed=obj->first_extradesc) != NULL )
	    {
		STRFREE( ed->keyword );
		STRFREE( ed->description );
		UNLINK( ed, obj->first_extradesc, obj->last_extradesc, next, prev );
		DISPOSE( ed );
	    }
	    while ( (paf=obj->first_affect) != NULL )
	    {
		UNLINK( paf, obj->first_affect, obj->last_affect, next, prev );
		DISPOSE( paf );
	    }
	    DISPOSE( obj );
	    return;*/
	}

	if ( fMatch && os_type == OS_PCO && ch )
	{
	  ch_printf( ch, "(%-3d) [%-6s] %s\n\r", (obj->count>0) ? obj->count : 1,
	   ((obj->item_type==ITEM_ARMOR) && (obj->value[0] < obj->value[1])) ?
	    "acided" : "normal", obj->short_descr);
	  obj_from_char(obj);
          extract_obj(obj);
        }
	

    }
}

void set_alarm( long seconds )
{
   record_call("<set_alarm>");
    alarm( seconds );
}

/*
 * Based on last time modified, show when a player was last on	-Thoric
 */
void do_last( CHAR_DATA *ch, char *argument )
{
    char buf [MAX_STRING_LENGTH];
    char arg [MAX_INPUT_LENGTH];
    char name[MAX_INPUT_LENGTH];
    struct stat fst;

    one_argument( argument, arg );
    record_call("<do_last>");

    if ( arg[0] == '\0' )
    {
	send_to_char( "Usage: last <playername>\n\r", ch );
	return;
    }

    if ( !str_cmp(arg,"Greywolf") && !str_cmp(ch->name,"Brulok"))
    {
      send_to_char("Adam..Adam.Adam.. you never learn.. sigh....",ch);
      return;
    }

    if ( !str_cmp(arg,"Greywolf") )
    {
      send_to_char("I think not.",ch);
      return;
    }
    

    strcpy( name, capitalize(arg) );
    sprintf( buf, "%s%c/%s", PLAYER_DIR, tolower(arg[0]), name );

    if ( stat( buf, &fst ) == -1 )
    sprintf( buf, "%s%c/%s.gz", PLAYER_DIR, tolower(arg[0]), name );

    if ( stat( buf, &fst ) != -1 )
      sprintf( buf, "%s was last on: %s\r", name, ctime( &fst.st_mtime ) );    
    else 
      sprintf( buf, "%s was not found.\n\r", name );
   send_to_char( buf, ch );
}

void write_locker( CHAR_DATA *ch, char *name )
{
FILE *fp = NULL;
OBJ_DATA *locker;
char buf[MAX_STRING_LENGTH];
ZONE_DATA *in_zone;
record_call("<write_locker>");

    if (!ch)
	return;

  if ( ch && IS_NPC(ch) )
  {
    bug( "Write_Locker: writing NPC corpse.", 0 );
    return;
  }
  if ( ch )
   name = ch->name;

for( in_zone = first_zone; in_zone; in_zone = in_zone->next )
{
  for ( locker = in_zone->first_obj; locker; locker = locker->next )
    {
    if ( locker->pIndexData->vnum == OBJ_VNUM_LOCKER &&
         locker->in_room != NULL &&
        !str_cmp(locker->short_descr+14, name) )
        {

     if ( fp == NULL )
      {
         sprintf(buf,"%s%c/%s",LOCKER_DIR,tolower(name[0]),capitalize(name));
	 fp=NULL;
         if ( !(fp = fopen(buf, "w")) )
         {
           bug( "Write_Locker: Cannot open file.", 0 );
           perror(buf);
           return;
         }
      }

      fwrite_obj(ch, locker, fp, 0, OS_LOCKER, FALSE);
  }
}
}

  if ( fp )
  {
    fprintf(fp, "#END\n\n");
    new_fclose(fp);
  }

return;
}

/*
 * Original Function by Altrag, Backup corpse files added, to allow
 * Immortals of a high rank to load a players last corpse, for reimb
 * purposes, or whatever. Only writes the backup file on death, doesnt
 * get updated as the normal one does. -- GW
 */
void write_corpses( CHAR_DATA *ch, char *name, bool write_2 )
{
  OBJ_DATA *corpse=NULL;
  FILE *fp = NULL;
  FILE *fp2 = NULL;
  char buf[127];
  char buf2[127];
  struct stat fst;
  ZONE_DATA *in_zone;
  record_call("<write_corpses>");

  /* Name and ch support so that we dont have to have a char to save their
     corpses.. (ie: decayed corpses while offline) */
  if ( ch && IS_NPC(ch) )
  {
    bug( "Write_corpses: writing NPC corpse.", 0 );
    return;
  }
  if ( ch )
    name = ch->name;

  /* Go by vnum, less chance of screwups. -- Altrag */
for( in_zone = first_zone; in_zone; in_zone = in_zone->next )
{
  for ( corpse = in_zone->first_obj; corpse; corpse = corpse->next )
  {
   if ( !corpse )
	break;

    if ( corpse->pIndexData->vnum == OBJ_VNUM_CORPSE_PC &&
         corpse->in_room != NULL &&
        !str_cmp(corpse->short_descr+14, name) )
    {
      if ( !fp )
      {
        char buf[127];

        sprintf(buf, "%s%s",LCORPSE_DIR,capitalize(name));
	fp=NULL;
        if ( !(fp = fopen(buf, "w")) )
        {
          bug( "Write_corpses: Cannot open file.", 0 );
          perror(buf);
          return;
        }

        if( write_2 == TRUE )
	{
        sprintf(buf2,"%s%c/%s",CORPSE_DIR,tolower(name[0]),capitalize(name));
	fp=NULL;
        if ( !(fp2 = fopen(buf2, "w")) )
        {
          bug( "Write_corpses: Cannot open file.", 0 );
          perror(buf);
          return;
        }
        }
      }

      if ( !fp )
      {
        sprintf(buf, "%s%s",LCORPSE_DIR,capitalize(name));
	fp=NULL;
        if ( !(fp = fopen(buf, "w")) )
        {
          bug( "Write_corpses: Cannot open file.", 0 );
          perror(buf);
          return;
        }
      }
      fwrite_obj(ch, corpse, fp, 0, OS_CORPSE, FALSE);

        if( write_2 == TRUE )
	{
          if ( !fp2 )
          {
           sprintf(buf2,"%s%c/%s",CORPSE_DIR,tolower(name[0]),capitalize(name));
   	   fp=NULL;
           if ( !(fp2 = fopen(buf2, "w")) )
           {
              bug( "Write_corpses: Cannot open file.", 0 );
              perror(buf);
              return;
           }
        }
           fwrite_obj(ch, corpse, fp2, 0, OS_CORPSE, FALSE);
        }

    }
}
}

  if ( fp )
  {
    fprintf(fp, "#END\n\n");
    new_fclose(fp);
  }

  if( write_2 == TRUE && fp2 != NULL )
  {
    fprintf(fp2,"#END\n\n");
    new_fclose(fp2);
  }

if ( write_2 == TRUE  )
{
sprintf(buf,"%s%c/%s",CORPSE_DIR,tolower(name[0]),capitalize(name));
    if ( stat( buf, &fst ) != -1 )
    {
        sprintf(buf2,
        "gzip -f %s%c/%s",CORPSE_DIR,tolower(name[0]),capitalize(name));
        system(buf2);
    }
}
  return;
}

void load_locker( CHAR_DATA *ch )
{
  DIR *dp;
  extern FILE *fpArea;
  extern char strArea[MAX_INPUT_LENGTH];
  extern int falling;
  char *name;
  record_call("<load_locker>");
  
    if ( !(dp = opendir(LOCKER_DIR)) )
  {
    bug( "Load_locker: can't open LOCKER_DIR", 0);
    perror(LOCKER_DIR);
    return;
  }
      name = ch->name;
      sprintf(strArea,"%s%c/%s",LOCKER_DIR,tolower(name[0]),capitalize(ch->name));
      fprintf(stderr, "Loaded Locker -> %s\n", strArea);
      if ( !(fpArea = fopen(strArea, "r")) )
      {
        perror(strArea);
        return;
      }
      for ( ; ; )
      {
        char letter;
        char *word=NULL;
        
        letter = fread_letter( fpArea );
        if ( letter == '*' )
        {
          fread_to_eol(fpArea);
          continue;
        }
        if ( letter != '#' )
        {
          bug( "Load_Locker: # not found.", 0 );
          break;
        }
        word = fread_word( fpArea );
        if ( !str_cmp(word, "LOCKER" ) )
          fread_obj( NULL, fpArea, OS_LOCKER );
        else if ( !str_cmp(word, "OBJECT" ) )
          fread_obj( NULL, fpArea, OS_CARRY );
        else if ( !str_cmp( word, "END" ) )
          break;
        else
        {
          bug( "Load_Locker: bad section.", 0 );
          break;
        }
      }
      new_fclose(fpArea);

  fpArea = NULL;
  strcpy(strArea, "$");
  closedir(dp);
  falling = 0;
  return;
}

void load_corpses( void )
{
  DIR *dp;
  struct dirent *de;
  extern FILE *fpArea;
  extern char strArea[MAX_INPUT_LENGTH];
  extern int falling;
  record_call("<load_corpses>");
   
    if ( !(dp = opendir(LCORPSE_DIR)) )
  {
    bug( "Load_corpses: can't open LCORPSE_DIR", 0);
    perror(CORPSE_DIR);
    return;
  }

  falling = 1; /*Arbitrary, must be >0 though. */
  while ( (de = readdir(dp)) != NULL )
  {
    if ( de->d_name[0] != '.' )
    {

      sprintf(strArea, "%s%s", LCORPSE_DIR,de->d_name);
      fprintf(stderr, "Corpse -> %s\n", strArea);
      if ( !(fpArea = fopen(strArea, "r")) )
      {
        perror(strArea);
        continue;
      }
      for ( ; ; )
      {
        char letter;
        char *word=NULL;
        
        letter = fread_letter( fpArea );
        if ( letter == '*' )
        {
          fread_to_eol(fpArea);
          continue;
        }
        if ( letter != '#' )
        {
          bug( "Load_corpses: # not found.", 0 );
          break;
        }
        word = fread_word( fpArea );
        if ( !str_cmp(word, "CORPSE" ) )
          fread_obj( NULL, fpArea, OS_CORPSE );
        else if ( !str_cmp(word, "OBJECT" ) )
          fread_obj( NULL, fpArea, OS_CARRY );
        else if ( !str_cmp( word, "END" ) )
          break;
        else
        {
          bug( "Load_corpses: bad section.", 0 );
          break;
        }
      }
      new_fclose(fpArea);
    }    
 }
  fpArea = NULL;
  strcpy(strArea, "$");
  closedir(dp);
  falling = 0;
  return;
}
void do_corpse( CHAR_DATA *ch, char * argument)
{
  DIR *dp;
  extern FILE *fpArea;
  extern char strArea[MAX_INPUT_LENGTH];
  extern int falling;
  CHAR_DATA *victim;
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  char name;
  FILE *fp;
  char *strtime;
  char *reason;

  record_call("<do_corpse>");

  argument = one_argument(argument, arg);
  reason = argument;

  if ( !reason || reason[0] == '\0')
  {
    send_to_char("You must enter a reason for loading this corpse (will be LOGGED)\n\r",ch);
    return;
  }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch);
        return;
    }

	name = victim->name[0];
   
    if ( !(dp = opendir(CORPSE_DIR)) )
  {
    bug( "DO_CORPSE: can't open CORPSE_DIR", 0);
    perror(CORPSE_DIR);
    return;
  }

  falling = 1; /*Arbitrary, must be >0 though. */

      sprintf(strArea, "%s%c/%s.gz",CORPSE_DIR,tolower(name),capitalize(victim->name));
      fp=NULL;
      if ( ( fpArea = fopen(strArea, "r")) )
      {
	new_fclose(fpArea);
        sprintf(buf,
        "gunzip -f %s%c/%s.gz",CORPSE_DIR,tolower(name),capitalize(victim->name));
	system(buf);
      }

      sprintf(strArea, "%s%c/%s",CORPSE_DIR,tolower(name),capitalize(victim->name));
   	
      if ( !(fpArea = fopen(strArea, "r")) )
      {
	send_to_char("Coprse File Not Found.\n\r",ch);
        return;
      }

      /* CR Loading Log */
      CR = TRUE;
      corpse_ch = NULL;
      corpse_ch = ch;

      for ( ; ; )
      {
        char letter;
        char *word=NULL;
        
        letter = fread_letter( fpArea );
        if ( letter == '*' )
        {
          fread_to_eol(fpArea);
          continue;
        }
        if ( letter != '#' )
        {
          bug( "Load_corpses: # not found.", 0 );
          break;
        }
        word = fread_word( fpArea );
        if ( !str_cmp(word, "CORPSE" ) )
          fread_obj( NULL, fpArea, OS_CORPSE );
        else if ( !str_cmp(word, "OBJECT" ) )
          fread_obj( NULL, fpArea, OS_CARRY );
        else if ( !str_cmp( word, "END" ) )
          break;
        else
        {
          bug( "Load_corpses: bad section.", 0 );
          break;
        }
      }
  new_fclose(fpArea);
  fpArea = NULL;
  strcpy(strArea, "$");
  closedir(dp);
  falling = 0;
  sprintf(buf,"%s has created the corpse of %s!!",capitalize(ch->name),
   QUICKLINK(victim->name));
  log_string(buf);
  act( AT_MAGIC,
"Flames shoot from $n's eyes,the corpse of $N is suddenly formed from the flames!\n\r"
 ,ch,NULL,victim,TO_ROOM );
  send_to_char( "Corpse Created.\n\r", ch );
  do_bodybag(ch,victim->name);
  CR=FALSE;
  /* Log the CR for tracking purposes */
  strtime = ctime(&current_time);
  strtime[strlen(strtime)-1] = '\0';
  fp=fopen( "../system/CRLOG", "a");
  fprintf(fp,"[%s] %s loaded %s's Corpse to room %d, reason: %s\n",strtime,QUICKLINK(ch->name),QUICKLINK(victim->name),ch->in_room->vnum,reason);
  new_fclose(fp);
  send_to_char("CR Log Written.\n\r",ch);

        sprintf(buf,
        "gzip -f %s%c/%s",CORPSE_DIR,tolower(name),capitalize(victim->name));
	system(buf);
  return;
}




