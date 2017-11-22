/*--------------------------------------------------------------------------*
 *                         ** WolfPaw 1.0 **                                *
 *--------------------------------------------------------------------------*
 *               WolfPaw 1.0 (c) 1997,1998 by Dale Corse                    *
 *--------------------------------------------------------------------------*
 *            The WolfPaw Coding Team is headed by: Greywolf                *
 *  With the Assitance from: Callinon, Dhamon, Sentra, Wyverns, Altrag      *
 *  Scryn, Thoric, Justice, Tricops and Mask.                               *
 *--------------------------------------------------------------------------*
 *		     Revised Clan Functions Module			    *
 *--------------------------------------------------------------------------*/
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "mud.h"

#define MAX_CLANS 	5
#define MAX_GUILDS	5
#define MAX_ORDERS	5

void pit_monster( OBJ_DATA *pit, CHAR_DATA *ch );


/*
 * Get pointer to clan structure from clan name.
 */
CLAN_DATA *get_clan( char *name )
{
    CLAN_DATA *clan;

    for ( clan = first_clan; clan; clan = clan->next )
       if ( !str_cmp( name, clan->name ) )
         return clan;
    return NULL;
}

void do_make( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    CLAN_DATA *clan;
    int level;
    ZONE_DATA *in_zone;

    if ( IS_NPC( ch ) || !ch->pcdata->clan )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    clan = ch->pcdata->clan;

    if ( str_cmp( ch->name, clan->leader )
    &&   str_cmp( ch->name, clan->deity )
    &&  (clan->clan_type != CLAN_GUILD )
    &&   ch->pcdata->clevel < 8 )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Make what?\n\r", ch );
        return;
    }

    pObjIndex = get_obj_index( clan->clanobj1,1 );
    level = 40;

    if ( !pObjIndex || !is_name( arg, pObjIndex->name ) )
    {
      pObjIndex = get_obj_index( clan->clanobj2,1 );
      level = 45;
    }
    if ( !pObjIndex || !is_name( arg, pObjIndex->name ) )
    {
      pObjIndex = get_obj_index( clan->clanobj3,1 );
      level = 50;
    }

    if ( !pObjIndex || !is_name( arg, pObjIndex->name ) )
    {
        send_to_char( "You don't know how to make that.\n\r", ch );
        return;
    }
    in_zone = find_zone(1);
    obj = create_object( pObjIndex, 1, in_zone );
    SET_BIT( obj->extra_flags, ITEM_CLANOBJECT );
    if ( CAN_WEAR(obj, ITEM_TAKE) )
      obj = obj_to_char( obj, ch );
    else
      obj = obj_to_room( obj, ch->in_room );
    act( AT_MAGIC, "$n makes $p!", ch, obj, NULL, TO_ROOM );
    act( AT_MAGIC, "You make $p!", ch, obj, NULL, TO_CHAR );
    return;
}

void do_clan( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char buf[MSL];
    CHAR_DATA *victim;
    CLAN_DATA *clan;
    int level;

    if ( IS_NPC( ch ) || !ch->pcdata->clan )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    clan = ch->pcdata->clan;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg[0] == '\0')
    {
        send_to_char( "clan <char> <Member|dismember|level #> \n\r", ch);
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "That player is not here.\n\r", ch);
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

 if ( !str_cmp( arg2, "dismember" ) )
{

 if ( !IS_IMMORTAL(ch) && victim->pcdata->clan != ch->pcdata->clan )
  {
   send_to_char( " That player isnt even in your clan!", ch);
    return;
 }

 if ( ch->pcdata->clevel <= 7 && ch != victim )
{
send_to_char( "Your to Lowly for that!", ch );
return;
}

 if ( !IS_IMMORTAL(ch) && ch->pcdata->clevel <= victim->pcdata->clevel && ch !=victim )
   {
   send_to_char( "You cant do that.", ch );
   return;
     }


    if ( victim->speaking & LANG_CLAN )
        victim->speaking = LANG_COMMON;
    REMOVE_BIT( victim->speaks, LANG_CLAN );
    --clan->members;

    victim->pcdata->clevel = 1;
    victim->pcdata->clan = NULL;
    STRFREE(victim->pcdata->clan_name);
    victim->pcdata->clan_name = STRALLOC( "" );
    act( AT_MAGIC, "You dismember $N from $t", ch, clan->fullname,victim,TO_CHAR);
    act( AT_MAGIC, "$n dismembers $N from $t", ch, clan->fullname, victim,TO_ROOM);
    act( AT_MAGIC, "$n has kicked you out of $t",ch,clan->fullname,victim,TO_VICT);
    victim->pcdata->start_room = 30601;

    sprintf(buf,"%s has removed %s from %s",QUICKLINK(ch->name),
        QUICKLINK(victim->name),clan->fullname );
    log_string(buf);
    save_char_obj( victim );    /* clan gets saved when pfile is saved */
    return;
}


  else if ( !str_cmp( arg2, "member" ) )
{

        if ( victim->level < 15 )
        {
            send_to_char( "This player is not worthy of joining yet.\n\r",ch );
            return;
        }
        if ( IS_SET(victim->act, PLR_OUTCAST) )
        {
        send_to_char("This character may not be membered, they have been outcast\n\r",ch);
        return;
        }

    if ( !IS_IMMORTAL(ch) && victim->pcdata->clan )
    {
        if ( victim->pcdata->clan == clan )
          send_to_char( "This player already belongs to your clan!\n\r",ch );
        else
          send_to_char( "This player already belongs to a clan!\n\r", ch);
        return;
      }

 if ( ch->pcdata->clevel <= 7 )
{
send_to_char( "Your to Lowly for that!", ch );
return;
}

    clan->members++;
    if ( clan->clan_type != CLAN_ORDER && clan->clan_type != CLAN_GUILD )
      SET_BIT(victim->speaks, LANG_CLAN);

    victim->pcdata->clan = clan;
    STRFREE(victim->pcdata->clan_name);
    victim->pcdata->clan_name = QUICKLINK( clan->name );
    act( AT_MAGIC, "You member $N into $t",ch,clan->fullname,victim,TO_CHAR );
    act( AT_MAGIC, "$n has membered $N into $t", ch,clan->fullname,victim, TO_ROOM);
    act( AT_MAGIC, "You are now a Member of $t", ch,clan->fullname,victim, TO_VICT);
    victim->pcdata->clevel = 1;
    sprintf(buf,"%s has Membered %s into %s",QUICKLINK(ch->name),
        QUICKLINK(victim->name),clan->fullname );
    log_string(buf);
    save_char_obj( victim );
    return;
}


 else  if ( !str_cmp( arg2, "level" ) );
{
bool raised;

 if ( ch->pcdata->clevel <= 8 )
{
send_to_char( "Your to Lowly for that!", ch );
return;
}

 if ( !IS_IMMORTAL(ch) && victim->pcdata->clan != ch->pcdata->clan )
  {
   send_to_char( " That player isnt even in your clan!", ch);
    return;
 }
    if ( ( level = atoi( arg2 ) ) < 1 || level > 10 )
    {
        send_to_char( "Level must be 1 to 10.\n\r", ch );
        return;
    }
   if ( ( level = atoi( arg2 ) ) > ch->pcdata->clevel )
  {
   send_to_char( "You cant give away clan ranks you dont have!", ch );
   return;
  }

   if ( level == victim->pcdata->clevel )
   {
        send_to_char("But that is not higher or lower!\n\r",ch);
        return;
   }

   if ( level > victim->pcdata->clevel )
        raised = TRUE;
      else
        raised = FALSE;

     if ( raised )
        act( AT_MAGIC, "You advance $N's clan rank.",ch,NULL,victim,TO_CHAR);
     else
        act( AT_MAGIC, "You decrease $N's clan rank.",ch,NULL,victim,TO_CHAR);

        sprintf(buf,"%s to clan rank %d by %s",QUICKLINK(victim->name),
                level,QUICKLINK(ch->name));
        log_string(buf);
        victim->pcdata->clevel = level;
 
   ch_printf(victim,"You clan rank has been %s to %d.\n\r",
        raised == TRUE ? "raised" : "lowered", victim->pcdata->clevel);

   save_char_obj( victim );     /* clan gets saved when pfile is saved */
   return;
}

send_to_char("Eh?\n\r",ch);
return;
}

void do_guild( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CLAN_DATA *guild;
    int iLevel;
    int level;

    if ( IS_NPC( ch ) || !ch->pcdata->guild )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    guild = ch->pcdata->guild;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg[0] == '\0')
    {
        send_to_char( "guild <char> <Member|dismember|level #> \n\r", ch);
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "That player is not here.\n\r", ch);
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "dismember" ) )
    {

    if ( !IS_IMMORTAL(ch) && victim->pcdata->guild != ch->pcdata->guild )
    {
      send_to_char( " That player isnt even in your guild!", ch);
      return;
    }

    if ( ch->pcdata->guildlevel <= 7 && ch != victim )
    {
     send_to_char( "Your to Lowly for that!", ch );
     return;
    }

    if ( !IS_IMMORTAL(ch) && ch->pcdata->guildlevel <= victim->pcdata->guildlevel && ch !=victim )
    {
      send_to_char( "You cant do that.", ch );
      return;
    }

    --guild->members;

    victim->pcdata->guildlevel = 1;
    victim->pcdata->guild = NULL;
    STRFREE(victim->pcdata->guild_name);
    victim->pcdata->guild_name = STRALLOC( "" );
    act( AT_MAGIC, "You dismember $N from $t", ch, guild->name,victim,TO_CHAR );
    act( AT_MAGIC, "$n dismembers $N from $t", ch, guild->name,victim,TO_ROOM );
    act( AT_MAGIC, "$n has DISMEMBERED YOU FROM $t", ch,guild->name,victim, TO_VICT);
    victim->pcdata->start_room = 30601;
    save_char_obj( victim );    /* clan gets saved when pfile is saved */
    return;
   }


  else if ( !str_cmp( arg2, "member" ) )
  {

        if ( victim->level < 50 && victim->level2 < 50 )
        {
            send_to_char( "This player is not worthy of joining yet.\n\r",ch );
            return;
        }

        if ( IS_SET(victim->act, PLR_OUTCAST) )
        {
        send_to_char("This character may not be membered, they have been outcast.\n\r",ch);
        return;
        }

        if ( !IS_IMMORTAL(ch) && victim->pcdata->guild )
        {
        if ( victim->pcdata->guild == guild )
          send_to_char( "This player already belongs to your guild!\n\r",ch );
        else
          send_to_char( "This player already belongs to a guild!\n\r",ch);
        return;
      }

 if ( ch->pcdata->guildlevel <= 7 )
 {
   send_to_char( "Your to Lowly for that!", ch );
   return;
 }

    guild->members++;

    victim->pcdata->guild = guild;
    STRFREE(victim->pcdata->guild_name);
    victim->pcdata->guild_name = QUICKLINK( guild->name );
    act( AT_MAGIC, "You member $N into $t",ch,guild->name,victim,TO_CHAR);
    act( AT_MAGIC, "$n has membered $N into $t", ch, guild->name,victim,TO_NOTVICT);
    act( AT_MAGIC, "You are now a Member of $t", ch, guild->name,victim,TO_VICT);
    victim->pcdata->guildlevel = 1;
    save_char_obj( victim );
    return;

}


 else  if ( !str_cmp( arg2, "level" ) );
{

 if ( ch->pcdata->guildlevel <= 8 )
 {
  send_to_char( "Your to Lowly for that!", ch );
   return;
 }

 if ( !IS_IMMORTAL(ch) && victim->pcdata->guild != ch->pcdata->guild )
  {
   send_to_char( " That player isnt even in your guild!", ch);
    return;
 }
    if ( ( level = atoi( arg2 ) ) < 1 || level > 10 )
    {
        send_to_char( "Level must be 1 to 10.\n\r", ch );
        return;
    }
   if ( ( level = atoi( arg2 ) ) > ch->pcdata->guildlevel )
  {
  send_to_char( "You cant give away guild ranks you dont have!", ch );
   return;
  }

     act( AT_MAGIC, "You advance $N's guild rank in $t", ch,guild->name,victim, TO_CHAR);
    for ( iLevel = victim->pcdata->guildlevel ; iLevel < level; iLevel++ )
    {
        if (level < 11 )
     send_to_char( "You guild rank advances!!\n\r", victim );
        victim->pcdata->guildlevel += 1;
    }

    save_char_obj( victim );    /* clan gets saved when pfile is saved */
   return;
}

send_to_char("Eh?\n\r",ch);
return;
}

void do_setclan( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CLAN_DATA *clan;

    if ( IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Usage: setclan <clan> <field> <deity|leader> <player>\n\r",ch);
        send_to_char( "\n\rField being one of:\n\r", ch );
        send_to_char( "Usage: setclan <clan> <field> <deity|leader> <player>\n\r",ch);
        send_to_char( "\n\rField being one of:\n\r", ch );
        send_to_char( " deity leader align storage recall\n\r", ch );
        send_to_char( " members board morgue whoname\n\r", ch );
        send_to_char( " obj1 obj2 obj3 guard1 guard2\n\r", ch );
        if ( get_trust( ch ) >= LEVEL_GOD )
        {
          send_to_char( " name filename motto desc fullname\n\r", ch );
          send_to_char( " favour strikes type associate\n\r", ch );
        }
        return;
    }

    clan = get_clan( arg1 );
    if ( !clan )
    {
        send_to_char( "No such clan.\n\r", ch );
        return;
    }
    if ( !strcmp( arg2, "deity" ) )
    {
        STRFREE( clan->deity );
        clan->deity = STRALLOC( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }

    if ( !strcmp( arg2, "associate" ) )
    {
        STRFREE( clan->associate );
        clan->associate = STRALLOC( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }

    if ( !strcmp( arg2, "leader" ) )
    {
        STRFREE( clan->leader );
        clan->leader = STRALLOC( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }

    if ( !strcmp( arg2, "board" ) )
    {
        clan->board = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }

    if ( !strcmp( arg2, "members" ) )
    {
        clan->members = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }

    if ( !strcmp( arg2, "recall" ) )
    {
        clan->recall = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }
    if ( !strcmp( arg2, "morgue" ) )
    {
        clan->morgue = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }

    if ( !strcmp( arg2, "storage" ) )
    {
        clan->storeroom = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }

    if ( !strcmp( arg2, "obj1" ) )
    {
        clan->clanobj1 = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }

    if ( !strcmp( arg2, "obj2" ) )
    {
        clan->clanobj2 = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }

    if ( !strcmp( arg2, "obj3" ) )
    {
        clan->clanobj3 = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }

    if ( !strcmp( arg2, "guard1" ) )
    {
        clan->guard1 = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }

    if ( !strcmp( arg2, "guard2" ) )
    {
        clan->guard2 = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }

    if ( get_trust( ch ) < LEVEL_GOD )
    {
        do_setclan( ch, "" );
        return;
    }

    if ( !strcmp( arg2, "align" ) )
    {
        clan->alignment = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }

    if ( !strcmp( arg2, "type" ) )
    {
        if ( !str_cmp( argument, "order" ) )
          clan->clan_type = CLAN_ORDER;
        else
        if ( !str_cmp( argument, "guild" ) )
          clan->clan_type = CLAN_GUILD;
        else
          clan->clan_type = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }

    if ( !strcmp( arg2, "class" ) )
    {
        clan->class = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }

    if ( !strcmp( arg2, "name" ) )
    {
        STRFREE( clan->name );
        clan->name = STRALLOC( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }

    if ( !strcmp( arg2, "fullname" ) )
    {
        STRFREE( clan->fullname );
        clan->fullname = STRALLOC( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }

    if ( !strcmp( arg2, "whoname" ) )
    {
     char *old_clan_name;
     PK_LIST *first_pk=NULL, *next_pk=NULL, *new_pk=NULL;
     CLAN_DATA *cln, *next_cln;
     bool found;

	old_clan_name = STRALLOC(clan->whoname);
        STRFREE( clan->whoname );
        clan->whoname = STRALLOC( argument );

     if ( clan->clan_type == CLAN_PLAIN )
     {
	/* Change the name if its chart */
	for ( first_pk=clan->pkdata->first_clan; first_pk; first_pk=next_pk )
        {
          next_pk=first_pk->next;

	  if ( !str_cmp(first_pk->name, old_clan_name) )
	  {
	     first_pk->name = STRALLOC( argument );
	     log_string("Pk_List: Name Updated");
	     break;
	  }
	 }

	/* Add the name to all other charts, or update as needed */
	for ( cln=first_clan; cln; cln=next_cln )
	{
	   next_cln = cln->next;

	   if ( cln==clan )
	    continue;

	   if ( cln->clan_type != CLAN_PLAIN )
		continue;

	    found = FALSE;

	   /* Try and update it .. */
	   for( first_pk=cln->pkdata->first_clan; first_pk; first_pk=next_pk )
	   {
	     next_pk = first_pk->next;
	     if ( !str_cmp( first_pk->name, old_clan_name ) )
	     {
		found = TRUE;
	     	first_pk->name = STRALLOC( argument );
		save_clan(cln);
	        log_string("Pk_List: Old Name found and Updated.");
		break;
	     }
	    }

	    if ( found )
		continue;

	    /* Can't Update - add it */
	    CREATE(new_pk,PK_LIST,1);
	    new_pk->name = STRALLOC(argument);
	    new_pk->pkills = 0;
	    LINK(new_pk,cln->pkdata->first_clan,cln->pkdata->last_clan,next,prev);
	    save_clan(cln);
	    log_string("Pk_List: Added new Entry");
	 }
	}
	     
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }

    if ( !strcmp( arg2, "filename" ) )
    {
        DISPOSE( clan->filename );
        clan->filename = str_dup( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        write_clan_list( );
        return;
    }

    if ( !strcmp( arg2, "motto" ) )
    {
        STRFREE( clan->motto );
        clan->motto = STRALLOC( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }

    if ( !strcmp( arg2, "desc" ) )
    {
        STRFREE( clan->description );
        clan->description = STRALLOC( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }

    do_setclan( ch, "" );
    return;
}

void do_showclan( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *clan;

    if ( IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Usage: showclan <clan>\n\r", ch );
        return;
    }

    clan = get_clan( argument );
    if ( !clan )
    {
        send_to_char( "No such clan.\n\r", ch );
        return;
    }

    ch_printf( ch, "Fullname: %s\n\r",clan->fullname);
    ch_printf( ch, "%s      : %s\n\rFilename: %s\n\rMotto   : %s\n\r",
                        clan->clan_type == CLAN_ORDER ? "Order" :
                       (clan->clan_type == CLAN_GUILD ? "Guild" : "Clan"),
                        clan->name,
                        clan->filename,
                        clan->motto );
    ch_printf( ch, "Description: %s\n\rDeity: %s\n\rLeader: %s\n\r",
                        clan->description,
                        clan->deity,
                        clan->leader );
    ch_printf( ch, "WhoName: %s\n\rAssociate: %s\n\r",
                        clan->whoname ? clan->whoname : "None",
                        clan->associate ? clan->associate : "None" );
    ch_printf(ch, "PKills: %6d    PDeaths: %6d\n\r",
                        clan->pkills,
                        clan->pdeaths );
    ch_printf( ch, "MKills: %6d    MDeaths: %6d\n\r",
                        clan->mkills,
                        clan->mdeaths );
    ch_printf( ch, "IllegalPK: %-6d Score: %d\n\r",
                        clan->illegal_pk,
                        clan->score );
    ch_printf( ch, "Type: %d    Favour: %6d  Strikes: %d\n\r",
                        clan->clan_type,
                       clan->favour,
                        clan->strikes );
    ch_printf( ch, "Members: %3d  Morgue %d   Alignment: %d  Class: %d\n\r",
                        clan->members,
                        clan->morgue,
                        clan->alignment,
                        clan->class );
    ch_printf( ch, "Board: %5d     Object1: %5d Object2: %5d Object3: %5d\n\r",
                        clan->board,
                        clan->clanobj1,
                        clan->clanobj2,
                        clan->clanobj3 );
    ch_printf( ch, "Recall: %5d  Storeroom: %5d  Guard1: %5d  Guard2: %5d\n\r",
                        clan->recall,
                        clan->storeroom,
                        clan->guard1,
                        clan->guard2 );
    return;
}

void do_makeclan( CHAR_DATA *ch, char *argument )
{
    char filename[256];
    CLAN_DATA *clan=NULL;
    bool found;

    if ( !argument || argument[0] == '\0' )
    {
        send_to_char( "Usage: makeclan <clan name>\n\r", ch );
        return;
    }

    found = FALSE;
    sprintf( filename, "%s%s", CLAN_DIR, strlower(argument) );

    CREATE( clan, CLAN_DATA, 1 );
    LINK( clan, first_clan, last_clan, next, prev );
    clan_pkill_record_setup( );
    clan->name          = STRALLOC( argument );
    clan->motto         = STRALLOC( "" );
    clan->description   = STRALLOC( "" );
    clan->deity         = STRALLOC( "" );
    clan->leader        = STRALLOC( "" );
    
}

void do_clans( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *clan;
    int count = 0;

    if( argument[0] == '\0' )
    {
    /* Switched deadly clan mobkills/mobdeaths to pkills -- Blodkai */
    set_char_color( AT_BLOOD, ch );
    send_to_char( "\n\rClan           Deity          Leader     Pkills\n\r",ch);
    for ( clan = first_clan; clan; clan = clan->next )
    {
        if ( clan->clan_type == CLAN_ORDER || clan->clan_type == CLAN_GUILD )
          continue;
        set_char_color( AT_NOTE, ch );
        ch_printf( ch, "%-14s %-14s %-14s", clan->name, clan->deity, clan->leader );
        set_char_color( AT_BLOOD, ch );
        ch_printf( ch, "   %7d\n\r", clan->pkills );
        count++;
    }
        send_to_char( "\n\r&rFor more information type &R'&rclans <clan>&R'&w\n\r",ch);

    if ( !count )
    {
        set_char_color( AT_BLOOD, ch);
        send_to_char( "There are no clans currently formed.\n\r", ch );
        return;
    }
    }
    else
    {
        clan = get_clan( argument );
        if( !clan )
        {
          do_clans( ch, "" );
          return;
        }
                ch_printf( ch, "&rClan     : %s\n\r", clan->fullname ?
                clan->fullname : clan->whoname );
                ch_printf( ch, "&rDeity    : &c%s\n\r", clan->deity );
                ch_printf( ch, "&rLeader   : &c%s\n\r", clan->leader );
                ch_printf( ch, "&rMembers  : &c%d\n\r", clan->members );
                ch_printf( ch, "&rAssociate: &c%s\n\r", clan->associate ? clan->associate : "&cNone" );
        }
  return;
}

void do_orders( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *order;
    int count = 0;

   /* Added displaying of mkills and mdeaths    - Brittany */
    set_char_color( AT_NOTE, ch );
    send_to_char( "Order            Deity          Leader        Mkills    Mdeaths\n\r",ch);
    for ( order = first_clan; order; order = order->next )
        if ( order->clan_type == CLAN_ORDER )
        {
            ch_printf( ch, "%-16s %-14s %-14s %5d       %5d\n\r",order->name,
                order->deity, order->leader, order->mkills, order->mdeaths);
            count++;
        }

    if ( !count )
    {
        send_to_char( "There are no Orders currently formed.\n\r", ch );
        return;
    }
}

void do_guilds( CHAR_DATA *ch, char *argument)
{
    CLAN_DATA *guild;
    int count = 0;

    /* Added guild mobkills/mobdeaths -- Blodkai */
    set_char_color( AT_NOTE, ch );
    send_to_char( "\n\rGuild                  Leader             Mkills      Mdeaths\n\r",ch);
    for ( guild = first_clan; guild; guild = guild->next )
        if ( guild->clan_type == CLAN_GUILD )
        {
            ++count;
            set_char_color( AT_YELLOW, ch );
            ch_printf( ch, "%-20s   %-14s     %-6d       %6d\n\r",guild->name,guild->leader,guild->mkills,guild->mdeaths);
        }

    set_char_color( AT_NOTE, ch );
    if ( !count )
        send_to_char( "There are no Guilds currently formed.\n\r", ch );
    else
        ch_printf( ch, "%d guilds found.\n\r", count );
}

void do_cdonate( CHAR_DATA *ch, char *argument)
{
   OBJ_DATA *pit;
   OBJ_DATA *obj;
   ROOM_INDEX_DATA *original;
   char arg[MAX_INPUT_LENGTH];
   CLAN_DATA *clan;
   bool NO_PIT=FALSE;

   argument = one_argument(argument, arg);

   if (ch == NULL )
        return;

   if (ch->pcdata->clan == NULL)
        return;

   if (arg[0] == '\0' )
   {
      send_to_char("Donate what to your clan?\n\r",ch);
      return;
   }

   original = ch->in_room;
   if (ch->position == POS_FIGHTING)
   {
      send_to_char(" You're fighting!\n\r",ch);
      return;
   }

   if ( IS_SET(ch->pcdata->flagstwo, MOREPC_HOARDER) )
   {
     send_to_char("No.\n\r",ch);
     return;
   }

   if ( (obj = get_obj_carry (ch, arg)) == NULL)
   {
      send_to_char("You do not have that!\n\r",ch);
      return;
   }
   else
   {
      if (!can_drop_obj(ch, obj) && ch->level < 51)
      {
         send_to_char("Eeek! You can't let go of it!\n\r",ch);
         return;
      }
        if ( !obj )
        return;

      if ((obj->item_type == ITEM_CORPSE_NPC) ||
         (obj->item_type == ITEM_CORPSE_PC))
      {
         send_to_char("You cannot donate that!\n\r",ch);
         return;
      }
      if (obj->timer > 0)
      {
         send_to_char("You cannot donate that.\n\r",ch);
         return;
      }

      if ( IS_SET( obj->second_flags, ITEM2_LIMITED ) )
      {
        separate_obj(obj);
        act(AT_MAGIC,"$p is destroyed as you attempt to donate it.",ch,obj,NULL,TO_CHAR);
        act(AT_MAGIC,"$p is destroyed as $n attempts to donate it.",ch,obj,NULL,TO_ROOM);
        extract_obj(obj);
        return;
      }

      if ( obj->item_type == ITEM_CONTAINER )
      {
        send_to_char("Containers cannot be donated.\n\r",ch);
        return;
      }

      if (ch->in_room != get_room_index(ROOM_VNUM_ALTAR,1))
         act(AT_PLAIN,"$n donates $p to thier clan.",ch,obj,NULL,TO_ROOM);
      act(AT_PLAIN,"You donate $p to your clan.",ch,obj,NULL,TO_CHAR);

         separate_obj(obj);
         char_from_room(ch);
         char_to_room(ch,get_room_index(ch->pcdata->clan->storeroom,1));
         if ( ( pit = get_obj_here( ch, "donation" ) ) == NULL )  
	   NO_PIT = TRUE;
         obj_from_char(obj);
	 if ( !NO_PIT )
	 {
           obj_to_obj(obj, pit );
	   pit_monster(pit,ch);
	 }
	 else
  	   obj_to_room( obj, ch->in_room );

    /* Clan storeroom checks */
    if ( IS_SET(ch->in_room->room_flags, ROOM_CLANSTOREROOM) )
        for ( clan = first_clan; clan; clan = clan->next )
          if ( clan->storeroom == ch->in_room->vnum )
            save_clan_storeroom(ch, clan);

    /* Save-Rooms --GW */
    if ( IS_SET(ch->in_room->room_flags, ROOM_SAVE) )
    save_save_room(ch);

         char_from_room(ch);
         char_to_room(ch,original);
         return;
     }
}

/*
 * Send the list of prices for clan/guild stones to a character --GW
 */
void clan_buy_list( CHAR_DATA *ch )
{
int clans=0,guilds=0,orders=0;
CLAN_DATA *temp, *temp_next;

for ( temp = first_clan; temp; temp = temp_next )
{
temp_next = temp->next;

if ( temp->clan_type == CLAN_ORDER )
  orders++;
else if ( temp->clan_type == CLAN_GUILD )
  guilds++;
else
  clans++;
}

send_to_char("\n\r",ch);
send_to_char(" *-*-* Age of the Ancients Clan/Guild Stone Price Lising *-*-*\n\r",ch);
send_to_char("\n\r",ch);
send_to_char(" Type                Price     Petitioners Signers Required\n\r",ch);
send_to_char(" ----------------------------------------------------------\n\r",ch);
send_to_char(" Guild Stone      400,000,000       25 Signitures\n\r",ch);
send_to_char(" Order Stone      750,000,000       35 Signitures\n\r",ch);
send_to_char(" PK Clan Stone    900,000,000       50 Signitures\n\r",ch);   
send_to_char("\n\r",ch);
ch_printf(ch, " Availablity:  Guilds [%d/%d]  Orders [%d/%d]  Clans [%d/%d]\n\r",
		guilds, MAX_GUILDS, orders, MAX_ORDERS, clans, MAX_CLANS ); 
send_to_char("\n\r",ch);
send_to_char("Note: Fees are 10,000,000 per month.\n\r",ch);
return;
}

void clan_buy( CHAR_DATA *ch, char *argument )
{
char arg[MSL];

one_argument( argument, arg );

if ( !str_prefix( arg, "guild stone" ) )
{
send_to_char("Not Currently Available\n\r",ch);
return;
}
if ( !str_prefix( arg, "pk clan stone" ) )
{
send_to_char("Not Currently Available\n\r",ch);
return;
}
if ( !str_prefix( arg, "order stone" ) )
{
send_to_char("Not Currently Available\n\r",ch);
return;
}

send_to_char("Buy What?\n\r",ch);
return;
}

/*
 * Donation Pit Monster - eats limited items if theres more then 2 --GW
 */
void pit_monster( OBJ_DATA *pit, CHAR_DATA *ch )
{
OBJ_DATA *obj, *next_obj;
CLAN_DATA *clan;
LIMIT_DATA *limit;

/* No Pit - try and find it --GW */
if ( !pit && ch )
{
 if ( ( pit = get_obj_here( ch, "donation" ) ) == NULL )  
 {
   bug("Pit_Monster: Null Pit! (V:%d)",ch->in_room->vnum);
   return;
 }
}

act(AT_PLAIN,"&RThe Pit Monster looks in the pit.. licking his lips.&w&W",ch,NULL,NULL,TO_ROOM);
act(AT_PLAIN,"&RThe Pit Monster looks in the pit.. licking his lips.&w&W",ch,NULL,NULL,TO_CHAR);

for ( obj=pit->first_content; obj; obj=next_obj )
{
  next_obj = obj->next_content;
  if ( IS_LIMITED(obj) )
  {
   int delete_cnt=0;
   OBJ_DATA *delete= NULL;

    /* over limit eq */
    limit=limit_lookup(obj->pIndexData->area->zone->number,obj->pIndexData->vnum);
    if ( limit->loaded > limit->limit )
    {
         act(AT_PLAIN,"&RThe Pit Monster eats $p! &C*Burp*&w&W",ch,obj,NULL,TO_ROOM);
         act(AT_PLAIN,"&RThe Pit Monster eats $p! &C*Burp*&w&W",ch,obj,NULL,TO_CHAR);
         separate_obj(obj);
         obj_from_obj(obj);
         extract_obj(obj);
	 continue;
    }

    /* Scan the pit for the next occurance(s) */
    if ( obj->count > 2 )
      obj->count = 2;

    delete_cnt = obj->count;
    for ( delete=pit->first_content; delete; delete = delete->next_content )
    {
      if ( ( delete->pIndexData->vnum == obj->pIndexData->vnum ) &&
	delete != obj )
      {
       delete_cnt++;
       if (delete_cnt > 2 )
       {
         act(AT_PLAIN,"&RThe Pit Monster eats $p! &C*Burp*&w&W",ch,delete,NULL,TO_ROOM);
         act(AT_PLAIN,"&RThe Pit Monster eats $p! &C*Burp*&w&W",ch,delete,NULL,TO_CHAR);
         separate_obj(delete);
         obj_from_obj(delete);
         extract_obj(delete);
       }
      }
    }
   }
}

    /* Save our changes --GW */
    if ( IS_SET(ch->in_room->room_flags, ROOM_CLANSTOREROOM) )
        for ( clan = first_clan; clan; clan = clan->next )
          if ( clan->storeroom == ch->in_room->vnum )
            save_clan_storeroom(ch, clan);

return;
}

