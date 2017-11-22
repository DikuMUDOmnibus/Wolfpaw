/*--------------------------------------------------------------------------*
 *                         ** WolfPaw 1.0 **                                *
 *--------------------------------------------------------------------------*
 *               WolfPaw 1.0 (c) 1997,1998 by Dale Corse                    *
 *--------------------------------------------------------------------------*
 *            The WolfPaw Coding Team is headed by: Greywolf                *
 *  With the Assitance from: Callinon, Dhamon, Sentra, Wyverns, Altrag      *
 *  Scryn, Thoric, Justice, Tricops and Mask.                               *
 *--------------------------------------------------------------------------*
 *     	        Commands and Functions for use with Zones		    *	
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

#define MIL MAX_INPUT_LENGTH

bool   fold_zone	   args( ( ZONE_DATA *pZone, char *filename ) );
bool   write_zone_list     args( ( void ) );
ZONE_DATA *make_new_zone   args( ( char *filename ) );

/*
 * Creates a New Zone, and saves all the files assocsiated with it --GW
 */
void do_zcreate( CHAR_DATA *ch, char *argument )
{
char arg[MIL];
ZONE_DATA *new_zone;
bool written;
record_call("<do_zcreate>");
written = FALSE;

one_argument( argument, arg );

if ( arg[0] == '0' )
{
send_to_char("Syntax: zcreate <zone name>\n\r",ch);
return;
}

if ( (new_zone=make_new_zone(arg) ) == NULL )
{
bug("MAKE_NEW_ZONE FAILED!!!!!",0);
send_to_char("Zone Creation FAILED!\n\r",ch);
return;
}

send_to_char("Zone Created\n",ch);
ch_printf(ch,"Zone Number: %d Zone Name: %s\n", new_zone->number,
		new_zone->name );
ch_printf(ch,"Zone Filename: %s Zone List File: zone%d/%s\n",new_zone->filename,
		new_zone->number,new_zone->list_filename );

written = write_zone_list( );

if( written )
{
send_to_char("Zone Creation Complete, Zone Installed.\n\r",ch);
return;
}

bug("Write_zone_list: Write FAILED!",0);
send_to_char("Zone Creation FAILED!!!\n\r",ch);
return;
}

/*
 * Creates the new Zone, adds it to the list, and sets all the
 * Nessesary infromation on the zone. --GW
 */
ZONE_DATA *make_new_zone( char *name )
{
char   filename_buf[MSL];
char   list_filename_buf[MSL];
int    number;
extern int top_zone;
ZONE_DATA *pZone=NULL;
char buf[MSL];

number = top_zone++;
strcpy(list_filename_buf,"area.lst");
sprintf(filename_buf,"zone%d.zone",number);

    CREATE( pZone, ZONE_DATA, 1 );
    pZone->name         	= name;
    pZone->filename     	= str_dup( filename_buf );
    pZone->number		= number;
    pZone->list_filename	= str_dup( list_filename_buf );
    pZone->first_mob		= NULL;
    pZone->last_mob		= NULL;
    pZone->first_obj		= NULL;
    pZone->last_obj		= NULL;
    
    LINK( pZone, first_zone, last_zone, next, prev );
    top_zone++;

    sprintf(buf,"mkdir zone%d",pZone->number);
    system(buf);
    log_string("Zone Directory Created.");
  
    if ( fold_zone(pZone,pZone->filename)== FALSE )
    { 
	bug("Fold_Zone FAILED!",0);
        return pZone;
    }

return pZone;
}

/*
 * 'folds' a Zone file, saving it to disk. --GW
 */
bool fold_zone( ZONE_DATA *pZone, char *filename )
{
char buf[MSL];
FILE *fp;

    sprintf( buf, "Saving %s...", filename );
    log_string_plus( buf, LOG_NORMAL, LEVEL_GOD );
             sprintf( buf, "%s.bak", filename );
             rename( filename, buf );

    if ( ( fp = fopen( filename, "w" ) ) == NULL )
    {
        bug( "Fold_Zone: fopen failed.", 0 );
        perror( filename );
        return FALSE;
    }

    fprintf( fp,"#ZONE	%s~\n\n", pZone->name );
    fprintf( fp,"#NUMBER\n%d\n\n",pZone->number );
    fprintf( fp,"#LIST_NAME %s~\n\n",pZone->list_filename );

    fprintf( fp,"#$\n" );
    new_fclose( fp );
    return TRUE;
}   

/*
 * Writes all zone filenames to the zone.lst file. --GW
 */
bool write_zone_list( void )
{
ZONE_DATA *in_zone;
FILE *fp;

    if ( ( fp = fopen( ZONE_LIST, "w" ) ) == NULL )
    {
        bug( "Write_Zone_List: fopen failed.", 0 );
        perror( ZONE_LIST );
        return FALSE;
    }

    for( in_zone = first_zone; in_zone; in_zone = in_zone->next )
    {
        fprintf( fp,"%s\n",in_zone->filename);
    }

    fprintf( fp,"$");
    new_fclose( fp );
    return TRUE;
}

/*
 * Sets all the statistics for zones, use with CARE --GW
 */
void do_zset( CHAR_DATA *ch, char *argument )
{
char arg1[MIL];
char arg2[MIL];
char arg3[MIL];
ZONE_DATA *in_zone;
record_call("<do_zset>");

argument = one_argument( argument, arg1 );
argument = one_argument( argument, arg2 );
argument = one_argument( argument, arg3 );

if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
{
   send_to_char("Syntax: zset <zone> <field> <argument>\n\r\n\r",ch);
   send_to_char("field being one of:\n\r",ch);
   send_to_char("number name list_filename filename\n\r",ch);
   return;
}

if ( ( in_zone=find_zone(atoi(arg1)) )== NULL )
{
send_to_char("No such zone exists\n\r",ch);
return;
}

if ( !str_cmp( arg2, "number" ) )
{
if ( !is_number( arg3 ) )
{
send_to_char( "Invalid Argument.\n\r",ch);
return;
}

in_zone->number = atoi(arg3);
send_to_char("Done.\n\r",ch);
return;
}

if ( !str_cmp( arg2, "name" ) )
{
smash_tilde(arg3);

if( arg3[0] != '\0' )
STRFREE(in_zone->name );

in_zone->name = str_dup(arg3);
send_to_char("Done.\n\r",ch);
return;
}

if ( !str_cmp( arg2, "list_filename" ) )
{
smash_tilde(arg3);

if( arg3[0] != '\0' )
STRFREE(in_zone->list_filename );

in_zone->list_filename = str_dup(arg3);
send_to_char("Done.\n\r",ch);
return;
}

if ( !str_cmp( arg2, "filename" ) )
{
smash_tilde(arg3);

if( arg3[0] != '\0' )
STRFREE(in_zone->filename );

in_zone->filename = str_dup(arg3);
send_to_char("Done.\n\r",ch);
return;
}

send_to_char("Invalid Field.\n\r",ch);
return;
}

void do_zstat( CHAR_DATA *ch, char *argument )
{
ZONE_DATA *in_zone;
int max_vnum;
int vnum_count;
char arg[MIL];
record_call("<do_zstat>");

one_argument( argument, arg );

if ( arg[0] == '\0' )
{
send_to_char("Syntax: zstat <zone #>\n\r",ch);
return;
}

if ( (in_zone=find_zone(atoi(arg)))==NULL )
{
send_to_char("No such zone exists.\n\r",ch);
return;
}
max_vnum = 32767;
vnum_count = in_zone->top_mob;
if( vnum_count < in_zone->top_obj ) vnum_count = in_zone->top_obj;
if( vnum_count < in_zone->top_room ) vnum_count = in_zone->top_room;

ch_printf( ch,"Zone: %s	  Number: %d\n\r",in_zone->name,in_zone->number );
ch_printf( ch,"List Filename: %s\n\r",in_zone->list_filename );
ch_printf( ch,"Filename: %s\n\r",in_zone->filename );
ch_printf( ch,"\n\rMaximum Usable Vnums: %d  Vnums in use: %d\n\r",
		max_vnum,vnum_count);
ch_printf( ch,"Objects: %d  Mobiles: %d  Rooms: %d\n\r",in_zone->top_obj,
		in_zone->top_mob,in_zone->top_room );
return;
}

void do_zlist( CHAR_DATA *ch, char *argument )
{
char buf[MSL];
ZONE_DATA *in_zone;
int znum = 0;
int percent;
int vnum_count, max_vnum;
record_call("<do_zlist>");
for ( in_zone = first_zone; in_zone; in_zone = in_zone->next )
{
percent = 0;
buf[0] = '\0';
max_vnum = 32767;
vnum_count = in_zone->top_mob;
if( vnum_count < in_zone->top_obj ) vnum_count = in_zone->top_obj;
if( vnum_count < in_zone->top_room ) vnum_count = in_zone->top_room;
percent = max_vnum / vnum_count;
sprintf(buf,"%d	%-15s   %d%% full.\n",in_zone->number,in_zone->name,percent);
send_to_char(buf,ch);
znum++;
}
sprintf(buf,"\n\rTotal Zones: %d\n\r",znum);
send_to_char(buf,ch);
return;
}

