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
 *			Upstatted EQ Module				    *
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

#define UPSTAT_LOG_FILE		"../system/upstatlog"

OBJ_DATA *upstat_obj( OBJ_DATA *obj, bool mend, CHAR_DATA *ch );

void init_upstat( void )
{
FILE *fp;

fp=fopen(UPSTAT_LOG_FILE,"w");
new_fclose(fp);
log_string("[UPSTAT] Initialized.");
return;
}

void upstat_log( char *string )
{
FILE *fp;

if ( (fp=fopen(UPSTAT_LOG_FILE,"a"))==NULL)
{
bug("Failed to open UPstat Log!");
perror(UPSTAT_LOG_FILE);
return;
}

fprintf(fp,string);
fprintf(fp,"\n\r");
new_fclose(fp);
return;
}

/* decide weather or not TO upstat the object */
OBJ_DATA *obj_upstat_check( OBJ_DATA *obj )
{
int rand=0;

/* Armor and Weapons only */
if ( obj->item_type != ITEM_WEAPON && obj->item_type != ITEM_ARMOR )
return obj;

rand=number_range(1,100);

if ( rand > 30 && rand < 65 )
return upstat_obj(obj,FALSE,NULL);

return obj;
}

/* actually upstat the object */
OBJ_DATA *upstat_obj( OBJ_DATA *obj, bool mend, CHAR_DATA *ch )
{
char buf[MSL];
int dam_up=0,hit_up=0,ac_down=0,mac_down=0;

/* random stats.. */
hit_up += number_range(1,10);
dam_up += number_range(1,5);
ac_down += number_range(1,20);
mac_down += number_range(1,10);

if ( !mend )
{
sprintf(buf,"UPSTAT: [%d](%d) Upped %d Damage, %d Hitroll, %d AC %d MAC",obj->pIndexData->vnum,
	obj->pIndexData->area ? obj->pIndexData->area->zone->number : 1, dam_up,hit_up,ac_down,mac_down);
log_string_plus(buf,LOG_NORMAL,LEVEL_HYPERION);
}
else
{
sprintf(buf,"MEND: [%s][%d](%d) Upped %d Damage, %d Hitroll, %d AC %d MAC",capitalize(ch->name),obj->pIndexData->vnum,
	obj->pIndexData->area ? obj->pIndexData->area->zone->number : 1, dam_up,hit_up,ac_down,mac_down);
log_string_plus(buf,LOG_NORMAL,ch->level);
}

upstat_log(buf);

/* Reset Upped */
obj->dam_bonus = 0;
obj->hit_bonus = 0;
obj->ac_bonus = 0;
obj->mac_bonus = 0;

/* Upstat it */
obj->dam_bonus += hit_up;
obj->hit_bonus += dam_up;
obj->ac_bonus += ac_down;
obj->mac_bonus += mac_down;

SET_BIT(obj->magic_flags, ITEM_UPSTATED);

return obj;
}

/* Show the objects upstatted this boot */
void do_upstatlog( CHAR_DATA *ch, char *argument )
{
show_file(ch,UPSTAT_LOG_FILE);
return;
}

/* Player command to TRY and upstat things .. between us.. the closer to at limit
 * it is, or if it is OVER limit, its gone :) --GW
 */
void do_mend( CHAR_DATA *ch, char *argument )
{
int rand=0;
char arg[MIL];
LIMIT_DATA *limit;
OBJ_DATA *obj;
char logbuf[MSL];

one_argument(argument,arg);

if ( ch->race != RACE_HUMAN )
{
  send_to_char("Only Humans possess the means to use this.\n\r",ch);
  return;
}

if ( !(obj=get_obj_carry(ch,arg)) )
{
  send_to_char("You cannot find that.\n\r",ch);
  return;
}

separate_obj(obj);

if ( ms_find_obj(ch) )
        return;

if ( IS_SET(obj->magic_flags, ITEM_UPSTATED ) )
{
  send_to_char("That Item is already of a high quality!\n\r",ch);
  return;
}

/* Not limited - 25% chance of Upping, 15% chance of Destroying */ 
if (!IS_LIMITED(obj))
{
rand=number_range(1,100);
if ( IS_IMP(ch) || rand <= 25 ) /* Upstat */
{
  send_to_char("You gasp as you look down apon the MASTERPIECE you have Created!\n\r",ch);
  obj=upstat_obj(obj,TRUE,ch);
  return;
}
else if ( rand >= 85 ) /* Destroy */
{
  send_to_char("You totally bungle your attempt to mend, and the object is DESTROYED!\n\r",ch);
  sprintf(logbuf,"MEND: %s DESTROYED %s",capitalize(ch->name),obj->short_descr);
  log_string_plus(logbuf,LOG_NORMAL,ch->level);
  obj_from_char(obj);
  extract_obj(obj);
}
else /* Fail */
{
  send_to_char("You fail to mend.\n\r",ch);
  return;
}

}

/* Limited
 * Overlimit - 5% chance of Upstat, 75% chance of Destruction
 * At limit - 10% chance of Upstat, 25% chance of Destruction
 * Below Limit - 15% chance of Upstat, 15% chance of Destruction
 */
if ( IS_LIMITED(obj) )
{
  limit=limit_lookup(obj->pIndexData->area->zone->number,obj->pIndexData->vnum);
  /* Overlimit */
  if ( limit->loaded > limit->limit )
  {
    rand=number_range(1,100);

    if ( IS_IMP(ch) || rand <= 5 ) /* Upstat */
    {
     send_to_char("You gasp as you look down apon the MASTERPIECE you have Created!\n\r",ch);
     obj=upstat_obj(obj,TRUE,ch);
     return;
    }
    else if ( rand >= 25 ) /* Destroy */
    {
     send_to_char("You totally bungle your attempt to mend, and the object is DESTROYED!\n\r",ch);
     sprintf(logbuf,"MEND: %s DESTROYED %s",capitalize(ch->name),obj->short_descr);
     log_string_plus(logbuf,LOG_NORMAL,ch->level);
     obj_from_char(obj);
     extract_obj(obj);
    }
    else /* Fail */
    {
      send_to_char("You fail to mend.\n\r",ch);
      return;
    }
  }

  /* At Limit */
  if ( limit->loaded == limit->limit )
  {
    rand=number_range(1,100);

    if ( IS_IMP(ch) || rand <= 10 ) /* Upstat */
    {
     send_to_char("You gasp as you look down apon the MASTERPIECE you have Created!\n\r",ch);
     obj=upstat_obj(obj,TRUE,ch);
     return;
    }
    else if ( rand >= 75 ) /* Destroy */
    {
     send_to_char("You totally bungle your attempt to mend, and the object is DESTROYED!\n\r",ch);
     sprintf(logbuf,"MEND: %s DESTROYED %s",capitalize(ch->name),obj->short_descr);
     log_string_plus(logbuf,LOG_NORMAL,ch->level);
     obj_from_char(obj);
     extract_obj(obj);
    }
    else /* Fail */
    {
      send_to_char("You fail to mend.\n\r",ch);
      return;
    }
  }

  /* Below Limit */
  if ( limit->loaded < limit->limit )
  {
    rand=number_range(1,100);

    if ( IS_IMP(ch) || rand <= 15 ) /* Upstat */
    {
     send_to_char("You gasp as you look down apon the MASTERPIECE you have Created!\n\r",ch);
     obj=upstat_obj(obj,TRUE,ch);
     return;
    }
    else if ( rand >= 85 ) /* Destroy */
    {
     send_to_char("You totally bungle your attempt to mend, and the object is DESTROYED!\n\r",ch);
     sprintf(logbuf,"MEND: %s DESTROYED %s",capitalize(ch->name),obj->short_descr);
     log_string_plus(logbuf,LOG_NORMAL,ch->level);
     obj_from_char(obj);
     extract_obj(obj);
    }
    else /* Fail */
    {
      send_to_char("You fail to mend.\n\r",ch);
      return;
    }
  }
}

return;
}
