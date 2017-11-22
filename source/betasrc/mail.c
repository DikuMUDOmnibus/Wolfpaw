/*--------------------------------------------------------------------------*
 *                         ** WolfPaw 1.0 **                                *
 *--------------------------------------------------------------------------*
 *               WolfPaw 1.0 (c) 1997,1998 by Dale Corse                    *
 *--------------------------------------------------------------------------*
 *            The WolfPaw Coding Team is headed by: Greywolf                *
 *  With the Assitance from: Callinon, Dhamon, Sentra, Wyverns, Altrag      *
 *  Scryn, Thoric, Justice, Tricops and Mask.                               *
 *--------------------------------------------------------------------------*
 *		Custom Mud-Mail Functions Module			    *
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

void load_letter	args( (MAIL_DATA *mail, FILE *fp ) );
void write_mail		args( ( void ) );
void edit_mail		args( ( CHAR_DATA *ch, MAIL_DATA *mail ) );
bool forward_mail( MAIL_DATA *mail, char *fw_email );
OFFLINE_DATA *od_lookup( char *name );

int mail_complete=0;
char *to;
char *subject;

#define KEY( literal, field, value )                                    \
                                if ( !str_cmp( word, literal ) )        \
                                {                                       \
                                    field  = value;                     \
                                    fMatch = TRUE;                      \
                                    break;                              \
                                }


void do_mail( CHAR_DATA *ch, char *argument )
{
MAIL_DATA *mail=NULL;
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
char fname[MSL];
char *strtime;
struct stat fst;
CHAR_DATA *mob;
OFFLINE_DATA *od=NULL;

argument = one_argument( argument, arg1 );
argument = one_argument( argument, arg2 );


     /*
      * Check for postmaster.
      */
         for ( mob = ch->in_room->first_person; mob; mob = mob->next_in_room )
           {
            if ( IS_NPC(mob) && IS_SET(mob->act, ACT_POSTMASTER) )
	    {
                 break;
	    }
	   }

	    if ( !mob )
	    {
	    send_to_char("I see no Postmaster here.\n\r",ch);
	    return;
	    }


     if ( mail_complete != 1 )
     {
	
	if ( arg1[0] == '\0' )
	{
	  send_to_char("Syntax: mail <char> <subject(optional)>\n\r",ch);
	  return;
	}

        arg1[0] = UPPER(arg1[0]);

        sprintf( fname, "%s%c/%s.gz", PLAYER_DIR, tolower(arg1[0]),
                 capitalize( arg1 ) );
	if ( stat( fname, &fst ) == -1 )
        sprintf( fname, "%s%c/%s", PLAYER_DIR, tolower(arg1[0]),
                 capitalize( arg1 ) );


        if ( stat( fname, &fst ) != -1 )
	{
	    to = STRALLOC( arg1 );
        }
        else
        {
            send_to_char("No player exists by that name.\n\r",ch);
            return;
        }

	if ( !str_cmp( arg1, ch->name ) )
	{
	  send_to_char("The Postmaster tells you 'You can't mail yourself!'\n\r",ch);
	  return;
	}

	if ( ch->gold < 150 )
	{
	  send_to_char("The Postmaster tells you 'You need 150 copper coins for a stamp.'\n\r",ch);
	  return;
	}
	else
	{
	  send_to_char("The Postmaster takes 150 copper coins for a stamp.\n\r",ch);
	  ch->gold -= 150;
	}

	if ( arg2[0] != '\0' )
	{
	    subject = STRALLOC( arg2 );
	}
	else
	{
	    send_to_char("No Subject specified, setting Subject to None\n\r",ch);
	    subject = STRALLOC( "None" );
	}
    }

	    strtime	= ctime( &current_time );
	    strtime[strlen(strtime)-1]	= '\0';
	    CREATE( mail, MAIL_DATA, 1 );
	    mail->sender = STRALLOC( ch->name );
	    mail->to 	 = STRALLOC( capitalize(to) );
	    mail->subject = STRALLOC( subject );
	    mail->date = STRALLOC( strtime );
	    mail->text	= STRALLOC( "" );
	    mail->read = 0;
	    edit_mail( ch, mail );
	    if ( mail_complete != 1 )
	    {
	    if ((od=od_lookup(mail->to))!=NULL)
	    {
	      if ( od->email_fw != NULL && str_cmp(od->email_fw, "(null)"))
	      {
               forward_mail(mail,od->email_fw);
	       ch_printf(ch,"Your Mud-Mail was Email-Forwarded to %s.\n\r",mail->to);
  	       DISPOSE(mail);
	       DISPOSE(od);
	       write_mail();
	       return;
	     }
	     DISPOSE(od);
	    }
            LINK(mail,first_mail,last_mail,next,prev);
            write_mail();
	    STRFREE( to );
	    STRFREE( subject );
	    }
            write_mail();
	    return;
}


void edit_mail( CHAR_DATA *ch, MAIL_DATA *mail )
{
	switch( ch->substate )
	{
	  default:
	    bug("do_mail: illegal substate.",0);
	    return;
	  
	  case SUB_RESTRICTED:
	    send_to_char("You can't do that from within another command.\n\r",ch);
	    return;

	  case SUB_NONE:
	    ch->substate = SUB_WRITING_NOTE;
	    ch->dest_buf = ch;
	    mail_complete = 1;
            start_editing( ch, mail->text );
   	    act( AT_BLUE,"$n starts writing some mail.",ch,NULL,NULL,TO_ROOM);
	    return;

	  case SUB_WRITING_NOTE:
	    STRFREE( mail->text );
	    mail->text = copy_buffer( ch );
	    stop_editing( ch );
	    mail_complete = 0;
	    act( AT_BLUE,"$n finishes sending some mail.",ch,NULL,NULL,TO_ROOM);
	    send_to_char("Mail Sent.\n\r",ch);
	    return;
	}
}

void do_receive( CHAR_DATA *ch, char *argument )
{
MAIL_DATA *mail,*mail_next=NULL;
char arg[MAX_INPUT_LENGTH];
OBJ_DATA *paper=NULL;
ZONE_DATA *in_zone;
char notebuf[MAX_STRING_LENGTH];
char short_desc_buf[MAX_STRING_LENGTH];
char long_desc_buf[MAX_STRING_LENGTH];
char keyword_buf[MAX_STRING_LENGTH];
bool found;
CHAR_DATA *mob;
int cnt=0;
bool postmaster;

arg[0] = '\0';

one_argument( argument, arg );

notebuf[0] = '\0';
short_desc_buf[0] = '\0';
long_desc_buf[0] = '\0';
keyword_buf[0] = '\0';
postmaster = FALSE;

     /*
      * Check for postmaster.
      */
         for ( mob = ch->in_room->first_person; mob; mob = mob->next_in_room )
           {
            if ( IS_NPC(mob) && IS_SET(mob->act, ACT_POSTMASTER) )
	    {
		 postmaster = TRUE;
                 break;
	    }
	   }

	    if ( !postmaster )
	    {
	    send_to_char("I see no Postmaster here.\n\r",ch);
	    return;
	    }

in_zone = find_zone(1);
found = FALSE;

for ( mail = first_mail; mail; mail = mail_next )
{
mail_next = mail->next;

if( !str_cmp( mail->to, ch->name ) && ( mail->read == 0 ) )
{
cnt++;
found = TRUE;
paper = create_object(get_obj_index(OBJ_VNUM_NOTE,1),0,in_zone);
		    sprintf(notebuf,"*-*-* Age of the Ancients Mud-Mail System *-*-*");
		    strcat(notebuf, "\n\nDate: ");
		    strcat(notebuf, mail->date);
                    strcat(notebuf, "\n\rFrom: ");
                    strcat(notebuf, mail->sender);
                    strcat(notebuf, "\n\rTo: ");
                    strcat(notebuf, mail->to);
                    strcat(notebuf, "\n\rSubject: ");
                    strcat(notebuf, mail->subject);
                    strcat(notebuf, "\n\r\n\r");
                    strcat(notebuf, mail->text);
                    strcat(notebuf, "\n\r");
                    paper->spare_desc = STRALLOC(notebuf);
                    paper->value[0] = 2;
                    paper->value[1] = 2;
                    paper->value[2] = 2;
                    sprintf(short_desc_buf, "Some Mail");
                    STRFREE(paper->short_descr);
                    paper->short_descr = STRALLOC(short_desc_buf);
                    sprintf(long_desc_buf, "Some Mail lies here.");
                    STRFREE(paper->description);
                    paper->description = STRALLOC(long_desc_buf);
                    sprintf(keyword_buf, "mail parchment paper %d",cnt);
                    STRFREE(paper->name);
                    paper->name = STRALLOC(keyword_buf);
send_to_char("The Postmaster hands you a piece of mail.\n\r",ch);
act(AT_BLUE,"The Postmaster hands $n a piece of mail.",ch,NULL,NULL,TO_ROOM);
obj_to_char(paper,ch);
mail->read = 1;
UNLINK(mail,first_mail,last_mail,next,prev);
DISPOSE(mail);
write_mail();
}

}

if ( !found )
send_to_char("The Postmaster tells you 'you have no mail waiting.'\n\r",ch);
else
write_mail();

return;
}

void do_read( CHAR_DATA *ch, char *argument )
{
OBJ_DATA *obj=NULL;
char arg[MAX_INPUT_LENGTH];

arg[0] = '\0';

one_argument( argument, arg );

if ( ms_find_obj(ch) )
return;

if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
{
send_to_char("You don't have that.\n\r",ch);
return;
}

if ( obj->pIndexData->vnum == OBJ_VNUM_NOTE )
{
send_to_char( obj->spare_desc, ch );
return;
}

send_to_char( "You can't seem to read that.\n\r",ch);
return;
}

void mail_notice( CHAR_DATA *ch )
{
MAIL_DATA *mail;
int cnt = 0;

for ( mail = first_mail; mail; mail = mail->next )
{
if( mail == NULL )
     break;

if( !str_cmp( mail->to, ch->name ) && ( mail->read == 0 ) )
     cnt++;
}

if ( cnt > 0 )
{
send_to_char("&BYOU HAVE MAIL\n\r",ch);
return;
}

return;
}

void mail_count(CHAR_DATA *ch)
{
  MAIL_DATA *mail;
  int cnt = 0;

  for ( mail = first_mail; mail; mail = mail->next )
  {
  if( mail == NULL )
	break;

  if( !str_cmp( mail->to, ch->name ) && ( mail->read == 0 ) )
          cnt++;
  }
  if ( cnt )
    ch_printf(ch, "You have %d mail messages waiting.\n\r", cnt);
  return;
}

void load_mail( void )
{
MAIL_DATA *mail=NULL;
FILE *fp;
bool fHeader = FALSE;

if( (fp=fopen( MAIL_FILE,"r" ) )==NULL )
{
bug("load_mail: CANNOT OPEN MAIL_FILE",0);
perror(MAIL_FILE);
return;
}


for ( ; ; )
{
char letter;
char *word;

letter = feof( fp ) ? '$' : fread_letter( fp );
if( letter == '*' )
{
fread_to_eol( fp );
continue;
}

if ( letter == '$' )
{
break;
}

if ( letter != '#' )
{
bug("Load_Mail: # not Found.",0);
break;
}

word = feof( fp ) ? "END" : fread_word( fp );
if ( !str_cmp(word, "START" ) )
{
fread_to_eol( fp );
fHeader = TRUE;
continue;
}
else if ( !str_cmp(word, "MAIL" ) )
{
mail=NULL;
CREATE( mail, MAIL_DATA,1);
load_letter( mail, fp );
LINK( mail, first_mail, last_mail, next, prev );
continue;
}
else if ( !str_cmp(word, "END" ) )
break;
else
{
bug("Load_Mail: Bad Section. (%s)",word);
break;
}

if ( !fHeader )
log_string("Load_Mail: #START MAIL not found!");

}
new_fclose(fp);
return;
}

void write_mail( void )
{
MAIL_DATA *mail;
FILE *fp;


if ( (fp=fopen( MAIL_FILE,"w" ) )==NULL)
{
bug("Cannot open MAIL_FILE!",0);
perror(MAIL_FILE);
return;
}

fprintf( fp,"#START MAIL\n\n");
for ( mail = first_mail; mail; mail = mail->next )
{
fprintf( fp,"#MAIL\n");
fprintf( fp,"Read	%d\n",  mail->read );
fprintf( fp,"Sender	%s~\n", mail->sender );
fprintf( fp,"Date	%s~\n", mail->date );
fprintf( fp,"To		%s~\n", mail->to );
fprintf( fp,"Subject	%s~\n", mail->subject );
fprintf( fp,"Text	%s~\n", mail->text );
fprintf( fp,"$$\n");
}

fprintf( fp,"#END MAIL" );
new_fclose(fp);
return;
}

void load_letter( MAIL_DATA *mail, FILE *fp )
{
    char *word=NULL;
    bool fMatch=FALSE;

    for( ; ; )
    {
	word = feof(fp) ? "$" : fread_word( fp );
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
  	KEY( "Date",	mail->date,	fread_string( fp ) );
	break;
	
	case 'R':
	KEY( "Read",	mail->read,	fread_number( fp ) );
	break;

	case 'S':
	KEY( "Sender",	mail->sender,	fread_string( fp ) );
	KEY( "Subject",	mail->subject,  fread_string( fp ) );
	break;

	case 'T':
	KEY( "To",	mail->to,	fread_string( fp ) );
	KEY( "Text",	mail->text,	fread_string( fp ) );
	break;
 
	}

	if ( !fMatch  )
	{
	  bug( "Load_Letter: no match for %s.", word );
	}
   }
}

bool forward_mail( MAIL_DATA *mail, char *fw_email )
{
char buf[MSL*3];
FILE *fp;

if ( !fw_email || !mail )
return FALSE;

sprintf(buf,"/bin/mail -s 'FW: AOTA MudMail from %s.' %s",capitalize(mail->sender),fw_email);
if ( (fp= popen( buf, "w" ))==NULL)
{
  bug("Forward Mail: Unable to open process pipe! (%s)",buf);
  return FALSE;
}
sprintf(buf,
"\n**************************************************************"
"\n*** THIS IS AN AUTO-GENERATED MAIL - DONOT REPLY TO IT!    ***"
"\n**************************************************************\n\n"
"The following Mail was forwarded to you from Age of the Ancients MUD.\n"
"Please report all abuse of this system to aota@ancients.org, and include\n"
"this mail attached.\n\n"
"Sender:  %s\n"
"To:      %s\n"
"Date:    %s\n"
"Subject: %s\n\n"
"%s\n\n"
"-------------------------------------------------------------\n"
"-=[ AOTA-Mail 1.2 ]=-\n"
"Copyright 2000, Wolfpaw Services Inc\n"
"Age of the Ancients MUD. Power Beyond your DREAMS.\n"
"http://www.ancients.org   telnet://mud.ancients.org:5000\n",mail->sender,mail->to,mail->date,
mail->subject,mail->text);
fprintf(fp,buf);
fprintf(fp,".");
fprintf(fp,".\n");
pclose(fp);
sprintf(buf,"MAIL: Forwarded Message from %s to %s to address %s.",mail->sender,mail->to,fw_email);
log_string_plus(buf,LOG_NORMAL,58);
return TRUE;
}

void do_forward( CHAR_DATA *ch, char *argument )
{
char arg[MSL];

argument=one_argument( argument, arg );

if ( IS_NPC(ch) )
return;

if ( arg[0] == '\0' )
{
  send_to_char("Syntax: forward <email address>\n\r",ch);
  send_to_char("&R**WARNING**&w&W: Abuse of this system will result in SITE BAN. It is Logged.\n\r",ch);
}

ch->pcdata->mail_forward = STRALLOC(arg);
send_to_char("Done.\n\r",ch);

return;
}


