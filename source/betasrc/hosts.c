/*--------------------------------------------------------------------------*
 *                         ** WolfPaw 1.0 **                                *
 *--------------------------------------------------------------------------*
 *               WolfPaw 1.0 (c) 1997,1998 by Dale Corse                    *
 *--------------------------------------------------------------------------*
 *            The WolfPaw Coding Team is headed by: Greywolf                *
 *  With the Assitance from: Callinon, Dhamon, Sentra, Wyverns, Altrag      *
 *  Scryn, Thoric, Justice, Tricops and Mask.                               *
 *--------------------------------------------------------------------------*
 *			Host Restriction Module				    *
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

/* 
 * Check the players lists to see if they are allowed. --GW
 */
bool check_hosts( CHAR_DATA *ch, char *site )
{
HOST_DATA *host, *next_host;
char buf[MSL];
bool fMatch = FALSE;

if ( !IS_SET(ch->pcdata->flagstwo, MOREPC_SITE_LOCK) )
{
  send_to_char("\n\rSITE_LOCK: Not Active, We Suggest you read help SITELOCK\n\r",ch);
  return TRUE;
}

for( host = ch->pcdata->first_host; host; host = next_host )
{
  next_host = host->next;
  if ( !str_prefix( host->site, site ) )
   fMatch=TRUE;
  if ( !str_suffix( host->site, site ) )
   fMatch=TRUE;
  if ( !str_cmp( host->site, site ) )
   fMatch=TRUE;

}

if ( !fMatch )
{
      send_to_char("\n\rSITE_LOCK: Host authentication failed! GoodBye!\n\r",ch);
      sprintf(buf,"[SECURITY NOTICE] %s attempted login from an invalid host!\n\r",capitalize(ch->name));
      echo_to_all(AT_PLAIN,buf,ECHOTAR_IMM);
      log_string(buf);
      do_quit2(ch,"");
      return FALSE;
}

send_to_char("\n\rSITE_LOCK: Host authenticated.\n\r",ch);
return TRUE;
}

/*
 * User Interface --GW
 */
void do_host( CHAR_DATA *ch, char *argument )
{
char arg1[MSL];
char arg2[MSL];

argument = one_argument( argument, arg1 );
argument = one_argument( argument, arg2 );

if ( IS_NPC(ch) )
return;

if ( arg1[0] == '\0' )
{
  send_to_char("Invalid: Read Help SITELOCK\n\r",ch);
  return;
}

if ( !str_cmp( arg1, "set" ) )
{
HOST_DATA *host=NULL;

if ( arg2[0] == '\0' )
{
  send_to_char("SITE_LOCK: Invalid: Read Help SITELOCK!\n\r",ch);
  return;
}

CREATE(host,HOST_DATA,1);
host->site=STRALLOC(arg2);
LINK(host,ch->pcdata->first_host,ch->pcdata->last_host,next,prev);
send_to_char("SITE_LOCK: Lock added.\n\r",ch);
SET_BIT(ch->pcdata->flagstwo, MOREPC_SITE_LOCK);
return;
}

if ( !str_cmp( arg1, "list" ) )
{
HOST_DATA *host, *next_host;

if ( !IS_SET(ch->pcdata->flagstwo, MOREPC_SITE_LOCK) )
{
  send_to_char("Your character is not SITELOCK'd\n\r",ch);
  return;
}

send_to_char("SITE_LOCK: Site List\n\r",ch);
send_to_char("====================\n\r",ch);
for( host = ch->pcdata->first_host; host; host = next_host )
{
  next_host = host->next;
  ch_printf(ch,"SITE_LOCK: %s\n\r",host->site);
}

return;
}

if ( !str_cmp( arg1, "delete" ) )
{
HOST_DATA *host, *next_host;
bool fMatch = FALSE;

if ( !IS_SET(ch->pcdata->flagstwo, MOREPC_SITE_LOCK) )
{
  send_to_char("Your character is not SITELOCK'd\n\r",ch);
  return;
}

for( host = ch->pcdata->first_host; host; host = next_host )
{
  next_host = host->next;
  if ( !str_cmp( host->site, arg2 ) )
  {
   STRFREE(host->site);
   UNLINK(host,ch->pcdata->first_host,ch->pcdata->last_host,next,prev);
   DISPOSE(host);
   send_to_char("SITE_LOCK: Site lock deleted.\n\r",ch);
   fMatch = TRUE;
   if ( ch->pcdata->first_host == NULL )
   REMOVE_BIT( ch->pcdata->flagstwo, MOREPC_SITE_LOCK);
   }
}

if ( !fMatch )
send_to_char("SITE_LOCK: Delete failed: Site not found.\n\r",ch);

return;
}

if ( !str_cmp( arg1, "mysite" ) )
{
ch_printf(ch,"SITE_LOCK: Your current site is: %s\n\r",ch->desc->host);
return;
}

send_to_char("Invalid: Read Help SITELOCK\n\r",ch);
return;
}

/*
 * Load a SITE_LOCK --GW
 */
void load_host( CHAR_DATA *ch, char *line )
{
HOST_DATA *host=NULL;

CREATE( host, HOST_DATA, 1 );
host->site = STRALLOC(line);
LINK(host,ch->pcdata->first_host,ch->pcdata->last_host,next,prev);
return;
}

/*
 * Save a players' Host locks --GW
 */
void save_hosts( CHAR_DATA *ch, FILE *fp )
{
HOST_DATA *host, *next_host;

if ( !IS_SET(ch->pcdata->flagstwo, MOREPC_SITE_LOCK) )
return;

for( host=ch->pcdata->first_host; host; host = next_host )
{
  next_host = host->next;
  smash_tilde(host->site);
  fprintf(fp,"SLOCK %s~\n",host->site);
}
return;
}

/* Setup the linked lists --GW */
void host_setup( CHAR_DATA *ch )
{
ch->pcdata->first_host = NULL;
ch->pcdata->last_host = NULL;
return;
}

/* Free the data --GW */
void free_hosts( CHAR_DATA *ch )
{
HOST_DATA *host, *next_host;

for( host = ch->pcdata->first_host; host; host = next_host )
{
  next_host = host->next;
  STRFREE(host->site);
  UNLINK(host,ch->pcdata->first_host, ch->pcdata->last_host, next, prev);
  DISPOSE(host);
}

return;
}

