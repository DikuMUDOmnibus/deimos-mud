/* ************************************************************************
*   File: config.c                                      Part of CircleMUD *
*  Usage: Configuration of various aspects of CircleMUD operation         *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#define __CONFIG_C__

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "interpreter.h"	/* alias_data definition for structs.h */

#define TRUE	1
#define YES  	1
#define FALSE	0
#define NO	  0

/*
 * Below are several constants which you can change to alter certain aspects
 * of the way CircleMUD acts.  Since this is a .c file, all you have to do
 * to change one of the constants (assuming you keep your object files around)
 * is change the constant in this file and type 'make'.  Make will recompile
 * this file and relink; you don't have to wait for the whole thing to
 * recompile as you do if you change a header file.
 *
 * I realize that it would be slightly more efficient to have lots of
 * #defines strewn about, so that, for example, the autowiz code isn't
 * compiled at all if you don't want to use autowiz.  However, the actual
 * code for the various options is quite small, as is the computational time
 * in checking the option you've selected at run-time, so I've decided the
 * convenience of having all your options in this one file outweighs the
 * efficency of doing it the other way.
 *
 */

/****************************************************************************/
/****************************************************************************/


/* GAME PLAY OPTIONS */

/*
 * pk_allowed sets the tone of the entire game.  If pk_allowed is set to
 * NO, then players will not be allowed to kill, summon, charm, or sleep
 * other players, as well as a variety of other "asshole player" protections.
 * However, if you decide you want to have an all-out knock-down drag-out
 * PK Mud, just set pk_allowed to YES - and anything goes.
 */
int pk_allowed = NO;

/* our global variables */
int freeze_game_running = 0;
int freeze_game_timer = 0;
int freeze_game_red = 0;
int freeze_game_blue = 0;
int freeze_game_iblue = 0;
int freeze_game_ired = 0;
int freeze_game_redpnt = 0;
int freeze_game_bluepnt = 0;
room_vnum red_room = 0;
room_vnum blue_room = 0;


/* is playerthieving allowed? */
int pt_allowed = NO;

/* minimum level a player must be to shout/holler/gossip/auction */
int level_can_shout = 1;

/* number of movement points it costs to holler */
int holler_move_cost = 20;

/* exp change limits */
int max_exp_gain = 10000000;	/* max gainable per kill */
int max_exp_loss = 500000000;	/* max losable per death */

/* number of tics (usually 75 seconds) before PC/NPC corpses decompose */
int max_npc_corpse_time = 5;
int max_pc_corpse_time = 100000;

/* How many ticks before a player is sent to the void or idle-rented. */
int idle_void = 10;
int idle_rent_time = 48;

/* This level and up is immune to idling, LVL_IMPL+1 will disable it. */
int idle_max_level = LVL_IMMORT;

/* should items in death traps automatically be junked? */
int dts_are_dumps = NO;

/*
 * Whether you want items that immortals load to appear on the ground or not.
 * It is most likely best to set this to 'YES' so that something else doesn't
 * grab the item before the immortal does, but that also means people will be
 * able to carry around things like boards.  That's not necessarily a bad
 * thing, but this will be left at a default of 'NO' for historic reasons.
 */
int load_into_inventory = NO;

/* "okay" etc. */
const char *OK = "&WOkay.&n\r\n";
const char *NOPERSON = "&RNo-one by that name here.&n\r\n";
const char *NOEFFECT = "&RNothing seems to happen.&n\r\n";

/*
 * You can define or not define TRACK_THOUGH_DOORS, depending on whether
 * or not you want track to find paths which lead through closed or
 * hidden doors. A setting of 'NO' means to not go through the doors
 * while 'YES' will pass through doors to find the target.
 */
int track_through_doors = YES;

/****************************************************************************/
/****************************************************************************/


/* RENT/CRASHSAVE OPTIONS */

/*
 * Should the MUD allow you to 'rent' for free?  (i.e. if you just quit,
 * your objects are saved at no cost, as in Merc-type MUDs.)
 */
int free_rent = YES;

/* maximum number of items players are allowed to rent */
int max_obj_save = 9999;

/* receptionist's surcharge on top of item costs */
int min_rent_cost = 0;

/*
 * Should the game automatically save people?  (i.e., save player data
 * every 4 kills (on average), and Crash-save as defined below.  This
 * option has an added meaning past bpl13.  If auto_save is YES, then
 * the 'save' command will be disabled to prevent item duplication via
 * game crashes.
 */
int auto_save = YES;

/*
 * if auto_save (above) is yes, how often (in minutes) should the MUD
 * Crash-save people's objects?   Also, this number indicates how often
 * the MUD will Crash-save players' houses.
 */
int autosave_time = 1;

/* Lifetime of crashfiles and forced-rent (idlesave) files in days */
int crash_file_timeout = 365;

/* Lifetime of normal rent files in days */
int rent_file_timeout = 365;

/* Do you want to automatically wipe players who've been gone too long? */
int auto_pwipe = NO;

/* Autowipe deletion criteria
   This struct holds information used to determine which players to wipe
   then the mud boots.  The levels must be in ascending order, with a
   descending level marking the end of the array.  A level -1 entry in the
   beginning is the case for players with the PLR_DELETED flag.  The
   values below match the stock purgeplay.c criteria.

   Detailed explanation by array element:
   * Element 0, level -1, days 0: Players with PLR_DELETED flag are always
       wiped.
   * Element 1, level 0, days 0: Players at level 0 have created a   
       character, but have never actually entered the game, so always
       wipe them.
   * Element 2, level 1, days 4: Players at level 1 are wiped if they
       haven't logged on in the past 4 days.
   * Element 3, level 4, days 7: Players level 2 through 4 are wiped if
       they haven't logged on in the past 7 days.
   * Element 4, level 10, days 30: Players level 5-10 get 30 days.
   * Element 5, level LVL_IMMORT - 1, days 60: All other mortals get
       60 days.
   * Element 6, level LVL_IMPL, days 90: Immortals get 90 days.
   * Element 7: Because -2 is less than LVL_IMPL, this is assumed to
       be the end of the criteria.  The days entry is not used in this
       case.
*/

struct pclean_criteria_data pclean_criteria[] = {
/*     LEVEL           DAYS    */
  {    -1              ,0      }, /* players with PLR_DELETE flag */
  {    0               ,500      }, /* level 0 */
  {    15              ,100      },
  {    30              ,400     },
  {    45              ,600     },      
  {    LVL_IMMORT - 1  ,800    }, /* highest mortal */
  {    LVL_IMPL        ,1000     }, /* all immortals */
  {    -2              ,0      }  /* no more level checks */
};

/* Do you want to save backups of players and their objects and mail when
   they are deleted by the pfile autocleaner?
*/

int backup_wiped_pfiles = NO;

/* Do you want players who self-delete to be wiped immediately with no
   backup?
*/
int selfdelete_fastwipe = YES;

/* Minimum level for pfile backup to occur when wiping players.
   Note:  If you set selfdelete_fastwipe to NO, you'll need this to
   prevent your backup directory from being filled with level 0 and
   1 deletions.
*/
int pfile_backup_minlevel = 5;                    
                                                  
/****************************************************************************/
/****************************************************************************/


/* ROOM NUMBERS */

/* virtual number of room that mortals should enter at */
room_vnum mortal_start_room = 200;

/* virtual number of room that immorts should enter at by default */
room_vnum immort_start_room = 1204;


/* virtual number of room that frozen players should enter at */
room_vnum frozen_start_room = 1202;

/*
 * virtual numbers of donation rooms.  note: you must change code in
 * do_drop of act.item.c if you change the number of non-NOWHERE
 * donation rooms.
 */
room_vnum donation_room_1 = 270;
room_vnum donation_room_2 = NOWHERE;	/* unused - room for expansion */
room_vnum donation_room_3 = NOWHERE;	/* unused - room for expansion */


/****************************************************************************/
/****************************************************************************/


/* GAME OPERATION OPTIONS */

/*
 * This is the default port on which the game should run if no port is
 * given on the command-line.  NOTE WELL: If you're using the
 * 'autorun' script, the port number there will override this setting.
 * Change the PORT= line in autorun instead of (or in addition to)
 * changing this.
 */
ush_int DFLT_PORT = 6666;

/*
 * IP address to which the MUD should bind.  This is only useful if
 * you're running Circle on a host that host more than one IP interface,
 * and you only want to bind to *one* of them instead of all of them.
 * Setting this to NULL (the default) causes Circle to bind to all
 * interfaces on the host.  Otherwise, specify a numeric IP address in
 * dotted quad format, and Circle will only bind to that IP address.  (Of
 * course, that IP address must be one of your host's interfaces, or it
 * won't work.)
 */
const char *DFLT_IP = NULL; /* bind to all interfaces */
/* const char *DFLT_IP = "192.168.1.1";  -- bind only to one interface */

/* default directory to use as data directory */
const char *DFLT_DIR = "lib";

/*
 * What file to log messages to (ex: "log/syslog").  Setting this to NULL
 * means you want to log to stderr, which was the default in earlier
 * versions of Circle.  If you specify a file, you don't get messages to
 * the screen. (Hint: Try 'tail -f' if you have a UNIX machine.)
 */
const char *LOGNAME = NULL;
/* const char *LOGNAME = "log/syslog";  -- useful for Windows users */

/* maximum number of players allowed before game starts to turn people away */
int max_playing = 1000;

/* maximum size of bug, typo and idea files in bytes (to prevent bombing) */
int max_filesize = 50000;

/* maximum number of password attempts before disconnection */
int max_bad_pws = 4;

/*
 * Rationale for enabling this, as explained by naved@bird.taponline.com.
 *
 * Usually, when you select ban a site, it is because one or two people are
 * causing troubles while there are still many people from that site who you
 * want to still log on.  Right now if I want to add a new select ban, I need
 * to first add the ban, then SITEOK all the players from that site except for
 * the one or two who I don't want logging on.  Wouldn't it be more convenient
 * to just have to remove the SITEOK flags from those people I want to ban
 * rather than what is currently done?
 */
int siteok_everyone = TRUE;

/*
 * Some nameservers are very slow and cause the game to lag terribly every 
 * time someone logs in.  The lag is caused by the gethostbyaddr() function
 * which is responsible for resolving numeric IP addresses to alphabetic names.
 * Sometimes, nameservers can be so slow that the incredible lag caused by
 * gethostbyaddr() isn't worth the luxury of having names instead of numbers
 * for players' sitenames.
 *
 * If your nameserver is fast, set the variable below to NO.  If your
 * nameserver is slow, of it you would simply prefer to have numbers
 * instead of names for some other reason, set the variable to YES.
 *
 * You can experiment with the setting of nameserver_is_slow on-line using
 * the SLOWNS command from within the MUD.
 */

int nameserver_is_slow = NO;

const char *QANSI = "Press Enter to Continue";

const char *MENU =
"\r\n"
"&rWelcome to DeimosMUD!&n\r\n"
"&Y0)&n &BExit from DeimosMUD.&n\r\n"
"&Y1)&n &BEnter DeimosMUD.&n\r\n"
"&Y2)&n &BEnter description.&n\r\n"
"&Y3)&n &BRead the background story.&n\r\n"
"&Y4)&n &BChange password.&n\r\n"
"&Y5)&n &BDelete this character.&n\r\n"
"\r\n"
"   &WMake your choice:&n ";



const char *GREETINGS =
"&R .______\r\n" 
"  `-.--.\\\r\n"
"   /||__||  ___  <> ___  __   ___   __,   \r\n"
"&R  _|||__|| || \\\\ /\\ |.-\\/-.| || \\\\ //\r\n"
"&R ' ||/__|| ||--' || || || || || || \\--\\   \r\n"
"&R  _/----'  \\\\__. |/ \\/ \\/ |/ \\\\_|| __//\r\n"
"&W          __      __   __     __   ________     &Y\\\r\n"
"&W         |   \\  /   | |  |   |  | |        \\    &Y/|\r\n"
"&C      ___&W|    \\/    |&C_&W|  |&C___&W|  |&C_&W|  .--.   |&C__&YO /      _\r\n"
"&C _,-``   &W|          | |  |   |  | |  |   |  |  &Y\\ |&R`````&Y| \\\r\n"
"&C  `-,,___&W|  |\\  /|  |&C_&W|  |&C___&W|  |&C_&W|  |&C___&W|  |&C__&Y/ |&R,,,,,&Y|_/\r\n"
"&W         |  | \\/ |  | |   '-'   | |  '--'   |  &YO \\\r\n"
"&W         |__|    |__|  \\_______/  |________/    &Y\\|\r\n"
"&Y                                               &Y/\r\n"
"&D                                               &n\r\n"
"&D                                               -ascii by mobius-&n\r\n"
"\r\n"
"                        Based on CircleMUD 3.0,\r\n"
"                       Created by Jeremy Elson\r\n"
"\r\n"
"                  A derivative of DikuMUD (GAMMA 0.0),\r\n"
"            created by Hans-Henrik Staerfeldt, Katja Nyboe,\r\n"
"           Tom Madsen, Michael Seifert, and Sebastian Hammer\r\n"
"\r\n\r\n\r\n\r\n"
"Enter Thy Name: ";

const char *WELC_MESSG =
"\r\n"
"&WWelcome to DeimosMUD&n"
"\r\n\r\n";

const char *START_MESSG =
"Welcome.  This is your new DeimosMUD character!  You can now earn gold,\r\n"
"gain experience, find weapons and equipment, and much more -- while\r\n"
"meeting people from around the world!\r\n";

/****************************************************************************/
/****************************************************************************/


/* AUTOWIZ OPTIONS */

/*
 * Should the game automatically create a new wizlist/immlist every time
 * someone immorts, or is promoted to a higher (or lower) god level?
 * NOTE: this only works under UNIX systems.
 */
int use_autowiz = NO;

/* If yes, what is the lowest level which should be on the wizlist?  (All
   immort levels below the level you specify will go on the immlist instead.) */
int min_wizlist_lev = LVL_GOD;

