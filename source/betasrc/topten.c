/*--------------------------------------------------------------------------*
 *                         ** WolfPaw 1.0 **                                *
 *--------------------------------------------------------------------------*
 *               WolfPaw 1.0 (c) 1997,1998 by Dale Corse                    *
 *--------------------------------------------------------------------------*
 *            The WolfPaw Coding Team is headed by: Greywolf                *
 *  With the Assitance from: Callinon, Dhamon, Sentra, Wyverns, Altrag      *
 *  Scryn, Thoric, Justice, Tricops and Mask.                               *
 *--------------------------------------------------------------------------*
 *			TopTen Control Module 			    	    *
 *--------------------------------------------------------------------------*/
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "mud.h"

#define TOPTEN_DAMAGE		1
#define TOPTEN_EXP		2
#define TOPTEN_PKS		3
#define TOPTEN_HITROLL		4
#define TOPTEN_AC		5
#define TOPTEN_MAC		6
#define TOPTEN_TAC		7
#define TOPTEN_MANA		8
#define TOPTEN_HP		9
#define TOPTEN_IMPCHAR		10

#define TOPTEN_FILE	"../system/topten.dat"

void show_topten( CHAR_DATA *ch, sh_int type );
char *get_topten_type_name( sh_int type );
TOPTEN_DATA *on_topten_list( TOPTEN_DATA *top, char *name, bool AC, long value );
TOPTEN_DATA * list_resort( TOPTEN_DATA *top, bool AC );
void topten_exist_check( TOPTEN_DATA *top );

void topten_setup( void )
{
TOPTEN_DATA *top_entry=NULL,*tmp=NULL,*next_tmp=NULL;
FILE *fp;

first_topten = NULL;
last_topten = NULL;
lowest_toptens[0] = 0;

if ( ( fp = fopen( TOPTEN_FILE, "r" ) )==NULL)
{
  bug("TopTen Setup: Cannot open Data file!",0);
  TOPTEN_ACTIVE = FALSE;
  return;
}
else
  TOPTEN_ACTIVE = TRUE;

for( ; ; )
{
  char *word;
  char letter;
  char buf[MSL];
  int cnt=0;

  letter = fread_letter(fp);

  if ( letter != '#' )
  {
    bug("TopTen Setup: # not found!",0);
    TOPTEN_ACTIVE = FALSE;
    break;
  }

  word = fread_word(fp);

  if ( !str_cmp( word, "ENTRY" ) )
  {
    top_entry=NULL;
    CREATE(top_entry, TOPTEN_DATA, 1 );
    top_entry->type = fread_number(fp);

    sprintf(buf,"Loading Entry: T:%d",top_entry->type);
    log_string(buf);

    for ( cnt = 0; cnt <= 9; cnt++ )
      top_entry->plist[cnt] = fread_string(fp);

    for ( cnt = 0; cnt <= 9; cnt++ )  
      top_entry->values[cnt] = fread_number(fp);

    lowest_toptens[top_entry->type] = top_entry->values[9];

    LINK(top_entry, first_topten, last_topten, next, prev );
 }
 else if ( !str_cmp( word, "END" ) )
          break;
 else
 {
    bug("TopTen Setup: bad word! (%s)", word );
    TOPTEN_ACTIVE = FALSE;
    break;
 }
}
new_fclose(fp);
log_string("TopTen Setup Conplete!"); 
log_string("TOPTEN: Checking to make sure all entries still exist..");
for ( tmp=first_topten; tmp; tmp=next_tmp )
{
  next_tmp= tmp->next;

  topten_exist_check(tmp);
}
log_string("Done.");
return;     
}

void topten_update( char *name, sh_int type, long value )
{
TOPTEN_DATA *top, *next_top;
int cnt=0;
bool AC;
TOPTEN_DATA *temp;
bool REMOVE_NAME=FALSE;
char buf[MSL];

if ( type == TOPTEN_AC || type == TOPTEN_MAC || type == TOPTEN_TAC )
AC = TRUE;
else
AC = FALSE;

if ( type == TOPTEN_IMPCHAR )
REMOVE_NAME=TRUE;

if ( REMOVE_NAME )
{
  for( top= first_topten; top; top=next_top )
  {
   next_top=top->next;

    if ( top->type != TOPTEN_PKS )
    {
       for ( cnt=0; cnt <= 9; cnt++ )
       {
	 if( !str_cmp(top->plist[cnt],name) )
	 {
	   sprintf(buf,"TOPTEN: Imp_char Found, Removing (%s)",name);
	   log_string_plus(buf,LOG_NORMAL,59);
      	   top->plist[cnt] = STRALLOC("None");
           top->values[cnt] = 0;
	   fwrite_topten();
	 }
       }
    }
  }
return;
}

for( top = first_topten; top; top=next_top )
{
  next_top = top->next;

  if( top->type == type )
  {
    if ( ( temp=on_topten_list(top,name,AC,value) )!=NULL)
    {
       /* Update Low values */
       UNLINK(top,first_topten,last_topten,next,prev);
       DISPOSE(top);
       LINK(temp,first_topten,last_topten,next,prev);
       break;
    }

    /* Only Insert if it is higher then the lowest in the list --GW */
    if ( ( !AC && value < lowest_toptens[type] ) ||
         ( AC && value > lowest_toptens[type] ) )
    return;

    for( cnt=0; cnt <= 9; cnt++ )
    {
      if ( ( !AC && value > top->values[cnt] ) ||
           ( AC && value < top->values[cnt] ) )
      {
	if ( cnt == 9 )
	{
	   top->plist[9] = name;
           top->values[9] = value;
	   break;
        }
        else
        {
        /* Move everyone else up, and insert myself */
   	    top->plist[9] = top->plist[8]; /* put me in 9th */
	    top->values[9]= top->values[8];
  	if ( cnt < 8 ) /* put me in 8th */
        {
	    top->plist[8] = top->plist[7]; 
	    top->values[8]= top->values[7];
	}
  	if ( cnt < 7 ) /* put me in 7th */
        {
	    top->plist[7] = top->plist[6]; 
	    top->values[7]= top->values[6];
	}
  	if ( cnt < 6 ) /* put me in 6th */
        {
	    top->plist[6] = top->plist[5]; 
	    top->values[6]= top->values[5];
	}
  	if ( cnt < 5 ) /* put me in 5th */
        {
	    top->plist[5] = top->plist[4]; 
	    top->values[5]= top->values[4];
	}
  	if ( cnt < 4 ) /* put me in 4th */
        {
	    top->plist[4] = top->plist[3]; 
	    top->values[4]= top->values[3];
	}
  	if ( cnt < 3 ) /* put me in 3rd */
        {
	    top->plist[3] = top->plist[2]; 
	    top->values[3]= top->values[2];
	}
  	if ( cnt < 2 ) /* Put me in 2nd */
        {
	    top->plist[2] = top->plist[1]; 
	    top->values[2]= top->values[1];
	}
  	if ( cnt < 1 ) /* Put me in 1st */
        {
	    top->plist[1] = top->plist[0]; 
	    top->values[1]= top->values[0];
	}

	top->plist[cnt] = name;
        top->values[cnt] = value;
	if ( top->values[cnt] == 0 )
	top->plist[cnt] = STRALLOC("None");
        break;
   } /* Else */


}/* if ( value is greater )*/
}/* for ( value scan )*/

lowest_toptens[type] = top->values[9];

}/* if  ( type is right )*/
}/* for (tpye check)*/

fwrite_topten();
return;	    
}

void do_topten( CHAR_DATA *ch, char *argument )
{
char arg1[MSL];
TOPTEN_DATA *top, *next_top;
int cnt=0;
char buf[MSL];

if ( !TOPTEN_ACTIVE )
{
send_to_char("TopTen is not Active.\n\r",ch);
return;
}

if ( IS_NPC(ch) )
return;

one_argument( argument, arg1 );

if ( arg1[0] == '\0' )
{
send_to_char("Syntax: topten <type>\n\r",ch);
send_to_char("\n\rTypes:\n\r",ch);
send_to_char("damroll exp pks hitroll eqac mac ac mana hp\n\r",ch);
return;
}

if ( !str_cmp( arg1, "damroll" ) )
{
  show_topten(ch, TOPTEN_DAMAGE);
  return;
}
if ( !str_cmp( arg1, "exp" ) )
{
  show_topten(ch, TOPTEN_EXP);
  return;
}
if ( !str_cmp( arg1, "pks" ) )
{
  show_topten(ch, TOPTEN_PKS);
  return;
}
if ( !str_cmp( arg1, "hitroll" ) )
{
  show_topten(ch, TOPTEN_HITROLL);
  return;
}
if ( !str_cmp( arg1, "eqac" ) )
{
  show_topten(ch, TOPTEN_AC);
  return;
}
if ( !str_cmp( arg1, "mac" ) )
{
  show_topten(ch, TOPTEN_MAC);
  return;
}
if ( !str_cmp( arg1, "ac" ) )
{
  show_topten(ch, TOPTEN_TAC);
  return;
}
if ( !str_cmp( arg1, "mana" ) )
{
  show_topten(ch, TOPTEN_MANA);
  return;
}
if ( !str_cmp( arg1, "hp" ) )
{
  show_topten(ch, TOPTEN_HP);
  return;
}

/* Secret Value for Hyps */
if ( IS_IMP(ch) )
{
  if ( !str_cmp( arg1,"clear" ) )
  {
    sprintf(buf,"%s cleared the TopTen Data",capitalize(ch->name));
    log_string_plus(buf,LOG_NORMAL,LEVEL_HYPERION);
    for( top = first_topten; top; top = next_top )
    {
      next_top = top->next;
      for( cnt = 0; cnt <= 9; cnt++ )
      {
        top->plist[cnt] = STRALLOC("None");
        top->values[cnt] = 0;
      }
    }
  }
fwrite_topten();
}

send_to_char("Syntax: topten <type>\n\r",ch);
send_to_char("\n\rTypes:\n\r",ch);
send_to_char("damroll exp pks hitroll eqac mac ac mana hp\n\r",ch);
return;
}

void show_topten( CHAR_DATA *ch, sh_int type )
{
int cnt=0;
TOPTEN_DATA *top, *next_top;
char buf[MSL];

for ( top = first_topten; top; top = next_top )
{
  next_top = top->next;
  if ( top->type == type )
    break;
}

ch_printf(ch,"%s\n\r\n\r",get_topten_type_name(type));

for(cnt=0; cnt <= 9; cnt++ )
{
  sprintf(buf,"%2.2d) %-25s %ld\n\r",cnt+1,capitalize(top->plist[cnt]),top->values[cnt]);
  send_to_char(buf,ch);
}

return;
}

void topten_check( CHAR_DATA *ch )
{
if ( IS_NPC(ch) )
  return;

if ( IS_IMMORTAL(ch) )
  return;

if ( !TOPTEN_ACTIVE )
  return;

if ( char_died(ch) )
return;

/* PK's Check */
  topten_update(ch->name,TOPTEN_PKS,ch->pcdata->pkills);

if ( IS_SET(ch->pcdata->flagstwo, MOREPC_NO_LIMIT_CNT) )
{
  topten_update(ch->name,TOPTEN_IMPCHAR,0);
  return;
}

if ( char_died(ch) )
return;

/* Damage Check */
  topten_update(ch->name,TOPTEN_DAMAGE,GET_DAMROLL(ch));

if ( char_died(ch) )
return;

/* Exp check */
  topten_update(ch->name,TOPTEN_EXP,ch->exp);

if ( char_died(ch) )
return;

/* Hitroll Check */
  topten_update(ch->name,TOPTEN_HITROLL,GET_HITROLL(ch));

if ( char_died(ch) )
return;

/* EQ-AC Check */
  topten_update(ch->name,TOPTEN_AC,GET_EQAC(ch));

if ( char_died(ch) )
return;

/* MAC Check */
  topten_update(ch->name,TOPTEN_MAC,GET_MAC(ch));

if ( char_died(ch) )
return;

/* TAC Check */
  topten_update(ch->name,TOPTEN_TAC,GET_AC(ch));

if ( char_died(ch) )
return;

if ( str_cmp( ch->name, "ancientone" ) ) /* Keep the bot off */
{
/* HP Check */
  topten_update(ch->name,TOPTEN_HP,ch->max_hit);

if ( char_died(ch) )
return;

/* Mana Check */
 if ( !IS_VAMPIRE(ch) )
  topten_update(ch->name,TOPTEN_MANA,ch->max_mana);
}
return;
}

char *get_topten_type_name( sh_int type )
{

switch( type )
{
  case TOPTEN_DAMAGE:
    return "--=={== Top Ten Players Sorted by Damage Roll ==}==--";
  case TOPTEN_EXP:
    return "--=={== Top Ten Players Sorted by Experiance Points ==}==--";
  case TOPTEN_PKS:
    return "--=={== Top Ten Players Sorted by Player Kills ==}==--";
  case TOPTEN_HITROLL:
    return "--=={== Top Ten Players Sorted by Hit Roll ==}==--";
  case TOPTEN_AC:
    return "--=={== Top Ten Players Sorted by EQ-AC ==}==--";
  case TOPTEN_MAC:
    return "--=={== Top Ten Players Sorted by Magical AC ==}==--";
  case TOPTEN_TAC:
    return "--=={== Top Ten Players Sorted by Total AC ==}==--";
  case TOPTEN_MANA:
    return "--=={== Top Ten Players Sorted by Mana Points ==}==--";
  case TOPTEN_HP:
    return "--=={== Top Ten Players Sorted by Hit Points ==}==--";
  default:
    return "[**ERROR**]";
}

return NULL;
}

void fwrite_topten( void )
{
FILE *fp;
TOPTEN_DATA *top, *next_top;
int cnt=0;

if ( ( fp = fopen( TOPTEN_FILE, "w" ) )==NULL )
{
  bug("fwrite_topten: cannot open db file!",0);
  return;
}

for( top = first_topten; top; top = next_top )
{
   next_top = top->next;

   
   fprintf(fp,"#ENTRY\n");
   fprintf(fp,"%d\n",top->type);

   for( cnt=0; cnt <= 9; cnt++ )
   fprintf(fp,"%s~\n",top->plist[cnt]);

   for( cnt=0; cnt <= 9; cnt++ )
   fprintf(fp,"%ld\n",top->values[cnt]);
   
}
   fprintf(fp,"#END\n");
   new_fclose(fp);
return;
}

TOPTEN_DATA *on_topten_list( TOPTEN_DATA *top, char *name, bool AC, long value )
{
int cnt=0;
TOPTEN_DATA *temp;

for( cnt = 0; cnt <= 9; cnt++ )
{
  if (!str_cmp( top->plist[cnt], name ) &&
      str_cmp( top->plist[cnt], "ancientone") )
  {
    top->values[cnt] = value;
    temp=list_resort(top,AC);
    return temp;
  }
}

return NULL;
}

TOPTEN_DATA *list_resort( TOPTEN_DATA *top, bool AC )
{
long top_value=0;
int main_cnt=0;
TOPTEN_DATA *temp=NULL;

CREATE(temp, TOPTEN_DATA, 1);

temp->type = top->type;

for( main_cnt=0; main_cnt <=9; main_cnt++ )
{
/* Make it Stop at None */
/*if ( !str_cmp( top->plist[main_cnt], "None" ) )
{
  temp->plist[main_cnt] = STRALLOC("None");
  temp->values[main_cnt] = 0;
  continue;
}*/

if ( !AC )
top_value = get_top_number(top);
else
top_value = get_low_number(top);

temp->values[main_cnt] = top->values[top_value]; 
temp->plist[main_cnt] = top->plist[top_value];
top->values[top_value] = 0;
}

return temp;
}

int get_top_number( TOPTEN_DATA *top )
{
int cnt=0;
long top_value=0;
int top_cnt=0;

for( cnt=0; cnt <= 9; cnt++ )
{

if ( top->values[cnt] > top_value )
{
  top_value = top->values[cnt];
  top_cnt=cnt;
}

}

return top_cnt;
}

int get_low_number( TOPTEN_DATA *top )
{
int cnt=0;
long top_value=0;
int top_cnt=0;

for( cnt=0; cnt <= 9; cnt++ )
{

if ( top->values[cnt] < top_value )
{
top_value = top->values[cnt];
top_cnt = cnt;
}

}

return top_cnt;
}

/* Remove Deleted Players from the Lists --GW */
void topten_exist_check( TOPTEN_DATA *top )
{
FILE *fp;
int cnt=0;
bool found;
char buf[MSL];
char file[MSL];
char *entry;

buf[0] = '\0';
file[0] = '\0';

for( cnt=0; cnt <= 9; cnt++ )
{
  found = FALSE;
  entry = top->plist[cnt];
  if ( !str_cmp(top->plist[cnt],"Ancientone") )
  {
       sprintf(buf,"TOPTEN: %s - Registered BOT, Removing.",top->plist[cnt]);
       log_string(buf);
       top->plist[cnt] = STRALLOC("None");
       top->values[cnt] = 0;
       continue;
  }
  entry[0] = LOWER(entry[0]);
  sprintf(file,"%s%c/%s.gz",PLAYER_DIR,entry[0],capitalize(entry));
  if ( ( fp=fopen(file,"r") )==NULL)
  {
    sprintf(file,"%s%c/%s",PLAYER_DIR,entry[0],capitalize(entry));
    if ( ( fp=fopen(file,"r") )==NULL)
    {
       sprintf(buf,"TOPTEN: %s Not found, Removing.",top->plist[cnt]);
       log_string(buf);
       top->plist[cnt] = STRALLOC("None");
       top->values[cnt] = 0;
    }
    else
     new_fclose(fp);
  }
  else
    new_fclose(fp);
}

return;
}
