#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "mud.h"

void do_remor( CHAR_DATA *ch, char *argument )
{
   record_call("<do_remor>");
    send_to_char( "If you want to REMORT, you must spell it out.\n\r", ch );
    return;
}

void do_remort( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    char strsave[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
   record_call("<do_remort>");

    if ( IS_NPC(ch) || ( d = ch->desc ) == NULL )
	return;

    if ( ch->level != 50 || ch->level2 != 50 || 
	IS_SET(ch->pcdata->flags,PCFLAG_ADVANCED))
    {
	send_to_char(
	    "You must be level 50 in both classes to become advanced.\n\r", ch );
	return;
    }

else
	{
	    /*
	     * Get ready to delete the pfile, send a nice informational message.
	     */
	    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( ch->name ) );
	    stop_fighting( ch, TRUE );
	    send_to_char( "You have chosen to advance.  You will now be dropped in at the race\n\r", ch );
	    send_to_char( "selection section of character creation, and will be allowed to choose from\n\r", ch );
	    send_to_char( "a wider selection of races and classes.\n\r\n\r", ch );
	    send_to_char( "In the unlikely event that you are disconnected or the MUD\n\r", ch );
	    send_to_char( "crashes while you are creating your character, create a new character\n\r", ch );
	    send_to_char( "as normal and write a note to 'immortal'.\n\r", ch );
	    send_to_char( "\n\r\n\r", ch );
	sprintf(buf,"%s has Advanced!", ch->name );
	    log_string( buf );

	send_to_char( "<*<* Current Advanced Classes Available: *>*>\n\r\n\r",ch);
        send_to_char( "[1]Avatar      Available to: All\n", ch );
        send_to_char( "[2]Dragon      Available to: All\n\r",ch);
 	send_to_char( "\n\r *** NOTE: READ THE HELP FILES ***\n\r",ch);
        send_to_char( "Select a class: ", ch );
 
            ch->advlevel = 1;
            ch->exp      = exp_level(ch, 1);
	    SET_BIT(ch->pcdata->flags, PCFLAG_ADVANCED);
	    d->connected = CON_GET_NEW_REMORT_CLASS;	    
	    return;
	}

    if ( argument[0] != '\0' )
    {
	send_to_char( "Just type remort.  No argument.\n\r", ch );
	return;
    }

}
