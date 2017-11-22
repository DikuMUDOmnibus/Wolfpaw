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
 * 			Database management module			    *
 ****************************************************************************/

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

extern	int	_filbuf		args( (FILE *) );

#if defined(KEY)
#undef KEY
#endif

void init_supermob();
void inroom_update(void);

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}


/*
 * Globals.
 */

ZONE_DATA *	zone_pointer;


WEB_DESCRIPTOR *first_webdesc;
WEB_DESCRIPTOR *last_webdesc;
int		top_web_desc;

WIZENT *	first_wiz;
WIZENT *	last_wiz;

bool		mudlog_off;

time_t                  last_restore_all_time = 0;
HELP_DATA *		first_help;
HELP_DATA *		last_help;

SHOP_DATA *		first_shop;
SHOP_DATA *		last_shop;

REPAIR_DATA *		first_repair;
REPAIR_DATA *		last_repair;

TELEPORT_DATA *		first_teleport;
TELEPORT_DATA *		last_teleport;

OBJ_DATA *		extracted_obj_queue;
EXTRACT_CHAR_DATA *	extracted_char_queue;

char			bug_buf		[2*MAX_INPUT_LENGTH];
CHAR_DATA *		first_char;
CHAR_DATA *		last_char;
char *			help_greeting[3];
char *			first_greeting;
char			log_buf		[2*MAX_INPUT_LENGTH];

SECURE_DATA *		first_secure;
SECURE_DATA *		last_secure;
REFFERAL_DATA *		first_refferal;
REFFERAL_DATA *		last_refferal;
PETITION_DATA *		first_petition;
PETITION_DATA *		last_petition;
MAIL_DATA *		first_mail;
MAIL_DATA *		last_mail;
ZONE_DATA *		first_zone;
ZONE_DATA *		last_zone;
/* Commented out by Greywolf, zones dont use these.*/
/*OBJ_DATA *		first_object;
OBJ_DATA *		last_object;*/
TIME_INFO_DATA		time_info;
WEATHER_DATA		weather_info;
TOPTEN_DATA *		first_topten;
TOPTEN_DATA *		last_topten;
bool			TOPTEN_ACTIVE=FALSE;
bool			MAIM_DEATH;
long			lowest_toptens[10];
int			cur_qobjs;
int			cur_qchars;
int			nummobsloaded;
int			numobjsloaded;
int			physicalobjects;
bool			ALIENS_RAN;
bool			WEBSERVER_STATUS;
bool			DRAGONS_RAN;
bool			FORCE_DRAGON_RUN;
bool			EMAIL_AUTH;

MAP_INDEX_DATA  *       first_map;	/* maps */

AUCTION_DATA    * 	auction;	/* auctions */

FILE		*	fpLOG;
FILE		*	fpMem;

/* weaponry */
sh_int			gsn_pugilism;
sh_int			gsn_long_blades;
sh_int			gsn_short_blades;
sh_int			gsn_flexible_arms;
sh_int			gsn_talonous_arms;
sh_int			gsn_bludgeons;
sh_int			gsn_missile_weapons;

/* thief */
sh_int			gsn_detrap;
sh_int          	gsn_backstab;
sh_int			gsn_circle;
sh_int			gsn_dodge;
sh_int			gsn_hide;
sh_int			gsn_peek;
sh_int			gsn_pick_lock;
sh_int			gsn_sneak;
sh_int			gsn_steal;
sh_int			gsn_gouge;
sh_int			gsn_poison_weapon;

/* thief & warrior */
sh_int          	gsn_disarm;
sh_int			gsn_enhanced_damage;
sh_int			gsn_kick;
sh_int			gsn_parry;
sh_int			gsn_rescue;
sh_int			gsn_second_attack;
sh_int			gsn_third_attack;
sh_int          	gsn_fourth_attack;
sh_int         		gsn_fifth_attack;
sh_int			gsn_sixth_attack;
sh_int			gsn_seventh_attack;
sh_int			gsn_dual_wield;
sh_int			gsn_punch;
sh_int			gsn_bash;
sh_int			gsn_stun;
sh_int                  gsn_bashdoor;
sh_int			gsn_grip; 
sh_int			gsn_berserk;
sh_int			gsn_hitall;

/* vampire */
sh_int          	gsn_feed;

/* other   */
sh_int			gsn_aid;
sh_int			gsn_track;
sh_int			gsn_search;
sh_int			gsn_dig;
sh_int			gsn_mount;
sh_int			gsn_bite;
sh_int			gsn_claw;
sh_int			gsn_sting;
sh_int			gsn_tail;
sh_int			gsn_scribe;
sh_int			gsn_brew;
sh_int			gsn_climb;
sh_int                  gsn_scan;
sh_int			gsn_slice;

/* archmage */
sh_int			gsn_ancient_lore;

/* adept */
sh_int			gsn_assassinate;

/* spells */
sh_int			gsn_aqua_breath;
sh_int          	gsn_blindness;
sh_int			gsn_charm_person;
sh_int			gsn_curse;
sh_int			gsn_invis;
sh_int			gsn_mass_invis;
sh_int			gsn_poison;
sh_int			gsn_sleep;
sh_int			gsn_possess;
sh_int			gsn_fireball;
sh_int			gsn_chill_touch;
sh_int			gsn_lightning_bolt;
sh_int			gsn_reflect_magic;
/* languages */
sh_int			gsn_common;
sh_int			gsn_elven;
sh_int			gsn_dwarven;
sh_int			gsn_pixie;
sh_int			gsn_ogre;
sh_int			gsn_orcish;
sh_int			gsn_trollish;
sh_int			gsn_goblin;
sh_int			gsn_halfling;
sh_int                  gsn_vampiric;
sh_int                  gsn_wolfish;
sh_int                  gsn_draconian;
sh_int                  gsn_satyr;
sh_int                  gsn_wraith;
sh_int                  gsn_centaur;
sh_int                  gsn_drider;
sh_int                  gsn_drowish;
sh_int                  gsn_minotaur;
sh_int                  gsn_cyclops;
sh_int                  gsn_arewyndel;
sh_int                  gsn_brownie;
sh_int                  gsn_leprachaun;
sh_int                  gsn_mrrshan;

/* for searching */
sh_int			gsn_first_spell;
sh_int			gsn_first_skill;
sh_int			gsn_first_weapon;
sh_int			gsn_first_tongue;
sh_int			gsn_top_sn;
int			top_zone;

/*
 * Locals.
 */
/* Same Here, zone doesnt use these --GW*/
/*MOB_INDEX_DATA *	mob_index_hash		[MAX_KEY_HASH];
OBJ_INDEX_DATA *	obj_index_hash		[MAX_KEY_HASH];
ROOM_INDEX_DATA *	room_index_hash		[MAX_KEY_HASH];*/

WARRENT_DATA *		first_warrent;
WARRENT_DATA *		last_warrent;
KOMBAT_DATA *		kombat;
CHALLENGE_DATA *	first_challenge;
CHALLENGE_DATA *	last_challenge;
ARENA_DATA *		first_arena;
ARENA_DATA *		last_arena;
AREA_DATA *		first_area;
AREA_DATA *		last_area;
AREA_DATA *		first_build;
AREA_DATA *		last_build;
AREA_DATA *		first_asort;
AREA_DATA *		last_asort;
AREA_DATA *		first_bsort;
AREA_DATA *		last_bsort;

SYSTEM_DATA		sysdata;

int			top_affect;
int			top_area;
int			top_zone;
int			top_ed;
int			top_exit;
int			top_help;
int			top_mob_index;
int			top_obj_index;
int			top_reset;
int			top_room;
int			top_shop;
int			top_repair;
int			top_vroom;
int		        chaos;  /* CHAOS (FREE PKILL) on/off */
int		        doubleexp; /*double EXP*/
int		        war; /*WAR MODE ON/OFF*/
sh_int			PULSE_PER_SECOND;
bool			TURBO_FIGHT;
int			TURBO_NUM;
bool			noenforce;
sh_int			dam_no_msg;
bool			DRAGON_DROP;
long			CASINO_BANK;
extern bool		CR;
char 			BOOTED_BY[MSL];
char 			BOOT_TIME[MSL];
char 			COMPILED_TIME[MSL];
char 			COMPILED_BY[MSL];
char 			MUD_VERSION[MSL];
char 			VERSNUM[MSL];
bool			ENABLE_DEATH_CRY;
extern bool		skip_locker;

/*
 * Semi-locals.
 */
bool			fBootDb;
FILE *			fpArea;
char			strArea[MAX_INPUT_LENGTH];


/*
 * Local booting procedures.
 */
void	init_mm		args( ( void ) );

void    load_mail	args( ( void ) );
void    load_zone_file  args( ( ZONE_DATA *tzone, char *filename ) );
void	boot_log	args( ( const char *str, ... ) );
void	load_area	args( ( FILE *fp ) );
void    load_author     args( ( AREA_DATA *tarea, FILE *fp ) );
void    load_economy    args( ( AREA_DATA *tarea, FILE *fp ) );
void	load_resetmsg	args( ( AREA_DATA *tarea, FILE *fp ) ); /* Rennard */
void    load_flags      args( ( AREA_DATA *tarea, FILE *fp ) );
void    load_zone       args( ( AREA_DATA *tarea, FILE *fp ) ); /* zone */
void	load_helps	args( ( AREA_DATA *tarea, FILE *fp ) );
void	load_mobiles	args( ( AREA_DATA *tarea, FILE *fp ) );
void	load_objects	args( ( AREA_DATA *tarea, FILE *fp ) );
void	load_resets	args( ( AREA_DATA *tarea, FILE *fp ) );
void	load_rooms	args( ( AREA_DATA *tarea, FILE *fp ) );
void	load_shops	args( ( AREA_DATA *tarea, FILE *fp ) );
void 	load_repairs	args( ( AREA_DATA *tarea, FILE *fp ) );
void	load_specials	args( ( AREA_DATA *tarea, FILE *fp ) );
void	load_vehicles	args( ( AREA_DATA *tarea, FILE *fp ) );
void    load_ranges	args( ( AREA_DATA *tarea, FILE *fp ) );
void	load_buildlist	args( ( void ) );
bool	load_systemdata	args( ( SYSTEM_DATA *sys ) );
void    load_banlist    args( ( void ) );
void	initialize_economy args( ( void ) );
void	fix_exits	args( ( void ) );
void	load_refferals  args( ( void ) );
void    clean_pfiles    args( ( void ) );
void    clean_lfiles    args( ( void ) );
AREA_DATA * find_room_area args( ( int vnum ) );
AREA_DATA * find_obj_area args( ( int vnum ) );
AREA_DATA * find_mob_area args( ( int vnum ) );

/*
 * External booting function
 */
void	load_corpses	args( ( void ) );
void	renumber_put_resets	args( ( AREA_DATA *pArea ) );
void    init_kombat         args( ( void ) );
void    init_challenge      args( ( void ) );
void    init_arenas         args( ( void ) );
void	load_races	    args( ( void ) );
void    make_web_wizlist    args( ( void ) );
void	init_security	    args( ( void ) );
void    load_imps	    args( ( void ) );
void 	load_mud_version( void );
void	load_compile_data( void );
void	load_boot_data( void );
void    pk_list_cleaner( void );
void	overlimit_msg( void );
void    show_overlimit( CHAR_DATA *ch );
void init_upstat( void );

/*
 * MUDprogram locals
 */

int 		mprog_name_to_type	args ( ( char* name ) );
MPROG_DATA *	mprog_file_read 	args ( ( char* f, MPROG_DATA* mprg,
						MOB_INDEX_DATA *pMobIndex ) );
/* int 		oprog_name_to_type	args ( ( char* name ) ); */
MPROG_DATA *	oprog_file_read 	args ( ( char* f, MPROG_DATA* mprg,
						OBJ_INDEX_DATA *pObjIndex ) );
/* int 		rprog_name_to_type	args ( ( char* name ) ); */
MPROG_DATA *	rprog_file_read 	args ( ( char* f, MPROG_DATA* mprg,
						ROOM_INDEX_DATA *pRoomIndex ) );
void		load_mudprogs           args ( ( AREA_DATA *tarea, FILE* fp ) );
void		load_objprogs           args ( ( AREA_DATA *tarea, FILE* fp ) );
void		load_roomprogs          args ( ( AREA_DATA *tarea, FILE* fp ) );
void   		mprog_read_programs     args ( ( FILE* fp,
						MOB_INDEX_DATA *pMobIndex) );
void   		oprog_read_programs     args ( ( FILE* fp,
						OBJ_INDEX_DATA *pObjIndex) );
void   		rprog_read_programs     args ( ( FILE* fp,
						ROOM_INDEX_DATA *pRoomIndex) );

char last_function_call [MSL]; /* Last Function Call Buffer */

void examine_last_command ()
{
    FILE *fp=NULL;
    FILE *fp2=NULL;
    FILE *fp3=NULL;
    char buf [MSL];
    char buf2 [MSL];
    char text [MSL];
    char * strtime;
    char   sender[MSL];
    char   date[MSL];

    fp2 = fopen (LAST_FUNCTION_FILE, "r" );
    if (!fp2)
	return;

    fscanf (fp2, "%[^\n]", buf2);
    new_fclose (fp2);
    unlink (LAST_FUNCTION_FILE);

    fp = fopen (LAST_COMMAND_FILE, "r");
    if (!fp)
        return;

    fscanf (fp, "%[^\n]", buf);
    new_fclose (fp);
    unlink (LAST_COMMAND_FILE);


        strtime                         = ctime( &current_time );
        strtime[strlen(strtime)-1]      = '\0';
        strcpy(date,strtime);
        strcpy(sender,"CrashLog");
        strcpy(text,buf);
        
        if ( ( fp3 = fopen( CRASH_FILE, "a" ) ) == NULL )
        {
            bug("Cannot Open CrashLog File!",0);
            perror( DIARY_FILE );
        }
        else
        {
            fprintf( fp3, "[%s] %-10s:\n[****] LAST COMMAND BEFORE CRASH [****]\n%s\n",
                date,
                sender,
                buf
                );
	    fprintf( fp3, "**** LAST FUNCTION CALLED WAS ****\n%s",
		buf2 );
            new_fclose( fp3 );
        }

 }

 void shutdown_mud( char *reason )
{
    FILE *fp;
    char buf[MSL];

    if ( (fp = fopen( SHUTDOWN_FILE, "a" )) != NULL )
    {
	fprintf( fp, "%s\n", reason );
	new_fclose( fp );
    }
    sprintf(buf,"MUD SHUTTING DOWN: %s",reason);
    log_string(buf);
}


/*
 * Big mama top level function.
 */
void boot_db( bool fCopyOver )
{
    sh_int wear, x;
    ZONE_DATA * zone;
    char	zone_buf[MSL];
    DIR *dp;
    extern bool DISABLE_LIMIT_SCAN;

    show_hash( 32 );
    unlink( BOOTLOG_FILE );
    unlink( BUG_FILE );
    unlink( MUDLOG_FILE );
    mudlog_off = TRUE;
    load_limited_toggle(TRUE);
    boot_log( "---------------------[ Boot Log ]--------------------" );

    log_string( "Loading commands" );
    load_commands();
    log_string( "Loading sysdata configuration..." );

    /* default values */
    sysdata.read_all_mail		= LEVEL_GOD;
    sysdata.read_mail_free 		= LEVEL_IMMORTAL;
    sysdata.write_mail_free 		= LEVEL_IMMORTAL;
    sysdata.take_others_mail		= LEVEL_GOD;
    sysdata.imc_mail_vnum		= 0;
    sysdata.muse_level			= LEVEL_HYPERION;
    sysdata.think_level			= LEVEL_HEAD_GOD;
    sysdata.build_level			= LEVEL_FATE;
    sysdata.log_level			= LEVEL_LOG;
    sysdata.level_modify_proto		= LEVEL_GOD;
    sysdata.level_override_private	= LEVEL_GOD;
    sysdata.level_mset_player		= LEVEL_GOD;
    sysdata.stun_plr_vs_plr		= 65;
    sysdata.stun_regular		= 15;
    sysdata.max_hp			= 1500;
    sysdata.max_mana			= 2500;
    sysdata.maxaward			= 500;
    sysdata.segvio		        = 0;
    sysdata.webserv			= 1;
    sysdata.dam_plr_vs_plr		= 100;
    sysdata.dam_plr_vs_mob		= 100;
    sysdata.dam_mob_vs_plr		= 100;
    sysdata.dam_mob_vs_mob		= 100;
    sysdata.level_getobjnotake 		= LEVEL_FATE;
    sysdata.save_frequency		= 20;	/* minutes */
    sysdata.save_flags			= SV_DEATH | SV_PASSCHG | SV_AUTO
    					| SV_PUT | SV_DROP | SV_GIVE
    					| SV_AUCTION | SV_ZAPDROP | SV_IDLE;
    sysdata.first_approve 		= NULL;
    sysdata.last_approve		= NULL;

    if ( !load_systemdata(&sysdata) )
    {
	log_string( "Not found.  Creating new configuration." );
	sysdata.alltimemax = 0;
    }

    /* New DB Loader --GW */
    load_main_db();

    log_string("Loading socials");
    load_socials();

    log_string("Loading skill table");
    load_skill_table();
    sort_skill_table();

    gsn_first_spell  = 0;
    gsn_first_skill  = 0;
    gsn_first_weapon = 0;
    gsn_first_tongue = 0;
    gsn_top_sn	     = top_sn;

    for ( x = 0; x < top_sn; x++ )
	if ( !gsn_first_spell && skill_table[x]->type == SKILL_SPELL )
	    gsn_first_spell = x;
	else
	if ( !gsn_first_skill && skill_table[x]->type == SKILL_SKILL )
	    gsn_first_skill = x;
	else
	if ( !gsn_first_weapon && skill_table[x]->type == SKILL_WEAPON )
	    gsn_first_weapon = x;
	else
	if ( !gsn_first_tongue && skill_table[x]->type == SKILL_TONGUE )
	    gsn_first_tongue = x;

    log_string("Loading classes");
    load_classes();

    log_string("Loading races");
    load_races();

    log_string("Loading herb table");
    load_herb_table();

    log_string("Making wizlist");
    make_wizlist();

    log_string("Making Web Wizlist");
    make_web_wizlist();

    log_string("Loading Mail");
    first_mail		= NULL;
    last_mail		= NULL;
    load_mail();

/* Never Used anyway --GW
 *   log_string("Loading Refferals");
 *   first_refferal	= NULL;
 *   last_refferal	= NULL;
 *   load_refferals();
 */

    log_string("Initializing Kombat...");
    init_kombat( );

    log_string("Initializing Challenge...");
    init_challenge( );

    log_string("Initializing Arena(s)...");
    init_arenas( );

    log_string("Initializing Stocks...");
    setup_shares();

    log_string("Setting up TopTen..");
    topten_setup();

    log_string("Initializing request pipe");
    init_request_pipe();
    
    fBootDb		= TRUE;
    
    CR			= FALSE;
    war			= 0; /* OFF is GOOD --GW */
    doubleexp		= 0; /* Double EXP off at reboot time =) GW */
    chaos		= 0; /* Chaos off at reboot is a good thing ... */
    dam_no_msg		= 0;
    skip_locker		= FALSE;
    TURBO_FIGHT		= FALSE; /* TURBO FIGHTING SPEED OFF .. GW */
    TURBO_NUM		= 4; /* 4 is normal */
    WEBSERVER_STATUS	= FALSE;
    noenforce		= FALSE; /* Enforce Laws! HELL YES! --GW */
    DRAGON_DROP		= TRUE; /* Dragon Dropping On! --GW */
    MAIM_DEATH		= FALSE;
    ENABLE_DEATH_CRY	= TRUE; /* Death Cry Switch */
    nummobsloaded	= 0;
    numobjsloaded	= 0;
    physicalobjects	= 0;
    sysdata.maxplayers	= 0;
    sysdata.logins	= 0;
    first_zone		= NULL;
    last_zone		= NULL;
    first_char		= NULL;
    last_char		= NULL;
    first_area		= NULL;
    last_area		= NULL;
    first_build		= NULL;
    last_area		= NULL;
    first_shop		= NULL;
    last_shop		= NULL;
    first_repair	= NULL;
    last_repair		= NULL;
    first_teleport	= NULL;
    last_teleport	= NULL;
    first_asort		= NULL;
    last_asort		= NULL;
    extracted_obj_queue	= NULL;
    extracted_char_queue= NULL;
    cur_qobjs		= 0;
    cur_qchars		= 0;
    cur_char		= NULL;
    cur_obj		= 0;
    cur_obj_serial	= 0;
    cur_char_died	= FALSE;
    cur_obj_extracted	= FALSE;
    cur_room		= NULL;
    quitting_char	= NULL;
    loading_char	= NULL;
    saving_char		= NULL;
    CREATE( auction, AUCTION_DATA, 1);
    auction->item 	= NULL;
    for ( wear = 0; wear < MAX_WEAR; wear++ )
	for ( x = 0; x < MAX_LAYERS; x++ )
	    save_equipment[wear][x] = NULL;

    /*
     * Init random number generator.
     */
    log_string("Initializing random number generator");
    init_mm( );

    /*
     * Set time and weather.
     */
    {
	long lhour, lday, lmonth;

	log_string("Setting time and weather");

	lhour		= (current_time - 650336715)
			/ (PULSE_TICK / PULSE_PER_SECOND);
	time_info.hour	= lhour  % 24;
	lday		= lhour  / 24;
	time_info.day	= lday   % 35;
	lmonth		= lday   / 35;
	time_info.month	= lmonth % 17;
	time_info.year	= lmonth / 17;

	     if ( time_info.hour <  5 ) weather_info.sunlight = SUN_DARK;
	else if ( time_info.hour <  6 ) weather_info.sunlight = SUN_RISE;
	else if ( time_info.hour < 19 ) weather_info.sunlight = SUN_LIGHT;
	else if ( time_info.hour < 20 ) weather_info.sunlight = SUN_SET;
	else                            weather_info.sunlight = SUN_DARK;

	weather_info.change	= 0;
	weather_info.mmhg	= 960;
	if ( time_info.month >= 7 && time_info.month <=12 )
	    weather_info.mmhg += number_range( 1, 50 );
	else
	    weather_info.mmhg += number_range( 1, 80 );


	     if ( weather_info.mmhg <=  980 ) weather_info.sky = SKY_LIGHTNING;
	else if ( weather_info.mmhg <= 1000 ) weather_info.sky = SKY_RAINING;
	else if ( weather_info.mmhg <= 1020 ) weather_info.sky = SKY_CLOUDY;
	else                                  weather_info.sky = SKY_CLOUDLESS;

    }


    /*
     * Assign gsn's for skills which need them.
     */
    {
	log_string("Assigning gsn's");

	ASSIGN_GSN( gsn_pugilism,	"pugilism" );
	ASSIGN_GSN( gsn_long_blades,	"long blades" );
	ASSIGN_GSN( gsn_short_blades,	"short blades" );
	ASSIGN_GSN( gsn_flexible_arms,	"flexible arms" );
	ASSIGN_GSN( gsn_talonous_arms,	"talonous arms" );
	ASSIGN_GSN( gsn_bludgeons,	"bludgeons" );
	ASSIGN_GSN( gsn_missile_weapons,"missile weapons" );
	ASSIGN_GSN( gsn_detrap,		"detrap" );
	ASSIGN_GSN( gsn_backstab,	"backstab" );
	ASSIGN_GSN( gsn_assassinate,	"assassinate" );
	ASSIGN_GSN( gsn_circle,		"circle" );
	ASSIGN_GSN( gsn_dodge,		"dodge" );
	ASSIGN_GSN( gsn_hide,		"hide" );
	ASSIGN_GSN( gsn_peek,		"peek" );
	ASSIGN_GSN( gsn_pick_lock,	"pick lock" );
	ASSIGN_GSN( gsn_sneak,		"sneak" );
	ASSIGN_GSN( gsn_steal,		"steal" );
	ASSIGN_GSN( gsn_gouge,		"gouge" );
	ASSIGN_GSN( gsn_poison_weapon, 	"poison weapon" );
	ASSIGN_GSN( gsn_disarm,		"disarm" );
	ASSIGN_GSN( gsn_enhanced_damage, "enhanced damage" );
	ASSIGN_GSN( gsn_kick,		"kick" );
	ASSIGN_GSN( gsn_parry,		"parry" );
	ASSIGN_GSN( gsn_rescue,		"rescue" );
	ASSIGN_GSN( gsn_second_attack, 	"second attack" );
	ASSIGN_GSN( gsn_third_attack, 	"third attack" );
	ASSIGN_GSN( gsn_fourth_attack, 	"fourth attack" );
	ASSIGN_GSN( gsn_fifth_attack, 	"fifth attack" );
	ASSIGN_GSN( gsn_sixth_attack,	"sixth attack" );
	ASSIGN_GSN( gsn_seventh_attack,	"seventh attack" );
	ASSIGN_GSN( gsn_dual_wield,	"dual wield" );
	ASSIGN_GSN( gsn_punch,		"punch" );
	ASSIGN_GSN( gsn_bash,		"bash" );
	ASSIGN_GSN( gsn_stun,		"stun" );
	ASSIGN_GSN( gsn_bashdoor,	"doorbash" );
	ASSIGN_GSN( gsn_grip,		"grip" ); 
	ASSIGN_GSN( gsn_berserk,	"berserk" );
	ASSIGN_GSN( gsn_hitall,		"hitall" );
	ASSIGN_GSN( gsn_feed,		"feed" );
	ASSIGN_GSN( gsn_ancient_lore,	"ancient lore" );
	ASSIGN_GSN( gsn_aid,		"aid" );
	ASSIGN_GSN( gsn_track,		"track" );
	ASSIGN_GSN( gsn_search,		"search" );
	ASSIGN_GSN( gsn_dig,		"dig" );
	ASSIGN_GSN( gsn_mount,		"mount" );
	ASSIGN_GSN( gsn_bite,		"bite" );
	ASSIGN_GSN( gsn_claw,		"claw" );
	ASSIGN_GSN( gsn_sting,		"sting" );
	ASSIGN_GSN( gsn_tail,		"tail" );
	ASSIGN_GSN( gsn_scribe,		"scribe" );
	ASSIGN_GSN( gsn_brew,		"brew" );
	ASSIGN_GSN( gsn_climb,		"climb" );
	ASSIGN_GSN( gsn_scan,		"scan" );
	ASSIGN_GSN( gsn_slice,		"slice" );
	ASSIGN_GSN( gsn_reflect_magic,  "reflect magic" );
	ASSIGN_GSN( gsn_fireball,	"fireball" );
	ASSIGN_GSN( gsn_chill_touch,	"chill touch" );
	ASSIGN_GSN( gsn_lightning_bolt,	"lightning bolt" );
	ASSIGN_GSN( gsn_aqua_breath,	"aqua breath" );
	ASSIGN_GSN( gsn_blindness,	"blindness" );
	ASSIGN_GSN( gsn_charm_person, 	"charm person" );
	ASSIGN_GSN( gsn_curse,		"curse" );
	ASSIGN_GSN( gsn_invis,		"invis" );
	ASSIGN_GSN( gsn_mass_invis,	"mass invis" );
	ASSIGN_GSN( gsn_poison,		"poison" );
	ASSIGN_GSN( gsn_sleep,		"sleep" );
	ASSIGN_GSN( gsn_possess,	"possess" );
	ASSIGN_GSN( gsn_common,		"common" );
	ASSIGN_GSN( gsn_elven,		"elven" );
	ASSIGN_GSN( gsn_dwarven,	"dwarven" );
	ASSIGN_GSN( gsn_pixie,		"pixie" );
	ASSIGN_GSN( gsn_ogre,		"ogre" );
	ASSIGN_GSN( gsn_orcish,		"orcish" );
	ASSIGN_GSN( gsn_trollish,	"trollese" );
	ASSIGN_GSN( gsn_goblin,		"goblin" );
	ASSIGN_GSN( gsn_halfling,	"halfling" );
        ASSIGN_GSN( gsn_vampiric,       "vampiric" );
        ASSIGN_GSN( gsn_wolfish,        "wolfish" );
        ASSIGN_GSN( gsn_draconian,      "draconian" );
        ASSIGN_GSN( gsn_satyr,          "satyr" );
        ASSIGN_GSN( gsn_wraith,         "wraith" );
        ASSIGN_GSN( gsn_centaur,        "centaur" );
        ASSIGN_GSN( gsn_drider,         "drider" );
        ASSIGN_GSN( gsn_drowish,        "drowish" );
        ASSIGN_GSN( gsn_minotaur,       "minotaur" );
        ASSIGN_GSN( gsn_cyclops,        "cyclops" );
        ASSIGN_GSN( gsn_arewyndel,      "arewyndel" );
        ASSIGN_GSN( gsn_brownie,        "brownie" );
        ASSIGN_GSN( gsn_leprachaun,     "leprachaun" );
        ASSIGN_GSN( gsn_mrrshan,        "mrrshan" );
    }

    /*
     * Read in all the area files.
     */
    {
	FILE *fpList;

	top_zone = 0;
	log_string("Reading in zones...");
	if ( ( fpList = fopen( ZONE_LIST, "r" ) ) == NULL )
	{
	    perror( ZONE_LIST );
	    shutdown_mud( "Unable to open zone list" );
	    exit( 1 );
	}

	for ( ; ; )
	{
	    strcpy( strArea, fread_word( fpList ) );
	    if ( strArea[0] == '$' )
		break;

    	    load_zone_file( last_zone, strArea );
	
	}
	new_fclose( fpList );
     }    

for ( zone = first_zone; zone; zone = zone->next )
{

sprintf(zone_buf,"zone%d",zone->number);

zone_pointer = zone;
zone->first_obj = NULL;
zone->last_obj = NULL;
zone->first_mob = NULL;
zone->last_mob = NULL;

    if ( !(dp = opendir(zone_buf)) )
  {
    bug( "Load_Zones: can't open %s", zone_buf);
    perror(zone_buf);
    return;
  }

sprintf(zone_buf,"zone%d/%s",zone->number,zone->list_filename);

    /*
     * Read in all the area files.
     */
    {
	FILE *fpList;

	log_string("Reading in area files...");
	if ( ( fpList = fopen( zone_buf, "r" ) ) == NULL )
	{
	    perror( AREA_LIST );
	    shutdown_mud( "Unable to open area list" );
	    exit( 1 );
	}

	for ( ; ; )
	{
	    strcpy( strArea, fread_word( fpList ) );
	    if ( strArea[0] == '$' )
		break;
	
	sprintf(zone_buf,"zone%d/%s",zone->number,strArea);

	    load_area_file( last_area, zone_buf );
  	    /* Dont USe, --GW fold_area(last_area,zone_buf,FALSE); */
	    
	}
	new_fclose( fpList );
    }

  closedir(dp);

}
/**/

   /*
    *   initialize supermob.
    *    must be done before reset_area!
    *
    */
    init_supermob();


    /*
     * Fix up exits.
     * Declare db booting over.
     * Reset all areas once.
     * Load up the notes file.
     */
    {
	log_string( "Fixing exits" );
	fix_exits( );
	log_string( "Initializing economy" );
	initialize_economy( );
	log_string( "Limit system Loading....");
	load_limit_db();

     if ( !DISABLE_LIMIT_SCAN )
     {
	log_string( "Reinitializing Limit Tables...");
	reinit_limits();
        log_string( "Updating Limits....");
        object_limit_update();
     }
	log_string( "Writing Overlimit File...");
	overlimit_msg();
	log_string( "Resetting areas" );
	area_update( );
	fBootDb	= FALSE;
	log_string( "Loading buildlist" );
	load_buildlist( );
	log_string( "Loading boards" );
	load_boards( );
	log_string( "Loading clans" );
	load_clans( );
	log_string( "Setting Up PK-TRACKING" );
        clan_pkill_record_setup( );
	log_string( "Loading councils" );
	load_deity( );
	log_string( "Loading deities" );
	load_councils( );
        log_string( "Loading bans" );
        load_banlist( );
        log_string( "Loading corpses" );
        load_corpses( );
	log_string( "Loading Warrents" );
	first_warrent = NULL;
	last_warrent = NULL;
        load_warrents( ); 
        log_string("Setting up Aliens...");
	ALIENS_RAN = FALSE;
        log_string("Setting up Dragons...");
        DRAGONS_RAN = TRUE;
	FORCE_DRAGON_RUN = FALSE;
	log_string( "Loading System Messages" );
        load_system_messages( );
	log_string( "Loading SAVE-ROOMS -- Disabled" );
/*	load_save_rooms( );*/
        log_string( "Disabling Email Authorization" );
        EMAIL_AUTH=FALSE;
	log_string( "Setting up the Casino Bank..");
	setup_casino();
//        log_string("Setting up CTF..");
//        setup_ctf();

	/* Security --GW */
	log_string("Initializing Security Features..");
	init_security();
	log_string("WME-SECURE: Loading...");
	load_imps();

	/* Version/Compile/Boot Shell Stuff --GW */
	log_string("Loading Compile Data..");
	load_compile_data();
	log_string("Loading Boot Data..");
	load_boot_data();
	log_string("Loading MUD Version..");
	load_mud_version();

	/* Upstat --GW */
	init_upstat();

	if ( !fCopyOver ) /* Slows CopyOver Down to much --GW */
	{
	/* Clean Pfiles Up */
/*	clean_pfiles();*/
	/* Clean Locker Files Up */
/*	clean_lfiles();*/
	}

        MOBtrigger = TRUE;
        if (fCopyOver)
         {
	  reset_copyover_boot_time();
          log_string("Running copyover_recover.");
          copyover_recover();
         }
    }

    /* init_maps ( ); */

    mudlog_off = FALSE;
    return;
}

/*
 * Kombat Data Init Function --GW
 */
void init_kombat( void )
{
CREATE( kombat, KOMBAT_DATA, 1 );
kombat->bits = 0;
kombat->hi_lev = 0;
kombat->low_lev = 0;
return;
}

/*
 * Init Areans Function --GW
 */
void init_arenas( void )
{
ARENA_DATA *arena=NULL;

first_arena = NULL;
last_arena = NULL;

/* Arena #1 */
log_string("Loading Arena #1...");
CREATE( arena, ARENA_DATA, 1 );
arena->char_room = 42;
arena->vict_room = 43;
arena->in_use = FALSE;
LINK( arena, first_arena, last_arena, next, prev );

/* Done */
log_string("Arena Loading Complete.");
return;
}

/*
 * Init Challenge Function --GW 
 */
void init_challenge( void )
{
first_challenge = NULL;
last_challenge = NULL;
return;
}

/*
 * Load an 'area' header line.
 */
void load_area( FILE *fp )
{
    AREA_DATA *pArea=NULL;

    CREATE( pArea, AREA_DATA, 1 );
    pArea->first_reset	= NULL;
    pArea->last_reset	= NULL;
    pArea->name		= fread_string_nohash( fp );
    pArea->author       = STRALLOC( "unknown" );
    pArea->filename	= str_dup( strArea );
    pArea->age		= 15;
    pArea->nplayer	= 0;
    pArea->low_r_vnum	= 0;
    pArea->low_o_vnum	= 0;
    pArea->low_m_vnum	= 0;
    pArea->hi_r_vnum	= 0;
    pArea->hi_o_vnum	= 0;
    pArea->hi_m_vnum	= 0;
    pArea->low_soft_range = 0;
    pArea->hi_soft_range  = MAX_LEVEL;
    pArea->low_hard_range = 0;
    pArea->hi_hard_range  = MAX_LEVEL;
    pArea->zone		  = zone_pointer;
    pArea->version_number = 0;
    pArea->version_name   = STRALLOC("Unknown");

    LINK( pArea, first_area, last_area, next, prev );
    top_area++;
    return;
}

/*
 * Load an 'zone' header line.
 */
void load_zone_header( FILE *fp )
{
    ZONE_DATA *pZone=NULL;

    CREATE( pZone, ZONE_DATA, 1 );
    pZone->name		= fread_string_nohash( fp );
    pZone->filename	= str_dup( strArea );

    LINK( pZone, first_zone, last_zone, next, prev );
    return;
}


/*
 * Load zones list file name .. --GW
 */
void load_zone_lname( ZONE_DATA *tzone, FILE *fp )
{
    if ( !tzone )
    {
	bug( "Load_author: no #ZONE seen yet." );
	if ( fBootDb )
	{
	  shutdown_mud( "No #ZONE" );
	  exit( 1 );
	}
	else
	  return;
    }

    if ( tzone->list_filename )
      STRFREE( tzone->list_filename );
    tzone->list_filename   = fread_string( fp );
    return;
}

/*
 * Load zones name .. --GW
 */
void load_zone_name( ZONE_DATA *tzone, FILE *fp )
{
    if ( !tzone )
    {
	bug( "Load_author: no #ZONE seen yet." );
	if ( fBootDb )
	{
	  shutdown_mud( "No #ZONE" );
	  exit( 1 );
	}
	else
	  return;
    }

    if ( tzone->name )
      STRFREE( tzone->name );
    tzone->name   = fread_string( fp );
    return;
}

/*
 * Load an author section. Scryn 2/1/96
 */
void load_author( AREA_DATA *tarea, FILE *fp )
{
    if ( !tarea )
    {
	bug( "Load_author: no #AREA seen yet." );
	if ( fBootDb )
	{
	  shutdown_mud( "No #AREA" );
	  exit( 1 );
	}
	else
	  return;
    }

    if ( tarea->author )
      STRFREE( tarea->author );
    tarea->author   = fread_string( fp );
    return;
}

/*
 * Map filename --GW
 */
void load_mapfile( AREA_DATA *tarea, FILE *fp )
{
    if ( !tarea )
    {
	bug( "Load_author: no #AREA seen yet." );
	if ( fBootDb )
	{
	  shutdown_mud( "No #AREA" );
	  exit( 1 );
	}
	else
	  return;
    }

    if ( tarea->map_file )
      DISPOSE( tarea->map_file );
    tarea->map_file   = fread_string_nohash( fp );
    return;
}

/*
 * Load Version Info
 */
void load_version( AREA_DATA *tarea, FILE *fp )
{
char *temp=NULL;

    if ( !tarea )
    {
	bug( "Load_author: no #AREA seen yet." );
	if ( fBootDb )
	{
	  shutdown_mud( "No #AREA" );
	  exit( 1 );
	}
	else
	  return;
    }

    if ( tarea->version_name )
      STRFREE( tarea->author );

    temp = fread_string(fp);
    tarea->version_number = atoi(&temp[0]);
    temp++;
    temp++;
    tarea->version_name = STRALLOC(temp);
    tarea->version_name[strlen(tarea->version_name)-1] = '\0';
    return;
}

/*
 * Load an club section. --GW
 */
void load_club( AREA_DATA *tarea, FILE *fp )
{
    if ( !tarea )
    {
	bug( "Load_club: no #AREA seen yet." );
	if ( fBootDb )
	{
	  shutdown_mud( "No #AREA" );
	  exit( 1 );
	}
	else
	  return;
    }

    if ( tarea->club )
      STRFREE( tarea->club );
    tarea->club   = fread_string( fp );
    return;
}

/*
 * Load an economy section. Thoric
 */
void load_economy( AREA_DATA *tarea, FILE *fp )
{
    if ( !tarea )
    {
	bug( "Load_economy: no #AREA seen yet." );
	if ( fBootDb )
	{
	  shutdown_mud( "No #AREA" );
	  exit( 1 );
	}
	else
	  return;
    }

    tarea->high_economy	= fread_number( fp );
    tarea->low_economy	= fread_number( fp );
    return;
}

/* Reset Message Load, Rennard */
void load_resetmsg( AREA_DATA *tarea, FILE *fp )
{
    if ( !tarea )
    {
	bug( "Load_resetmsg: no #AREA seen yet." );
	if ( fBootDb )
	{
	  shutdown_mud( "No #AREA" );
	  exit( 1 );
	}
	else
	  return;
    }
    
    if ( tarea->resetmsg )
	DISPOSE( tarea->resetmsg );
    tarea->resetmsg = fread_string_nohash( fp );
    return;
}


/*
 * Load area flags. Narn, Mar/96 
 */
void load_flags( AREA_DATA *tarea, FILE *fp )
{
    char *ln;
    int x1, x2, x3, x4;

    if ( !tarea )
    {
	bug( "Load_flags: no #AREA seen yet." );
	if ( fBootDb )
	{
	  shutdown_mud( "No #AREA" );
	  exit( 1 );
	}
	else
	  return;
    }
    ln = fread_line( fp );
    x1=x2=x3=x4=0;
    sscanf( ln, "%d %d %d %d",
	&x1, &x2, &x3, &x4 );
    tarea->flags = x1;
    tarea->locklev = x2;
    tarea->reset_frequency = x3;
    tarea->seclev = x4;

    if ( IS_SET( tarea->flags, AFLAG_STARTED ) )
	REMOVE_BIT( tarea->flags, AFLAG_STARTED );

    if ( x3 )
	tarea->age = x3;
    return;
}

/*
 * Load Zone Number -- GW 
 */
void load_zone_number( ZONE_DATA *tzone, FILE *fp )
{
    char *ln;
    int x1;

    if ( !tzone )
    {
	bug( "Load_Zone_Number: no #ZONE seen yet." );
	if ( fBootDb )
	{
	  shutdown_mud( "No #ZONE" );
	  exit( 1 );
	}
	else
	  return;
    }
    ln = fread_line( fp );
    x1=0;
    sscanf( ln, "%d",
	&x1 );
    tzone->number = x1;
    return;
}

/*
 * Adds a help page to the list if it is not a duplicate of an existing page.
 * Page is insert-sorted by keyword.			-Thoric
 * (The reason for sorting is to keep do_hlist looking nice)
 */
void add_help( HELP_DATA *pHelp )
{
    HELP_DATA *tHelp;
    int match;

    for ( tHelp = first_help; tHelp; tHelp = tHelp->next )
	if ( pHelp->level == tHelp->level
	&&   strcmp(pHelp->keyword, tHelp->keyword) == 0 )
	{
	    bug( "add_help: duplicate: %s.  Deleting.", pHelp->keyword );
	    STRFREE( pHelp->text );
	    STRFREE( pHelp->keyword );
	    DISPOSE( pHelp );
	    return;
	}
	else
	if ( (match=strcmp(pHelp->keyword[0]=='\'' ? pHelp->keyword+1 : pHelp->keyword,
			   tHelp->keyword[0]=='\'' ? tHelp->keyword+1 : tHelp->keyword)) < 0
	||   (match == 0 && pHelp->level > tHelp->level) )
	{
	    if ( !tHelp->prev )
		first_help	  = pHelp;
	    else
		tHelp->prev->next = pHelp;
	    pHelp->prev		  = tHelp->prev;
	    pHelp->next		  = tHelp;
	    tHelp->prev		  = pHelp;
	    break;
	}

    if ( !tHelp )
	LINK( pHelp, first_help, last_help, next, prev );

    top_help++;
}

/*
 * Load a help section.
 */
void load_helps( AREA_DATA *tarea, FILE *fp )
{
    HELP_DATA *pHelp=NULL;
    char buf[MSL];

    for ( ; ; )
    {
	pHelp=NULL;
	CREATE( pHelp, HELP_DATA, 1 );
	pHelp->level	= fread_number( fp );
	if( pHelp->level > MAX_LEVEL )
		pHelp->level = MAX_LEVEL;
	pHelp->keyword	= fread_string( fp );
	if ( pHelp->keyword[0] == '$' )
	    break;

	/* DO we REALLY want to see this?? --GW */
	if ( mudarg_scan('H') )
	{
  	  sprintf(buf,"Loading Help: %s",pHelp->keyword);
	  log_string(buf);
	}

	/* Added this to keep track of who does what --GW */
	pHelp->update	= fread_string( fp );
	pHelp->text	= fread_string( fp );
	if ( pHelp->keyword[0] == '\0' )
	{
	    STRFREE( pHelp->text );
	    STRFREE( pHelp->keyword );
	    DISPOSE( pHelp );
	    continue;
	}
	 if ( !str_cmp( pHelp->keyword, "greeting" ) )
	     first_greeting = pHelp->text;

         if ( !str_cmp( pHelp->keyword, "greeting1" ) )
             help_greeting[0] = pHelp->text;
         
         if ( !str_cmp( pHelp->keyword, "greeting2" ) )
             help_greeting[1] = pHelp->text;

         if ( !str_cmp( pHelp->keyword, "greeting3" ) )
             help_greeting[2] = pHelp->text;

          add_help(pHelp);

}

    return;
}


/*
 * Add a character to the list of all characters		-Thoric
 */
void add_char( CHAR_DATA *ch )
{
    LINK( ch, first_char, last_char, next, prev );
}


/*
 * Load a mob section.
 */
void load_mobiles( AREA_DATA *tarea, FILE *fp )
{
    MOB_INDEX_DATA *pMobIndex=NULL;
    char *ln;
    int x1, x2, x3, x4, x5, x6, x7, x8;

    if ( !tarea )
    {
	bug( "Load_mobiles: no #AREA seen yet." );
	if ( fBootDb )
	{
	    shutdown_mud( "No #AREA" );
	    exit( 1 );
	}
	else
	    return;
    }

    for ( ; ; )
    {
	char buf[MAX_STRING_LENGTH];
	sh_int vnum;
	char letter;
	int iHash;
	bool oldmob;
	bool tmpBootDb;
/*	char diceletter;
	char diceletter2;

	diceletter  = 'd';
	diceletter2 = '+';
*/
	letter				= fread_letter( fp );
	if ( letter != '#' )
	{
	    bug( "Load_mobiles: # not found." );
	    if ( fBootDb )
	    {
		shutdown_mud( "# not found" );
		exit( 1 );
	    }
	    else
		return;
	}

	vnum				= fread_number( fp );
	if ( vnum == 0 )
	    break;

	tmpBootDb = fBootDb;
	fBootDb = FALSE;
	if ( get_mob_index( vnum,(int)tarea->zone->number ) )
	{
	    if ( tmpBootDb )
	    {
		bug( "Load_mobiles: vnum %d duplicated.", vnum );
		shutdown_mud( "duplicate vnum" );
		exit( 1 );
	    }
	    else
	    {
		pMobIndex = get_mob_index( vnum,(int)tarea->zone->number );
		sprintf( buf, "Cleaning mobile: %d", vnum );
		log_string_plus( buf, LOG_BUILD, sysdata.log_level );
		clean_mob( pMobIndex );
		oldmob = TRUE;
	    }
	}
	else
	{
	  oldmob = FALSE;
	  pMobIndex=NULL;
	  CREATE( pMobIndex, MOB_INDEX_DATA, 1 );
	}
	fBootDb = tmpBootDb;

	pMobIndex->vnum			= vnum;
	if ( fBootDb )
	{
	    if ( !tarea->low_m_vnum )
		tarea->low_m_vnum	= vnum;
	    if ( vnum > tarea->hi_m_vnum )
		tarea->hi_m_vnum	= vnum;
	}
	pMobIndex->player_name		= fread_string( fp );
	pMobIndex->short_descr		= fread_string( fp );
	pMobIndex->long_descr		= fread_string( fp );
	pMobIndex->description		= fread_string( fp );
        pMobIndex->area			= tarea;

	pMobIndex->long_descr[0]	= UPPER(pMobIndex->long_descr[0]);
	pMobIndex->description[0]	= UPPER(pMobIndex->description[0]);

	/* Modification Tracking --GW */
	if ( tarea->version_number > 1 )
	{
	 pMobIndex->modified_by		= fread_string( fp );
	 pMobIndex->modified_date	= fread_string( fp );
	}
	else
	{
	 pMobIndex->modified_by		= STRALLOC("Unknown");
	 pMobIndex->modified_date	= STRALLOC("Unknown");
	}

	pMobIndex->act			= fread_number( fp ) | ACT_IS_NPC;
	pMobIndex->flags		= 0;
	pMobIndex->affected_by		= fread_bitvector( fp );
	pMobIndex->pShop		= NULL;
	pMobIndex->rShop		= NULL;
	pMobIndex->alignment		= fread_number( fp );
	letter				= fread_letter( fp );
	/* second act mob bit sting -- GW */
	pMobIndex->acttwo		= fread_number( fp );
	pMobIndex->smart		= fread_number( fp ); 

	pMobIndex->level		= fread_number( fp );

	pMobIndex->mobthac0		= fread_number( fp );
	pMobIndex->ac			= fread_number( fp );
	pMobIndex->hitnodice		= fread_number( fp );
/*	diceletter  */			  fread_letter( fp ); 
	pMobIndex->hitsizedice		= fread_number( fp );
/*	diceletter2 */		  	  fread_letter( fp ); 
	pMobIndex->hitplus		= fread_number( fp );
	pMobIndex->damnodice		= fread_number( fp );
/*	diceletter  */			  fread_letter( fp ); 
	pMobIndex->damsizedice		= fread_number( fp );
/*	diceletter2 */			  fread_letter( fp ); 
	pMobIndex->damplus		= fread_number( fp );
	pMobIndex->gold			= fread_number( fp );
	pMobIndex->exp			= fread_number( fp );
        pMobIndex->exp 			= 0; /* no exp on mobs man --GW*/
	pMobIndex->position		= fread_number( fp );
	pMobIndex->defposition		= fread_number( fp );

	/*
	 * Back to meaningful values.
	 */
	pMobIndex->sex			= fread_number( fp );

	if ( letter != 'S' && letter != 'C' )
	{
	    bug( "Load_mobiles: vnum %d: letter '%c' not S or C.", vnum,
	        letter );
	    shutdown_mud( "bad mob data" );
	    exit( 1 );
	}


	if ( letter == 'C' ) /* Realms complex mob 	-Thoric */
	{
	    pMobIndex->perm_str			= fread_number( fp );
	    pMobIndex->perm_int			= fread_number( fp );
	    pMobIndex->perm_wis			= fread_number( fp );
	    pMobIndex->perm_dex			= fread_number( fp );
	    pMobIndex->perm_con			= fread_number( fp );
	    pMobIndex->perm_cha			= fread_number( fp );
	    pMobIndex->perm_lck			= fread_number( fp );
 	    pMobIndex->saving_poison_death	= fread_number( fp );
	    pMobIndex->saving_wand		= fread_number( fp );
	    pMobIndex->saving_para_petri	= fread_number( fp );
	    pMobIndex->saving_breath		= fread_number( fp );
	    pMobIndex->saving_spell_staff	= fread_number( fp );
	    ln = fread_line( fp );
	    x1=x2=x3=x4=x5=x6=x7=0;
	    sscanf( ln, "%d %d %d %d %d %d %d %d",
		&x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8 );
	    pMobIndex->race		= x1;
	    pMobIndex->class		= x2;
	    pMobIndex->height		= x3;
	    pMobIndex->weight		= x4;
	    pMobIndex->speaks		= x5;
	    pMobIndex->speaking		= x6;
	    pMobIndex->numattacks	= x7;

		/* numattacks bug fix by Greywolf */

		if ( pMobIndex->numattacks < 1 )
		{
		  sprintf(buf,"LOAD_MOBS: NUMATTACKS LESS THEN 1 mob %d",
		  pMobIndex->vnum );
		  bug(buf);
	          bug("Fixing...");
		  pMobIndex->numattacks = 1;
		}

	    if ( race_table[pMobIndex->race] )
	    {
	    if ( !pMobIndex->speaks )
		pMobIndex->speaks = race_table[pMobIndex->race]->language | LANG_COMMON;
	    if ( !pMobIndex->speaking )
		pMobIndex->speaking = race_table[pMobIndex->race]->language;
	    }
	    else
	    {
	    pMobIndex->speaks = LANG_COMMON;
	    pMobIndex->speaking = LANG_COMMON;
            }
	    ln = fread_line( fp );
	    x1=x2=x3=x4=x5=x6=x7=x8=0;
	    sscanf( ln, "%d %d %d %d %d %d %d %d",
		&x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8 );
	    pMobIndex->hitroll		= x1;
	    pMobIndex->damroll		= x2;
	    pMobIndex->xflags		= x3;
	    pMobIndex->resistant	= x4;
	    pMobIndex->immune		= x5;
	    pMobIndex->susceptible	= x6;
	    pMobIndex->attacks		= x7;
	    pMobIndex->defenses		= x8;
	}
	else
	{
	    pMobIndex->perm_str		= 13;
	    pMobIndex->perm_dex		= 13;
	    pMobIndex->perm_int		= 13;
	    pMobIndex->perm_wis		= 13;
	    pMobIndex->perm_cha		= 13;
	    pMobIndex->perm_con		= 13;
	    pMobIndex->perm_lck		= 13;
	    pMobIndex->race		= 0;
	    pMobIndex->class		= 3;
	    pMobIndex->xflags		= 0;
	    pMobIndex->resistant	= 0;
	    pMobIndex->immune		= 0;
	    pMobIndex->susceptible	= 0;
	    pMobIndex->numattacks	= 0;
	    pMobIndex->attacks		= 0;
	    pMobIndex->defenses		= 0;
	}

	letter = fread_letter( fp );
	if ( letter == '>' )
	{
	    ungetc( letter, fp );
	    mprog_read_programs( fp, pMobIndex );
	}
	else ungetc( letter,fp );

	SET_BIT( pMobIndex->act, ACT_IS_NPC );

	if ( !oldmob )
	{
	    iHash			= vnum % MAX_KEY_HASH;
	    pMobIndex->next		= zone_pointer->mob_index_hash[iHash];
	    zone_pointer->mob_index_hash[iHash]	= pMobIndex;
	    top_mob_index++;
	    zone_pointer->top_mob++;
	}
    }

    return;
}



/*
 * Load an obj section.
 */
void load_objects( AREA_DATA *tarea, FILE *fp )
{
    OBJ_INDEX_DATA *pObjIndex=NULL;
    char letter;
    char *ln;
    int x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11;

    if ( !tarea )
    {
	bug( "Load_objects: no #AREA seen yet." );
	if ( fBootDb )
	{
	  shutdown_mud( "No #AREA" );
	  exit( 1 );
	}
	else
	  return;
    }

    for ( ; ; )
    {
	char buf[MAX_STRING_LENGTH];
	int vnum;
	int iHash;
	bool tmpBootDb;
	bool oldobj;

	letter				= fread_letter( fp );
	if ( letter != '#' )
	{
	    bug( "Load_objects: # not found." );
	    if ( fBootDb )
	    {
		shutdown_mud( "# not found" );
		exit( 1 );
	    }
	    else
		return;
	}

	vnum				= fread_number( fp );
	if ( vnum == 0 )
	    break;

	tmpBootDb = fBootDb;
	fBootDb = FALSE;
	if ( get_obj_index( vnum,(int)tarea->zone->number ) )
	{
	    if ( tmpBootDb )
	    {
		bug( "Load_objects: vnum %d duplicated.", vnum );
		shutdown_mud( "duplicate vnum" );
		exit( 1 );
	    }
	    else
	    {
		pObjIndex = get_obj_index( vnum,(int)tarea->zone->number);
		sprintf( buf, "Cleaning object: %d", vnum );
		log_string_plus( buf, LOG_BUILD, sysdata.log_level );
		clean_obj( pObjIndex );
		oldobj = TRUE;
	    }
	}
	else
	{
	  oldobj = FALSE;
	  pObjIndex=NULL;
	  CREATE( pObjIndex, OBJ_INDEX_DATA, 1 );
	}
	fBootDb = tmpBootDb;

	pObjIndex->vnum			= vnum;
	if ( fBootDb )
	{
	  if ( !tarea->low_o_vnum )
	    tarea->low_o_vnum		= vnum;
	  if ( vnum > tarea->hi_o_vnum )
	    tarea->hi_o_vnum		= vnum;
	}
	pObjIndex->name			= fread_string( fp );
	pObjIndex->short_descr		= fread_string( fp );
	pObjIndex->description		= fread_string( fp );
	pObjIndex->action_desc		= fread_string( fp );

	/* Modification Tracking --GW */
	if ( tarea->version_number > 1 )
	{
	 pObjIndex->modified_by		= fread_string( fp );
	 pObjIndex->modified_date	= fread_string( fp );
	}
	else
	{
	 pObjIndex->modified_by		= STRALLOC("Unknown");
	 pObjIndex->modified_date	= STRALLOC("Unknown");
	}

        /* Commented out by Narn, Apr/96 to allow item short descs like 
           Bonecrusher and Oblivion */
	/*pObjIndex->short_descr[0]	= LOWER(pObjIndex->short_descr[0]);*/
	pObjIndex->description[0]	= UPPER(pObjIndex->description[0]);
        pObjIndex->area			= tarea;

	ln = fread_line( fp );
	x1=x2=x3=x4=x5=x6=x7=x8=x9=x10=x11=0;
	sscanf( ln, "%d %d %d %d %d %d %d %d %d %d %d",
		&x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8, &x9, &x10, &x11 );
	pObjIndex->item_type		= x1;
	pObjIndex->extra_flags		= x2;
	pObjIndex->wear_flags		= x3;
        pObjIndex->level		= x4;
	pObjIndex->minlevel		= x5;
	pObjIndex->second_flags		= x6;
	/* Still in the file format .. not used tho .. maybe recylye them? =) --GW*/
	pObjIndex->loaded		= 0;
	pObjIndex->limit		= 0;
        pObjIndex->has_moved		= 0;
	/* ------------------------------------ */
        pObjIndex->ego			= x10;
	/* Make SURE Layers is LAST loaded! --GW */
        pObjIndex->layers		= x11;

	ln = fread_line( fp );
	x1=x2=x3=x4=x5=x6=0;
	sscanf( ln, "%d %d %d %d %d %d",
		&x1, &x2, &x3, &x4, &x5, &x6 );
	pObjIndex->value[0]		= x1;
	pObjIndex->value[1]		= x2;
	pObjIndex->value[2]		= x3;
	pObjIndex->value[3]		= x4;
	pObjIndex->value[4]		= x5;
	pObjIndex->value[5]		= x6;

	/* Fix a DT Crasher (Damage Type) --GW */
	if ( pObjIndex->item_type == ITEM_WEAPON )
	{
	  if ( pObjIndex->value[3] > DAM_PEA )
	  {
	     bug("Weapon: %d had bad Damage Type! Setting to hit!",pObjIndex->vnum);
	     pObjIndex->value[3] = DAM_HIT;
          }
	}

	pObjIndex->weight		= fread_number( fp );
	pObjIndex->weight = UMAX( 1, pObjIndex->weight );
	pObjIndex->cost			= fread_number( fp );
	pObjIndex->rent		  	= fread_number( fp );
	for ( ; ; )
	{
	    letter = fread_letter( fp );

	    if ( letter == 'A' )
	    {
		AFFECT_DATA *paf=NULL;

		CREATE( paf, AFFECT_DATA, 1 );
		paf->type		= -1;
		paf->duration		= -1;
		paf->location		= fread_number( fp );
		if ( paf->location == APPLY_WEAPONSPELL
		||   paf->location == APPLY_WEARSPELL
		||   paf->location == APPLY_REMOVESPELL
		||   paf->location == APPLY_STRIPSN )
		  paf->modifier		= slot_lookup( fread_number(fp) );
		else
		  paf->modifier		= fread_number( fp );
		xCLEAR_BITS(paf->bitvector);
		LINK( paf, pObjIndex->first_affect, pObjIndex->last_affect,
			   next, prev );
		top_affect++;
	    }

	    else if ( letter == 'E' )
	    {
		EXTRA_DESCR_DATA *ed=NULL;

		CREATE( ed, EXTRA_DESCR_DATA, 1 );
		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		LINK( ed, pObjIndex->first_extradesc, pObjIndex->last_extradesc,
			  next, prev );
		top_ed++;
	    }

	    else if ( letter == '>' )
	    {
	        ungetc( letter, fp );
	        oprog_read_programs( fp, pObjIndex );
	    }

	    else
	    {
		ungetc( letter, fp );
		break;
	    }
	}

	/*
	 * Translate spell "slot numbers" to internal "skill numbers."
	 */
	switch ( pObjIndex->item_type )
	{
	case ITEM_PILL:
	case ITEM_POTION:
	case ITEM_SCROLL:
	    pObjIndex->value[1] = slot_lookup( pObjIndex->value[1] );
	    pObjIndex->value[2] = slot_lookup( pObjIndex->value[2] );
	    pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
	    break;

	case ITEM_STAFF:
	case ITEM_WAND:
	    pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
	    break;
	case ITEM_SALVE:
	    pObjIndex->value[4] = slot_lookup( pObjIndex->value[4] );
	    pObjIndex->value[5] = slot_lookup( pObjIndex->value[5] );
	    break;
	}

        /* Temp fix for area file fuckups, dont use --GW */
/*	pObjIndex->layers = 0;
        pObjIndex->second_flags = 0;
        pObjIndex->limit = 0;
        pObjIndex->loaded = 0;
        pObjIndex->ego = 0;
        pObjIndex->has_moved = 0;
        pObjIndex->minlevel = 0;*/
	if ( !oldobj )
	{
	  iHash			= vnum % MAX_KEY_HASH;
	  pObjIndex->next	= zone_pointer->obj_index_hash[iHash];
	  zone_pointer->obj_index_hash[iHash]	= pObjIndex;
	  top_obj_index++;
	  zone_pointer->top_obj++;
	}
    }

    return;
}



/*
 * Load a reset section.
 */
void load_resets( AREA_DATA *tarea, FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    bool not01 = FALSE;
    int count = 0;
    int zone;

    if ( !tarea )
    {
	bug( "Load_resets: no #AREA seen yet." );
	if ( fBootDb )
	{
	  shutdown_mud( "No #AREA" );
	  exit( 1 );
	}
	else
	  return;
    }

     if ( tarea->zone )
     zone = (int)tarea->zone->number;
     else
     zone = 1;

    if ( tarea->first_reset )
    {
	if ( fBootDb )
	{
	  RESET_DATA *rtmp;

	  bug( "load_resets: WARNING: resets already exist for this area." );
	  for ( rtmp = tarea->first_reset; rtmp; rtmp = rtmp->next )
		++count;
	}
	else
	{
	 /*
	  * Clean out the old resets
	  */
	  sprintf( buf, "Cleaning resets: %s", tarea->name );
	  log_string_plus( buf, LOG_BUILD, sysdata.log_level );
	  clean_resets( tarea );
	}	
    }

    for ( ; ; )
    {
	ROOM_INDEX_DATA *pRoomIndex;
	EXIT_DATA *pexit;
	char letter;
	int extra, arg1, arg2, arg3;

	if ( ( letter = fread_letter( fp ) ) == 'S' )
	    break;

	if ( letter == '*' )
	{
	    fread_to_eol( fp );
	    continue;
	}

	extra	= fread_number( fp );
	arg1	= fread_number( fp );
	arg2	= fread_number( fp );
	arg3	= (letter == 'G' || letter == 'R')
		  ? 0 : fread_number( fp );
		  fread_to_eol( fp );

	++count;

	/*
	 * Validate parameters.
	 * We're calling the index functions for the side effect.
	 */
	switch ( letter )
	{
	default:
	    bug( "Load_resets: bad command '%c'.", letter );
	    if ( fBootDb )
	      boot_log( "Load_resets: %s (%d) bad command '%c'.", tarea->filename, count, letter );
	    return;

	case 'M':
	    if ( get_mob_index( arg1,zone ) == NULL && fBootDb )
		boot_log( "Load_resets: %s (%d) 'M': mobile %d doesn't exist.",
		    tarea->filename, count, arg1 );
	    if ( get_room_index( arg3,zone ) == NULL && fBootDb )
		boot_log( "Load_resets: %s (%d) 'M': room %d doesn't exist.",
		    tarea->filename, count, arg3 );
	    break;

	case 'O':
	    if ( get_obj_index(arg1,zone) == NULL && !fBootDb )
		boot_log( "Load_resets: %s (%d) '%c': object %d doesn't exist.",
		    tarea->filename, count, letter, arg1 );
	    if ( get_room_index(arg3,zone) == NULL && !fBootDb )
		boot_log( "Load_resets: %s (%d) '%c': room %d doesn't exist.",
		    tarea->filename, count, letter, arg3 );
	    break;

	case 'P':
	    if ( get_obj_index(arg1,zone) == NULL && !fBootDb )
		boot_log( "Load_resets: %s (%d) '%c': object %d doesn't exist.",
		    tarea->filename, count, letter, arg1 );
	    if ( arg3 > 0 )
	    {
		if ( get_obj_index(arg3,zone) == NULL && !fBootDb )
		    boot_log( "Load_resets: %s (%d) 'P': destination object %d doesn't exist.",
			tarea->filename, count, arg3 );
	    }
	    else if ( extra > 1 )
	      not01 = TRUE;
	    break;

	case 'G':
	case 'E':
	    if ( get_obj_index(arg1,zone) == NULL && !fBootDb )
		boot_log( "Load_resets: %s (%d) '%c': object %d doesn't exist.",
		    tarea->filename, count, letter, arg1 );
	    break;

	case 'T':
	    break;

	case 'H':
	    if ( arg1 > 0 )
		if ( get_obj_index(arg1,zone) == NULL && !fBootDb )
		    boot_log( "Load_resets: %s (%d) 'H': object %d doesn't exist.",
			tarea->filename, count, arg1 );
	    break;

	case 'D':
	    pRoomIndex = get_room_index( arg1,zone );
	    if ( !pRoomIndex )
	    {
		bug( "Load_resets: 'D': room %d doesn't exist.", arg1 );
		bug( "Reset: %c %d %d %d %d", letter, extra, arg1, arg2,
		    arg3 );
		if ( fBootDb )
		   boot_log( "Load_resets: %s (%d) 'D': room %d doesn't exist.",
			tarea->filename, count, arg1 );
		break;
	    }

	    if ( arg2 < 0
	    ||   arg2 > MAX_DIR+1
	    || ( pexit = get_exit(pRoomIndex, arg2)) == NULL
	    || !IS_SET( pexit->exit_info, EX_ISDOOR ) )
	    {
		bug( "Load_resets: 'D': exit %d not door.", arg2 );
		bug( "Reset: %c %d %d %d %d", letter, extra, arg1, arg2,
		    arg3 );
		if ( fBootDb )
		   boot_log( "Load_resets: %s (%d) 'D': exit %d not door.",
			tarea->filename, count, arg2 );
	    }

	    if ( arg3 < 0 || arg3 > 2 )
	    {
		bug( "Load_resets: 'D': bad 'locks': %d.", arg3 );
		if ( fBootDb )
		  boot_log( "Load_resets: %s (%d) 'D': bad 'locks': %d.",
			tarea->filename, count, arg3 );
	    }
	    break;

	case 'R':
	    pRoomIndex = get_room_index( arg1, zone );
	    if ( !pRoomIndex && fBootDb )
		boot_log( "Load_resets: %s (%d) 'R': room %d doesn't exist.",
		    tarea->filename, count, arg1 );

	    if ( arg2 < 0 || arg2 > 6 )
	    {
		bug( "Load_resets: 'R': bad exit %d.", arg2 );
		if ( fBootDb )
		  boot_log( "Load_resets: %s (%d) 'R': bad exit %d.",
			tarea->filename, count, arg2 );
		break;
	    }

	    break;
	}

	/* finally, add the reset */
	add_reset( tarea, letter, extra, arg1, arg2, arg3 );
    }
    
    if ( !not01 )
      renumber_put_resets(tarea);

    return;
}



/*
 * Load a room section.
 */
void load_rooms( AREA_DATA *tarea, FILE *fp )
{
    ROOM_INDEX_DATA *pRoomIndex=NULL;
    char buf[MAX_STRING_LENGTH];
    char *ln;
    int zone;

    if ( !tarea )
    {
	bug( "Load_rooms: no #AREA seen yet." );
	shutdown_mud( "No #AREA" );
	exit( 1 );
    }

    if ( tarea->zone )
    zone = tarea->zone->number;
    else
    zone = 1;

    for ( ; ; )
    {
	int vnum;
	char letter;
	int door;
	int iHash;
	bool tmpBootDb;
	bool oldroom;
	int x1, x2, x3, x4, x5, x6, x7;

	letter				= fread_letter( fp );
	if ( letter != '#' )
	{
	    bug( "Load_rooms: # not found." );
	    if ( fBootDb )
	    {
		shutdown_mud( "# not found" );
		exit( 1 );
	    }
	    else
		return;
	}

	vnum				= fread_number( fp );
	if ( vnum == 0 )
	    break;

	tmpBootDb = fBootDb;
	fBootDb = FALSE;
	if ( get_room_index( vnum,zone ) != NULL )
	{
	    if ( tmpBootDb )
	    {
	      bug( "Load_rooms: vnum %d duplicated.", vnum );
	      shutdown_mud( "duplicate vnum" );
	      exit( 1 );
	    }
	    else
	    {
	      pRoomIndex = get_room_index( vnum,zone);
	      sprintf( buf, "Cleaning room: %d", vnum );
	      log_string_plus( buf, LOG_BUILD, sysdata.log_level );
	      clean_room( pRoomIndex );
	      oldroom = TRUE;
	    }
	}
	else
	{
	  oldroom = FALSE;
	  pRoomIndex=NULL;
	  CREATE( pRoomIndex, ROOM_INDEX_DATA, 1 );
	  pRoomIndex->first_person	= NULL;
	  pRoomIndex->last_person	= NULL;
	  pRoomIndex->first_content	= NULL;
	  pRoomIndex->last_content	= NULL;
	}

	fBootDb = tmpBootDb;
	pRoomIndex->area		= tarea;
	pRoomIndex->vnum		= vnum;
	pRoomIndex->first_extradesc	= NULL;
	pRoomIndex->last_extradesc	= NULL;
        pRoomIndex->room_cnt		= 0;

	if ( fBootDb )
	{
	  if ( !tarea->low_r_vnum )
	    tarea->low_r_vnum		= vnum;
	  if ( vnum > tarea->hi_r_vnum )
	    tarea->hi_r_vnum		= vnum;
	}
	pRoomIndex->name		= fread_string( fp );
	pRoomIndex->description		= fread_string( fp );
	pRoomIndex->music		= fread_string( fp );

	/* Modification Tracking --GW */
	if ( tarea->version_number > 1 )
	{
	 pRoomIndex->modified_by	= fread_string( fp );
	 pRoomIndex->modified_date	= fread_string( fp );
	}
	else
	{
	 pRoomIndex->modified_by	= STRALLOC( "Unknown" );
	 pRoomIndex->modified_date	= STRALLOC( "Unknown" );
	}
	 
	/* Area number			  fread_number( fp ); */
	ln = fread_line( fp );
	x1=x2=x3=x4=x5=x6=x7=0;
	sscanf( ln, "%d %d %d %d %d %d %d",
	      &x1, &x2, &x3, &x4, &x5, &x6, &x7 );

	pRoomIndex->room_flags		= x2;
	pRoomIndex->sector_type		= x3;
        pRoomIndex->level               = x4;

	if ( pRoomIndex->level > LEVEL_HYPERION )
	pRoomIndex->level = LEVEL_HYPERION;

	pRoomIndex->tele_vnum		= x5;
	pRoomIndex->tele_delay		= x6;
	pRoomIndex->tunnel		= x7;

	if (pRoomIndex->sector_type < 0 || pRoomIndex->sector_type == SECT_MAX)
	{
	  bug( "Fread_rooms: vnum %d has bad sector_type %d.", vnum ,
	      pRoomIndex->sector_type);
	  pRoomIndex->sector_type = 1;
	}
	pRoomIndex->light		= 0;
	pRoomIndex->first_exit		= NULL;
	pRoomIndex->last_exit		= NULL;

	for ( ; ; )
	{
	    letter = fread_letter( fp );

	    if ( letter == 'S' )
		break;

	    if ( letter == 'D' )
	    {
		EXIT_DATA *pexit;
		int locks;

		door = fread_number( fp );
		if ( door < 0 || door > 10 )
		{
		    bug( "Fread_rooms: vnum %d has bad door number %d.", vnum,
		        door );
		    if ( fBootDb )
		      exit( 1 );
		}
		else
		{
		  pexit = make_exit( pRoomIndex, NULL, door );
		  pexit->description	= fread_string( fp );
		  pexit->keyword	= fread_string( fp );
		  pexit->exit_info	= 0;
		  ln = fread_line( fp );
		  x1=x2=x3=x4=0;
		  sscanf( ln, "%d %d %d %d",
		      &x1, &x2, &x3, &x4 );

		  locks			= x1;
		  pexit->key		= x2;
		  pexit->vnum		= x3;
		  pexit->vdir		= door;
		  pexit->distance	= x4;

		  switch ( locks )
		  {
		    case 1:  pexit->exit_info = EX_ISDOOR;                break;
		    case 2:  pexit->exit_info = EX_ISDOOR | EX_PICKPROOF; break;
		    default: pexit->exit_info = locks;
		  }
		}
	    }
	    else if ( letter == 'E' )
	    {
		EXTRA_DESCR_DATA *ed=NULL;

		CREATE( ed, EXTRA_DESCR_DATA, 1 );
		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		LINK( ed, pRoomIndex->first_extradesc, pRoomIndex->last_extradesc,
			  next, prev );
		top_ed++;
	    }
	    else if ( letter == 'M' )    /* Disabled */
	    {
	      int vnum,x,y;
  	      char entry;

	      vnum = fread_number(fp);
	      x = fread_number(fp);
	      y = fread_number(fp);
	      entry = fread_letter(fp);
	    }

/*	    {
		MAP_DATA *map;
		MAP_INDEX_DATA *map_index;
		int i, j;

		CREATE( map, MAP_DATA, 1);
                map->vnum                     = fread_number( fp );
                map->x                        = fread_number( fp ); 
                map->y                        = fread_number( fp );
		map->entry		      = fread_letter( fp );
		
                pRoomIndex->map               = map;
		if(  (map_index = get_map_index(map->vnum)) == NULL  )
		{
                     CREATE( map_index, MAP_INDEX_DATA, 1);
		     map_index->vnum = map->vnum;
		     map_index->next = first_map;
                     first_map       = map_index;
		     for (i = 0; i <  49; i++) {
			     for (j = 0; j <  79; j++) {
			       map_index->map_of_vnums[i][j] = -1;
                             }
                     }
		}
		if( (map->y <0) || (map->y >48) )
		{
                    bug("Map y coord out of range.  Room %d\n\r", map->y);

		}
		if( (map->x <0) || (map->x >78) )
		{
                    bug("Map x coord out of range.  Room %d\n\r", map->x);

		}
		if(  (map->x >0) 
		   &&(map->x <80) 
		   &&(map->y >0) 
		   &&(map->y <48) )
		   map_index->map_of_vnums[map->y][map->x]=pRoomIndex->vnum;
            }
*/
	    else if ( letter == '>' )
	    {
	      ungetc( letter, fp );
	      rprog_read_programs( fp, pRoomIndex );
            }
	    else
	    {
		bug( "Load_rooms: vnum %d has flag '%c' not 'DES'.", vnum,
		    letter );
		shutdown_mud( "Room flag not DES" );
		exit( 1 );
	    }

	}

	if ( !oldroom )
	{
	  iHash			 = vnum % MAX_KEY_HASH;
	  pRoomIndex->next	 = zone_pointer->room_index_hash[iHash];
	  zone_pointer->room_index_hash[iHash] = pRoomIndex;
	  top_room++;
	  zone_pointer->top_room++;
	}
    }

    return;
}



/*
 * Load a shop section.
 */
void load_shops( AREA_DATA *tarea, FILE *fp )
{
    SHOP_DATA *pShop=NULL;

    for ( ; ; )
    {
	MOB_INDEX_DATA *pMobIndex;
	int iTrade;
	
	pShop=NULL;
	CREATE( pShop, SHOP_DATA, 1 );
	pShop->keeper		= fread_number( fp );
	if ( pShop->keeper == 0 )
	    break;
	for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
	    pShop->buy_type[iTrade]	= fread_number( fp );
	pShop->profit_buy	= fread_number( fp );
	pShop->profit_sell	= fread_number( fp );
	pShop->profit_buy	= URANGE( pShop->profit_sell+5, pShop->profit_buy, 1000 );
	pShop->profit_sell	= URANGE( 0, pShop->profit_sell, pShop->profit_buy-5 );
	pShop->open_hour	= fread_number( fp );
	pShop->close_hour	= fread_number( fp );
				  fread_to_eol( fp );
	pMobIndex		= get_mob_index(pShop->keeper,(int)tarea->zone->number );
	pMobIndex->pShop	= pShop;

	if ( !first_shop )
	    first_shop		= pShop;
	else
	    last_shop->next	= pShop;
	pShop->next		= NULL;
	pShop->prev		= last_shop;
	last_shop		= pShop;
	top_shop++;
    }
    return;
}

/*
 * Load a repair shop section.					-Thoric
 */
void load_repairs( AREA_DATA *tarea, FILE *fp )
{
    REPAIR_DATA *rShop=NULL;

    for ( ; ; )
    {
	MOB_INDEX_DATA *pMobIndex;
	int iFix;

	rShop=NULL;
	CREATE( rShop, REPAIR_DATA, 1 );
	rShop->keeper		= fread_number( fp );
	if ( rShop->keeper == 0 )
	    break;
	for ( iFix = 0; iFix < MAX_FIX; iFix++ )
	  rShop->fix_type[iFix] = fread_number( fp );
	rShop->profit_fix	= fread_number( fp );
	rShop->shop_type	= fread_number( fp );
	rShop->open_hour	= fread_number( fp );
	rShop->close_hour	= fread_number( fp );
				  fread_to_eol( fp );
	pMobIndex		= get_mob_index( rShop->keeper,(int)tarea->zone->number );
	pMobIndex->rShop	= rShop;

	if ( !first_repair )
	  first_repair		= rShop;
	else
	  last_repair->next	= rShop;
	rShop->next		= NULL;
	rShop->prev		= last_repair;
	last_repair		= rShop;
	top_repair++;
    }
    return;
}


/*
 * Load Vehicle Data
 */
void load_vehicles( AREA_DATA *tarea, FILE *fp )
{
char buf[MSL];

    for ( ; ; )
    {
	OBJ_INDEX_DATA *pObjIndex;
	char letter;

	switch ( letter = fread_letter( fp ) )
	{
	default:
	    bug( "Load_vehicles: letter '%c' not *MS.", letter );
	    exit( 1 );

	case 'S':
	    return;

	case '*':
	    break;

	case 'V':
	    /* Format for Vehicle saving is : 
             * V <vnum> <description>   <flags> --GW
             */
	    pObjIndex		      = get_obj_index	( fread_number ( fp ),tarea->zone->number );
	    if ( !pObjIndex )
	    {
	      bug("Load_vehicles: NULL Obj Index!",0);
	      continue;
	    }
	    sprintf(buf,"Loading Vehicle Data: #%d\n",pObjIndex->vnum );
	    log_string(buf);
	    CREATE( pObjIndex->vehicle, VEHICLE_DATA, 1 );
            pObjIndex->vehicle->contents        = NULL;
            pObjIndex->vehicle->serial_number	= 0;
	    pObjIndex->vehicle->in_room		= NULL;
	    pObjIndex->vehicle->vehicle_flags	= fread_number(fp);
	    pObjIndex->vehicle->obj_vnum        = pObjIndex->vnum;
	    break;
	}

	fread_to_eol( fp );
    }
}

/*
 * Load spec proc declarations.
 */
void load_specials( AREA_DATA *tarea, FILE *fp )
{
    for ( ; ; )
    {
	MOB_INDEX_DATA *pMobIndex;
	char letter;

	switch ( letter = fread_letter( fp ) )
	{
	default:
	    bug( "Load_specials: letter '%c' not *MS.", letter );
	    exit( 1 );

	case 'S':
	    return;

	case '*':
	    break;

	case 'M':
	    pMobIndex		= get_mob_index	( fread_number ( fp ),tarea->zone->number );
	    pMobIndex->spec_fun	= spec_lookup	( fread_word   ( fp ) );
	    if ( pMobIndex->spec_fun == 0 )
	    {
		bug( "Load_specials: 'M': vnum %d.", pMobIndex->vnum );
		exit( 1 );
	    }
	    break;
	}

	fread_to_eol( fp );
    }
}


/*
 * Load soft / hard area ranges.
 */
void load_ranges( AREA_DATA *tarea, FILE *fp )
{
    int x1, x2, x3, x4;
    char *ln;

    if ( !tarea )
    {
	bug( "Load_ranges: no #AREA seen yet." );
	shutdown_mud( "No #AREA" );
	exit( 1 );
    }

    for ( ; ; )
    {
	ln = fread_line( fp );

	if (ln[0] == '$')
	  break;

	x1=x2=x3=x4=0;
	sscanf( ln, "%d %d %d %d",
	      &x1, &x2, &x3, &x4 );

	tarea->low_soft_range = x1;
	tarea->hi_soft_range = x2;
	tarea->low_hard_range = x3;
	tarea->hi_hard_range = x4;
    }
    return;

}              

/*
 * Go through all areas, and set up initial economy based on mob
 * levels and gold
 */
void initialize_economy( void )
{
    AREA_DATA *tarea;
    MOB_INDEX_DATA *mob;
    int idx, gold, rng;

    for ( tarea = first_area; tarea; tarea = tarea->next )
    {
	/* skip area if they already got some gold */
	if ( tarea->high_economy > 0 || tarea->low_economy > 10000 )
	  continue;
	rng = tarea->hi_soft_range - tarea->low_soft_range;
	if ( rng )
	  rng /= 2;
	else
	  rng = 25;
	gold = rng * rng * 50000;
	boost_economy( tarea, gold );
	for ( idx = tarea->low_m_vnum; idx < tarea->hi_m_vnum; idx++ )
	    if ( (mob=get_mob_index(idx,(int)tarea->zone->number)) != NULL)
		boost_economy( tarea, mob->gold * 10 );
    }
}

/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 * Check for bad reverse exits.
 */
void fix_exits( void )
{
    ROOM_INDEX_DATA *pRoomIndex;
    EXIT_DATA *pexit, *pexit_next, *rev_exit;
    int iHash;
    ZONE_DATA *in_zone;

for ( in_zone = first_zone; in_zone; in_zone = in_zone->next )
{
    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
	for ( pRoomIndex  = in_zone->room_index_hash[iHash];
	      pRoomIndex;
	      pRoomIndex  = pRoomIndex->next )
	{
	    bool fexit;

	    fexit = FALSE;
	    for ( pexit = pRoomIndex->first_exit; pexit; pexit = pexit_next )
	    {
		pexit_next = pexit->next;
		pexit->rvnum = pRoomIndex->vnum;
		if ( pexit->vnum <= 0 ||(pexit->to_room=get_room_index(pexit->vnum,(int)in_zone->number))== NULL)
		{
		    if ( fBootDb )
			boot_log( "Fix_exits: room %d, exit %s leads to bad vnum (%d)",
				pRoomIndex->vnum, dir_name[pexit->vdir], pexit->vnum );
		    
		    bug( "Deleting %s exit in room %d", dir_name[pexit->vdir],
				pRoomIndex->vnum );
		    extract_exit( pRoomIndex, pexit );
		}
		else
		  fexit = TRUE;
	    }
	    if ( !fexit )
	      SET_BIT( pRoomIndex->room_flags, ROOM_NO_MOB );
	}
    }

    /* Set all the rexit pointers 	-Thoric */
    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
	for ( pRoomIndex  = in_zone->room_index_hash[iHash];
	      pRoomIndex;
	      pRoomIndex  = pRoomIndex->next )
	{
	    for ( pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next )
	    {
		if ( pexit->to_room && !pexit->rexit )
		{
		   rev_exit = get_exit_to( pexit->to_room, rev_dir[pexit->vdir], pRoomIndex->vnum );
		   if ( rev_exit )
		   {
			pexit->rexit	= rev_exit;
			rev_exit->rexit	= pexit;
		   }
		}
	    }
	}
    }
}

    return;
}


/*
 * Get diku-compatable exit by number				-Thoric
 */
EXIT_DATA *get_exit_number( ROOM_INDEX_DATA *room, int xit )
{
    EXIT_DATA *pexit;
    int count;

    count = 0;
    for ( pexit = room->first_exit; pexit; pexit = pexit->next )
	if ( ++count == xit )
	  return pexit;
    return NULL;
}

/*
 * (prelude...) This is going to be fun... NOT!
 * (conclusion) QSort is f*cked!
 */
int exit_comp( EXIT_DATA **xit1, EXIT_DATA **xit2 )
{
    int d1, d2;

    d1 = (*xit1)->vdir;
    d2 = (*xit2)->vdir;

    if ( d1 < d2 )
      return -1;
    if ( d1 > d2 )
      return 1;
    return 0;
}

void sort_exits( ROOM_INDEX_DATA *room )
{
    EXIT_DATA *pexit; /* *texit */ /* Unused */
    EXIT_DATA *exits[MAX_REXITS];
    int x, nexits;

    nexits = 0;
    for ( pexit = room->first_exit; pexit; pexit = pexit->next )
    {
	exits[nexits++] = pexit;
	if ( nexits > MAX_REXITS )
	{
	    bug( "sort_exits: more than %d exits in room... fatal", nexits );
	    return;
	}
    }
    qsort( &exits[0], nexits, sizeof( EXIT_DATA * ),
		(int(*)(const void *, const void *)) exit_comp );
    for ( x = 0; x < nexits; x++ )
    {
	if ( x > 0 )
	  exits[x]->prev	= exits[x-1];
	else
	{
	  exits[x]->prev	= NULL;
	  room->first_exit	= exits[x];
	}
	if ( x >= (nexits - 1) )
	{
	  exits[x]->next	= NULL;
	  room->last_exit	= exits[x];
	}
	else
	  exits[x]->next	= exits[x+1];
    }
}

void randomize_exits( ROOM_INDEX_DATA *room, sh_int maxdir )
{
    EXIT_DATA *pexit;
    int nexits, /* maxd, */ d0, d1, count, door; /* Maxd unused */
    int vdirs[MAX_REXITS];

    nexits = 0;
    for ( pexit = room->first_exit; pexit; pexit = pexit->next )
       vdirs[nexits++] = pexit->vdir;

    for ( d0 = 0; d0 < nexits; d0++ )
    {
	if ( vdirs[d0] > maxdir )
	  continue;
	count = 0;
	while ( vdirs[(d1 = number_range( d0, nexits - 1 ))] > maxdir
	||      ++count > 5 );
	if ( vdirs[d1] > maxdir )
	  continue;
	door		= vdirs[d0];
	vdirs[d0]	= vdirs[d1];
	vdirs[d1]	= door;
    }
    count = 0;
    for ( pexit = room->first_exit; pexit; pexit = pexit->next )
       pexit->vdir = vdirs[count++];

    sort_exits( room );
}


/*
 * Repopulate areas periodically.
 */
void area_update( void )
{
    AREA_DATA *pArea;

    for ( pArea = first_area; pArea; pArea = pArea->next )
    {
	CHAR_DATA *pch;
	int reset_age = pArea->reset_frequency ? pArea->reset_frequency : 15;

	if ( (reset_age == -1 && pArea->age == -1)
	||    ++pArea->age < (reset_age-1) )
	    continue;

	if ( !IS_SET( pArea->flags, AFLAG_STARTED ) && 
	     !IS_SET( pArea->flags, AFLAG_ALWAYS_START ) )
		continue;

	if ( IS_SET( pArea->flags, AFLAG_NEVER_START ) )
	continue;

	/*
	 * Check for PC's.
	 */
	if ( pArea->nplayer > 0 && pArea->age == (reset_age-1) )
	{
	    char buf[MAX_STRING_LENGTH];

	    /* Rennard */
	    if ( pArea->resetmsg )
		sprintf( buf, "%s\n\r", pArea->resetmsg );
	    else
		strcpy( buf, "You hear some squeaking sounds...\n\r" );
	    for ( pch = first_char; pch; pch = pch->next )
	    {
		if ( !IS_NPC(pch)
		&&   IS_AWAKE(pch)
		&&   pch->in_room
		&&   pch->in_room->area == pArea )
		{
		    set_char_color( AT_RESET, pch );
		    send_to_char( buf, pch );
		}
	    }
	}

	/*
	 * Check age and reset.
	 * Note: Mud Academy resets every 3 minutes (not 15).
	 */
	if ( pArea->nplayer == 0 || pArea->age >= reset_age )
	{
	    ROOM_INDEX_DATA *pRoomIndex;

	    if ( IS_SET( pArea->flags, AFLAG_ALWAYS_START ) )
	    fprintf( stderr, "AUTO-AREA-RESET: %s\n", pArea->filename );
	   else	    
	    fprintf( stderr, "RUNTIME-RESET: %s\n", pArea->filename );

	    reset_area( pArea );

	    if ( reset_age == -1 )
		pArea->age = -1;
	    else
		pArea->age = number_range( 0, reset_age / 5 );
	    pRoomIndex = get_room_index( ROOM_VNUM_SCHOOL,1 );
	    if ( pRoomIndex != NULL && pArea == pRoomIndex->area
	    &&   pArea->reset_frequency == 0 )
		pArea->age = 15 - 3;
	}
    }
    return;
}


/*
 * Create an instance of a mobile.
 */
CHAR_DATA *create_mobile( MOB_INDEX_DATA *pMobIndex, ZONE_DATA *zone )
{
    CHAR_DATA *mob=NULL;

    if ( !pMobIndex )
    {
	bug( "Create_mobile: NULL pMobIndex." );
	return NULL;
    }

    CREATE( mob, CHAR_DATA, 1 );
    clear_char( mob );
    mob->pIndexData		= pMobIndex;

    mob->name			= QUICKLINK( pMobIndex->player_name );
    mob->short_descr		= QUICKLINK( pMobIndex->short_descr );
    mob->long_descr		= QUICKLINK( pMobIndex->long_descr  );
    mob->description		= QUICKLINK( pMobIndex->description );
    mob->spec_fun		= pMobIndex->spec_fun;
    mob->mpscriptpos		= 0;
    mob->level			= number_fuzzy( pMobIndex->level );
    mob->act			= pMobIndex->act;
    mob->acttwo			= pMobIndex->acttwo;
    mob->smart			= pMobIndex->smart;
    mob->affected_by		= pMobIndex->affected_by;
    mob->alignment		= pMobIndex->alignment;
    mob->sex			= pMobIndex->sex;

    if ( !pMobIndex->ac )
      mob->armor		= pMobIndex->ac;
    else
      mob->armor		= interpolate( mob->level, 100, -100 );

    if ( !pMobIndex->hitnodice )
      mob->max_hit		= mob->level * 8 + number_range(
					mob->level * mob->level / 4,
					mob->level * mob->level );
    else
      mob->max_hit		= pMobIndex->hitnodice * number_range(1, pMobIndex->hitsizedice )
				      + pMobIndex->hitplus;
    mob->hit			= mob->max_hit;
    /* lets put things back the way they used to be! -Thoric */
    mob->damroll		= mob->level * 1;
    mob->hitroll		= mob->level * 1.5;
    mob->gold			= pMobIndex->gold;
    mob->exp			= pMobIndex->exp;
    mob->position		= pMobIndex->position;
    mob->defposition		= pMobIndex->defposition;
    mob->barenumdie		= pMobIndex->damnodice;
    mob->baresizedie		= pMobIndex->damsizedice;
    mob->mobthac0		= pMobIndex->mobthac0;
    mob->hitplus		= pMobIndex->hitplus;
    mob->damplus		= pMobIndex->damplus;

    mob->perm_str		= pMobIndex->perm_str;
    mob->perm_dex		= pMobIndex->perm_dex;
    mob->perm_wis		= pMobIndex->perm_wis;
    mob->perm_int		= pMobIndex->perm_int;
    mob->perm_con		= pMobIndex->perm_con;
    mob->perm_cha		= pMobIndex->perm_cha;
    mob->perm_lck 		= pMobIndex->perm_lck;
    mob->race			= pMobIndex->race;
    mob->class			= pMobIndex->class;
    mob->xflags			= pMobIndex->xflags;
    mob->saving_poison_death	= pMobIndex->saving_poison_death;
    mob->saving_wand		= pMobIndex->saving_wand;
    mob->saving_para_petri	= pMobIndex->saving_para_petri;
    mob->saving_breath		= pMobIndex->saving_breath;
    mob->saving_spell_staff	= pMobIndex->saving_spell_staff;
    mob->height			= pMobIndex->height;
    mob->weight			= pMobIndex->weight;
    mob->resistant		= pMobIndex->resistant;
    mob->immune			= pMobIndex->immune;
    mob->susceptible		= pMobIndex->susceptible;
    mob->attacks		= pMobIndex->attacks;
    mob->defenses		= pMobIndex->defenses;
    mob->numattacks		= pMobIndex->numattacks;
    mob->speaks			= pMobIndex->speaks;
    mob->speaking		= pMobIndex->speaking;

    /*
     * Insert in list.
     */
/* put it in the right list, thanks Thoric, --GW */

if ( fBootDb )
LINK(mob,pMobIndex->area->zone->first_mob,pMobIndex->area->zone->last_mob,next,prev);
else
LINK(mob,zone->first_mob,zone->last_mob, next,prev);

    pMobIndex->count++;
    nummobsloaded++;
    return mob;
}



/*
 * Create an instance of an object.
 */
OBJ_DATA *create_object( OBJ_INDEX_DATA *pObjIndex, int level,ZONE_DATA *zone )
{
    OBJ_DATA *obj=NULL;
    extern bool LOAD_LIMITED;

    if ( !pObjIndex )
    {
	bug( "Create_object: NULL pObjIndex." );
	return NULL;
    }

/*
 * Limits --GW
 */
if ( LOAD_LIMITED == TRUE )
{
    if ( IS_SET( pObjIndex->second_flags, ITEM2_LIMITED ) &&
	 !IS_SET( pObjIndex->extra_flags, ITEM_PROTOTYPE ) )
	{ 
	  if  ( obj_limit_check(pObjIndex)==TRUE ) 
  	    return NULL;
	    else
      	    adjust_limits( pObjIndex, 1, LOADED_ADJUST_UP );
	}
}

    CREATE( obj, OBJ_DATA, 1 );

    obj->pIndexData	= pObjIndex;
    obj->in_room	= NULL;
    obj->wear_loc	= -1;
/*    obj->deleted	= FALSE;*/

    if ( level > LEVEL_HYPERION )
     level = LEVEL_HYPERION;

    obj->level		= level;
    obj->count		= 1;
    cur_obj_serial = UMAX((cur_obj_serial + 1 ) & (BV30-1), 1);
    obj->serial = obj->pIndexData->serial = cur_obj_serial;

    obj->name		= QUICKLINK( pObjIndex->name 	 );
    obj->short_descr	= QUICKLINK( pObjIndex->short_descr );
    obj->description	= QUICKLINK( pObjIndex->description );
    obj->action_desc	= QUICKLINK( pObjIndex->action_desc );
    obj->item_type	= pObjIndex->item_type;
    obj->extra_flags	= pObjIndex->extra_flags;
    obj->second_flags	= pObjIndex->second_flags;
    obj->wear_flags	= pObjIndex->wear_flags;
    obj->minlevel	= pObjIndex->minlevel;
    obj->ego		= pObjIndex->ego;
    obj->value[0]	= pObjIndex->value[0];
    obj->value[1]	= pObjIndex->value[1];
    obj->value[2]	= pObjIndex->value[2];
    obj->value[3]	= pObjIndex->value[3];
    obj->value[4]	= pObjIndex->value[4];
    obj->value[5]	= pObjIndex->value[5];
    obj->weight		= pObjIndex->weight;
    obj->cost		= pObjIndex->cost;
    /*
    obj->cost		= number_fuzzy( 10 )
			* number_fuzzy( level ) * number_fuzzy( level );
     */

    /*
     * Mess with object properties.
     */
    switch ( obj->item_type )
    {
    default:
	bug( "Read_object: vnum %d bad type.", pObjIndex->vnum );
	bug( "------------------------>     ", obj->item_type );
	break;

    case ITEM_LIGHT:
    case ITEM_TREASURE:
    case ITEM_FURNITURE:
    case ITEM_TRASH:
    case ITEM_CONTAINER:
    case ITEM_DRINK_CON:
    case ITEM_KEY:
	break;
    case ITEM_FOOD:
	/*
	 * optional food condition (rotting food)		-Thoric
	 * value1 is the max condition of the food
	 * value4 is the optional initial condition
	 */
	if ( obj->value[4] )
	  obj->timer = obj->value[4];
	else
	  obj->timer = obj->value[1];
	break;
    case ITEM_BOAT:
    case ITEM_CORPSE_NPC:
    case ITEM_CORPSE_PC:
    case ITEM_FOUNTAIN:
    case ITEM_BLOOD:
    case ITEM_BLOODSTAIN:
    case ITEM_SCRAPS:
    case ITEM_PIPE:
    case ITEM_HERB_CON:
    case ITEM_HERB:
    case ITEM_INCENSE:
    case ITEM_FIRE:
    case ITEM_BOOK:
    case ITEM_SWITCH:
    case ITEM_LEVER:
    case ITEM_PULLCHAIN:
    case ITEM_BUTTON:
    case ITEM_DIAL:
    case ITEM_RUNE:
    case ITEM_RUNEPOUCH:
    case ITEM_MATCH:
    case ITEM_TRAP:
    case ITEM_MAP:
    case ITEM_PORTAL:
    case ITEM_PAPER:
    case ITEM_PEN:
    case ITEM_TINDER:
    case ITEM_LOCKPICK:
    case ITEM_SPIKE:
    case ITEM_DISEASE:
    case ITEM_OIL:
    case ITEM_FUEL:
   
    case ITEM_QUIVER:
    case ITEM_SHOVEL:
    case ITEM_LIFE_PROTECTION:
    case ITEM_INSIG:
    case ITEM_SHRINE:
    case ITEM_PK_BOOK:
    case ITEM_VEHICLE:
	break;

    case ITEM_SALVE:
	obj->value[3]	= number_fuzzy( obj->value[3] );
	break;

    case ITEM_SCROLL:
	obj->value[0]	= number_fuzzy( obj->value[0] );
	break;

    case ITEM_WAND:
    case ITEM_STAFF:
	obj->value[0]	= number_fuzzy( obj->value[0] );
	obj->value[1]	= number_fuzzy( obj->value[1] );
	obj->value[2]	= obj->value[1];
	break;

    case ITEM_MISSILE_WEAPON:
    case ITEM_PROJECTILE:
    case ITEM_WEAPON:
	if ( obj->value[1] && obj->value[2] )
	   obj->value[2] *= obj->value[1];
	else
	{
	   obj->value[1] = number_fuzzy( number_fuzzy( 1 * level / 4 + 2 ) );
	   obj->value[2] = number_fuzzy( number_fuzzy( 3 * level / 4 + 6 ) );
	}
	if (obj->value[0] == 0)
	   obj->value[0] = INIT_WEAPON_CONDITION;
	break;

    case ITEM_ARMOR:
	if ( obj->value[0] == 0 )
	  obj->value[0]	= number_fuzzy( level / 4 + 2 );
	if (obj->value[1] == 0)
	    obj->value[1] = obj->value[0];
	break;

    case ITEM_POTION:
    case ITEM_PILL:
	obj->value[0]	= number_fuzzy( number_fuzzy( obj->value[0] ) );
	break;

    case ITEM_MONEY:
	obj->value[0]	= obj->cost;
	break;
    }

/* put it in the right list, thanks Thoric, --GW */

    if ( fBootDb )
    LINK( obj,pObjIndex->area->zone->first_obj,pObjIndex->area->zone->last_obj,next, prev );
    else
    LINK( obj,zone->first_obj,zone->last_obj,next,prev);

    ++pObjIndex->count;
    ++numobjsloaded;
    ++physicalobjects;

    return obj;
}


/*
 * Clear a new character.
 */
void clear_char( CHAR_DATA *ch )
{
    ch->hunting			= NULL;
    ch->fearing			= NULL;
    ch->hating			= NULL;
    ch->name			= NULL;
    ch->short_descr		= NULL;
    ch->long_descr		= NULL;
    ch->description		= NULL;
    ch->next			= NULL;
    ch->prev			= NULL;
    ch->first_carrying		= NULL;
    ch->last_carrying		= NULL;
    ch->next_in_room		= NULL;
    ch->prev_in_room		= NULL;
    ch->fighting		= NULL;
    ch->switched		= NULL;
    ch->first_affect		= NULL;
    ch->last_affect		= NULL;
    ch->dest_buf		= NULL;
    ch->spare_ptr		= NULL;
    ch->mount			= NULL;
    xCLEAR_BITS(ch->affected_by);
    ch->armor			= 100;
    ch->position		= POS_STANDING;
    ch->practice		= 0;
    ch->hit			= 20;
    ch->max_hit			= 20;
    ch->mana			= 100;
    ch->max_mana		= 100;
    ch->move			= 100;
    ch->max_move		= 100;
    ch->height			= 72;
    ch->weight			= 180;
    ch->xflags			= 0;
    ch->race			= -1;
    ch->class			= -1;
    ch->class2			= -1;
    ch->advclass		= -1;
    ch->speaking		= LANG_COMMON;
    ch->speaks			= LANG_COMMON;
    ch->barenumdie		= 1;
    ch->baresizedie		= 4;
    ch->substate		= 0;
    ch->tempnum			= 0;
    ch->perm_str		= 13;
    ch->perm_dex		= 13;
    ch->perm_int		= 13;
    ch->perm_wis		= 13;
    ch->perm_cha		= 13;
    ch->perm_con		= 13;
    ch->perm_lck		= 13;
    ch->mod_str			= 0;
    ch->mod_dex			= 0;
    ch->mod_int			= 0;
    ch->mod_wis			= 0;
    ch->mod_cha			= 0;
    ch->mod_con			= 0;
    ch->mod_lck			= 0;
    ch->stun_rounds		= 0;		/* Not stunned to start */
 /*   ch->deleted			= FALSE;	Not Extracted --GW*/
    return;
}



/*
 * Free a character.
 */
void free_char( CHAR_DATA *ch )
{
    OBJ_DATA *obj;
    AFFECT_DATA *paf;
    TIMER *timer;
    MPROG_ACT_LIST *mpact, *mpact_next;
    NOTE_DATA *comments, *comments_next;
    sh_int pos=0;


    /* To Catch Stale Pointers --GW */
/*    if ( ch->deleted == TRUE )
    return;*/

    if ( !ch )
    {
      bug( "Free_char: null ch!" );
      return;
    }

    if ( ch->desc )
      bug( "Free_char: char still has descriptor." );

    while ( (obj = ch->last_carrying) != NULL )
	extract_obj( obj );

    while ( (paf = ch->last_affect) != NULL )
	affect_remove( ch, paf );

    while ( (timer = ch->first_timer) != NULL )
	extract_timer( ch, timer );
	
  if( ch->name )
    STRFREE( ch->name		);
  if( ch->short_descr )
    STRFREE( ch->short_descr	);
  if( ch->long_descr )
    STRFREE( ch->long_descr	);
  if( ch->description )
    STRFREE( ch->description	);

    stop_hunting( ch );
    stop_hating ( ch );
    stop_fearing( ch );
    free_fight  ( ch );

    if ( ch->pcdata )
    {

    for ( comments = ch->pcdata->comments; comments; comments = comments_next )
    {
	comments_next = comments->next;
	STRFREE( comments->text    );
	STRFREE( comments->to_list );
	STRFREE( comments->subject );
	STRFREE( comments->sender  );
	STRFREE( comments->date    );
	DISPOSE( comments          );
    }

      if ( ch->pcdata->pnote )
	free_note( ch->pcdata->pnote );
      if ( ch->pcdata->inter_editing )
        DISPOSE( ch->pcdata->inter_editing );
      if ( ch->pcdata->editor )
        stop_editing( ch );
      if ( ch->pcdata->gladiator )
        STRFREE( ch->pcdata->gladiator );

    /* Free thier maps --GW */
    if ( ch->pcdata->color_maps )
       free_char_colormaps(ch);

    /* Free Psocials --GW */
    if ( ch->pcdata->personal_socials )
       free_char_psocials(ch);

    /* Free there offline data */
    if ( ch->pcdata->offline )
	free_offline_data(ch);

    /* Free Site Locks */
    if ( ch->pcdata->first_host )
	free_hosts(ch);

     if ( ch->pcdata->clan_name )
	STRFREE( ch->pcdata->clan_name	);
     if ( ch->pcdata->council_name )
	STRFREE( ch->pcdata->council_name );
     if ( ch->pcdata->pwd )
        DISPOSE( ch->pcdata->pwd	);  /* no hash */
     if ( ch->pcdata->bamfin )
	DISPOSE( ch->pcdata->bamfin	);  /* no hash */
     if ( ch->pcdata->bamfout )
	DISPOSE( ch->pcdata->bamfout	);  /* no hash */
     if ( ch->pcdata->rank )
	DISPOSE( ch->pcdata->rank	);
     if ( ch->pcdata->title )
	STRFREE( ch->pcdata->title	);
     if ( ch->pcdata->bio )
	STRFREE( ch->pcdata->bio	); 
     if ( ch->pcdata->bestowments )
	DISPOSE( ch->pcdata->bestowments ); /* no hash */
     if ( ch->pcdata->homepage )
	DISPOSE( ch->pcdata->homepage	);  /* no hash */
     if ( ch->pcdata->authed_by )
	STRFREE( ch->pcdata->authed_by	);
     if ( ch->pcdata->prompt )
	STRFREE( ch->pcdata->prompt	);
     if ( ch->pcdata->subprompt )
        STRFREE( ch->pcdata->subprompt );
     if ( ch->pcdata->email )
        STRFREE( ch->pcdata->email );
     if ( ch->pcdata->guild_name )
        STRFREE( ch->pcdata->guild_name );
     if ( ch->pcdata->deity_name )
        STRFREE( ch->pcdata->deity_name );
     if ( ch->pcdata->stitle )
        STRFREE( ch->pcdata->stitle );
     if ( ch->pcdata->rreply )
        STRFREE( ch->pcdata->rreply );
     if ( ch->pcdata->rreply_name )
        STRFREE( ch->pcdata->rreply_name );
     if ( ch->pcdata->helled_by )
        STRFREE( ch->pcdata->helled_by );
     if ( ch->pcdata->spouse )
        STRFREE( ch->pcdata->spouse );
     if ( ch->pcdata->engaged )
        STRFREE( ch->pcdata->engaged );
     if ( ch->pcdata->challenge_char_name )
        STRFREE( ch->pcdata->challenge_char_name );
     if ( ch->pcdata->name )
        STRFREE( ch->pcdata->name );

     /* Aliases --GW */
     for( pos=0; pos < MAX_ALIAS; pos++ )
     {
	if ( ch->pcdata->alias[pos] != NULL )
	     STRFREE(ch->pcdata->alias[pos]);

	if ( ch->pcdata->alias_sub[pos] != NULL )
	     STRFREE(ch->pcdata->alias_sub[pos]);
     }

     /* Ignores --GW */
     for( pos=0; pos < MAX_IGNORE; pos++ )
     {
	if ( ch->pcdata->ignore[pos] != NULL )
	  STRFREE(ch->pcdata->ignore[pos]);
     }

     /* Com-Track --GW */
     for( pos=0; pos < MAX_COM; pos++ );
     {
	if( ch->pcdata->comtrack[pos] != NULL && ch->pcdata->comtrack[pos][0] != '\0' )
	  STRFREE(ch->pcdata->comtrack[pos]);

	if( ch->pcdata->lastcom[pos] != NULL && ch->pcdata->lastcom[pos][0] != '\0' )
	  STRFREE(ch->pcdata->lastcom[pos]);
     }

	if ( ch->pcdata->arena_viewing )
	  STRFREE( ch->pcdata->arena_viewing );

	if ( ch->pcdata )
	DISPOSE( ch->pcdata );
     }


    for ( mpact = ch->mpact; mpact; mpact = mpact_next )
    {
	mpact_next = mpact->next;
	DISPOSE( mpact->buf );
	DISPOSE( mpact	    );
    }

/*    ch->deleted = TRUE;*/
    DISPOSE( ch );
    return;
}



/*
 * Get an extra description from a list.
 */
char *get_extra_descr( const char *name, EXTRA_DESCR_DATA *ed )
{
    for ( ; ed; ed = ed->next )
	if ( is_name( name, ed->keyword ) )
	    return ed->description;

    return NULL;
}



/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup. -- Zone Support Added -- GW
 */
MOB_INDEX_DATA *get_mob_index( sh_int vnum, sh_int zone )
{
    MOB_INDEX_DATA *pMobIndex;
    ZONE_DATA *in_zone;

    if ( vnum < 0 )
      vnum = 0;
    if ( zone <= 0 )
      zone = 1;

/*find the right zone, and use that index -- GW*/
if ((in_zone = find_zone(zone))==NULL)
{
bug("Get_mob_index: ZONE ACCESS VIOLATION!! (*FATAL*)",0);
return NULL;
}

    for ( pMobIndex  = in_zone->mob_index_hash[vnum % MAX_KEY_HASH];
	  pMobIndex;
	  pMobIndex  = pMobIndex->next )
        if ( pMobIndex->vnum == vnum )
            return pMobIndex;

/*    if ( fBootDb ) <-- What an Annoyance. --GW
 *	bug( "Get_mob_index: bad vnum %d.", vnum );
 */
    return NULL;
}



/*
 * Translates obj virtual number to its obj index struct.
 * Hash table lookup. -- Zone Support Added -- GW
 */
OBJ_INDEX_DATA *get_obj_index( int vnum, int zone )
{
    OBJ_INDEX_DATA *pObjIndex;
    ZONE_DATA *in_zone;

    if ( vnum < 0 )
      vnum = 0;
    if ( zone <= 0 )
      zone = 1;


/*find the right zone, and use that index -- GW*/
if ((in_zone = find_zone(zone))==NULL)
{
bug("Get_obj_index: ZONE ACCESS VIOLATION!! (*FATAL*)",0);
return NULL;
}

    for ( pObjIndex  = in_zone->obj_index_hash[vnum % MAX_KEY_HASH];
	  pObjIndex;
	  pObjIndex  = pObjIndex->next )
	if ( pObjIndex->vnum == vnum )
	    return pObjIndex;

    return NULL;
}



/*
 * Translates room virtual number to its room index struct.
 * Hash table lookup. -- Zone Support Added -- GW
 */
ROOM_INDEX_DATA *get_room_index( int vnum, int zone )
{
    ROOM_INDEX_DATA *pRoomIndex;
    ZONE_DATA *in_zone;

    if ( vnum < 0 )
      vnum = 0;
    if ( zone <= 0 )
      zone = 1;


/*find the right zone, and use that index -- GW*/
if ((in_zone = find_zone(zone))==NULL)
{
bug("Get_room_index: ZONE ACCESS VIOLATION!! (*FATAL*)",0);
return NULL;
}

    for ( pRoomIndex  = in_zone->room_index_hash[vnum % MAX_KEY_HASH];
	  pRoomIndex;
	  pRoomIndex  = pRoomIndex->next )
	if ( pRoomIndex->vnum == vnum )
	    return pRoomIndex;

    return NULL;
}

ZONE_DATA *find_zone( int zone )
{
ZONE_DATA *in_zone;

if ( zone > top_zone )
zone = top_zone;
if ( zone <= 0 )
zone = 1;

/* search for the zone */
for( in_zone = first_zone; in_zone; in_zone = in_zone->next)
{
    if (in_zone->number == zone )
    { /* zone found, return the data */
      return in_zone;
    }
}

/* zone not found */

return find_zone(1);
}

/*
 * Added lots of EOF checks, as most of the file crashes are based on them.
 * If an area file encounters EOF, the fread_* functions will shutdown the
 * MUD, as all area files should be read in in full or bad things will
 * happen during the game.  Any files loaded in without fBootDb which
 * encounter EOF will return what they have read so far.   These files
 * should include player files, and in-progress areas that are not loaded
 * upon bootup.
 * -- Altrag
 */


/*
 * Read a letter from a file.
 */
char fread_letter( FILE *fp )
{
    char c;

    do
    {
        if ( feof(fp) )
        {
          bug("fread_letter: EOF encountered on read.\n\r");
          if ( fBootDb )
            exit(1);
          return '\0';
        }
	c = getc( fp );
    }
    while ( isspace(c) );

    return c;
}



/*
 * Read a number from a file.
 */
long fread_number( FILE *fp )
{
    long number;
    bool sign;
    char c;

    do
    {
        if ( feof(fp) )
        {
          bug("fread_number: EOF encountered on read.\n\r");
          if ( fBootDb )
            exit(1);
          return 0;
        }
	c = getc( fp );
    }
    while ( isspace(c) );

    number = 0;

    sign   = FALSE;
    if ( c == '+' )
    {
	c = getc( fp );
    }
    else if ( c == '-' )
    {
	sign = TRUE;
	c = getc( fp );
    }

    if ( c == '\0' )
       return 0;

    if ( !isdigit(c) )
    {
	bug( "Fread_number: bad format. (%c)", c );
	if ( fBootDb )
	  exit( 1 );
	return 0;
    }

    while ( isdigit(c) )
    {
        if ( feof(fp) )
        {
          bug("fread_number: EOF encountered on read.\n\r");
          if ( fBootDb )
            exit(1);
          return number;
        }
	number = number * 10 + c - '0';
	c      = getc( fp );
    }

    if ( sign )
	number = 0 - number;

    if ( c == '|' )
	number += fread_number( fp );
    else if ( c != ' ' )
	ungetc( c, fp );

    return number;
}


/*
 * custom str_dup using create					-Thoric
 */
char *str_dup( char const *str )
{
    char *ret=NULL;
    int len=0;

    if ( !str )
	return NULL;
    
    len = strlen(str)+1;

    CREATE( ret, char, len );
    strcpy( ret, str );
    return ret;
}

/*
 * Read a string from file fp
 */
char *fread_string( FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *plast;
    char c;
    int ln;

    plast = buf;
    buf[0] = '\0';
    ln = 0;

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
	if ( feof(fp) )
	{
	    bug("fread_string: EOF encountered on read.\n\r");
	    if ( fBootDb )
		exit(1);
	    return STRALLOC("");
	}
	c = getc( fp );
    }
    while ( isspace(c) );

    if ( ( *plast++ = c ) == '~' )
	return STRALLOC( "" );

    for ( ;; )
    {
	if ( ln >= (MAX_STRING_LENGTH - 1) )
	{
	     bug( "fread_string: string too long" );
	     *plast = '\0';
	     return STRALLOC( buf );
	}
	switch ( *plast = getc( fp ) )
	{
	default:
	    plast++; ln++;
	    break;

	case EOF:
	    bug( "Fread_string: EOF" );
	    if ( fBootDb )
	      exit( 1 );
	    *plast = '\0';
	    return STRALLOC(buf);
	    break;

	case '\n':
	    plast++;  ln++;
	    *plast++ = '\r';  ln++;
	    break;

	case '\r':
	    break;

	case '~':
	    *plast = '\0';
	    return STRALLOC( buf );
	}
    }
}

/*
 * Read a string from file fp using str_dup (ie: no string hashing)
 */
char *fread_string_nohash( FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *plast;
    char c;
    int ln;

    plast = buf;
    buf[0] = '\0';
    ln = 0;

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
	if ( feof(fp) )
	{
	    bug("fread_string_no_hash: EOF encountered on read.\n\r");
	    if ( fBootDb )
		exit(1);
	    return str_dup("");
	}
	c = getc( fp );
    }
    while ( isspace(c) );

    if ( ( *plast++ = c ) == '~' )
	return str_dup( "" );

    for ( ;; )
    {
	if ( ln >= (MAX_STRING_LENGTH - 1) )
	{
	   bug( "fread_string_no_hash: string too long" );
	   *plast = '\0';
	   return str_dup( buf );
	}
	switch ( *plast = getc( fp ) )
	{
	default:
	    plast++; ln++;
	    break;

	case EOF:
	    bug( "Fread_string_no_hash: EOF" );
	    if ( fBootDb )
	      exit( 1 );
	    *plast = '\0';
	    return str_dup(buf);
	    break;

	case '\n':
	    plast++;  ln++;
	    *plast++ = '\r';  ln++;
	    break;

	case '\r':
	    break;

	case '~':
	    *plast = '\0';
	    return str_dup( buf );
	}
    }
}



/*
 * Read to end of line (for comments).
 */
void fread_to_eol( FILE *fp )
{
    char c;

    do
    {
	if ( feof(fp) )
	{
	    bug("fread_to_eol: EOF encountered on read.\n\r");
	    if ( fBootDb )
		exit(1);
	    return;
	}
	c = getc( fp );
    }
    while ( c != '\n' && c != '\r' );

    do
    {
	c = getc( fp );
    }
    while ( c == '\n' || c == '\r' );

    ungetc( c, fp );
    return;
}

/*
 * Read to end of line into static buffer			-Thoric
 */
char *fread_line( FILE *fp )
{
    static char line[MAX_STRING_LENGTH];
    char *pline;
    char c;
    int ln;

    pline = line;
    line[0] = '\0';
    ln = 0;

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
	if ( feof(fp) )
	{
	    bug("fread_line: EOF encountered on read.\n\r");
	    if ( fBootDb )
		exit(1);
	    strcpy(line, "");
	    return line;
	}
	c = getc( fp );
    }
    while ( isspace(c) );

    ungetc( c, fp );
    do
    {
	if ( feof(fp) )
	{
	    bug("fread_line: EOF encountered on read.\n\r");
	    if ( fBootDb )
		exit(1);
	    *pline = '\0';
	    return line;
	}
	c = getc( fp );
	*pline++ = c; ln++;
	if ( ln >= (MAX_STRING_LENGTH - 1) )
	{
	    bug( "fread_line: line too long" );
	    break;
	}
    }
    while ( c != '\n' && c != '\r' );

    do
    {
	c = getc( fp );
    }
    while ( c == '\n' || c == '\r' );

    ungetc( c, fp );
    *pline = '\0';
    return line;
}



/*
 * Read one word (into static buffer).
 */
char *fread_word( FILE *fp )
{
    static char word[MAX_INPUT_LENGTH];
    char *pword;
    char cEnd;

    do
    {
	if ( feof(fp) )
	{
	    bug("fread_word: EOF encountered on read.\n\r");
	    if ( fBootDb )
		exit(1);
	    word[0] = '\0';
	    return word;
	}
	cEnd = getc( fp );
    }
    while ( isspace( cEnd ) );

    if ( cEnd == '\'' || cEnd == '"' )
    {
	pword   = word;
    }
    else
    {
	word[0] = cEnd;
	pword   = word+1;
	cEnd    = ' ';
    }

    for ( ; pword < word + MAX_INPUT_LENGTH; pword++ )
    {
	if ( feof(fp) )
	{
	    bug("fread_word: EOF encountered on read.\n\r");
	    if ( fBootDb )
		exit(1);
	    *pword = '\0';
	    return word;
	}
	*pword = getc( fp );
	if ( cEnd == ' ' ? isspace(*pword) : *pword == cEnd )
	{
	    if ( cEnd == ' ' )
		ungetc( *pword, fp );
	    *pword = '\0';
	    return word;
	}
    }

    bug( "Fread_word: word too long" );
    exit( 1 );
    return NULL;
}


void do_memory( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int hash;
    CHAR_DATA *wch;
    extern bool limits_off;
    extern bool LOAD_LIMITED;

    argument = one_argument( argument, arg );
    ch_printf( ch, "Affects %5d    Areas   %5d\n\r",  top_affect, top_area   );
    ch_printf( ch, "ExtDes  %5d    Exits   %5d\n\r", top_ed,	 top_exit   );
    ch_printf( ch, "Helps   %5d    Resets  %5d\n\r", top_help,   top_reset  );
    ch_printf( ch, "IdxMobs %5d    Mobs    %5d\n\r", top_mob_index, nummobsloaded );
    ch_printf( ch, "IdxObjs %5d    Objs    %5d (%d)\n\r", top_obj_index, numobjsloaded, physicalobjects );
    ch_printf( ch, "Rooms   %5d    VRooms  %5d\n\r", top_room,   top_vroom   );
    ch_printf( ch, "Shops   %5d    RepShps %5d\n\r", top_shop,   top_repair );
    ch_printf( ch, "CurOq's %5d    CurCq's %5d\n\r", cur_qobjs,  cur_qchars );
    ch_printf( ch, "Players %5d    Maxplrs %5d\n\r", num_descriptors, sysdata.maxplayers );
    ch_printf( ch, "MaxEver %5d    Topsn   %5d (%d)\n\r", sysdata.alltimemax, top_sn, MAX_SKILL );
    ch_printf( ch, "MaxEver time recorded at:   %s\n\r", sysdata.time_of_max );
    ch_printf( ch, "Limits: %s     LOAD_LIMITED: %s\n\r",
	limits_off ? "OFF" : "ON", LOAD_LIMITED ? "Alter Limits As Needed" : "Donot Alter Limits at this time." );
 
    if ( !str_cmp( arg, "check" ) )
    {
#ifdef HASHSTR
	send_to_char( check_hash(argument), ch );
#else
	send_to_char( "Hash strings not enabled.\n\r", ch );
#endif
	return;
    }
    if ( !str_cmp( arg, "showhigh" ) )
    {
#ifdef HASHSTR
	show_high_hash( atoi(argument) );
#else
	send_to_char( "Hash strings not enabled.\n\r", ch );
#endif
	return;
    }
    if ( argument[0] != '\0' )
      hash = atoi(argument);
    else
      hash = -1;
    if ( !str_cmp( arg, "hash" ) )
    {
#ifdef HASHSTR
	ch_printf( ch, "Hash statistics:\n\r%s", hash_stats() );
	if ( hash != -1 )
	  hash_dump( hash );
#else
	send_to_char( "Hash strings not enabled.\n\r", ch );
#endif
    }

    if ( !str_cmp( arg, "roomlist" ) )
    {
	send_to_char("Linked Char->in_room listing (ALL)\n\r",ch);
	
	if ( !ch->in_room || !ch->in_room->first_person )
	{
	  send_to_char("Fatal ERROR\n",ch);
	  return;
	}

	for( wch = ch->in_room->first_person; wch; wch = wch->next_in_room)
	{
	 if (!wch)
	 {
	 send_to_char("Warning: Null Wch in List.\n",ch);
	 break;
	 }
	if ( !wch->name )
	send_to_char("WARNING -- NULL CHAR IN LIST HERE!\n",ch);
	else
	ch_printf( ch, "Ch: %s Zone: %d Vnum: %d Master: %s Area: %s\n",wch->name,wch->in_room->area->zone->number,
		wch->in_room->vnum,wch->master, ch->in_room->area->filename);
	}
   }

    if ( !str_cmp( arg, "limitlist" ) )
    {
    extern LIMIT_DATA *first_limit;
    LIMIT_DATA *limit, *next_limit;
    char limit_buf[MSL];

     for( limit = first_limit; limit; limit = next_limit )
     {
	next_limit = limit->next;

      sprintf(limit_buf,"Z:%d Vnum: %d Loaded: %d Limit: %d\n\r",limit->zone,limit->vnum,
      limit->loaded, limit->limit);
      send_to_char(limit_buf,ch);
     }
    }

    if ( !str_cmp( arg, "charlist" ) )
    {
	send_to_char("Linked Char Listing (!NPC):\n\r",ch);

	for ( wch = first_char; wch; wch = wch->next )
	{
	if ( !wch )
	{
	send_to_char("Warning: Null Wch in list.\n",ch);
	continue;
	}
/*	if ( !IS_NPC(wch) )
	{*/
	if ( !wch->name )
	send_to_char("WARNING -- NULL CHAR IN LIST HERE!\n",ch);
	else
	ch_printf( ch, "%s\n",wch->name);
/*	}*/
        }
    }
    if ( !str_cmp(arg,"cleanpk"))
     pk_list_cleaner();

    if ( !str_cmp(arg,"overlimit") )
    {
	overlimit_msg();
	show_overlimit(ch);
    }
    if ( !str_cmp(arg,"inroomupdate"))
    inroom_update();


    return;
}



/*
 * Stick a little fuzz on a number.
 */
int number_fuzzy( int number )
{
    switch ( number_bits( 2 ) )
    {
	case 0:  number -= 1; break;
	case 3:  number += 1; break;
    }

    return UMAX( 1, number );
}



/*
 * Generate a random number.
 */
long number_range( int from, int to )
{
/*    int power;
    int number;*/

    if ( ( to = to - from + 1 ) <= 1 )
	return from;

/*    for ( power = 2; power < to; power <<= 1 )
	;

    while ( ( number = number_mm( ) & (power - 1) ) >= to )
	;

    return from + number;*/
    return (number_mm() % to) + from;
}



/*
 * Generate a percentile roll.
 */
int number_percent( void )
{
/*    int percent;

    while ( ( percent = number_mm( ) & (128-1) ) > 99 )
	;

    return 1 + percent;*/
    return number_mm() % 100;
}



/*
 * Generate a random door.
 */
int number_door( void )
{
    int door;

    while ( ( door = number_mm( ) & (16-1) ) > 9 )
	;

    return door;
/*    return number_mm() & 10; */
}



int number_bits( int width )
{
    return number_mm( ) & ( ( 1 << width ) - 1 );
}



/*
 * I've gotten too many bad reports on OS-supplied random number generators.
 * This is the Mitchell-Moore algorithm from Knuth Volume II.
 * Best to leave the constants alone unless you've read Knuth.
 * -- Furey
 */
static	int	rgiState[2+55];

void init_mm( )
{
    int *piState;
    int iState;

    piState	= &rgiState[2];

    piState[-2]	= 55 - 55;
    piState[-1]	= 55 - 24;

    piState[0]	= ((int) current_time) & ((1 << 30) - 1);
    piState[1]	= 1;
    for ( iState = 2; iState < 55; iState++ )
    {
	piState[iState] = (piState[iState-1] + piState[iState-2])
			& ((1 << 30) - 1);
    }
    return;
}



long number_mm( void )
{
    int *piState;
    long iState1;
    long iState2;
    long iRand;

    piState		= &rgiState[2];
    iState1	 	= piState[-2];
    iState2	 	= piState[-1];
    iRand	 	= (piState[iState1] + piState[iState2])
			& ((1 << 30) - 1);
    piState[iState1]	= iRand;
    if ( ++iState1 == 55 )
	iState1 = 0;
    if ( ++iState2 == 55 )
	iState2 = 0;
    piState[-2]		= iState1;
    piState[-1]		= iState2;
    return iRand >> 6;
}



/*
 * Roll some dice.						-Thoric
 */
int dice( int number, int size )
{
    int idice;
    int sum;

    switch ( size )
    {
      case 0: return 0;
      case 1: return number;
    }

    for ( idice = 0, sum = 0; idice < number; idice++ )
	sum += number_range( 1, size );

    return sum;
}



/*
 * Simple linear interpolation.
 */
int interpolate( int level, int value_00, int value_32 )
{
    return value_00 + level * (value_32 - value_00) / 32;
}


/*
 * Removes the tildes from a string.
 * Used for player-entered strings that go into disk files.
 */
void smash_tilde( char *str )
{
    for ( ; *str != '\0'; str++ )
	if ( *str == '~' )
	    *str = '-';

    return;
}

/*
 * Encodes the tildes in a string.				-Thoric
 * Used for player-entered strings that go into disk files.
 */
void hide_tilde( char *str )
{
    for ( ; *str != '\0'; str++ )
	if ( *str == '~' )
	    *str = HIDDEN_TILDE;

    return;
}

char *show_tilde( char *str )
{
    static char buf[MAX_STRING_LENGTH];
    char *bufptr;

    bufptr = buf;
    for ( ; *str != '\0'; str++, bufptr++ )
    {
	if ( *str == HIDDEN_TILDE )
	    *bufptr = '~';
	else
	    *bufptr = *str;
    }
    *bufptr = '\0';

    return buf;
}



/*
 * Compare strings, case insensitive.
 * Return TRUE if different
 *   (compatibility with historical functions).
 */
bool str_cmp( const char *astr, const char *bstr )
{
    if ( !astr )
    {
	bug( "Str_cmp: null astr." );
	if ( bstr )
	  fprintf( stderr, "str_cmp: astr: (null)  bstr: %s\n", bstr );
	return TRUE;
    }

    if ( !bstr )
    {
	bug( "Str_cmp: null bstr." );
        bug( astr );
	if ( astr )
	  fprintf( stderr, "str_cmp: astr: %s  bstr: (null)\n", astr );
	return TRUE;
    }

    for ( ; *astr || *bstr; astr++, bstr++ )
    {
	if ( LOWER(*astr) != LOWER(*bstr) )
	    return TRUE;
    }

    return FALSE;
}



/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix( const char *astr, const char *bstr )
{
    if ( !astr )
    {
	bug( "Strn_cmp: null astr." );
	return TRUE;
    }

    if ( !bstr )
    {
	bug( "Strn_cmp: null bstr." );
	return TRUE;
    }

    for ( ; *astr; astr++, bstr++ )
    {
	if ( LOWER(*astr) != LOWER(*bstr) )
	    return TRUE;
    }

    return FALSE;
}



/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns TRUE is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool str_infix( const char *astr, const char *bstr )
{
    int sstr1;
    int sstr2;
    int ichar;
    char c0;

    if ( ( c0 = LOWER(astr[0]) ) == '\0' )
	return FALSE;

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);

    for ( ichar = 0; ichar <= sstr2 - sstr1; ichar++ )
	if ( c0 == LOWER(bstr[ichar]) && !str_prefix( astr, bstr + ichar ) )
	    return FALSE;

    return TRUE;
}



/*
 * Compare strings, case insensitive, for suffix matching.
 * Return TRUE if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool str_suffix( const char *astr, const char *bstr )
{
    int sstr1;
    int sstr2;

     if ( !astr || !bstr )
       return FALSE;

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);
    if ( sstr1 <= sstr2 && !str_cmp( astr, bstr + sstr2 - sstr1 ) )
	return FALSE;
    else
	return TRUE;
}



/*
 * Returns an initial-capped string.
 */
char *capitalize( char *str )
{
    static char strcap[MAX_STRING_LENGTH];
    int i=0;
    record_call("<capitalize>");

    strcap[0] = '\0';

    if ( str == NULL )
    return NULL;

    for ( i = 0; str[i] != '\0'; i++ )
	strcap[i] = LOWER(str[i]);
    strcap[i] = '\0';
    strcap[0] = UPPER(strcap[0]);
    return strcap;
}


/*
 * Returns a lowercase string.
 */
char *strlower( const char *str )
{
    static char strlow[MAX_STRING_LENGTH];
    int i;

    for ( i = 0; str[i] != '\0'; i++ )
	strlow[i] = LOWER(str[i]);
    strlow[i] = '\0';
    return strlow;
}

/*
 * Returns an uppercase string.
 */
char *strupper( const char *str )
{
    static char strup[MAX_STRING_LENGTH];
    int i;

    for ( i = 0; str[i] != '\0'; i++ )
	strup[i] = UPPER(str[i]);
    strup[i] = '\0';
    return strup;
}

/*
 * Returns TRUE or FALSE if a letter is a vowel			-Thoric
 */
bool isavowel( char letter )
{
    char c;

    c = tolower( letter );
    if ( c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' )
      return TRUE;
    else
      return FALSE;
}

/*
 * Shove either "a " or "an " onto the beginning of a string	-Thoric
 */
char *aoran( const char *str )
{
    static char temp[MAX_STRING_LENGTH];

    if ( !str )
    {
	bug( "Aoran(): NULL str" );
	return "";
    }

    if ( isavowel(str[0])
    || ( strlen(str) > 1 && tolower(str[0]) == 'y' && !isavowel(str[1])) )
      strcpy( temp, "an " );
    else
      strcpy( temp, "a " );
    strcat( temp, str );
    return temp;
}


/*
 * Append a string to a file.
 */
void append_file( CHAR_DATA *ch, char *file, char *str )
{
    FILE *fp;

    if ( IS_NPC(ch) || str[0] == '\0' )
	return;

    new_fclose( fpLOG );
    if ( ( fp = fopen( file, "a" ) ) == NULL )
    {
	perror( file );
	send_to_char( "Could not open the file!\n\r", ch );
    }
    else
    {
	fprintf( fp, "[%5d] %s: %s\n",
	    ch->in_room ? ch->in_room->vnum : 0, ch->name, str );
	new_fclose( fp );
    }

    fpLOG = fopen( NULL_FILE, "r" );
    return;
}

/*
 * Append a string to a file.
 */
void append_to_file( char *file, char *str )
{
    FILE *fp;

    if ( ( fp = fopen( file, "a" ) ) == NULL )
      perror( file );
    else
    {
	fprintf( fp, "%s\n", str );
	new_fclose( fp );
    }

    return;
}


/*
 * Reports a bug.
 */
void bug( const char *str, ... )
{
    char buf[MAX_STRING_LENGTH];
    FILE *fp;
    struct stat fst;

	if ( str == NULL )
	   return;


    if ( fpArea != NULL )
    {
	int iLine;
	int iChar;


	if ( fpArea == stdin )
	{
	    iLine = 0;
	}
	else
	{
	    iChar = ftell( fpArea );
	    fseek( fpArea, 0, 0 );
	    for ( iLine = 0; ftell( fpArea ) < iChar; iLine++ )
	    {
		if (!iChar)
		  break;

		if ( !fpArea )
     		  break;

		while ( getc( fpArea ) != '\n' );
		
	    }
	    fseek( fpArea, iChar, 0 );
	}

        if ( !strArea )
           return;

	if ( iLine <= 0 )
            return;

        if (  iLine > 3900 )
	   return;
        
	sprintf( buf, "[*****] FILE: %s LINE: %d", strArea, iLine );

        if ( strlen(buf) > 4096 )
          return;

	log_string( buf );

	if ( stat( SHUTDOWN_FILE, &fst ) != -1 )	/* file exists */
	{
	    if ( ( fp = fopen( SHUTDOWN_FILE, "a" ) ) != NULL )
	    {
		fprintf( fp, "[*****] %s\n", buf );
		new_fclose( fp );
	    }
	}
    }

    strcpy( buf, "[*****] BUG: " );
    {
	va_list param;
    
	va_start(param, str);
	vsprintf( buf + strlen(buf), str, param );
	va_end(param);
    }
    log_string( buf );

    new_fclose( fpLOG );
    if ( ( fp = fopen( BUG_FILE, "a" ) ) != NULL )
    {
	fprintf( fp, "%s\n", buf ); 
	new_fclose( fp );
    }
    fpLOG = fopen( NULL_FILE, "r" );

    return;
}

/*
 * Add a string to the boot-up log				-Thoric
 */
void boot_log( const char *str, ... )
{
    char buf[MAX_STRING_LENGTH];
    FILE *fp;
    va_list param;

    if ( str == NULL )
	return;

    strcpy( buf, "[*****] BOOT: " );
    va_start(param, str);
    vsprintf( buf+strlen(buf), str, param );
    va_end(param);
    log_string( buf );

    new_fclose( fpLOG );
    if ( ( fp = fopen( BOOTLOG_FILE, "a" ) ) != NULL )
    {
	fprintf( fp, "%s\n", buf );
 	new_fclose( fp );
    }
    fpLOG = fopen( NULL_FILE, "r" );

    return;
}

/*
 * Dump a text file to a player, a line at a time		-Thoric
 */
void show_file( CHAR_DATA *ch, char *filename )
{
    FILE *fp;
    char buf[MAX_STRING_LENGTH];
    int c;
    int num = 0;

    if ( (fp = fopen( filename, "r" )) != NULL )
    {
      while ( !feof(fp) )
      {
	while ((buf[num]=fgetc(fp)) != EOF
	&&      buf[num] != '\n'
	&&      buf[num] != '\r'
	&&      num < (MAX_STRING_LENGTH-2))
	  num++;
	c = fgetc(fp);
	if ( (c != '\n' && c != '\r') || c == buf[num] )
	  ungetc(c, fp);
	buf[num++] = '\n';
	buf[num++] = '\r';
	buf[num  ] = '\0';

	if ( strlen(buf) > 32000 )
        {
	 bug("Strlen Greater then 32000: show_file",0);
	 buf[32000] = '\0';
        }
	if ( ( send_to_pager( buf, ch ) )==FALSE)
	return;

	num = 0;
      }
    }
new_fclose(fp);
}

/*
 * Show the boot log file					-Thoric
 */
void do_dmesg( CHAR_DATA *ch, char *argument )
{
    send_to_char("Command Curently Disabled.\n\r",ch);
    return;
    set_pager_color( AT_LOG, ch );
    show_file( ch, BOOTLOG_FILE );
}

void do_buglog( CHAR_DATA *ch, char *argument )
{
    send_to_char("Command Curently Disabled.\n\r",ch);
    return;
    set_pager_color( AT_LOG, ch );
    show_file( ch, BUG_FILE );

}

void do_memlog( CHAR_DATA *ch, char *argument )
{
    send_to_char("Command Curently Disabled.\n\r",ch);
    return;
    set_pager_color( AT_LOG, ch );
    show_file( ch, MEMLOG_FILE );
}

void do_mudlog( CHAR_DATA *ch, char *argument )
{
    send_to_char("Command Curently Disabled. Im SURE you meant Muststat :) --GW\n\r",ch);
    return;
    set_pager_color( AT_LOG, ch );
    show_file( ch, MUDLOG_FILE );
}

void do_crashlog( CHAR_DATA *ch, char *argument )
{
    send_to_char("Command Curently Disabled.\n\r",ch);
    return;
    set_pager_color( AT_LOG, ch );
    show_file( ch, CRASH_FILE );
}

/* Show the CR Log */
void do_crlog( CHAR_DATA *ch, char *argument )
{
    set_pager_color( AT_LOG, ch );
    show_file( ch, "../system/CRLOG" );
}

/*
 * Writes a string to the log, extended version			-Thoric
 */
void log_string_plus( const char *str, sh_int log_type, sh_int level )
{
    char *strtime;
    int offset;
/*    FILE *fp;*/

/* disabled for now  --GW      
        if ( ( fp = fopen( MUDLOG_FILE, "a" ) ) == NULL )
        {
            bug("Cannot Open mudlog File!",0);
            perror( MUDLOG_FILE );
        }
*/
    strtime                    = ctime( &current_time );
    strtime[strlen(strtime)-1] = '\0';
    fprintf( stderr, "%s :: %s\n", strtime, str ); 

/*         if ( mudlog_off == FALSE )
    fprintf( fp, "%s :: %s\n", strtime, str ); */

    if ( strncmp( str, "Log ", 4 ) == 0 )
      offset = 4;
    else
      offset = 0;
    switch( log_type )
    {
	default:
	  to_channel( str + offset, CHANNEL_LOG, "[ ** ", level );
	  break;
	case LOG_BUILD:
	  to_channel( str + offset, CHANNEL_BUILD, "[ ** ", level );
	  break;
	case LOG_COMM:
	  to_channel( str + offset, CHANNEL_COMM, "[ ** ", level );
	  break;
	case LOG_ALL:
	  break;
    }
  /*  new_fclose(fp);*/
    return;
}

#define WEBWIZLIST_FILE  SYSTEM_DIR "WEBWIZLIST"

/*
 * wizlist builder!						-Thoric
 * Web Version --GW
 */
void towebwizfile( const char *line )
{
  int filler, xx;
  char outline[MAX_STRING_LENGTH];
  FILE *wfp;

  outline[0] = '\0';

  if ( line && line[0] != '\0' )
  {
   filler = ( 78-strlen( line ) );
   if ( filler < 1 )
     filler = 1;
   filler /= 2;
   for ( xx = 0; xx < filler; xx++ )
      strcat( outline, " " );
    strcat( outline, line );
  }
  strcat( outline, "<BR>" );
  wfp = fopen( WEBWIZLIST_FILE, "a" );
  if ( wfp )
  {
    fputs( outline, wfp );
    new_fclose( wfp );
  }
}


/*
 * wizlist builder!						-Thoric
 */

void towizfile( const char *line )
{
  int filler, xx;
  char outline[MAX_STRING_LENGTH];
  FILE *wfp;

  outline[0] = '\0';

  if ( line && line[0] != '\0' )
  {
   filler = ( 78-strlen( line ) );
   if ( filler < 1 )
     filler = 1;
   filler /= 2;
   for ( xx = 0; xx < filler; xx++ )
      strcat( outline, " " );
    strcat( outline, line );
  }
  strcat( outline, "\n\r" );
  wfp = fopen( WIZLIST_FILE, "a" );
  if ( wfp )
  {
    fputs( outline, wfp );
    new_fclose( wfp );
  }
}

void add_to_wizlist( char *name, int level, int flags )
{
  WIZENT *wiz=NULL, *tmp=NULL;

#ifdef DEBUG
  log_string( "Adding to wizlist..." );
#endif

  CREATE( wiz, WIZENT, 1 );
  wiz->name	= str_dup( name );
  wiz->level	= level;
  wiz->flags	= flags;

  if ( !first_wiz )
  {
    wiz->last	= NULL;
    wiz->next	= NULL;
    first_wiz	= wiz;
    last_wiz	= wiz;
    return;
  }

  /* insert sort, of sorts */
  for ( tmp = first_wiz; tmp; tmp = tmp->next )
    if ( level > tmp->level )
    {
      if ( !tmp->last )
	first_wiz	= wiz;
      else
	tmp->last->next = wiz;
      wiz->last = tmp->last;
      wiz->next = tmp;
      tmp->last = wiz;
      return;
    }

  wiz->last		= last_wiz;
  wiz->next		= NULL;
  last_wiz->next	= wiz;
  last_wiz		= wiz;
  return;
}

/*
 * Web Wizlist builder						-Thoric
 * Modified by GW
 */
void make_web_wizlist( )
{
  DIR *dp;
  struct dirent *dentry;
  FILE *gfp;
  char *word;
  int ilevel, iflags;
  WIZENT *wiz, *wiznext;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  int cnt=0;

  if ( !fBootDb )
  {
  for ( wiz = first_wiz; wiz; wiz = wiznext )
  {
    wiznext = wiz->next;
    if ( wiz )
    {
    DISPOSE(wiz->name);
    DISPOSE(wiz);
    }
  }
  }

  first_wiz = NULL;
  last_wiz = NULL;

  dp = opendir( GOD_DIR );

  ilevel = 0;
  dentry = readdir( dp );
  while ( dentry )
  {
      if ( dentry->d_name[0] != '.' )
      {
	sprintf( buf, "%s%s", GOD_DIR, dentry->d_name );
        sprintf( buf2, "%s", dentry->d_name );
	gfp = fopen( buf, "r" );
	if ( gfp )
	{
	  word = feof( gfp ) ? "End" : fread_word( gfp );
	  ilevel = fread_number( gfp );	  
          fread_to_eol( gfp );
	  word = feof( gfp ) ? "End" : fread_word( gfp );
          if ( !str_cmp( word, "Pcflags" ) )
	    iflags = fread_number( gfp );
          else
	    iflags = 0;
	  new_fclose( gfp );

	  add_to_wizlist( dentry->d_name, ilevel, iflags );

	  if ( !IS_SET( iflags, PCFLAG_GUEST ) && !IS_SET( iflags, PCFLAG_RETIRED) )
	    cnt++;

	}
      }
      dentry = readdir( dp );
  }
  closedir( dp );

  buf[0] = '\0';
  unlink( WEBWIZLIST_FILE );
  towebwizfile( "<CENTER>" );
  towebwizfile( "-----===={== <B><I>Age of the Ancients Immortals</B></I> ==}===-----");
  ilevel = 65535;
  for ( wiz = first_wiz; wiz; wiz = wiz->next )
  {
	  if ( IS_SET( wiz->flags, PCFLAG_RETIRED ) ||
               IS_SET( wiz->flags, PCFLAG_GUEST ) )
		continue;

    if ( wiz->level < ilevel )
    {
      if ( buf[0] )
      {
	towebwizfile( buf );
	buf[0] = '\0';
      }
      towebwizfile( "" );
      ilevel = wiz->level;

      switch(ilevel)
      {
	case MAX_LEVEL -  0: towebwizfile( "<B>&lt;~&gt; <I>HYPERION</I> &lt;~&gt;</B>");break;
	case MAX_LEVEL -  1: towebwizfile( "<B>&lt;~&gt; <I>COMMITTEE</I> &lt;~&gt;</B>");break;
	case MAX_LEVEL -  2: towebwizfile( "<B>&lt;~&gt; <I>COUNCIL</I> &lt;~&gt;</B>");break;
	case MAX_LEVEL -  3: towebwizfile( "<B>&lt;~&gt; <I>HEAD GOD</I> &lt;~&gt;</B>");break;
	case MAX_LEVEL -  4: towebwizfile( "<B>&lt;~&gt; <I>GOD</I> &lt;~&gt;</B>");break;
	case MAX_LEVEL -  5: towebwizfile( "<B>&lt;~&gt; <I>FATE</I> &lt;~&gt;</B>");break;
	case MAX_LEVEL -  6: towebwizfile( "<B>&lt;~&gt; <I>TITAN</I> &lt;~&gt;</B>");break;
	case MAX_LEVEL -  7: towebwizfile( "<B>&lt;~&gt; <I>LORD</I> &lt;~&gt;</B>");break;
	case MAX_LEVEL -  8: towebwizfile( "<B>&lt;~&gt; <I>WIZARD</I> &lt;~&gt;</B>");break;
	case MAX_LEVEL -  9: towebwizfile( "<B>&lt;~&gt; <I>EMPATH</I> &lt;~&gt;</B>");break;
	default:	     towebwizfile( "<B>&lt;~&gt; <I>BUG</I> &lt;~&gt;</B>" );break;
      }
    }

    if ( strlen( buf ) + strlen( wiz->name ) > 76 )
    {
	towebwizfile( buf );
	buf[0] = '\0';
    }

    strcat( buf, " " );
    strcat( buf, wiz->name );
    if ( strlen( buf ) > 70 )
    {
      towebwizfile( buf );
      buf[0] = '\0';
    }
  }

  if ( buf[0] )
    towebwizfile( buf );

  towebwizfile( "</CENTER>" );
  sprintf( buf, "\n\r<B>Total Immortals:</B> <I>%d</I>\n\r",cnt);
  towebwizfile( buf );
}


/*
 * Wizlist builder						-Thoric
 */
void make_wizlist( )
{
  DIR *dp;
  struct dirent *dentry;
  FILE *gfp;
  char *word;
  int ilevel, iflags;
  WIZENT *wiz, *wiznext;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  int cnt=0;

  if ( !fBootDb )
  {
  for ( wiz = first_wiz; wiz; wiz = wiznext )
  {
    wiznext = wiz->next;
    if ( wiz )
    {
    DISPOSE(wiz->name);
    DISPOSE(wiz);
    }
  }
  }

  first_wiz = NULL;
  last_wiz = NULL;

  dp = opendir( GOD_DIR );

  ilevel = 0;
  dentry = readdir( dp );
  while ( dentry )
  {
      if ( dentry->d_name[0] != '.' )
      {
	sprintf( buf, "%s%s", GOD_DIR, dentry->d_name );
        sprintf( buf2, "%s", dentry->d_name );
	gfp = fopen( buf, "r" );
	if ( gfp )
	{
	  word = feof( gfp ) ? "End" : fread_word( gfp );
	  ilevel = fread_number( gfp );
          fread_to_eol( gfp );
	  word = feof( gfp ) ? "End" : fread_word( gfp );
          if ( !str_cmp( word, "Pcflags" ) )
	    iflags = fread_number( gfp );
          else
	    iflags = 0;
	  new_fclose( gfp );

	  add_to_wizlist( dentry->d_name, ilevel, iflags );

	  if ( !IS_SET( iflags, PCFLAG_GUEST ) && !IS_SET( iflags, PCFLAG_RETIRED) )
          cnt++;

	}
      }
      dentry = readdir( dp );
  }
  closedir( dp );

  buf[0] = '\0';
  unlink( WIZLIST_FILE );
  ilevel = 65535;
  towizfile( "-----===={== Age of the Ancients Immortals ==}===-----" );
  for ( wiz = first_wiz; wiz; wiz = wiz->next )
  {
	  if ( IS_SET( wiz->flags, PCFLAG_RETIRED ) ||
               IS_SET( wiz->flags, PCFLAG_GUEST ) )
		continue;

    if ( wiz->level < ilevel )
    {
      if ( buf[0] )
      {
	towizfile( buf );
	buf[0] = '\0';
      }
      towizfile( "" );
      ilevel = wiz->level;

      switch(ilevel)
      {
	case MAX_LEVEL -  0: towizfile( "<~> HYPERION <~> " );break;
	case MAX_LEVEL -  1: towizfile( "<~> COMMITTEE <~> " );break;
	case MAX_LEVEL -  2: towizfile( "<~> COUNCIL <~>" );break;
	case MAX_LEVEL -  3: towizfile( "<~> HEAD GOD <~>" );break;
	case MAX_LEVEL -  4: towizfile( "<~> GOD <~>" );break;
	case MAX_LEVEL -  5: towizfile( "<~> FATE <~>" );break;
	case MAX_LEVEL -  6: towizfile( "<~> TITAN <~>" );break;
	case MAX_LEVEL -  7: towizfile( "<~> LORD <~>" );break;
	case MAX_LEVEL -  8: towizfile( "<~> WIZARD <~>" );break;
	case MAX_LEVEL -  9: towizfile( "<~> EMPATH <~>" );break;
	default:	     towizfile( "<~> BUG <~>" );	break;
      }
    }

    if ( strlen( buf ) + strlen( wiz->name ) > 76 )
    {
	towizfile( buf );
	buf[0] = '\0';
    }

    strcat( buf, " " );
    strcat( buf, wiz->name );
    if ( strlen( buf ) > 70 )
    {
      towizfile( buf );
      buf[0] = '\0';
    }
  }

  if ( buf[0] )
    towizfile( buf );
  
  sprintf( buf, "\n\rTotal Immortals: %d\n\r",cnt);
  towizfile( buf );
}


void do_makewizlist( CHAR_DATA *ch, char *argument )
{
  make_wizlist();
  make_web_wizlist();
}


/* mud prog functions */

/* This routine reads in scripts of MUDprograms from a file */

int mprog_name_to_type ( char *name )
{
   if ( !str_cmp( name, "in_file_prog"   ) )	return IN_FILE_PROG;
   if ( !str_cmp( name, "act_prog"       ) )    return ACT_PROG;
   if ( !str_cmp( name, "speech_prog"    ) )	return SPEECH_PROG;
   if ( !str_cmp( name, "rand_prog"      ) ) 	return RAND_PROG;
   if ( !str_cmp( name, "fight_prog"     ) )	return FIGHT_PROG;
   if ( !str_cmp( name, "hitprcnt_prog"  ) )	return HITPRCNT_PROG;
   if ( !str_cmp( name, "death_prog"     ) )	return DEATH_PROG;
   if ( !str_cmp( name, "entry_prog"     ) )	return ENTRY_PROG;
   if ( !str_cmp( name, "greet_prog"     ) )	return GREET_PROG;
   if ( !str_cmp( name, "all_greet_prog" ) )	return ALL_GREET_PROG;
   if ( !str_cmp( name, "give_prog"      ) ) 	return GIVE_PROG;
   if ( !str_cmp( name, "bribe_prog"     ) )	return BRIBE_PROG;
   if ( !str_cmp( name, "time_prog"     ) )	return TIME_PROG;
   if ( !str_cmp( name, "hour_prog"     ) )	return HOUR_PROG;
   if ( !str_cmp( name, "wear_prog"     ) )	return WEAR_PROG;
   if ( !str_cmp( name, "remove_prog"   ) )	return REMOVE_PROG;
   if ( !str_cmp( name, "sac_prog"      ) )	return SAC_PROG;
   if ( !str_cmp( name, "look_prog"     ) )	return LOOK_PROG;
   if ( !str_cmp( name, "exa_prog"      ) )	return EXA_PROG;
   if ( !str_cmp( name, "zap_prog"      ) )	return ZAP_PROG;
   if ( !str_cmp( name, "get_prog"      ) ) 	return GET_PROG;
   if ( !str_cmp( name, "drop_prog"     ) )	return DROP_PROG;
   if ( !str_cmp( name, "damage_prog"   ) )	return DAMAGE_PROG;
   if ( !str_cmp( name, "repair_prog"   ) )	return REPAIR_PROG;
   if ( !str_cmp( name, "greet_prog"    ) )	return GREET_PROG;
   if ( !str_cmp( name, "randiw_prog"   ) )	return RANDIW_PROG;
   if ( !str_cmp( name, "speechiw_prog" ) )	return SPEECHIW_PROG;
   if ( !str_cmp( name, "pull_prog"	) )     return PULL_PROG;
   if ( !str_cmp( name, "push_prog"	) )     return PUSH_PROG;
   if ( !str_cmp( name, "sleep_prog"    ) )	return SLEEP_PROG;
   if ( !str_cmp( name, "rest_prog"	) )	return REST_PROG;
   if ( !str_cmp( name, "rfight_prog"   ) )	return FIGHT_PROG;
   if ( !str_cmp( name, "enter_prog"    ) )	return ENTRY_PROG;
   if ( !str_cmp( name, "leave_prog"    ) )	return LEAVE_PROG;
   if ( !str_cmp( name, "rdeath_prog"	) )	return DEATH_PROG;
   if ( !str_cmp( name, "script_prog"	) )	return SCRIPT_PROG;
   if ( !str_cmp( name, "use_prog"	) )	return USE_PROG;
   return( ERROR_PROG );
}

MPROG_DATA *mprog_file_read( char *f, MPROG_DATA *mprg,
			    MOB_INDEX_DATA *pMobIndex )
{

  char        MUDProgfile[ MAX_INPUT_LENGTH ];
  FILE       *progfile;
  char        letter;
  MPROG_DATA *mprg_next=NULL, *mprg2=NULL;
  bool        done = FALSE;

  sprintf( MUDProgfile, "%s%s", PROG_DIR, f );

  progfile = fopen( MUDProgfile, "r" );
  if ( !progfile )
  {
     bug( "Mob: %d couldn't open mudprog file", pMobIndex->vnum );
     exit( 1 );
  }

  mprg2 = mprg;
  switch ( letter = fread_letter( progfile ) )
  {
    case '>':
     break;
    case '|':
       bug( "empty mudprog file." );
       exit( 1 );
     break;
    default:
       bug( "in mudprog file syntax error." );
       exit( 1 );
     break;
  }

  while ( !done )
  {
    mprg2->type = mprog_name_to_type( fread_word( progfile ) );
    switch ( mprg2->type )
    {
     case ERROR_PROG:
	bug( "mudprog file type error" );
	exit( 1 );
      break;
     case IN_FILE_PROG:
	bug( "mprog file contains a call to file." );
	exit( 1 );
      break;
     default:
	pMobIndex->progtypes = pMobIndex->progtypes | mprg2->type;
	mprg2->arglist       = fread_string( progfile );
	mprg2->comlist       = fread_string( progfile );
	switch ( letter = fread_letter( progfile ) )
	{
	  case '>':
	     CREATE( mprg_next, MPROG_DATA, 1 );
	     mprg_next->next = mprg2;
	     mprg2 = mprg_next;
	   break;
	  case '|':
	     done = TRUE;
	   break;
	  default:
	     bug( "in mudprog file syntax error." );
	     exit( 1 );
	   break;
	}
      break;
    }
  }
  new_fclose( progfile );
  return mprg2;
}

/* Load a MUDprogram section from the area file.
 */
void load_mudprogs( AREA_DATA *tarea, FILE *fp )
{
  MOB_INDEX_DATA *iMob=NULL;
  MPROG_DATA     *original=NULL;
  MPROG_DATA     *working=NULL;
  char            letter;
  int             value;

  for ( ; ; )
    switch ( letter = fread_letter( fp ) )
    {
    default:
      bug( "Load_mudprogs: bad command '%c'.",letter);
      exit(1);
      break;
    case 'S':
    case 's':
      fread_to_eol( fp );
      return;
    case '*':
      fread_to_eol( fp );
      break;
    case 'M':
    case 'm':
      value = fread_number( fp );
      if ( ( iMob = get_mob_index( value,(int)tarea->zone->number ) ) ==NULL )
      {
	bug( "Load_mudprogs: vnum %d doesnt exist", value );
	exit( 1 );
      }

      /* Go to the end of the prog command list if other commands
	 exist */

      if ( (original = iMob->mudprogs) != NULL )
	for ( ; original->next; original = original->next );

      CREATE( working, MPROG_DATA, 1 );
      if ( original )
	original->next = working;
      else
	iMob->mudprogs = working;
      working = mprog_file_read( fread_word( fp ), working, iMob );
      working->next = NULL;
      fread_to_eol( fp );
      break;
    }

  return;

}

/* This procedure is responsible for reading any in_file MUDprograms.
 */

void mprog_read_programs( FILE *fp, MOB_INDEX_DATA *pMobIndex)
{
  MPROG_DATA *mprg=NULL;
  char        letter;
  bool        done = FALSE;

  if ( ( letter = fread_letter( fp ) ) != '>' )
  {
      bug( "Load_mobiles: vnum %d MUDPROG char", pMobIndex->vnum );
      exit( 1 );
  }
  CREATE( mprg, MPROG_DATA, 1 );
  pMobIndex->mudprogs = mprg;

  while ( !done )
  {
    mprg->type = mprog_name_to_type( fread_word( fp ) );
    switch ( mprg->type )
    {
     case ERROR_PROG:
	bug( "Load_mobiles: vnum %d MUDPROG type.", pMobIndex->vnum );
	exit( 1 );
      break;
     case IN_FILE_PROG:
	mprg = mprog_file_read( fread_string( fp ), mprg,pMobIndex );
	fread_to_eol( fp );
	switch ( letter = fread_letter( fp ) )
	{
	  case '>':
	     CREATE( mprg->next, MPROG_DATA, 1 );
	     mprg = mprg->next;
	   break;
	  case '|':
	     mprg->next = NULL;
	     fread_to_eol( fp );
	     done = TRUE;
	   break;
	  default:
	     bug( "Load_mobiles: vnum %d bad MUDPROG.", pMobIndex->vnum );
	     exit( 1 );
	   break;
	}
      break;
     default:
	pMobIndex->progtypes = pMobIndex->progtypes | mprg->type;
	mprg->arglist        = fread_string( fp );
	fread_to_eol( fp );
	mprg->comlist        = fread_string( fp );
	fread_to_eol( fp );
	switch ( letter = fread_letter( fp ) )
	{
	  case '>':
	     CREATE( mprg->next, MPROG_DATA, 1 );
	     mprg = mprg->next;
	   break;
	  case '|':
	     mprg->next = NULL;
	     fread_to_eol( fp );
	     done = TRUE;
	   break;
	  default:
	     bug( "Load_mobiles: vnum %d bad MUDPROG.", pMobIndex->vnum );
	     exit( 1 );
	   break;
	}
      break;
    }
  }

  return;

}



/*************************************************************/
/* obj prog functions */
/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. This allows the use of the words in the
 *  mob/script files.
 */

/* This routine reads in scripts of OBJprograms from a file */


MPROG_DATA *oprog_file_read( char *f, MPROG_DATA *mprg,
			    OBJ_INDEX_DATA *pObjIndex )
{

  char        MUDProgfile[ MAX_INPUT_LENGTH ];
  FILE       *progfile=NULL;
  char        letter;
  MPROG_DATA *mprg_next=NULL, *mprg2=NULL;
  bool        done = FALSE;

  sprintf( MUDProgfile, "%s%s", PROG_DIR, f );

  progfile = fopen( MUDProgfile, "r" );
  if ( !progfile )
  {
     bug( "Obj: %d couldnt open mudprog file", pObjIndex->vnum );
     exit( 1 );
  }

  mprg2 = mprg;
  switch ( letter = fread_letter( progfile ) )
  {
    case '>':
     break;
    case '|':
       bug( "empty objprog file." );
       exit( 1 );
     break;
    default:
       bug( "in objprog file syntax error." );
       exit( 1 );
     break;
  }

  while ( !done )
  {
    mprg2->type = mprog_name_to_type( fread_word( progfile ) );
    switch ( mprg2->type )
    {
     case ERROR_PROG:
	bug( "objprog file type error" );
	exit( 1 );
      break;
     case IN_FILE_PROG:
	bug( "objprog file contains a call to file." );
	exit( 1 );
      break;
     default:
	pObjIndex->progtypes = pObjIndex->progtypes | mprg2->type;
	mprg2->arglist       = fread_string( progfile );
	mprg2->comlist       = fread_string( progfile );
	switch ( letter = fread_letter( progfile ) )
	{
	  case '>':
	     CREATE( mprg_next, MPROG_DATA, 1 );
	     mprg_next->next = mprg2;
	     mprg2 = mprg_next;
	   break;
	  case '|':
	     done = TRUE;
	   break;
	  default:
	     bug( "in objprog file syntax error." );
	     exit( 1 );
	   break;
	}
      break;
    }
  }
  new_fclose( progfile );
  return mprg2;
}

/* Load a MUDprogram section from the area file.
 */
void load_objprogs( AREA_DATA *tarea, FILE *fp )
{
  OBJ_INDEX_DATA *iObj;
  MPROG_DATA     *original=NULL;
  MPROG_DATA     *working=NULL;
  char            letter;
  int             value;

  for ( ; ; )
    switch ( letter = fread_letter( fp ) )
    {
    default:
      bug( "Load_objprogs: bad command '%c'.",letter);
      exit(1);
      break;
    case 'S':
    case 's':
      fread_to_eol( fp );
      return;
    case '*':
      fread_to_eol( fp );
      break;
    case 'M':
    case 'm':
      value = fread_number( fp );
      if ( ( iObj = get_obj_index( value,(int)tarea->zone->number ) ) ==NULL )
      {
	bug( "Load_objprogs: vnum %d doesnt exist", value );
	exit( 1 );
      }

      /* Go to the end of the prog command list if other commands
	 exist */

      if ( (original = iObj->mudprogs) != NULL )
	for ( ; original->next; original = original->next );

      CREATE( working, MPROG_DATA, 1 );
      if ( original )
	original->next = working;
      else
	iObj->mudprogs = working;
      working = oprog_file_read( fread_word( fp ), working, iObj );
      working->next = NULL;
      fread_to_eol( fp );
      break;
    }

  return;

}

/* This procedure is responsible for reading any in_file OBJprograms.
 */

void oprog_read_programs( FILE *fp, OBJ_INDEX_DATA *pObjIndex)
{
  MPROG_DATA *mprg=NULL;
  char        letter;
  bool        done = FALSE;

  if ( ( letter = fread_letter( fp ) ) != '>' )
  {
      bug( "Load_objects: vnum %d OBJPROG char", pObjIndex->vnum );
      exit( 1 );
  }
  CREATE( mprg, MPROG_DATA, 1 );
  pObjIndex->mudprogs = mprg;

  while ( !done )
  {
    mprg->type = mprog_name_to_type( fread_word( fp ) );
    switch ( mprg->type )
    {
     case ERROR_PROG:
	bug( "Load_objects: vnum %d OBJPROG type.", pObjIndex->vnum );
	exit( 1 );
      break;
     case IN_FILE_PROG:
	mprg = oprog_file_read( fread_string( fp ), mprg,pObjIndex );
	fread_to_eol( fp );
	switch ( letter = fread_letter( fp ) )
	{
	  case '>':
	     CREATE( mprg->next, MPROG_DATA, 1 );
	     mprg = mprg->next;
	   break;
	  case '|':
	     mprg->next = NULL;
	     fread_to_eol( fp );
	     done = TRUE;
	   break;
	  default:
	     bug( "Load_objects: vnum %d bad OBJPROG.", pObjIndex->vnum );
	     exit( 1 );
	   break;
	}
      break;
     default:
	pObjIndex->progtypes = pObjIndex->progtypes | mprg->type;
	mprg->arglist        = fread_string( fp );
	fread_to_eol( fp );
	mprg->comlist        = fread_string( fp );
	fread_to_eol( fp );
	switch ( letter = fread_letter( fp ) )
	{
	  case '>':
	     CREATE( mprg->next, MPROG_DATA, 1 );
	     mprg = mprg->next;
	   break;
	  case '|':
	     mprg->next = NULL;
	     fread_to_eol( fp );
	     done = TRUE;
	   break;
	  default:
	     bug( "Load_objects: vnum %d bad OBJPROG.", pObjIndex->vnum );
	     exit( 1 );
	   break;
	}
      break;
    }
  }

  return;

}


/*************************************************************/
/* room prog functions */
/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. This allows the use of the words in the
 *  mob/script files.
 */

/* This routine reads in scripts of OBJprograms from a file */
MPROG_DATA *rprog_file_read( char *f, MPROG_DATA *mprg,
			    ROOM_INDEX_DATA *RoomIndex )
{

  char        MUDProgfile[ MAX_INPUT_LENGTH ];
  FILE       *progfile;
  char        letter;
  MPROG_DATA *mprg_next=NULL, *mprg2=NULL;
  bool        done = FALSE;

  sprintf( MUDProgfile, "%s%s", PROG_DIR, f );

  progfile = fopen( MUDProgfile, "r" );
  if ( !progfile )
  {
     bug( "Room: %d couldnt open roomprog file", RoomIndex->vnum );
     exit( 1 );
  }

  mprg2 = mprg;
  switch ( letter = fread_letter( progfile ) )
  {
    case '>':
     break;
    case '|':
       bug( "empty roomprog file." );
       exit( 1 );
     break;
    default:
       bug( "in roomprog file syntax error." );
       exit( 1 );
     break;
  }

  while ( !done )
  {
    mprg2->type = mprog_name_to_type( fread_word( progfile ) );
    switch ( mprg2->type )
    {
     case ERROR_PROG:
	bug( "roomprog file type error" );
	exit( 1 );
      break;
     case IN_FILE_PROG:
	bug( "roomprog file contains a call to file." );
	exit( 1 );
      break;
     default:
	RoomIndex->progtypes = RoomIndex->progtypes | mprg2->type;
	mprg2->arglist       = fread_string( progfile );
	mprg2->comlist       = fread_string( progfile );
	switch ( letter = fread_letter( progfile ) )
	{
	  case '>':
	     CREATE( mprg_next, MPROG_DATA, 1 );
	     mprg_next->next = mprg2;
	     mprg2 = mprg_next;
	   break;
	  case '|':
	     done = TRUE;
	   break;
	  default:
	     bug( "in roomprog file syntax error." );
	     exit( 1 );
	   break;
	}
      break;
    }
  }
  new_fclose( progfile );
  return mprg2;
}

/* Load a ROOMprogram section from the area file.
 */
void load_roomprogs( AREA_DATA *tarea, FILE *fp )
{
  ROOM_INDEX_DATA *iRoom;
  MPROG_DATA     *original=NULL;
  MPROG_DATA     *working=NULL;
  char            letter;
  int             value;

  for ( ; ; )
    switch ( letter = fread_letter( fp ) )
    {
    default:
      bug( "Load_objprogs: bad command '%c'.",letter);
      exit(1);
      break;
    case 'S':
    case 's':
      fread_to_eol( fp );
      return;
    case '*':
      fread_to_eol( fp );
      break;
    case 'M':
    case 'm':
      value = fread_number( fp );
      if ( ( iRoom = get_room_index( value,(int)tarea->zone->number ) ) ==NULL )
      {
	bug( "Load_roomprogs: vnum %d doesnt exist", value );
	exit( 1 );
      }

      /* Go to the end of the prog command list if other commands
	 exist */

      if ( (original = iRoom->mudprogs) != NULL )
	for ( ; original->next; original = original->next );

      CREATE( working, MPROG_DATA, 1 );
      if ( original )
	original->next = working;
      else
	iRoom->mudprogs = working;
      working = rprog_file_read( fread_word( fp ), working, iRoom );
      working->next = NULL;
      fread_to_eol( fp );
      break;
    }

  return;

}

/* This procedure is responsible for reading any in_file ROOMprograms.
 */

void rprog_read_programs( FILE *fp, ROOM_INDEX_DATA *pRoomIndex)
{
  MPROG_DATA *mprg=NULL;
  char        letter;
  bool        done = FALSE;

  if ( ( letter = fread_letter( fp ) ) != '>' )
  {
      bug( "Load_rooms: vnum %d ROOMPROG char", pRoomIndex->vnum );
      exit( 1 );
  }
  CREATE( mprg, MPROG_DATA, 1 );
  pRoomIndex->mudprogs = mprg;

  while ( !done )
  {
    mprg->type = mprog_name_to_type( fread_word( fp ) );
    switch ( mprg->type )
    {
     case ERROR_PROG:
	bug( "Load_rooms: vnum %d ROOMPROG type.", pRoomIndex->vnum );
	exit( 1 );
      break;
     case IN_FILE_PROG:
	mprg = rprog_file_read( fread_string( fp ), mprg,pRoomIndex );
	fread_to_eol( fp );
	switch ( letter = fread_letter( fp ) )
	{
	  case '>':
	     CREATE( mprg->next, MPROG_DATA, 1 );
	     mprg = mprg->next;
	   break;
	  case '|':
	     mprg->next = NULL;
	     fread_to_eol( fp );
	     done = TRUE;
	   break;
	  default:
	     bug( "Load_rooms: vnum %d bad ROOMPROG.", pRoomIndex->vnum );
	     exit( 1 );
	   break;
	}
      break;
     default:
	pRoomIndex->progtypes = pRoomIndex->progtypes | mprg->type;
	mprg->arglist        = fread_string( fp );
	fread_to_eol( fp );
	mprg->comlist        = fread_string( fp );
	fread_to_eol( fp );
	switch ( letter = fread_letter( fp ) )
	{
	  case '>':
	     CREATE( mprg->next, MPROG_DATA, 1 );
	     mprg = mprg->next;
	   break;
	  case '|':
	     mprg->next = NULL;
	     fread_to_eol( fp );
	     done = TRUE;
	   break;
	  default:
	     bug( "Load_rooms: vnum %d bad ROOMPROG.", pRoomIndex->vnum );
	     exit( 1 );
	   break;
	}
      break;
    }
  }

  return;

}


/*************************************************************/
/* Function to delete a room index.  Called from do_rdelete in build.c
   Narn, May/96
*/
bool delete_room( ROOM_INDEX_DATA *room, int zone )
{
    int iHash;
    ROOM_INDEX_DATA *tmp, *prev=NULL;
    ZONE_DATA *in_zone;

    iHash = room->vnum % MAX_KEY_HASH;
    if( (in_zone=find_zone(zone))==NULL)
    {
	bug("delete_room: ZONE ACCESS VIOLATION!!! STOPING NOW!",0);
	return FALSE;
    }

    /* Take the room index out of the hash list. */
    for( tmp = in_zone->room_index_hash[iHash]; tmp && tmp != room; tmp = tmp->next )
    {
      prev = tmp;
    }

    if( !tmp )
    {
      bug( "Delete_room: room not found" );
      return FALSE;
    }

    if( prev )
    {
      prev->next = room->next;
    }
    else
    {
      in_zone->room_index_hash[iHash] = room->next;
    }   
 
    /* Free up the ram for all strings attached to the room. */
    STRFREE( room->name );
    STRFREE( room->description );

    /* Free up the ram held by the room index itself. */
    DISPOSE( room );

    top_room--;
    return TRUE;
}

/* See comment on delete_room. */
bool delete_obj( OBJ_INDEX_DATA *obj )
{
    return TRUE;
}

/* See comment on delete_room. */
bool delete_mob( MOB_INDEX_DATA *mob )
{
    return TRUE;
}

/*
 * Creat a new room (for online building)			-Thoric
 */
ROOM_INDEX_DATA *make_room( int vnum, int zone )
{
	ROOM_INDEX_DATA *pRoomIndex=NULL;
	int	iHash;
	ZONE_DATA *in_zone=NULL;

if( !(in_zone=find_zone(zone)) )
{
bug("Make_Room: ZONE ACCESS VIOLATION!!! Stoping.",0);
return NULL;
}

	CREATE( pRoomIndex, ROOM_INDEX_DATA, 1 );
	pRoomIndex->first_person	= NULL;
	pRoomIndex->last_person		= NULL;
	pRoomIndex->first_content	= NULL;
	pRoomIndex->last_content	= NULL;
	pRoomIndex->first_extradesc	= NULL;
	pRoomIndex->last_extradesc	= NULL;
	pRoomIndex->area		= NULL;
	pRoomIndex->vnum		= vnum;
	pRoomIndex->name		= STRALLOC("Floating in a void");
	pRoomIndex->description		= STRALLOC("");
	pRoomIndex->music		= STRALLOC("");
	pRoomIndex->room_flags		= ROOM_PROTOTYPE;
	pRoomIndex->sector_type		= 1;
	pRoomIndex->light		= 0;
        pRoomIndex->level		= 0;
	pRoomIndex->first_exit		= NULL;
	pRoomIndex->last_exit		= NULL;
        pRoomIndex->area		= find_room_area(pRoomIndex->vnum);
	iHash			= vnum % MAX_KEY_HASH;
	pRoomIndex->next	= in_zone->room_index_hash[iHash];
	in_zone->room_index_hash[iHash]	= pRoomIndex;
	top_room++;

	return pRoomIndex;
}

/*
 * Create a new INDEX object (for online building)		-Thoric
 * Option to clone an existing index object.
 */
OBJ_INDEX_DATA *make_object( int vnum, int cvnum, char *name, int zone )
{
	OBJ_INDEX_DATA *pObjIndex=NULL, *cObjIndex=NULL;
	char buf[MAX_STRING_LENGTH];
	int	iHash;
	ZONE_DATA *in_zone;

if ( !(in_zone=find_zone(zone)) )
{
bug("Make_Object: ZONE ACCESS VIOLATION!!!! Stoping.",0);
return NULL;
}

	if ( cvnum > 0 )
	  cObjIndex = get_obj_index( cvnum, zone );
	else
	  cObjIndex = NULL;
	CREATE( pObjIndex, OBJ_INDEX_DATA, 1 );
	pObjIndex->vnum			= vnum;
	pObjIndex->name			= STRALLOC( name );
	pObjIndex->first_affect		= NULL;
	pObjIndex->last_affect		= NULL;
	pObjIndex->first_extradesc	= NULL;
	pObjIndex->last_extradesc	= NULL;
	if ( !cObjIndex )
	{
	  sprintf( buf, "A newly created %s", name );
	  pObjIndex->short_descr	= STRALLOC( buf  );
	  sprintf( buf, "Some god dropped a newly created %s here.", name );
	  pObjIndex->description	= STRALLOC( buf );
	  pObjIndex->action_desc	= STRALLOC( "" );
	  pObjIndex->short_descr[0]	= LOWER(pObjIndex->short_descr[0]);
	  pObjIndex->description[0]	= UPPER(pObjIndex->description[0]);
	  pObjIndex->item_type		= ITEM_TRASH;
	  pObjIndex->extra_flags	= ITEM_PROTOTYPE;
	  pObjIndex->wear_flags		= 0;
	  pObjIndex->value[0]		= 0;
	  pObjIndex->value[1]		= 0;
	  pObjIndex->value[2]		= 0;
	  pObjIndex->value[3]		= 0;
	  pObjIndex->weight		= 1;
	  pObjIndex->cost		= 0;
	  pObjIndex->area		= find_obj_area(pObjIndex->vnum);
	}
	else
	{
	  EXTRA_DESCR_DATA *ed=NULL,  *ced=NULL;
	  AFFECT_DATA	   *paf=NULL, *cpaf=NULL;

	  pObjIndex->short_descr	= QUICKLINK( cObjIndex->short_descr );
	  pObjIndex->description	= QUICKLINK( cObjIndex->description );
	  pObjIndex->action_desc	= QUICKLINK( cObjIndex->action_desc );
	  pObjIndex->item_type		= cObjIndex->item_type;
	  pObjIndex->extra_flags	= cObjIndex->extra_flags
	  				| ITEM_PROTOTYPE;
	  pObjIndex->second_flags	= cObjIndex->second_flags;
	  pObjIndex->wear_flags		= cObjIndex->wear_flags;
	  pObjIndex->minlevel		= cObjIndex->minlevel;
	  pObjIndex->ego		= cObjIndex->ego;
	  pObjIndex->value[0]		= cObjIndex->value[0];
	  pObjIndex->value[1]		= cObjIndex->value[1];
	  pObjIndex->value[2]		= cObjIndex->value[2];
	  pObjIndex->value[3]		= cObjIndex->value[3];
	  pObjIndex->weight		= cObjIndex->weight;
	  pObjIndex->cost		= cObjIndex->cost;
	  pObjIndex->area		= find_obj_area(pObjIndex->vnum);
	  for ( ced = cObjIndex->first_extradesc; ced; ced = ced->next )
	  {
		CREATE( ed, EXTRA_DESCR_DATA, 1 );
		ed->keyword		= QUICKLINK( ced->keyword );
		ed->description		= QUICKLINK( ced->description );
		LINK( ed, pObjIndex->first_extradesc, pObjIndex->last_extradesc,
			  next, prev );
		top_ed++;
	  }
	  for ( cpaf = cObjIndex->first_affect; cpaf; cpaf = cpaf->next )
	  {
		CREATE( paf, AFFECT_DATA, 1 );
		paf->type		= cpaf->type;
		paf->duration		= cpaf->duration;
		paf->location		= cpaf->location;
		paf->modifier		= cpaf->modifier;
		paf->bitvector		= cpaf->bitvector;
		LINK( paf, pObjIndex->first_affect, pObjIndex->last_affect,
			   next, prev );
		top_affect++;
	  }
	}
	pObjIndex->count		= 0;
	iHash				= vnum % MAX_KEY_HASH;
	pObjIndex->next			= in_zone->obj_index_hash[iHash];
	in_zone->obj_index_hash[iHash]		= pObjIndex;
	top_obj_index++;

	return pObjIndex;
}

/*
 * Create a new INDEX mobile (for online building)		-Thoric
 * Option to clone an existing index mobile.
 */
MOB_INDEX_DATA *make_mobile( sh_int vnum, sh_int cvnum, char *name, int zone )
{
	MOB_INDEX_DATA *pMobIndex=NULL, *cMobIndex=NULL;
	char buf[MAX_STRING_LENGTH];
	int	iHash;
	ZONE_DATA *in_zone=NULL;


if ( !(in_zone=find_zone(zone)) )
{
bug("Make_Mobile: ZONE ACCESS VIOLATION! Stoping.");
return NULL;
}

	if ( cvnum > 0 )
	  cMobIndex = get_mob_index( cvnum, zone );
	else
	  cMobIndex = NULL;
	CREATE( pMobIndex, MOB_INDEX_DATA, 1 );
	pMobIndex->vnum			= vnum;
	pMobIndex->count		= 0;
	pMobIndex->killed		= 0;
	pMobIndex->player_name		= STRALLOC( name );
	if ( !cMobIndex )
	{
	  sprintf( buf, "A newly created %s", name );
	  pMobIndex->short_descr	= STRALLOC( buf  );
	  sprintf( buf, "Some god abandoned a newly created %s here.\n\r", name );
	  pMobIndex->long_descr		= STRALLOC( buf );
	  pMobIndex->description	= STRALLOC( "" );
	  pMobIndex->short_descr[0]	= LOWER(pMobIndex->short_descr[0]);
	  pMobIndex->long_descr[0]	= UPPER(pMobIndex->long_descr[0]);
	  pMobIndex->description[0]	= UPPER(pMobIndex->description[0]);
	  pMobIndex->act		= ACT_IS_NPC | ACT_PROTOTYPE;
	  pMobIndex->flags		= 0;
	  xCLEAR_BITS(pMobIndex->affected_by);
	  pMobIndex->pShop		= NULL;
	  pMobIndex->rShop		= NULL;
	  pMobIndex->spec_fun		= NULL;
	  pMobIndex->mudprogs		= NULL;
	  pMobIndex->progtypes		= 0;
	  pMobIndex->alignment		= 0;
	  pMobIndex->level		= 1;
	  pMobIndex->mobthac0		= 0;
	  pMobIndex->ac			= 0;
	  pMobIndex->hitnodice		= 0;
	  pMobIndex->hitsizedice	= 0;
	  pMobIndex->hitplus		= 0;
	  pMobIndex->damnodice		= 0;
	  pMobIndex->damsizedice	= 0;
	  pMobIndex->damplus		= 0;
	  pMobIndex->gold		= 0;
	  pMobIndex->exp		= 0;
	  pMobIndex->position		= 8;
	  pMobIndex->defposition	= 8;
	  pMobIndex->sex		= 0;
	  pMobIndex->perm_str		= 13;
	  pMobIndex->perm_dex		= 13;
	  pMobIndex->perm_int		= 13;
	  pMobIndex->perm_wis		= 13;
	  pMobIndex->perm_cha		= 13;
	  pMobIndex->perm_con		= 13;
	  pMobIndex->perm_lck		= 13;
	  pMobIndex->race		= 0;
	  pMobIndex->class		= 3;
	  pMobIndex->xflags		= 0;
	  pMobIndex->resistant		= 0;
	  pMobIndex->immune		= 0;
	  pMobIndex->susceptible	= 0;
	  pMobIndex->numattacks		= 0;
	  pMobIndex->attacks		= 0;
	  pMobIndex->defenses		= 0;
	  pMobIndex->area		= find_mob_area(pMobIndex->vnum);
	}
	else
	{
	  pMobIndex->short_descr	= QUICKLINK( cMobIndex->short_descr );
	  pMobIndex->long_descr		= QUICKLINK( cMobIndex->long_descr  );
	  pMobIndex->description	= QUICKLINK( cMobIndex->description );
	  pMobIndex->act		= cMobIndex->act | ACT_PROTOTYPE;
	  pMobIndex->flags		= 0;
	  pMobIndex->acttwo		= cMobIndex->acttwo;
	  pMobIndex->smart		= cMobIndex->smart;
	  pMobIndex->affected_by	= cMobIndex->affected_by;
	  pMobIndex->pShop		= NULL;
	  pMobIndex->rShop		= NULL;
	  pMobIndex->spec_fun		= cMobIndex->spec_fun;
	  pMobIndex->mudprogs		= NULL;
	  pMobIndex->progtypes		= 0;
	  pMobIndex->alignment		= cMobIndex->alignment;
	  pMobIndex->level		= cMobIndex->level;
	  pMobIndex->mobthac0		= cMobIndex->mobthac0;
	  pMobIndex->ac			= cMobIndex->ac;
	  pMobIndex->hitnodice		= cMobIndex->hitnodice;
	  pMobIndex->hitsizedice	= cMobIndex->hitsizedice;
	  pMobIndex->hitplus		= cMobIndex->hitplus;
	  pMobIndex->damnodice		= cMobIndex->damnodice;
	  pMobIndex->damsizedice	= cMobIndex->damsizedice;
	  pMobIndex->damplus		= cMobIndex->damplus;
	  pMobIndex->gold		= cMobIndex->gold;
	  pMobIndex->exp		= cMobIndex->exp;
	  pMobIndex->position		= cMobIndex->position;
	  pMobIndex->defposition	= cMobIndex->defposition;
	  pMobIndex->sex		= cMobIndex->sex;
	  pMobIndex->perm_str		= cMobIndex->perm_str;
	  pMobIndex->perm_dex		= cMobIndex->perm_dex;
	  pMobIndex->perm_int		= cMobIndex->perm_int;
	  pMobIndex->perm_wis		= cMobIndex->perm_wis;
	  pMobIndex->perm_cha		= cMobIndex->perm_cha;
	  pMobIndex->perm_con		= cMobIndex->perm_con;
	  pMobIndex->perm_lck		= cMobIndex->perm_lck;
	  pMobIndex->race		= cMobIndex->race;
	  pMobIndex->class		= cMobIndex->class;
	  pMobIndex->xflags		= cMobIndex->xflags;
	  pMobIndex->resistant		= cMobIndex->resistant;
	  pMobIndex->immune		= cMobIndex->immune;
	  pMobIndex->susceptible	= cMobIndex->susceptible;
	  pMobIndex->numattacks		= cMobIndex->numattacks;
	  pMobIndex->attacks		= cMobIndex->attacks;
	  pMobIndex->defenses		= cMobIndex->defenses;
	  pMobIndex->area		= find_mob_area(pMobIndex->vnum);
	}
	SET_BIT( pMobIndex->act, ACT_IS_NPC );
	iHash				= vnum % MAX_KEY_HASH;
	pMobIndex->next			= in_zone->mob_index_hash[iHash];
	in_zone->mob_index_hash[iHash]		= pMobIndex;
	top_mob_index++;

	return pMobIndex;
}

/*
 * Creates a simple exit with no fields filled but rvnum and optionally
 * to_room and vnum.						-Thoric
 * Exits are inserted into the linked list based on vdir.
 */
EXIT_DATA *make_exit( ROOM_INDEX_DATA *pRoomIndex, ROOM_INDEX_DATA *to_room, sh_int door )
{
	EXIT_DATA *pexit=NULL, *texit=NULL;
	bool broke;

	CREATE( pexit, EXIT_DATA, 1 );
	pexit->vdir		= door;
	pexit->rvnum		= pRoomIndex->vnum;
	pexit->to_room		= to_room;
	pexit->distance		= 1;
	if ( to_room )
	{
	    pexit->vnum = to_room->vnum;
	    texit = get_exit_to( to_room, rev_dir[door], pRoomIndex->vnum );
	    if ( texit )	/* assign reverse exit pointers */
	    {
		texit->rexit = pexit;
		pexit->rexit = texit;
	    }
	}
	broke = FALSE;
	for ( texit = pRoomIndex->first_exit; texit; texit = texit->next )
	   if ( door < texit->vdir )
	   {
	     broke = TRUE;
	     break;
	   }
	if ( !pRoomIndex->first_exit )
	  pRoomIndex->first_exit	= pexit;
	else
	{
	  /* keep exits in incremental order - insert exit into list */
	  if ( broke && texit )
	  {
	    if ( !texit->prev )
	      pRoomIndex->first_exit	= pexit;
	    else
	      texit->prev->next		= pexit;
	    pexit->prev			= texit->prev;
	    pexit->next			= texit;
	    texit->prev			= pexit;
	    top_exit++;
	    return pexit;
	  }
	  pRoomIndex->last_exit->next	= pexit;
	}
	pexit->next			= NULL;
	pexit->prev			= pRoomIndex->last_exit;
	pRoomIndex->last_exit		= pexit;
	top_exit++;
	return pexit;
}

void fix_area_exits( AREA_DATA *tarea )
{
    ROOM_INDEX_DATA *pRoomIndex;
    EXIT_DATA *pexit, *rev_exit;
    int rnum;
    bool fexit;
    int in_zone;

if ( tarea->zone )
in_zone = tarea->zone->number;
else
in_zone = 1;

    for ( rnum = tarea->low_r_vnum; rnum <= tarea->hi_r_vnum; rnum++ )
    {
	if ( (pRoomIndex = get_room_index( rnum,in_zone )) ==NULL )
	  continue;

	fexit = FALSE;
	for ( pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next )
	{
		fexit = TRUE;
		pexit->rvnum = pRoomIndex->vnum;
		if ( pexit->vnum <= 0 )
	       	  pexit->to_room = NULL;
		else
		  pexit->to_room = get_room_index(pexit->vnum,in_zone );
	}
	if ( !fexit )
	  SET_BIT( pRoomIndex->room_flags, ROOM_NO_MOB );
    }


    for ( rnum = tarea->low_r_vnum; rnum <= tarea->hi_r_vnum; rnum++ )
    {
	if ( (pRoomIndex = get_room_index( rnum,in_zone )) ==NULL )
	  continue;

	for ( pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next )
	{
		if ( pexit->to_room && !pexit->rexit )
		{
		   rev_exit = get_exit_to( pexit->to_room, rev_dir[pexit->vdir], pRoomIndex->vnum );
		   if ( rev_exit )
		   {
			pexit->rexit	= rev_exit;
			rev_exit->rexit	= pexit;
		   }
		}
	}
    }
}

void load_zone_file( ZONE_DATA *tzone, char *filename )
{

    if ( fBootDb )
      tzone = last_zone;
    if ( !fBootDb && !tzone )
    {
	bug( "Load_zone: null zone!" );
	return;
    }

    if ( ( fpArea = fopen( filename, "r" ) ) == NULL )
    {
	perror( filename );
	bug( "load_zone: error loading file (can't open)" );
	bug( filename );
	return;
    }

    for ( ; ; )
    {
	char *word;

	if ( fread_letter( fpArea ) != '#' )
	{
	    bug( tzone->filename );
	    bug( "load_zone: # not found." );
	    exit( 1 );
	}

	word = fread_word( fpArea );

	     if ( word[0] == '$'               )                 break;
	else if ( !str_cmp( word, "ZONE"     ) )
	{
		if ( fBootDb )
		{
		  load_zone_header    (fpArea);
		  tzone = last_zone;
		}
		else
		{
		  DISPOSE( tzone->name );
		  tzone->name = fread_string_nohash( fpArea );
		}
	}
	/* GW */
	else if ( !str_cmp( word, "NUMBER"    ) ) load_zone_number (tzone,fpArea);
	else if ( !str_cmp( word, "LIST_NAME" ) ) load_zone_lname  (tzone,fpArea);
	else
	{
	    bug( tzone->filename );
	    bug( "load_zone: bad section name." );
	    if ( fBootDb )
	      exit( 1 );
	    else
	    {
	      new_fclose( fpArea );
	      fpArea = NULL;
	      return;
	    }
	}
    }
    new_fclose( fpArea );
    fpArea = NULL;
    if ( tzone )
    {
	fprintf( stderr, "%-14s Zone #: %d\n",
		 tzone->name,tzone->number );
	   top_zone++;
    }
    else
      fprintf( stderr, "(%s)\n", filename );
}

void load_area_file( AREA_DATA *tarea, char *filename )
{
/*    FILE *fpin;
    what intelligent person stopped using fpArea?????
    if fpArea isn't being used, then no filename or linenumber
    is printed when an error occurs during loading the area..
    (bug uses fpArea)
      --TRI  */

    if ( fBootDb )
      tarea = last_area;
    if ( !fBootDb && !tarea )
    {
	bug( "Load_area: null area!" );
	return;
    }

    if ( ( fpArea = fopen( filename, "r" ) ) == NULL )
    {
	perror( filename );
	bug( "load_area: error loading file (can't open)" );
	bug( filename );
	return;
    }

    for ( ; ; )
    {
	char *word;

	if ( fread_letter( fpArea ) != '#' )
	{
	    bug( tarea->filename );
	    bug( "load_area: # not found." );
	    exit( 1 );
	}

	word = fread_word( fpArea );

	     if ( word[0] == '$'               )                 break;
	else if ( !str_cmp( word, "AREA"     ) )
	{
		if ( fBootDb )
		{
		  load_area    (fpArea);
		  tarea = last_area;
		}
		else
		{
		  DISPOSE( tarea->name );
		  tarea->name = fread_string_nohash( fpArea );
		}
	}
	else if ( !str_cmp( word, "VERSION"  ) ) load_version (tarea, fpArea);
	else if ( !str_cmp( word, "AUTHOR"   ) ) load_author  (tarea, fpArea);
	else if ( !str_cmp( word, "FLAGS"    ) ) load_flags   (tarea, fpArea);
	else if ( !str_cmp( word, "RANGES"   ) ) load_ranges  (tarea, fpArea);
	else if ( !str_cmp( word, "ECONOMY"  ) ) load_economy (tarea, fpArea);
	else if ( !str_cmp( word, "RESETMSG" ) ) load_resetmsg(tarea, fpArea); 
	else if ( !str_cmp( word, "CLUB"     ) ) load_club    (tarea,fpArea); 
	else if ( !str_cmp( word, "MAPFILE"  ) ) load_mapfile (tarea,fpArea);
	/* Rennard */
	else if ( !str_cmp( word, "HELPS"    ) ) load_helps   (tarea, fpArea);
	else if ( !str_cmp( word, "MOBILES"  ) ) load_mobiles (tarea, fpArea);
	else if ( !str_cmp( word, "MUDPROGS" ) ) load_mudprogs(tarea, fpArea);
	else if ( !str_cmp( word, "OBJECTS"  ) ) load_objects (tarea, fpArea);
	else if ( !str_cmp( word, "OBJPROGS" ) ) load_objprogs(tarea, fpArea);
	else if ( !str_cmp( word, "RESETS"   ) ) load_resets  (tarea, fpArea);
	else if ( !str_cmp( word, "ROOMS"    ) ) load_rooms   (tarea, fpArea);
	else if ( !str_cmp( word, "SHOPS"    ) ) load_shops   (tarea, fpArea);
	else if ( !str_cmp( word, "REPAIRS"  ) ) load_repairs (tarea, fpArea);
	else if ( !str_cmp( word, "SPECIALS" ) ) load_specials(tarea, fpArea);
	else if ( !str_cmp( word, "VEHICLES" ) ) load_vehicles(tarea, fpArea);
	else
	{
	    bug( tarea->filename );
	    bug( "load_area: bad section name." );
	    if ( fBootDb )
	      exit( 1 );
	    else
	    {
	      new_fclose( fpArea );
	      fpArea = NULL;
	      return;
	    }
	}
    }
    new_fclose( fpArea );
    fpArea = NULL;
    if ( tarea )
    {
	if ( fBootDb )
	  sort_area( tarea, FALSE );

	fprintf( stderr, "%-14s: Rooms: %5d - %-5d Objs: %5d - %-5d Mobs: %5d - %d\n",
		 tarea->filename,
		 tarea->low_r_vnum, tarea->hi_r_vnum,
		 tarea->low_o_vnum, tarea->hi_o_vnum,
		 tarea->low_m_vnum, tarea->hi_m_vnum );
	if ( !tarea->author )
	  tarea->author = STRALLOC( "" );
	SET_BIT( tarea->status, AREA_LOADED );
    }
    else
      fprintf( stderr, "(%s)\n", filename );
}



/* Build list of in_progress areas.  Do not load areas.
 * define AREA_READ if you want it to build area names rather than reading
 * them out of the area files. -- Altrag */
void load_buildlist( void )
{
	DIR *dp;
	struct dirent *dentry;
	FILE *fp;
	char buf[MAX_STRING_LENGTH];
	AREA_DATA *pArea=NULL;
	char line[81];
	char word[81];
	int low, hi;
	int mlow, mhi, olow, ohi, rlow, rhi;
	bool badfile = FALSE;
	char temp;
        struct stat fst;
	
	dp = opendir( GOD_DIR );
	dentry = readdir( dp );
	while ( dentry )
	{
		if ( dentry->d_name[0] != '.' )
		{
			sprintf( buf, "%s%s", GOD_DIR, dentry->d_name );
			if ( !(fp = fopen( buf, "r" )) )
			{
				bug( "Load_buildlist: invalid file" );
				perror( buf );
				dentry = readdir(dp);
				continue;
			}
			log_string( buf );
			badfile = FALSE;
			rlow=rhi=olow=ohi=mlow=mhi=0;
			while ( !feof(fp) && !ferror(fp) )
			{
				low = 0; hi = 0; word[0] = 0; line[0] = 0;
				if ( (temp = fgetc(fp)) != EOF )
					ungetc( temp, fp );
				else
					break;
				
				fgets(line, 80, fp);
				sscanf( line, "%s %d %d", word, &low, &hi );
				if ( !strcmp( word, "Level" ) )
				{
					if ( low < LEVEL_IMMORTAL )
					{
						sprintf( buf, "%s: God file with level %d < %d",
							dentry->d_name, low, LEVEL_IMMORTAL );
						badfile = TRUE;
					}
				}
				if ( !strcmp( word, "RoomRange" ) )
					rlow = low, rhi = hi;
				else if ( !strcmp( word, "MobRange" ) )
					mlow = low, mhi = hi;
				else if ( !strcmp( word, "ObjRange" ) )
					olow = low, ohi = hi;
			}
			new_fclose( fp );
			if ( rlow && rhi && !badfile )
			{
				sprintf( buf, "%s%s.are", BUILD_DIR, dentry->d_name );

                                if ( stat( buf, &fst ) != -1 )
                                {
				if ( !(fp = fopen( buf, "r" )) )
				{
					bug( "Load_buildlist: cannot open area file for read" );
					perror( buf );
					dentry = readdir(dp);
					continue;
				}
			        }
				else
				{
				dentry = readdir(dp);
				continue;
				}

#if !defined(READ_AREA)  /* Dont always want to read stuff.. dunno.. shrug */
				strcpy( word, fread_word( fp ) );
				if ( word[0] != '#' || strcmp( &word[1], "AREA" ) )
				{
					sprintf( buf, "Make_buildlist: %s.are: no #AREA found.",
						dentry->d_name );
					new_fclose( fp );
					dentry = readdir(dp);
					continue;
				}
#endif
				pArea=NULL;
				CREATE( pArea, AREA_DATA, 1 );
				sprintf( buf, "%s.are", dentry->d_name );
				pArea->author = STRALLOC( dentry->d_name );
				pArea->filename = str_dup( buf );
#if !defined(READ_AREA)
				pArea->name = fread_string_nohash( fp );
#else
				sprintf( buf, "{PROTO} %s's area in progress", dentry->d_name );
				pArea->name = str_dup( buf );
#endif
				new_fclose( fp );
				pArea->low_r_vnum = rlow; pArea->hi_r_vnum = rhi;
				pArea->low_m_vnum = mlow; pArea->hi_m_vnum = mhi;
				pArea->low_o_vnum = olow; pArea->hi_o_vnum = ohi;
				pArea->low_soft_range = -1; pArea->hi_soft_range = -1;
				pArea->low_hard_range = -1; pArea->hi_hard_range = -1;
				pArea->first_reset = NULL; pArea->last_reset = NULL;
				LINK( pArea, first_build, last_build, next, prev );
				fprintf( stderr, "%-14s: Rooms: %5d - %-5d Objs: %5d - %-5d "
								 "Mobs: %5d - %-5d\n",
					pArea->filename,
					pArea->low_r_vnum, pArea->hi_r_vnum,
					pArea->low_o_vnum, pArea->hi_o_vnum,
					pArea->low_m_vnum, pArea->hi_m_vnum );
				sort_area( pArea, TRUE );
			}
		}
		dentry = readdir(dp);
	}
	closedir(dp);
}


/*
 * Sort by room vnums					-Altrag & Thoric
 */
void sort_area( AREA_DATA *pArea, bool proto )
{
    AREA_DATA *area = NULL;
    AREA_DATA *first_sort, *last_sort;
    bool found;

    if ( !pArea )
    {
	bug( "Sort_area: NULL pArea" );
	return;
    }

    if ( proto )
    {
	first_sort = first_bsort;
	last_sort  = last_bsort;
    }
    else
    {
	first_sort = first_asort;
	last_sort  = last_asort;
    }
	
    found = FALSE;
    pArea->next_sort = NULL;
    pArea->prev_sort = NULL;

    if ( !first_sort )
    {
	pArea->prev_sort = NULL;
	pArea->next_sort = NULL;
	first_sort	 = pArea;
	last_sort	 = pArea;
	found = TRUE;
    }
    else
    for ( area = first_sort; area; area = area->next_sort )
	if ( pArea->low_r_vnum < area->low_r_vnum )
	{
	    if ( !area->prev_sort )
	      first_sort	= pArea;
	    else
	      area->prev_sort->next_sort = pArea;
	    pArea->prev_sort = area->prev_sort;
	    pArea->next_sort = area;
	    area->prev_sort  = pArea;
	    found = TRUE;
	    break;
        }

    if ( !found )
    {
	pArea->prev_sort     = last_sort;
	pArea->next_sort     = NULL;
	last_sort->next_sort = pArea;
	last_sort	     = pArea;
    }

    if ( proto )
    {
	first_bsort = first_sort;
	last_bsort  = last_sort;
    }
    else
    {
	first_asort = first_sort;
	last_asort  = last_sort;
    }
}


/*
 * Display vnums currently assigned to areas		-Altrag & Thoric
 * Sorted, and flagged if loaded.
 */
void show_vnums( CHAR_DATA *ch, int low, int high, bool proto, bool shownl,
		 char *loadst, char *notloadst )
{
    AREA_DATA *pArea, *first_sort;
    int count, loaded;

    count = 0;	loaded = 0;
    set_pager_color( AT_PLAIN, ch );
    if ( proto )
      first_sort = first_bsort;
    else
      first_sort = first_asort;
    for ( pArea = first_sort; pArea; pArea = pArea->next_sort )
    {
	if ( IS_SET(pArea->status, AREA_LOADED) )
	   loaded++;

	if ( IS_SET( pArea->status, AREA_DELETED ) )
	   continue;
	if ( pArea->low_r_vnum < low )
	   continue;
	if ( pArea->hi_r_vnum > high )
	   break;
	if ( !shownl )
	   continue;

        if ( pArea->low_r_vnum > 0 && pArea->hi_r_vnum > 0 )
        {
	pager_printf(ch, "%-15s| Rooms: %5d - %-5d"
		     " Objs: %5d - %-5d Mobs: %5d - %-5d%s\n\r",
		(pArea->filename ? pArea->filename : "(invalid)"),
		pArea->low_r_vnum, pArea->hi_r_vnum,
		pArea->low_o_vnum, pArea->hi_o_vnum,
		pArea->low_m_vnum, pArea->hi_m_vnum,
		IS_SET(pArea->status, AREA_LOADED) ? loadst : notloadst);
	count++;

        }
    }
    pager_printf( ch, "Areas listed: %d  Loaded: %d\n\r",count, loaded );
    return;
}

/*
 * Shows prototype vnums ranges, and if loaded
 */
void do_vnums( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int low, high;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    low = 0;	high = 32766;
    if ( arg1[0] != '\0' )
    {
	low = atoi(arg1);
	if ( arg2[0] != '\0' )
	  high = atoi(arg2);
    }
    show_vnums( ch, low, high, TRUE, TRUE, " *", "" );
}

/*
 * Shows installed areas, sorted.  Mark unloaded areas with an X
 */
void do_zones( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int low, high;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    low = 0;	high = 32766;
    if ( arg1[0] != '\0' )
    {
	low = atoi(arg1);
	if ( arg2[0] != '\0' )
	  high = atoi(arg2);
    }
    show_vnums( ch, low, high, FALSE, TRUE, "", " X" );
}

/*
 * Show prototype areas, sorted.  Only show loaded areas
 */
void do_newzones( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int low, high;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    low = 0;	high = 32766;
    if ( arg1[0] != '\0' )
    {
	low = atoi(arg1);
	if ( arg2[0] != '\0' )
	  high = atoi(arg2);
    }
    show_vnums( ch, low, high, TRUE, FALSE, "", " X" );
}

/*
 * Save system info to data file
 */
void save_sysdata( SYSTEM_DATA sys )
{
    FILE *fp;
    char filename[MAX_INPUT_LENGTH];
    APPROVE_DATA *approve, *next_approve;

    sprintf( filename, "%ssysdata.dat", SYSTEM_DIR );
    
//    new_fclose( fpReserve );
    if ( ( fp = fopen( filename, "w" ) ) == NULL )
    {
    	bug( "save_sysdata: fopen" );
    	perror( filename );
    }
    else
    {
	fprintf( fp, "#SYSTEM\n" );
	fprintf( fp, "Highplayers    %d\n", sys.alltimemax		);
	fprintf( fp, "Highplayertime %s~\n", sys.time_of_max		);
	fprintf( fp, "Nameresolving  %d\n", sys.NO_NAME_RESOLVING	);
	fprintf( fp, "Waitforauth    %d\n", sys.WAIT_FOR_AUTH		);
	fprintf( fp, "Game_Wizlocked %d\n", sys.GAME_WIZLOCKED		);
	fprintf( fp, "Rent_Ratio     %d\n", sys.rent_ratio		);
        fprintf( fp, "Max_Hp	     %d\n", sys.max_hp			);
        fprintf( fp, "Max_Mana       %d\n", sys.max_mana		);
	fprintf( fp, "Maxaward	     %d\n", sys.maxaward		);
        fprintf( fp, "SegVio 	     %d\n", sys.segvio			);
	fprintf( fp, "WebServ	     %d\n", sys.webserv			);
	fprintf( fp, "ConnectSound   %s~\n",sys.connectsound		);
	fprintf( fp, "EnterSound     %s~\n",sys.entersound		);
	fprintf( fp, "Readallmail    %d\n", sys.read_all_mail		);
	fprintf( fp, "Readmailfree   %d\n", sys.read_mail_free		);
	fprintf( fp, "Writemailfree  %d\n", sys.write_mail_free		);
	fprintf( fp, "Takeothersmail %d\n", sys.take_others_mail	);
	fprintf( fp, "IMCMailVnum    %d\n", sys.imc_mail_vnum		);
	fprintf( fp, "Muse           %d\n", sys.muse_level		);
	fprintf( fp, "Think          %d\n", sys.think_level		);
	fprintf( fp, "Build          %d\n", sys.build_level		);
	fprintf( fp, "Log            %d\n", sys.log_level		);
	fprintf( fp, "Protoflag      %d\n", sys.level_modify_proto	);
	fprintf( fp, "Overridepriv   %d\n", sys.level_override_private	);
	fprintf( fp, "Msetplayer     %d\n", sys.level_mset_player	);
	fprintf( fp, "Stunplrvsplr   %d\n", sys.stun_plr_vs_plr		);
	fprintf( fp, "Stunregular    %d\n", sys.stun_regular		);
	fprintf( fp, "Damplrvsplr    %d\n", sys.dam_plr_vs_plr		);
	fprintf( fp, "Damplrvsmob    %d\n", sys.dam_plr_vs_mob		);
	fprintf( fp, "Dammobvsplr    %d\n", sys.dam_mob_vs_plr		);
	fprintf( fp, "Dammobvsmob    %d\n", sys.dam_mob_vs_mob		);
	fprintf( fp, "Forcepc        %d\n", sys.level_forcepc		);
	fprintf( fp, "Guildoverseer  %s~\n", sys.guild_overseer		);
	fprintf( fp, "Guildadvisor   %s~\n", sys.guild_advisor		);
	fprintf( fp, "PKChamp	     %s~\n", sys.pkchamp		);
	fprintf( fp, "BootTime	     %d\n", (int)sys.boottime		);
	fprintf( fp, "MaxPlrsBoot    %d\n", sys.max_plrs_boot		);
	fprintf( fp, "Logins	     %d\n", sys.logins			);
	fprintf( fp, "Saveflags      %d\n", sys.save_flags		);
	fprintf( fp, "Savefreq       %d\n", sys.save_frequency		);

        /* Save Approves --GW */
if ( EMAIL_AUTH )
{
	for ( approve = sysdata.first_approve; approve; approve = next_approve )
        {
          next_approve = approve->next;
   
          fprintf(fp,"APPROVE %s %s~\n",approve->email, approve->password);
         }
}
	fprintf( fp, "End\n\n"						);
	fprintf( fp, "#END\n"						);
    }

    new_fclose( fp );
//    fpReserve = fopen( NULL_FILE, "r" );
    return;
}


void fread_sysdata( SYSTEM_DATA *sys, FILE *fp )
{
    char *word=NULL;
    bool fMatch=FALSE;

    sys->time_of_max = NULL;
    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;
	case 'A':
            /* Load Approves --GW */
	    if ( !str_cmp( word, "APPROVE" ) )
            {
	      char *appstring;
	      char app_email[MSL], app_pass[MSL];
              APPROVE_DATA *approve=NULL;
           
		fMatch=TRUE;
		appstring=fread_string(fp);
 		appstring=one_argument( appstring, app_email );
	        appstring=one_argument( appstring, app_pass );
	
		CREATE(approve, APPROVE_DATA, 1 );
		approve->email = STRALLOC( app_email );
 		approve->password =  STRALLOC( app_pass );
		LINK(approve, sysdata.first_approve, sysdata.last_approve, next, prev);
	        sprintf(log_buf,"Loaded Approval: E:%s P:%s",approve->email, approve->password);
		log_string(log_buf);
	        break;
	     }
	     break;

	case 'B':
	    KEY( "Build",	   sys->build_level,	  fread_number( fp ) );
	    KEY( "BootTime",	   sys->boottime,	  fread_number( fp ) );
	    break;

	case 'C':
	    KEY( "ConnectSound",   sys->connectsound,	  fread_string( fp ) );
	    break;

	case 'D':
	    KEY( "Damplrvsplr",	   sys->dam_plr_vs_plr,	  fread_number( fp ) );
	    KEY( "Damplrvsmob",	   sys->dam_plr_vs_mob,	  fread_number( fp ) );
	    KEY( "Dammobvsplr",	   sys->dam_mob_vs_plr,	  fread_number( fp ) );
	    KEY( "Dammobvsmob",	   sys->dam_mob_vs_mob,	  fread_number( fp ) );
	    break;

	case 'E':
	    KEY( "EnterSound",	   sys->entersound,	fread_string( fp ) );
	    if ( !str_cmp( word, "End" ) )
	    {
		if ( !sys->time_of_max )
		    sys->time_of_max = str_dup("(not recorded)");
		return;
	    }
	    break;

	case 'F':
	    KEY( "Forcepc",	   sys->level_forcepc,	  fread_number( fp ) );
	    break;
	    
	case 'G':
	    KEY( "Game_Wizlocked", sys->GAME_WIZLOCKED,  fread_number( fp ) );
	    KEY( "Guildoverseer",  sys->guild_overseer,  fread_string( fp ) );
	    KEY( "Guildadvisor",   sys->guild_advisor,   fread_string( fp ) );
	    break;

	case 'H':
	    KEY( "Highplayers",	   sys->alltimemax,	  fread_number( fp ) );
	    KEY( "Highplayertime", sys->time_of_max,      fread_string_nohash( fp ) );
	    break;

	case 'I':
	    KEY( "IMCMailVnum",    sys->imc_mail_vnum,    fread_number( fp ) );
	    break;

	case 'L':
	    KEY( "Log",		   sys->log_level,	  fread_number( fp ) );
	    KEY( "Logins",	   sys->logins,		  fread_number( fp ) );
	    break;

	case 'M':
	    KEY( "Msetplayer",	   sys->level_mset_player, fread_number( fp ) );
	    KEY( "Muse",	   sys->muse_level,	   fread_number( fp ) );
	    KEY( "Max_Hp",	   sys->max_hp,		   fread_number( fp ) );
	    KEY( "Max_Mana",	   sys->max_mana,	   fread_number( fp ) );
	    KEY( "Maxaward",	   sys->maxaward,	   fread_number( fp ) );
	    KEY( "MaxPlrsBoot",	   sys->max_plrs_boot,    fread_number( fp ) );
	    break;

	case 'N':
            KEY( "Nameresolving",  sys->NO_NAME_RESOLVING, fread_number( fp ) );
	    break;

	case 'O':
	    KEY( "Overridepriv",   sys->level_override_private, fread_number( fp ) );
	    break;

	case 'P':
	    KEY( "Protoflag",	   sys->level_modify_proto, fread_number( fp ) );
	    KEY( "PKChamp",	   sys->pkchamp, 	fread_string( fp ) );
	    break;

	case 'R':
	    KEY( "Readallmail",	   sys->read_all_mail,	fread_number( fp ) );
	    KEY( "Readmailfree",   sys->read_mail_free,	fread_number( fp ) );
  	    KEY( "Rent_Ratio",     sys->rent_ratio,     fread_number( fp ) );
	    break;

	case 'S':
	    KEY( "Stunplrvsplr",   sys->stun_plr_vs_plr, fread_number( fp ) );
	    KEY( "Stunregular",    sys->stun_regular,	fread_number( fp ) );
	    KEY( "Saveflags",	   sys->save_flags,	fread_number( fp ) );
	    KEY( "Savefreq",	   sys->save_frequency,	fread_number( fp ) );
	    KEY( "SegVio",	   sys->segvio,		fread_number( fp ) );
	    break;

	case 'T':
	    KEY( "Takeothersmail", sys->take_others_mail, fread_number( fp ) );
	    KEY( "Think",	   sys->think_level,	fread_number( fp ) );
	    break;


	case 'W':
	    KEY( "Waitforauth",	   sys->WAIT_FOR_AUTH,	  fread_number( fp ) );
	    KEY( "Writemailfree",  sys->write_mail_free,  fread_number( fp ) );
	    KEY( "WebServ",	   sys->webserv,	  fread_number( fp ) );
	    break;
	}
	

	if ( !fMatch )
	{
            bug( "Fread_sysdata: no match: %s", word );
	}
    }
}



/*
 * Load the sysdata file
 */
bool load_systemdata( SYSTEM_DATA *sys )
{
    char filename[MAX_INPUT_LENGTH];
    FILE *fp;
    bool found;

    found = FALSE;
    sprintf( filename, "%ssysdata.dat", SYSTEM_DIR );

    if ( ( fp = fopen( filename, "r" ) ) != NULL )
    {

	found = TRUE;
	for ( ; ; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
		bug( "Load_sysdata_file: # not found." );
		break;
	    }

	    word = fread_word( fp );
	    if ( !str_cmp( word, "SYSTEM" ) )
	    {
	    	fread_sysdata( sys, fp );
	    	break;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
	        break;
	    else
	    {
		bug( "Load_sysdata_file: bad section." );
		break;
	    }
	}
	new_fclose( fp );
    }

    if ( !sysdata.guild_overseer ) sysdata.guild_overseer = str_dup( "" );
    if ( !sysdata.guild_advisor  ) sysdata.guild_advisor  = str_dup( "" );
    return found;
}


void load_banlist( void )
{
  BAN_DATA *pban=NULL;
  FILE *fp=NULL;
  int number;
  char letter;
  
  if ( !(fp = fopen( SYSTEM_DIR BAN_LIST, "r" )) )
    return;
    
  for ( ; ; )
  {
    if ( feof( fp ) )
    {
      bug( "Load_banlist: no -1 found." );
      new_fclose( fp );
      return;
    }
    number = fread_number( fp );
    if ( number == -1 )
    {
      new_fclose( fp );
      return;
    }
    pban=NULL;
    CREATE( pban, BAN_DATA, 1 );
    pban->level = number;
    pban->name = fread_string_nohash( fp );
    if ( (letter = fread_letter(fp)) == '~' )
      pban->ban_time = fread_string_nohash( fp );
    else
    {
      ungetc(letter, fp);
      pban->ban_time = str_dup( "(unrecorded)" );
    }
    pban->ban_issuer = fread_string_nohash( fp );
   LINK( pban, first_ban, last_ban, next, prev );
  }
}

/* Run Grux --GW */
void clean_pfiles( void )
{
log_string("Scanning for Old Player Files.......");
system("/home/mud/aota/aoa/system/grux >> /home/mud/aota/aoa/system/grux.log");
log_string("Done. Check grux.log in the System Dir for Details.");
log_string("Compressing Player Files....");
system("gzip -rf /home/mud/aota/aoa/player");
log_string("Done.");
return;
}

void clean_lfiles( void )
{
log_string("Scanning for Old Locker Files.......");
system("/home/mud/aota/aoa/system/locker >> /home/mud/aota/aoa/system/locker.log");
log_string("Done. Check locker.log in the System Dir for Details.");
return;
}

/* Check to make sure range of vnums is free - Scryn 2/27/96 */

void do_check_vnums( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    AREA_DATA *pArea;
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    bool room, mob, obj, all, area_conflict;
    int low_range, high_range;

    room = FALSE;
    mob  = FALSE;
    obj  = FALSE;
    all  = FALSE;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (arg1[0] == '\0')
    {
      send_to_char("Please specify room, mob, object, or all as your first argument.\n\r", ch);
      return;
    }

    if(!str_cmp(arg1, "room"))
      room = TRUE;

    else if(!str_cmp(arg1, "mob"))
      mob = TRUE;

    else if(!str_cmp(arg1, "object"))
      obj = TRUE;
   
    else if(!str_cmp(arg1, "all"))
      all = TRUE;
    else 
    {
      send_to_char("Please specify room, mob, or object as your first argument.\n\r", ch);
      return;
    }

    if(arg2[0] == '\0')
    {
      send_to_char("Please specify the low end of the range to be searched.\n\r", ch);
      return;
    }

    if(argument[0] == '\0')
    {
      send_to_char("Please specify the high end of the range to be searched.\n\r", ch);
      return;
    }

    low_range = atoi(arg2);
    high_range = atoi(argument);

    if (low_range < 1 || low_range > 32767 )
    {
      send_to_char("Invalid argument for bottom of range.\n\r", ch);
      return;
    }

    if (high_range < 1 || high_range > 32767 )
    {
      send_to_char("Invalid argument for top of range.\n\r", ch);
      return;
    }
      
    if (high_range < low_range)
    {
      send_to_char("Bottom of range must be below top of range.\n\r", ch);
      return;
    }
    
    if (all)
    {
      sprintf(buf, "room %d %d", low_range, high_range);
      do_check_vnums(ch, buf);
      sprintf(buf, "mob %d %d", low_range, high_range);
      do_check_vnums(ch, buf);
      sprintf(buf, "object %d %d", low_range, high_range);
      do_check_vnums(ch, buf);
      return;
    }
    set_char_color( AT_PLAIN, ch );

    for ( pArea = first_asort; pArea; pArea = pArea->next_sort )
    {
        area_conflict = FALSE;
	if ( IS_SET( pArea->status, AREA_DELETED ) )
	   continue;
	else
	if (room)
	{
	  if ( low_range < pArea->low_r_vnum && pArea->low_r_vnum < high_range )
	    area_conflict = TRUE;

	  if ( low_range < pArea->hi_r_vnum && pArea->hi_r_vnum < high_range )
	    area_conflict = TRUE;

	  if ( ( low_range >= pArea->low_r_vnum )
	  && ( low_range <= pArea->hi_r_vnum ) )
	    area_conflict = TRUE;

	  if ( ( high_range <= pArea->hi_r_vnum )
	  && ( high_range >= pArea->low_r_vnum ) ) 
	    area_conflict = TRUE;
	}

	if (mob)
	{
	  if ( low_range < pArea->low_m_vnum && pArea->low_m_vnum < high_range )
	    area_conflict = TRUE;

	  if ( low_range < pArea->hi_m_vnum && pArea->hi_m_vnum < high_range )
	    area_conflict = TRUE;
	  if ( ( low_range >= pArea->low_m_vnum )
	  && ( low_range <= pArea->hi_m_vnum ) )
	    area_conflict = TRUE;

	  if ( ( high_range <= pArea->hi_m_vnum )
	  && ( high_range >= pArea->low_m_vnum ) )
	    area_conflict = TRUE;
	}

	if (obj)
	{
	  if ( low_range < pArea->low_o_vnum && pArea->low_o_vnum < high_range )
	    area_conflict = TRUE;

	  if ( low_range < pArea->hi_o_vnum && pArea->hi_o_vnum < high_range )
	    area_conflict = TRUE;

	  if ( ( low_range >= pArea->low_o_vnum )
	  && ( low_range <= pArea->hi_o_vnum ) )
	    area_conflict = TRUE;

	  if ( ( high_range <= pArea->hi_o_vnum )
	  && ( high_range >= pArea->low_o_vnum ) )
	    area_conflict = TRUE;
	}

	if (area_conflict)
	{
	sprintf(buf, "Conflict:%-15s| ",
		(pArea->filename ? pArea->filename : "(invalid)"));
        if(room)
          sprintf( buf2, "Rooms: %5d - %-5d\n\r", pArea->low_r_vnum, 
          pArea->hi_r_vnum);
        if(mob)
          sprintf( buf2, "Mobs: %5d - %-5d\n\r", pArea->low_m_vnum, 
          pArea->hi_m_vnum);
        if(obj)
          sprintf( buf2, "Objects: %5d - %-5d\n\r", pArea->low_o_vnum, 
          pArea->hi_o_vnum);
        
        strcat( buf, buf2 );
	send_to_char(buf, ch);
    	}
    }    
    for ( pArea = first_bsort; pArea; pArea = pArea->next_sort )
    {
        area_conflict = FALSE;
	if ( IS_SET( pArea->status, AREA_DELETED ) )
	   continue;
	else
	if (room)
	{
	  if ( low_range < pArea->low_r_vnum && pArea->low_r_vnum < high_range )
	    area_conflict = TRUE;

	  if ( low_range < pArea->hi_r_vnum && pArea->hi_r_vnum < high_range )
	    area_conflict = TRUE;

	  if ( ( low_range >= pArea->low_r_vnum )
	  && ( low_range <= pArea->hi_r_vnum ) )
	    area_conflict = TRUE;

	  if ( ( high_range <= pArea->hi_r_vnum )
	  && ( high_range >= pArea->low_r_vnum ) ) 
	    area_conflict = TRUE;
	}

	if (mob)
	{
	  if ( low_range < pArea->low_m_vnum && pArea->low_m_vnum < high_range )
	    area_conflict = TRUE;

	  if ( low_range < pArea->hi_m_vnum && pArea->hi_m_vnum < high_range )
	    area_conflict = TRUE;
	  if ( ( low_range >= pArea->low_m_vnum )
	  && ( low_range <= pArea->hi_m_vnum ) )
	    area_conflict = TRUE;

	  if ( ( high_range <= pArea->hi_m_vnum )
	  && ( high_range >= pArea->low_m_vnum ) )
	    area_conflict = TRUE;
	}

	if (obj)
	{
	  if ( low_range < pArea->low_o_vnum && pArea->low_o_vnum < high_range )
	    area_conflict = TRUE;

	  if ( low_range < pArea->hi_o_vnum && pArea->hi_o_vnum < high_range )
	    area_conflict = TRUE;

	  if ( ( low_range >= pArea->low_o_vnum )
	  && ( low_range <= pArea->hi_o_vnum ) )
	    area_conflict = TRUE;

	  if ( ( high_range <= pArea->hi_o_vnum )
	  && ( high_range >= pArea->low_o_vnum ) )
	    area_conflict = TRUE;
	}

	if (area_conflict)
	{
	sprintf(buf, "Conflict:%-15s| ",
		(pArea->filename ? pArea->filename : "(invalid)"));
        if(room)
          sprintf( buf2, "Rooms: %5d - %-5d\n\r", pArea->low_r_vnum, 
          pArea->hi_r_vnum);
        if(mob)
          sprintf( buf2, "Mobs: %5d - %-5d\n\r", pArea->low_m_vnum, 
          pArea->hi_m_vnum);
        if(obj)
          sprintf( buf2, "Objects: %5d - %-5d\n\r", pArea->low_o_vnum, 
          pArea->hi_o_vnum);
        
        strcat( buf, buf2 );
	send_to_char(buf, ch);
    	}
    }    

/*
    for ( pArea = first_asort; pArea; pArea = pArea->next_sort )
    {
        area_conflict = FALSE;
	if ( IS_SET( pArea->status, AREA_DELETED ) )
	   continue;
	else
	if (room)
	  if((pArea->low_r_vnum >= low_range) 
	  && (pArea->hi_r_vnum <= high_range))
	    area_conflict = TRUE;

	if (mob)
	  if((pArea->low_m_vnum >= low_range) 
	  && (pArea->hi_m_vnum <= high_range))
	    area_conflict = TRUE;

	if (obj)
	  if((pArea->low_o_vnum >= low_range) 
	  && (pArea->hi_o_vnum <= high_range))
	    area_conflict = TRUE;

	if (area_conflict)
	  ch_printf(ch, "Conflict:%-15s| Rooms: %5d - %-5d"
		     " Objs: %5d - %-5d Mobs: %5d - %-5d\n\r",
		(pArea->filename ? pArea->filename : "(invalid)"),
		pArea->low_r_vnum, pArea->hi_r_vnum,
		pArea->low_o_vnum, pArea->hi_o_vnum,
		pArea->low_m_vnum, pArea->hi_m_vnum );
    }

    for ( pArea = first_bsort; pArea; pArea = pArea->next_sort )
    {
        area_conflict = FALSE;
	if ( IS_SET( pArea->status, AREA_DELETED ) )
	   continue;
	else
	if (room)
	  if((pArea->low_r_vnum >= low_range) 
	  && (pArea->hi_r_vnum <= high_range))
	    area_conflict = TRUE;

	if (mob)
	  if((pArea->low_m_vnum >= low_range) 
	  && (pArea->hi_m_vnum <= high_range))
	    area_conflict = TRUE;

	if (obj)
	  if((pArea->low_o_vnum >= low_range) 
	  && (pArea->hi_o_vnum <= high_range))
	    area_conflict = TRUE;

	if (area_conflict)
	  sprintf(ch, "Conflict:%-15s| Rooms: %5d - %-5d"
		     " Objs: %5d - %-5d Mobs: %5d - %-5d\n\r",
		(pArea->filename ? pArea->filename : "(invalid)"),
		pArea->low_r_vnum, pArea->hi_r_vnum,
		pArea->low_o_vnum, pArea->hi_o_vnum,
		pArea->low_m_vnum, pArea->hi_m_vnum );
    }
*/
    return;
}

/*
 * Find what area a room vnum is in --GW
 * ability to search proto areas added --GW
 */
AREA_DATA *find_room_area( int vnum )
{
AREA_DATA *temp;
AREA_DATA *temp_next;

for( temp = first_build; temp; temp = temp_next )
{
temp_next = temp->next;

if ( ( temp->low_r_vnum <= vnum ) &&
     ( temp->hi_r_vnum >= vnum ) )
     return temp;
}

for( temp = first_area; temp; temp = temp_next )
{
temp_next = temp->next;

if ( ( temp->low_r_vnum <= vnum ) &&
     ( temp->hi_r_vnum >= vnum ) )
     return temp;
}

return first_area;
}

/*
 * Find what area an object vnum is in --GW
 * ability to search proto areas added --GW
 */
AREA_DATA *find_obj_area( int vnum )
{
AREA_DATA *temp;
AREA_DATA *temp_next;

for( temp = first_build; temp; temp = temp_next )
{
temp_next = temp->next;

if ( ( temp->low_o_vnum <= vnum ) &&
     ( temp->hi_o_vnum >= vnum ) )
     return temp;
}

for( temp = first_area; temp; temp = temp_next )
{
temp_next = temp->next;

if ( ( temp->low_o_vnum <= vnum ) &&
     ( temp->hi_o_vnum >= vnum ) )
     return temp;
}

return first_area;
}

/*
 * Find what area a mob vnum is in --GW
 * ability to search proto areas added --GW
 */
AREA_DATA *find_mob_area( int vnum )
{
AREA_DATA *temp;
AREA_DATA *temp_next;

for( temp = first_build; temp; temp = temp_next )
{
temp_next = temp->next;

if ( ( temp->low_m_vnum <= vnum ) &&
     ( temp->hi_m_vnum >= vnum ) )
     return temp;
}


for( temp = first_area; temp; temp = temp_next )
{
temp_next = temp->next;

if ( ( temp->low_m_vnum <= vnum ) &&
     ( temp->hi_m_vnum >= vnum ) )
     return temp;
}



return first_area;
}

/*
 * This function is here to aid in debugging.
 * If the last expression in a function is another function call,
 *   gcc likes to generate a JMP instead of a CALL.
 * This is called "tail chaining."
 * It hoses the debugger call stack for that call.
 * So I make this the last call in certain critical functions,
 *   where I really need the call stack to be right for debugging!
 *
 * If you don't understand this, then LEAVE IT ALONE.
 * Don't remove any calls to tail_chain anywhere.
 *
 * -- Furey
 */
void tail_chain( void )
{
    return;
}
