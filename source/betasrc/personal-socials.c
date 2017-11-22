/*--------------------------------------------------------------------------*
 *                         ** WolfPaw 1.0 **                                *
 *--------------------------------------------------------------------------*
 *               WolfPaw 1.0 (c) 1997,1998 by Dale Corse                    *
 *--------------------------------------------------------------------------*
 *            The WolfPaw Coding Team is headed by: Greywolf                *
 *  With the Assitance from: Callinon, Dhamon, Sentra, Wyverns, Altrag      *
 *  Scryn, Thoric, Justice, Tricops and Mask.                               *
 *--------------------------------------------------------------------------*
 *			Personal Social Handling Module			    *
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

void load_char_social( FILE *fp, CHAR_DATA *ch );
void save_char_socials( FILE *fp, CHAR_DATA *ch );
void setup_char_socials(CHAR_DATA *ch);
SOCIALTYPE *personal_social_check(CHAR_DATA *ch, char *name );
bool social_syntax_check( CHAR_DATA *ch, char *txt, int type );
bool SOCIALTYPE_HAS_VICT(int type);
PSOC_DATA *get_char_social( CHAR_DATA *ch, char *name );
bool social_name_check( CHAR_DATA *ch, char *name );
void create_psocial( CHAR_DATA *ch, char *name );
void delete_psocial( CHAR_DATA *ch,PSOC_DATA *social );

#define SOCIAL_OK(social)	\
	(IS_SET((social)->flags, PSOC_CNOARG) && \
	 IS_SET((social)->flags, PSOC_ONOARG) && \
	 IS_SET((social)->flags, PSOC_CFOUND) && \
	 IS_SET((social)->flags, PSOC_OFOUND) && \
	 IS_SET((social)->flags, PSOC_VFOUND) && \
	 IS_SET((social)->flags, PSOC_CAUTO) && \
	 IS_SET((social)->flags, PSOC_OAUTO) ) 

#define MAX_PSOCIALS 10 /* Max PSocials Allowed per Player */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )                                    \
                                if ( !str_cmp( word, literal ) )        \
                                {                                       \
                                    field  = value;                     \
                                    fMatch = TRUE;                      \
                                    break;                              \
                               }

/*
 * User interface, sets everything up --GW 
 * Syntax:
 * psocial <view/create/set/delete> <name> <field> <string>
 */
void do_psocial( CHAR_DATA *ch, char *argument )
{
char arg1[MIL];
char arg2[MIL];
char arg3[MIL];
PSOC_DATA *social;

argument = one_argument( argument, arg1 ); /* Create/Set */
argument = one_argument( argument, arg2 ); /* Name */
argument = one_argument( argument, arg3 ); /* Field */
/* Left is raw string */
if ( arg1[0] == '\0' )
{
  send_to_char("Syntax: psocial <view/create/set/delete> <name> <field> <string>\n\r\n\r",ch);
  send_to_char("Field being one of:\n\r",ch);
  send_to_char("cnoarg onoarg cfound ofound vfound cauto oauto\n\r",ch);
  send_to_char("PLEASE READ HELP PSOCIAL BEFORE YOU USE THIS!\n\r",ch);
  send_to_char("Type 'psocials' for a list of the socials you have.\n\r",ch);
  return;
}

/* Make a new Social */
if ( !str_cmp( arg1, "create" ) )
{
  if ( arg2[0] == '\0' )
  {
  send_to_char("Syntax: psocial <view/create/set/delete> <name> <field> <string>\n\r\n\r",ch);
  send_to_char("Field being one of:\n\r",ch);
  send_to_char("cnoarg onoarg cfound ofound vfound cauto oauto\n\r",ch);
  send_to_char("PLEASE READ HELP PSOCIAL BEFORE YOU USE THIS!\n\r",ch);
  send_to_char("Type 'psocials' for a list of the socials you have.\n\r",ch);
  return;
  }

  if ( social_name_check(ch,arg2)==FALSE )
  {
    send_to_char("Unable to Create Social, read HELP PSOCIALS\n\r",ch);
    return;
  }

  if ( ch->pcdata->psocials == MAX_PSOCIALS )
  {
    ch_printf(ch,"Sorry, your only allowed %d Persoanl Socials!\n\r",MAX_PSOCIALS);
    return;
  }

  ch->pcdata->psocials++;
  create_psocial(ch,arg2);
  send_to_char("Social Created.\n\r",ch);
  return;
}

/* Remove a Social */
if ( !str_cmp( arg1, "delete" ) )
{
  if ( arg2[0] == '\0' )
  {
  send_to_char("Syntax: psocial <view/create/set/delete> <name> <field> <string>\n\r\n\r",ch);
  send_to_char("Field being one of:\n\r",ch);
  send_to_char("cnoarg onoarg cfound ofound vfound cauto oauto\n\r",ch);
  send_to_char("PLEASE READ HELP PSOCIAL BEFORE YOU USE THIS!\n\r",ch);
  send_to_char("Type 'psocials' for a list of the socials you have.\n\r",ch);
  return;
  }

   if ( !(social=get_char_social(ch,arg2)))
   {
     send_to_char("You have no Personal Social named that!\n\r",ch);
     return;
   }

   ch->pcdata->psocials--;
   delete_psocial(ch,social);
   send_to_char("Social Deleted.\n\r",ch);
   return;
}

/* Set stuff on a Social */
if ( !str_cmp( arg1, "set" ) )
{
  if ( arg2[0] == '\0' || arg3[0] == '\0' || !argument[0] || 
       isspace(argument[0]) )
  {
  send_to_char("Syntax: psocial <view/create/set/delete> <name> <field> <string>\n\r\n\r",ch);
  send_to_char("Field being one of:\n\r",ch);
  send_to_char("cnoarg onoarg cfound ofound vfound cauto oauto\n\r",ch);
  send_to_char("PLEASE READ HELP PSOCIAL BEFORE YOU USE THIS!\n\r",ch);
  send_to_char("Type 'psocials' for a list of the socials you have.\n\r",ch);
  return;
  }

   if ( !(social=get_char_social(ch,arg2)))
   {
     send_to_char("You have no Personal Social named that!\n\r",ch);
     return;
   }

   if ( !str_cmp( arg3, "cnoarg" ) )
   {
     if ( social_syntax_check(ch,argument,SOCTYPE_CNOARG)==FALSE ) 
       return;

     smash_tilde(argument);
     social->char_no_arg = STRALLOC(argument);    
     SET_BIT(social->flags, PSOC_CNOARG);
     send_to_char("Done.\n\r",ch);
     return;
   }
   if ( !str_cmp( arg3, "onoarg" ) )
   {
     if ( social_syntax_check(ch,argument,SOCTYPE_ONOARG)==FALSE ) 
       return;

     smash_tilde(argument);
     social->others_no_arg = STRALLOC(argument);    
     SET_BIT(social->flags, PSOC_ONOARG);
     send_to_char("Done.\n\r",ch);
     return;
   }
   if ( !str_cmp( arg3, "cfound" ) )
   {
     if ( social_syntax_check(ch,argument,SOCTYPE_CFOUND)==FALSE ) 
       return;

     smash_tilde(argument);
     social->char_found = STRALLOC(argument);    
     SET_BIT(social->flags, PSOC_CFOUND);
     send_to_char("Done.\n\r",ch);
     return;
   }
   if ( !str_cmp( arg3, "ofound" ) )
   {
     if ( social_syntax_check(ch,argument,SOCTYPE_OFOUND)==FALSE ) 
       return;

     smash_tilde(argument);
     social->others_found = STRALLOC(argument);    
     SET_BIT(social->flags, PSOC_OFOUND);
     send_to_char("Done.\n\r",ch);
     return;
   }
   if ( !str_cmp( arg3, "vfound" ) )
   {
     if ( social_syntax_check(ch,argument,SOCTYPE_VFOUND)==FALSE ) 
       return;

     smash_tilde(argument);
     social->vict_found = STRALLOC(argument);    
     SET_BIT(social->flags, PSOC_VFOUND);
     send_to_char("Done.\n\r",ch);
     return;
   }
   if ( !str_cmp( arg3, "cauto" ) )
   {
     if ( social_syntax_check(ch,argument,SOCTYPE_CAUTO)==FALSE ) 
       return;

     smash_tilde(argument);
     social->char_auto = STRALLOC(argument);    
     SET_BIT(social->flags, PSOC_CAUTO);
     send_to_char("Done.\n\r",ch);
     return;
   }
   if ( !str_cmp( arg3, "oauto" ) )
   {
     if ( social_syntax_check(ch,argument,SOCTYPE_OAUTO)==FALSE ) 
       return;

     smash_tilde(argument);
     social->others_auto = STRALLOC(argument);    
     SET_BIT(social->flags, PSOC_OAUTO);
     send_to_char("Done.\n\r",ch);
     return;
   }
}

if ( !str_cmp( arg1, "view" ) )
{
  if ( arg2[0] == '\0' )
  {
  send_to_char("Syntax: psocial <view/create/set/delete> <name> <field> <string>\n\r\n\r",ch);
  send_to_char("Field being one of:\n\r",ch);
  send_to_char("cnoarg onoarg cfound ofound vfound cauto oauto\n\r",ch);
  send_to_char("PLEASE READ HELP PSOCIAL BEFORE YOU USE THIS!\n\r",ch);
  send_to_char("Type 'psocials' for a list of the socials you have.\n\r",ch);
  return;
  }

   if ( !(social=get_char_social(ch,arg2)))
   {
     send_to_char("You have no Personal Social named that!\n\r",ch);
     return;
   }

  ch_printf(ch,"\n\rName  : %s\n\r",social->name );
  ch_printf(ch,"CNoArg:	%s\n\r",social->char_no_arg ? social->char_no_arg : "(None)" );
  ch_printf(ch,"ONoArg:	%s\n\r",social->others_no_arg ? social->others_no_arg : "(None)" ); 
  ch_printf(ch,"CFound:	%s\n\r",social->char_found ? social->char_found : "(None)" );
  ch_printf(ch,"OFound:	%s\n\r",social->others_found ? social->others_found : "(None)" );
  ch_printf(ch,"VFound:	%s\n\r",social->vict_found ? social->vict_found : "(None)" );
  ch_printf(ch,"CAuto :	%s\n\r",social->char_auto ? social->char_auto : "(None)");
  ch_printf(ch,"OAuto :	%s\n\r",social->others_auto ? social->others_auto : "(None)");
  if ( IS_IMP(ch) )
  ch_printf(ch,"Flags : %d\n\r",social->flags);

  return;
}

send_to_char("Syntax: psocial <view/create/delete/set> <name> <field> <string>\n\r\n\r",ch);
send_to_char("Field being one of:\n\r",ch);
send_to_char("cnoarg onoarg cfound ofound vfound cauto oauto\n\r",ch);
send_to_char("PLEASE READ HELP PSOCIAL BEFORE YOU USE THIS!\n\r",ch);
send_to_char("Type 'psocials' for a list of the socials you have.\n\r",ch);
return;
}

/*
 * Load a Social from Pfile --GW
 */
void load_char_social( FILE *fp, CHAR_DATA *ch )
{
PSOC_DATA *social=NULL;
char *word=NULL;
bool fMatch=FALSE;
char buf[MSL];

CREATE( social, PSOC_DATA, 1 );

for( ; ; )
{
   word = feof(fp) ? "End" : fread_word(fp);
   fMatch = FALSE;
   
   switch ( UPPER(word[0]))
   {

     case '*':
     fMatch=TRUE;
     fread_to_eol(fp);
     return;

	case 'C':
	KEY("CNoArg",	social->char_no_arg,	fread_string(fp) );
	KEY("CFound",	social->char_found,	fread_string(fp) );
	KEY("CAuto",	social->char_auto,	fread_string(fp) );     
     	break;

	case 'E':
	if ( !str_cmp( word, "End" ) )
	{
          LINK(social,ch->pcdata->personal_socials->first_social,ch->pcdata->personal_socials->last_social,next,prev);
	  return;	   
	}
	break;

     	case 'F':
	KEY("Flags",	social->flags,		fread_number(fp) );	
    	break;

     	case 'O':
	KEY("ONoArg",	social->others_no_arg,	fread_string(fp) );
	KEY("OFound",	social->others_found,	fread_string(fp) );
	KEY("OAuto",	social->others_auto,	fread_string(fp) );
     	break;

     	case 'N':
	KEY("Name",	social->name,		fread_string(fp) );
     	break;

     	case 'V':
	KEY("VFound",	social->vict_found,	fread_string(fp) );
     	break;
   }
  
   if ( !fMatch )
   {
     sprintf(buf,"Load_char_psocals: no match for %s",word);
     bug( buf, 0 );
   }
}

return;
}

/*
 * Write Personal Socials to pfile.. --GW
 */
void save_char_socials( FILE *fp, CHAR_DATA *ch )
{
PSOC_DATA *social, *next_social;

if ( !ch->pcdata->personal_socials )
setup_char_socials(ch);

for( social=ch->pcdata->personal_socials->first_social; social; social=next_social)
{
  next_social=social->next;

  fprintf(fp,"#PSOCIAL\n");
  fprintf(fp,"Name	%s~\n",social->name);
  fprintf(fp,"CNoArg    %s~\n",social->char_no_arg);
  fprintf(fp,"ONoArg	%s~\n",social->others_no_arg);
  fprintf(fp,"CFound	%s~\n",social->char_found);
  fprintf(fp,"OFound    %s~\n",social->others_found);
  fprintf(fp,"VFound	%s~\n",social->vict_found);
  fprintf(fp,"CAuto	%s~\n",social->char_auto);
  fprintf(fp,"OAuto	%s~\n",social->others_auto);
  fprintf(fp,"Flags	%d\n",social->flags);
  fprintf(fp,"End\n");
}

return;
}

/*
 * Setup a Characters PSocials --GW
 */
void setup_char_socials(CHAR_DATA *ch)
{
CREATE(ch->pcdata->personal_socials, PSOCIAL_DATA, 1 );
ch->pcdata->personal_socials->first_social = NULL;
ch->pcdata->personal_socials->last_social = NULL;
return;
}

void free_char_psocials(CHAR_DATA *ch)
{
PSOC_DATA *social, *next_social;

for(social=ch->pcdata->personal_socials->first_social;social;social=next_social)
{
  next_social=social->next;
  STRFREE(social->name);
 if ( social->char_no_arg )
  STRFREE(social->char_no_arg);
 if ( social->others_no_arg )
  STRFREE(social->others_no_arg);
 if ( social->char_found )
  STRFREE(social->char_found);
 if ( social->others_found )
  STRFREE(social->others_found);
 if ( social->vict_found )
  STRFREE(social->vict_found);
 if ( social->char_auto )
  STRFREE(social->char_auto);
 if ( social->others_auto )
  STRFREE(social->others_auto);

UNLINK(social,ch->pcdata->personal_socials->first_social,ch->pcdata->personal_socials->last_social,next,prev);
DISPOSE(social);
}

return;
}

/*
 * Internal Interface, checks for a personal Social, and if 
 * found, executes it, and returns TRUE, 
 * False if it is not Found. --GW
 */
SOCIALTYPE *personal_social_check(CHAR_DATA *ch, char *name )
{
PSOC_DATA *social, *next_social;
bool found = FALSE;
SOCIALTYPE *soc=NULL;

if ( IS_NPC(ch) )
return NULL;

for( social = ch->pcdata->personal_socials->first_social; social; social = next_social )
{
  next_social = social->next;

  if ( !social || !social->name )
   continue;

  if ( !str_cmp( social->name, name ) )
  {
    found = TRUE;
    break;
  }
}

if ( found )
{

if ( !SOCIAL_OK(social) )
return NULL;

CREATE(soc, SOCIALTYPE, 1 );
soc->name = 		STRALLOC(social->name);
soc->char_no_arg = 	STRALLOC(social->char_no_arg);
soc->others_no_arg = 	STRALLOC(social->others_no_arg);
soc->char_found = 	STRALLOC(social->char_found);
soc->others_found =     STRALLOC(social->others_found);
soc->vict_found = 	STRALLOC(social->vict_found);
soc->char_auto = 	STRALLOC(social->char_auto);
soc->others_auto = 	STRALLOC(social->others_auto);
return soc;
}

return NULL;
}

void free_socialtype_data( SOCIALTYPE *soc )
{
STRFREE(soc->name);
STRFREE(soc->char_no_arg);
STRFREE(soc->others_no_arg);
STRFREE(soc->char_found);
STRFREE(soc->others_found);
STRFREE(soc->vict_found);
STRFREE(soc->char_auto);
STRFREE(soc->others_auto);
DISPOSE(soc);
}

/*
 * Checks the Syntax of a social, makes sure it wont bomb the MUD --GW
 * TRUE is Syntax is OK
 * False if not!
 */
bool social_syntax_check( CHAR_DATA *ch, char *txt, int type )
{
char buf[MSL];
int str=0;
bool error=FALSE;

sprintf(buf,"%s",txt);

for( str=0; buf[str]; str++ )
{

if ( buf[str] != '$' || buf[str+1] == '\0' )
continue;

switch( buf[str+1] )
{

/* These are ok if the conditions are met */

/* name of Victim */
   case 'N':
   if ( !SOCIALTYPE_HAS_VICT(type) )
    error=TRUE;
   break;

/* Name of Character */
   case 'n':
   break;

/* he/she/it of char */
   case 'e':
   break;

/* he/She/it of Victim */
   case 'E':
   if ( !SOCIALTYPE_HAS_VICT(type) )
    error=TRUE;
   break;

/* Him/Her/It of Victim */
   case 'M':
   if ( !SOCIALTYPE_HAS_VICT(type) )
    error=TRUE;
   break;

/* Him/Her/It of Character */
   case 'm':
   break;

/* His/hers/its of Char */
   case 's':
   break;

/* His/Hers/Its of Victim */
   case 'S':
   if ( !SOCIALTYPE_HAS_VICT(type) )
    error=TRUE;
   break;

/* Reject anything else */
   default:
   error=TRUE;
   break;
}

}

if ( !error )
  return TRUE;

return FALSE;
}


/*
 * Returns TRUE if the Social type HAS a Vict.
 * False if it doesn't
 */
bool SOCIALTYPE_HAS_VICT(int type)
{
if ( type == SOCTYPE_CNOARG )
return FALSE;
if ( type == SOCTYPE_ONOARG )
return FALSE;
if ( type == SOCTYPE_VFOUND )
return FALSE;
if ( type == SOCTYPE_CAUTO )
return FALSE;
if ( type == SOCTYPE_OAUTO )
return FALSE;

return TRUE;
}

PSOC_DATA *get_char_social( CHAR_DATA *ch, char *name )
{
PSOC_DATA *social, *next_social;

for ( social=ch->pcdata->personal_socials->first_social; social; social=next_social )
{
  next_social=social->next;

if ( !str_cmp( social->name, name ) )
return social;
}

return NULL;
}

bool social_name_check( CHAR_DATA *ch, char *name )
{
CMDTYPE *command;
SOCIALTYPE *social;

if ( (command=find_command( name ))!=NULL)
return FALSE;

if ( (social=find_social(ch,name,FALSE))!=NULL)
return FALSE;

if ( (social=find_social(ch,name,TRUE))!=NULL)
return FALSE;

return TRUE;
}

void create_psocial( CHAR_DATA *ch, char *name )
{
PSOC_DATA *social=NULL;

CREATE(social, PSOC_DATA, 1 );
social->name = STRALLOC(name);
social->flags = 0;
LINK(social,ch->pcdata->personal_socials->first_social,ch->pcdata->personal_socials->last_social,next,prev);
return;
}

void delete_psocial( CHAR_DATA *ch,PSOC_DATA *social )
{
STRFREE(social->name);
STRFREE(social->char_no_arg);
STRFREE(social->others_no_arg);
STRFREE(social->char_found);
STRFREE(social->others_found);
STRFREE(social->vict_found);
STRFREE(social->char_auto);
STRFREE(social->others_auto);
UNLINK(social,ch->pcdata->personal_socials->first_social,ch->pcdata->personal_socials->last_social, next, prev );
DISPOSE(social);
return;
}

void do_psocials( CHAR_DATA *ch, char *argument )
{
PSOC_DATA *social, *next_social;
bool found=FALSE;
int cnt=0;
char buf[MSL];

for( social=ch->pcdata->personal_socials->first_social; social; social=next_social)
{
  next_social=social->next;

  if ( social )
  {
    cnt++;
    sprintf(buf,"%d) %s\n\r",cnt,social->name);
    send_to_char(buf,ch);
    found=TRUE;
  }
}

if ( !found )
{
send_to_char("You have no Personal Socials.\n\r",ch);
return;
}
else
{
sprintf(buf,"%d social%s found.\n\r",cnt,(cnt > 1) ? "s" : "");
send_to_char(buf,ch);
}

return;
}

