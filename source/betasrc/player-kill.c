/*--------------------------------------------------------------------------*
 *                         ** WolfPaw 1.0 **                                *
 *--------------------------------------------------------------------------*
 *               WolfPaw 1.0 (c) 1997,1998 by Dale Corse                    *
 *--------------------------------------------------------------------------*
 *            The WolfPaw Coding Team is headed by: Greywolf                *
 *  With the Assitance from: Callinon, Dhamon, Sentra, Wyverns, Altrag      *
 *  Scryn, Thoric, Justice, Tricops and Mask.                               *
 *--------------------------------------------------------------------------*
 *			New Player Killing Handler			    *
 *--------------------------------------------------------------------------*/
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

/*
 * Checks to see if the player can be attacked -- 
 * TRUE if yes
 * FALSE if no --GW
 */
bool check_pk_ok( CHAR_DATA *ch, CHAR_DATA *killer )
{

  /* Mobs are ok! */
  if ( IS_NPC(ch) || IS_NPC(killer) )
  return TRUE;

  /* Suicide ? =P */
  if ( killer == ch )
  return TRUE;

  /* Arena? */
  if ( in_arena(ch) && in_arena(killer) )
  return TRUE;

  /* Chaos on? */
  if ( chaos==1 )
  return TRUE;

  /* In Kombat? */ 
  if ( IS_SET(ch->pcdata->flags, PCFLAG_KOMBAT) )
  return TRUE;

  /* CTF */ 
  if ( IS_SET(ch->pcdata->flagstwo, MOREPC_CTF) )
  return TRUE;

  if ( IS_SET(killer->pcdata->flagstwo, MOREPC_SAFETY) && ch != killer )
  {
    send_to_char("Safety Flag &RDISABLED&W\n\r",killer);
    REMOVE_BIT(killer->pcdata->flagstwo, MOREPC_SAFETY);
  }

  /* Killers */
  if ( IS_SET(ch->act, PLR_KILLER) )
  return TRUE;

  /* Thieves */
  if ( IS_SET(ch->act, PLR_THIEF) )
  return TRUE;

  /* Bountied? */
  if ( IS_AFFECTED(ch, AFF_BOUNTY) )
  return TRUE;

  /* Assassin? */
  if ( IS_SET(ch->pcdata->flags, PCFLAG_ASSASSIN) )
  return TRUE;

  /* Non-PK Chars CANNOT Attack --GW */
  /* OOps .. guess they should be able to cast offensive spells on 
     themselves... */
  if ( !IS_SET(killer->pcdata->flags, PCFLAG_DEADLY ) && killer != ch )
  {
    send_to_char("Get a PK Flag if you want to kill someone!\n\r",ch);
    return FALSE;
  }

  /* If they disable safety -- There free --GW */
  if ( !IS_SET(ch->pcdata->flagstwo, MOREPC_SAFETY) )
    return TRUE;

  /* In a FREE-PK Zone? (Clubs, or Armageddon) */
  if ( ch->in_room && ch->in_room->area && 
	IS_SET(ch->in_room->area->flags,AFLAG_CLUB_HALL) )
     return TRUE;

  /* Got a PK Flag? */
  if ( !IS_SET(ch->pcdata->flags, PCFLAG_DEADLY) )
    return FALSE;

  if ( ch->level2 < 1 ) /* Newbies */
  {
    /* 5 Levels */
    if ( (ch->level+5) >= killer->level )
    return TRUE;
  }
  else if ( !IS_ADVANCED(ch) ) /* Duals */
  {
    /* 10 Levels */
    if ( ( (ch->level+10) >= killer->level ) ||
         ( (ch->level2+10) >= killer->level2 ) )    
    return TRUE;
  }
  else if ( IS_ADVANCED(ch) && !IS_ADV_DUAL(ch) ) /* Advanced */
  {
    if ( (ch->advlevel+10) >= killer->advlevel )
    return TRUE;
  }
  else if ( IS_ADV_DUAL(ch) ) /* Quad */
  {
    if ( (ch->advlevel2+10) >= killer->advlevel2 )
    return TRUE;
  }

send_to_char("You may not attack this player.\n\r",killer);
ch_printf(ch,"%s just tried to attack you!\n\r",capitalize(killer->name));
act(AT_BLOOD,"$n tries to attack $N, but $S gaurdian angel interferes!",killer,NULL,ch,TO_ROOM);
send_to_char("Safety Flag &RENABLED&W\n\r",killer);
SET_BIT(killer->pcdata->flagstwo, MOREPC_SAFETY);
return FALSE;
}

void do_safety( CHAR_DATA *ch, char *argument )
{

if (IS_NPC(ch) )
return;

if ( ch->fighting && ch->fighting->who )
{
  send_to_char("No! You cannot do this Fighting!\n\r",ch);
  return;
}

if ( IS_SET( ch->pcdata->flagstwo, MOREPC_SAFETY ) )
{
  send_to_char("Safety Flag &RDISABLED!&W\n\r",ch);
  REMOVE_BIT(ch->pcdata->flagstwo, MOREPC_SAFETY);
}
else
{
  send_to_char("Safety Flag &RENABLED!&W\n\r",ch);
  SET_BIT(ch->pcdata->flagstwo, MOREPC_SAFETY);
}

return;
}

/*
 * Setup for PK Tracking
 */
void clan_pkill_record_setup( void )
{
CLAN_DATA *clan, *next_clan, *cln, *cln_next, *check,*next_check;
PK_LIST *pklist=NULL,*pk=NULL,*next_pk=NULL;
char buf[MSL];
bool found=FALSE;
char name[MSL];

for ( cln = first_clan; cln; cln = cln_next )
{
   cln_next = cln->next;

if ( cln->clan_type == CLAN_GUILD || cln->clan_type == CLAN_ORDER )
  continue;

sprintf(log_buf,"PKBRD: Checking Records for: %s",cln->name);
log_string(log_buf);

if ( cln->pkdata )
  continue;

sprintf(buf,"Setting Up Pkdata for: %s",cln->fullname);
log_string(buf);

CREATE(cln->pkdata, PK_DATA, 1 );
cln->pkdata->first_clan = NULL;
cln->pkdata->last_clan = NULL;

for( clan=first_clan; clan; clan = next_clan )
{
   next_clan = clan->next;

   if ( clan->clan_type == CLAN_GUILD || clan->clan_type == CLAN_ORDER )
	continue;

   CREATE(pklist, PK_LIST, 1);

if (clan->whoname)
   pklist->name =   STRALLOC(clan->whoname);
else
   pklist->name =   STRALLOC("Unavailable");

   pklist->pkills = 0;
   LINK(pklist, cln->pkdata->first_clan, cln->pkdata->last_clan, next, prev );
}

}

/* Make sure the clan Exists.. */
/* loop thro all loaded clans ..*/
for( clan=first_clan; clan; clan=next_clan )
{
  next_clan = clan->next;
  
  /* Clans only */
  if ( clan->clan_type != CLAN_PLAIN )
  continue;

sprintf(log_buf,"PKBRD: Checking to make sure all recorded clans still exist: %s",clan->name);
log_string(log_buf);

  for ( pk=clan->pkdata->first_clan; pk; pk=next_pk )
  {
    next_pk = pk->next;

    /* set the name..*/
    sprintf(name,"%s",pk->name);
    found=FALSE;

      for( check=first_clan; check; check=next_check )
      {
        next_check=check->next;

        if ( check->clan_type != CLAN_PLAIN )
        continue;

        if ( !str_cmp(check->whoname, name ) )
        {
          found=TRUE;
          break;
        }
      }
      
      if ( !found )
      {
        sprintf(log_buf,"PKBRD: Outdated PK Data found (%s): %s",pk->name,clan->name);
        log_string(log_buf);
	STRFREE(pk->name);
        pk->pkills=0;
	UNLINK(pk,clan->pkdata->first_clan,clan->pkdata->last_clan,next,prev);
	DISPOSE(pk);
      }
  } /* End PK loop scan */
} /* end  */

/* Check the chart and make sure everyone is there for each club */
for( clan=first_clan; clan; clan=next_clan )
{
 next_clan=clan->next;
 
 if ( clan->clan_type != CLAN_PLAIN )
 continue;

 sprintf(name,"%s",clan->whoname);

 sprintf(log_buf,"PKBRD: Checking to make sure all cross referances exist: %s",clan->name);
 log_string(log_buf);

 /* Scan all the clans, and for each one make sure the 'name' is in thier PK Data */
 for ( cln=first_clan; cln; cln=cln_next )
 {
  cln_next=cln->next;

  if ( cln->clan_type != CLAN_PLAIN )
  continue;

  found=FALSE;
  for ( pk=cln->pkdata->first_clan; pk; pk=next_pk )
  {
    next_pk=pk->next;
    if ( !str_cmp(pk->name,name) )
    {
      found=TRUE;
      break;
    }
   }
   if ( !found )
   {
     sprintf(log_buf,"PKBRD: PK Entry Missing, Updating..(%s): %s",cln->name,clan->name);
     log_string(log_buf);

     CREATE(pklist,PK_LIST,1);
     pklist->name = STRALLOC(name);
     pklist->pkills = 0;
     LINK(pklist,cln->pkdata->first_clan,cln->pkdata->last_clan,next,prev);
   }
  }
}

return;
}

void fread_pkill_data( FILE *fp, CLAN_DATA *cln )
{
char *name;
int pks=0;
PK_LIST *clan=NULL;

   if ( cln->clan_type == CLAN_GUILD || cln->clan_type == CLAN_ORDER )
	return;

fread_to_eol(fp);
name=fread_string(fp);
pks=fread_number(fp);

if ( !cln->pkdata )
{
   CREATE(cln->pkdata, PK_DATA, 1 );
   cln->pkdata->first_clan = NULL;
   cln->pkdata->last_clan  = NULL;
}

      CREATE(clan, PK_LIST, 1);
      clan->name =	STRALLOC(name);
      clan->pkills = 	pks;
      LINK(clan,cln->pkdata->first_clan,cln->pkdata->last_clan,next,prev);     

return;
}

void fwrite_pkill_data( FILE *fp, CLAN_DATA *cln )
{
PK_LIST *clan, *next_clan;

   if ( cln->clan_type == CLAN_GUILD || cln->clan_type == CLAN_ORDER )
	return;

for( clan = cln->pkdata->first_clan; clan; clan = next_clan )
{
  next_clan = clan->next;

 if ( !str_cmp(clan->name,"Unavailable") )
   continue;

  fprintf(fp,"#PKDATA BEGIN\n");
  fprintf(fp,"%s~\n",clan->name);
  fprintf(fp,"%d\n",clan->pkills );

}

return;
}

void display_pkill_chart( CHAR_DATA *ch )
{
char topbuf[MSL*2], sidebuf[MSL*2];
CLAN_DATA *clan, *next_clan;
PK_LIST *pk, *pk_next;
long pks=0;

if ( IS_NPC(ch) )
return;

/* Setup the Top Row */
topbuf[0] = '\0';
for( clan = first_clan; clan; clan = next_clan )
{
  next_clan = clan->next;

   if ( clan->clan_type != CLAN_PLAIN )
	continue;

  if ( topbuf[0] == '\0' )
    sprintf(topbuf,"          |%-10s&w",clan->whoname );
  else
      sprintf(topbuf,"%s|%-10s&w",topbuf,clan->whoname );
}
sprintf(topbuf,"%s|%-10s",topbuf," Total ");
strcat(topbuf,"&w|\n\r");
send_to_char(topbuf,ch);

/* Write the lines Below */
sidebuf[0] = '\0';
for( clan = first_clan; clan; clan = next_clan )
{
   next_clan = clan->next;

   if ( clan->clan_type != CLAN_PLAIN )
	continue;

   pks=0;
   sprintf(sidebuf,"%-10s&w|",clan->whoname);

   for( pk = clan->pkdata->first_clan; pk; pk = pk_next )
   {
     pk_next = pk->next;
     sprintf(sidebuf,"%s%-10d|",sidebuf,pk->pkills);
     pks += pk->pkills;
   }

sprintf(sidebuf,"%s%-10ld|",sidebuf,pks);
strcat(sidebuf,"\n\r");
send_to_char(sidebuf,ch);
}

send_to_char("Done.\n\r",ch);
return;
}

void update_clan_pk_data( CLAN_DATA *killer_clan, CLAN_DATA *vict_clan )
{
PK_LIST *pk, *pk_next;

if ( killer_clan->clan_type == CLAN_GUILD || killer_clan->clan_type == CLAN_ORDER )
  return;

if ( vict_clan->clan_type == CLAN_GUILD || vict_clan->clan_type == CLAN_ORDER )
  return;

for( pk = killer_clan->pkdata->first_clan; pk; pk = pk_next )
{
  pk_next = pk->next;
  
  if ( !str_cmp(pk->name, vict_clan->whoname) )
  {
    pk->pkills++;
    break;
  }
}

return;
}

/* Clean the fuckin chart .. erg. --GW */
void pk_list_cleaner( void )
{
CLAN_DATA *clan, *next_clan, *clan2, *clan2_next;
PK_LIST *list, *next_list;
bool found;

for ( clan = first_clan; clan; clan = next_clan )
{
   next_clan = clan->next;

   if ( clan->clan_type != CLAN_PLAIN )
   continue;

   for ( list=clan->pkdata->first_clan; list; list=next_list )
   {
     next_list = list->next;

     found = FALSE;
  
     for ( clan2=first_clan; clan2; clan2=clan2_next )
     {
        clan2_next=clan2->next;

        if ( !str_cmp(list->name,clan->whoname) )
	{
           found=TRUE;
           break; 
	}
      }

	if ( !found )
	{
          STRFREE(list->name);
	  list->pkills=0;
	  UNLINK(list, clan->pkdata->first_clan,clan->pkdata->last_clan,next,prev);
	  log_string("Deleted Old Clan Pk Data.");
	  save_clan(clan);
	}
      }	  
}
log_string("Done.");
return;
}


