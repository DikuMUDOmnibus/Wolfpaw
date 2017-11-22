/****************************************************************************e
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG (C) 1994, 1995, 1996 by Derek Snider                 |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Narn, Scryn, Haus, Swordbearer,   |   / ' ' \   *
 * Altrag, Grishnakh and Tricops                              |~'~.VxvxV.~'~*
 * ------------------------------------------------------------------------ *
 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 * ------------------------------------------------------------------------ *
 *			    Main mud header file			    *
 ****************************************************************************/

#include <stdlib.h>
#include <limits.h>
#include <sys/cdefs.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>

/*
#include <malloc.h> 
*/
typedef	int				ch_ret;
typedef	int				obj_ret;

/*
 * Accommodate old non-Ansi compilers.
 */
#if defined(TRADITIONAL)
#define const
#define args( list )			( )
#define DECLARE_DO_FUN( fun )		void fun( )
#define DECLARE_SPEC_FUN( fun )		bool fun( )
#define DECLARE_SPELL_FUN( fun )	ch_ret fun( )
#else
#define args( list )			list
#define DECLARE_DO_FUN( fun )		DO_FUN    fun
#define DECLARE_SPEC_FUN( fun )		SPEC_FUN  fun
#define DECLARE_SPELL_FUN( fun )	SPELL_FUN fun
#endif


/*
 * Short scalar types.
 * Diavolo reports AIX compiler has bugs with short types.
 */
#if	!defined(FALSE)
#define FALSE	 0
#endif

#if	!defined(TRUE)
#define TRUE	 1
#endif

#if	!defined(BERR)
#define BERR	 255
#endif

#if	defined(_AIX)
#if	!defined(const)
#define const
#endif
typedef int				sh_int;
typedef int				bool;
#define unix
#else
typedef short    int			sh_int;
typedef unsigned char			bool;
#endif

/*
  * Structure types.
 */
typedef struct  disabled_data           DISABLED_DATA;
typedef struct	affect_data		AFFECT_DATA;
typedef struct	area_data		AREA_DATA;
typedef struct  auction_data            AUCTION_DATA; /* auction data */
typedef struct	ban_data		BAN_DATA;
typedef struct	ban_data		PERMBAN_DATA;
typedef struct	extracted_char_data	EXTRACT_CHAR_DATA;
typedef struct	char_data		CHAR_DATA;
typedef struct	hunt_hate_fear		HHF_DATA;
typedef struct	fighting_data		FIGHT_DATA;
typedef struct	descriptor_data		DESCRIPTOR_DATA;
typedef struct	exit_data		EXIT_DATA;
typedef struct	extra_descr_data	EXTRA_DESCR_DATA;
typedef struct	help_data		HELP_DATA;
typedef struct	menu_data		MENU_DATA;
typedef struct	mob_index_data		MOB_INDEX_DATA;
typedef struct	note_data		NOTE_DATA;
typedef struct	mail_data		MAIL_DATA;
typedef struct	mode_data		MODE_DATA;
typedef struct	comment_data		COMMENT_DATA;
typedef struct	board_data		BOARD_DATA;
typedef struct	obj_data		OBJ_DATA;
typedef struct  zone_data		ZONE_DATA;
typedef struct  refferal_data		REFFERAL_DATA;
typedef struct  kombat_data		KOMBAT_DATA;
typedef struct  colormap_data		COLORMAP_DATA;
typedef struct  cmap_data		CMAP_DATA;
typedef struct  personalsocial_data	PSOCIAL_DATA;
typedef struct  psocial_data		PSOC_DATA;
typedef struct  offline_data		OFFLINE_DATA;
typedef struct  limit_data		LIMIT_DATA;
typedef struct  vehicle_data		VEHICLE_DATA;
typedef struct  petition_data		PETITION_DATA;
typedef struct  acc_data		ACC_DATA;
typedef struct  challenge_data		CHALLENGE_DATA;
typedef struct  arena_data		ARENA_DATA;
typedef struct  warrent_data		WARRENT_DATA;
typedef struct	obj_index_data		OBJ_INDEX_DATA;
typedef struct	pc_data			PC_DATA;
typedef struct	reset_data		RESET_DATA;
typedef struct	map_index_data		MAP_INDEX_DATA;   /* maps */
typedef struct	map_data		MAP_DATA;   /* maps */
typedef struct	room_index_data		ROOM_INDEX_DATA;
typedef struct	shop_data		SHOP_DATA;
typedef struct	repairshop_data		REPAIR_DATA;
typedef struct	time_info_data		TIME_INFO_DATA;
typedef struct	hour_min_sec		HOUR_MIN_SEC;
typedef struct	weather_data		WEATHER_DATA;
typedef	struct	clan_data		CLAN_DATA;
typedef struct  council_data 		COUNCIL_DATA;
typedef struct  tourney_data            TOURNEY_DATA;
typedef struct	mob_prog_data		MPROG_DATA;
typedef struct	mob_prog_act_list	MPROG_ACT_LIST;
typedef	struct	editor_data		EDITOR_DATA;
typedef struct	teleport_data		TELEPORT_DATA;
typedef struct	timer_data		TIMER;
typedef struct  godlist_data		GOD_DATA;
typedef struct	system_data		SYSTEM_DATA;
typedef	struct	smaug_affect		SMAUG_AFF;
typedef struct  who_data                WHO_DATA;
typedef	struct	skill_type		SKILLTYPE;
typedef	struct	social_type		SOCIALTYPE;
typedef	struct	cmd_type		CMDTYPE;
typedef	struct	killed_data		KILLED_DATA;
typedef struct  deity_data		DEITY_DATA;
typedef struct	wizent			WIZENT;
typedef struct  extended_bitvector	EXT_BV;
typedef struct  web_descriptor 		WEB_DESCRIPTOR; 
typedef struct  pk_data			PK_DATA;
typedef struct  pk_list			PK_LIST;
typedef struct  approve_data		APPROVE_DATA;
typedef struct  topten_data		TOPTEN_DATA;
typedef struct  host_data		HOST_DATA;
typedef struct	secure_data		SECURE_DATA;
typedef struct	secure_name		SECURE_NAME;

/*
 * Function types.
 */
typedef	void	DO_FUN		args( ( CHAR_DATA *ch, char *argument ) );
typedef bool	SPEC_FUN	args( ( CHAR_DATA *ch ) );
typedef ch_ret	SPELL_FUN	args( ( int sn, int level, CHAR_DATA *ch, void *vo ) );

#define DUR_CONV	23.333333333333333333333333
#define HIDDEN_TILDE	'*'

#define BV00		(1 <<  0)
#define BV01		(1 <<  1)
#define BV02		(1 <<  2)
#define BV03		(1 <<  3)
#define BV04		(1 <<  4)
#define BV05		(1 <<  5)
#define BV06		(1 <<  6)
#define BV07		(1 <<  7)
#define BV08		(1 <<  8)
#define BV09		(1 <<  9)
#define BV10		(1 << 10)
#define BV11		(1 << 11)
#define BV12		(1 << 12)
#define BV13		(1 << 13)
#define BV14		(1 << 14)
#define BV15		(1 << 15)
#define BV16		(1 << 16)
#define BV17		(1 << 17)
#define BV18		(1 << 18)
#define BV19		(1 << 19)
#define BV20		(1 << 20)
#define BV21		(1 << 21)
#define BV22		(1 << 22)
#define BV23		(1 << 23)
#define BV24		(1 << 24)
#define BV25		(1 << 25)
#define BV26		(1 << 26)
#define BV27		(1 << 27)
#define BV28		(1 << 28)
#define BV29		(1 << 29)
#define BV30		(1 << 30)
#define BV31		(1 << 31)
/* 32 USED! DO NOT ADD MORE! SB */

/* Menu Looks like ... --GW*/
#define MENU         \
"&W\n\r\x1B[7mWelcome to [SOMEONE DIDNT CHANGE THIS :)]!\x1B[0m\n\r\n\
0] Exit the server.\n\r\
1] Enter the game.\n\r\
   Pick one: "

#define SLASH "\\"  	/* SLASH needed by MSP routine */

#define DEATH_MENU	\
"&R	YOU HAVE BEEN KILLED!\n\r\n\
&BYou have 2 Choices here:\n\r\n\
Type 'Y' to Ressurect yourself, and lose 1 Con point.\n\r\
or\n\r\
Type 'N' to Not Ressurect yourself, and lose 1 Level.\n\r\n\
NOTE: Loss of Link at this time will cause you to NOT Ressurect\n\r\n\
Make your Selection: "

/* Moved from old clans.c so all files can use them --GW */
#define MAX_NEST 	100
CLAN_DATA * first_clan;
CLAN_DATA * last_clan;
COUNCIL_DATA * first_council;
COUNCIL_DATA * last_council;

/*
 * String and memory management parameters.
 */
#define MAX_KEY_HASH		 2048
#define MAX_STRING_LENGTH	 4096  /* buf */
#define MAX_INPUT_LENGTH	 1024  /* arg */
#define MAX_INBUF_SIZE		 1024

#define HASHSTR			 /* use string hashing */
/* in make file now */

#define	MAX_LAYERS		 8	/* maximum clothing layers */

#define MAX_KILLTRACK		25	/* track mob vnums killed */

/* Requests */
#define REQUESTS

/* Ctf --GW */
/* Guard Vnums */ 
#define CTF_BLUE_GUARD 21441
#define CTF_RED_GUARD 21440
/* Base Rooms */
#define CTF_RED_BASE 21321
#define CTF_BLUE_BASE 21307
/* Teams */
#define TEAM_RED        	1
#define TEAM_BLUE       	2   
#define RED_FLAG_CARRIER	3
#define BLUE_FLAG_CARRIER	4
/* Flags */
#define CTF_RED_FLAG	21400
#define CTF_BLUE_FLAG	21401
/*
 * Psionicist gsn's (by Thelonius).
 */
extern  int     gsn_chameleon;
extern  int     gsn_domination;
extern  int     gsn_heighten;
extern  int     gsn_shadow;

/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 * adjust them in the DB (wolfpaw.dat) .. here doesnt matter for
 * the ones marked DB.. it changes them. --GW
 */
#define MAXDATA		     1024
#define MSL                  MAX_STRING_LENGTH
#define MIL                  MAX_INPUT_LENGTH
#define RESTART_COMMAND	     "/home/aota/aoa/src/AOTA"
#define MAX_EXP_WORTH	     160000000
#define MIN_EXP_WORTH		    1

#define MAX_REXITS		   20	/* Maximum exits allowed in 1 room */
#define MAX_SKILL		  400   /* DB Loaded */
#define MAX_CLASS           	   28   /* Increased to 28 for Mystic -GW*/
#define MAX_NPC_CLASS		   44
#define MAX_RACE		   24 /* Increased to 24 for new races GW*/
#define MAX_NPC_RACE		   96
#define MAX_LEVEL		   60
#define MAX_MOB_LEVEL		   200
#define MAX_ALIAS		   20 /* max alias */
#define MAX_IGNORE		   20 /* max ignore */
#define MAX_CLAN		   50
#define MAX_DEITY		   50
#define MAX_CPD			   16   /* Maximum council power level difference */
#define MAX_COM			   10  /* maximum commands kept (for history)*/
#define	MAX_HERB		   20
#define WEBPORT			   5002
#define MASTER_PASSWORD		   "GrKo6TakNHaFc"

#define LEVEL_OVERSEER		    MAX_LEVEL
#define LEVEL_HYPERION		   (MAX_LEVEL-1)
#define LEVEL_COUNCIL		   (MAX_LEVEL-2)
#define LEVEL_HEAD_GOD		   (MAX_LEVEL-3)
#define LEVEL_GOD		   (MAX_LEVEL-4)
#define LEVEL_FATE		   (MAX_LEVEL-5)
#define LEVEL_TITAN		   (MAX_LEVEL-6)
#define LEVEL_LORD		   (MAX_LEVEL-7)
#define LEVEL_WIZARD		   (MAX_LEVEL-8)
#define LEVEL_EMPATH		   (MAX_LEVEL-9)
#define LEVEL_IMMORTAL		   (MAX_LEVEL-9)
#define LEVEL_AVATAR		   (MAX_LEVEL-10)

#define LEVEL_LOG		    LEVEL_FATE

/*#define PULSE_PER_SECOND	    4 -- in comm.c. now */
#define PULSE_VIOLENCE		  ( 3 * PULSE_PER_SECOND)
#define PULSE_MOBILE		  ( 4 * PULSE_PER_SECOND)
#define PULSE_TICK		  (35 * PULSE_PER_SECOND)
#define PULSE_AREA		  (60 * PULSE_PER_SECOND)
#define PULSE_AUCTION             (10 * PULSE_PER_SECOND)

#define send_to_char send_to_char_color
#define send_to_pager send_to_pager_color

/*
 * Command logging types.
 */
typedef enum
{
  LOG_NORMAL, LOG_ALWAYS, LOG_NEVER, LOG_BUILD, LOG_HIGH, LOG_COMM, LOG_ALL
} log_types;

/*
 * Return types for move_char, damage, greet_trigger, etc, etc
 * Added by Thoric to get rid of bugs
 */
typedef enum
{
  rNONE, rCHAR_DIED, rVICT_DIED, rBOTH_DIED, rCHAR_QUIT, rVICT_QUIT,
  rBOTH_QUIT, rSPELL_FAILED, rOBJ_SCRAPPED, rOBJ_EATEN, rOBJ_EXPIRED,
  rOBJ_TIMER, rOBJ_SACCED, rOBJ_QUAFFED, rOBJ_USED, rOBJ_EXTRACTED,
  rOBJ_DRUNK, rCHAR_IMMUNE, rVICT_IMMUNE, rCHAR_AND_OBJ_EXTRACTED = 128,
  rERROR = 255
} ret_types;

/* Echo types for echo_to_all */
#define ECHOTAR_ALL	0
#define ECHOTAR_PC	1
#define ECHOTAR_IMM	2
#define ECHOTAR_SOUND   3

/* defines for new do_who */
#define WT_MORTAL 0
#define WT_DEADLY 1
#define WT_IMM    2

/*
 * Defines for extended bitvectors
 */
#ifndef INTBITS
  #define INTBITS       32
#endif
#define XBM             31      /* extended bitmask   ( INTBITS - 1 )   */
#define RSV             5       /* right-shift value  ( sqrt(XBM+1) )   */
#define XBI             4       /* integers in an extended bitvector    */
#define MAX_BITS	XBI * INTBITS

/*
 * Structure for extended bitvectors -- Thoric
 */
struct extended_bitvector
{
    int         bits[XBI];
};


/*
 * do_who output structure -- Narn
 */ 
struct who_data
{
  WHO_DATA *prev;
  WHO_DATA *next;
  char *text;
  int  type;
  int ic;
};

 
/*
 * Perm Site ban structure.
 */
/* ummm no, i don't think so.. what the hell is the point of this????
   it's exactly the same as ban_data!!! pointless pointless pointless...
   just add things to ban_data if needed for permbans, and ignore the
   extra fields when doing normal bans... -- TRI
struct  permban_data
{
    PERMBAN_DATA *  next;
    PERMBAN_DATA *	prev;
    char *	name;
    int		level;
    char *	ban_time;
    char *	ban_issuer;
};
*/


/*
 * Site ban structure.
 */
struct	ban_data
{
    BAN_DATA *	next;
    BAN_DATA *	prev;
    char *	name;
    int		level;
    char *	ban_time;
    char *	ban_issuer;
};


/*
 * Time and weather stuff.
 */
typedef enum
{
  SUN_DARK, SUN_RISE, SUN_LIGHT, SUN_SET
} sun_positions;

typedef enum
{
  SKY_CLOUDLESS, SKY_CLOUDY, SKY_RAINING, SKY_LIGHTNING
} sky_conditions;

struct	time_info_data
{
    int		hour;
    int		day;
    int		month;
    int		year;
};

struct hour_min_sec
{
  int hour;
  int min;
  int sec;
  int manual;
};

struct	weather_data
{
    int		mmhg;
    int		change;
    int		sky;
    int		sunlight;
};


/*
 * Structure used to build wizlist
 */
struct	wizent
{
    WIZENT *		next;
    WIZENT *		last;
    char *		name;
    sh_int		level;
    int			flags;
};


/*
 * Connected state for a channel.
 */
typedef enum
{
   CON_GET_NAME = -100,
   CON_GATEWAY,
   CON_GET_ACC,
   CON_GET_ACC_PASS,
   CON_GET_OLD_PASSWORD,
   CON_CONFIRM_NEW_NAME,
   CON_GET_NEW_PASSWORD,
   CON_CONFIRM_NEW_PASSWORD,
   CON_GET_NEW_SEX,
   CON_GET_NEW_CLASS,
   CON_READ_MOTD,
   CON_GET_NEW_RACE,
   CON_GET_EMULATION,
   CON_GET_WANT_RIPANSI,
   CON_TITLE,
   CON_PRESS_ENTER,
   CON_WAIT_1,
   CON_WAIT_2,
   CON_WAIT_3,
   CON_ACCEPTED,
   CON_GET_PKILL,
   CON_READ_IMOTD,
   CON_COPYOVER_RECOVER,
   CON_GET_NEW_REMORT_RACE,
   CON_GET_NEW_REMORT_CLASS,
   CON_PLAYER_MENU,
   CON_GET_SELECTION,
   CON_CHANGE_PASSWORD,
   CON_DELETE_CHAR,
   CON_CONFIRM_CHANGED_PASSWORD,
   CON_CONFIRM_PASS,
   CON_CONFIRM_DELETE,
   CON_GET_EMAIL,
   CON_GET_DEITY,
   CON_CONFIRM_NEW_RACE,
   CON_CONFIRM_NEW_CLASS,
   CON_GET_REFFERAL,
   CON_DEATH_SELECT,
   CON_GREDITING,
   CON_NMOTD_CONFIRM,
   CON_GET_NAME_ENTER,
   CON_PLAYING = 1,
   CON_EDITING = -1
} connection_types;

/*
 * Character substates
 */
typedef enum
{
  SUB_NONE, SUB_PAUSE, SUB_PERSONAL_DESC, SUB_OBJ_SHORT, SUB_OBJ_LONG,
  SUB_OBJ_EXTRA, SUB_MOB_LONG, SUB_MOB_DESC, SUB_ROOM_DESC, SUB_ROOM_EXTRA,
  SUB_ROOM_EXIT_DESC, SUB_WRITING_NOTE, SUB_MPROG_EDIT, SUB_HELP_EDIT,
  SUB_WRITING_MAP, SUB_PERSONAL_BIO, SUB_REPEATCMD, SUB_RESTRICTED,
  SUB_DEITYDESC, SUB_REDIT_MAIN, SUB_REDIT_RNAME, SUB_REDIT_ROOMDESC,
  SUB_REDIT_ROOMFLAGS, SUB_REDIT_SECTOR, SUB_REDIT_EXIT_N,
  SUB_REDIT_EXIT_E, SUB_REDIT_EXIT_S, SUB_REDIT_EXIT_W, SUB_REDIT_EXIT_U,
  SUB_REDIT_EXIT_D, SUB_REDIT_EXTRA_DESC,
  /* timer types ONLY below this point */
  SUB_TIMER_DO_ABORT = 128, SUB_TIMER_CANT_ABORT
} char_substates;

/*
 * Descriptor (channel) structure.
 */
struct	descriptor_data
{
    DESCRIPTOR_DATA *	next;
    DESCRIPTOR_DATA *	prev;
    DESCRIPTOR_DATA *	snoop_by;
    CHAR_DATA *		character;
    CHAR_DATA *		original;
    char *		host;
    int			port;
    int			descriptor;
    sh_int		connected;
    sh_int		idle;
    sh_int		lines;
    sh_int		scrlen;
    bool		fcommand;
    char		inbuf		[MAX_INBUF_SIZE];
    char		incomm		[MAX_INPUT_LENGTH];
    char		inlast		[MAX_INPUT_LENGTH];
    int			repeat;
    char *		outbuf;
    unsigned long	outsize;
    int			outtop;
    char *		pagebuf;
    unsigned long	pagesize;
    int			pagetop;
    char *		pagepoint;
    char		pagecmd;
    char		pagecolor;
    int			auth_inc;
    int			auth_state;
    char		abuf[ 256 ];
    int			auth_fd;
    char *		user;
    int 		atimes;
    int			newstate;
    unsigned char	prevcolor;
};



/*
 * Attribute bonus structures.
 */
struct	str_app_type
{
    sh_int	tohit;
    sh_int	todam;
    sh_int	carry;
    sh_int	wield;
};

struct	int_app_type
{
    sh_int	learn;
};

struct	wis_app_type
{
    sh_int	practice;
};

struct	dex_app_type
{
    sh_int	defensive;
};

struct	con_app_type
{
    sh_int	hitp;
    sh_int	shock;
};

struct	cha_app_type
{
    sh_int	charm;
};

struct  lck_app_type
{
    sh_int	luck;
};

/* the races */
#define RACE_HUMAN	    0
#define RACE_ELF            1
#define RACE_DWARF          2
#define RACE_HALFLING       3
#define RACE_PIXIE          4
#define RACE_VAMPIRE        5
#define RACE_HALF_OGRE      6
#define RACE_HALF_ORC       7
#define RACE_HALF_TROLL     8
#define RACE_HALF_ELF       9
#define RACE_GITH           10
#define RACE_WAREWOLF	    11
#define RACE_DRACONIAN      12
#define RACE_SATYR	    13
#define RACE_WRAITH	    14
#define RACE_CENTAUR        15
#define RACE_DRIDER         16
#define RACE_DROW           17
#define RACE_MINOTAUR       18
#define RACE_CYCLOPS        19
#define RACE_AREWYNDEL      20
#define RACE_BROWNIE        21
#define RACE_LEPRACHAUN    22
#define RACE_MRRSHAN        23

/* npc races */
#define	RACE_DRAGON	    31

#define CLASS_NONE	   -1 /* For skill/spells according to guild */
#define CLASS_MAGE	    0
#define CLASS_CLERIC	    1
#define CLASS_THIEF	    2
#define CLASS_WARRIOR	    3
#define CLASS_VAMPIRE	    4
#define CLASS_DRUID	    5
#define CLASS_RANGER	    6
#define CLASS_AUGURER	    7 /* 7-7-96 SB */
#define CLASS_PALADIN       8
#define CLASS_ASSASSIN      9
#define CLASS_WEREWOLF     10
#define CLASS_KINJU	   11
#define CLASS_AVATAR       12
#define CLASS_PSIONICIST   13
#define CLASS_DRAGON       14
#define CLASS_BERSERKER    15
#define CLASS_ARCHMAGE	   16
#define CLASS_BISHOP       17
#define CLASS_ALCHEMIST    18
#define CLASS_PROPHET      19
#define CLASS_CRUSADER     20
#define CLASS_SAMURAI      21
#define CLASS_WRAITH       22
#define CLASS_HUNTER	   23
#define CLASS_ADEPT        24
#define CLASS_NINJA	   25
#define CLASS_JACKEL       26
#define CLASS_MYSTIC	   27

/*
 * Languages -- Altrag
 */
#define LANG_COMMON      BV00  /* Human base language */
#define LANG_ELVEN       BV01  /* Elven base language */
#define LANG_DWARVEN     BV02  /* Dwarven base language */
#define LANG_PIXIE       BV03  /* Pixie/Fairy base language */
#define LANG_OGRE        BV04  /* Ogre base language */
#define LANG_ORCISH      BV05  /* Orc base language */
#define LANG_TROLLISH    BV06  /* Troll base language */
#define LANG_LEPRACHAUN  BV07  /* Was Rodent munched cuase it's never used */
#define LANG_INSECTOID   BV08  /* Insects */
#define LANG_MAMMAL      BV09  /* Larger mammals */
#define LANG_MRRSHAN     BV10  /* Was Reptile munched for above reason */
#define LANG_DRAGON      BV11  /* Large reptiles, Dragons */
#define LANG_SPIRITUAL   BV12  /* Necromancers or undeads/spectres */
#define LANG_MAGICAL     BV13  /* Spells maybe?  Magical creatures */
#define LANG_GOBLIN      BV14  /* Goblin base language */
#define LANG_GOD         BV15  /* Clerics possibly?  God creatures */
#define LANG_ANCIENT     BV16  /* Prelude to a glyph read skill? */
#define LANG_HALFLING    BV17  /* Halfling base language */
#define LANG_CLAN	 BV18  /* Clan language */
#define LANG_GITH	 BV19  /* Gith Language */
#define LANG_VAMPIRIC    BV20  /* Vampire Language */
#define LANG_WOLFISH     BV21  /* Werewolf Language */
#define LANG_DRACONIAN   BV22  /* Draconian Language */
#define LANG_SATYR       BV23  /* What is a satyr anyway */
#define LANG_WRAITH      BV24  /* Wraith Language */
#define LANG_CENTAUR     BV25  /* Centaur Language */
#define LANG_DRIDER      BV26  /* Drider Language */
#define LANG_DROWISH     BV27  /* Dark Elf Language */
#define LANG_MINOTAUR    BV28  /* Minotaur Language */
#define LANG_CYCLOPS     BV29  /* Cyclops Language */
#define LANG_AREWYNDEL   BV30  /* Arewyndel Language */
#define LANG_BROWNIE     BV31  /* Brownie Language */
#define LANG_UNKNOWN        0  /* Anything that doesnt fit a category */

/*
 * This Wrapping error every compile is pising me off .. fixed it. -- GW
 */

#define V_L_1 ( LANG_COMMON | LANG_ELVEN | LANG_DWARVEN | LANG_PIXIE )
#define V_L_2 ( LANG_OGRE | LANG_ORCISH | LANG_TROLLISH | LANG_LEPRACHAUN )
#define V_L_3 ( LANG_MRRSHAN | LANG_GOBLIN | LANG_HALFLING | LANG_GITH )
#define V_L_4 ( LANG_VAMPIRIC | LANG_WOLFISH | LANG_DRACONIAN | LANG_SATYR )
#define V_L_5 ( LANG_WRAITH | LANG_CENTAUR | LANG_DRIDER | LANG_DROWISH )
#define V_L_6 ( LANG_MINOTAUR | LANG_CYCLOPS | LANG_AREWYNDEL | LANG_BROWNIE )
#define V_L_7 ( LANG_LEPRACHAUN | LANG_MRRSHAN )
#define VALID_LANGS ( V_L_1 | V_L_2 | V_L_3 | V_L_4 | V_L_5 | V_L_6 | V_L_7 )
/* 26 Languages */

/*
 * TO types for act.
 */
#define TO_ROOM		    0
#define TO_NOTVICT	    1
#define TO_VICT		    2
#define TO_CHAR		    3

/*
 * Real action "TYPES" for act.
 */
#define AT_BLACK	    0
#define AT_BLOOD	    1
#define AT_DGREEN           2
#define AT_ORANGE	    3
#define AT_DBLUE	    4
#define AT_PURPLE	    5
#define AT_CYAN	  	    6
#define AT_GREY		    7
#define AT_DGREY	    8
#define AT_RED		    9
#define AT_GREEN	   10
#define AT_YELLOW	   11
#define AT_BLUE		   12
#define AT_PINK		   13
#define AT_LBLUE	   14
#define AT_WHITE	   15
#define AT_BLINK	   16
#define AT_PLAIN	   AT_GREY
#define AT_ACTION	   AT_GREY
#define AT_SAY		   AT_LBLUE
#define AT_GOSSIP	   AT_LBLUE
#define AT_YELL	           AT_WHITE
#define AT_TELL		   AT_WHITE
#define AT_HIT		   AT_GREEN
#define AT_HITME	   AT_RED
#define AT_IMMORT	   AT_YELLOW
#define AT_HURT		   AT_RED + AT_BLINK
#define AT_FALLING	   AT_WHITE + AT_BLINK
#define AT_DANGER	   AT_RED + AT_BLINK
#define AT_MAGIC	   AT_BLUE
#define AT_CONSIDER	   AT_GREY
#define AT_REPORT	   AT_GREY
#define AT_POISON	   AT_GREEN
#define AT_SOCIAL	   AT_CYAN
#define AT_DYING	   AT_YELLOW
#define AT_DEAD		   AT_RED
#define AT_SKILL	   AT_GREEN
#define AT_CARNAGE	   AT_BLOOD
#define AT_DAMAGE	   AT_WHITE
#define AT_FLEE		   AT_YELLOW
#define AT_RMNAME	   AT_WHITE
#define AT_RMDESC	   AT_YELLOW
#define AT_OBJECT	   AT_GREEN
#define AT_PERSON	   AT_PINK
#define AT_LIST		   AT_BLUE
#define AT_BYE		   AT_GREEN
#define AT_GOLD		   AT_YELLOW
#define AT_GTELL	   AT_BLUE
#define AT_NOTE		   AT_GREEN
#define AT_HUNGRY	   AT_ORANGE
#define AT_THIRSTY	   AT_BLUE
#define	AT_FIRE		   AT_RED
#define AT_SOBER	   AT_WHITE
#define AT_WEAROFF	   AT_YELLOW
#define AT_EXITS	   AT_WHITE
#define AT_SCORE	   AT_LBLUE
#define AT_RESET	   AT_DGREEN
#define AT_LOG		   AT_PURPLE
#define AT_DIEMSG	   AT_WHITE
#define AT_WARTALK         AT_RED

/* INVERSE code .. and UNINVERSE --GW */
#define INVERSE		   "\x1B[7m"
#define UINVERSE	   "\x1B[0m"

#define INIT_WEAPON_CONDITION    12
#define MAX_ITEM_IMPACT		 30

/* Martial Arts 'Ratings' */
#define MA_WHITE_SASH		0
#define MA_WHITE_YELLOW_SASH	1
#define MA_YELLOW_SASH		2
#define MA_YELLOW_GREEN_SASH	3
#define MA_GREEN_SASH		4
#define MA_GREEN_BLUE_SASH	5
#define MA_BLUE_SASH		6
#define MA_BLUE_RED_SASH	7
#define MA_RED_SASH		8
#define MA_RED_BLACK_SASH	9
#define MA_BLACK_SASH		10
#define MA_APPRENTICE_SASH	11
#define MA_ADEPT_SASH 12
#define MA_MASTERY_SASH		13

/*
 * Help table types.
 */
struct	help_data
{
    HELP_DATA *	next;
    HELP_DATA * prev;
    sh_int	level;
    char *	keyword;
    char *	text;
    char *      update;
};



/*
 * Shop types.
 */
#define MAX_TRADE	 5

struct	shop_data
{
    SHOP_DATA *	next;			/* Next shop in list		*/
    SHOP_DATA * prev;			/* Previous shop in list	*/
    int		keeper;			/* Vnum of shop keeper mob	*/
    sh_int	buy_type [MAX_TRADE];	/* Item types shop will buy	*/
    sh_int	profit_buy;		/* Cost multiplier for buying	*/
    sh_int	profit_sell;		/* Cost multiplier for selling	*/
    sh_int	open_hour;		/* First opening hour		*/
    sh_int	close_hour;		/* First closing hour		*/
};

#define MAX_FIX		4
#define SHOP_FIX	1
#define SHOP_RECHARGE	2

struct	repairshop_data
{
    REPAIR_DATA * next;			/* Next shop in list		*/
    REPAIR_DATA * prev;			/* Previous shop in list	*/
    int		  keeper;		/* Vnum of shop keeper mob	*/
    sh_int	  fix_type [MAX_FIX];	/* Item types shop will fix	*/
    sh_int	  profit_fix;		/* Cost multiplier for fixing	*/
    sh_int	  shop_type;		/* Repair shop type		*/
    sh_int	  open_hour;		/* First opening hour		*/
    sh_int	  close_hour;		/* First closing hour		*/
};


/* Mob program structures */
struct  act_prog_data
{
    struct act_prog_data *next;
    void *vo;
};

struct	mob_prog_act_list
{
    MPROG_ACT_LIST * next;
    char *	     buf;
    CHAR_DATA *      ch;
    OBJ_DATA *	     obj;
    void *	     vo;
};

struct	mob_prog_data
{
    MPROG_DATA * next;
    int		 type;
    bool	 triggered;
    int		 resetdelay;
    char *	 arglist;
    char *	 comlist;
};

bool	MOBtrigger;

#define CLASS_TYPE_MAGIC	0
#define CLASS_TYPE_FIGHTER	1
#define CLASS_TYPE_EXCLUDE	2

/* Ac types */
#define AC_TYPE_PURE_FIGHTER	0
#define AC_TYPE_PURE_MAGIC_USER 1
#define AC_TYPE_HALFLING	2

/*
 * Per-class stuff.
 */
struct	class_type
{
    char *	who_name;		/* Name for 'who'		*/
    char *      filename;		/* Filename --GW 		*/
    sh_int	attr_prime;		/* Prime attribute		*/
    int		weapon;			/* First weapon			*/
    int		guild;			/* Vnum of guild room		*/
    sh_int	skill_adept;		/* Maximum skill level		*/
    sh_int	thac0_00;		/* Thac0 for level  0		*/
    sh_int	thac0_32;		/* Thac0 for level 32		*/
    sh_int	hp_min;			/* Min hp gained on leveling	*/
    sh_int	hp_max;			/* Max hp gained on leveling	*/
    bool	fMana;			/* Class gains mana on level	*/
    sh_int	exp_base;		/* Class base exp		*/
    sh_int	type;			/* Class Type (Magic/Fighter) --GW */
};

/* race dedicated stuff */
struct	race_type
{
    char *	race_name;		/* Race name			*/
    EXT_BV	affected;		/* Default affect bitvectors	*/
    int		number;			/* race number */
    sh_int	str_plus;		/* Str bonus/penalty		*/
    sh_int	dex_plus;		/* Dex      "			*/
    sh_int	wis_plus;		/* Wis      "			*/
    sh_int	int_plus;		/* Int      "			*/
    sh_int	con_plus;		/* Con      "			*/
    sh_int	cha_plus;		/* Cha      "			*/
    sh_int	lck_plus;		/* Lck 	    "			*/
    sh_int      hit;
    sh_int      mana;
    sh_int      resist;
    sh_int      suscept;
    int		class_restriction;	/* Flags for illegal classes	*/
    int         language;               /* Default racial language      */
};

typedef enum {
CLAN_PLAIN, CLAN_VAMPIRE, CLAN_WARRIOR, CLAN_DRUID, CLAN_MAGE, CLAN_CELTIC,
CLAN_THIEF, CLAN_CLERIC, CLAN_UNDEAD, CLAN_CHAOTIC, CLAN_NEUTRAL, CLAN_LAWFUL,
CLAN_NOKILL, CLAN_ORDER, CLAN_GUILD } clan_types;

typedef enum { GROUP_CLAN, GROUP_COUNCIL, GROUP_GUILD } group_types;

/*
 * Damage types from the attack_table[]
 */
typedef enum
{
   DAM_HIT, DAM_SLICE, DAM_STAB, DAM_SLASH, DAM_WHIP, DAM_CLAW,
   DAM_BLAST, DAM_POUND, DAM_CRUSH, DAM_GREP, DAM_BITE, DAM_PIERCE,
   DAM_SUCTION, DAM_BOLT, DAM_ARROW, DAM_DART, DAM_STONE, DAM_PEA
} damage_types;

struct	clan_data
{
    CLAN_DATA * next;		/* next clan in list			*/
    CLAN_DATA * prev;		/* previous clan in list		*/
    char *	filename;	/* Clan filename			*/
    char *	name;		/* Clan name				*/
    char * 	fullname;	/* FULL Name, for new clan member --GW	*/
    char *	whoname;	/* Clan Who name -- GW		        */
    char *	motto;		/* Clan motto				*/
    char *	description;	/* A brief description of the clan	*/
    char *	deity;		/* Clan's deity				*/
    char *	leader;		/* Head clan leader			*/
    char *      associate;      /* Associated Clans (chare hall and channel */
    int		pkills;		/* Number of pkills on behalf of clan	*/
    int		pdeaths;	/* Number of pkills against clan	*/
    int		mkills;		/* Number of mkills on behalf of clan	*/
    int		mdeaths;	/* Number of clan deaths due to mobs	*/
    int		illegal_pk;	/* Number of illegal pk's by clan	*/
    int		score;		/* Overall score			*/
    sh_int	clan_type;	/* See clan type defines		*/
    sh_int	favour;		/* Deities favour upon the clan		*/
    sh_int	strikes;	/* Number of strikes against the clan	*/
    sh_int	members;	/* Number of clan members		*/
    sh_int	alignment;	/* Clan's general alignment		*/
    int		board;		/* Vnum of clan board			*/
    int		clanobj1;	/* Vnum of first clan obj (ring)	*/
    int		clanobj2;	/* Vnum of second clan obj (shield)	*/
    int		clanobj3;	/* Vnum of third clan obj (weapon)	*/
    int		recall;		/* Vnum of clan's recall room		*/
    int		storeroom;	/* Vnum of clan's store room		*/
    int		guard1;		/* Vnum of clan guard type 1		*/
    int		guard2;		/* Vnum of clan guard type 2		*/
    int		class;		/* For guilds				*/
    int		morgue;		/* for morgue's -Greywolf */
    PK_DATA    *pkdata;		/* PK Records --GW */
};

struct pk_data
{
PK_LIST *first_clan;
PK_LIST *last_clan;
};

struct pk_list
{
PK_LIST *next;
PK_LIST *prev;
char *name;
int pkills;
};

struct	council_data
{
    COUNCIL_DATA * next;	/* next council in list			*/
    COUNCIL_DATA * prev;	/* previous council in list		*/
    char *	filename;	/* Council filename			*/
    char *	name;		/* Council name				*/
    char *	description;	/* A brief description of the council	*/
    char *	head;		/* Council head 			*/
    char *	powers;		/* Council powers			*/
    sh_int	members;	/* Number of council members		*/
    int		board;		/* Vnum of council board		*/
    int		meeting;	/* Vnum of council's meeting room	*/
};

struct	deity_data
{
    DEITY_DATA * next;
    DEITY_DATA * prev;
    char *	filename;
    char *	name;
    char *	description;
    sh_int	alignment;
    sh_int	worshippers;
    sh_int	scorpse;
    sh_int	sdeityobj;
    sh_int	savatar;
    sh_int	recall;
    sh_int	flee;
    sh_int	flee_npcrace;
    sh_int	flee_npcfoe;
    sh_int	kill;
    sh_int	kill_magic;
    sh_int	kill_npcrace;
    sh_int	kill_npcfoe;
    sh_int	sac;
    sh_int	bury_corpse;
    sh_int	aid_spell;
    sh_int	aid;
    sh_int	backstab;
    sh_int	steal;
    sh_int	die;
    sh_int	die_npcrace;
    sh_int	die_npcfoe;
    sh_int	spell_aid;
    sh_int	dig_corpse;
    int		race;
    int		class;
    int		element;
    int		sex;
    int		avatar;
    int		deityobj;
    EXT_BV	affected;
    int		npcrace;
    int		npcfoe;
    int		suscept;
};


struct tourney_data
{
    int    open;
    int    low_level;
    int    hi_level;
};

/*
 * Data structure for notes.
 */
struct	note_data
{
    NOTE_DATA *	next;
    NOTE_DATA * prev;
    char *	sender;
    char *	date;
    char *	to_list;
    char *	subject;
    int         voting;
    char *	yesvotes;
    char *	novotes;
    char *	abstentions;
    char *	text;
};

/*
 * Game Mode Data --GW
 */
struct mode_data
{
   MODE_DATA * next;
   MODE_DATA * prev;
   char * name;
   char * desc;
   char * long_desc;
   char * who;
   char * date;
   int    flags;
};

/*
 * Data structure for mail system.
 */
struct	mail_data
{
    MAIL_DATA *	next;
    MAIL_DATA * prev;
    char *	sender;
    char *	date;
    char *	to;
    char *	subject;
    char *	text;
    int		read;
};

struct	board_data
{
    BOARD_DATA * next;			/* Next board in list		   */
    BOARD_DATA * prev;			/* Previous board in list	   */
    NOTE_DATA *  first_note;		/* First note on board		   */
    NOTE_DATA *  last_note;		/* Last note on board		   */
    char *	 note_file;		/* Filename to save notes to	   */
    char *	 read_group;		/* Can restrict a board to a       */
    char *	 post_group;		/* council, clan, guild etc        */
    char *	 extra_readers;		/* Can give read rights to players */
    char *       extra_removers;        /* Can give remove rights to players */
    int		 board_obj;		/* Vnum of board object		   */
    sh_int	 num_posts;		/* Number of notes on this board   */
    sh_int	 min_read_level;	/* Minimum level to read a note	   */
    sh_int	 min_post_level;	/* Minimum level to post a note    */
    sh_int	 min_remove_level;	/* Minimum level to remove a note  */
    sh_int	 max_posts;		/* Maximum amount of notes allowed */
    int          type;                  /* Normal board or mail board? */
};


/*
 * An affect.
 */
struct	affect_data
{
    AFFECT_DATA *	next;
    AFFECT_DATA *	prev;
    sh_int		type;
    sh_int		duration;
    sh_int		location;
    int			modifier;
    EXT_BV		bitvector;
};


/*
 * A SMAUG spell
 */
struct	smaug_affect
{
    SMAUG_AFF *		next;
    char *		duration;
    sh_int		location;
    char *		modifier;
    int			bitvector;
};


/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (Start of section ... start here)                     *
 *                                                                         *
 ***************************************************************************/

/*
 * Well known mob virtual numbers.
 * Defined in #MOBILES.
 */
#define MOB_VNUM_CITYGUARD	   30653
#define MOB_VNUM_VAMPIRE	   3404
#define MOB_VNUM_ANIMATED_CORPSE   5
#define MOB_VNUM_POLY_WOLF	   10
#define MOB_VNUM_POLY_MIST	   11
#define MOB_VNUM_POLY_BAT	   12
#define MOB_VNUM_POLY_HAWK	   13
#define MOB_VNUM_POLY_CAT	   14
#define MOB_VNUM_POLY_DOVE	   15
#define MOB_VNUM_POLY_FISH	   16

/*
 * ACT bits for mobs.
 * Used in #MOBILES.
 */
#define ACT_IS_NPC		 BV00		/* Auto set for mobs	*/
#define ACT_SENTINEL		 BV01		/* Stays in one room	*/
#define ACT_SCAVENGER		 BV02		/* Picks up objects	*/
#define ACT_AGGRESSIVE		 BV05		/* Attacks PC's		*/
#define ACT_STAY_AREA		 BV06		/* Won't leave area	*/
#define ACT_WIMPY		 BV07		/* Flees when hurt	*/
#define ACT_PET			 BV08		/* Auto set for pets	*/
#define ACT_POSTMASTER		 BV09		/* Can Accept/Give out mail */
#define ACT_PRACTICE		 BV10		/* Can practice PC's	*/
#define ACT_IMMORTAL		 BV11		/* Cannot be killed	*/
#define ACT_DEADLY		 BV12		/* Has a deadly poison  */
#define ACT_META_AGGR		 BV13		/* Can Open Lockers     */
#define ACT_LOCKER		 BV14		/* Extremely aggressive */
#define ACT_GUARDIAN		 BV15		/* Protects master	*/
#define ACT_RUNNING		 BV16		/* Hunts quickly	*/
#define ACT_NOWANDER		 BV17		/* Doesn't wander	*/
#define ACT_MOUNTABLE		 BV18		/* Can be mounted	*/
#define ACT_MOUNTED		 BV19		/* Is mounted		*/
#define ACT_SCHOLAR              BV20           /* Can teach languages  */
#define ACT_SECRETIVE		 BV21		/* actions aren't seen	*/
#define ACT_POLYMORPHED		 BV22		/* Mob is a ch		*/
#define ACT_MOBINVIS		 BV23		/* Like wizinvis	*/
#define ACT_NOASSIST		 BV24		/* Doesn't assist mobs	*/
#define ACT_QUESTTARGET		 BV25           /* is a quest target --GW */
#define ACT_NOTRACK		 BV26		/* no_track --GW */
#define ACT_SELL_TO_ALL		 BV27		/* sell to kilers --GW */
#define ACT_CREATE_LIFE_MOB	 BV28		/* Create Life Mobile --GW */
#define ACT_PROTOTYPE		 BV30		/* A prototype mob	*/
#define ACT_BANKER		 BV31
/* 20 acts */

/*
 * ch->acttwo bits, FOR NPC MOBS ONLY!!! --GW
 */

#define ACT2_NONE		 BV00
#define ACT2_BWEST		 BV01
#define ACT2_BEAST		 BV02
#define ACT2_BNORTH		 BV03
#define ACT2_BSOUTH		 BV04
#define ACT2_BUP		 BV05
#define ACT2_BDOWN		 BV06
#define ACT2_SEVERED		 BV07
#define ACT2_SEE_CLOAK		 BV08
#define ACT2_ACID_REPAIR	 BV09
/* DONOT go past BV31 */

/*
 * Bits for MOREPC_FLAGS -- PC ONLY --GW
 */
#define MOREPC_INCOG		 BV00
#define MOREPC_WAR		 BV01
#define MOREPC_OUTWAR		 BV02
#define MOREPC_SIGNEDPKILL	 BV03
#define MOREPC_SETSTART		 BV04
#define MOREPC_STARTINVIS        BV05
#define MOREPC_SAFETY		 BV06
#define MOREPC_FREELOOT		 BV07
#define MOREPC_CORPSELOOTER	 BV08
#define MOREPC_LOCKERLOOTER	 BV09
#define MOREPC_TOWNOUTCAST	 BV10
#define MOREPC_ARRESTED		 BV11
#define MOREPC_NAME_SET		 BV12
#define MOREPC_ABDUCTING	 BV13
#define MOREPC_ALIENS		 BV14
#define MOREPC_ETERNAL_PK	 BV15
#define MOREPC_HOARDER		 BV16
#define MOREPC_IC		 BV17
#define MOREPC_OWNED		 BV18
#define MOREPC_SITE_LOCK	 BV19
#define MOREPC_CTF		 BV20
#define MOREPC_KNIGHT		 BV21
#define MOREPC_AFFMSGS		 BV22
#define MOREPC_NOSPAM		 BV23
#define MOREPC_NO_LIMIT_CNT	 BV24
/* donot go past 31! --GW */

/*
 * Bits for 'affected_by'.
 * Used in #MOBILES.
 */
typedef enum
{
AFF_BLIND,AFF_INVISIBLE,AFF_DETECT_EVIL,AFF_DETECT_INVIS,AFF_DETECT_MAGIC,
AFF_DETECT_HIDDEN,AFF_REFLECT_MAGIC,AFF_SANCTUARY,AFF_FAERIE_FIRE,AFF_INFRARED,
AFF_CURSE,AFF_BOUNTY,AFF_POISON,AFF_PROTECT,AFF_PARALYSIS,AFF_SNEAK,AFF_HIDE,
AFF_SLEEP,AFF_CHARM,AFF_FLYING,AFF_PASS_DOOR,AFF_FLOATING,AFF_TRUESIGHT,AFF_DETECTTRAPS,
AFF_SCRYING,AFF_FIRESHIELD,AFF_SHOCKSHIELD,AFF_CLOAK,AFF_ICESHIELD,AFF_POSSESS,
AFF_BERSERK,AFF_AQUA_BREATH,AFF_MERLINS_ROBE,AFF_GHOST,AFF_REFLECT_DAMAGE,
MAX_AFFECTED_BY
} affected_by_types;

/*
 * Resistant Immune Susceptible flags
 */
#define RIS_FIRE		  BV00
#define RIS_COLD		  BV01
#define RIS_ELECTRICITY		  BV02
#define RIS_ENERGY		  BV03
#define RIS_BLUNT		  BV04
#define RIS_PIERCE		  BV05
#define RIS_SLASH		  BV06
#define RIS_ACID		  BV07
#define RIS_POISON		  BV08
#define RIS_DRAIN		  BV09
#define RIS_SLEEP		  BV10
#define RIS_CHARM		  BV11
#define RIS_HOLD		  BV12
#define RIS_NONMAGIC		  BV13
#define RIS_PLUS1		  BV14
#define RIS_PLUS2		  BV15
#define RIS_PLUS3		  BV16
#define RIS_PLUS4		  BV17
#define RIS_PLUS5		  BV18
#define RIS_PLUS6		  BV19
#define RIS_MAGIC		  BV20
#define RIS_PARALYSIS		  BV21
#define RIS_GAS_BREATH		  BV22
/* 21 RIS's*/

/* 
 * Attack types
 */
#define ATCK_BITE		  BV00
#define ATCK_CLAWS		  BV01
#define ATCK_TAIL		  BV02
#define ATCK_STING		  BV03
#define ATCK_PUNCH		  BV04
#define ATCK_KICK		  BV05
#define ATCK_TRIP		  BV06
#define ATCK_BASH		  BV07
#define ATCK_STUN		  BV08
#define ATCK_GOUGE		  BV09
#define ATCK_BACKSTAB		  BV10
#define ATCK_FEED		  BV11
#define ATCK_DRAIN		  BV12
#define ATCK_FIREBREATH		  BV13
#define ATCK_FROSTBREATH	  BV14
#define ATCK_ACIDBREATH		  BV15
#define ATCK_LIGHTNBREATH	  BV16
#define ATCK_GASBREATH		  BV17
#define ATCK_POISON		  BV18
#define ATCK_NASTYPOISON	  BV19
#define ATCK_GAZE		  BV20
#define ATCK_BLINDNESS		  BV21
#define ATCK_CAUSESERIOUS	  BV22
#define ATCK_EARTHQUAKE		  BV23
#define ATCK_CAUSECRITICAL	  BV24
#define ATCK_CURSE		  BV25
#define ATCK_FLAMESTRIKE	  BV26
#define ATCK_HARM		  BV27
#define ATCK_FIREBALL		  BV28
#define ATCK_COLORSPRAY		  BV29
#define ATCK_WEAKEN		  BV30
#define ATCK_SPIRALBLAST	  BV31
/* 32 USED! DO NOT ADD MORE! SB */

/*
 * Defense types
 */
#define DFND_PARRY		  BV00
#define DFND_DODGE		  BV01
#define DFND_HEAL		  BV02
#define DFND_CURELIGHT		  BV03
#define DFND_CURESERIOUS	  BV04
#define DFND_CURECRITICAL	  BV05
#define DFND_DISPELMAGIC	  BV06
#define DFND_DISPELEVIL		  BV07
#define DFND_SANCTUARY		  BV08
#define DFND_FIRESHIELD		  BV09
#define DFND_SHOCKSHIELD	  BV10
#define DFND_SHIELD		  BV11
#define DFND_BLESS		  BV12
#define DFND_STONESKIN		  BV13
#define DFND_TELEPORT		  BV14
#define DFND_MONSUM1		  BV15
#define DFND_MONSUM2		  BV16
#define DFND_MONSUM3		  BV17
#define DFND_MONSUM4		  BV18
#define DFND_DISARM		  BV19
#define DFND_ICESHIELD 		  BV20
#define DFND_GRIP		  BV21
/* 21 def's */

/*
 * Body parts
 */
#define PART_HEAD		  BV00
#define PART_ARMS		  BV01
#define PART_LEGS		  BV02
#define PART_HEART		  BV03
#define PART_BRAINS		  BV04
#define PART_GUTS		  BV05
#define PART_HANDS		  BV06
#define PART_FEET		  BV07
#define PART_FINGERS		  BV08
#define PART_EAR		  BV09
#define PART_EYE		  BV10
#define PART_LONG_TONGUE	  BV11
#define PART_EYESTALKS		  BV12
#define PART_TENTACLES		  BV13
#define PART_FINS		  BV14
#define PART_WINGS		  BV15
#define PART_TAIL		  BV16
#define PART_SCALES		  BV17
/* for combat */
#define PART_CLAWS		  BV18
#define PART_FANGS		  BV19
#define PART_HORNS		  BV20
#define PART_TUSKS		  BV21
#define PART_TAILATTACK		  BV22
#define PART_SHARPSCALES	  BV23
#define PART_BEAK		  BV24

#define PART_HAUNCH		  BV25
#define PART_HOOVES		  BV26
#define PART_PAWS		  BV27
#define PART_FORELEGS		  BV28
#define PART_FEATHERS		  BV29

/*
 * SMART Flags --GW
 */
#define SMART_ATTACK_WEAK 	  BV00
#define SMART_LOOTSAC		  BV01
#define SMART_CORPSESAC		  BV02
#define SMART_LOOTER		  BV03
#define SMART_ARCHMAGE		  BV04
#define SMART_BLADEMASTER	  BV05
#define SMART_BISHOP		  BV06
#define SMART_WRAITH		  BV07
#define SMART_MYSTIC		  BV08
#define SMART_NO_GET		  BV09

/*
 * Autosave flags
 */
#define SV_DEATH		  BV00
#define SV_KILL			  BV01
#define SV_PASSCHG		  BV02
#define SV_DROP			  BV03
#define SV_PUT			  BV04
#define SV_GIVE			  BV05
#define SV_AUTO			  BV06
#define SV_ZAPDROP		  BV07
#define SV_AUCTION		  BV08
#define SV_GET			  BV09
#define SV_RECEIVE		  BV10
#define SV_IDLE			  BV11
#define SV_BACKUP		  BV12

/*
 * Pipe flags
 */
#define PIPE_TAMPED		  BV01
#define PIPE_LIT		  BV02
#define PIPE_HOT		  BV03
#define PIPE_DIRTY		  BV04
#define PIPE_FILTHY		  BV05
#define PIPE_GOINGOUT		  BV06
#define PIPE_BURNT		  BV07
#define PIPE_FULLOFASH		  BV08

/*
 * Skill/Spell flags	The minimum BV *MUST* be 11!
 */
#define SF_WATER		  BV11
#define SF_EARTH		  BV12
#define SF_AIR			  BV13
#define SF_ASTRAL		  BV14
#define SF_AREA			  BV15  /* is an area spell		*/
#define SF_DISTANT		  BV16  /* affects something far away	*/
#define SF_REVERSE		  BV17
#define SF_SAVE_HALF_DAMAGE	  BV18  /* save for half damage		*/
#define SF_SAVE_NEGATES		  BV19  /* save negates affect		*/
#define SF_ACCUMULATIVE		  BV20  /* is accumulative		*/
#define SF_RECASTABLE		  BV21  /* can be refreshed		*/
#define SF_NOSCRIBE		  BV22  /* cannot be scribed		*/
#define SF_NOBREW		  BV23  /* cannot be brewed		*/
#define SF_GROUPSPELL		  BV24  /* only affects group members	*/
#define SF_OBJECT		  BV25	/* directed at an object	*/
#define SF_CHARACTER		  BV26  /* directed at a character	*/
#define SF_SECRETSKILL		  BV27	/* hidden unless learned	*/
#define SF_PKSENSITIVE		  BV28	/* much harder for plr vs. plr	*/
#define SF_STOPONFAIL		  BV29	/* stops spell on first failure */

typedef enum { SS_NONE, SS_POISON_DEATH, SS_ROD_WANDS, SS_PARA_PETRI,
	       SS_BREATH, SS_SPELL_STAFF } save_types;

#define ALL_BITS		INT_MAX
#define SDAM_MASK		ALL_BITS & ~(BV00 | BV01 | BV02)
#define SACT_MASK		ALL_BITS & ~(BV03 | BV04 | BV05)
#define SCLA_MASK		ALL_BITS & ~(BV06 | BV07 | BV08)
#define SPOW_MASK		ALL_BITS & ~(BV09 | BV10)

typedef enum { SD_NONE, SD_FIRE, SD_COLD, SD_ELECTRICITY, SD_ENERGY, SD_ACID,
	       SD_POISON, SD_DRAIN } spell_dam_types;

typedef enum { SA_NONE, SA_CREATE, SA_DESTROY, SA_RESIST, SA_SUSCEPT,
	       SA_DIVINATE, SA_OBSCURE, SA_CHANGE } spell_act_types;

typedef enum { SP_NONE, SP_MINOR, SP_GREATER, SP_MAJOR } spell_power_types;

typedef enum { SC_NONE, SC_LUNAR, SC_SOLAR, SC_TRAVEL, SC_SUMMON,
	       SC_LIFE, SC_DEATH, SC_ILLUSION } spell_class_types;

/*
 * Sex.
 * Used in #MOBILES.
 */
typedef enum { SEX_NEUTRAL, SEX_MALE, SEX_FEMALE } sex_types;

typedef enum {
  TRAP_TYPE_POISON_GAS = 1, TRAP_TYPE_POISON_DART,    TRAP_TYPE_POISON_NEEDLE,
  TRAP_TYPE_POISON_DAGGER,  TRAP_TYPE_POISON_ARROW,   TRAP_TYPE_BLINDNESS_GAS,
  TRAP_TYPE_SLEEPING_GAS,   TRAP_TYPE_FLAME,	      TRAP_TYPE_EXPLOSION,
  TRAP_TYPE_ACID_SPRAY,	    TRAP_TYPE_ELECTRIC_SHOCK, TRAP_TYPE_BLADE,
  TRAP_TYPE_SEX_CHANGE } trap_types;

#define MAX_TRAPTYPE		   TRAP_TYPE_SEX_CHANGE

#define TRAP_ROOM      		   BV00
#define TRAP_OBJ	      	   BV01
#define TRAP_ENTER_ROOM		   BV02
#define TRAP_LEAVE_ROOM		   BV03
#define TRAP_OPEN		   BV04 
#define TRAP_CLOSE		   BV05
#define TRAP_GET		   BV06
#define TRAP_PUT		   BV07
#define TRAP_PICK		   BV08
#define TRAP_UNLOCK		   BV09
#define TRAP_N			   BV10
#define TRAP_S			   BV11 
#define TRAP_E	      		   BV12
#define TRAP_W	      		   BV13
#define TRAP_U	      		   BV14
#define TRAP_D	      		   BV15
#define TRAP_EXAMINE		   BV16
#define TRAP_NE			   BV17
#define TRAP_NW			   BV18
#define TRAP_SE			   BV19
#define TRAP_SW			   BV20

/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
/* Currency */

#define COPPER		1
#define SILVER		2
#define GOLD		3

/* Copper */
#define OBJ_VNUM_MONEY_COPPER_ONE	      4
#define OBJ_VNUM_MONEY_COPPER_SOME	      5

/* Silver */
#define OBJ_VNUM_MONEY_SILVER_ONE	      6
#define OBJ_VNUM_MONEY_SILVER_SOME	      7

/* Gold */
#define OBJ_VNUM_MONEY_ONE		      2
#define OBJ_VNUM_MONEY_SOME		      3

/* Other */

#define OBJ_VNUM_CORPSE_NPC	     10
#define OBJ_VNUM_CORPSE_PC	     11
#define OBJ_VNUM_SEVERED_HEAD	     12
#define OBJ_VNUM_TORN_HEART	     13
#define OBJ_VNUM_SLICED_ARM	     14
#define OBJ_VNUM_SLICED_LEG	     15
#define OBJ_VNUM_SPILLED_GUTS	     16
#define OBJ_VNUM_BLOOD		     17
#define OBJ_VNUM_BLOODSTAIN	     18
#define OBJ_VNUM_SCRAPS		     19

#define OBJ_VNUM_MUSHROOM	     20
#define OBJ_VNUM_LIGHT_BALL	     21
#define OBJ_VNUM_SPRING		     22

#define OBJ_VNUM_SLICE		     24
#define OBJ_VNUM_SHOPPING_BAG	     25

#define OBJ_VNUM_FIRE		     30
#define OBJ_VNUM_TRAP		     31
#define OBJ_VNUM_PORTAL		     32

#define OBJ_VNUM_BLACK_POWDER	     33
#define OBJ_VNUM_SCROLL_SCRIBING     34
#define OBJ_VNUM_FLASK_BREWING       35
#define OBJ_VNUM_NOTE		     36
#define OBJ_VNUM_LOCKER		     51
#define OBJ_VNUM_PKPIN		     52
#define OBJ_VNUM_LEGS		     53

/* Newbie eq */
#define OBJ_VNUM_SCHOOL_MACE	  25507
#define OBJ_VNUM_SCHOOL_DAGGER	  25508
#define OBJ_VNUM_SCHOOL_SWORD	  25509
#define OBJ_VNUM_SCHOOL_VEST	  25510
#define OBJ_VNUM_SCHOOL_SHIELD	  25511
#define OBJ_VNUM_SCHOOL_CLOAK     25512
#define OBJ_VNUM_SCHOOL_HELMET    25513
#define OBJ_VNUM_SCHOOL_LEGGINGS  25514
#define OBJ_VNUM_SCHOOL_BOOTS     25515
#define OBJ_VNUM_SCHOOL_GLOVES    25516
#define OBJ_VNUM_SCHOOL_ARMBANDS  25517
#define OBJ_VNUM_SCHOOL_CAPE      25518
#define OBJ_VNUM_SCHOOL_BELT      25519
#define OBJ_VNUM_SCHOOL_BRACER    25520

/* Quest Items --GW */
#define OBJ_VNUM_QUEST_SWORD	  59
#define OBJ_VNUM_QUEST_CROWN	  60
#define OBJ_VNUM_QUEST_RING	  61
#define OBJ_VNUM_QUEST_DAGGER	  62

/* Object/character reference code, to avoid getting pointers to extracted
 * objects in loops. IMO it's more elegant than the 'deleted' method of
 * Envy.
 *
 * Copyright 1997 Oliver Jowett <oliver@jowett.manawatu.planet.co.nz>. You can
 * freely use this code, so long as this notice remains intact.
 *
 */

/* Structures for registering object/character pointers */

#define OBJ_NEXT          1
#define OBJ_NEXTCONTENT   2
#define OBJ_NULL          3

struct obj_ref_type
{
  bool inuse;
  struct obj_ref_type *next;
  OBJ_DATA **var;
  int type;                    /* OBJ_xxxx */
};

#define CHAR_NEXT         1
#define CHAR_NEXTROOM     2
#define CHAR_NULL         3

struct char_ref_type
{
  bool inuse;
  struct char_ref_type *next;
  CHAR_DATA **var;
  int type;
};

#define OREF(v, type) do { \
static struct obj_ref_type s={FALSE,NULL,NULL,type}; s.var=&v; \
obj_reference(&s); } while(0)

#define OUREF(var) obj_unreference(&var);

#define CREF(v, type) do { \
static struct char_ref_type s={FALSE,NULL,NULL,type}; s.var=&v; \
char_reference(&s); } while(0)
#define CUREF(var) char_unreference(&var);

void obj_reference(struct obj_ref_type *ref);
void obj_unreference(OBJ_DATA **var);
void char_reference(struct char_ref_type *ref);
void char_unreference(CHAR_DATA **var);






/*
 * Item types.
 * Used in #OBJECTS.
 */
typedef enum
{
  ITEM_NONE, ITEM_LIGHT, ITEM_SCROLL, ITEM_WAND, ITEM_STAFF, ITEM_WEAPON,
  ITEM_FIREWEAPON, ITEM_MISSILE, ITEM_TREASURE, ITEM_ARMOR, ITEM_POTION,
  ITEM_WORN, ITEM_FURNITURE, ITEM_TRASH, ITEM_OLDTRAP, ITEM_CONTAINER,
  ITEM_NOTE, ITEM_DRINK_CON, ITEM_KEY, ITEM_FOOD, ITEM_MONEY, ITEM_PEN,
  ITEM_BOAT, ITEM_CORPSE_NPC, ITEM_CORPSE_PC, ITEM_FOUNTAIN, ITEM_PILL,
  ITEM_BLOOD, ITEM_BLOODSTAIN, ITEM_SCRAPS, ITEM_PIPE, ITEM_HERB_CON,
  ITEM_HERB, ITEM_INCENSE, ITEM_FIRE, ITEM_BOOK, ITEM_SWITCH, ITEM_LEVER,
  ITEM_PULLCHAIN, ITEM_BUTTON, ITEM_DIAL, ITEM_RUNE, ITEM_RUNEPOUCH,
  ITEM_MATCH, ITEM_TRAP, ITEM_MAP, ITEM_PORTAL, ITEM_PAPER,
  ITEM_TINDER, ITEM_LOCKPICK, ITEM_SPIKE, ITEM_DISEASE, ITEM_OIL, ITEM_FUEL,
  ITEM_MISSILE_WEAPON, ITEM_PROJECTILE, ITEM_QUIVER,
  ITEM_SHOVEL, ITEM_SALVE, ITEM_SHRINE, ITEM_INSIG, ITEM_PK_BOOK,
  ITEM_GUILD_STONE, ITEM_PK_CLAN_STONE, ITEM_NONPK_CLAN_STONE, ITEM_VEHICLE,
  ITEM_LIFE_PROTECTION } item_types;


#define MAX_ITEM_TYPE		     ITEM_LIFE_PROTECTION

/*
 * Extra flags.
 * Used in #OBJECTS.
 */
#define ITEM_GLOW		BV00
#define ITEM_HUM		BV01
#define ITEM_DARK		BV02
#define ITEM_LOYAL		BV03
#define ITEM_EVIL		BV04
#define ITEM_INVIS		BV05
#define ITEM_MAGIC		BV06
#define ITEM_NODROP		BV07
#define ITEM_BLESS		BV08
#define ITEM_ANTI_GOOD		BV09
#define ITEM_ANTI_EVIL		BV10
#define ITEM_ANTI_NEUTRAL	BV11
#define ITEM_NOREMOVE		BV12
#define ITEM_INVENTORY		BV13
#define ITEM_MAGE_ONLY		BV14
#define ITEM_THIEF_ONLY	        BV15
#define ITEM_WARRIOR_ONLY	BV16
#define ITEM_CLERIC_ONLY	BV17
#define ITEM_ORGANIC		BV18
#define ITEM_METAL		BV19
#define ITEM_DONATION		BV20
#define ITEM_CLANOBJECT		BV21
#define ITEM_CLANCORPSE		BV22
#define ITEM_VAMPIRE_ONLY	BV23
#define ITEM_DRUID_ONLY	        BV24
#define ITEM_HIDDEN		BV25
#define ITEM_POISONED		BV26
#define ITEM_COVERING		BV27
#define ITEM_DEATHROT		BV28
#define ITEM_BURRIED		BV29	/* item is underground */
#define ITEM_PROTOTYPE		BV30
#define ITEM_NO_RESTRING	BV31

/* obj2 flags */
#define ITEM2_LIMITED		BV00
#define ITEM2_NO_SAC		BV01
#define ITEM2_NO_SCRAP		BV02
#define ITEM2_DUAL_ONLY		BV03
#define ITEM2_ADVANCED_ONLY	BV04
#define ITEM2_DUAL_ADV_ONLY	BV05
#define ITEM2_NOSAVE		BV06
#define ITEM2_PERM		BV07
#define ITEM2_RANGER_ONLY	BV08
#define ITEM2_AUGURER_ONLY	BV09
#define ITEM2_KINJU_ONLY	BV10
#define ITEM2_PALADIN_ONLY	BV11
#define ITEM2_ASSASSIN_ONLY	BV12
#define ITEM2_PSI_ONLY		BV13
#define ITEM2_WEREWOLF_ONLY	BV14
#define ITEM2_AVATAR_ONLY	BV15
#define ITEM2_DRAGON_ONLY	BV16
#define ITEM2_BISHOP_ONLY	BV17
#define ITEM2_ARCHMAGE_ONLY	BV18
#define ITEM2_BLADEMASTER_ONLY  BV19
#define ITEM2_ALCHEMIST_ONLY	BV20
#define ITEM2_PROPHET_ONLY	BV21
#define ITEM2_CRUSADER_ONLY	BV22
#define ITEM2_SAMURAI_ONLY	BV23
#define ITEM2_WRAITH_ONLY	BV24
#define ITEM2_HUNTER_ONLY	BV25
#define ITEM2_ADEPT_ONLY	BV26
#define ITEM2_NINJA_ONLY	BV27
#define ITEM2_JACKEL_ONLY	BV28
#define ITEM2_AUTO_SAC		BV29
#define ITEM2_PK_ONLY		BV30
#define ITEM2_MYSTIC_ONLY	BV31

/* Magic flags - extra extra_flags for objects that are used in spells */
#define ITEM_RETURNING		BV00
#define ITEM_BACKSTABBER  	BV01
#define ITEM_BANE		BV02
#define ITEM_LOYAL		BV03
#define ITEM_HASTE		BV04
#define ITEM_DRAIN		BV05
#define ITEM_LIGHTNING_BLADE  	BV06
/* Ok im outta Bits .. im using these for normal now --GW */
#define ITEM_UPSTATED		BV07
#define ITEM_DISSOLVE_IMM	BV08

/* Lever/dial/switch/button/pullchain flags */
#define TRIG_UP			BV00
#define TRIG_UNLOCK		BV01
#define TRIG_LOCK		BV02
#define TRIG_D_NORTH		BV03
#define TRIG_D_SOUTH		BV04
#define TRIG_D_EAST		BV05
#define TRIG_D_WEST		BV06
#define TRIG_D_UP		BV07
#define TRIG_D_DOWN		BV08
#define TRIG_DOOR		BV09
#define TRIG_CONTAINER		BV10
#define TRIG_OPEN		BV11
#define TRIG_CLOSE		BV12
#define TRIG_PASSAGE		BV13
#define TRIG_OLOAD		BV14
#define TRIG_MLOAD		BV15
#define TRIG_TELEPORT		BV16
#define TRIG_TELEPORTALL	BV17
#define TRIG_TELEPORTPLUS	BV18
#define TRIG_DEATH		BV19
#define TRIG_CAST		BV20
#define TRIG_FAKEBLADE		BV21
#define TRIG_RAND4		BV22
#define TRIG_RAND6		BV23
#define TRIG_TRAPDOOR		BV24
#define TRIG_ANOTHEROOM		BV25
#define TRIG_USEDIAL		BV26
#define TRIG_ABSOLUTEVNUM	BV27
#define TRIG_SHOWROOMDESC	BV28
#define TRIG_AUTORETURN		BV29

#define TELE_SHOWDESC		BV00
#define TELE_TRANSALL		BV01
#define TELE_TRANSALLPLUS	BV02


/*
 * Wear flags.
 * Used in #OBJECTS.
 */
#define ITEM_TAKE		BV00
#define ITEM_WEAR_FINGER	BV01
#define ITEM_WEAR_NECK		BV02
#define ITEM_WEAR_BODY		BV03
#define ITEM_WEAR_HEAD		BV04
#define ITEM_WEAR_LEGS		BV05
#define ITEM_WEAR_FEET		BV06
#define ITEM_WEAR_HANDS		BV07
#define ITEM_WEAR_ARMS		BV08
#define ITEM_WEAR_SHIELD	BV09
#define ITEM_WEAR_ABOUT		BV10
#define ITEM_WEAR_WAIST		BV11
#define ITEM_WEAR_WRIST		BV12
#define ITEM_WIELD		BV13
#define ITEM_HOLD		BV14
#define ITEM_DUAL_WIELD		BV15
#define ITEM_WEAR_EARS		BV16
#define ITEM_WEAR_EYES		BV17
#define ITEM_MISSILE_WIELD	BV18
#define ITEM_WEAR_INSIG		BV19
#define ITEM_WEAR_CHAMP		BV20
#define ITEM_WEAR_CLUB		BV21

/*
 * LIMIT Adjustment Types --GW
 */
#define LOADED_ADJUST_UP	1
#define LOADED_ADJUST_DOWN	2
#define LIMIT_ADJUST_SET	3
#define LOADED_ADJUST_SET	4

/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
typedef enum
{
  APPLY_NONE, APPLY_STR, APPLY_DEX, APPLY_INT, APPLY_WIS, APPLY_CON,
  APPLY_SEX, APPLY_CLASS, APPLY_LEVEL, APPLY_AGE, APPLY_HEIGHT, APPLY_WEIGHT,
  APPLY_MANA, APPLY_HIT, APPLY_MOVE, APPLY_GOLD, APPLY_EXP, APPLY_AC,
  APPLY_HITROLL, APPLY_DAMROLL, APPLY_SAVING_POISON, APPLY_SAVING_ROD,
  APPLY_SAVING_PARA, APPLY_SAVING_BREATH, APPLY_SAVING_SPELL, APPLY_CHA,
  APPLY_AFFECT, APPLY_RESISTANT, APPLY_IMMUNE, APPLY_SUSCEPTIBLE,
  APPLY_WEAPONSPELL, APPLY_LCK, APPLY_BACKSTAB, APPLY_PICK, APPLY_TRACK,
  APPLY_STEAL, APPLY_SNEAK, APPLY_HIDE, APPLY_PALM, APPLY_DETRAP, APPLY_DODGE,
  APPLY_PEEK, APPLY_SCAN, APPLY_GOUGE, APPLY_SEARCH, APPLY_MOUNT, APPLY_DISARM,
  APPLY_KICK, APPLY_PARRY, APPLY_BASH, APPLY_STUN, APPLY_PUNCH, APPLY_CLIMB,
  APPLY_GRIP, APPLY_SCRIBE, APPLY_BREW, APPLY_WEARSPELL, APPLY_REMOVESPELL,
  APPLY_EMOTION, APPLY_MENTALSTATE, APPLY_STRIPSN, APPLY_REMOVE, APPLY_DIG,
  APPLY_FULL, APPLY_THIRST, APPLY_DRUNK, APPLY_BLOOD, APPLY_ALIGN,
  APPLY_ROUND_STUN, APPLY_EGO, APPLY_EXT_AFFECT, MAX_APPLY_TYPE
} apply_types;

#define REVERSE_APPLY		   1000

/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE		      1
#define CONT_PICKPROOF		      2
#define CONT_CLOSED		      4
#define CONT_LOCKED		      8

/*
 * Well known room virtual numbers.
 * Defined in #ROOMS.
 */
#define ROOM_VNUM_LIMBO               2
#define ROOM_VNUM_POLY                3
#define ROOM_VNUM_CELL		      7
#define ROOM_VNUM_CHAT             1200
#define ROOM_VNUM_TEMPLE           30601
#define ROOM_VNUM_CURSED	   21700
#define ROOM_VNUM_ALTAR		   30602
#define ROOM_VNUM_SCHOOL           25518
#define ROOM_AUTH_START            25518
#define ROOM_VNUM_MORGUE	   30697
#define ROOM_VNUM_DONATE           30609
#define ROOM_VNUM_HOARDERS_VOID    19
#define BLACKJACK 		   30694          /* Room to play black jack in. */

/*
 * Room flags.           Holy cow!  Talked about stripped away..
 * Used in #ROOMS.       Those merc guys know how to strip code down.
 *			 Lets put it all back... ;)
 */

#define ROOM_DARK		BV00
#define ROOM_DEATH		BV01
#define ROOM_NO_MOB		BV02
#define ROOM_INDOORS		BV03
#define ROOM_LAWFUL		BV04
#define ROOM_NEUTRAL		BV05
#define ROOM_CHAOTIC		BV06
#define ROOM_NO_MAGIC		BV07
#define ROOM_TUNNEL		BV08
#define ROOM_PRIVATE		BV09
#define ROOM_SAFE		BV10
#define ROOM_SOLITARY		BV11
#define ROOM_PET_SHOP		BV12
#define ROOM_NO_RECALL		BV13
#define ROOM_DONATION		BV14
#define ROOM_NODROPALL		BV15
#define ROOM_SILENCE		BV16
#define ROOM_LOGSPEECH		BV17
#define ROOM_NODROP		BV18
#define ROOM_CLANSTOREROOM	BV19
#define ROOM_NO_SUMMON		BV20
#define ROOM_NO_ASTRAL		BV21
#define ROOM_TELEPORT		BV22
#define ROOM_TELESHOWDESC	BV23
#define ROOM_NOFLOOR		BV24
#define ROOM_SAVE		BV25
#define ROOM_NO_QUEST		BV26
#define ROOM_HOT		BV27
#define ROOM_COLD		BV28
#define ROOM_PROTOTYPE	     	BV30
#define ROOM_BANK		BV31
/* 32 rflags :(*/

/*
 * Directions.
 * Used in #ROOMS.
 */
typedef enum
{
  DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST, DIR_UP, DIR_DOWN,
  DIR_NORTHEAST, DIR_NORTHWEST, DIR_SOUTHEAST, DIR_SOUTHWEST, DIR_SOMEWHERE
} dir_types;

#define MAX_DIR			DIR_SOUTHWEST	/* max for normal walking */
#define DIR_PORTAL		DIR_SOMEWHERE	/* portal direction	  */


/*
 * Exit flags.
 * Used in #ROOMS.
 */
#define EX_ISDOOR		  BV00
#define EX_CLOSED		  BV01
#define EX_LOCKED		  BV02
#define EX_SECRET		  BV03
#define EX_SWIM			  BV04
#define EX_PICKPROOF		  BV05
#define EX_FLY			  BV06
#define EX_CLIMB		  BV07
#define EX_DIG			  BV08
#define EX_RES1                   BV09	/* are these res[1-4] important? */
#define EX_NOPASSDOOR		  BV10
#define EX_HIDDEN		  BV11
#define EX_PASSAGE		  BV12
#define EX_PORTAL 		  BV13
#define EX_RES2			  BV14
#define EX_RES3			  BV15
#define EX_xCLIMB		  BV16
#define EX_xENTER		  BV17
#define EX_xLEAVE		  BV18
#define EX_xAUTO		  BV19
#define EX_RES4	  		  BV20
#define EX_xSEARCHABLE		  BV21
#define EX_BASHED                 BV22
#define EX_BASHPROOF              BV23
#define EX_NOMOB		  BV24
#define EX_WINDOW		  BV25
#define EX_xLOOK		  BV26
#define EX_xNO_LOOK		  BV27
#define MAX_EXFLAG		  27

/*
 * Sector types.
 * Used in #ROOMS.
 */
typedef enum
{
  SECT_INSIDE, SECT_CITY, SECT_FIELD, SECT_FOREST, SECT_HILLS, SECT_MOUNTAIN,
  SECT_WATER_SWIM, SECT_WATER_NOSWIM, SECT_UNDERWATER, SECT_AIR, SECT_DESERT,
  SECT_DUNNO, SECT_OCEANFLOOR, SECT_UNDERGROUND, SECT_RENT, SECT_RENTED, SECT_MAX
} sector_types;

/*
 * Equpiment wear locations.
 * Used in #RESETS.
 */
typedef enum
{
  WEAR_NONE = -1, WEAR_LIGHT = 0, WEAR_FINGER_L, WEAR_FINGER_R, WEAR_NECK_1,
  WEAR_NECK_2, WEAR_BODY, WEAR_HEAD, WEAR_LEGS, WEAR_FEET, WEAR_HANDS,
  WEAR_ARMS, WEAR_SHIELD, WEAR_ABOUT, WEAR_WAIST, WEAR_WRIST_L, WEAR_WRIST_R,
  WEAR_WIELD, WEAR_HOLD, WEAR_DUAL_WIELD, WEAR_EARS, WEAR_EYES,
  WEAR_MISSILE_WIELD, WEAR_INSIG, WEAR_CHAMP, WEAR_CLUB, MAX_WEAR
} wear_locations;

/* Board Types */
typedef enum { BOARD_NOTE, BOARD_MAIL, BOARD_PK } board_types;

/* Auth Flags */
#define FLAG_WRAUTH		      1
#define FLAG_AUTH		      2

/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (End of this section ... stop here)                   *
 *                                                                         *
 ***************************************************************************/

/*
 * Conditions.
 */
typedef enum
{
  COND_DRUNK, COND_FULL, COND_THIRST, COND_BLOODTHIRST, MAX_CONDS
} conditions;

/*
 * Positions.
 */
typedef enum
{
  POS_DEAD, POS_MORTAL, POS_INCAP, POS_STUNNED, POS_SLEEPING, POS_RESTING,
  POS_SITTING, POS_FIGHTING, POS_STANDING, POS_MOUNTED, POS_SHOVE, POS_DRAG
} positions;

/*
 * ACT bits for players.
 */
#define PLR_IS_NPC		      BV00	/* Don't EVER set.	*/
#define PLR_BOUGHT_PET		      BV01
#define PLR_SHOVEDRAG		      BV02
#define PLR_AUTOEXIT		      BV03
#define PLR_AUTOLOOT		      BV04
#define PLR_AUTOSAC                   BV05
#define PLR_BLANK		      BV06
#define PLR_OUTCAST 		      BV07
#define PLR_BRIEF		      BV08
#define PLR_COMBINE		      BV09
#define PLR_PROMPT		      BV10
#define PLR_TELNET_GA		      BV11
#define PLR_HOLYLIGHT		   BV12
#define PLR_WIZINVIS		   BV13
#define PLR_ROOMVNUM		   BV14
#define	PLR_SILENCE		   BV15
#define PLR_NO_EMOTE		   BV16
#define PLR_ATTACKER		   BV17
#define PLR_NO_TELL		   BV18
#define PLR_LOG			   BV19
#define PLR_DENY		   BV20
#define PLR_FREEZE		   BV21
#define PLR_THIEF	           BV22
#define PLR_KILLER	           BV23
#define PLR_LITTERBUG	           BV24
#define PLR_ANSI	           BV25
#define PLR_RIP		           BV26
#define PLR_NICE	           BV27
#define PLR_FLEE	           BV28
#define PLR_AUTOGOLD               BV29
#define PLR_AUTOMAP                BV30
#define PLR_AFK                    BV31


/* Bits for pc_data->flags. */
#define PCFLAG_R1                  BV00
#define PCFLAG_DEADLY              BV01
#define PCFLAG_UNAUTHED		   BV02
#define PCFLAG_NORECALL            BV03
#define PCFLAG_NOINTRO             BV04
#define PCFLAG_GAG		   BV05
#define PCFLAG_RETIRED             BV06
#define PCFLAG_GUEST               BV07
#define PCFLAG_NOSUMMON		   BV08
#define PCFLAG_PAGERON		   BV09
#define PCFLAG_NOTITLE             BV10
#define PCFLAG_CHALLENGED          BV11
#define PCFLAG_CHALLENGER          BV12
#define PCFLAG_WED                 BV13
#define PCFLAG_ENGAGED             BV14
#define PCFLAG_ASSASSIN            BV15
#define PCFLAG_TENNANT             BV16
#define PCFLAG_OPEN_LOCKER	   BV17
#define PCFLAG_ADVANCED		   BV18
#define PCFLAG_NEWBIEHELP	   BV19
#define PCFLAG_SUSPENDED	   BV20
#define PCFLAG_SOUND		   BV21
#define PCFLAG_PLOG		   BV22
#define PCFLAG_CHEATER		   BV23
#define PCFLAG_NO_ECHO             BV24
#define PCFLAG_KOMBAT		   BV25
#define PCFLAG_PKCHAMP		   BV26
#define PCFLAG_SEVERED		   BV27
#define PCFLAG_QUESTING		   BV28
#define PCFLAG_ADV_DUAL		   BV29
#define PCFLAG_CENSORED		   BV30
#define PCFLAG_OLD_DEATH	   BV31
/* Donto Add More! --GW */

/*
 * Kombat bits --GW
 */
#define KOMBAT_ON		   BV00
#define KOMBAT_ACCEPT		   BV01
#define KOMBAT_DUAL		   BV02
#define KOMBAT_ADVANCED		   BV03
#define KOMBAT_NODRAGON		   BV04
#define KOMBAT_NOAVATAR		   BV05

/*
 * Hoarder Types --GW
 */

#define HOARD_NONE		0
#define HOARD_SEND_TO_VOID	1
#define HOARD_DROP_TRIGGER	2
#define HOARD_SORTING_EQ	3
#define HOARD_DONE_SORTING	4
#define HOARD_IN_VOID		5
#define HOARD_AUCD_ITEM		6

/*
 * Hoarder Check Types 
 */
#define HCHECK_LOGIN		1
#define HCHECK_DROP		2
#define HCHECK_MOVE		3
#define HCHECK_AUCTION		4

#define IS_HOARDER(ch)		( !IS_NPC(ch) && (ch)->pcdata->hoard_status > HOARD_NONE)

/*
 * Vehicle bits --GW
 */
#define VEHICLE_DARK		   BV00

typedef enum
{
  TIMER_NONE, TIMER_RECENTFIGHT, TIMER_SHOVEDRAG, TIMER_DO_FUN, 
TIMER_APPLIED, TIMER_PKILLED, TIMER_KOMBAT, TIMER_CTF } timer_types;

struct timer_data
{
    TIMER  *	prev;
    TIMER  *	next;
    DO_FUN *	do_fun;
    int		value;
    sh_int	type;
    sh_int	count;
};


/*
 * Channel bits.
 */
#define	CHANNEL_AUCTION		   BV00
#define	CHANNEL_CHAT		   BV01
#define	CHANNEL_QUEST		   BV02
#define	CHANNEL_IMMTALK		   BV03
#define	CHANNEL_MUSIC		   BV04
#define	CHANNEL_ASK		   BV05
#define	CHANNEL_SHOUT		   BV06
#define	CHANNEL_YELL		   BV07
#define CHANNEL_MONITOR		   BV08
#define CHANNEL_LOG		   BV09
#define CHANNEL_HIGHGOD		   BV10
#define CHANNEL_CLAN		   BV11
#define CHANNEL_BUILD		   BV12
#define CHANNEL_HIGH		   BV13
#define CHANNEL_AVTALK		   BV14
#define CHANNEL_PRAY		   BV15
#define CHANNEL_COUNCIL 	   BV16
#define CHANNEL_GUILD              BV17
#define CHANNEL_COMM		   BV18
#define CHANNEL_TELLS		   BV19
#define CHANNEL_ORDER              BV20
#define CHANNEL_NEWBIE             BV21
#define CHANNEL_WARTALK            BV22
#define CHANNEL_BEEP		   BV23
#define CHANNEL_AOTA		   BV24
#define CHANNEL_FLAME              BV25
#define CHANNEL_KOMBAT		   BV26
#define CHANNEL_OLYMPUS		   BV27
#define CHANNEL_INFO		   BV28
#define CHANNEL_MOBLOG		   BV29 /* Added by Callinon - 11-16-98 */
#define CHANNEL_IC		   BV30

/* Area Version Info --GW */
#define AREA_VERSION_NUMBER		2
#define AREA_VERSION_NAME		"AV-2-TXT-ZC"

/* Area defines - Scryn 8/11
 *
 */
#define AREA_DELETED		   BV00
#define AREA_LOADED                BV01

/* Area flags - Narn Mar/96 */
#define AFLAG_NOPKILL               BV00
#define AFLAG_CLOSED		    BV01
#define AFLAG_LOCKED		    BV02
#define AFLAG_CLUB_HALL		    BV03
#define AFLAG_TOWN		    BV04
#define AFLAG_STARTED		    BV05
#define AFLAG_ALWAYS_START	    BV06
#define AFLAG_NEVER_START	    BV07
#define AFLAG_NO_QUEST		    BV08
#define AFLAG_LOW_DUAL		    BV09
#define AFLAG_LOW_ADV		    BV10
#define AFLAG_LOW_DUAL_ADV	    BV11
#define AFLAG_HIGH_SINGLE	    BV12
#define AFLAG_HIGH_DUAL		    BV13
#define AFLAG_HIGH_ADV		    BV14
#define AFLAG_HIGH_DUAL_ADV	    BV15
#define AFLAG_DRAGON_DROP	    BV16

/* Area Search Types --GW */
#define AREA_SEARCH_MOB		1
#define AREA_SEARCH_OBJ		2
#define AREA_SEARCH_ROOM	3

/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
struct	mob_index_data
{
    MOB_INDEX_DATA *	next;
    MOB_INDEX_DATA *	next_sort;
    SPEC_FUN *		spec_fun;
    SHOP_DATA *		pShop;
    REPAIR_DATA *	rShop;
    MPROG_DATA *	mudprogs;
    AREA_DATA *		area;
    int			progtypes;
    char *		player_name;
    char *		short_descr;
    char *		long_descr;
    char *		description;
    int			vnum;
    sh_int		count;
    sh_int		killed;
    sh_int		sex;
    sh_int		level;
    sh_int		level2;
    int			act;
    int			acttwo;
    int			flags;
    int			smart; /* Smart Mob Flags --GW */
    EXT_BV		affected_by;
    sh_int		alignment;
    sh_int		mobthac0;		/* Unused */
    sh_int		ac;
    sh_int		hitnodice;
    sh_int		hitsizedice;
    int			hitplus;
    sh_int		damnodice;
    sh_int		damsizedice;
    sh_int		damplus;
    sh_int		numattacks;
    long		gold;
    long		silver;
    long		real_gold;
    long		exp;
    int			xflags;
    int			resistant;
    int			immune;
    int			susceptible;
    int			attacks;
    int			defenses;
    int			speaks;
    int 		speaking;
    sh_int		position;
    sh_int		defposition;
    sh_int		height;
    sh_int		weight;
    sh_int		race;
    sh_int		class;
    sh_int		hitroll;
    sh_int		damroll;
    sh_int		perm_str;
    sh_int		perm_int;
    sh_int		perm_wis;
    sh_int		perm_dex;
    sh_int		perm_con;
    sh_int		perm_cha;
    sh_int		perm_lck;
    sh_int		saving_poison_death;
    sh_int		saving_wand;
    sh_int		saving_para_petri;
    sh_int		saving_breath;
    sh_int		saving_spell_staff;
    char *		modified_by;
    char *		modified_date;
};


struct hunt_hate_fear
{
    char *		name;
    CHAR_DATA *		who;
};

struct fighting_data
{
    CHAR_DATA *		who;
    int			xp;
    sh_int		align;
    sh_int		duration;
    sh_int		timeskilled;
};

struct	editor_data
{
    sh_int		numlines;
    sh_int		on_line;
    sh_int		size;
    char		line[49][81];
};

struct	extracted_char_data
{
    EXTRACT_CHAR_DATA *	next;
    CHAR_DATA *		ch;
    ROOM_INDEX_DATA *	room;
    ch_ret		retcode;
    bool		extract;
};

/*
 * One character (PC or NPC).
 * (Shouldn't most of that build interface stuff use substate, dest_buf,
 * spare_ptr and tempnum?  Seems a little redundant)
 */
struct	char_data
{
    CHAR_DATA *		next;
    CHAR_DATA *		prev;
    CHAR_DATA *		next_in_room;
    CHAR_DATA *		prev_in_room;
    CHAR_DATA *		master;
    CHAR_DATA *		leader;
    FIGHT_DATA *	fighting;
    CHAR_DATA *		mount;
    HHF_DATA *		hunting;
    HHF_DATA *		fearing;
    HHF_DATA *		hating;
    SPEC_FUN *		spec_fun;
    AREA_DATA *		area;
    MPROG_ACT_LIST *	mpact;
    int			mpactnum;
    sh_int		mpscriptpos;
    MOB_INDEX_DATA *	pIndexData;
    AFFECT_DATA *	first_affect;
    AFFECT_DATA *	last_affect;
    OBJ_DATA *		first_carrying;
    OBJ_DATA *		last_carrying;
    ROOM_INDEX_DATA *	in_room;
    ROOM_INDEX_DATA *	was_in_room;
    ROOM_INDEX_DATA *	last_to_room;
    ZONE_DATA *		was_in_zone;
    PC_DATA *		pcdata;
    void *		dest_buf;
    void *		spare_ptr;
    int			tempnum;
    TIMER	*	first_timer;
    TIMER	*	last_timer;
    DESCRIPTOR_DATA *   desc;
    CHAR_DATA *         switched;
    char *		name;
    char *		short_descr;
    char *		long_descr;
    char *		description;
    sh_int		num_fighting;
    sh_int		substate;
    sh_int		sex;
    sh_int		class;
    sh_int		class2;
    sh_int		advclass;
    sh_int		advclass2;
    sh_int		race;
    sh_int		level;
    sh_int		level2;
    sh_int		advlevel;
    sh_int		advlevel2;
    sh_int		timer;
    sh_int		wait;
    int			hit;
    int			max_hit;
    int			mana;
    int			max_mana;
    int			move;
    int			max_move;
    sh_int		practice;
    sh_int		numattacks;
    long		gold; /* Copper now .. just easier --GW */
    long		silver;
    long		real_gold;
    long		exp;
    int 		act;
    int 		acttwo;
    int			smart;
    EXT_BV		affected_by;
    int			carry_weight;
    int			carry_number;
    int			xflags;
    int			resistant;
    int			immune;
    int			susceptible;
    int			attacks;
    int			defenses;
    int			speaks;
    int			speaking;
    sh_int		saving_poison_death;
    sh_int		saving_wand;
    sh_int		saving_para_petri;
    sh_int		saving_breath;
    sh_int		saving_spell_staff;
    sh_int		alignment;
    sh_int		barenumdie;
    sh_int		baresizedie;
    sh_int		mobthac0;
    sh_int		hitroll;
    sh_int		damroll;
    int			hitplus;
    sh_int		damplus;
    sh_int		position;
    sh_int		defposition;
    sh_int		height;
    sh_int		weight;
    sh_int		armor;
    sh_int		magical_armor;
    sh_int		wimpy;
    sh_int		perm_str;
    sh_int		perm_int;
    sh_int		perm_wis;
    sh_int		perm_dex;
    sh_int		perm_con;
    sh_int		perm_cha;
    sh_int		perm_lck;
    sh_int		mod_str;
    sh_int		mod_int;
    sh_int		mod_wis;
    sh_int		mod_dex;
    sh_int		mod_con;
    sh_int		mod_cha;
    sh_int		mod_lck;
    sh_int		mental_state;		/* simplified */
    sh_int		emotional_state;	/* simplified */
    int			retran;
    sh_int		mobinvis;	/* Mobinvis level SB */
    sh_int		stun_rounds;
    bool		deleted;
    ROOM_INDEX_DATA     *died_in_room;
};


struct killed_data
{
    sh_int		vnum;
    char		count;
};

/*
 * Data which only PC's have.
 */
struct	pc_data
{
    CLAN_DATA *		guild;
    CLAN_DATA *		clan;
    COUNCIL_DATA * 	council;
    AREA_DATA *		area;
    DEITY_DATA *	deity;
    VEHICLE_DATA *	vehicle;
    char *		email;
    char *		homepage;
    char *		clan_name;
    char *		guild_name;
    char * 		council_name;
    char *		deity_name;
    char *		pwd;
    char *		bamfin;
    char *		bamfout;
    char *              rank;
    char *		title;
    char *		stitle;
    char *		bestowments;	/* Special bestowed commands	   */
    char *		rreply;
    char *		rreply_name;
    char *		alias[MAX_ALIAS];
    char *		alias_sub[MAX_ALIAS];
    char *		lastcom[MAX_COM];
    char *		comtrack[MAX_COM];
    long		imc_deaf;
    long		imc_allow;
    long		imc_deny;
    int			confirm_delete; /* delete status*/
    int                 flags;		/* Whether the player is deadly and whatever else we add.      */
    int			pkills;		/* Number of pkills on behalf of clan */
    int			pdeaths;	/* Number of times pkilled (legally)  */
    int			mkills;		/* Number of mobs killed		   */
    int			mdeaths;	/* Number of deaths due to mobs       */
    int			illegal_pk;	/* Number of illegal pk's committed   */
    int			awins;		/* Arena Wins */
    int			alosses;	/* Arena Losses */
    int			plr_wager;	/* Gamble Code Wager Amount */
    long int            outcast_time;	/* The time at which the char was outcast */
    long int            restore_time;	/* The last time the char did a restore all */
    sh_int		r_range_lo;	/* room range */
    sh_int		r_range_hi;
    sh_int		m_range_lo;	/* mob range  */
    sh_int		m_range_hi;
    sh_int		o_range_lo;	/* obj range  */
    sh_int		o_range_hi;		
    sh_int		wizinvis;	/* wizinvis level */
    sh_int		min_snoop;	/* minimum snoop level */
    sh_int		condition	[MAX_CONDS];
    sh_int		incog_level;	/* incognito level */
    sh_int		learned		[MAX_SKILL];
    KILLED_DATA		killed		[MAX_KILLTRACK];
    sh_int		quest_number;	/* current *QUEST BEING DONE* DON'T REMOVE! */
    sh_int		quest_curr;	/* current number of quest points */
    int			quest_accum;	/* quest points accumulated in players life */
    sh_int		favor;		/* deity favor */
    int			auth_state;
    time_t		release_date;	/* Auto-helling.. Altrag */
    char *		helled_by;
    char *		bio;		/* Personal Bio */
    char *		authed_by;	/* what crazy imm authed this name ;) */
    SKILLTYPE *		special_skills[5]; /* personalized skills/spells */
    char *		prompt;		/* User config prompts */
    char *		subprompt;	/* Substate prompt */
    sh_int		pagerlen;	/* For pager (NOT menus) */
    bool		openedtourney;
    int			rentroom;
    char *              spouse; 
    char *		engaged;
    int			security; /* Creation Security for Cperm */
    CHAR_DATA *		irc_channel_master;
    int			num_kombats;
    int			num_kombatants;
    int			num_kombats_won;
    int			pkchamps;
    int			start_room;
    int			invis_start;
    bool		died_entrance;
    char *		challenge_char_name;
    char *		ignore[MAX_IGNORE];
    CHAR_DATA *         questgiver;     /* who gave the quest quest */
    int                 questpoints;
    int                 nextquest;
    int                 countdown;
    int                 questobj;
    int                 questmob;
    int			questinroom;
    int			ego;
    int			keeper_state;
    int			old_hp;
    int			old_mana;
    int			old_move;
    int			old_exp;
    int			clssdropped;
    char	*	name;
    int			nat_hp;
    int			nat_mana;
    sh_int		minoraffects;
    sh_int		majoraffects;
    sh_int		minorresist;
    sh_int		majorresist;
    sh_int		skill100s;
    sh_int		glory_status;
    sh_int		pre_name_toggle;
    char *		arena_viewing;
    COLORMAP_DATA *	color_maps;
    int			colormap_settings;
    PSOCIAL_DATA *	personal_socials;
    int			psocials;
    OFFLINE_DATA *	offline;
    int			hoard_status;
    /* Moved the Below from CHAR_DATA to conserve memory --GW */
    CHAR_DATA *         reply;
    NOTE_DATA *         pnote;  
    NOTE_DATA *         comments;
    EDITOR_DATA *       editor;
    DO_FUN *            last_cmd;
    DO_FUN *            prev_cmd;   /* mapping */
    char *              gladiator;
    long                bank;
    long                silver_bank;
    long                gold_bank;
    sh_int              cperm;
    sh_int              clevel;
    sh_int              guildlevel;
    sh_int              imc;
    int                 delete; /*delete confirmation*/
    sh_int              clanwho;
    sh_int              trust;
    int                 played;
    time_t              logon;   
    time_t              save_time;
    time_t              update_time;
    int                 flagstwo; 
    int                 deaf;   
    int                 bounty;
    int                 regoto;
    int                 pagelen;                        /* BUILD INTERFACE */
    sh_int              inter_page;                     /* BUILD INTERFACE */
    sh_int              inter_type;                     /* BUILD INTERFACE */
    char                *inter_editing;                 /* BUILD INTERFACE */
    int                 inter_editing_vnum;             /* BUILD INTERFACE */
    sh_int              inter_substate;                 /* BUILD INTERFACE */
    int 		shares[4];		/* Shares --GW */
    /* End of Move --GW */ 
    HOST_DATA		*first_host;
    HOST_DATA		*last_host;
    long		battle_xp;
    char *		ice_listen;
    /* upstatted obj's */
    int			hit_bonus;
    int			dam_bonus;
    int			ac_bonus;
    int			mac_bonus;
    /* rooms */
    int			warm_to_room;
    char *		mail_forward;
    int			team;
    sh_int		martial_arts_rating;
};



/*
 * Liquids.
 */
#define LIQ_WATER        0
#define LIQ_MAX		20

struct	liq_type
{
    char *	liq_name;
    char *	liq_color;
    sh_int	liq_affect[6];
};



/*
 * Extra description data for a room or object.
 */
struct	extra_descr_data
{
    EXTRA_DESCR_DATA *next;	/* Next in list                     */
    EXTRA_DESCR_DATA *prev;	/* Previous in list                 */
    char *keyword;              /* Keyword in look/examine          */
    char *description;          /* What to see                      */
};



/*
 * Prototype for an object.
 */
struct	obj_index_data
{
    OBJ_INDEX_DATA *	next;
    OBJ_INDEX_DATA *	next_sort;
    EXTRA_DESCR_DATA *	first_extradesc;
    EXTRA_DESCR_DATA *	last_extradesc;
    AFFECT_DATA *	first_affect;
    AFFECT_DATA *	last_affect;
    MPROG_DATA *	mudprogs;               /* objprogs */
    AREA_DATA *		area;
    VEHICLE_DATA *	vehicle;		/* vehicles --GW */
    int			progtypes;              /* objprogs */
    char *		name;
    char *		short_descr;
    char *		description;
    char *		action_desc;
    int			vnum;
    sh_int		item_type;
    int			extra_flags;
    int			second_flags; /* more bivectors for objs */
    int			magic_flags; /*Need more bitvectors for spells - Scryn*/
    int			wear_flags;
    sh_int		count;
    sh_int		weight;
    int			cost;
    int			value	[6];
    int			serial;
    sh_int		layers;
    int			rent;			
    int		        level;
    int		        minlevel;
    int			loaded;
    int			limit;
    int			has_moved;
    int			ego;	
    char *		modified_by;
    char *		modified_date;
};

/* Personal Social Stuff --GW */

/*
 * Persoanl Social Data --GW
 */
struct personalsocial_data
{
PSOC_DATA *	 first_social;
PSOC_DATA * 	last_social;
};

/*
 * PSocial Info Structure --GW
 */
struct psocial_data
{
    PSOC_DATA *		next;
    PSOC_DATA *		prev;
    char *		name;
    char *		char_no_arg;
    char *		others_no_arg;
    char *		char_found;
    char *		others_found;
    char *		vict_found;
    char *		char_auto;
    char *		others_auto;
    int			flags;
};

/*
 * PSocial Flags --GW
 */
#define	PSOC_CNOARG	BV01
#define PSOC_ONOARG	BV02
#define PSOC_CFOUND	BV03
#define PSOC_OFOUND	BV04
#define PSOC_VFOUND	BV05
#define PSOC_CAUTO	BV06
#define PSOC_OAUTO	BV07

/*
 * Social Arg Types --GW 
 */
#define SOCTYPE_CNOARG	1
#define SOCTYPE_ONOARG	2
#define SOCTYPE_CFOUND	3
#define SOCTYPE_OFOUND	4
#define SOCTYPE_VFOUND	5
#define SOCTYPE_CAUTO	6
#define SOCTYPE_OAUTO	7

/* End Psocials --GW */

/* Color Mapping Stuff --GW */

/*
 * Color Map Data --GW
 */
struct colormap_data
{
CMAP_DATA	*first_map;
CMAP_DATA	*last_map;
};

/*
 * Structure that holds the Color Mapping Info --GW
 */
struct cmap_data
{
CMAP_DATA	*next;
CMAP_DATA	*prev;
char *		remap_string;
sh_int		remap;
int		mapped_color;
int		map_type;
};

/*
 * Color Map Types --GW
 */
#define CMAP_RUMOR	1
#define CMAP_TELL	2
#define CMAP_AUCTION	3
#define CMAP_PLAY	4
#define CMAP_INFO	5
#define CMAP_QUEST	6
#define CMAP_WAR	7
#define CMAP_MUSIC	8
#define CMAP_ASK	9
#define CMAP_FLAME	10
#define CMAP_SHOUT	11
#define CMAP_YELL	12
#define CMAP_CLAN	13
#define CMAP_COUNCIL	14
#define CMAP_AVATAR	15
/* Colors only below this line */
#define CMAP_TELLS	16
#define CMAP_ROOMDESC	17
#define CMAP_ROOMNAME	18
#define CMAP_SOCIALS	19
#define CMAP_EXITS	20
#define CMAP_OBJ	21
#define CMAP_MOB	22
/* End of Colors Only */
#define CMAP_IMMTALK	23
#define CMAP_IMP	24
#define CMAP_OLYMPUS	25
#define CMAP_NEWBIECHAT 26
#define CMAP_THINK	27

/*
 * BV's to determine what cmaps are set on a char --GW
 * ch->pcdata->colormap_settings
 */
#define SCMAP_RUMOR	BV01
#define SCMAP_TELL	BV02
#define SCMAP_AUCTION	BV03
#define SCMAP_PLAY	BV04
#define SCMAP_INFO	BV05
#define SCMAP_QUEST	BV06
#define SCMAP_WAR	BV07
#define SCMAP_MUSIC	BV08
#define SCMAP_ASK	BV09
#define SCMAP_FLAME	BV10
#define SCMAP_SHOUT	BV11
#define SCMAP_YELL	BV12
#define SCMAP_CLAN	BV13
#define SCMAP_COUNCIL	BV14
#define SCMAP_AVATAR	BV15
#define SCMAP_TELLS	BV16
#define SCMAP_ROOMDESC	BV17
#define SCMAP_ROOMNAME	BV18
#define SCMAP_SOCIALS	BV19
#define SCMAP_EXITS	BV20
#define SCMAP_OBJ	BV21
#define SCMAP_MOB	BV22
#define SCMAP_IMMTALK	BV23
#define SCMAP_IMP	BV24
#define SCMAP_OLYMPUS	BV25
#define SCMAP_NEWBIECHAT BV26
#define SCMAP_THINK	BV27

/*
 * Color Map Colors --GW
 */
#define CMCOLOR_BLACK	1
#define CMCOLOR_BLOOD   2
#define CMCOLOR_GREEN	3
#define CMCOLOR_BROWN	4
#define CMCOLOR_DBLUE	5
#define CMCOLOR_PURPLE	6
#define CMCOLOR_CYAN	7
#define CMCOLOR_GREY	8
#define CMCOLOR_DGREY	9
#define CMCOLOR_LRED	10
#define CMCOLOR_LGREEN  11
#define CMCOLOR_YELLOW	12
#define CMCOLOR_BLUE	13
#define CMCOLOR_LBLUE	14
#define CMCOLOR_WHITE	15
#define CMCOLOR__BLACK  16
#define CMCOLOR__GREEN  17
#define CMCOLOR__BLUE	18
#define CMCOLOR__CYAN	19
#define CMCOLOR__RED	20
#define CMCOLOR__ORANGE 21
#define CMCOLOR__PURPLE 22
#define CMCOLOR__GREY	23

/* End Color Map Stuff --GW */

/*
 * Kombat --GW
 */
struct kombat_data
{
int 	low_lev;
int	hi_lev;
int	bits;
};

/*
 * Data stored for chars when they are NOT online --GW
 */
struct offline_data
{
char *name;
char *email_address;
char *last_site;
long gold;
long bank;
char *email_fw;
};

/*
 * Data for Vehicles --GW
 */
struct vehicle_data
{
char *desc;
char *operator;
int  obj_vnum;
OBJ_DATA *contents;
ROOM_INDEX_DATA *in_room;
int	serial_number;
int	vehicle_flags;
};

/*
 * Challenge Data for Arena --GW 
 */
struct challenge_data
{
CHALLENGE_DATA *next;
CHALLENGE_DATA *prev;
char *	challenger;
char *  challenged;
bool fight_in_progress;
int arena_number;
};

/*
 * Data for Arena's --GW
 */
struct arena_data
{
ARENA_DATA *next;
ARENA_DATA *prev;
int  char_room;
int  vict_room;
bool in_use;
int  arena_number;
};

/*
 * Refferal Data --GW
 */
struct refferal_data
{
REFFERAL_DATA *		next;
REFFERAL_DATA *		prev;
char *			name;
char *			refferal;
char *			site;
char *			date;
char *			email;
};

/*
 * Warrent Data --GW
 */
struct warrent_data
{
WARRENT_DATA *		next;
WARRENT_DATA *		prev;
char *			name;
char *			offense;
sh_int			executed;
char *			date;
char *			issuer;
};

/*
 * Web Server Stuff --GW
 */
struct web_descriptor {
    int fd;
    char request[MAXDATA*2];
    struct sockaddr_in *their_addr;
    int sin_size;
    WEB_DESCRIPTOR *next;
    WEB_DESCRIPTOR *prev;
    bool valid;
};

/*
 * Limit Data --GW
 */
struct limit_data
{
LIMIT_DATA	*next;
LIMIT_DATA	*prev;
int		vnum;
int		zone;
int		limit;
int		loaded;
int		checked_this_boot;
};

/*
 * Zone Data -- GW
 */
struct zone_data
{
ZONE_DATA *		next;
ZONE_DATA *		prev;
ROOM_INDEX_DATA *	room_index_hash[MAX_KEY_HASH];
OBJ_INDEX_DATA *	obj_index_hash[MAX_KEY_HASH];
MOB_INDEX_DATA *	mob_index_hash[MAX_KEY_HASH];
CHAR_DATA *		first_mob;
CHAR_DATA *		last_mob;
OBJ_DATA *		first_obj;
OBJ_DATA *		last_obj;
char *			list_filename;
int			number;
char *	                name;
char *                  filename;
int			top_obj;
int			top_mob;
int			top_room;
};

struct petition_data
{
   PETITION_DATA *	next;
   PETITION_DATA *	prev;
   char *		owner;
   char *		info;
   int			signers;
};

/*
 * One object.
 */
struct obj_data
{
    OBJ_DATA *		next;
    OBJ_DATA *		prev;
    OBJ_DATA *		next_content;
    OBJ_DATA *		prev_content;
    OBJ_DATA *		first_content;
    OBJ_DATA *		last_content;
    OBJ_DATA *		in_obj;
    CHAR_DATA *		carried_by;
    EXTRA_DESCR_DATA *	first_extradesc;
    EXTRA_DESCR_DATA *	last_extradesc;
    AFFECT_DATA *	first_affect;
    AFFECT_DATA *	last_affect;
    OBJ_INDEX_DATA *	pIndexData;
    ROOM_INDEX_DATA *	in_room;
    AREA_DATA *		area;
    char *		name;
    char *		spare_desc;
    char *		short_descr;
    char *		description;
    char *		action_desc;
    sh_int		item_type;
    sh_int		mpscriptpos;
    int			extra_flags;
    int			second_flags;
    int			magic_flags; /*Need more bitvectors for spells - Scryn*/
    int			wear_flags; 
    MPROG_ACT_LIST *	mpact;		/* mudprogs */
    int			mpactnum;	/* mudprogs */
    sh_int		wear_loc;
    sh_int		weight;
    int			cost;
    sh_int		timer;
    int			value	[6];
    sh_int		count;		/* support for object grouping */
    int			serial;		/* serial number	       */
    int		        level;
    int		        rent;
    int			minlevel;
    int			ego;
    bool		deleted;
    char *		modified_by;
    char *		modified_date;
    char *		loaded_by;
    /* Upstatted Obj's --GW */
    int			dam_bonus;
    int			hit_bonus;
    int			ac_bonus;
    int			mac_bonus;
};


/*
 * Exit data.
 */
struct	exit_data
{
    EXIT_DATA *		prev;		/* previous exit in linked list	*/
    EXIT_DATA *		next;		/* next exit in linked list	*/
    EXIT_DATA *		rexit;		/* Reverse exit pointer		*/
    ROOM_INDEX_DATA *	to_room;	/* Pointer to destination room	*/
    char *		keyword;	/* Keywords for exit or door	*/
    char *		description;	/* Description of exit		*/
    int			vnum;		/* Vnum of room exit leads to	*/
    int			rvnum;		/* Vnum of room in opposite dir	*/
    int			exit_info;	/* door states & other flags	*/
    int			key;		/* Key vnum			*/
    sh_int		vdir;		/* Physical "direction"		*/
    sh_int		distance;	/* how far to the next room	*/
};



/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'H': hide an object
 *   'B': set a bitvector
 *   'T': trap an object
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list)
 */

/*
 * Area-reset definition.
 */
struct	reset_data
{
    RESET_DATA *	next;
    RESET_DATA *	prev;
    char		command;
    int			extra;
    int			arg1;
    int			arg2;
    int			arg3;
};

/* Constants for arg2 of 'B' resets. */
#define	BIT_RESET_DOOR			0
#define BIT_RESET_OBJECT		1
#define BIT_RESET_MOBILE		2
#define BIT_RESET_ROOM			3
#define BIT_RESET_TYPE_MASK		0xFF	/* 256 should be enough */
#define BIT_RESET_DOOR_THRESHOLD	8
#define BIT_RESET_DOOR_MASK		0xFF00	/* 256 should be enough */
#define BIT_RESET_SET			BV30
#define BIT_RESET_TOGGLE		BV31
#define BIT_RESET_FREEBITS	  0x3FFF0000	/* For reference */



/*
 * Area definition.
 */
struct	area_data
{
    AREA_DATA *		next;
    AREA_DATA *		prev;
    AREA_DATA *		next_sort;
    AREA_DATA *		prev_sort;
    RESET_DATA *	first_reset;
    RESET_DATA *	last_reset;
    char *		name;
    char *		filename;
    int                 flags;
    sh_int              status;  /* h, 8/11 */
    sh_int		age;
    sh_int		nplayer;
    sh_int		reset_frequency;
    int			low_r_vnum;
    int			hi_r_vnum;
    int			low_o_vnum;
    int			hi_o_vnum;
    sh_int		low_m_vnum;
    sh_int		hi_m_vnum;
    int			low_soft_range;
    int			hi_soft_range;
    int			low_hard_range;
    int			hi_hard_range;
    char *		author; /* Scryn */
    char *              resetmsg; /* Rennard */
    char *		club;
    RESET_DATA *	last_mob_reset;
    RESET_DATA *	last_obj_reset;
    sh_int		max_players;
    int			mkills;
    int			mdeaths;
    int			pkills;
    int			pdeaths;
    int			gold_looted;
    int			illegal_pk;
    int			high_economy;
    int			low_economy;
    int			locklev;
    int			seclev;
    ZONE_DATA *		zone;
    int			version_number;
    char *		version_name;
    char *		map_file;
};



/*
 * Load in the gods building data. -- Altrag
 */
struct	godlist_data
{
    GOD_DATA *		next;
    GOD_DATA *		prev;
    int			level;
    int			low_r_vnum;
    int			hi_r_vnum;
    int			low_o_vnum;
    int			hi_o_vnum;
    sh_int		low_m_vnum;
    sh_int		hi_m_vnum;
};


/*
 * Used to keep track of system settings and statistics		-Thoric
 */
struct	system_data
{
    int		maxplayers;		/* Maximum players this boot   */
    int		alltimemax;		/* Maximum players ever	  */
    char *	time_of_max;		/* Time of max ever */
    bool	NO_NAME_RESOLVING;	/* Hostnames are not resolved  */
    bool    	DENY_NEW_PLAYERS;	/* New players cannot connect  */
    bool	WAIT_FOR_AUTH;		/* New players must be auth'ed */
    bool	GAME_WIZLOCKED;		/* Used to wizlock over reboots*/
    int		rent_ratio;		/* Used to adjust rent online */
    int		max_hp;			/* max hp a player can have */
    int		max_mana;		/* max mana a player can have */
    int		maxaward;		/* max glory someone can receive from a mob */
    int		segvio;			/* segvio on or off? */
    int		webserv;		/* Webserver on or off? --GW*/
    char *	entersound;		/* Sound when you enter the game */
    char *	connectsound;		/* Sound when you connect to the MUD --GW */
    sh_int	read_all_mail;		/* Read all player mail(was 54)*/
    sh_int	read_mail_free;		/* Read mail for free (was 51) */
    sh_int	write_mail_free;	/* Write mail for free(was 51) */
    sh_int	take_others_mail;	/* Take others mail (was 54)   */
    sh_int	imc_mail_vnum;		/* Board vnum for IMC mail     */
    sh_int	muse_level;		/* Level of muse channel */
    sh_int	think_level;		/* Level of think channel LEVEL_HIGOD*/
    sh_int	build_level;		/* Level of build channel LEVEL_BUILD*/
    sh_int	log_level;		/* Level of log channel LEVEL LOG*/
    sh_int	level_modify_proto;	/* Level to modify prototype stuff LEVEL_LESSER */
    sh_int	level_override_private;	/* override private flag */
    sh_int	level_mset_player;	/* Level to mset a player */
    sh_int	stun_plr_vs_plr;	/* Stun mod player vs. player */
    sh_int	stun_regular;		/* Stun difficult */
    sh_int	dam_plr_vs_plr;		/* Damage mod player vs. player */
    sh_int	dam_plr_vs_mob;		/* Damage mod player vs. mobile */
    sh_int	dam_mob_vs_plr;		/* Damage mod mobile vs. player */
    sh_int	dam_mob_vs_mob;		/* Damage mod mobile vs. mobile */
    sh_int	level_getobjnotake;     /* Get objects without take flag */
    sh_int      level_forcepc;          /* The level at which you can use force on players. */
    sh_int	max_sn;			/* Max skills */
    char       *guild_overseer;         /* Pointer to char containing the name of the */
    char       *guild_advisor;		/* guild overseer and advisor. */ 
    char       *pkchamp;		/* Player Kill Champ --GW */
    time_t	boottime;		/* Saves boot time for Warmboot--GW */
    int		max_plrs_boot;		/* Saves max Players this boot --GW */
    int		logins;			/* Logins per Boot --GW */
    int		save_flags;		/* Toggles for saving conditions */
    sh_int	save_frequency;		/* How old to autosave someone */
    APPROVE_DATA *first_approve;        /* Password Approval System --GW*/
    APPROVE_DATA *last_approve;		/* Password Approval System --GW */
};

struct approve_data
{
APPROVE_DATA *next;
APPROVE_DATA *prev;
char *email;
char *password;
};

struct host_data
{
HOST_DATA *next;
HOST_DATA *prev;
char *site;
};

struct topten_data
{
TOPTEN_DATA *next;
TOPTEN_DATA *prev;
sh_int type;
char *plist[10];
long values[10];
};

/*
 * Room type.
 */
struct	room_index_data
{
    ROOM_INDEX_DATA *	next;
    ROOM_INDEX_DATA *	next_sort;
    CHAR_DATA *		first_person;
    CHAR_DATA *		last_person;
    OBJ_DATA *		first_content;
    OBJ_DATA *		last_content;
    EXTRA_DESCR_DATA *	first_extradesc;
    EXTRA_DESCR_DATA *	last_extradesc;
    AREA_DATA *		area;
    EXIT_DATA *		first_exit;
    EXIT_DATA *		last_exit;
    char *		name;
    MAP_DATA *		map;                 /* maps */
    char *		description;
    int			vnum;
    int			room_flags;
    MPROG_ACT_LIST *	mpact;               /* mudprogs */
    int			mpactnum;            /* mudprogs */
    MPROG_DATA *	mudprogs;            /* mudprogs */
    sh_int		mpscriptpos;
    int			progtypes;           /* mudprogs */
    sh_int		light;
    sh_int		sector_type;
    int			tele_vnum;
    sh_int		tele_delay;
    sh_int		tunnel;		     /* max people that will fit */
    int			level;
    char *		music;		     /* MSP enabled rooms */
    char *		modified_by;
    char *		modified_date;
    int			room_cnt;
};

/*
 * Delayed teleport type.
 */
struct	teleport_data
{
    TELEPORT_DATA *	next;
    TELEPORT_DATA *	prev;
    ROOM_INDEX_DATA *	room;
    sh_int		timer;
};


/*
 * Types of skill numbers.  Used to keep separate lists of sn's
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that.
 */
#define TYPE_UNDEFINED               -1
#define TYPE_HIT                     1000  /* allows for 1000 skills/spells */
#define TYPE_HERB		     2000  /* allows for 1000 attack types  */
#define TYPE_PERSONAL		     3000  /* allows for 1000 herb types    */

/*
 *  Target types.
 */
typedef enum
{
  TAR_IGNORE, TAR_CHAR_OFFENSIVE, TAR_CHAR_DEFENSIVE, TAR_CHAR_SELF,
  TAR_OBJ_INV
} target_types;

typedef enum
{
  SKILL_UNKNOWN, SKILL_SPELL, SKILL_SKILL, SKILL_WEAPON, SKILL_TONGUE,
  SKILL_HERB
} skill_types;



struct timerset
{
  int num_uses;
  struct timeval total_time;
  struct timeval min_time;
  struct timeval max_time;
};



/*
 * Skills include spells as a particular case.
 */
struct	skill_type
{
    char *	name;			/* Name of skill		*/
    sh_int	skill_level[MAX_CLASS];	/* Level needed by class	*/
    sh_int	skill_adept[MAX_CLASS];	/* Max attainable % in this skill */
    sh_int	race_skill_level[MAX_RACE]; /* Needed for Race Skills--GW*/
    sh_int	race_skill_adept[MAX_RACE]; /* Needed for Race Skills--GW*/
    SPELL_FUN *	spell_fun;		/* Spell pointer (for spells)	*/
    DO_FUN *	skill_fun;		/* Skill pointer (for skills)	*/
    sh_int	target;			/* Legal targets		*/
    sh_int	minimum_position;	/* Position for caster / user	*/
    sh_int	slot;			/* Slot for #OBJECT loading	*/
    sh_int	min_mana;		/* Minimum mana used		*/
    sh_int	beats;			/* Rounds required to use skill	*/
    char *	noun_damage;		/* Damage message		*/
    char *	msg_off;		/* Wear off message		*/
    sh_int	guild;			/* Which guild the skill belongs to */
    sh_int	min_level;		/* Minimum level to be able to cast */
    sh_int	type;			/* Spell/Skill/Weapon/Tongue	*/
    int		info;			/* Not to sure --GW */
    int		flags;			/* extra stuff			*/
    char *	hit_char;		/* Success message to caster	*/
    char *	hit_vict;		/* Success message to victim	*/
    char *	hit_room;		/* Success message to room	*/
    char *	miss_char;		/* Failure message to caster	*/
    char *	miss_vict;		/* Failure message to victim	*/
    char *	miss_room;		/* Failure message to room	*/
    char *	die_char;		/* Victim death msg to caster	*/
    char *	die_vict;		/* Victim death msg to victim	*/
    char *	die_room;		/* Victim death msg to room	*/
    char *	imm_char;		/* Victim immune msg to caster	*/
    char *	imm_vict;		/* Victim immune msg to victim	*/
    char *	imm_room;		/* Victim immune msg to room	*/
    char *	dice;			/* Dice roll			*/
    int		value;			/* Misc value			*/
    char	saves;			/* What saving spell applies	*/
    char	difficulty;		/* Difficulty of casting/learning */
    SMAUG_AFF *	affects;		/* Spell affects, if any	*/
    char *	components;		/* Spell components, if any	*/
    char *	teachers;		/* Skill requires a special teacher */
    char	participants;		/* # of required participants	*/
    struct	timerset	userec;	/* Usage record			*/
};


struct  auction_data
{
    OBJ_DATA  * item;   /* a pointer to the item */
    CHAR_DATA * seller; /* a pointer to the seller - which may NOT quit */
    CHAR_DATA * buyer;  /* a pointer to the buyer - which may NOT quit */
    int         bet;    /* last bet - or 0 if noone has bet anything */
    sh_int      going;  /* 1,2, sold */
    sh_int      pulse;  /* how many pulses (.25 sec) until another call-out ? */
    int 	starting;
    int		flags;  /* Auction Flags --GW */
};

/* Auction Flags */

#define AUCTION_HOARD_AUC		BV01

/*
 * These are skill_lookup return values for common skills and spells.
 */
extern bool WEBSERVER_STATUS;
extern char last_function_call [MSL];
extern char last_command [MSL];
extern DISABLED_DATA       *		disabled_first;
extern  sh_int  gsn_missile_weapons;
extern	sh_int	gsn_detrap;
extern	sh_int	gsn_assassinate;
extern	sh_int	gsn_backstab;
extern  sh_int  gsn_circle;
extern	sh_int	gsn_dodge;
extern	sh_int	gsn_hide;
extern	sh_int	gsn_peek;
extern	sh_int	gsn_pick_lock;
extern  sh_int  gsn_scan;
extern	sh_int	gsn_sneak;
extern	sh_int	gsn_steal;
extern	sh_int	gsn_gouge;
extern	sh_int	gsn_track;
extern	sh_int	gsn_search;
extern  sh_int  gsn_dig;
extern	sh_int	gsn_mount;
extern  sh_int  gsn_bashdoor;
extern	sh_int	gsn_berserk;
extern	sh_int	gsn_hitall;

extern	sh_int	gsn_disarm;
extern	sh_int	gsn_enhanced_damage;
extern	sh_int	gsn_kick;
extern	sh_int	gsn_parry;
extern	sh_int	gsn_rescue;
extern	sh_int	gsn_second_attack;
extern	sh_int	gsn_third_attack;
extern	sh_int	gsn_fourth_attack;
extern	sh_int	gsn_fifth_attack;
extern  sh_int	gsn_sixth_attack;
extern	sh_int	gsn_seventh_attack;
extern	sh_int	gsn_dual_wield;

extern  sh_int	gsn_ancient_lore;

extern	sh_int	gsn_feed;

extern	sh_int	gsn_aid;

/* used to do specific lookups */
extern	sh_int	gsn_first_spell;
extern	sh_int	gsn_first_skill;
extern	sh_int	gsn_first_weapon;
extern	sh_int	gsn_first_tongue;
extern	sh_int	gsn_top_sn;

/* spells */
extern	sh_int	gsn_blindness;
extern	sh_int	gsn_charm_person;
extern  sh_int  gsn_aqua_breath;
extern	sh_int	gsn_curse;
extern	sh_int	gsn_invis;
extern	sh_int	gsn_mass_invis;
extern	sh_int	gsn_poison;
extern	sh_int	gsn_sleep;
extern  sh_int  gsn_possess;
extern	sh_int	gsn_fireball;		/* for fireshield  */
extern	sh_int	gsn_chill_touch;	/* for iceshield   */
extern	sh_int	gsn_lightning_bolt;	/* for shockshield */
extern  sh_int  gsn_reflect_magic;	/* for reflect magic */

/* newer attack skills */
extern	sh_int	gsn_punch;
extern	sh_int	gsn_bash;
extern	sh_int	gsn_stun;
extern	sh_int	gsn_bite;
extern	sh_int	gsn_claw;
extern	sh_int	gsn_sting;
extern	sh_int	gsn_tail;

extern  sh_int  gsn_poison_weapon;
extern  sh_int  gsn_scribe;
extern  sh_int  gsn_brew;
extern	sh_int	gsn_climb;

extern	sh_int	gsn_pugilism;
extern	sh_int	gsn_long_blades;
extern	sh_int	gsn_short_blades;
extern	sh_int	gsn_flexible_arms;
extern	sh_int	gsn_talonous_arms;
extern	sh_int	gsn_bludgeons;

extern  sh_int  gsn_grip;
extern  sh_int  gsn_slice;

/* Language gsns. -- Altrag */
extern  sh_int  gsn_common;
extern  sh_int  gsn_elven;
extern  sh_int  gsn_dwarven;
extern  sh_int  gsn_pixie;
extern  sh_int  gsn_ogre;
extern  sh_int  gsn_orcish;
extern  sh_int  gsn_trollish;
extern  sh_int  gsn_goblin;
extern  sh_int  gsn_halfling;
extern  sh_int  gsn_vampiric;
extern  sh_int  gsn_wolfish;
extern  sh_int  gsn_draconian;
extern  sh_int  gsn_satyr;
extern  sh_int  gsn_wraith;
extern  sh_int  gsn_centaur;
extern  sh_int  gsn_drider;
extern  sh_int  gsn_drowish;
extern  sh_int  gsn_minotaur;
extern  sh_int  gsn_cyclops;
extern  sh_int  gsn_arewyndel;
extern  sh_int  gsn_brownie;
extern  sh_int  gsn_leprachaun;
extern  sh_int  gsn_mrrshan;

/*
 * Utility macros.
 */
#define UMIN(a, b)		((a) < (b) ? (a) : (b))
#define UMAX(a, b)		((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)		((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define LOWER(c)		((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)		((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))
#define IS_SET(flag, bit)	((flag) & (bit))
#define SET_BIT(var, bit)	((var) |= (bit))
#define REMOVE_BIT(var, bit)	((var) &= ~(bit))
#define TOGGLE_BIT(var, bit)	((var) ^= (bit))
#define CH(d)			((d)->original ? (d)->original : (d)->character)


bool    ext_is_empty            args( ( EXT_BV *bits ) );
void    ext_clear_bits          args( ( EXT_BV *bits ) );
int     ext_has_bits            args( ( EXT_BV *var, EXT_BV *bits) );
bool    ext_same_bits           args( ( EXT_BV *var, EXT_BV *bits) );
void    ext_set_bits            args( ( EXT_BV *var, EXT_BV *bits) );
void    ext_remove_bits         args( ( EXT_BV *var, EXT_BV *bits) );
void    ext_toggle_bits         args( ( EXT_BV *var, EXT_BV *bits) );


/*
 * Here are the extended bitvector macros:
 */
#define xIS_SET(var, bit)       ((var).bits[(bit) >> RSV] & 1 << ((bit) & XBM))
#define xSET_BIT(var, bit)      ((var).bits[(bit) >> RSV] |= 1 << ((bit) & XBM))
#define xSET_BITS(var, bit)     (ext_set_bits(&(var), &(bit)))
#define xREMOVE_BIT(var, bit)   ((var).bits[(bit) >> RSV] &= ~(1 << ((bit) & XBM)))
#define xREMOVE_BITS(var, bit)  (ext_remove_bits(&(var), &(bit)))
#define xTOGGLE_BIT(var, bit)   ((var).bits[(bit) >> RSV] ^= 1 << ((bit) & XBM))
#define xTOGGLE_BITS(var, bit)  (ext_toggle_bits(&(var), &(bit)))
#define xCLEAR_BITS(var)        (ext_clear_bits(&(var)))
#define xIS_EMPTY(var)          (ext_is_empty(&(var)))
#define xHAS_BITS(var, bit)     (ext_has_bits(&(var), &(bit)))
#define xSAME_BITS(var, bit)    (ext_same_bits(&(var), &(bit)))

/* 
 * Fclose (gnulibc thing)
 */
#ifndef new_fclose
#define new_fclose(fp)						\
do {								\
if ( (fp) == NULL )						\
{ bug("Halting new_fclose call with NULL fp (%s:%d)",__FILE__,__LINE__); } \
if ( (fp) != NULL ) {						\
fclose((fp)); }							\
} while(0)							
#endif

/*
 * Memory allocation macros.
 */

#define CREATE(result, type, number)				\
do								\
{								\
   if ( result )						\
    { bug("CREATING in non-NULL structure -- Destroying Previous (F: %s L: %d)",__FILE__,__LINE__); result=NULL; } \
								\
   if (!((result) = (type *) calloc ((number), sizeof(type))))	\
	{ perror("malloc failure"); abort(); }			\
} while(0)

#define RECREATE(result,type,number)				\
do								\
{								\
  if (!((result) = (type *) realloc ((result), sizeof(type) * (number))))\
	{ perror("realloc failure"); abort(); }			\
} while(0)


#define DISPOSE(point) 						\
do								\
{								\
  sprintf(membuf,"DISPOSING: %s: %d\n",__FILE__,__LINE__);	\
  write_memlog_line(membuf);					\
  if (!(point))							\
  {								\
	bug( "Freeing null pointer" ); \
	fprintf( stderr, "DISPOSEing NULL in %s, line %d\n", __FILE__, __LINE__ ); \
  }								\
  else free(point);						\
  point = NULL;						\
} while(0)

#ifdef HASHSTR
#define STRALLOC(point)		str_alloc((point))
#define QUICKLINK(point)	quick_link((point))
#define QUICKMATCH(p1, p2)	(int) (p1) == (int) (p2)
#define STRFREE(point)						\
do								\
{								\
  sprintf(membuf,"STRFREEing: %s: %d\n",__FILE__,__LINE__);	\
  write_memlog_line(membuf);					\
  if (!(point))							\
  {								\
	bug( "Freeing null pointer" );	 			\
	fprintf( stderr, "STRFREEing NULL in %s, line %d\n", __FILE__, __LINE__ ); \
  }								\
  else if (str_free((point))==-1) 				\
    fprintf( stderr, "STRFREEing bad pointer in %s, line %d\n", __FILE__, __LINE__ ); \
  point = NULL;  						\
} while(0)
#else
#define STRALLOC(point)		str_dup((point))
#define QUICKLINK(point)	str_dup((point))
#define QUICKMATCH(p1, p2)	strcmp((p1), (p2)) == 0
#define STRFREE(point)						\
do								\
{								\
  sprintf(membuf,"STRFREE: %s: %d\n",__FILE__,__LINE__);	\
  write_memlog_line(membuf);					\
  if (!(point))							\
  {								\
	bug( "Freeing null pointer" );				\
	fprintf(stderr, "STRFREEing NULL in %s, line %d\n",__FILE__,__LINE__ ); \
  }								\
  else free((point));						\
  point = NULL; 						\
} while(0)
#endif

/* double-linked list handling macros -Thoric */

#define LINK(link, first, last, next, prev)			\
do								\
{								\
    if ( !(first) )						\
      (first)			= (link);			\
    else							\
      (last)->next		= (link);			\
    (link)->next		= NULL;				\
    (link)->prev		= (last);			\
    (last)			= (link);			\
} while(0)

#define INSERT(link, insert, first, next, prev)			\
do								\
{								\
    (link)->prev		= (insert)->prev;		\
    if ( !(insert)->prev )					\
      (first)			= (link);			\
    else							\
      (insert)->prev->next	= (link);			\
    (insert)->prev		= (link);			\
    (link)->next		= (insert);			\
} while(0)

#define UNLINK(link, first, last, next, prev)			\
do								\
{								\
    if ( !(link)->prev )					\
      (first)			= (link)->next;			\
    else							\
      (link)->prev->next	= (link)->next;			\
    if ( !(link)->next )					\
      (last)			= (link)->prev;			\
    else							\
      (link)->next->prev	= (link)->prev;			\
} while(0)


#define CHECK_LINKS(first, last, next, prev, type)		\
do {								\
  type *ptr, *pptr = NULL;					\
  if ( !(first) && !(last) )					\
    break;							\
  if ( !(first) )						\
  {								\
    bug( "CHECK_LINKS: last with NULL first!  %s.",		\
        __STRING(first) );					\
    for ( ptr = (last); ptr->prev; ptr = ptr->prev );		\
    (first) = ptr;						\
  }								\
  else if ( !(last) )						\
  {								\
    bug( "CHECK_LINKS: first with NULL last!  %s.",		\
        __STRING(first) );					\
    for ( ptr = (first); ptr->next; ptr = ptr->next );		\
    (last) = ptr;						\
  }								\
  if ( (first) )						\
  {								\
    for ( ptr = (first); ptr; ptr = ptr->next )			\
    {								\
      if ( ptr->prev != pptr )					\
      {								\
        bug( "CHECK_LINKS(%s): %p:->prev != %p.  Fixing.",	\
            __STRING(first), ptr, pptr );			\
        ptr->prev = pptr;					\
      }								\
      if ( ptr->prev && ptr->prev->next != ptr )		\
      {								\
        bug( "CHECK_LINKS(%s): %p:->prev->next != %p.  Fixing.",\
            __STRING(first), ptr, ptr );			\
        ptr->prev->next = ptr;					\
      }								\
      pptr = ptr;						\
    }								\
    pptr = NULL;						\
  }								\
  if ( (last) )							\
  {								\
    for ( ptr = (last); ptr; ptr = ptr->prev )			\
    {								\
      if ( ptr->next != pptr )					\
      {								\
        bug( "CHECK_LINKS (%s): %p:->next != %p.  Fixing.",	\
            __STRING(first), ptr, pptr );			\
        ptr->next = pptr;					\
      }								\
      if ( ptr->next && ptr->next->prev != ptr )		\
      {								\
        bug( "CHECK_LINKS(%s): %p:->next->prev != %p.  Fixing.",\
            __STRING(first), ptr, ptr );			\
        ptr->next->prev = ptr;					\
      }								\
      pptr = ptr;						\
    }								\
  }								\
} while(0)

/* Room macro's --GW */
/* This was setup this way to help me track where the room bug is.. please
 * donot mess with it --GW
 */
#define char_to_room(ch,room)	    \
do 				    \
{				    \
char_to_room_code((ch),(room));	    \
if ( !IS_NPC(ch))		   \
roomlog("CHAR_TO_ROOM: (%s) %s: %d\n",(ch)->name,__FILE__,__LINE__);	\
} while (0)

#define char_from_room(ch)       \
do				 \
{				 \
char_from_room_code((ch));	 \
if ( !IS_NPC(ch))		\
roomlog("CHAR_FROM_ROOM: (%s) %s: %d\n",(ch)->name,__FILE__,__LINE__);	\
} while(0)



#define ASSIGN_GSN(gsn, skill)					\
do								\
{								\
    if ( ((gsn) = skill_lookup((skill))) == -1 )		\
	fprintf( stderr, "ASSIGN_GSN: Skill %s not found.\n",	\
		(skill) );					\
} while(0)

#define CHECK_SUBRESTRICTED(ch)					\
do								\
{								\
    if ( (ch)->substate == SUB_RESTRICTED )			\
    {								\
	send_to_char( "You cannot use this command from within another command.\n\r", ch );	\
	return;							\
    }								\
} while(0)


/*
 * Character macros.
 */
#define IS_NPC(ch)		( (ch) && (ch)->act && IS_SET((ch)->act,ACT_IS_NPC))

#define IS_CHAN_MAPPED(ch)      ( !IS_NPC(ch) && \
				(IS_SET((ch)->pcdata->colormap_settings,SCMAP_RUMOR) || \
				IS_SET((ch)->pcdata->colormap_settings,SCMAP_TELL ) || \
				IS_SET((ch)->pcdata->colormap_settings,SCMAP_AUCTION ) || \
				IS_SET((ch)->pcdata->colormap_settings,SCMAP_PLAY) || \
				IS_SET((ch)->pcdata->colormap_settings,SCMAP_INFO) || \
				IS_SET((ch)->pcdata->colormap_settings,SCMAP_QUEST) || \
				IS_SET((ch)->pcdata->colormap_settings,SCMAP_WAR) || \
				IS_SET((ch)->pcdata->colormap_settings,SCMAP_MUSIC) || \
				IS_SET((ch)->pcdata->colormap_settings,SCMAP_ASK) || \
				IS_SET((ch)->pcdata->colormap_settings,SCMAP_FLAME) || \
				IS_SET((ch)->pcdata->colormap_settings,SCMAP_SHOUT) || \
				IS_SET((ch)->pcdata->colormap_settings,SCMAP_YELL) || \
				IS_SET((ch)->pcdata->colormap_settings,SCMAP_CLAN) || \
				IS_SET((ch)->pcdata->colormap_settings,SCMAP_COUNCIL) || \
				IS_SET((ch)->pcdata->colormap_settings,SCMAP_AVATAR) || \
				IS_SET((ch)->pcdata->colormap_settings,SCMAP_TELLS) || \
				IS_SET((ch)->pcdata->colormap_settings,SCMAP_IMMTALK) || \
				IS_SET((ch)->pcdata->colormap_settings,SCMAP_IMP) || \
				IS_SET((ch)->pcdata->colormap_settings,SCMAP_OLYMPUS) || \
				IS_SET((ch)->pcdata->colormap_settings,SCMAP_NEWBIECHAT) || \
				IS_SET((ch)->pcdata->colormap_settings,SCMAP_THINK) \
				))

#define IS_IMMORTAL(ch)		(get_trust((ch)) >= LEVEL_IMMORTAL)
#define IS_HERO(ch)		(get_trust((ch)) >= LEVEL_AVATAR)
#define IS_AFFECTED(ch, sn)	(xIS_SET((ch)->affected_by, (sn)))
#define HAS_BODYPART(ch, part)	((ch)->xflags == 0 || IS_SET((ch)->xflags, (part)))

#define CAN_CAST(ch)		((ch)->class != 2 && (ch)->class != 3)

#define IS_VAMPIRE(ch)		(!IS_NPC(ch)				    \
				&& ((ch)->race==RACE_VAMPIRE		    \
				||  (ch)->class==CLASS_VAMPIRE  	    \
                                ||  (ch)->class==CLASS_WEREWOLF))
#define IS_GOOD(ch)		((ch)->alignment >= 350)
#define IS_EVIL(ch)		((ch)->alignment <= -350)
#define IS_NEUTRAL(ch)		(!IS_GOOD(ch) && !IS_EVIL(ch))

#define IS_AWAKE(ch)		((ch)->position > POS_SLEEPING)

#define IS_OUTSIDE(ch)		( (ch)->in_room && !IS_SET(				    \
				    (ch)->in_room->room_flags,		    \
				    ROOM_INDOORS))

#define IS_DRUNK(ch, drunk)     (number_percent() < \
			        ( (ch)->pcdata->condition[COND_DRUNK] \
				* 2 / (drunk) ) )

#define IS_CLANNED(ch)		(!IS_NPC((ch))				    \
				&& (ch)->pcdata->clan			    \
				&& (ch)->pcdata->clan->clan_type != CLAN_ORDER  \
				&& (ch)->pcdata->clan->clan_type != CLAN_GUILD)

#define IS_ORDERED(ch)		(!IS_NPC((ch))				    \
				&& (ch)->pcdata->clan			    \
				&& (ch)->pcdata->clan->clan_type == CLAN_ORDER)

#define IS_GUILDED(ch)		(!IS_NPC((ch))				    \
				&& (ch)->pcdata->clan			    \
				&& (ch)->pcdata->clan->clan_type == CLAN_GUILD)

#define IS_DEADLYCLAN(ch)	(!IS_NPC((ch))				    \
				&& (ch)->pcdata->clan			    \
				&& (ch)->pcdata->clan->clan_type != CLAN_NOKILL) \
				&& (ch)->pcdata->clan->clan_type != CLAN_ORDER)  \
				&& (ch)->pcdata->clan->clan_type != CLAN_GUILD)

#define IS_DEVOTED(ch)		(!IS_NPC((ch))				    \
				&& (ch)->pcdata->deity)

#define IS_PKILL(ch)            (ch->pcdata && IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ))
#define IS_ADV_DUAL(ch)		(ch->pcdata && IS_SET( ch->pcdata->flags, PCFLAG_ADV_DUAL))
#define IS_ADVANCED(ch)         (ch->pcdata && IS_SET( ch->pcdata->flags, PCFLAG_ADVANCED ))
#define CAN_PKILL(ch)           (IS_PKILL(ch) && ch->level >= 5 && get_age( ch ) >= 18 )

#define WAIT_STATE(ch, npulse)	((ch)->wait = UMAX((ch)->wait, (npulse)))


#define EXIT(ch, door)		( get_exit( (ch)->in_room, door ) )

#define CAN_GO(ch, door)	(EXIT((ch),(door))			 \
				&& (EXIT((ch),(door))->to_room != NULL)  \
                          	&& !IS_SET(EXIT((ch), (door))->exit_info, EX_CLOSED))

#define CAN_USE_SK(ch, sk)	(IS_NPC(ch) || (((ch)->level  >= (sk)->skill_level[(ch)->class ]) || \
				 ((ch)->level2 >= (sk)->skill_level[(ch)->class2]) || \
				 ((ch)->advlevel >= (sk)->skill_level[(ch)->advclass]) || \
			         ((ch)->advlevel2 >= (sk)->skill_level[(ch)->advclass2])))

#define CAN_USE_RACE_SK(ch, sk)	 ((sk)->race_skill_level[(ch)->race] > 0 && \
				  (sk)->race_skill_level[(ch)->race] < 51  ) 

#define SP_MANA(ch, sk)		(UMIN(((sk)->skill_level[(ch)->class ] && \
				      (sk)->skill_level[(ch)->class2]), \
				      (sk)->skill_level[(ch)->advclass]))
				
#define IS_VALID_SN(sn)		( (sn) >=0 && (sn) < MAX_SKILL		     \
				&& skill_table[(sn)]			     \
				&& skill_table[(sn)]->name )

#define IS_VALID_HERB(sn)	( (sn) >=0 && (sn) < MAX_HERB		     \
				&& herb_table[(sn)]			     \
				&& herb_table[(sn)]->name )

#define SPELL_FLAG(skill, flag)	( IS_SET((skill)->flags, (flag)) )
#define SPELL_DAMAGE(skill)	( ((skill)->flags     ) & 7 )
#define SPELL_ACTION(skill)	( ((skill)->flags >> 3) & 7 )
#define SPELL_CLASS(skill)	( ((skill)->flags >> 6) & 7 )
#define SPELL_POWER(skill)	( ((skill)->flags >> 9) & 3 )
#define SET_SDAM(skill, val)	( (skill)->flags =  ((skill)->flags & SDAM_MASK) + ((val) & 7) )
#define SET_SACT(skill, val)	( (skill)->flags =  ((skill)->flags & SACT_MASK) + (((val) & 7) << 3) )
#define SET_SCLA(skill, val)	( (skill)->flags =  ((skill)->flags & SCLA_MASK) + (((val) & 7) << 6) )
#define SET_SPOW(skill, val)	( (skill)->flags =  ((skill)->flags & SPOW_MASK) + (((val) & 3) << 9) )

/* Retired and guest imms. */
#define IS_RETIRED(ch) (ch->pcdata && IS_SET(ch->pcdata->flags,PCFLAG_RETIRED))
#define IS_GUEST(ch) (ch->pcdata && IS_SET(ch->pcdata->flags,PCFLAG_GUEST))

/* RIS by gsn lookups. -- Altrag.
   Will need to add some || stuff for spells that need a special GSN. */

#define IS_FIRE(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_FIRE )
#define IS_COLD(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_COLD )
#define IS_ACID(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_ACID )
#define IS_ELECTRICITY(dt)	( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_ELECTRICITY )
#define IS_ENERGY(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_ENERGY )

#define IS_DRAIN(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_DRAIN )

#define IS_POISON(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_POISON )


#define NOT_AUTHED(ch)		(!IS_NPC(ch) && ch->level == 1 \
			      && IS_SET(ch->pcdata->flags, PCFLAG_UNAUTHED) )

#define IS_WAITING_FOR_AUTH(ch) (!IS_NPC(ch) && ch->desc		     \
			      && ch->pcdata->auth_state == 1		     \
			      && IS_SET(ch->pcdata->flags, PCFLAG_UNAUTHED) ) 

#define is_looter(ch)		(!IS_NPC(ch) && IS_SET((ch)->flags,MOREPC_CORPSELOOTER) )

#define is_lockerlooter(ch)     (!IS_NPC(ch) && IS_SET((ch)->flags,MOREPC_LOCKERLOOTER) )


/*
 * Object macros.
 */
#define CAN_WEAR(obj, part)	(IS_SET((obj)->wear_flags,  (part)))
#define IS_OBJ_STAT(obj, stat)	(IS_SET((obj)->extra_flags, (stat)))

#define IS_LIMITED(obj)		(IS_SET((obj)->second_flags, ITEM2_LIMITED))


/*
 * Description macros.
 */
#define TPERS(ch, looker)	( can_see( (looker), (ch) ) ?		\
				( IS_NPC(ch) ? (ch)->short_descr	\
				 : "" ) : "Someone" )

#define PERS(ch, looker)	( can_see( (looker), (ch) ) ?		\
				( IS_NPC(ch) ? (ch)->short_descr	\
				 : GUILDCHECK(ch) ) : "Someone" )

#define log_string( txt )	( log_string_plus( (txt), LOG_NORMAL, LEVEL_LOG ) )


/*
 * Structure for a command in the command lookup table.
 */
struct	cmd_type
{
    CMDTYPE *		next;
    char *		name;
    DO_FUN *		do_fun;
    sh_int		position;
    sh_int		level;
    sh_int		log;
    struct		timerset	userec;
};



/*
 * Structure for a social in the socials table.
 */
struct	social_type
{
    SOCIALTYPE *	next;
    char *		name;
    char *		char_no_arg;
    char *		others_no_arg;
    char *		char_found;
    char *		others_found;
    char *		vict_found;
    char *		char_auto;
    char *		others_auto;
};



/*
 * Global constants.
 */
long	SHARE_COST;
bool    SHARE_STATUS;
char	membuf[MSL];
extern  WARRENT_DATA *first_warrent;
extern  WARRENT_DATA *last_warrent;
extern  time_t last_restore_all_time;
extern  time_t boot_time;  /* this should be moved down */
extern  HOUR_MIN_SEC * set_boot_time; 
extern  struct  tm *new_boot_time;
extern  time_t new_boot_time_t;
extern  int ptotal;
extern  int ctotal;
extern  int chaos;
extern  sh_int PULSE_PER_SECOND;
extern  bool TURBO_FIGHT;
extern  int  TURBO_NUM;
extern  long  CASINO_BANK;
extern  int doubleexp;
extern  int war;
extern  TIMER * KOMBAT_TIME;
extern  TOPTEN_DATA *first_topten;
extern  TOPTEN_DATA *last_topten;
extern  bool TOPTEN_ACTIVE;
extern  long lowest_toptens[10];
extern  bool DRAGON_DROP;
extern	const	struct	str_app_type	str_app		[26];
extern	const	struct	int_app_type	int_app		[26];
extern	const	struct	wis_app_type	wis_app		[26];
extern	const	struct	dex_app_type	dex_app		[26];
extern	const	struct	con_app_type	con_app		[26];
extern	const	struct	cha_app_type	cha_app		[26];
extern  const	struct	lck_app_type	lck_app		[26];

extern	const	struct	liq_type	liq_table	[LIQ_MAX];
extern	char *	const			attack_table	[18];

extern	char *	const	skill_tname	[];
extern	sh_int	const	movement_loss	[SECT_MAX];
extern	char *	const	dir_name	[];
extern	char *	const	where_name	[];
extern	const	sh_int	rev_dir		[];
extern	const	int	trap_door	[];
extern	char *	const	r_flags		[];
extern	char *	const	w_flags		[];
extern	char *	const	o_flags		[];
extern	char *	const	obj_flags	[];
extern	char *	const	kombat_flags	[];
extern	char *	const	a_flags		[];
extern	char *	const	o_types		[];
extern	char *	const	a_types		[];
extern	char *	const	act_flags	[];
extern	char *	const	curse_table	[];
extern	char *	const	censor_table	[];
extern	char *	const	acttwo_flags	[];
extern	char *	const	smart_flags	[];
extern	char *	const	morepc_flags	[];
extern	char *	const	plr_flags	[];
extern	char *	const	pc_flags	[];
extern	char *	const	trap_flags	[];
extern	char *	const	ris_flags	[];
extern	char *	const	trig_flags	[];
extern	char *	const	part_flags	[];
extern	char *	const	npc_race	[];
extern	char *	const	npc_class	[];
extern	char *	const	defense_flags	[];
extern	char *	const	attack_flags	[];
extern	char *	const	area_flags	[];

extern	int	const	lang_array      [];
extern	char *	const	lang_names      [];

/*
 * Global variables.
 */
extern	int	numobjsloaded;
extern	int	nummobsloaded;
extern	int	physicalobjects;
extern	int	num_descriptors;
extern	struct	system_data		sysdata;
extern	int	top_sn;
extern	int	top_vroom;
extern	int	top_herb;
extern  bool    noenforce;
extern  char    offense[1024];
extern  char	mudargs[MSL];

extern		CMDTYPE		  *	command_hash	[126];

extern		struct	race_type *     race_table	[MAX_RACE];
extern		struct	class_type *	class_table	[MAX_CLASS];
extern		char *			title_table	[MAX_CLASS]
							[MAX_LEVEL+1]
							[2];

extern		SKILLTYPE	  *	skill_table	[MAX_SKILL];
extern		SOCIALTYPE	  *	social_index	[27];
extern		CHAR_DATA	  *	cur_char;
extern		ROOM_INDEX_DATA	  *	cur_room;
extern		bool			cur_char_died;
extern		ch_ret			global_retcode;
extern		SKILLTYPE	  *	herb_table	[MAX_HERB];

extern		int			cur_obj;
extern		int			cur_obj_serial;
extern		bool			cur_obj_extracted;
extern		obj_ret			global_objcode;

extern		HELP_DATA	  *	first_help;
extern		HELP_DATA	  *	last_help;
extern		SHOP_DATA	  *	first_shop;
extern		SHOP_DATA	  *	last_shop;
extern		REPAIR_DATA	  *	first_repair;
extern		REPAIR_DATA	  *	last_repair;

extern          PERMBAN_DATA      *     ban_list;
extern		BAN_DATA	  *	first_ban;
extern		BAN_DATA	  *	last_ban;
extern		CHAR_DATA	  *	first_char;
extern		CHAR_DATA	  *	last_char;
extern		DESCRIPTOR_DATA   *	first_descriptor;
extern		DESCRIPTOR_DATA   *	last_descriptor;
extern		BOARD_DATA	  *	first_board;
extern		BOARD_DATA	  *	last_board;
extern		REFFERAL_DATA	  *	first_refferal;
extern		REFFERAL_DATA	  *	last_refferal;
extern		ZONE_DATA	  *	first_zone;
extern          ZONE_DATA	  *     last_zone;
extern		MAIL_DATA	  *	first_mail;
extern		MAIL_DATA	  *	last_mail;
extern		MODE_DATA	  *	first_mode;
extern		MODE_DATA	  *	last_mode;
extern		OBJ_DATA	  *	first_object;
extern		OBJ_DATA	  *	last_object;
extern		CLAN_DATA	  *	first_clan;
extern		CLAN_DATA	  *	last_clan;
extern 		COUNCIL_DATA 	  *	first_council;
extern		COUNCIL_DATA	  * 	last_council;
extern		DEITY_DATA	  *	first_deity;
extern		DEITY_DATA	  *	last_deity;
extern		AREA_DATA	  *	first_area;
extern		AREA_DATA	  *	last_area;
extern		AREA_DATA	  *	first_build;
extern		AREA_DATA	  *	last_build;
extern		AREA_DATA	  *	first_asort;
extern		AREA_DATA	  *	last_asort;
extern		AREA_DATA	  *	first_bsort;
extern		AREA_DATA	  *	last_bsort;
extern		KOMBAT_DATA	  *	kombat;
extern		CHALLENGE_DATA	  *	first_challenge;
extern		CHALLENGE_DATA	  *	last_challenge;
extern		ARENA_DATA	  *	first_arena;
extern		ARENA_DATA	  *	last_arena;
extern 		PETITION_DATA	  *	first_petition;
extern 		PETITION_DATA	  *	last_petition;
/*
extern		GOD_DATA	  *	first_imm;
extern		GOD_DATA	  *	last_imm;
*/
extern		TELEPORT_DATA	  *	first_teleport;
extern		TELEPORT_DATA	  *	last_teleport;
extern		OBJ_DATA	  *	extracted_obj_queue;
extern		EXTRACT_CHAR_DATA *	extracted_char_queue;
extern		OBJ_DATA	  *	save_equipment[MAX_WEAR][MAX_LAYERS];
extern		CHAR_DATA	  *	quitting_char;
extern		CHAR_DATA	  *	loading_char;
extern		CHAR_DATA	  *	saving_char;
extern		OBJ_DATA	  *	all_obj;

extern		char			bug_buf		[];
extern		time_t			current_time;
extern		bool			fLogAll;
extern		FILE *			fpReserve;
extern		FILE *			fpLOG;
extern		FILE *			fpMem;
extern		char			log_buf		[];
extern		TIME_INFO_DATA		time_info;
extern		WEATHER_DATA		weather_info;

extern          AUCTION_DATA      *     auction;
extern		struct act_prog_data *	mob_act_list;


/*
 * Command functions.
 * Defined in act_*.c (mostly).
 */
DECLARE_DO_FUN( do_attribute    );
DECLARE_DO_FUN( do_approve      );
DECLARE_DO_FUN( do_releaseme	);
DECLARE_DO_FUN( do_arrest	);
DECLARE_DO_FUN( do_judge	);
DECLARE_DO_FUN( do_warrent	);
DECLARE_DO_FUN( do_wizlog	);
DECLARE_DO_FUN( do_finger       );
DECLARE_DO_FUN( do_fire		);
DECLARE_DO_FUN( do_locker	);
DECLARE_DO_FUN( do_loop		);
DECLARE_DO_FUN( do_limits	);
DECLARE_DO_FUN( do_bet		);
DECLARE_DO_FUN( do_arena	);
DECLARE_DO_FUN( do_avator	);
DECLARE_DO_FUN( do_challenge	);
DECLARE_DO_FUN( do_crlog	);
DECLARE_DO_FUN( do_ctf		);
DECLARE_DO_FUN( do_changes	);
DECLARE_DO_FUN( do_color	);
DECLARE_DO_FUN( do_colors	);
DECLARE_DO_FUN( do_setrace	);
DECLARE_DO_FUN( do_host		);
DECLARE_DO_FUN( do_share	);
DECLARE_DO_FUN( do_safety	);
DECLARE_DO_FUN( do_stay		);
DECLARE_DO_FUN( do_special	);
DECLARE_DO_FUN( do_sever	);
DECLARE_DO_FUN( do_steer	);
DECLARE_DO_FUN( do_showplayer	);
DECLARE_DO_FUN( do_showrace	);
DECLARE_DO_FUN( do_makerace	);
DECLARE_DO_FUN( do_mode		);
DECLARE_DO_FUN( do_memlog	);
DECLARE_DO_FUN( do_buglog	);
DECLARE_DO_FUN( do_remort	);
DECLARE_DO_FUN( do_refferal	);
DECLARE_DO_FUN( do_cperm	);
DECLARE_DO_FUN( do_atrain	);
DECLARE_DO_FUN( do_rchat	);
DECLARE_DO_FUN( do_rtell	);
DECLARE_DO_FUN( do_rreply	);
DECLARE_DO_FUN( do_rwho		);
DECLARE_DO_FUN( do_rquery	);
DECLARE_DO_FUN( do_sign		);
DECLARE_DO_FUN( do_ic		);
DECLARE_DO_FUN( do_ooc		);
DECLARE_DO_FUN( do_istart	);
DECLARE_DO_FUN( do_imc		);
DECLARE_DO_FUN( do_ilist	);
DECLARE_DO_FUN( do_imclist	);
DECLARE_DO_FUN( do_isetup	);
DECLARE_DO_FUN( do_irc		);
DECLARE_DO_FUN( do_endirc	);
DECLARE_DO_FUN( do_exchange	);
DECLARE_DO_FUN( do_addirc	);
DECLARE_DO_FUN( do_irckick	);
DECLARE_DO_FUN( do_rcode	);
DECLARE_DO_FUN( do_rcode	);
DECLARE_DO_FUN( do_beep		);
DECLARE_DO_FUN( do_balance	);
DECLARE_DO_FUN( do_deposit	);
DECLARE_DO_FUN( do_withdraw     );
DECLARE_DO_FUN( do_rbeep	);
DECLARE_DO_FUN( do_istats	);
DECLARE_DO_FUN( do_rping	);
DECLARE_DO_FUN( do_icommand	);
DECLARE_DO_FUN( do_rchannels	);
DECLARE_DO_FUN( do_rimm		);
DECLARE_DO_FUN( do_rinfo	);
DECLARE_DO_FUN( do_ichannels	);
DECLARE_DO_FUN( do_rsockets	);
DECLARE_DO_FUN( do_rconnect	);
DECLARE_DO_FUN( do_rdisconnect	);
DECLARE_DO_FUN( do_rignore	);
DECLARE_DO_FUN( do_rfinger	);
DECLARE_DO_FUN( do_rwhois	);
DECLARE_DO_FUN( do_mailq	);
DECLARE_DO_FUN( do_rchanset	);
DECLARE_DO_FUN( skill_notfound	);
DECLARE_DO_FUN( do_badvance     );
DECLARE_DO_FUN( do_delete	);
DECLARE_DO_FUN( do_pray		);
DECLARE_DO_FUN( do_portarea	);
DECLARE_DO_FUN( do_peek		);
DECLARE_DO_FUN( do_psocial	);
DECLARE_DO_FUN( do_psocials	);
DECLARE_DO_FUN( do_play		);
DECLARE_DO_FUN( do_prize	);
DECLARE_DO_FUN( do_plog		);
DECLARE_DO_FUN( do_pipe		);
DECLARE_DO_FUN( do_aassign	);
DECLARE_DO_FUN( do_aota		);
DECLARE_DO_FUN( do_procure	);
DECLARE_DO_FUN( do_diary	);
DECLARE_DO_FUN( do_corpse	);
DECLARE_DO_FUN( do_suspend	);
DECLARE_DO_FUN( do_startroom	);
DECLARE_DO_FUN( do_newbie	);
DECLARE_DO_FUN( do_nslookup	);
DECLARE_DO_FUN( do_refit	);
DECLARE_DO_FUN( do_renamearea	);
DECLARE_DO_FUN( do_turbospeed	);
DECLARE_DO_FUN( do_topten	);
DECLARE_DO_FUN( do_test		);
DECLARE_DO_FUN( do_chaos	);
DECLARE_DO_FUN( do_warmode	);
DECLARE_DO_FUN( do_warme	);
DECLARE_DO_FUN( do_doubleexp	);
DECLARE_DO_FUN( do_nukerep	);
DECLARE_DO_FUN( do_incognito	);
DECLARE_DO_FUN( do_ignore	);
DECLARE_DO_FUN( do_alias	);
DECLARE_DO_FUN( do_unalias	);
DECLARE_DO_FUN( do_use		);
DECLARE_DO_FUN( do_upstatlog	);
DECLARE_DO_FUN( do_rental	);
DECLARE_DO_FUN( do_checkout	);
DECLARE_DO_FUN( do_bonus	);
DECLARE_DO_FUN( do_bounty	);
DECLARE_DO_FUN( do_unbounty	);
DECLARE_DO_FUN( do_pbounty	);
DECLARE_DO_FUN( do_marry	);
DECLARE_DO_FUN( do_engage	);
DECLARE_DO_FUN( do_home		);
DECLARE_DO_FUN( do_history	);
DECLARE_DO_FUN( do_unmarry	);
DECLARE_DO_FUN( do_disable	);
DECLARE_DO_FUN(	do_advance	);
DECLARE_DO_FUN( do_addlag	);
DECLARE_DO_FUN( do_donate	);
DECLARE_DO_FUN(	do_advance2	);
DECLARE_DO_FUN( do_affected     );
DECLARE_DO_FUN( do_afk          );
DECLARE_DO_FUN(	do_aid		);
DECLARE_DO_FUN(	do_allow	);
DECLARE_DO_FUN(	do_permallow	);
DECLARE_DO_FUN( do_ansi		);
DECLARE_DO_FUN(	do_answer	);
DECLARE_DO_FUN( do_apply	);
DECLARE_DO_FUN(	do_appraise	);
DECLARE_DO_FUN(	do_areas	);
DECLARE_DO_FUN( do_aset		);
DECLARE_DO_FUN(	do_ask		);
DECLARE_DO_FUN( do_astat	);
DECLARE_DO_FUN(	do_at		);
DECLARE_DO_FUN(	do_auction	);
DECLARE_DO_FUN( do_authorize	);
DECLARE_DO_FUN( do_avtalk	);
DECLARE_DO_FUN(	do_assassinate	);
DECLARE_DO_FUN(	do_backstab	);
DECLARE_DO_FUN(	do_balzhur	);
DECLARE_DO_FUN(	do_bamfin	);
DECLARE_DO_FUN(	do_bamfout	);
DECLARE_DO_FUN(	do_ban		);
DECLARE_DO_FUN(	do_bash		);
DECLARE_DO_FUN( do_bashdoor     );
DECLARE_DO_FUN( do_berserk	);
DECLARE_DO_FUN( do_bestow	);
DECLARE_DO_FUN( do_bestowarea	);
DECLARE_DO_FUN(	do_bio		);
DECLARE_DO_FUN(	do_bite		);
DECLARE_DO_FUN( do_boards	);
DECLARE_DO_FUN( do_bodybag	);
DECLARE_DO_FUN(	do_brandish	);
DECLARE_DO_FUN( do_brew 	);
DECLARE_DO_FUN( do_bset		);
DECLARE_DO_FUN( do_bstat	);
DECLARE_DO_FUN(	do_bug		);
DECLARE_DO_FUN( do_bury		);
DECLARE_DO_FUN(	do_buy		);
DECLARE_DO_FUN(	do_cast		);
DECLARE_DO_FUN(	do_crashlog	);
DECLARE_DO_FUN( do_cdonate	);
DECLARE_DO_FUN( do_calculate	);
DECLARE_DO_FUN(	do_cedit	);
DECLARE_DO_FUN( do_censor	);
DECLARE_DO_FUN(	do_channels	);
DECLARE_DO_FUN(	do_chat		);
DECLARE_DO_FUN(	do_check_vnums  );
DECLARE_DO_FUN( do_circle	);
DECLARE_DO_FUN(	do_clans	);
DECLARE_DO_FUN( do_clan		);
DECLARE_DO_FUN(	do_clantalk	);
DECLARE_DO_FUN(	do_claw		);
DECLARE_DO_FUN(	do_climb	);
DECLARE_DO_FUN(	do_close	);
DECLARE_DO_FUN(	do_cmdtable	);
DECLARE_DO_FUN(	do_cmenu	);
DECLARE_DO_FUN(	do_commands	);
DECLARE_DO_FUN(	do_comment	);
DECLARE_DO_FUN(	do_compare	);
DECLARE_DO_FUN(	do_config	);
DECLARE_DO_FUN(	do_consider	);
DECLARE_DO_FUN( do_copyover	);
DECLARE_DO_FUN( do_council_induct);
DECLARE_DO_FUN( do_council_outcast);
DECLARE_DO_FUN( do_councils	);
DECLARE_DO_FUN( do_counciltalk	);
DECLARE_DO_FUN( do_credits	);
DECLARE_DO_FUN(	do_cset		);
DECLARE_DO_FUN( do_deities	);
DECLARE_DO_FUN(	do_deny		);
DECLARE_DO_FUN(	do_description	);
DECLARE_DO_FUN( do_destro       );
DECLARE_DO_FUN( do_destroy      );
DECLARE_DO_FUN(	do_detrap	);
DECLARE_DO_FUN( do_devote	);
DECLARE_DO_FUN( do_dig		);
DECLARE_DO_FUN(	do_disarm	);
DECLARE_DO_FUN(	do_disconnect	);
DECLARE_DO_FUN( do_dismiss	);
DECLARE_DO_FUN( do_dismount	);
DECLARE_DO_FUN(	do_dmesg	);
DECLARE_DO_FUN(	do_down		);
DECLARE_DO_FUN( do_drag 	);
DECLARE_DO_FUN(	do_drink	);
DECLARE_DO_FUN(	do_drop		);
DECLARE_DO_FUN(	do_diagnose	);
DECLARE_DO_FUN(	do_east		);
DECLARE_DO_FUN(	do_eat		);
DECLARE_DO_FUN(	do_echo		);
DECLARE_DO_FUN(	do_emote	);
DECLARE_DO_FUN(	do_empty	);
DECLARE_DO_FUN(	do_enter	);
DECLARE_DO_FUN(	do_equipment	);
DECLARE_DO_FUN(	do_examine	);
DECLARE_DO_FUN(	do_exits	);
DECLARE_DO_FUN(	do_feed		);
DECLARE_DO_FUN(	do_fill		);
DECLARE_DO_FUN(	do_fixchar	);
DECLARE_DO_FUN( do_flame        );
DECLARE_DO_FUN(	do_flee		);
DECLARE_DO_FUN( do_foldarea	);
DECLARE_DO_FUN(	do_follow	);
DECLARE_DO_FUN( do_for          );
DECLARE_DO_FUN(	do_force	);
DECLARE_DO_FUN( do_forceclose	);
DECLARE_DO_FUN( do_fquit	);     /* Gorog */
DECLARE_DO_FUN( do_form_password);
DECLARE_DO_FUN(	do_freeze	);
DECLARE_DO_FUN(	do_forward	);
DECLARE_DO_FUN(	do_gredit	);
DECLARE_DO_FUN(	do_gmedit	);
DECLARE_DO_FUN(	do_goedit	);
DECLARE_DO_FUN(	do_get		);
DECLARE_DO_FUN(	do_give		);
DECLARE_DO_FUN(	do_glance	);
DECLARE_DO_FUN( do_gold         );
DECLARE_DO_FUN(	do_goto		);
DECLARE_DO_FUN( do_gouge	);
DECLARE_DO_FUN(	do_group	);
DECLARE_DO_FUN(	do_grub 	);
DECLARE_DO_FUN(	do_gtell	);
DECLARE_DO_FUN( do_guilds       );
DECLARE_DO_FUN( do_guild        );
DECLARE_DO_FUN( do_guildtalk    );
DECLARE_DO_FUN( do_hedit	);
DECLARE_DO_FUN( do_hell		);
DECLARE_DO_FUN(	do_help		);
DECLARE_DO_FUN(	do_hide		);
DECLARE_DO_FUN( do_bj_hit	);
DECLARE_DO_FUN( do_hitall	);
DECLARE_DO_FUN( do_hlist	);
DECLARE_DO_FUN(	do_holylight	);
DECLARE_DO_FUN(	do_homepage	);
DECLARE_DO_FUN( do_hset		);
DECLARE_DO_FUN(	do_ide		);
DECLARE_DO_FUN(	do_idea		);
DECLARE_DO_FUN(	do_immortalize	);
DECLARE_DO_FUN(	do_immtalk	);
DECLARE_DO_FUN( do_installarea	);
DECLARE_DO_FUN( do_instaroom	);
DECLARE_DO_FUN( do_instazone	);
DECLARE_DO_FUN(	do_inventory	);
DECLARE_DO_FUN(	do_invis	);
DECLARE_DO_FUN( do_khistory	);
DECLARE_DO_FUN(	do_kick		);
DECLARE_DO_FUN(	do_kombat	);
DECLARE_DO_FUN(	do_kill		);
DECLARE_DO_FUN( do_languages    );
DECLARE_DO_FUN( do_last		);
DECLARE_DO_FUN(	do_leave	);
DECLARE_DO_FUN(	do_level	);
DECLARE_DO_FUN(	do_level2	);
DECLARE_DO_FUN(	do_light	);
DECLARE_DO_FUN(	do_list		);
DECLARE_DO_FUN(	do_litterbug	);
DECLARE_DO_FUN( do_loadarea	);
DECLARE_DO_FUN( do_loadup	);
DECLARE_DO_FUN(	do_lock		);
DECLARE_DO_FUN(	do_log		);
DECLARE_DO_FUN(	do_look		);
DECLARE_DO_FUN(	do_low_purge	);
DECLARE_DO_FUN( do_mail		);
DECLARE_DO_FUN( do_receive	);
DECLARE_DO_FUN( do_read		);
DECLARE_DO_FUN(	do_make		);
DECLARE_DO_FUN(	do_makeboard	);
DECLARE_DO_FUN(	do_makeclan	);
DECLARE_DO_FUN( do_makecouncil 	);
DECLARE_DO_FUN( do_makedeity	);
DECLARE_DO_FUN( do_makeguild    );
DECLARE_DO_FUN( do_makerepair	);
DECLARE_DO_FUN( do_makeshop	);
DECLARE_DO_FUN( do_makewizlist	);
DECLARE_DO_FUN( do_massign	);
DECLARE_DO_FUN( do_mudlog	);
DECLARE_DO_FUN(	do_memory	);
DECLARE_DO_FUN( do_mcreate	);
DECLARE_DO_FUN( do_mdelete	);
DECLARE_DO_FUN(	do_mfind	);
DECLARE_DO_FUN(	do_minvoke	);
DECLARE_DO_FUN( do_mlist	);
DECLARE_DO_FUN( do_mount	);
DECLARE_DO_FUN(	do_mset		);
DECLARE_DO_FUN(	do_mstat	);
DECLARE_DO_FUN( do_mudstat      );
DECLARE_DO_FUN(	do_murde	);
DECLARE_DO_FUN(	do_murder	);
DECLARE_DO_FUN(	do_muse		);
DECLARE_DO_FUN(	do_music	);
DECLARE_DO_FUN(	do_mwhere	);
DECLARE_DO_FUN( do_perm		);
DECLARE_DO_FUN( do_name		);
DECLARE_DO_FUN( do_mname	);
DECLARE_DO_FUN( do_newbiechat   );
DECLARE_DO_FUN( do_newbieset    );
DECLARE_DO_FUN( do_newscore	);
DECLARE_DO_FUN( do_newzones	);
DECLARE_DO_FUN(	do_noemote	);
DECLARE_DO_FUN( do_noecho       );
DECLARE_DO_FUN( do_noresolve	);
DECLARE_DO_FUN(	do_north	);
DECLARE_DO_FUN( do_northeast	);
DECLARE_DO_FUN( do_northwest	);
DECLARE_DO_FUN(	do_notell	);
DECLARE_DO_FUN( do_notitle      );
DECLARE_DO_FUN(	do_noteroom	);
DECLARE_DO_FUN( do_oassign	);
DECLARE_DO_FUN( do_ocreate	);
DECLARE_DO_FUN( do_odelete	);
DECLARE_DO_FUN(	do_ofind	);
DECLARE_DO_FUN(	do_ogrub 	);
DECLARE_DO_FUN(	do_oinvoke	);
DECLARE_DO_FUN(	do_oldscore	);
DECLARE_DO_FUN( do_olist	);
DECLARE_DO_FUN(	do_open		);
DECLARE_DO_FUN( do_opentourney  );
DECLARE_DO_FUN(	do_order	);
DECLARE_DO_FUN(	do_orders	);
DECLARE_DO_FUN(	do_ordertalk	);
DECLARE_DO_FUN(	do_oset		);
DECLARE_DO_FUN(	do_ostat	);
DECLARE_DO_FUN(	do_ot		);
DECLARE_DO_FUN(	do_owhere	);
DECLARE_DO_FUN( do_pager	);
DECLARE_DO_FUN(	do_pardon	);
DECLARE_DO_FUN(	do_password	);
DECLARE_DO_FUN(	do_peace	);
DECLARE_DO_FUN(	do_pick		);
DECLARE_DO_FUN( do_poison_weapon);
DECLARE_DO_FUN(	do_pose		);
DECLARE_DO_FUN(	do_practice	);
DECLARE_DO_FUN( do_prompt	);
DECLARE_DO_FUN( do_pstat	);
DECLARE_DO_FUN( do_pull		);
DECLARE_DO_FUN(	do_punch	);
DECLARE_DO_FUN(	do_purge	);
DECLARE_DO_FUN( do_push		);
DECLARE_DO_FUN(	do_put		);
DECLARE_DO_FUN( do_qpset	);
DECLARE_DO_FUN(	do_quaff	);
DECLARE_DO_FUN(	do_quest	);
DECLARE_DO_FUN(	do_qwest	);
DECLARE_DO_FUN(	do_qui		);
DECLARE_DO_FUN(	do_quit		);
DECLARE_DO_FUN( do_quit2	);
DECLARE_DO_FUN(	do_rank	        );
DECLARE_DO_FUN( do_rassign	);
DECLARE_DO_FUN( do_rat		);
DECLARE_DO_FUN( do_mat		);
DECLARE_DO_FUN( do_mend		);
DECLARE_DO_FUN( do_map		);
DECLARE_DO_FUN( do_oat		);
DECLARE_DO_FUN( do_rdelete	);
DECLARE_DO_FUN(	do_reboo	);
DECLARE_DO_FUN(	do_reboot	);
DECLARE_DO_FUN( do_recall	);
DECLARE_DO_FUN(	do_recho	);
DECLARE_DO_FUN(	do_recite	);
DECLARE_DO_FUN(	do_redit	);
DECLARE_DO_FUN( do_regoto       );
DECLARE_DO_FUN(	do_remove	);
DECLARE_DO_FUN(	do_rent		);
DECLARE_DO_FUN(	do_repair	);
DECLARE_DO_FUN(	do_repairset	);
DECLARE_DO_FUN(	do_repairshops	);
DECLARE_DO_FUN(	do_repairstat	);
DECLARE_DO_FUN(	do_reply	);
DECLARE_DO_FUN(	do_report	);
DECLARE_DO_FUN(	do_rescue	);
DECLARE_DO_FUN(	do_rest		);
DECLARE_DO_FUN( do_reset	);
DECLARE_DO_FUN(	do_restore	);
DECLARE_DO_FUN(	do_restoretime	);
DECLARE_DO_FUN(	do_restrict	);
DECLARE_DO_FUN( do_retire       );
DECLARE_DO_FUN( do_retran       );
DECLARE_DO_FUN(	do_return	);
DECLARE_DO_FUN(	do_revert	);
DECLARE_DO_FUN( do_rip		);
DECLARE_DO_FUN( do_rlist	);
DECLARE_DO_FUN( do_rreset	);
DECLARE_DO_FUN(	do_rset		);
DECLARE_DO_FUN(	do_rstat	);
DECLARE_DO_FUN(	do_sacrifice	);
DECLARE_DO_FUN(	do_save		);
DECLARE_DO_FUN( do_savearea	);
DECLARE_DO_FUN(	do_say		);
DECLARE_DO_FUN( do_allscan	);
DECLARE_DO_FUN( do_scan         );
DECLARE_DO_FUN(	do_score	);
DECLARE_DO_FUN( do_scribe       );
DECLARE_DO_FUN( do_search	);
DECLARE_DO_FUN(	do_sedit	);
DECLARE_DO_FUN(	do_sell		);
DECLARE_DO_FUN( do_set_boot_time);
DECLARE_DO_FUN( do_setclan	);
DECLARE_DO_FUN( do_setclass	);
DECLARE_DO_FUN( do_setcouncil   );
DECLARE_DO_FUN( do_setdeity	);
DECLARE_DO_FUN(	do_shops	);
DECLARE_DO_FUN(	do_shopset	);
DECLARE_DO_FUN(	do_shopstat	);
DECLARE_DO_FUN(	do_shout	);
DECLARE_DO_FUN(	do_scatter	);
DECLARE_DO_FUN( do_shove  	);
DECLARE_DO_FUN( do_showclass	);
DECLARE_DO_FUN( do_showclan	);
DECLARE_DO_FUN( do_showcouncil	);
DECLARE_DO_FUN( do_showdeity	);
DECLARE_DO_FUN(	do_shutdow	);
DECLARE_DO_FUN(	do_shutdown	);
DECLARE_DO_FUN(	do_silence	);
DECLARE_DO_FUN(	do_sit		);
DECLARE_DO_FUN(	do_sla		);
DECLARE_DO_FUN(	do_slay		);
DECLARE_DO_FUN(	do_sleep	);
DECLARE_DO_FUN( do_slice        );
DECLARE_DO_FUN( do_slist        );
DECLARE_DO_FUN(	do_slookup	);
DECLARE_DO_FUN(	do_smoke	);
DECLARE_DO_FUN(	do_sneak	);
DECLARE_DO_FUN(	do_snoop	);
DECLARE_DO_FUN(	do_sober	);
DECLARE_DO_FUN(	do_socials	);
DECLARE_DO_FUN(	do_south	);
DECLARE_DO_FUN(	do_southeast	);
DECLARE_DO_FUN(	do_southwest	);
DECLARE_DO_FUN( do_speak        );
DECLARE_DO_FUN(	do_split	);
DECLARE_DO_FUN(	do_sset		);
DECLARE_DO_FUN(	do_stand	);
DECLARE_DO_FUN( do_starttourney );
DECLARE_DO_FUN(	do_steal	);
DECLARE_DO_FUN(	do_sting	);
DECLARE_DO_FUN(	do_stun		);
DECLARE_DO_FUN( do_supplicate	);
DECLARE_DO_FUN(	do_switch	);
DECLARE_DO_FUN(	do_tail		);
DECLARE_DO_FUN(	do_tamp		);
DECLARE_DO_FUN(	do_tell		);
DECLARE_DO_FUN(	do_think	);
DECLARE_DO_FUN(	do_olympus	);
DECLARE_DO_FUN(	do_time		);
DECLARE_DO_FUN(	do_timecmd	);
DECLARE_DO_FUN(	do_title	);
DECLARE_DO_FUN( do_track	);
DECLARE_DO_FUN(	do_transfer	);
DECLARE_DO_FUN(	do_trust	);
DECLARE_DO_FUN(	do_typo		);
DECLARE_DO_FUN(	do_unfoldarea	);
DECLARE_DO_FUN( do_unhell	);
DECLARE_DO_FUN(	do_unlock	);
DECLARE_DO_FUN( do_unsilence    );
DECLARE_DO_FUN(	do_up		);
DECLARE_DO_FUN(	do_users	);
DECLARE_DO_FUN(	do_value	);
DECLARE_DO_FUN(	do_view 	);
DECLARE_DO_FUN(	do_visible	);
DECLARE_DO_FUN( do_vnums	);
DECLARE_DO_FUN( do_vsearch	);
DECLARE_DO_FUN(	do_wake		);
DECLARE_DO_FUN( do_wartalk      );
DECLARE_DO_FUN(	do_wear		);
DECLARE_DO_FUN(	do_weather	);
DECLARE_DO_FUN(	do_west		);
DECLARE_DO_FUN(	do_where	);
DECLARE_DO_FUN(	do_who		);
DECLARE_DO_FUN(	do_whois	);
DECLARE_DO_FUN(	do_wimpy	);
DECLARE_DO_FUN(	do_wizhelp	);
DECLARE_DO_FUN( do_wizlist	);
DECLARE_DO_FUN(	do_wizlock	);
DECLARE_DO_FUN(	do_yell		);
DECLARE_DO_FUN(	do_zap		);
DECLARE_DO_FUN( do_zones	);
DECLARE_DO_FUN( do_zcreate	); /* Zone Creation .. GW */
DECLARE_DO_FUN( do_zset		); /* Zone Creation .. GW */
DECLARE_DO_FUN( do_zstat	); /* Zone Creation .. GW */
DECLARE_DO_FUN( do_zlist	); /* Zone Creation .. GW */
DECLARE_DO_FUN( do_appeal       ); /* Brulok */
/* mob prog stuff */
DECLARE_DO_FUN( do_mpfear );
DECLARE_DO_FUN( do_mpassassinate );
DECLARE_DO_FUN( do_mphate );
DECLARE_DO_FUN( do_mphunt ); 
DECLARE_DO_FUN( do_mp_close_passage );
DECLARE_DO_FUN( do_mp_damage );
DECLARE_DO_FUN( do_mp_restore );
DECLARE_DO_FUN( do_mp_open_passage );
DECLARE_DO_FUN( do_mp_practice );
DECLARE_DO_FUN( do_mp_slay);
DECLARE_DO_FUN( do_mpadvance    );
DECLARE_DO_FUN( do_mpasound     );
DECLARE_DO_FUN( do_mpat         );
DECLARE_DO_FUN( do_mpaward	);
DECLARE_DO_FUN( do_mpdream	);
DECLARE_DO_FUN( do_mp_deposit	);
DECLARE_DO_FUN( do_mp_withdraw	);
DECLARE_DO_FUN( do_mpecho       );
DECLARE_DO_FUN( do_mpechoaround );
DECLARE_DO_FUN( do_mpechoat     );
DECLARE_DO_FUN( do_mpedit       );
DECLARE_DO_FUN( do_mrange       );
DECLARE_DO_FUN( do_opedit       );
DECLARE_DO_FUN( do_orange       );
DECLARE_DO_FUN( do_rpedit       );
DECLARE_DO_FUN( do_mpforce      );
DECLARE_DO_FUN( do_mpinvis	);
DECLARE_DO_FUN( do_mpgoto       );
DECLARE_DO_FUN( do_mpjunk       );
DECLARE_DO_FUN( do_mpkill       );
DECLARE_DO_FUN( do_mpmload      );
DECLARE_DO_FUN( do_mpaligngood  );
DECLARE_DO_FUN( do_mpalignevil  );
DECLARE_DO_FUN( do_mpmset	);
DECLARE_DO_FUN( do_mpnothing	);
DECLARE_DO_FUN( do_mpoload      );
DECLARE_DO_FUN( do_mposet	);
DECLARE_DO_FUN( do_mppurge      );
DECLARE_DO_FUN( do_mpstat       );
DECLARE_DO_FUN( do_opstat       );
DECLARE_DO_FUN( do_rpstat       );
DECLARE_DO_FUN( do_mptransfer   );
DECLARE_DO_FUN( do_mpapply	);
DECLARE_DO_FUN( do_mpapplyb  	);
DECLARE_DO_FUN( do_mppkset	);
DECLARE_DO_FUN( do_mpfavor	);


 
/*
 * Psi spell_functions, in magic.c (by Thelonius).
 */
DECLARE_SPELL_FUN(      spell_adrenaline_control);
DECLARE_SPELL_FUN(      spell_agitation         );
DECLARE_SPELL_FUN(      spell_aura_sight        );
DECLARE_SPELL_FUN(      spell_awe               );
DECLARE_SPELL_FUN(      spell_ballistic_attack  );
DECLARE_SPELL_FUN(      spell_biofeedback       );
DECLARE_SPELL_FUN(      spell_cell_adjustment   );
DECLARE_SPELL_FUN(      spell_combat_mind       );
DECLARE_SPELL_FUN(      spell_complete_healing  );
DECLARE_SPELL_FUN(      spell_control_flames    );
DECLARE_SPELL_FUN(      spell_create_sound      );
DECLARE_SPELL_FUN(      spell_death_field       );
DECLARE_SPELL_FUN(      spell_detonate          );
DECLARE_SPELL_FUN(      spell_disintegrate      );
DECLARE_SPELL_FUN(      spell_displacement      );
DECLARE_SPELL_FUN(      spell_domination        );
DECLARE_SPELL_FUN(      spell_ectoplasmic_form  );
DECLARE_SPELL_FUN(      spell_ego_whip          );
DECLARE_SPELL_FUN(      spell_energy_containment);
DECLARE_SPELL_FUN(      spell_enhance_armor     );
DECLARE_SPELL_FUN(      spell_enhanced_strength );
DECLARE_SPELL_FUN(      spell_flesh_armor       );
DECLARE_SPELL_FUN(      spell_inertial_barrier  );
DECLARE_SPELL_FUN(      spell_inflict_pain      );
DECLARE_SPELL_FUN(      spell_intellect_fortress);
DECLARE_SPELL_FUN(      spell_lend_health       );
DECLARE_SPELL_FUN(      spell_levitation        );
DECLARE_SPELL_FUN(      spell_mental_barrier    );
DECLARE_SPELL_FUN(      spell_mind_thrust       );
DECLARE_SPELL_FUN(      spell_project_force     );
DECLARE_SPELL_FUN(      spell_psionic_blast     );
DECLARE_SPELL_FUN(      spell_psychic_crush     );
DECLARE_SPELL_FUN(      spell_psychic_drain     );
DECLARE_SPELL_FUN(      spell_psychic_healing   );
DECLARE_SPELL_FUN(      spell_share_strength    );
DECLARE_SPELL_FUN(      spell_thought_shield    );
DECLARE_SPELL_FUN(      spell_ultrablast        );

/*
 * Spell functions.
 * Defined in magic.c.
 */
DECLARE_SPELL_FUN(	spell_null		);
DECLARE_SPELL_FUN(	spell_notfound		);
DECLARE_SPELL_FUN(	spell_acid_blast	);
DECLARE_SPELL_FUN(	spell_animate_dead	);
DECLARE_SPELL_FUN(	spell_astral_walk	);
DECLARE_SPELL_FUN(	spell_blindness		);
DECLARE_SPELL_FUN(	spell_burning_hands	);
DECLARE_SPELL_FUN(	spell_call_lightning	);
DECLARE_SPELL_FUN(	spell_cause_critical	);
DECLARE_SPELL_FUN(	spell_cause_light	);
DECLARE_SPELL_FUN(	spell_cause_serious	);
DECLARE_SPELL_FUN(	spell_change_sex	);
DECLARE_SPELL_FUN(	spell_charm_person	);
DECLARE_SPELL_FUN(	spell_chill_touch	);
DECLARE_SPELL_FUN(	spell_colour_spray	);
DECLARE_SPELL_FUN(	spell_control_weather	);
DECLARE_SPELL_FUN(	spell_create_food	);
DECLARE_SPELL_FUN(	spell_create_water	);
DECLARE_SPELL_FUN(	spell_cure_blindness	);
DECLARE_SPELL_FUN(	spell_cure_poison	);
DECLARE_SPELL_FUN(	spell_curse		);
DECLARE_SPELL_FUN(	spell_detect_poison	);
DECLARE_SPELL_FUN(	spell_dispel_evil	);
DECLARE_SPELL_FUN(	spell_dispel_magic	);
DECLARE_SPELL_FUN(	spell_dragon_shriek	);
DECLARE_SPELL_FUN(      spell_dream             );
DECLARE_SPELL_FUN(	spell_earthquake	);
DECLARE_SPELL_FUN(	spell_enchant_weapon	);
DECLARE_SPELL_FUN(      spell_enchant_armor     );
DECLARE_SPELL_FUN(	spell_energy_drain	);
DECLARE_SPELL_FUN(	spell_faerie_fire	);
DECLARE_SPELL_FUN(	spell_faerie_fog	);
DECLARE_SPELL_FUN(	spell_farsight		);
DECLARE_SPELL_FUN(	spell_fireball		);
DECLARE_SPELL_FUN(	spell_flamestrike	);
DECLARE_SPELL_FUN(	spell_gate		);
DECLARE_SPELL_FUN(      spell_knock             );
DECLARE_SPELL_FUN(	spell_harm		);
DECLARE_SPELL_FUN(	spell_identify		);
DECLARE_SPELL_FUN(	spell_invis		);
DECLARE_SPELL_FUN(	spell_know_alignment	);
DECLARE_SPELL_FUN(	spell_lightning_bolt	);
DECLARE_SPELL_FUN(	spell_locate_object	);
DECLARE_SPELL_FUN(	spell_magic_missile	);
DECLARE_SPELL_FUN(	spell_merlins_robe	);
DECLARE_SPELL_FUN(	spell_mist_walk		);
DECLARE_SPELL_FUN(	spell_pass_door		);
DECLARE_SPELL_FUN(	spell_plant_pass	);
DECLARE_SPELL_FUN(	spell_poison		);
DECLARE_SPELL_FUN(	spell_polymorph		);
DECLARE_SPELL_FUN(	spell_possess		);
DECLARE_SPELL_FUN(	spell_recharge		);
DECLARE_SPELL_FUN(	spell_ressurection	);
DECLARE_SPELL_FUN(	spell_remove_curse	);
DECLARE_SPELL_FUN(	spell_remove_invis	);
DECLARE_SPELL_FUN(	spell_remove_trap	);
DECLARE_SPELL_FUN(	spell_replenish_blood	);
DECLARE_SPELL_FUN(	spell_shocking_grasp	);
DECLARE_SPELL_FUN(	spell_sleep		);
DECLARE_SPELL_FUN(	spell_smaug		);
DECLARE_SPELL_FUN(	spell_solar_flight	);
DECLARE_SPELL_FUN(	spell_soul_thief	);
DECLARE_SPELL_FUN(	spell_summon		);
DECLARE_SPELL_FUN(	spell_teleport		);
DECLARE_SPELL_FUN(	spell_ventriloquate	);
DECLARE_SPELL_FUN(	spell_weaken		);
DECLARE_SPELL_FUN(	spell_acid_breath	);
DECLARE_SPELL_FUN(	spell_fire_breath	);
DECLARE_SPELL_FUN(	spell_frost_breath	);
DECLARE_SPELL_FUN(	spell_gas_breath	);
DECLARE_SPELL_FUN(	spell_lightning_breath	);
DECLARE_SPELL_FUN(	spell_spiral_blast	);
DECLARE_SPELL_FUN(	spell_scorching_surge	);
DECLARE_SPELL_FUN(	spell_helical_flow	);
DECLARE_SPELL_FUN(      spell_transport      		);
DECLARE_SPELL_FUN(      spell_portal                    );

DECLARE_SPELL_FUN(      spell_ethereal_fist                 );
DECLARE_SPELL_FUN(      spell_spectral_furor                );
DECLARE_SPELL_FUN(      spell_hand_of_chaos                 );
DECLARE_SPELL_FUN(      spell_disruption                    );
DECLARE_SPELL_FUN(      spell_sonic_resonance               );
DECLARE_SPELL_FUN(      spell_mind_wrack                    );
DECLARE_SPELL_FUN(      spell_mind_wrench                   );
DECLARE_SPELL_FUN(      spell_revive                        );
DECLARE_SPELL_FUN(      spell_sulfurous_spray               );
DECLARE_SPELL_FUN(      spell_caustic_fount                 );
DECLARE_SPELL_FUN(      spell_acetum_primus                 );
DECLARE_SPELL_FUN(      spell_galvanic_whip                 );
DECLARE_SPELL_FUN(      spell_magnetic_thrust               );
DECLARE_SPELL_FUN(      spell_quantum_spike                 );
DECLARE_SPELL_FUN(      spell_black_hand                    );
DECLARE_SPELL_FUN(      spell_black_fist                    );
DECLARE_SPELL_FUN(      spell_black_lightning               );
DECLARE_SPELL_FUN(      spell_midas_touch                   );

/*
 * New Spells -- Magic2.c --GW
 */
DECLARE_SPELL_FUN(      spell_create_life               );
DECLARE_SPELL_FUN(	skill_focusing			);
DECLARE_SPELL_FUN(	spell_sphere			);
DECLARE_SPELL_FUN(	spell_psi_blade			);
DECLARE_SPELL_FUN(	spell_mind_bash			);
DECLARE_SPELL_FUN(	spell_warp_reality		);
DECLARE_SPELL_FUN(	spell_ethereal_strike		);
DECLARE_SPELL_FUN(	spell_syphon			);
DECLARE_SPELL_FUN(	spell_molecular_healing		);
DECLARE_SPELL_FUN(	spell_psychic_bonds		);
DECLARE_SPELL_FUN(	spell_willed_strength		);

/*
 * OS-dependent declarations.
 * These are all very standard library functions,
 *   but some systems have incomplete or non-ansi header files.
 */
#if	defined(_AIX)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(apollo)
int	atoi		args( ( const char *string ) );
void *	calloc		args( ( unsigned nelem, size_t size ) );
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(hpux)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(interactive)
#endif

#if	defined(linux)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(MIPS_OS)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(NeXT)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(sequent)
char *	crypt		args( ( const char *key, const char *salt ) );
int	fclose		args( ( FILE *stream ) );
int	fprintf		args( ( FILE *stream, const char *format, ... ) );
int	fread		args( ( void *ptr, int size, int n, FILE *stream ) );
int	fseek		args( ( FILE *stream, long offset, int ptrname ) );
void	perror		args( ( const char *s ) );
int	ungetc		args( ( int c, FILE *stream ) );
#endif

#if	defined(sun)
char *	crypt		args( ( const char *key, const char *salt ) );
int	fclose		args( ( FILE *stream ) );
int	fprintf		args( ( FILE *stream, const char *format, ... ) );
#if 	defined(SYSV)
size_t 	fread		args( ( void *ptr, size_t size, size_t n,
				FILE *stream ) );
#else
/*int	fread		args( ( void *ptr, int size, int n, FILE *stream )
);*/
#endif
int	fseek		args( ( FILE *stream, long offset, int ptrname ) );
void	perror		args( ( const char *s ) );
int	ungetc		args( ( int c, FILE *stream ) );
#endif

#if	defined(ultrix)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

/*
 * The crypt(3) function is not available on some operating systems.
 * In particular, the U.S. Government prohibits its export from the
 *   United States to foreign countries.
 * Turn on NOCRYPT to keep passwords in plain text.
 */
#if	defined(NOCRYPT)
#define crypt(s1, s2)	(s1)
#endif



/*
 * Data files used by the server.
 *
 * AREA_LIST contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 *
 * The NULL_FILE is held open so that we have a stream handle in reserve,
 *   so players can go ahead and telnet to all the other descriptors.
 * Then we close it whenever we need to open a file (e.g. a save file).
 */
#define PLAYER_DIR	"../player/"	/* Player files			*/
#define BACKUP_DIR	"../backup/"    /* Backup Player files       */
#define PLAYER_OBJ_DIR  "../player-obj/"/* Player Objects		*/
#define GOD_DIR		"../gods/"	/* God Info Dir			*/
#define BOARD_DIR	"../boards/"	/* Board data dir		*/
#define CLAN_DIR	"../clans/"	/* Clan data dir		*/
#define PETITION_DIR	"../petitions/"	/* Petition data dir */
#define COUNCIL_DIR  	"../councils/"  /* Council data dir		*/
#define GUILD_DIR       "../guilds/"    /* Guild data dir*/
#define DEITY_DIR	"../deity/"	/* Deity data dir		*/
#define BUILD_DIR       "../building/"  /* Online building save dir*/
#define SYSTEM_DIR	"../system/"	/* Main system files		*/
#define PROG_DIR	"mudprogs/"	/* MUDProg files		*/
#define LOCKER_DIR	"../lockers/"   /* locker dir */
#define FINGER_DIR	"../finger/"    /* finger dir */
#define CORPSE_DIR	"../corpses/"	/* Corpses*/
#define RACE_DIR	"../races/"	/* races */
#define NULL_FILE	"/dev/null"	/* To reserve one stream	*/
#define	CLASS_DIR	"../classes/"	/* Classes */
#define DIARY_FILE	"../system/diary.dat" /*Diary File*/
#define CRASH_FILE	"../system/crashlog.dat" /*crashlog File*/
#define MAIL_FILE	"../system/mail.dat" /*mud-mail file*/
#define AREA_LIST	"area.lst"	/* List of areas */
#define ZONE_LIST	"zone.lst"	/* List of zones */
#define BAN_LIST        "ban.lst"       /* List of bans                 */
#define CLAN_LIST	"clan.lst"	/* List of clans		*/
#define COUNCIL_LIST	"council.lst"	/* List of councils		*/
#define GUILD_LIST      "guild.lst"     /* List of guilds               */
#define GOD_LIST	"gods.lst"	/* List of gods			*/
#define DEITY_LIST	"deity.lst"	/* List of deities		*/
#define	CLASS_LIST	"class.lst"	/* List of classes		*/

#define BOARD_FILE	"boards.txt"		/* For bulletin boards	 */
#define SHUTDOWN_FILE	"shutdown.txt"		/* For 'shutdown'	 */

#define DISABLED_FILE "disabled.txt"  /* disabled commands */
#define RIPSCREEN_FILE	SYSTEM_DIR "mudrip.rip"
#define RIPTITLE_FILE	SYSTEM_DIR "mudtitle.rip"
#define ANSITITLE_FILE	SYSTEM_DIR "mudtitle.ans"
#define ASCTITLE_FILE	SYSTEM_DIR "mudtitle.asc"
#define BOOTLOG_FILE	SYSTEM_DIR "boot.txt"	  /* Boot up error file	 */
#define MUDLOG_FILE	SYSTEM_DIR "mudlog.txt"	  /* Mudlog's            */
#define MEMLOG_FILE	SYSTEM_DIR "memlog.txt"   /* memlog's            */
#define ROOMLOG_FILE	SYSTEM_DIR "roomlog.txt"   /* roomlog's            */
#define BUG_FILE	SYSTEM_DIR "bugs.txt"	  /* For 'bug' and bug( )*/
#define IDEA_FILE	SYSTEM_DIR "ideas.txt"	  /* For 'idea'		 */
#define TYPO_FILE	SYSTEM_DIR "typos.txt"	  /* For 'typo'		 */
#define APPEAL_FILE	SYSTEM_DIR "appeals.txt"  /* for 'appeal'	 */
#define LOG_FILE	SYSTEM_DIR "log.txt"	  /* For talking in logged rooms */
#define WIZLIST_FILE	SYSTEM_DIR "WIZLIST"	  /* Wizlist		 */
#define WHO_FILE	SYSTEM_DIR "WHO"	  /* Who output file	 */
#define WEBWHO_FILE	SYSTEM_DIR "WEBWHO"	  /* WWW Who output file */
#define REQUEST_PIPE	SYSTEM_DIR "REQUESTS"	  /* Request FIFO	 */
#define SKILL_FILE	SYSTEM_DIR "skills.dat"   /* Skill table	 */
#define HERB_FILE	SYSTEM_DIR "herbs.dat"	  /* Herb table		 */
#define SOCIAL_FILE	SYSTEM_DIR "socials.dat"  /* Socials		 */
#define COMMAND_FILE	SYSTEM_DIR "commands.dat" /* Commands		 */
#define USAGE_FILE	SYSTEM_DIR "usage.txt"    /* How many people are on 
 						     every half hour - trying to
						     determine best reboot time */
#define COPYOVER_FILE	SYSTEM_DIR "copyover.dat" /* for warm reboots	 */
#define EXE_FILE	"../bin/Wolfpaw4"	  /* executable path*/
#define CLASSDIR	"../classes/"
#define LAST_COMMAND_FILE "../system/last_command.txt"
#define LAST_FUNCTION_FILE "../system/last_function.dat"
#define RACE_LIST	   RACE_DIR "race.lst"
#define REFF_FILE	 SYSTEM_DIR "refferal.dat"  /* refferals --GW*/
#define WARRENT_FILE     SYSTEM_DIR "warrents.dat" /* Warrents --GW*/
#define DB_FILE		 SYSTEM_DIR "wolfpaw.dat"  /* Db File --GW*/
#define SYSMESSAGE_FILE  SYSTEM_DIR "sysmessage.dat" /* System Messages File --GW */
#define PETITION_LIST	 "petitions.lst"
#define SAVE_ROOM_DIR   "../save-rooms/"
#define SAVE_ROOM_LIST  "save-rooms.lst"

/*
 * Our function prototypes.
 * One big lump ... this is every function in Merc.
 */
#define CD	CHAR_DATA
#define MID	MOB_INDEX_DATA
#define OD	OBJ_DATA
#define OID	OBJ_INDEX_DATA
#define RID	ROOM_INDEX_DATA
#define ZD	ZONE_DATA
#define SF	SPEC_FUN
#define BD	BOARD_DATA
#define CL	CLAN_DATA
#define EDD	EXTRA_DESCR_DATA
#define RD	RESET_DATA
#define ED	EXIT_DATA
#define	ST	SOCIALTYPE
#define	CO	COUNCIL_DATA
#define DE	DEITY_DATA
#define SK	SKILLTYPE

/* websvr.c */
void init_web(int port);
void handle_web(void);
void shutdown_web(void);

/* extras.c */
bool is_ignoring  args ( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void show_mode args( ( DESCRIPTOR_DATA *d ) );
void show_version( CHAR_DATA *ch );
void show_boot_and_compile( CHAR_DATA *ch );
void info ( char *txt );
void load_system_messages( void );

/* alias.c */
void    substitute_alias args( (DESCRIPTOR_DATA *d, char *input) );

/* act_comm.c */
bool	circle_follow	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	add_follower	args( ( CHAR_DATA *ch, CHAR_DATA *master ) );
void	stop_follower	args( ( CHAR_DATA *ch ) );
void	die_follower	args( ( CHAR_DATA *ch ) );
bool	is_same_group	args( ( CHAR_DATA *ach, CHAR_DATA *bch ) );
void	send_rip_screen args( ( CHAR_DATA *ch ) );
void	send_rip_title	args( ( CHAR_DATA *ch ) );
void	send_ansi_title args( ( CHAR_DATA *ch ) );
void	send_ascii_title args( ( CHAR_DATA *ch ) );
void	to_channel	args( ( const char *argument, int channel,
				const char *verb, sh_int level ) );
void  	talk_auction    args( ( char *argument, bool chan_name ) );
bool    knows_language  args( ( CHAR_DATA *ch, int language,
				CHAR_DATA *cch ) );
bool    can_learn_lang  args( ( CHAR_DATA *ch, int language ) );
int     countlangs      args( ( int languages ) );
char *  translate       args( ( CHAR_DATA *ch, CHAR_DATA *victim,
				const char *argument ) );
char *	obj_short	args( ( OBJ_DATA *obj ) );
void record_call	args( ( char * call_name ) );

/* act_info.c */
int	get_door	args( ( char *arg ) );
char *	format_obj_to_char	args( ( OBJ_DATA *obj, CHAR_DATA *ch,
				    bool fShort ) );
void	show_list_to_char	args( ( OBJ_DATA *list, CHAR_DATA *ch,
				    bool fShort, bool fShowNothing ) );

char *GUILDCHECK	args( ( CHAR_DATA *ch ) );
void talk_channel( CHAR_DATA *ch, char *argument, int channel, char *verb );

/* act_move.c */
void	clear_vrooms	args( ( void ) );
ED *	find_door	args( ( CHAR_DATA *ch, char *arg, bool quiet ) );
ED *	get_exit	args( ( ROOM_INDEX_DATA *room, sh_int dir ) );
ED *	get_exit_to	args( ( ROOM_INDEX_DATA *room, sh_int dir, int vnum ) );
ED *	get_exit_num	args( ( ROOM_INDEX_DATA *room, sh_int count ) );
ch_ret	move_char	args( ( CHAR_DATA *ch, EXIT_DATA *pexit, int fall, bool follower_loop ) );
void	teleport	args( ( CHAR_DATA *ch, sh_int room, int flags ) );
sh_int	encumbrance	args( ( CHAR_DATA *ch, sh_int move ) );
bool	will_fall	args( ( CHAR_DATA *ch, int fall ) );

/* act_obj.c */

obj_ret	damage_obj	args( ( OBJ_DATA *obj ) );
sh_int	get_obj_resistance args( ( OBJ_DATA *obj ) );
void    save_clan_storeroom args( ( CHAR_DATA *ch, CLAN_DATA *clan ) );
void    obj_fall  	args( ( OBJ_DATA *obj, bool through ) );

/* act_wiz.c */
RID *	find_location	args( ( CHAR_DATA *ch, char *arg, int zone ) );
void	echo_to_all	args( ( sh_int AT_COLOR, char *argument,sh_int tar ) );
char    *sector_name         args( ( int sector ) );
void   	get_reboot_string args( ( void ) );
struct tm *update_time  args( ( struct tm *old_time ) );
void	free_social	args( ( SOCIALTYPE *social ) );
void	add_social	args( ( SOCIALTYPE *social ) );
void	free_command	args( ( CMDTYPE *command ) );
void	unlink_command	args( ( CMDTYPE *command ) );
void	add_command	args( ( CMDTYPE *command ) );

/* boards.c */
void	load_boards	args( ( void ) );
BD *	get_board	args( ( OBJ_DATA *obj ) );
void	free_note	args( ( NOTE_DATA *pnote ) );

/* build.c */
char *	flag_string	args( ( int bitvector, char * const flagarray[] ) );
int	get_mpflag	args( ( char *flag ) );
int	get_dir		args( ( char *txt  ) );
char *	strip_cr	args( ( char *str  ) );
int     get_langflag( char *flag );
int get_risflag( char *flag );

/* clans.c */
CL *	get_clan	args( ( char *name ) );
void	load_clans	args( ( void ) );
void	save_clan	args( ( CLAN_DATA *clan ) );

CO *	get_council	args( ( char *name ) );
void	load_councils	args( ( void ) );
void 	save_council	args( ( COUNCIL_DATA *council ) );
void    write_clan_list args( ( void ) );

/* new-clans.c */
void clan_buy_list( CHAR_DATA *ch );
void clan_buy( CHAR_DATA *ch, char *argument );

/* deity.c */
DE *	get_deity	args( ( char *name ) );
void	load_deity	args( ( void ) );
void	save_deity	args( ( DEITY_DATA *deity ) );

/* comm.c */
void	close_socket	args( ( DESCRIPTOR_DATA *dclose, bool force ) );
bool	write_to_buffer	args( ( DESCRIPTOR_DATA *d, const char *txt,
				int length ) );
bool	write_to_pager	args( ( DESCRIPTOR_DATA *d, const char *txt,
				int length ) );
bool	send_to_char	args( ( char *txt, CHAR_DATA *ch ) );
bool	send_to_char_color	args( ( char *txt, CHAR_DATA *ch ));
bool	send_to_pager	args( ( char *txt, CHAR_DATA *ch ) );
bool	send_to_pager_color	args( ( char *txt, CHAR_DATA *ch ));
void	set_char_color  args( ( sh_int AType, CHAR_DATA *ch ) );
void	set_pager_color	args( ( sh_int AType, CHAR_DATA *ch ) );
void	ch_printf	args( ( CHAR_DATA *ch, char *fmt, ... ) );
void	pager_printf	args( (CHAR_DATA *ch, char *fmt, ...) );
void	log_printf	args( (char *fmt, ...) );
void	act		args( ( sh_int AType, char *format, CHAR_DATA *ch,
			    const void *arg1, const void *arg2, int type ) );
void	copyover_recover	args( (void) );
char *act_string(const char *format, CHAR_DATA *to, CHAR_DATA *ch,const void *arg1, const void *arg2);

/* reset.c */
RD  *	make_reset	args( ( char letter, int extra, int arg1, int arg2, int arg3 ) );
RD  *	add_reset	args( ( AREA_DATA *tarea, char letter, int extra, int arg1, int arg2, int arg3 ) );
RD  *	place_reset	args( ( AREA_DATA *tarea, char letter, int extra, int arg1, int arg2, int arg3 ) );
void	reset_area	args( ( AREA_DATA * pArea ) );
bool     write_to_descriptor     args( ( int desc, char *txt, int length ) );
void    save_sysdata args( ( SYSTEM_DATA sys ) );

/* db.c */

void  load_disabled   args( ( void ) );
void  save_disabled   args( ( void ) );
void	show_file	args( (
CHAR_DATA *ch, char *filename ) );
char *	str_dup		args( ( char const *str ) );
void	boot_db		args( ( bool fCopyOver ) );
void	area_update	args( ( void ) );
void	add_char	args( ( CHAR_DATA *ch ) );
CD *	create_mobile	args( ( MOB_INDEX_DATA *pMobIndex,ZONE_DATA *zone ) );
OD *	create_object	args( ( OBJ_INDEX_DATA *pObjIndex, int level,ZONE_DATA *zone ) );
void	clear_char	args( ( CHAR_DATA *ch ) );
void	free_char	args( ( CHAR_DATA *ch ) );
char *	get_extra_descr	args( ( const char *name, EXTRA_DESCR_DATA *ed ) );
MID *	get_mob_index	args( ( sh_int vnum, sh_int zone ) );
OID *	get_obj_index	args( ( int vnum, int zone ) );
RID *	get_room_index	args( ( int vnum, int zone ) );
ZD  *   find_zone	args( ( int zone ) );
char	fread_letter	args( ( FILE *fp ) );
long	fread_number	args( ( FILE *fp ) );
char *	fread_string	args( ( FILE *fp ) );
char *	fread_string_nohash args( ( FILE *fp ) );
void	fread_to_eol	args( ( FILE *fp ) );
char *	fread_word	args( ( FILE *fp ) );
char *	fread_line	args( ( FILE *fp ) );
int	number_fuzzy	args( ( int number ) );
long	number_range	args( ( int from, int to ) );
int	number_percent	args( ( void ) );
int	number_door	args( ( void ) );
int	number_bits	args( ( int width ) );
long	number_mm	args( ( void ) );
int	dice		args( ( int number, int size ) );
int	interpolate	args( ( int level, int value_00, int value_32 ) );
void	smash_tilde	args( ( char *str ) );
void	hide_tilde	args( ( char *str ) );
char *	show_tilde	args( ( char *str ) );
bool	str_cmp		args( ( const char *astr, const char *bstr ) );
bool	str_prefix	args( ( const char *astr, const char *bstr ) );
bool	str_infix	args( ( const char *astr, const char *bstr ) );
bool	str_suffix	args( ( const char *astr, const char *bstr ) );
char *	capitalize	args( ( char *str ) );
char *	strlower	args( ( const char *str ) );
char *	strupper	args( ( const char *str ) );
char *  aoran		args( ( const char *str ) );
void	append_file	args( ( CHAR_DATA *ch, char *file, char *str ) );
void	append_to_file	args( ( char *file, char *str ) );
void	bug		args( ( const char *str, ... ) );
void	log_string_plus	args( ( const char *str, sh_int log_type, sh_int level ) );
RID *	make_room	args( ( int vnum, int zone ) );
OID *	make_object	args( ( int vnum, int cvnum, char *name,int zone ));
MID *	make_mobile	args( ( sh_int vnum, sh_int cvnum, char *name, int zone ) );
ED  *	make_exit	args( ( ROOM_INDEX_DATA *pRoomIndex, ROOM_INDEX_DATA *to_room, sh_int door ) );
void	add_help	args( ( HELP_DATA *pHelp ) );
void	fix_area_exits	args( ( AREA_DATA *tarea ) );
void	load_area_file	args( ( AREA_DATA *tarea, char *filename ) );
void	randomize_exits	args( ( ROOM_INDEX_DATA *room, sh_int maxdir ) );
void	make_wizlist	args( ( void ) );
void	tail_chain	args( ( void ) );
bool    delete_room     args( ( ROOM_INDEX_DATA *room, int zone ) );
bool    delete_obj      args( ( OBJ_INDEX_DATA *obj ) );
bool    delete_mob      args( ( MOB_INDEX_DATA *mob ) );
/* Functions to add to sorting lists. -- Altrag */
/*void	mob_sort	args( ( MOB_INDEX_DATA *pMob ) );
void	obj_sort	args( ( OBJ_INDEX_DATA *pObj ) );
void	room_sort	args( ( ROOM_INDEX_DATA *pRoom ) );*/
void	sort_area	args( ( AREA_DATA *pArea, bool proto ) );

/* build.c */
void	start_editing	args( ( CHAR_DATA *ch, char *data ) );
void	stop_editing	args( ( CHAR_DATA *ch ) );
void	edit_buffer	args( ( CHAR_DATA *ch, char *argument ) );
char *	copy_buffer	args( ( CHAR_DATA *ch ) );
bool	can_rmodify	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *room ) );
bool	can_omodify	args( ( CHAR_DATA *ch, OBJ_DATA *obj  ) );
bool	can_mmodify	args( ( CHAR_DATA *ch, CHAR_DATA *mob ) );
bool	can_medit	args( ( CHAR_DATA *ch, MOB_INDEX_DATA *mob ) );
void	free_reset	args( ( AREA_DATA *are, RESET_DATA *res ) );
void	free_area	args( ( AREA_DATA *are ) );
void	assign_area	args( ( CHAR_DATA *ch ) );
EDD *	SetRExtra	args( ( ROOM_INDEX_DATA *room, char *keywords ) );
bool	DelRExtra	args( ( ROOM_INDEX_DATA *room, char *keywords ) );
EDD *	SetOExtra	args( ( OBJ_DATA *obj, char *keywords ) );
bool	DelOExtra	args( ( OBJ_DATA *obj, char *keywords ) );
EDD *	SetOExtraProto	args( ( OBJ_INDEX_DATA *obj, char *keywords ) );
bool	DelOExtraProto	args( ( OBJ_INDEX_DATA *obj, char *keywords ) );
void	fold_area	args( ( AREA_DATA *tarea, char *filename, bool install ) );
int	get_otype	args( ( char *type ) );
int	get_atype	args( ( char *type ) );
int	get_aflag	args( ( char *flag ) );
int	get_oflag	args( ( char *flag ) );
int	get_wflag	args( ( char *flag ) );

/* fight.c */
int	max_fight	args( ( CHAR_DATA *ch ) );
void	violence_update	args( ( void ) );
ch_ret	multi_hit	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
sh_int	ris_damage	args( ( CHAR_DATA *ch, long dam, int ris ) );
ch_ret	damage		args( ( CHAR_DATA *ch, CHAR_DATA *victim, long dam,
			    int dt ) );
void	update_pos	args( ( CHAR_DATA *victim ) );
void	set_fighting	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	stop_fighting	args( ( CHAR_DATA *ch, bool fBoth ) );
void	free_fight	args( ( CHAR_DATA *ch ) );
CD *	who_fighting	args( ( CHAR_DATA *ch ) );
void	check_killer	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	check_attacker	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	death_cry	args( ( CHAR_DATA *ch ) );
void	stop_hunting	args( ( CHAR_DATA *ch ) );
void	stop_hating	args( ( CHAR_DATA *ch ) );
void	stop_fearing	args( ( CHAR_DATA *ch ) );
void	start_hunting	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	start_hating	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	start_fearing	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	is_hunting	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	is_hating	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	is_fearing	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	is_safe		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	legal_loot	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
sh_int	VAMP_AC		args( ( CHAR_DATA *ch ) );
bool    check_illegal_pk args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    raw_kill        args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );   
bool	in_arena	args( ( CHAR_DATA *ch ) );
bool    fight_in_progress; /* for pk arena scheduling */
int     align_compute   args( ( CHAR_DATA *gch, CHAR_DATA *victim ) );
ch_ret projectile_hit( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield,
                       OBJ_DATA *projectile, sh_int dist );

/* makeobjs.c */
void	make_corpse	args( ( CHAR_DATA *ch, CHAR_DATA *killer, bool DT ) );
void	make_blood	args( ( CHAR_DATA *ch ) );
void	make_bloodstain args( ( CHAR_DATA *ch ) );
void	make_scraps	args( ( OBJ_DATA *obj ) );
void	make_fire	args( ( ROOM_INDEX_DATA *in_room, sh_int timer) );
OD *	make_trap	args( ( int v0, int v1, int v2, int v3 ) );
OD *	create_money	args( ( int amount, int type ) );

/* misc.c */
void actiondesc args( ( CHAR_DATA *ch, OBJ_DATA *obj, void *vo ) );

/* deity.c */
void adjust_favor	args( ( CHAR_DATA *ch, int field, int mod ) );

/* mud_comm.c */
char *	mprog_type_to_name	args( ( int type ) );

/* mud_prog.c */
#ifdef DUNNO_STRSTR
char *  strstr                  args ( (const char *s1, const char *s2 ) );
#endif

void	mprog_wordlist_check    args ( ( char * arg, CHAR_DATA *mob,
                			CHAR_DATA* actor, OBJ_DATA* object,
					void* vo, int type ) );
void	mprog_percent_check     args ( ( CHAR_DATA *mob, CHAR_DATA* actor,
					OBJ_DATA* object, void* vo,
					int type ) );
void	mprog_act_trigger       args ( ( char* buf, CHAR_DATA* mob,
		                        CHAR_DATA* ch, OBJ_DATA* obj,
					void* vo ) );
void	mprog_bribe_trigger     args ( ( CHAR_DATA* mob, CHAR_DATA* ch,
		                        int amount ) );
void	mprog_entry_trigger     args ( ( CHAR_DATA* mob ) );
void	mprog_give_trigger      args ( ( CHAR_DATA* mob, CHAR_DATA* ch,
                		        OBJ_DATA* obj ) );
void	mprog_greet_trigger     args ( ( CHAR_DATA* mob ) );
void    mprog_fight_trigger     args ( ( CHAR_DATA* mob, CHAR_DATA* ch ) );
void    mprog_hitprcnt_trigger  args ( ( CHAR_DATA* mob, CHAR_DATA* ch ) );
void    mprog_death_trigger     args ( ( CHAR_DATA *killer, CHAR_DATA* mob ) );
void    mprog_random_trigger    args ( ( CHAR_DATA* mob ) );
void    mprog_speech_trigger    args ( ( char* txt, CHAR_DATA* mob ) );
void    mprog_script_trigger    args ( ( CHAR_DATA *mob ) );
void    mprog_hour_trigger      args ( ( CHAR_DATA *mob ) );
void    mprog_time_trigger      args ( ( CHAR_DATA *mob ) );
void    progbug                 args( ( char *str, CHAR_DATA *mob ) );
void	rset_supermob		args( ( ROOM_INDEX_DATA *room) );
void	release_supermob	args( ( ) );

/* player.c */
void	set_title	args( ( CHAR_DATA *ch, char *title ) );
int GET_HITROLL( CHAR_DATA *ch );
int GET_DAMROLL( CHAR_DATA *ch );
int GET_AC( CHAR_DATA *ch );
bool check_new_char_pass( char *argument );
int GET_EQAC( CHAR_DATA *ch );
int GET_MAC( CHAR_DATA *ch );

/* skills.c */
bool	check_skill		args( ( CHAR_DATA *ch, char *command, char *argument ) );
void	learn_from_success	args( ( CHAR_DATA *ch, int sn ) );
void	learn_from_failure	args( ( CHAR_DATA *ch, int sn ) );
bool	check_parry		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_dodge		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool 	check_grip		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	disarm			args( ( CHAR_DATA *ch, CHAR_DATA *victim) );
void	trip			args( ( CHAR_DATA *ch, CHAR_DATA *victim) );


/* handler.c */
int	get_exp		args( ( CHAR_DATA *ch ) );
int	get_exp_worth	args( ( CHAR_DATA *ch ) );
int	exp_level	args( ( CHAR_DATA *ch, sh_int level ) );
int     exp_level2	args( ( CHAR_DATA *ch, sh_int level ) );
int	get_bloodthirst args( ( CHAR_DATA *ch ) );
sh_int	get_trust	args( ( CHAR_DATA *ch ) );
sh_int	get_age		args( ( CHAR_DATA *ch ) );
sh_int	get_curr_str	args( ( CHAR_DATA *ch ) );
sh_int	get_curr_int	args( ( CHAR_DATA *ch ) );
sh_int	get_curr_wis	args( ( CHAR_DATA *ch ) );
sh_int	get_curr_dex	args( ( CHAR_DATA *ch ) );
sh_int	get_curr_con	args( ( CHAR_DATA *ch ) );
sh_int	get_curr_cha	args( ( CHAR_DATA *ch ) );
sh_int  get_curr_lck	args( ( CHAR_DATA *ch ) );
bool	can_take_proto	args( ( CHAR_DATA *ch ) );
int	can_carry_n	args( ( CHAR_DATA *ch ) );
int	can_carry_w	args( ( CHAR_DATA *ch ) );
bool	is_name		args( ( const char *str, char *namelist ) );
bool	is_name_prefix	args( ( const char *str, char *namelist ) );
bool	nifty_is_name	args( ( char *str, char *namelist ) );
bool	nifty_is_name_prefix args( ( char *str, char *namelist ) );
void	affect_modify	args( ( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd ) );
void	affect_to_char	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_remove	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_strip	args( ( CHAR_DATA *ch, int sn ) );
bool	is_affected	args( ( CHAR_DATA *ch, int sn ) );
void	affect_join	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	char_from_room_code args( ( CHAR_DATA *ch ) );
void	char_to_room_code args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex ) );
OD *	obj_to_char	args( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void	obj_from_char	args( ( OBJ_DATA *obj ) );
int	apply_ac	args( ( OBJ_DATA *obj, int iWear ) );
OD *	get_eq_char	args( ( CHAR_DATA *ch, int iWear ) );
void	equip_char	args( ( CHAR_DATA *ch, OBJ_DATA *obj, int iWear ) );
void	unequip_char	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
int	count_obj_list	args( ( OBJ_INDEX_DATA *obj, OBJ_DATA *list ) );
void	obj_from_room	args( ( OBJ_DATA *obj ) );
OD *	obj_to_room	args( ( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex ) );
OD *	obj_to_obj	args( ( OBJ_DATA *obj, OBJ_DATA *obj_to ) );
void	obj_from_obj	args( ( OBJ_DATA *obj ) );
void	extract_obj	args( ( OBJ_DATA *obj ) );
void	extract_exit	args( ( ROOM_INDEX_DATA *room, EXIT_DATA *pexit ) );
void	extract_room	args( ( ROOM_INDEX_DATA *room ) );
void	clean_room	args( ( ROOM_INDEX_DATA *room ) );
void	clean_obj	args( ( OBJ_INDEX_DATA *obj ) );
void	clean_mob	args( ( MOB_INDEX_DATA *mob ) );
void	clean_resets	args( ( AREA_DATA *tarea ) );
void	extract_char	args( ( CHAR_DATA *ch, bool fPull ) );
CD *	get_char_room	args( ( CHAR_DATA *ch, char *argument ) );
CD *	get_char_world	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_type	args( ( OBJ_INDEX_DATA *pObjIndexData ) );
OD *	get_obj_list	args( ( CHAR_DATA *ch, char *argument,
			    OBJ_DATA *list ) );
OD *	get_obj_list_rev args( ( CHAR_DATA *ch, char *argument,
			    OBJ_DATA *list ) );
OD *	get_obj_carry	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_wear	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_here	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_world	args( ( CHAR_DATA *ch, char *argument ) );
int	get_obj_number	args( ( OBJ_DATA *obj ) );
int	get_obj_weight	args( ( OBJ_DATA *obj ) );
bool	room_is_dark	args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool	room_is_private	args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool	can_see		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	can_see_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
bool	can_drop_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
char *	item_type_name	args( ( OBJ_DATA *obj ) );
char *	affect_loc_name	args( ( int location ) );
char *	affect_bit_name	args( ( EXT_BV *vector ) );
char *	extra_bit_name	args( ( int extra_flags ) );
char *	extra_bit_name2	args( ( int extra_flags ) );
char *	magic_bit_name	args( ( int magic_flags ) );
ch_ret	check_for_trap	args( ( CHAR_DATA *ch, OBJ_DATA *obj, int flag ) );
ch_ret	check_room_for_traps args( ( CHAR_DATA *ch, int flag ) );
bool	is_trapped	args( ( OBJ_DATA *obj ) );
OD *	get_trap	args( ( OBJ_DATA *obj ) );
ch_ret	spring_trap     args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
void	name_stamp_stats args( ( CHAR_DATA *ch ) );
void	fix_char	args( ( CHAR_DATA *ch ) );
void	showaffect	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	set_cur_obj	args( ( OBJ_DATA *obj ) );
bool	obj_extracted	args( ( OBJ_DATA *obj ) );
void	queue_extracted_obj	args( ( OBJ_DATA *obj ) );
void	clean_obj_queue	args( ( void ) );
void	set_cur_char	args( ( CHAR_DATA *ch ) );
bool	char_died	args( ( CHAR_DATA *ch ) );
void	queue_extracted_char	args( ( CHAR_DATA *ch, bool extract ) );
void	clean_char_queue	args( ( void ) );
void	add_timer	args( ( CHAR_DATA *ch, sh_int type, sh_int count, DO_FUN *fun, int value ) );
TIMER * get_timerptr	args( ( CHAR_DATA *ch, sh_int type ) );
sh_int	get_timer	args( ( CHAR_DATA *ch, sh_int type ) );
void	extract_timer	args( ( CHAR_DATA *ch, TIMER *timer ) );
void	remove_timer	args( ( CHAR_DATA *ch, sh_int type ) );
bool	in_soft_range	args( ( CHAR_DATA *ch, AREA_DATA *tarea ) );
bool	in_hard_range	args( ( CHAR_DATA *ch, AREA_DATA *tarea ) );
bool	chance  	args( ( CHAR_DATA *ch, sh_int percent ) );
bool 	chance_attrib	args( ( CHAR_DATA *ch, sh_int percent, sh_int attrib ) );
OD *	clone_object	args( ( OBJ_DATA *obj ) );
void	split_obj	args( ( OBJ_DATA *obj, int num ) );
void	separate_obj	args( ( OBJ_DATA *obj ) );
bool	empty_obj	args( ( OBJ_DATA *obj, OBJ_DATA *destobj,
				ROOM_INDEX_DATA *destroom ) );
OD *	find_obj	args( ( CHAR_DATA *ch, char *argument,
				bool carryonly ) );
bool	ms_find_obj	args( ( CHAR_DATA *ch ) );
void	worsen_mental_state args( ( CHAR_DATA *ch, int mod ) );
void	better_mental_state args( ( CHAR_DATA *ch, int mod ) );
void	boost_economy	args( ( AREA_DATA *tarea, int gold ) );
void	lower_economy	args( ( AREA_DATA *tarea, int gold ) );
void	economize_mobgold args( ( CHAR_DATA *mob ) );
bool	economy_has	args( ( AREA_DATA *tarea, int gold ) );
void	add_kill	args( ( CHAR_DATA *ch, CHAR_DATA *mob ) );
int	times_killed	args( ( CHAR_DATA *ch, CHAR_DATA *mob ) );
void    area_reset_check args( ( ROOM_INDEX_DATA *room ) );
char 	*num_comma ( long value );
long get_power( CHAR_DATA *ch );
bool    remove_obj      args( ( CHAR_DATA *ch, int iWear, bool fReplace ));
bool	mudarg_scan	args( ( char arg ) );

/* interp.c */
bool	check_pos	args( ( CHAR_DATA *ch, sh_int position ) );
void	interpret	args( ( CHAR_DATA *ch, char *argument ) );
bool	is_number	args( ( char *arg ) );
int	number_argument	args( ( char *argument, char *arg ) );
char *	one_argument	args( ( char *argument, char *arg_first ) );
char *	one_argument2	args( ( char *argument, char *arg_first ) );
ST *	find_social	args( ( CHAR_DATA *ch, char *command, bool personal ) );
CMDTYPE *find_command	args( ( char *command ) );
void	hash_commands	args( ( ) );
void	start_timer	args( ( struct timeval *stime ) );
time_t	end_timer	args( ( struct timeval *stime ) );
void	send_timer	args( ( struct timerset *vtime, CHAR_DATA *ch ) );
void	update_userec	args( ( struct timeval *time_used,
				struct timerset *userec ) );

/* magic.c */
bool	process_spell_components args( ( CHAR_DATA *ch, int sn ) );
int	ch_slookup	args( ( CHAR_DATA *ch, const char *name ) );
int	find_spell	args( ( CHAR_DATA *ch, const char *name, bool know ) );
int	find_skill	args( ( CHAR_DATA *ch, const char *name, bool know ) );
int	find_weapon	args( ( CHAR_DATA *ch, const char *name, bool know ) );
int	find_tongue	args( ( CHAR_DATA *ch, const char *name, bool know ) );
int	skill_lookup	args( ( const char *name ) );
int	herb_lookup	args( ( const char *name ) );
int	personal_lookup	args( ( CHAR_DATA *ch, const char *name ) );
int	slot_lookup	args( ( int slot ) );
int	bsearch_skill	args( ( const char *name, int first, int top ) );
int	bsearch_skill_exact args( ( const char *name, int first, int top ) );
bool	saves_poison_death	args( ( int level, CHAR_DATA *victim ) );
bool	saves_wand		args( ( int level, CHAR_DATA *victim ) );
bool	saves_para_petri	args( ( int level, CHAR_DATA *victim ) );
bool	saves_breath		args( ( int level, CHAR_DATA *victim ) );
bool	saves_spell_staff	args( ( int level, CHAR_DATA *victim ) );
ch_ret	obj_cast_spell	args( ( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj ) );
int	dice_parse	args( (CHAR_DATA *ch, int level, char *exp) );
SK *	get_skilltype	args( ( int sn ) );

/* request.c */
void	init_request_pipe	args( ( void ) );
void	check_requests		args( ( void ) );

/* save.c */
/* object saving defines for fread/write_obj. -- Altrag */
#define OS_CARRY	0
#define OS_CORPSE	1
#define OS_LOCKER	2 /*GW -- lockers */
#define OS_SAVEROOM     3 /* Saverooms --GW */
#define OS_PCO		4 /* pstat obj --GW */

void	save_char_obj	args( ( CHAR_DATA *ch ) );
bool	load_char_obj	args( ( DESCRIPTOR_DATA *d, char *name, bool preload ) );
void	set_alarm	args( ( long seconds ) );
void	requip_char	args( ( CHAR_DATA *ch ) );
void    fwrite_obj      args( ( CHAR_DATA *ch,  OBJ_DATA  *obj, FILE *fp, 
				int iNest, sh_int os_type, sh_int no_limit ) );
void	fread_obj	args( ( CHAR_DATA *ch,  FILE *fp, sh_int os_type ) );
void	de_equip_char	args( ( CHAR_DATA *ch ) );
void	re_equip_char	args( ( CHAR_DATA *ch ) );
void    save_finger     args( ( CHAR_DATA *ch ) );
void    fwrite_finger   args( ( CHAR_DATA *ch, FILE *fp ) );
void    read_finger     args( ( CHAR_DATA *ch, char *argument ) );
void    fread_finger    args( ( CHAR_DATA *ch, FILE *fp ) );

/* shops.c */

/* special.c */
SF *	spec_lookup	args( ( const char *name ) );
char *	lookup_spec	args( ( SPEC_FUN *special ) );

/* tables.c */
int	get_skill	args( ( char *skilltype ) );
char *	spell_name	args( ( SPELL_FUN *spell ) );
char *	skill_name	args( ( DO_FUN *skill ) );
void	load_skill_table args( ( void ) );
void	save_skill_table args( ( void ) );
void	sort_skill_table args( ( void ) );
void	load_socials	args( ( void ) );
void	save_socials	args( ( void ) );
void	load_commands	args( ( void ) );
void	save_commands	args( ( void ) );
SPELL_FUN *spell_function args( ( char *name ) );
DO_FUN *skill_function  args( ( char *name ) );
void	write_class_file args( ( int cl ) );
void	save_classes	args( ( void ) );
void	load_classes	args( ( void ) );
void	load_herb_table	args( ( void ) );
void	save_herb_table	args( ( void ) );

/* track.c */
void	found_prey	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	hunt_victim	args( ( CHAR_DATA *ch) );

/* update.c */
void	advance_level	args( ( CHAR_DATA *ch, bool SHOW, int class ) );
void	advance_level2	args( ( CHAR_DATA *ch, bool SHOW ) );
void	gain_exp	args( ( CHAR_DATA *ch, int gain, bool fGain ) );
void	gain_condition	args( ( CHAR_DATA *ch, int iCond, int value ) );
void	update_handler	args( ( void ) );
void	reboot_check	args( ( time_t reset ) );
#if 0
void    reboot_check    args( ( char *arg ) );
#endif
void    auction_update  args( ( void ) );
void	remove_portal	args( ( OBJ_DATA *portal ) );
void    dragon_update( void );
void lose_level( CHAR_DATA *ch, int lvl, int classnum );

/* hashstr.c */
char *	str_alloc	args( ( char *str ) );
char *	quick_link	args( ( char *str ) );
int	str_free	args( ( char *str ) );
void	show_hash	args( ( int count ) );
char *	hash_stats	args( ( void ) );
char *	check_hash	args( ( char *str ) );
void	hash_dump	args( ( int hash ) );
void	show_high_hash	args( ( int top ) );

/* newscore.c */
char *  get_class 	args( (CHAR_DATA *ch) );
char *  get_race 	args( (CHAR_DATA *ch) );

/* laws.c */
bool arrest 		args( (CHAR_DATA *ch) );
void check_looter	args( (CHAR_DATA *ch, OBJ_DATA *corpse ) );
void check_lockerlooter args( (CHAR_DATA *ch, OBJ_DATA *locker ) );
void arrest_char	args( (CHAR_DATA *victim) );
void destroy_warrent    args( (WARRENT_DATA *warrent) );
void create_warrent     args( (CHAR_DATA *ch, CHAR_DATA *victim, char *offense) );
void read_warrent	args( ( WARRENT_DATA *warrent, FILE *fp ) );  
void load_warrents	args( ( void ) );
void write_warrents     args( ( void ) );
bool check_warrents( CHAR_DATA *ch );

/* extras.c */
void write_memlog_line	args( ( char *log ) );
void roomlog		args( ( char *log, ... ) );

/* ext_bv.c */
EXT_BV fread_bitvector( FILE *fp );
EXT_BV  meb             args( ( int bit ) );
EXT_BV  multimeb        args( ( int bit, ... ) );
char *print_bitvector( EXT_BV *bits );

/* color-mapping.c */
char *colormap_parse( CHAR_DATA *ch, int map_type, bool advanced, char *speaker, char *string );
void create_map(CHAR_DATA *ch,int map_type,int color_num,bool string,char *txt);
void free_char_colormaps( CHAR_DATA *ch );   
void load_char_cmap( CHAR_DATA *ch, char *argument );
void save_char_cmaps( FILE *fp, CHAR_DATA *ch );
int get_map_type( char *name, bool immortal );
int get_color_num( char *color );
bool colorstringcheck( int map_type );    
void setup_char_color_maps( CHAR_DATA *ch );
void set_cmap_bv( CHAR_DATA *ch, int map_type );
void remove_cmap_bv( CHAR_DATA *ch, int map_type );
void delete_char_color_map( CHAR_DATA *ch, int map_type );
bool colormap_check( CHAR_DATA *ch, int map_type );
bool advanced_map_name_check( char *argument );
char *advanced_map_convert( char *argument );
int get_cmap_bv_by_type( int map_type );
CMAP_DATA *find_char_map_by_type(CHAR_DATA *ch, int map_type);
char *find_real_color_string(CMAP_DATA *map);
int get_channel_cmap_num(int chan);
CHAR_DATA *get_char args( ( char *name ) );
int get_real_act_color_num(int orig);
char *get_map_name_by_num( int num );
char *get_color_word_by_num( int num );

/* Save-rooms.c */
void load_save_rooms( void );
void load_save_room( char *filename );
void save_save_room( CHAR_DATA *ch );
void clean_save_room( ROOM_INDEX_DATA *room );

/* hoarder.c */
void hoard_handler(CHAR_DATA *ch, OBJ_DATA *obj, int type );
void hoarder_check(CHAR_DATA *ch, OBJ_DATA *obj, int type );
int get_num_objs_char( CHAR_DATA *ch );

/* aliens.c */
bool dt_scan( CHAR_DATA *ch );
void char_alien_update( CHAR_DATA *ch );
void aliens_update(void);

/* act_obj2.c */
bool anti_class_check( CHAR_DATA *ch, OBJ_DATA *obj );
bool get_life_protection_char( CHAR_DATA *ch );

/* player-kill.c */
void display_pkill_chart( CHAR_DATA *ch );
void clan_pkill_record_setup( void );
bool check_pk_ok( CHAR_DATA *ch, CHAR_DATA *killer );
void update_clan_pk_data( CLAN_DATA *killer_clan, CLAN_DATA *vict_clan );
void fwrite_pkill_data( FILE *fp, CLAN_DATA *cln );
void fread_pkill_data( FILE *fp, CLAN_DATA *cln );

/* limits.c */
void load_limit_db( void );
void write_limit_db( void );
bool obj_limit_check( OBJ_INDEX_DATA *obj );
void load_limited_toggle( bool on_off  );
void adjust_limits( OBJ_INDEX_DATA *obj, int mod, int type );
LIMIT_DATA *limit_lookup( int zone, int vnum );
void set_limits( void );
void create_limit_entry( OBJ_DATA *obj );
void delete_limit_entry( OBJ_DATA *obj );

/* wp-olc.c */
void gredit_engine args( ( DESCRIPTOR_DATA *d, char *argument ) );

/* arena.c */
void write_to_gallery( CHAR_DATA *fighter, char *message );
void stop_viewers( void );
void pk_champ_check( CHAR_DATA *ch, CHAR_DATA *victim );

/* hosts.c */
bool check_hosts( CHAR_DATA *ch, char *site );
void free_hosts( CHAR_DATA *ch );
void host_setup( CHAR_DATA *ch );
void load_host( CHAR_DATA *ch, char *line );
void save_hosts( CHAR_DATA *ch, FILE *fp );

/* bank.c */
int get_total_levels( CHAR_DATA *ch );
void setup_shares( void );
int add_share_ac( CHAR_DATA *ch );
int add_share_mac( CHAR_DATA *ch );
int add_share_hitndam( CHAR_DATA *ch );

/* db2.c */
void load_main_db( void );
void load_max_skills( FILE *fp );
void load_limit_list( FILE *fp );
AREA_DATA *find_area( int vnum, sh_int type );
void reset_copyover_boot_time( void );
OFFLINE_DATA *load_offline_data_file( FILE *fp );
void free_offline_data( CHAR_DATA *ch );
void object_limit_update( void );
LIMIT_DATA *limit_lookup( int zone, int vnum );
void reinit_limits( void );
bool save_offline_data( CHAR_DATA *ch );
void load_char_offline_data( CHAR_DATA *ch );

/* topten.c */
void show_topten( CHAR_DATA *ch, sh_int type );
char *get_topten_type_name( sh_int type );
TOPTEN_DATA *on_topten_list( TOPTEN_DATA *top, char *name, bool AC, long value );
TOPTEN_DATA * list_resort( TOPTEN_DATA *top, bool AC );
void topten_setup(void);
void topten_check( CHAR_DATA *ch );
void fwrite_topten( void );
int get_top_number( TOPTEN_DATA *top );
int get_low_number( TOPTEN_DATA *top );

/* personal-socials.c */
void free_char_psocials(CHAR_DATA *ch);
void free_socialtype_data( SOCIALTYPE *soc );
void save_char_socials( FILE *fp, CHAR_DATA *ch );
void load_char_social( FILE *fp, CHAR_DATA *ch );
void setup_char_socials(CHAR_DATA *ch);

/* smart-mobs.c */
void smart_update args( ( CHAR_DATA *mob, CHAR_DATA *fighter ) );

/* Death.c */
void grim_reaper( CHAR_DATA *victim, bool olddeath );

/* plog.c */
void plog_check( CHAR_DATA *ch, char *string );

/* race.c */
void write_races( void );

/* quest-master.c */
void quest_update        args(( void ));

/* quest.c */
int      number_kombat           args( ( void ) );

/* casino.c */
void setup_casino( void );
int do_blackjack_enter(CHAR_DATA *ch);
int do_blackjack_exit(CHAR_DATA *ch);

#undef	SK
#undef	CO
#undef	ST
#undef	CD
#undef	MID
#undef	OD
#undef	OID
#undef	RID
#undef	SF
#undef	BD 
#undef	CL
#undef	EDD
#undef	RD
#undef	ED

/*
 *
 *  New Build Interface Stuff Follows
 *
 */


/*
 *  Data for a menu page
 */
struct	menu_data
{
    char		*sectionNum;
    char		*charChoice;
    int			x;
    int			y;
    char		*outFormat;
    void		*data;
    int			ptrType;
    int			cmdArgs;
    char		*cmdString;
};

DECLARE_DO_FUN( do_redraw_page  );
DECLARE_DO_FUN( do_refresh_page );
DECLARE_DO_FUN( do_pagelen	);
DECLARE_DO_FUN( do_omenu  	);
DECLARE_DO_FUN( do_rmenu  	);
DECLARE_DO_FUN( do_mmenu  	);
DECLARE_DO_FUN( do_clear  	);

extern		MENU_DATA		room_page_a_data[];
extern		MENU_DATA		room_page_b_data[];
extern		MENU_DATA		room_page_c_data[];
extern		MENU_DATA		room_help_page_data[];

extern		MENU_DATA		mob_page_a_data[];
extern		MENU_DATA		mob_page_b_data[];
extern		MENU_DATA		mob_page_c_data[];
extern		MENU_DATA		mob_page_d_data[];
extern		MENU_DATA		mob_page_e_data[];
extern		MENU_DATA		mob_page_f_data[];
extern		MENU_DATA		mob_help_page_data[];

extern		MENU_DATA		obj_page_a_data[];
extern		MENU_DATA		obj_page_b_data[];
extern		MENU_DATA		obj_page_c_data[];
extern		MENU_DATA		obj_page_d_data[];
extern		MENU_DATA		obj_page_e_data[];
extern		MENU_DATA		obj_help_page_data[];

extern		MENU_DATA		control_page_a_data[];
extern		MENU_DATA		control_help_page_data[];

extern	const   char    room_page_a[];
extern	const   char    room_page_b[];
extern	const   char    room_page_c[];
extern	const   char    room_help_page[];

extern	const   char    obj_page_a[];
extern	const   char    obj_page_b[];
extern	const   char    obj_page_c[];
extern	const   char    obj_page_d[];
extern	const   char    obj_page_e[];
extern	const   char    obj_help_page[];

extern	const   char    mob_page_a[];
extern	const   char    mob_page_b[];
extern	const   char    mob_page_c[];
extern	const   char    mob_page_d[];
extern	const   char    mob_page_e[];
extern	const   char    mob_page_f[];
extern	const   char    mob_help_page[];
extern	const   char *  npc_sex[3];
extern	const   char *  ris_strings[];

extern	const   char    control_page_a[];
extern	const   char    control_help_page[];

#define SH_INT 1
#define INT 2
#define CHAR 3
#define STRING 4
#define SPECIAL 5


#define NO_PAGE    0
#define MOB_PAGE_A 1
#define MOB_PAGE_B 2
#define MOB_PAGE_C 3
#define MOB_PAGE_D 4
#define MOB_PAGE_E 5
#define MOB_PAGE_F 17
#define MOB_HELP_PAGE 14
#define ROOM_PAGE_A 6
#define ROOM_PAGE_B 7
#define ROOM_PAGE_C 8
#define ROOM_HELP_PAGE 15
#define OBJ_PAGE_A 9
#define OBJ_PAGE_B 10
#define OBJ_PAGE_C 11
#define OBJ_PAGE_D 12
#define OBJ_PAGE_E 13
#define OBJ_HELP_PAGE 16
#define CONTROL_PAGE_A 18
#define CONTROL_HELP_PAGE 19

#define NO_TYPE   0
#define MOB_TYPE  1
#define OBJ_TYPE  2
#define ROOM_TYPE 3
#define CONTROL_TYPE 4

#define SUB_NORTH DIR_NORTH
#define SUB_EAST  DIR_EAST
#define SUB_SOUTH DIR_SOUTH
#define SUB_WEST  DIR_WEST
#define SUB_UP    DIR_UP
#define SUB_DOWN  DIR_DOWN
#define SUB_NE    DIR_NORTHEAST
#define SUB_NW    DIR_NORTHWEST
#define SUB_SE    DIR_SOUTHEAST
#define SUB_SW    DIR_SOUTHWEST


/*
 * defines for use with this get_affect function
 */

#define RIS_000		BV00
#define RIS_R00		BV01
#define RIS_0I0		BV02
#define RIS_RI0		BV03
#define RIS_00S		BV04
#define RIS_R0S		BV05
#define RIS_0IS		BV06
#define RIS_RIS		BV07

#define GA_AFFECTED	BV09
#define GA_RESISTANT	BV10
#define GA_IMMUNE	BV11
#define GA_SUSCEPTIBLE	BV12
#define GA_RIS          BV30



/*
 *   Map Structures
 */

DECLARE_DO_FUN( do_mapout 	);
DECLARE_DO_FUN( do_lookmap	);

struct  map_data	/* contains per-room data */
{
  int vnum;		/* which map this room belongs to */
  int x;		/* horizontal coordinate */
  int y;		/* vertical coordinate */
  char entry;		/* code that shows up on map */ 
};


struct  map_index_data
{
  MAP_INDEX_DATA  *next;
  int 		  vnum;  		  /* vnum of the map */
  int             map_of_vnums[49][81];   /* room vnums aranged as a map*/
};


MAP_INDEX_DATA *get_map_index(int vnum);
void            init_maps();


/*
 * mudprograms stuff
 */
extern	CHAR_DATA *supermob;

void oprog_speech_trigger( char *txt, CHAR_DATA *ch );
void oprog_random_trigger( OBJ_DATA *obj );
void oprog_wear_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
bool oprog_use_trigger( CHAR_DATA *ch, OBJ_DATA *obj, 
                        CHAR_DATA *vict, OBJ_DATA *targ, void *vo );
void oprog_remove_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_sac_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_damage_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_repair_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_drop_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_zap_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
char *oprog_type_to_name( int type );

/*
 * MUD_PROGS START HERE
 * (object stuff)
 */
void oprog_greet_trigger( CHAR_DATA *ch );
void oprog_speech_trigger( char *txt, CHAR_DATA *ch );
void oprog_random_trigger( OBJ_DATA *obj );
void oprog_random_trigger( OBJ_DATA *obj );
void oprog_remove_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_sac_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_get_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_damage_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_repair_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_drop_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_examine_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_zap_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_pull_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_push_trigger( CHAR_DATA *ch, OBJ_DATA *obj );


/* mud prog defines */

#define ERROR_PROG        -1
#define IN_FILE_PROG       0
#define ACT_PROG           BV00
#define SPEECH_PROG        BV01
#define RAND_PROG          BV02
#define FIGHT_PROG         BV03
#define RFIGHT_PROG        BV03
#define DEATH_PROG         BV04
#define RDEATH_PROG        BV04
#define HITPRCNT_PROG      BV05
#define ENTRY_PROG         BV06
#define ENTER_PROG         BV06
#define GREET_PROG         BV07
#define RGREET_PROG	   BV07
#define OGREET_PROG        BV07
#define ALL_GREET_PROG	   BV08
#define GIVE_PROG	   BV09
#define BRIBE_PROG	   BV10
#define HOUR_PROG	   BV11
#define TIME_PROG	   BV12
#define WEAR_PROG          BV13  
#define REMOVE_PROG        BV14  
#define SAC_PROG           BV15  
#define LOOK_PROG          BV16  
#define EXA_PROG           BV17  
#define ZAP_PROG           BV18  
#define GET_PROG 	   BV19  
#define DROP_PROG	   BV20  
#define DAMAGE_PROG	   BV21  
#define REPAIR_PROG	   BV22  
#define RANDIW_PROG	   BV23  
#define SPEECHIW_PROG	   BV24  
#define PULL_PROG	   BV25  
#define PUSH_PROG	   BV26  
#define SLEEP_PROG         BV27  
#define REST_PROG          BV28  
#define LEAVE_PROG         BV29
#define SCRIPT_PROG	   BV30
#define USE_PROG           BV31

void rprog_leave_trigger( CHAR_DATA *ch );
void rprog_enter_trigger( CHAR_DATA *ch );
void rprog_sleep_trigger( CHAR_DATA *ch );
void rprog_rest_trigger( CHAR_DATA *ch );
void rprog_rfight_trigger( CHAR_DATA *ch );
void rprog_death_trigger( CHAR_DATA *killer, CHAR_DATA *ch );
void rprog_speech_trigger( char *txt, CHAR_DATA *ch );
void rprog_random_trigger( CHAR_DATA *ch );
void rprog_time_trigger( CHAR_DATA *ch );
void rprog_hour_trigger( CHAR_DATA *ch );
char *rprog_type_to_name( int type );

#define OPROG_ACT_TRIGGER
#ifdef OPROG_ACT_TRIGGER
void oprog_act_trigger( char *buf, OBJ_DATA *mobj, CHAR_DATA *ch,
			OBJ_DATA *obj, void *vo );
#endif
#define RPROG_ACT_TRIGGER
#ifdef RPROG_ACT_TRIGGER
void rprog_act_trigger( char *buf, ROOM_INDEX_DATA *room, CHAR_DATA *ch,
			OBJ_DATA *obj, void *vo );
#endif


#define GET_ADEPT(ch,sn)    (  skill_table[(sn)]->skill_adept[(ch)->class])
#define LEARNED(ch,sn)      (IS_NPC(ch) ? 80 : URANGE(0,ch->pcdata->learned[sn], 101 ) )

/* CTF */
extern char *CTFLOWSTR;
extern char *CTFHIGHSTR;
extern bool CTF_ACCEPTING;
extern bool QUAD_CTF_HIGH;
extern bool ADV_CTF_HIGH;
extern bool DUAL_CTF_HIGH;
extern bool QUAD_CTF_LOW;
extern bool ADV_CTF_LOW;
extern bool DUAL_CTF_LOW;
extern bool CTF_ACTIVE;
extern int  CTF_LOW_LEV;
extern int  CTF_HIGH_LEV;

/*
 * Security Stuff --GW
 */

#define IS_HIMP(ch)	    ( HIMP_CHECK(ch) == TRUE )

#define IS_IMP(ch)	    ( IMP_CHECK(ch) == TRUE )

#define IS_OWNER(ch)	    ( OWNER_CHECK(ch) == TRUE )

#define IS_BPORT_IMP(ch)    ( BPORT_IMP_CHECK(ch) == TRUE )

#define IS_BPORT_HIMP(ch)   ( BPORT_HIMP_CHECK(ch) == TRUE )

bool HIMP_CHECK(CHAR_DATA *ch);
bool IMP_CHECK(CHAR_DATA *ch);
bool OWNER_CHECK(CHAR_DATA *ch);
bool BPORT_IMP_CHECK(CHAR_DATA *ch);
bool BPORT_HIMP_CHECK(CHAR_DATA *ch);

#define SECURE_TYPE_HIMP		1
#define SECURE_TYPE_IMP			2
#define SECURE_TYPE_OWNER		3
#define SECURE_TYPE_BPORT_IMP		4
#define SECURE_TYPE_BPORT_HIMP		5

struct secure_data 
{
SECURE_DATA	*next;
SECURE_DATA	*prev;
int		type;
SECURE_NAME	*first_name;
SECURE_NAME	*last_name;
};

struct secure_name
{
SECURE_NAME 	*next;
SECURE_NAME 	*prev;
char *name;
};

extern	SECURE_DATA 	*first_secure;
extern	SECURE_DATA 	*last_secure;

/* Files */
#define SECURE_DIR		"../secure/"
#define HIMP_FILE		"HIMP"
#define IMP_FILE		"IMP"
#define OWNER_FILE		"OWNER"
#define BPORT_IMP_FILE		"BPORT_IMP"
#define BPORT_HIMP_FILE		"BPORT_HIMP"

/* End Security Stuff */

/* Maps... */
#define MAP_DIR 		"../maps/"


