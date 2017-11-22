/*--------------------------------------------------------------------------*
 *                         ** WolfPaw 1.0 **                                *
 *--------------------------------------------------------------------------*
 *               WolfPaw 1.0 (c) 1997,1998 by Dale Corse                    *
 *--------------------------------------------------------------------------*
 *            The WolfPaw Coding Team is headed by: Greywolf                *
 *  With the Assitance from: Callinon, Dhamon, Sentra, Wyverns, Altrag      *
 *  Scryn, Thoric, Justice, Tricops and Mask.                               *
 *--------------------------------------------------------------------------*
 *		         Player Logging Module				    *
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
#include <fcntl.h>
#include "mud.h"

#define PLOG_DIR "../plog"
#define MAX_LOG 300000 /* 300k Meg Max for plog Files */

/*
 * Write a Line to a plog'd Mort or Immorts Log File, at Wyverns Request
 * Written by Greywolf
 * Log Flip Part by: SamMaEl
 */
void write_plog( char *name, int vnum, char *string )
{
FILE *fp;
int fd, length;
char buf[MSL];
char log_buf[MSL];
char *strtime;

/* Set the Filename */
sprintf( buf, "%s/%s", PLOG_DIR, name );

/* Open the file in Append Mode, if it is not there, Create it --GW */
if ( ( fp=fopen( buf, "a" ) )==NULL)
{

sprintf(log_buf,"PLOG: Starting New PLOG File for: %s", capitalize(name));
log_string_plus( log_buf, LOG_NORMAL, 59 );

if (  ( fp=fopen( buf, "w" ) )==NULL)
{
bug("PLOG: Cannot Open PLOG file for: %s!",capitalize(name));
return;
}

new_fclose(fp);
}
new_fclose(fp);

strtime                    = ctime( &current_time );
strtime[strlen(strtime)-1] = '\0';

/* Begin Sammuel's Code */
   fd = open(buf, O_CREAT | O_WRONLY | O_SYNC, S_IRUSR | S_IWUSR);
   length = lseek(fd, 0, SEEK_END);

   if (length > MAX_LOG)
   {
      close(fd);
      fd = open(buf, O_CREAT | O_WRONLY | O_SYNC | O_TRUNC, S_IRUSR | S_IWUSR);
   }

   sprintf( log_buf, "%s [%d]: %s\n", strtime,vnum, string );
   write(fd, log_buf, strlen(log_buf));
   close(fd);
/* End Sammuel's Code -- Thanks Bud --GW */

return;
}

/*
 * Checks to see if the Person should be PLOGGED
 * Currently Yes if: 1. Player has a PLOG bit, or is Immortal --GW
 */
void plog_check( CHAR_DATA *ch, char *string )
{

if ( IS_NPC(ch) )
return;

if ( IS_IMMORTAL(ch) )
write_plog( ch->name, ch->in_room->vnum, string );

if ( IS_SET( ch->pcdata->flags, PCFLAG_PLOG ) )
write_plog( ch->name, ch->in_room->vnum, string );

return;
}

/*
 * Hyperion Command -- Controls Plogging
 * Sets Bit, Removes Bit, Can View Log from in Game
 */
void do_plog( CHAR_DATA *ch, char *argument )
{
char arg[MIL];
char arg2[MIL];
CHAR_DATA *victim=NULL;
char buf[MSL];

argument = one_argument( argument, arg );
argument = one_argument( argument, arg2 );

if ( arg[0] == '\0' || arg2[0] == '\0' )
{
send_to_char("Syntax: plog <char> <on/off/view>\n\r",ch);
return;
}

if ( !str_cmp( arg2, "on" ) || !str_cmp( arg2, "off" ) )
{
if ( ( victim = get_char_world( ch, arg ) )==NULL)
{
send_to_char("I don't see that person here...\n\r",ch);
return;
}
}

if ( !str_cmp( arg2, "on" ) )
{
SET_BIT( victim->pcdata->flags, PCFLAG_PLOG );
send_to_char("PLOG Bit Set.\n\r",ch);
return;
}

if ( !str_cmp( arg2, "off" ) )
{
REMOVE_BIT( victim->pcdata->flags, PCFLAG_PLOG );
send_to_char("PLOG Bit Removed.\n\r",ch);
return;
}

if ( !str_cmp( arg2, "view" ) )
{
sprintf(buf,"%s/%s",PLOG_DIR,capitalize(arg));
show_file( ch, buf );
return;
}

send_to_char("Syntax: plog <char> <on/off/view>\n\r",ch);
return;
}

