/*--------------------------------------------------------------------------*
 *                         ** WolfPaw 1.0 **                                *
 *--------------------------------------------------------------------------*
 *               WolfPaw 1.0 (c) 1997,1998 by Dale Corse                    *
 *--------------------------------------------------------------------------*
 *            The WolfPaw Coding Team is headed by: Greywolf                *
 *  With the Assitance from: Callinon, Dhamon, Sentra, Wyverns, Altrag      *
 *  Scryn, Thoric, Justice, Tricops and Mask.                               *
 *--------------------------------------------------------------------------*
 *		Online Race Editor and Creation Module			    *
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

#define MAX_LANG 32
#define KEY( literal, field, value )                                    \
                                if ( !str_cmp( word, literal ) )        \
                                {                                       \
                                    field  = value;                     \
                                    fMatch = TRUE;                      \
                                    break;                              \
                                }

void load_race	     args( ( struct race_type *race, FILE *fp ) );
bool load_race_file  args( ( char *filename ) );
struct	race_type *   race_table	[MAX_RACE];
void write_races     args( ( void ) );
char *getlangname    args( ( int value ) );

int race_number;

void do_setrace( CHAR_DATA *ch, char *argument )
{
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
char arg3[MAX_INPUT_LENGTH];
int value;
//extern int const lang_array    [];
//extern char * const lang_names  [];
struct race_type *race=NULL;
int cnt = 0;
bool found = FALSE;

argument = one_argument( argument, arg1 );
argument = one_argument( argument, arg2 );
argument = one_argument( argument, arg3 );
record_call("<do_setrace>");

if ( str_cmp( ch->name, "greywolf" ) )
{
send_to_char("DONOT USE THIS YET!! --GW\n\r",ch);
return;
}

if( !str_cmp( arg2,"writeall") )
{
  write_races();
  send_to_char("Done\n\r",ch);
  return;
}


if ( arg1[0] == '0' || arg2[0] == '\0' || arg3[0] == '\0' )
{
send_to_char("Syntax: setrace <race name> <field> <value>\n\r\n\r",ch);

send_to_char("Fields:\n\r",ch);
send_to_char("affected lang\n\r\n\r",ch);
return;
}

/*find the race */
for ( cnt = 0; cnt < MAX_RACE && race_table[cnt]; cnt++ )
{
if( !str_cmp( race_table[cnt]->race_name, arg1 ) )
{
found = TRUE;
race = race_table[cnt];
}
}

if ( !found )
{
send_to_char("Race not found.\n\r",ch);
return;
}

if( !str_cmp( arg2, "affected" ) )
{
           value = get_aflag( arg3 );
           if ( value < 0 || value > 31 )
             ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
           else
           xTOGGLE_BIT( race->affected, 1 << value );
	   write_races( );
	   send_to_char("Done.\n\r",ch);
	   return;
}

if( !str_cmp( arg2, "lang" ))
{

            value = get_langflag( arg3 );
            if ( value == LANG_UNKNOWN )
	    {
                ch_printf( ch, "Unknown language: %s\n\r", arg3 );
		return;
	    }
	    race->language = value;
  	    write_races( );
	    send_to_char( "Done.\n\r",ch);
	    return;
}

return;
}

void do_makerace( CHAR_DATA *ch, char *argument )
{
struct race_type *race=NULL;
char arg[MAX_INPUT_LENGTH];

one_argument( argument, arg );
record_call("<do_makerace>");

if ( arg[0] == '\0' )
{
send_to_char("Syntax: makerace <racename>\n\r",ch);
send_to_char("Note: Please use this only AFTER you have hard coded the race\n\r",ch);
return;
}

if ( strlen(arg) > 16 )
{
send_to_char("Name too long, 16 characters maximum.\n\r",ch);
return;
}

CREATE( race, struct race_type, 1 );

race->race_name		= STRALLOC( arg );
xCLEAR_BITS(race->affected);
race->str_plus		= 0;
race->dex_plus		= 0;
race->wis_plus		= 0;
race->int_plus		= 0;
race->con_plus		= 0;
race->cha_plus		= 0;
race->lck_plus		= 0;
race->hit		= 0;
race->mana		= 0;
race->resist		= 0;
race->suscept		= 0;
race->class_restriction	= 0;
race->language		= LANG_COMMON;
send_to_char("Done.",ch);
return;
}

void do_showrace( CHAR_DATA *ch, char *argument )
{
struct race_type *race;
char arg[MAX_INPUT_LENGTH];
bool found;
int cnt;

one_argument( argument, arg );
record_call("<do_showrace>");

if( arg[0] == '\0' )
{
send_to_char("Syntax: showrace <race name>\n\r",ch);
return;
}

found = FALSE;
for ( cnt = 0; cnt < MAX_RACE && race_table[cnt]; cnt++ )
{
if( !str_cmp( race_table[cnt]->race_name, arg ) )
{
found = TRUE;
race = race_table[cnt];
ch_printf( ch, "Race Number [%d]\n\r",cnt);
ch_printf( ch, "Name: %s	Language: %s\n",capitalize(race->race_name),getlangname(race->language));
ch_printf( ch, "Affected: %s\n",affect_bit_name(&race->affected));
ch_printf( ch, "Bonuses:\n Str %d Dex %d Wis %d Int %d Con %d Cha %d Lck %d Hit %d Mana %d\n",
		race->str_plus,race->dex_plus,race->wis_plus,race->int_plus,
		race->con_plus,race->cha_plus,race->lck_plus,race->hit,race->mana);
ch_printf( ch, "Suscept: %d\n",race->suscept);
ch_printf( ch, "Resist:  %d\n",race->resist);
ch_printf( ch, "Class Restriction: %d\n",race->class_restriction);

send_to_char("\n\r*** To Lookup RACE SKILLS use SLOOKUP, as usual!\n\r",ch);
}
}

if ( !found )
send_to_char( "Race not found.\n\r",ch);

return;
}

void load_races( void )
{
    FILE *fpList;
    char *filename;
    char racelist[256];
    char buf[MAX_STRING_LENGTH];
record_call("<load_races>");

    sprintf( racelist, "%s%s", RACE_DIR, RACE_LIST );
    if ( ( fpList = fopen( racelist, "r" ) ) == NULL )
    {
        perror( racelist );
        exit( 1 );
    }

    race_number = -1;

    for ( ; ; )
    {
        filename = feof( fpList ) ? "$" : fread_word( fpList );
        if ( filename[0] == '$' )
          break;

	race_number++;
        if ( !load_race_file( filename ) )
        {
          sprintf( buf, "Cannot load race file: %s", filename );
          bug( buf, 0 );
        }
    }
    new_fclose( fpList );
    return;
}

bool load_race_file( char *filename )
{
struct race_type *race=NULL;
FILE *fp;

record_call("<load_race_file>");
 
if( (fp=fopen( filename,"r" ) )==NULL )
{
bug("load_race_file: Cannot Open %s",filename);
perror(filename);
return FALSE;
}

CREATE( race, struct race_type ,1);
//fpArea = fp;
//strcpy(strArea, filename);
load_race( race, fp );
race_table[race_number] = race;
new_fclose(fp);
//fpArea = NULL;
//strcpy(strArea, "$");
return TRUE;
}

void write_races( void )
{
FILE *Wfp;
char racefile[MSL];
int cnt,x,y,cl;
record_call("<write_races>");

for( cnt = 0; cnt < MAX_RACE && race_table[cnt]; cnt++ )
{

sprintf(racefile,"%s%s.race",RACE_DIR,capitalize(race_table[cnt]->race_name));
if ( ( Wfp=fopen( racefile, "w" ) )==NULL )
{
bug("Cannot Open %s",racefile);
perror(racefile);
return;
}
fprintf( Wfp, "Name		%s~\n",race_table[cnt]->race_name);
fprintf( Wfp, "Affected		%s\n",print_bitvector(&race_table[cnt]->affected));
fprintf( Wfp, "StrPlus		%d\n",race_table[cnt]->str_plus);
fprintf( Wfp, "DexPlus		%d\n",race_table[cnt]->dex_plus);
fprintf( Wfp, "WisPlus		%d\n",race_table[cnt]->wis_plus);
fprintf( Wfp, "IntPlus		%d\n",race_table[cnt]->int_plus);
fprintf( Wfp, "ConPlus		%d\n",race_table[cnt]->con_plus);
fprintf( Wfp, "ChaPlus		%d\n",race_table[cnt]->cha_plus);
fprintf( Wfp, "LckPlus		%d\n",race_table[cnt]->lck_plus);
fprintf( Wfp, "Hit		%d\n",race_table[cnt]->hit);
fprintf( Wfp, "Mana		%d\n",race_table[cnt]->mana);
fprintf( Wfp, "Resist		%d\n",race_table[cnt]->resist);
fprintf( Wfp, "Suscept		%d\n",race_table[cnt]->suscept);
fprintf( Wfp, "Restriction	%d\n",race_table[cnt]->class_restriction);
fprintf( Wfp, "Lang		%d\n",race_table[cnt]->language);
cl = cnt;
for ( x = 0; x < top_sn; x++ )
{
        if ( !skill_table[x]->name || skill_table[x]->name[0] == '\0' )
           break;
	if ( skill_table[x]->race_skill_level[cl] < 1 )
	   continue; 
        if ( (y=skill_table[x]->race_skill_level[cl]) < LEVEL_IMMORTAL )
          fprintf( Wfp, "Skill '%s' %d %d\n",
                skill_table[x]->name,y,skill_table[x]->race_skill_adept[cl] );
}
fprintf( Wfp, "$ $\n");
new_fclose(Wfp);
}
return;
}

void load_race( struct race_type *race, FILE *fp )
{
    char *word=NULL;
    bool fMatch=FALSE;
    int cl;
    char buf[MSL];

    cl = race_number;
   record_call("<load_race>");

    for( ; ; )
    {
        word = feof( fp ) ? "$$" : fread_word( fp );
        fMatch = FALSE;

        switch ( UPPER(word[0]) )
        {
        case '*':
                fMatch = TRUE;
                fread_to_eol( fp );
                break;

        case '$':
                fMatch = TRUE;
		fread_to_eol(fp);
                return;

        case 'A':
	KEY( "Affected",	race->affected,		fread_bitvector( fp ) );
        break;

	case 'C':
	KEY( "ConPlus",		race->con_plus,		fread_number( fp ) );
	KEY( "ChaPlus",		race->cha_plus,		fread_number( fp ) );
	break;

	case 'D':
	KEY( "DexPlus",		race->dex_plus,		fread_number( fp ) );
	break;

	case 'H':
	KEY( "Hit",		race->hit,		fread_number( fp ) );
	break;

	case 'I':
	KEY( "IntPlus",		race->int_plus,		fread_number( fp ) );
	break;

	case 'L':
	KEY( "LckPlus",		race->lck_plus,		fread_number( fp ) );
	KEY( "Lang",		race->language,		fread_number( fp ) );
	break;

	case 'M':
	KEY( "Mana",		race->mana,		fread_number( fp ) );
	break;

	case 'N':
	KEY( "Name",		race->race_name,	fread_string( fp ) );
	KEY( "Number",		race->number,		fread_number( fp ) );
	break;

	case 'R':
	KEY( "Resist",		race->resist,		fread_number( fp ) );
	KEY( "Restriction",	race->class_restriction,fread_number( fp ) );
	break;

	case 'S':
            if ( !str_cmp( word, "Skill" ) )
            {
                int sn, lev, adp;

                word = fread_word( fp );
                lev = fread_number( fp );
                adp = fread_number( fp );
                sn = skill_lookup( word );
                if ( cl < 0 || cl >= MAX_RACE )
                {
                    sprintf( buf, "load_race: Skill %s -- class bad/not found",word);
                    bug( buf, 0 );
                }
                else
                if ( !IS_VALID_SN(sn) )
                {
                    sprintf( buf, "load_race: Skill %s unknown",word );
                    bug( buf, 0 );
                }
                else
                {
                    skill_table[sn]->race_skill_level[cl] = lev;
                    skill_table[sn]->race_skill_adept[cl] = adp;
                }
                fMatch = TRUE;
                break;
            }

        KEY( "StrPlus", 	race->str_plus, 	fread_number( fp ) );
	KEY( "Suscept",		race->suscept,		fread_number( fp ) );
	break;

	case 'W':
	KEY( "WisPlus",		race->wis_plus,		fread_number( fp ) );
	break;
        }

        if ( !fMatch  )
        {
          bug( "Load_Race: no match for %s(%s).", word );
        }
   }
}

/*
 * Returns the Text name of a language, by Bivector --GW
 */
char *getlangname( int value )
{

if ( value == BV00 ) return "Common";
if ( value == BV01 ) return "Elven";
if ( value == BV02 ) return "Dwarven";
if ( value == BV03 ) return "Pixie";
if ( value == BV04 ) return "Ogre";
if ( value == BV05 ) return "Orcish";
if ( value == BV06 ) return "Trollish";
if ( value == BV07 ) return "Leprachaun";
if ( value == BV08 ) return "Insectoid";
if ( value == BV09 ) return "Mammel";
if ( value == BV10 ) return "Mrrshan";
if ( value == BV11 ) return "Dragon";
if ( value == BV12 ) return "Spiritual";
if ( value == BV13 ) return "Magical";
if ( value == BV14 ) return "Goblin";
if ( value == BV15 ) return "God";
if ( value == BV16 ) return "Ancient";
if ( value == BV17 ) return "Halfling";
if ( value == BV18 ) return "Clan";
if ( value == BV19 ) return "Gith";
if ( value == BV20 ) return "Vampiric";
if ( value == BV21 ) return "Wolfish";
if ( value == BV22 ) return "Draconian";
if ( value == BV23 ) return "Satyr";
if ( value == BV24 ) return "Wraith";
if ( value == BV25 ) return "Centaur";
if ( value == BV26 ) return "Drider";
if ( value == BV27 ) return "Drowish";
if ( value == BV28 ) return "Minotuar";
if ( value == BV29 ) return "Cyclops";
if ( value == BV30 ) return "Arewyndel";
if ( value == BV31 ) return "Brownie";

return "Unknown";
}
