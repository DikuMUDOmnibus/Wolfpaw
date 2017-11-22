/*--------------------------------------------------------------------------*
 *                         ** WolfPaw 1.0 **                                *
 *--------------------------------------------------------------------------*
 *               WolfPaw 1.0 (c) 1997,1998 by Dale Corse                    *
 *--------------------------------------------------------------------------*
 *            The WolfPaw Coding Team is headed by: Greywolf                *
 *  With the Assitance from: Callinon, Dhamon, Sentra, Wyverns, Altrag      *
 *  Scryn, Thoric, Justice, Tricops and Mask.                               *
 *--------------------------------------------------------------------------*
 *		       Equipment Limits Handling Module			    *
 *--------------------------------------------------------------------------*/
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include "mud.h"

/* Prototypes --GW */
void load_limit_db( void );
void write_limit_db( void );
bool obj_limit_check( OBJ_INDEX_DATA *obj );
void load_limited_toggle( bool on_off  );
void adjust_limits( OBJ_INDEX_DATA *obj, int mod, int type );
LIMIT_DATA *limit_lookup( int zone, int vnum );
void set_limits( void );
void create_limit_entry( OBJ_DATA *obj );
void delete_limit_entry( OBJ_DATA *obj );

/* Limit File */
#define LIMITDB_FILE 	"../system/limits.db"

/* Globals */
bool LOAD_LIMITED;
bool limits_off;
LIMIT_DATA *first_limit;
LIMIT_DATA *last_limit;

/*
 * Load the LIMIT DB --GW
 */
void load_limit_db( void )
{
FILE *fp;
char buf[MSL];
LIMIT_DATA *limit=NULL;

log_string("Initiazlizing Limits System...");
first_limit = NULL;
last_limit = NULL;
LOAD_LIMITED = TRUE;
limits_off = TRUE;

log_string("Opening Limit Database");
if ( !( fp=fopen( LIMITDB_FILE, "r" ) ) )  
{
bug("Load_Limit_DB: Unable to open LIMITDB_FILE!",0);
bug("Limit Code Disabled.",0);
limits_off = TRUE;
return;
}


log_string("Reading Limits Database");
for( ; ;  )
{
char *ln;
int xZONE=0,xVNUM=0,xLIMIT=0,xLOADED=0;

if ( feof(fp) )
break;

ln = fread_line( fp );

if ( feof(fp) || !str_cmp(ln,"$"))
break;

xZONE=xVNUM=xLIMIT=xLOADED=0;
sscanf( ln, "%d %d %d %d", &xZONE, &xVNUM, &xLIMIT, &xLOADED );
limit=NULL;
CREATE( limit, LIMIT_DATA, 1 );
limit->zone 	= xZONE;
limit->vnum	= xVNUM;
limit->limit	= xLIMIT;
limit->loaded	= xLOADED;
limit->checked_this_boot = 0;
LINK( limit, first_limit, last_limit, next, prev );
sprintf(buf,"LIMIT LOADED: Z:%d, V:%d, LIMIT: %d, LOADED: %d",
	xZONE, xVNUM, xLIMIT, xLOADED);
log_string(buf);

}
new_fclose(fp);
log_string("Database Read");
set_limits();
return;
}

void set_limits( void )
{
LIMIT_DATA *limit, *limit_next;
OBJ_INDEX_DATA *obj;

log_string("Setting Object Limits...");
for( limit=first_limit; limit; limit=limit_next)
{
limit_next = limit->next;

obj = get_obj_index( limit->vnum, limit->zone );
if ( !obj )
continue;
obj->limit		= limit->limit;
obj->loaded		= limit->loaded; 
}

log_string("Object Limits Set");

log_string("Done Loading Limits");
return;
}

#define LIMIT_DBBACK   "../system/limits.bak"

/*
 * Save LIMIT DB --GW
 */
void write_limit_db( void )
{
LIMIT_DATA *limit,*limit_next;
FILE *fp;
int cnt=0;

rename( LIMITDB_FILE, LIMIT_DBBACK);

if ( !( fp=fopen( LIMITDB_FILE, "w" ) ) )
{
bug("Write_Limit_DB: Open DB Failed",0);
return;
}

for( limit = first_limit; limit; limit = limit_next )
{
limit_next = limit->next;
fprintf( fp, "%d %d %d %d\n",limit->zone,limit->vnum,limit->limit,limit->loaded);
cnt++;
}

/* Bug Fix - Empty Db crashes --Feyr */
if ( cnt == 0 )
fprintf(fp,"$\n");

new_fclose(fp);
return;
}

/*
 * LIMITS Control Panel, for Hyperions. --GW
 */
void do_limits( CHAR_DATA *ch, char *argument )
{
ZONE_DATA *zone;
OBJ_INDEX_DATA *obj;
LIMIT_DATA *limit=NULL;

if ( str_cmp( ch->name, "greywolf" ) )
{
send_to_char("I think you better ask Greywolf about this before you use it!",ch);
return;
}

send_to_char("Updating Limits DB\n\r",ch);

first_limit = NULL;
last_limit = NULL;

for( zone = first_zone; zone; zone = zone->next )
{
int cnt=0;

for ( cnt = 1; cnt < 33000; cnt++ )
{
obj = get_obj_index(cnt,zone->number);

if ( !obj )
continue;

if ( IS_SET( obj->second_flags, ITEM2_LIMITED ) )
{
  CREATE( limit, LIMIT_DATA, 1 );
  limit->zone 	= zone->number;
  limit->vnum   = obj->vnum;
  limit->limit  = obj->limit;
  limit->loaded = obj->loaded;
  LINK(limit, first_limit, last_limit, next, prev );
  log_string("Created Limit");
}
}
}

write_limit_db();
send_to_char("Done",ch);
return;
}

/*
 * Check and See if the Object is at limit --GW
 * Returns TRUE if OVER or AT limit
 * FALSE if not.
 */
bool obj_limit_check( OBJ_INDEX_DATA *obj )
{
LIMIT_DATA *limit;

if ( !LOAD_LIMITED )
return FALSE;

limit = limit_lookup(obj->area->zone->number,obj->vnum);

if ( !limit )
return FALSE;

if ( limit->limit <= limit->loaded )
return TRUE;

return FALSE;
}

/*
 * Toggle LIMIT Counting on and off --GW
 */
void load_limited_toggle( bool on_off  )
{
LOAD_LIMITED=on_off;
return;
}

/*
 * Change an Objects limit, or how many are currently in circulation --GW
 */
void adjust_limits( OBJ_INDEX_DATA *obj, int mod, int type )
{
LIMIT_DATA *limit;

if ( !LOAD_LIMITED || limits_off == TRUE )
return;

switch( type )
{
	case LOADED_ADJUST_UP:
	limit = limit_lookup(obj->area->zone->number, obj->vnum);
	if ( !limit ) break;
	limit->loaded += mod;
	obj->loaded += mod;
	write_limit_db();
	break;
	case LOADED_ADJUST_DOWN:
	limit = limit_lookup(obj->area->zone->number, obj->vnum);
	if ( !limit ) break;
	limit->loaded -= mod;
	obj->loaded -= mod;
	write_limit_db();
	break;
	case LIMIT_ADJUST_SET:
	limit = limit_lookup(obj->area->zone->number, obj->vnum);
	if ( !limit ) break;
	limit->limit = mod;
	obj->limit = mod;
	write_limit_db();
	break;
	case LOADED_ADJUST_SET:
	limit = limit_lookup(obj->area->zone->number,obj->vnum);
	if ( !limit ) break;
	limit->loaded = mod;
	obj->loaded = mod;
	write_limit_db();
	break;
	default:
	bug("Adjust_Limits: BAD TYPE!",0);
	return;
}

return;
}

LIMIT_DATA *limit_lookup( int zone, int vnum )
{
LIMIT_DATA *limit, *limit_next;

for ( limit = first_limit; limit; limit = limit_next)
{
limit_next = limit->next;

if ( ( limit->zone == zone ) && ( limit->vnum == vnum ) )
return limit;

}

return NULL;
}

/*
 * Create a New entry in the limit table --GW
 */
void create_limit_entry( OBJ_DATA *obj )
{
LIMIT_DATA *limit;

limit = NULL;
limit = limit_lookup(obj->pIndexData->area->zone->number, obj->pIndexData->vnum);

if (limit != NULL)
{
delete_limit_entry(obj);
return;
}

log_string("Adding Entry to Limits Table.");
limit = NULL;
CREATE(limit, LIMIT_DATA, 1 );
limit->zone = obj->pIndexData->area->zone->number;
limit->vnum = obj->pIndexData->vnum;
limit->limit  = obj->pIndexData->limit;
limit->loaded = obj->pIndexData->loaded;
LINK(limit, first_limit, last_limit, next, prev );
write_limit_db();
return;
}

/*
 * Remove an entry from the Limit Table --GW
 */
void delete_limit_entry( OBJ_DATA *obj )
{
LIMIT_DATA *limit;

limit = NULL;
limit = limit_lookup(obj->pIndexData->area->zone->number, obj->pIndexData->vnum);

if ( !limit )
return;

log_string("Deleting Entry from Lomit Table..");
UNLINK(limit, first_limit, last_limit, next, prev );
DISPOSE(limit);
return;
}

