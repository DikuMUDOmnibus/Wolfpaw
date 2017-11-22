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
 *			     Mud constants module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include "mud.h"

/* undef these at EOF */
#define AM 95
#define AC 95
#define AT 85
#define AW 85
#define AV 95
#define AD 95
#define AR 90
#define AA 95

/*
 * Race table. -- NO LONGER USED
 */
    /*  race name	   DEF_AFF      st dx ws in cn ch lk hp mn re su 	RESTRICTION                                                                         LANGUAGE */
/*
const	struct	race_type	race_table	[MAX_RACE]	=
{
    {
	"Human",		0,	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0,					    LANG_COMMON
    },

    {
	"Elf",	 AFF_DETECT_MAGIC,     -1, 2, 0, 1,-1, 1, 0, -2,10, 0, 0,
	0,						    LANG_ELVEN
    },

    {
	"Dwarf",     AFF_INFRARED,	1, 0, 1, 0, 2,-1, 0, 6,-6, 0, 0,
	(1 << CLASS_MAGE),								    LANG_DWARVEN
    },

    {
	"Halfling",     AFF_SNEAK,     -2, 1, 0, 0, 1, 0, 0, -3,10, 0, 0,
	(1 << CLASS_MAGE) | (1 << CLASS_DRUID) | (1 << CLASS_CLERIC) | (1 << CLASS_AUGURER),			    LANG_HALFLING
    },

    {
	"Pixie",       AFF_FLYING,     -4, 3, 0,-1,-2, 3, 0, -5,40, 0, 0,	
					(1 << CLASS_VAMPIRE)
					| (1 << CLASS_WARRIOR)
					| (1 << CLASS_AUGURER),
					LANG_PIXIE
    },

    {
	"_Vampire_",   AFF_INFRARED,	1, 0, 0, 0, 2,-2, 0,	0,
                                      
                                        LANG_COMMON

    },

    {
	"Half-Ogre",   AFF_INFRARED,    2,-3,-1,-2, 3,-4, 0, 5,-8, 0, 0,	
					(1 << CLASS_MAGE)
					| (1 << CLASS_CLERIC)
					| (1 << CLASS_DRUID)
					| (1 << CLASS_AUGURER) 
					| (1 << CLASS_RANGER) , LANG_OGRE
    },


    {
	"Half-Orc",    AFF_INFRARED,    1,-2,-2,-1, 2,-4, 0, 6,-9, 0, 0,
		  			(1 << CLASS_MAGE)
		  			| (1 << CLASS_CLERIC) 
		  			| (1 << CLASS_DRUID)
					| (1 << CLASS_AUGURER) 
		  			| (1 << CLASS_RANGER) , LANG_ORCISH
    },

    {
	"Half-Troll",  AFF_INFRARED,    3,-2,-2, 1, 2,-5, 0, 7,-9, 0, 0,
					(1 << CLASS_MAGE)
					| (1 << CLASS_CLERIC)
					| (1 << CLASS_DRUID)
					| (1 << CLASS_RANGER),
					LANG_TROLLISH
    },

    {
	"Half-Elf",    AFF_INFRARED,   -1, 1, 0, 1,-1, 1, 0, 3, 3, 0, 0, 
					(1 << CLASS_AUGURER),
					LANG_ELVEN
    },
    {
	"Gith",        AFF_DETECT_INVIS,    1, 2,-1, 1,-2,-5, 1, 4,20, 0, 0,
					(1 << CLASS_DRUID)
					| (1 << CLASS_RANGER),
					LANG_GITH
    },
    {
        "Werewolf",     AFF_INFRARED | AFF_HIDE | AFF_DETECT_HIDDEN,
					   1,2,-2,-1,1,0,0,0,0,0,0,
			 			LANG_WOLFISH
    },
    {
        "Draconian",     AFF_INFRARED | AFF_DETECT_INVIS,
					1,-1,-2,-1,2,0,0,0,0,0,0,    
			   LANG_DRACONIAN
    },

    {
        "Satyr",     AFF_HIDE,   0,0,2,0,0,-3,0,0,0,0,0,
			           
			             LANG_SATYR
    },
    {
        "Wraith",     AFF_PASS_DOOR |  AFF_INVISIBLE,
				   -1,2,0,1,-1,0,0,0,0,0,0,
			  
			 		     LANG_WRAITH
    },

    {
        "Centaur",     AFF_INFRARED,    2,0,-1,0,-1,0,0,0,0,0,0,
			            
			          LANG_CENTAUR
    },
    {
        "Drider",     AFF_INFRARED,    0,1,0,1,0,0,-2,0,0,0,0,0,
			            
				     LANG_DRIDER
    },

    {
        "Drow",     AFF_INFRARED |    0,2,1,1,0,-2,0,0,0,0,0,
		    AFF_FAERIE_FIRE ,   
			 		  LANG_DROWISH
    },
    {
        "Minotaur",     0,    3,0,0,-2,0,0,0,0,0,0,0,
			                 
			                 LANG_MINOTAUR
    },
    {
        "Cyclops",     0,     2,0,0,-1,0,-2,0,0,0,0,0,
			               
			 		  LANG_CYCLOPS
    },

    {
        "Arewyndel",     AFF_FLYING |    -2,1,1,1,1,-2,0,0,0,0,0,
			 AFF_DETECT_MAGIC ,  	
					LANG_AREWYNDEL
    },
    {
        "Brownie",     AFF_FLYING,    -2,0,1,2,-1,0,0,0,0,0,0,
			           
			            LANG_BROWNIE
    },
    {
        "Leprachaun",     AFF_FLYING,    -2,0,1,2,0,-1,3,0,0,0,0,
			           
			            LANG_LEPRACHAUN
    },
    {
        "Mrrshan",     AFF_FLYING,    1,3,-1,0,0,-2,0,0,0,0,0,
			           
			            LANG_MRRSHAN
    }

};
*/

char *	const	npc_race	[MAX_NPC_RACE] =
{
"human", "elf", "dwarf", "halfling", "pixie", "vampire", "half-ogre",
"half-orc", "half-troll", "half-elf", "gith", "werewolf", "draconian",
"satyr", "wraith", "centaur","drider", "drow", "minotaur", "cyclops",
"arewyndel","brownie","leprachaun","mrrshan","ant", "ape", "baboon",
"bat","bear", "bee",
"beetle", "boar", "bugbear", "cat", "dog", "dragon", "ferret", "fly",
"gargoyle", "gelatin", "ghoul", "gnoll", "gnome", "goblin", "golem",
"gorgon", "harpy", "hobgoblin", "kobold", "lizardman", "locust",
"lycanthrope", "minotaur", "mold", "mule", "neanderthal", "ooze", "orc",
"rat", "rustmonster", "shadow", "shapeshifter", "shrew", "shrieker",
"skeleton", "slime", "snake", "spider", "stirge", "thoul", "troglodyte",
"undead", "wight", "wolf", "worm", "zombie", "bovine", "canine", "feline",
"porcine", "mammal", "rodent", "avis", "reptile", "amphibian", "fish",
"crustacean", "insect", "spirit", "magical", "horse", "animal", "humanoid",
"monster", "god", "dragon2",
};


char *	const	npc_class	[MAX_NPC_CLASS] =
{
"mage", "cleric", "thief", "warrior", "vampire", "druid", "ranger",
"augurer", "paladin", "assassin", "werewolf", "kinju", "avatar",
"psionicist","dragon", "blademaster","archmage", "bishop", "alchemist",
"prophet","crusader","samurai","wraith","hunter","adept","ninja",
"jackel", "mystic","baker", "butcher", "blacksmith", "mayor", "king",
"queen"
};

char *  const   curse_table [] =
{
"fuck", "shit", "bullshit", "bitch", "ass", "damn", "goddamn",
"slut", "pussy", "cunt", "dick", "dickhead"
};

char *  const   censor_table [] =
{
"screw", "turd", "bullpucky", "dog", "buttocks", "darn", "goshdarn",
"lady", "kitten", "peepee", "weewee", "weeweehead"
};
  

/*
 * Attribute bonus tables.
 */
const	struct	str_app_type	str_app		[26]		=
{
    { -5, -4,   0,  0 },  /* 0  */
    { -5, -4,   3,  1 },  /* 1  */
    { -3, -2,   3,  2 },
    { -3, -1,  10,  3 },  /* 3  */
    { -2, -1,  25,  4 },
    { -2, -1,  55,  5 },  /* 5  */
    { -1,  0,  80,  6 },
    { -1,  0,  90,  7 },
    {  0,  0, 100,  8 },
    {  0,  0, 100,  9 },
    {  0,  0, 115, 10 }, /* 10  */
    {  0,  0, 115, 11 },
    {  0,  0, 140, 12 },
    {  0,  0, 140, 13 }, /* 13  */
    {  0,  1, 170, 14 },
    {  1,  1, 170, 15 }, /* 15  */
    {  1,  2, 195, 16 },
    {  2,  3, 220, 22 },
    {  2,  4, 250, 25 }, /* 18  */
    {  3,  5, 400, 30 },
    {  3,  6, 500, 35 }, /* 20  */
    {  4,  7, 600, 40 },
    {  5,  7, 700, 45 },
    {  6,  8, 800, 50 },
    {  8, 10, 900, 55 },
    { 10, 12, 999, 60 }  /* 25   */
};



const	struct	int_app_type	int_app		[26]		=
{
    {  3 },	/*  0 */
    {  5 },	/*  1 */
    {  7 },
    {  8 },	/*  3 */
    {  9 },
    { 10 },	/*  5 */
    { 11 },
    { 12 },
    { 13 },
    { 15 },
    { 17 },	/* 10 */
    { 19 },
    { 22 },
    { 25 },
    { 28 },
    { 31 },	/* 15 */
    { 34 },
    { 37 },
    { 40 },	/* 18 */
    { 44 },
    { 49 },	/* 20 */
    { 55 },
    { 60 },
    { 70 },
    { 85 },
    { 99 }	/* 25 */
};



const	struct	wis_app_type	wis_app		[26]		=
{
    { 0 },	/*  0 */
    { 0 },	/*  1 */
    { 0 },
    { 0 },	/*  3 */
    { 0 },
    { 1 },	/*  5 */
    { 1 },
    { 1 },
    { 1 },
    { 2 },
    { 2 },	/* 10 */
    { 2 },
    { 2 },
    { 2 },
    { 2 },
    { 3 },	/* 15 */
    { 3 },
    { 4 },
    { 5 },	/* 18 */
    { 5 },
    { 5 },	/* 20 */
    { 6 },
    { 6 },
    { 6 },
    { 6 },
    { 7 }	/* 25 */
};



const	struct	dex_app_type	dex_app		[26]		=
{
    {   60 },   /* 0 */
    {   50 },   /* 1 */
    {   50 },
    {   40 },
    {   30 },
    {   20 },   /* 5 */
    {   10 },
    {    0 },
    {    0 },
    {    0 },
    {    0 },   /* 10 */
    {    0 },
    {    0 },
    {    0 },
    {    0 },
    { - 10 },   /* 15 */
    { - 15 },
    { - 20 },
    { - 30 },
    { - 40 },
    { - 50 },   /* 20 */
    { - 60 },
    { - 75 },
    { - 90 },
    { -105 },
    { -120 }    /* 25 */
};



const	struct	con_app_type	con_app		[26]		=
{
    { -4, 20 },   /*  0 */
    { -3, 25 },   /*  1 */
    { -2, 30 },
    { -2, 35 },	  /*  3 */
    { -1, 40 },
    { -1, 45 },   /*  5 */
    { -1, 50 },
    {  0, 55 },
    {  0, 60 },
    {  0, 65 },
    {  0, 70 },   /* 10 */
    {  0, 75 },
    {  0, 80 },
    {  0, 85 },
    {  0, 88 },
    {  1, 90 },   /* 15 */
    {  2, 95 },
    {  2, 97 },
    {  3, 99 },   /* 18 */
    {  3, 99 },
    {  4, 99 },   /* 20 */
    {  4, 99 },
    {  5, 99 },
    {  6, 99 },
    {  7, 99 },
    {  8, 99 }    /* 25 */
};


const	struct	cha_app_type	cha_app		[26]		=
{
    { - 60 },   /* 0 */
    { - 50 },   /* 1 */
    { - 50 },
    { - 40 },
    { - 30 },
    { - 20 },   /* 5 */
    { - 10 },
    { -  5 },
    { -  1 },
    {    0 },
    {    0 },   /* 10 */
    {    0 },
    {    0 },
    {    0 },
    {    1 },
    {    5 },   /* 15 */
    {   10 },
    {   20 },
    {   30 },
    {   40 },
    {   50 },   /* 20 */
    {   60 },
    {   70 },
    {   80 },
    {   90 },
    {   99 }    /* 25 */
};

/* Have to fix this up - not exactly sure how it works (Scryn) */
const	struct	lck_app_type	lck_app		[26]		=
{
    {   60 },   /* 0 */
    {   50 },   /* 1 */
    {   50 },
    {   40 },
    {   30 },
    {   20 },   /* 5 */
    {   10 },
    {    0 },
    {    0 },
    {    0 },
    {    0 },   /* 10 */
    {    0 },
    {    0 },
    {    0 },
    {    0 },
    { - 10 },   /* 15 */
    { - 15 },
    { - 20 },
    { - 30 },
    { - 40 },
    { - 50 },   /* 20 */
    { - 60 },
    { - 75 },
    { - 90 },
    { -105 },
    { -120 }    /* 25 */
};


/*
 * Liquid properties.
 * Used in #OBJECT section of area file.
 */
const	struct	liq_type	liq_table	[LIQ_MAX]	=
{
    /* name			color     drunk/full/thirst/hp/mana/move*/
    { "water",			"clear",	{  0, 1, 10, 0, 0, 0 }	},/*  0 */
    { "beer",			"amber",	{  3, 2,  5, 0, 0, 0 }	},
    { "wine",			"rose",		{  5, 2,  5, 0, 0, 0 }	},
    { "ale",			"brown",	{  2, 2,  5, 0, 0, 0 }	},
    { "dark ale",		"dark",		{  1, 2,  5, 0, 0, 0 }	},

    { "whisky",			"golden",	{  6, 1,  4, 0, 0, 0 }	},/*  5 */
    { "lemonade",		"pink",		{  0, 1,  8, 0, 0, 0 }	},
    { "firebreather",		"boiling",	{ 10, 0,  0, 0, 0, 0 }	},
    { "local specialty",	"everclear",	{  3, 3,  3, 0, 0, 0 }	},
    { "slime mold juice",	"green",	{  0, 4, -8, 0, 0, 0 }	},

    { "milk",			"white",	{  0, 3,  6, 0, 0, 0 }	},/* 10 */
    { "tea",			"tan",		{  0, 1,  6, 0, 0, 0 }	},
    { "coffee",			"black",	{  0, 1,  6, 0, 0, 0 }	},
    { "blood",			"red",		{  0, 2, -1, 0, 0, 0 }	},
    { "salt water",		"clear",	{  0, 1, -2, 0, 0, 0 }	},

    { "cola",			"cherry",	{  0, 1,  5, 0 ,0 ,0 }	},/* 15 */
    { "mead",			"honey color",	{  4, 2,  5, 0, 0, 0 }	},/* 16 */
    { "grog",			"thick brown",	{  3, 2,  5, 0, 0, 0 }	},/* 17 */
    { "chocolate milk",         "brown",	{  0, 1,  5, 50, 100, 10 } },
    { "jolt cola",		"brown",	{  5, 1,  5, 100, 100, 100 } }
};

char *	const	attack_table	[18] =
{
    "hit",
    "slice",  "stab",  "slash", "whip", "claw",
    "blast",  "pound", "crush", "grep", "bite",
    "pierce", "suction", "bolt", "arrow", "dart",
    "stone", "pea"
};



/*
 * The skill and spell table.
 * Slot numbers must never be changed as they appear in #OBJECTS sections.
 */
#define SLOT(n)	n
#define LI LEVEL_IMMORTAL

#undef AM 
#undef AC 
#undef AT 
#undef AW 
#undef AV 
#undef AD 
#undef AR
#undef AA

#undef LI
