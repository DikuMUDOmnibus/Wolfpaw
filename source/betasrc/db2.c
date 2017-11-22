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
 *    		       Custom Database Loading Module			    *
 *--------------------------------------------------------------------------*/
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include "mud.h"

void load_main_db( void );
void load_max_skills( FILE *fp );
void load_limit_list( FILE *fp );
AREA_DATA *find_area( int vnum, sh_int type );
void reset_copyover_boot_time( void );
OFFLINE_DATA *load_offline_data_file( FILE *fp );
void free_offline_data( CHAR_DATA *ch );
void object_limit_update( void );
LIMIT_DATA *limit_lookup( int zone, int vnum );
void reinit_limits( void );

extern LIMIT_DATA *first_limit;
extern LIMIT_DATA *last_limit;

#define KEY( literal, field, value )                                    \
                                if ( !str_cmp( word, literal ) )        \
                                {                                       \
                                    field  = value;                     \
                                    fMatch = TRUE;                      \
                                    break;                              \
                                }

void load_main_db( void )
{
FILE *fp;

log_string( "Loading Main Database File.....");

if( !(fp=fopen( DB_FILE, "r" ) ) )
{
bug("Loading of: %s failed, exiting.",DB_FILE);
perror(DB_FILE);
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
bug("Load_Main_DB: # not Found.",0);
exit(1);
}

word = fread_word( fp );
if ( !str_cmp(word, "MAX_SKILLS" ) ) { load_max_skills(fp); continue; }
else if ( !str_cmp(word, "END" ) ) { fread_to_eol(fp); break; }
else
{
bug("Load_Main_DB: Bad Section.",0);
exit(1);  
}
}

new_fclose(fp);
return;
}

void load_max_skills( FILE *fp )
{
char buf[MSL];
int temp;

temp = fread_number(fp);

#undef MAX_SKILL
#define MAX_SKILL temp

sprintf(buf,"Loaded Max Skills: %d",MAX_SKILL);
log_string(buf);
return;
}

void load_limit_list( FILE *fp )
{

return;
}

AREA_DATA *find_area( int vnum, sh_int type )
{
AREA_DATA *tarea, *tarea_next;
bool found=FALSE;

for ( tarea = first_area; tarea; tarea = tarea_next )
{

  tarea_next = tarea->next;

	switch( type )
	{
	   case AREA_SEARCH_MOB:
	     if ( ( tarea->low_m_vnum <= vnum ) && ( tarea->hi_m_vnum >= vnum ) )
	     found = TRUE;
	     break;
	   case AREA_SEARCH_OBJ:
	     if ( ( tarea->low_o_vnum <= vnum ) && ( tarea->hi_o_vnum >= vnum ) )
	     found = TRUE;
	     break;
	   case AREA_SEARCH_ROOM:
	     if ( ( tarea->low_r_vnum <= vnum ) && ( tarea->hi_r_vnum >= vnum ) )
	     found = TRUE;
	     break;
	   default:
	   bug("Find_Area: BAD TYPE!",0);
	   break;
	}
}

if ( found )
return tarea;

for ( tarea = first_build; tarea; tarea = tarea_next )
{

  tarea_next = tarea->next;

	switch( type )
	{
	   case AREA_SEARCH_MOB:
	     if ( ( tarea->low_m_vnum <= vnum ) && ( tarea->hi_m_vnum >= vnum ) )
	     found = TRUE;
	     break;
	   case AREA_SEARCH_OBJ:
	     if ( ( tarea->low_o_vnum <= vnum ) && ( tarea->hi_o_vnum >= vnum ) )
	     found = TRUE;
	     break;
	   case AREA_SEARCH_ROOM:
	     if ( ( tarea->low_r_vnum <= vnum ) && ( tarea->hi_r_vnum >= vnum ) )
	     found = TRUE;
	     break;
	   default:
	   bug("Find_Area: BAD TYPE!",0);
	   break;
	}
}

if ( found )
return tarea;

return NULL;
}

void reset_copyover_boot_time( void )
{
extern struct timeval now_time;
extern char str_boot_time[MAX_INPUT_LENGTH];
extern HOUR_MIN_SEC        set_boot_time_struct;
extern HOUR_MIN_SEC *      set_boot_time;
extern struct tm *         new_boot_time;
extern struct tm           new_boot_struct;
extern time_t boot_time;

log_string("Reseting Boot Time to Sysdata Saved Value");

   /*
     * Init time.
     */
    gettimeofday( &now_time, NULL );
    current_time = (time_t) now_time.tv_sec;
    boot_time = (time_t) sysdata.boottime;
    strcpy( str_boot_time, ctime( &boot_time ) );

    /*
     * Init boot time.
     */
    set_boot_time = &set_boot_time_struct;
    set_boot_time->hour   = 6;
    set_boot_time->min    = 0;
    set_boot_time->sec    = 0;
    set_boot_time->manual = 0;

    new_boot_time = update_time(localtime(&current_time));
    /* Copies *new_boot_time to new_boot_struct, and then points
       new_boot_time to new_boot_struct again. -- Alty */
    new_boot_struct = *new_boot_time;
    new_boot_time = &new_boot_struct;
    new_boot_time->tm_mday += 1;
    if(new_boot_time->tm_hour > 12)
    new_boot_time->tm_mday += 1;
    new_boot_time->tm_sec = 0;
    new_boot_time->tm_min = 0;
    new_boot_time->tm_hour = 6;

    /* Update new_boot_time (due to day increment) */
    new_boot_time = update_time(new_boot_time);
    new_boot_struct = *new_boot_time;
    new_boot_time = &new_boot_struct;

log_string("Done.");
return;
}

/* Begin Offline Data Loading/Saving/Lookup --GW */

#define OFFLINE_DIR "../offline_data/"

OFFLINE_DATA *od_lookup( char *name )
{
FILE *fp;
char buf[MSL];
OFFLINE_DATA *off;
char arg[MSL];

if ( !name) 
return NULL;

arg[0] = '\0';
name=one_argument(name,arg);

sprintf(buf,"%s%c/%s",OFFLINE_DIR,arg[0],capitalize(arg));
if( !( fp=fopen( buf, "r" ) ) )
return NULL; /* File Not Found, Assumed non-existant --GW */

//sprintf(buf,"Looking up Offline Data for: %s", capitalize(arg));
//log_string(buf);
off = load_offline_data_file( fp );
new_fclose(fp);
return off;
}

bool save_offline_data( CHAR_DATA *ch )
{
FILE *fp=NULL;
char buf[MSL];

if ( IS_NPC(ch) )
  return FALSE;

if ( !ch->pcdata->offline )
  CREATE(ch->pcdata->offline, OFFLINE_DATA, 1);

ch->pcdata->offline->name = STRALLOC(ch->name);
ch->pcdata->offline->email_address = STRALLOC(ch->pcdata->email);

if (ch->desc)
ch->pcdata->offline->last_site = STRALLOC(ch->desc->host);

ch->pcdata->offline->gold = ch->gold;
ch->pcdata->offline->bank = ch->pcdata->bank;

buf[0] = '\0';
sprintf(buf,"%s%c/%s%c",OFFLINE_DIR,tolower(ch->name[0]),capitalize(ch->name),'\0');
if ( ( fp=fopen( buf, "w" ))==NULL)
{
   bug("UNABLE TO SAVE OFFLINE DATA: %s",buf);
   return FALSE;
}

fprintf(fp,"Name		%s~\n",ch->name);
fprintf(fp,"Email		%s~\n",ch->pcdata->email);

if (ch->desc)
fprintf(fp,"Site		%s~\n",ch->desc->host);
  else
fprintf(fp,"Site		%s~\n",ch->pcdata->offline->last_site);

fprintf(fp,"Gold		%ld\n",ch->gold);
fprintf(fp,"Bank		%ld\n",ch->pcdata->bank);
fprintf(fp,"FMail		%s~\n",ch->pcdata->mail_forward);
fprintf(fp,"$\n");
new_fclose(fp);
return TRUE;
}

OFFLINE_DATA *load_offline_data_file( FILE *fp )
{
char *word=NULL;
bool fMatch=FALSE;
OFFLINE_DATA *off=NULL;

CREATE(off, OFFLINE_DATA, 1 );
for ( ; ; )
{
word = feof(fp) ? "$" : fread_word(fp);
fMatch=FALSE;
switch( UPPER(word[0]) )
{
	case '*':
                fMatch = TRUE;
                fread_to_eol( fp );
                break;

        case '$':
                fMatch = TRUE;
                return off;

      case 'B':
	KEY( "Bank",	off->bank,		fread_number(fp));
	break;

      case 'E':
	KEY( "Email",	off->email_address,	fread_string(fp));
	break;

      case 'F':
	KEY( "FMail",	off->email_fw,		fread_string(fp));
	break;

      case 'G':
	KEY( "Gold",	off->gold,		fread_number(fp));
	break;

      case 'N':
	KEY( "Name",	off->name,		fread_string(fp));
	break;

      case 'S':
	KEY( "Site",	off->last_site,		fread_string(fp));
	break;
}

        if ( !fMatch  )
        {
          bug( "Load_Letter: no match for %s.", word );
        }
   }

return off;
}

void free_offline_data( CHAR_DATA *ch )
{
if ( !ch->pcdata->offline )
return;

if ( ch->pcdata->offline->name )
 STRFREE(ch->pcdata->offline->name);
if ( ch->pcdata->offline->email_address )
 STRFREE(ch->pcdata->offline->email_address);
if ( ch->pcdata->offline->last_site )
 STRFREE(ch->pcdata->offline->last_site);

DISPOSE(ch->pcdata->offline);
return;
}

void load_char_offline_data( CHAR_DATA *ch )
{
OFFLINE_DATA *off;

if ( ( off = od_lookup( ch->name ) )==NULL)
return;

CREATE(ch->pcdata->offline, OFFLINE_DATA, 1);
ch->pcdata->offline->name = STRALLOC(off->name);
ch->pcdata->offline->email_address = STRALLOC(off->email_address);
ch->pcdata->offline->last_site = STRALLOC(off->last_site);
ch->pcdata->offline->gold = off->gold;
ch->pcdata->offline->bank = off->bank;
STRFREE(off->name);
STRFREE(off->email_address);
STRFREE(off->last_site);
DISPOSE(off);
return;
}

/* End Offline Stuff --GW */

/* Reinitialize the Limit table .. Setting all the 0 and then we do a limit update, making
 * it definetly real.. --GW
 */
void reinit_limits( void )
{
LIMIT_DATA *limit, *next_limit;

for( limit = first_limit; limit; limit = next_limit )
{
  next_limit = limit->next;
  limit->loaded = 0;
}

log_string("Limit Loaded Table Cleared, Ready for Update..");
return;
}

/* 
 * Update all the object index's for limits --GW 
 */
void obj_index_limit_update( void )
{
OBJ_INDEX_DATA *obj=NULL;
ZONE_DATA *zone=NULL;
LIMIT_DATA *limit=NULL;
int hash=0;

zone=find_zone(1);
log_string("Updating Object Indices to reflect new Limits...");

for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
 for ( obj = zone->obj_index_hash[hash]; obj; obj = obj->next )
 {
  limit=NULL;
  if ( IS_LIMITED(obj) )
  {
   limit=limit_lookup(zone->number,obj->vnum);
   if ( !limit )
   {
     bug("Vnum %d has LIMITED FLAG and no Limit Entry!",obj->vnum);
     continue;
   }
   obj->limit=limit->limit;
   obj->loaded=limit->loaded;
  }
}
log_string("Indices Update Completed Successfully.");
return;
}

/* Read objects currently in circulation -- and update limits where
 * needed --GW 
 */
void object_limit_update( void )
{
FILE *fp, *fp2;
DIR *dp;
struct dirent *de;
extern int falling;
int alpha_loop=0;
char dirbuf[MSL];
OBJ_INDEX_DATA *dummy;
LIMIT_DATA *limit;
char buf[MSL], buf2[MSL];
char logbuf[MSL];
int vnum=0;
int count=1;

for( alpha_loop=0; alpha_loop <=25; alpha_loop++)
{
  sprintf(dirbuf,"%s%c/",PLAYER_OBJ_DIR,'a' + alpha_loop);
  if ( ( dp=opendir(dirbuf))==NULL)
  {
   bug("Object_Limit_Update: Failed to open Dir %s!!",dirbuf);
   continue;
  }

  falling = 1;
  while( (de = readdir(dp)) != NULL)
  {
    if (de->d_name[0] != '.' )
    {
      sprintf(buf,"%s%c/%s",PLAYER_OBJ_DIR,'a' + alpha_loop,de->d_name);

   if ( mudarg_scan('S') )
      log_string(buf);  

      if( (fp=fopen( buf, "r" ) )==NULL)
      {
       bug("Unable to OPEN: %s",de->d_name);
       continue;
      }

      sprintf(buf2,"%s%c/%s",PLAYER_DIR,'a' + alpha_loop,de->d_name);
      if ( !(fp2=fopen( buf2,"r" ) ) )
      {
        sprintf(buf2,"%s%c/%s.gz",PLAYER_DIR,'a' + alpha_loop,de->d_name);
	if ( !(fp2=fopen( buf2,"r" ) ) ) 
	{
	  sprintf(buf2,"[LIMIT UPDATE] Object file  (%s) found without owner! Deleting..",de->d_name);
	  log_string(buf2);
	  new_fclose(fp);
	  unlink(buf);
	  continue;
	}
      }
       new_fclose(fp2);

      count=1;

      for( ; ; )
      {
        char *word;
	char *string;
        
        word = feof(fp) ? "$" : fread_word( fp );

	if ( word[0] == '$' || !str_cmp(word,"#END") )
	 break;

	/* Oops .. forgot about grouped Objects.. */
	if ( !str_cmp(word,"Count") )
	{
         count=fread_number(fp);
	 continue;
        }

	if ( !str_cmp(word,"ImpChar") )
	{
	  string=fread_string(fp);
	  count=0;
	  sprintf(logbuf,"[LIMIT UPDATE]: Imp Char Exempt from Limit Count: %s",de->d_name);
          log_string(logbuf);
	  break;
	}
	if ( !str_cmp(word,"Immortal") )
	{
	  string=fread_string(fp);
	  count=0;
	  sprintf(logbuf,"[LIMIT UPDATE]: Immortal Char Exempt from Limit Count: %s",de->d_name);
          log_string(logbuf);
	  break;
	}

        if ( !str_cmp(word,"Vnum") )
	{
	  vnum=fread_number(fp);
	  dummy=get_obj_index(vnum,1);
	  if ( !dummy )
	   continue;

	  if ( IS_LIMITED(dummy) )
	  {
	    limit=limit_lookup(dummy->area->zone->number, vnum);
          if ( !limit )	  
 	  {
	  log_string("Created New Limit Entry");
	  CREATE(limit, LIMIT_DATA, 1 );
	  limit->zone = 1;
	  limit->vnum = vnum;
	  limit->limit  = 0;
	  limit->loaded = 0;
	  LINK(limit, first_limit, last_limit, next, prev );
          }

	    if (limit->checked_this_boot==0)
	    {
  	      limit->checked_this_boot=1;
	      limit->loaded = 0;
	      dummy->loaded = 0;
	    }
	    limit->loaded += count;
	    dummy->loaded++;
	    count=1;
	   }
	}
	else
	{
	  count=1;
	  continue;
	}

      }	  
   new_fclose(fp);
   }
  }
  closedir(dp);
  falling = 0;
}
  write_limit_db();
  obj_index_limit_update();
  return;
}

void load_compile_data ( void )
{
FILE *fp;
extern char COMPILED_BY[MSL];
extern char COMPILED_TIME[MSL];
char *buf;

buf = NULL;

COMPILED_BY[0] = '\0';
COMPILED_TIME[0] = '\0';

fp=fopen("../system/compile_who","r");
buf = fread_line(fp);
sprintf(COMPILED_BY,"%s",buf);
new_fclose(fp);
fp=fopen("../system/compile_time","r");
buf = fread_line(fp);
sprintf(COMPILED_TIME,"%s",buf);
new_fclose(fp);
return;
}

void load_boot_data ( void )
{
FILE *fp;
extern char BOOTED_BY[MSL];
extern char BOOT_TIME[MSL];
char *buf;

buf = NULL;

BOOTED_BY[0] = '\0';
BOOT_TIME[0] = '\0';

fp=fopen("../system/boot_who","r");
buf=fread_line(fp);
sprintf(BOOTED_BY,"%s",buf);
new_fclose(fp);
fp=fopen("../system/boot_time","r");
buf=fread_line(fp);
sprintf(BOOT_TIME,"%s",buf);
new_fclose(fp);
return;
}

void load_mud_version ( void )
{
FILE *fp;
extern char MUD_VERSION[MSL];
extern char VERSNUM[MSL];
char *buf;

buf = NULL;

MUD_VERSION[0] = '\0';

fp=fopen("../system/compile_version","r");
buf=fread_string(fp);
sprintf(MUD_VERSION,"WME-4.0A.%s",buf);
sprintf(VERSNUM,"%s",buf);
new_fclose(fp);
return;
}

#define OVERLIMIT_FILE		"../system/overlimit.txt"

void overlimit_msg( void )
{
LIMIT_DATA *limit, *next_limit;
OBJ_INDEX_DATA *obj;
FILE *fp;
ZONE_DATA *zone;
char *objshort=NULL;

zone=find_zone(1);

fp=fopen(OVERLIMIT_FILE, "w");

if ( !mudarg_scan('B') )
{
for( limit=first_limit; limit; limit=next_limit )
{
  next_limit=limit->next;
  obj=NULL;

  if ( limit->loaded > limit->limit )
  {
    /* Get the short desc of the object --GW */
    if ( (obj=get_obj_index(limit->vnum,1))==NULL)
    {
      bug("Overlimit_Msg: Object Index Not found! (Z:%d V:%d)",zone->number,limit->vnum);
      objshort = STRALLOC("(*NOT FOUND*)");
    }
    objshort=obj->short_descr;
    fprintf(fp,"&W&w[OVER LIMIT]: &R(&W&w%s&R) &W&wVnum: &R%d &W&wLD/LT: &R%d&W&w/&R%d&W&w\n",objshort,limit->vnum,limit->loaded,limit->limit);
  }
}
}
else
{
  fprintf(fp,"Overlimit Scanner Disabled in Build Version");
}
new_fclose(fp);
return;
}        

void show_overlimit( CHAR_DATA *ch )
{
show_file(ch,OVERLIMIT_FILE);
return;
}

