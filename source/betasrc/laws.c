/*--------------------------------------------------------------------------*
 *                         ** WolfPaw 1.0 **                                *
 *--------------------------------------------------------------------------*
 *               WolfPaw 1.0 (c) 1997,1998 by Dale Corse                    *
 *--------------------------------------------------------------------------*
 *            The WolfPaw Coding Team is headed by: Greywolf                *
 *  With the Assitance from: Callinon, Dhamon, Sentra, Wyverns, Altrag      *
 *  Scryn, Thoric, Justice, Tricops and Mask.                               *
 *--------------------------------------------------------------------------*
 *			Auto Enforced Laws Module			    *
 *--------------------------------------------------------------------------*/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include "mud.h"

char offense[1024];

#define KEY( literal, field, value )                                    \
                                if ( !str_cmp( word, literal ) )        \
                                {                                       \
                                    field  = value;                     \
                                    fMatch = TRUE;                      \
                                    break;                              \
                                }

/* Enforcing */

/*
 * Check to see if they need to be arrested
 */
bool arrest ( CHAR_DATA *ch )
{

if ( IS_NPC(ch) )
return FALSE;

/* NoEnforce? */
if ( noenforce == TRUE )
return FALSE;

/* Chaos on? */
if ( chaos == 1 )
return FALSE;

/* War Mode? */
if ( IS_SET( ch->pcdata->flagstwo, MOREPC_WAR ) )
return FALSE;

if ( IS_NPC(ch) || IS_IMMORTAL(ch) )
return FALSE;

if ( ch->in_room && ch->in_room->area &&
	IS_SET( ch->in_room->area->flags, AFLAG_TOWN ) )
{
if ( ch->fighting )
{

if ( !IS_NPC(ch->fighting->who) )
{
strcpy(offense,"Attempted Murder");
create_warrent( NULL, ch, offense );
return TRUE;
}


/* no good for newbies! --GW 
if ( IS_NPC(ch->fighting->who) && ch->fighting->who->pIndexData->vnum ==
MOB_VNUM_CITYGUARD )
{
strcpy(offense,"Attacking a Peacer Officer");
create_warrent( NULL, ch, offense );
return TRUE;
}*/
}


if ( IS_SET( ch->act, PLR_KILLER ) )
{
strcpy(offense,"Murder");
create_warrent( NULL, ch, offense );
return TRUE;
}

if ( IS_SET( ch->act, PLR_THIEF ) )
{
strcpy(offense,"Theft");
create_warrent( NULL, ch, offense );
return TRUE;
}

if ( IS_SET( ch->pcdata->flagstwo, MOREPC_TOWNOUTCAST ) )
{
strcpy(offense,"Outcast Violation");
create_warrent( NULL, ch, offense );
return TRUE;
}
}


return FALSE;
}

void check_looter( CHAR_DATA *ch, OBJ_DATA *corpse )
{

return;
}

void check_lockerlooter( CHAR_DATA *ch, OBJ_DATA *locker )
{

return;
}

void arrest_char ( CHAR_DATA *victim )
{
AFFECT_DATA af;
CHAR_DATA *ch;

if ( IS_IMMORTAL(victim) )
return;

ch = victim;

stop_fighting( victim, TRUE ); 

act( AT_MAGIC,"A group of Cityguards appear in a bright flash!",ch,NULL,NULL,TO_ROOM);
act( AT_MAGIC,"A group of Cityguards appear in a bright flash!",ch,NULL,NULL,TO_CHAR);
act( AT_BLOOD,"$n is knocked to the ground, stunned, as a guard hits $m in the back of the head!",ch, NULL,NULL,TO_ROOM);
act( AT_BLOOD,"You are knocked to the ground, stunned, as a guard hits you in the back of the head!",ch, NULL,NULL,TO_CHAR);
act( AT_MAGIC,"The Guards arrest $n, and carry $m to Jail.",ch,NULL,NULL,TO_ROOM);
act( AT_MAGIC,"The Guards arrest you, and carry you to Jail.",ch,NULL,NULL,TO_CHAR);

          af.type      = gsn_stun;
          af.location  = APPLY_AC;
          af.modifier  = 20;
          af.duration  = 30;
          af.bitvector = meb(AFF_PARALYSIS);
          affect_to_char( victim, &af );
          update_pos( victim );

char_from_room(ch);
char_to_room( ch, get_room_index( ROOM_VNUM_CELL,1));
SET_BIT(ch->pcdata->flagstwo, MOREPC_ARRESTED);
do_save(ch,"");
return;
}

bool check_warrents( CHAR_DATA *ch )
{
WARRENT_DATA *temp;
char buf[MSL];

for ( temp = first_warrent; temp; temp = temp->next )
{
if ( !str_cmp( temp->name, ch->name ) && temp->executed == 0 )
{
sprintf(buf,"Arresting %s for: %s (Issuer: %s)",capitalize(ch->name),
						temp->offense,
						temp->issuer);
log_string(buf);
arrest_char(ch);
temp->executed = 1;
write_warrents();
return TRUE;
}
}
return FALSE;
}


/* Loading and Saving */
void load_warrents( void )
{
WARRENT_DATA *warrent=NULL;
FILE *fp;
bool fHeader = FALSE;

if( (fp=fopen( WARRENT_FILE,"r" ) )==NULL )
{
bug("load_mail: CANNOT OPEN WARRENT_FILE",0);
perror(WARRENT_FILE);
return;
}


for ( ; ; )
{
char letter;
char *word;

letter = fread_letter( fp );
if( letter == '*' )
{
fread_to_eol( fp );
continue;
}

if ( letter != '#' )
{
bug("Load_Warrent: # not Found.",0);
break;
}

word = fread_word( fp );
if ( !str_cmp(word, "WARRENTS" ) )
{
fHeader = TRUE;
fread_to_eol( fp );
continue;
}
else if ( !str_cmp(word, "WARRENT" ) )
{
CREATE( warrent, WARRENT_DATA,1);
read_warrent( warrent, fp );
LINK(warrent, first_warrent, last_warrent, next, prev );
continue;
}
else if ( !str_cmp(word, "END" ) )
break;
else
{
bug("Load_Warrents: Bad Section.",0);
break;
}

if ( !fHeader )
bug ("Load_warrents: #WARRENTS NOT FOUND!",0);

}

new_fclose(fp);
return;
}

void read_warrent( WARRENT_DATA *warrent, FILE *fp )
{
    char *word=NULL;
    bool fMatch=FALSE;

    for( ; ; )
    {
        word = feof( fp ) ? "$$" : fread_word( fp );
        fMatch = FALSE;

        switch ( UPPER(word[0]) )
        {
        case '*':
                fMatch = TRUE;
                fread_to_eol( fp );
                break;

        case '$':
                fMatch = TRUE;
                return;

	case 'D':
	KEY( "Date", 	warrent->date,		fread_string( fp ) );
	break;

	case 'E':
	KEY( "Executed",warrent->executed,	fread_number( fp ) );
	break;	

	case 'I':
	KEY( "Issuer",	warrent->issuer,	fread_string( fp ) );
	break;

	case 'N':
	KEY( "Name",	warrent->name,		fread_string( fp ) );
	break;

	case 'O':
	KEY( "Offense",	warrent->offense,	fread_string( fp ) );
	break;

        }

        if ( !fMatch  )
        {
          bug( "Read_warrents: no match for %s.", word );
        }
   }
}

void write_warrents( void )
{
WARRENT_DATA *warrent;
FILE *fp;


if ( (fp=fopen( WARRENT_FILE,"w" ) )==NULL)
{
bug("Cannot open WARRENT_FILE!",0);
perror(WARRENT_FILE);
return;
}

fprintf( fp,"#WARRENTS DATA\n\n");
for ( warrent = first_warrent; warrent; warrent = warrent->next )
{
fprintf( fp, "Name	%s~\n",	warrent->name );
fprintf( fp, "Issuer	%s~\n",	warrent->issuer );
fprintf( fp, "Offense	%s~\n",	warrent->offense );
fprintf( fp, "Date	%s~\n",	warrent->date );
fprintf( fp, "Executed	%d\n",	warrent->executed );
fprintf( fp, "$$\n\n");
}

fprintf( fp,"#END DATA" );
new_fclose(fp);
return;
}

/* Create and delete Warrents */
void create_warrent( CHAR_DATA *ch, CHAR_DATA *victim, char *offense )
{
char *strtime;
WARRENT_DATA *warrent=NULL;
            strtime     = ctime( &current_time );
            strtime[strlen(strtime)-1]  = '\0';
            CREATE( warrent, WARRENT_DATA, 1 );
	    if ( !ch )
	    warrent->issuer	= STRALLOC("System Issued");
	    else
	    warrent->issuer	= STRALLOC(ch->name);
	    warrent->name	= STRALLOC(victim->name);
	    warrent->executed   = 0;
	    warrent->offense    = STRALLOC(offense);
	    warrent->date	= STRALLOC(strtime);
	    LINK(warrent, first_warrent, last_warrent, next, prev );
	    write_warrents();

return;
}

void destroy_warrent( WARRENT_DATA *warrent )
{
WARRENT_DATA *temp;

for ( temp = first_warrent; temp; temp = temp->next )
{
if ( warrent == temp )
{
UNLINK(temp,first_warrent,last_warrent,next, prev );
write_warrents();
break;
}
}

return;
}

/* User Commands */

void do_arrest( CHAR_DATA *ch, char *argument )
{
char arg[MIL];
char arg2[MIL];
CHAR_DATA *victim;

argument = one_argument( argument, arg );
argument = one_argument( argument, arg2 );


if ( arg[0] == '\0' || arg2[0] == '\0' )
{
send_to_char("Syntax: arrest <release/hold> <char>\n\r",ch);
return;
}

if ( ( victim=get_char_world( ch, arg2 ) )==NULL)
{
send_to_char("I dont see that person here...\n\r",ch);
return;
}

if ( !str_cmp( arg, "hold" ) )
{
arrest_char(victim);
send_to_char("Done.\n\r",ch);
return;
}

if ( !str_cmp( arg, "release" ) )
{
REMOVE_BIT( victim->pcdata->flagstwo, MOREPC_ARRESTED );
send_to_char("You have Been Released by an Immortal.\n\r",victim);
char_from_room(victim);
char_to_room(victim,get_room_index(30601,1));
do_look(victim,"auto");
do_save(victim,"");
send_to_char("Done.\n\r",ch);
return;
}

send_to_char("Syntax: arrest <release/hold> <char>\n\r",ch);
return;
}

void do_judge( CHAR_DATA *ch, char *argument )
{

return;
}

void do_warrent( CHAR_DATA *ch, char *argument )
{

return;
}

void do_releaseme( CHAR_DATA *ch, char *argument )
{

if ( !IS_SET( ch->pcdata->flagstwo, MOREPC_ARRESTED ) )
{
send_to_char("But your not in Jail?\n\r",ch);
return;
}

act( AT_MAGIC,"$n is escorted out by the Guards.",ch,NULL,NULL,TO_ROOM);
act( AT_MAGIC,"You escorted out or town by the Guards.",ch,NULL,NULL,TO_ROOM);
char_from_room(ch);
char_to_room(ch,get_room_index(ROOM_VNUM_CURSED,1));
do_look(ch,"auto");
REMOVE_BIT( ch->pcdata->flagstwo, MOREPC_ARRESTED );
SET_BIT(ch->pcdata->flagstwo, MOREPC_TOWNOUTCAST);
send_to_char("The Guards warn you not to return to town.\n\r",ch);
do_save(ch,"");
return;
}

