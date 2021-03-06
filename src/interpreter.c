/*
************************************************************************
*   File: interpreter.c                                 Part of CircleMUD *
*  Usage: parse user commands, search for specials, call ACMD1675 functions   *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#define __INTERPRETER_C__

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "comm.h"
#include "interpreter.h"
#include "db.h"
#include "utils.h"
#include "spells.h"
#include "handler.h"
#include "mail.h"
#include "screen.h"
#include "genolc.h"
#include "oasis.h"
#include "tedit.h"
#include "dg_scripts.h"
#include "pfdefaults.h"


extern room_vnum mortal_start_room;
extern room_rnum r_immort_start_room;
extern room_rnum r_frozen_start_room;
extern const char *class_menu;
extern const char *ANSI;
extern const char *GREETINGS;
extern char *motd;
extern char *imotd;
extern char *background;
extern char *MENU;
extern char *WELC_MESSG;
extern char *START_MESSG;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct player_index_element *player_table;
extern int top_of_p_table;
extern int circle_restrict;
extern int no_specials;
extern int max_bad_pws;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct room_data *world;
extern int backup_wiped_pfiles;
extern int selfdelete_fastwipe;
extern int xap_objs;
extern int idle_rent_time;
int color = 0;
extern struct clan_type *clan_info;
extern bool CANMULTIPLAY;
/* external functions */
void make_corpse (struct char_data *ch);
void echo_on (struct descriptor_data *d);
void echo_off (struct descriptor_data *d);
void do_start (struct char_data *ch);
int parse_class (char arg);
int special (struct char_data *ch, int cmd, char *arg);
int isbanned (char *hostname);
int Valid_Name (char *newname);
void read_aliases (struct char_data *ch);
void assemblies_parse (struct descriptor_data *d, char *arg);
extern void assedit_parse (struct descriptor_data *d, char *arg);
void read_saved_vars (struct char_data *ch);
void trigedit_parse (struct descriptor_data *d, char *arg);
void hedit_parse (struct descriptor_data *d, char *arg);
void add_llog_entry (struct char_data *ch, int type);
void remove_player (int pfilepos);
void roll_real_abils (struct char_data *ch);
void check_idling (struct char_data *ch);

/* local functions */
int perform_dupe_check (struct descriptor_data *d);
struct alias_data *find_alias (struct alias_data *alias_list, char *str);
void free_alias (struct alias_data *a);
void perform_complex_alias (struct txt_q *input_q, char *orig,
			    struct alias_data *a);
int perform_alias (struct descriptor_data *d, char *orig);
int reserved_word (char *argument);
int find_name (char *name);
int _parse_name (char *arg, char *name);


/* prototypes for all do_x functions are now in the interpreter.h. */

ACMD (do_assassinrank);
ACMD (do_mark);
ACMD (do_petition);

struct command_info *complete_cmd_info;

/* This is the Master Command List(tm).

 * You can put new commands in, take commands out, change the order
 * they appear in, etc.  You can adjust the "priority" of commands
 * simply by changing the order they appear in the command list.
 * (For example, if you want "as" to mean "assist" instead of "ask",
 * just put "assist" above "ask" in the Master Command List(tm).
 *
 * In general, utility commands such as "at" should have high priority;
 * infrequently used and dangerously destructive commands should have low
 * priority.
 */

cpp_extern const struct command_info cmd_info[] = {
	/* this must be first -- for specprocs */
  {"RESERVED", 0, 0, 0, 0},

  /* directions must come before other commands but after RESERVED */
  {"north", "north", POS_STANDING, do_move, 0, SCMD_NORTH},
  {"east" , "east" , POS_STANDING, do_move, 0, SCMD_EAST},
  {"south", "south", POS_STANDING, do_move, 0, SCMD_SOUTH},
  {"west" , "west" , POS_STANDING, do_move, 0, SCMD_WEST},
  {"up"   , "up"   , POS_STANDING, do_move, 0, SCMD_UP},
  {"down" , "down" , POS_STANDING, do_move, 0, SCMD_DOWN},

  /* now, the main list */
  {"at", "at", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_AT},
  {"advance", "adv", POS_DEAD, do_advance, LVL_IMPL, 0},
  {"aedit", "aed", POS_DEAD, do_oasis, LVL_IMMORT, SCMD_OASIS_AEDIT},
  {"alias", "ali", POS_DEAD, do_alias, 0, 0},
  {"affects", "affects", POS_SLEEPING, do_affects, 0, 0},
  {"afk", "afk", POS_SLEEPING, do_gen_tog, 0, SCMD_AFK},
  {"apply", "apply", POS_RESTING, do_not_here, 0, 0},
  {"ankh", "ankh", POS_DEAD, do_not_here, 1, 0},
  {"assist", "ass", POS_FIGHTING, do_assist, 1, 0},
  {"assedit", "assedit", POS_RESTING, do_immcmd, LVL_IMMORT, IMM_ASSEDIT},
  {"assemble", "assemble", POS_STANDING, do_not_here, 0, 0},
  /*{ "assemble" , POS_SITTING , do_assemble , 0, SCMD_ASSEMBLE }, */
  {"ask", "ask", POS_RESTING, do_spec_comm, 0, SCMD_ASK},
  {"auction", "auct", POS_SLEEPING, do_gen_comm, 0, SCMD_AUCTION},
  {"autoall", "autoall", POS_DEAD, do_autoall, 0, 0},
  {"autoexit", "autoexit", POS_DEAD, do_gen_tog, 0, SCMD_AUTOEXIT},
  {"autosplit", "autosplit", POS_DEAD, do_gen_tog, 0, SCMD_AUTOSPLIT},
  {"autodiag", "autodiag", POS_DEAD, do_gen_tog, 0, SCMD_AUTODIAG},
  {"autoloot", "autoloot", POS_DEAD, do_gen_tog, 0, SCMD_AUTOLOOT},
  {"autogold", "autogold", POS_DEAD, do_gen_tog, 0, SCMD_AUTOGOLD},
  {"autoassist", "autoassist", POS_DEAD, do_gen_tog, 0, SCMD_AUTOASSIST},
  {"autosacrifice", "autosacrifice", POS_DEAD, do_gen_tog, 0, SCMD_AUTOSAC},
  {"autoaffects", "autoaffects", POS_DEAD, do_gen_togx, 0, SCMD_AUTOAFFECTS},
  {"autotick", "autotick", POS_DEAD, do_gen_togx, 0, SCMD_AUTOTICK},
  {"autologon", "autologon", POS_DEAD, do_gen_togx, 0, SCMD_AUTOLOGON},

  {"backstab", "ba", POS_STANDING, do_backstab, 1, 0},
  {"bs", "bs", POS_STANDING, do_backstab, 1, 0},
  {"ban", "ban", POS_DEAD, do_ban, LVL_IMPL, 0},
  {"bandage", "bandage", POS_STANDING, do_bandage, 0, 0},
  {"bankxfer", "bankxfer", POS_RESTING, do_not_here, 0, 0},
  {"balance", "bal", POS_STANDING, do_not_here, 1, 0},
  {"banish", "banish", POS_STANDING, do_not_here, 1, 0},
  {"bash", "bash", POS_FIGHTING, do_bash, 1, 0},
  {"bet", "bet", POS_RESTING, do_not_here, 0, 0},
  {"boom", "boom", POS_SLEEPING, do_boom, LVL_DEITY, 0},
  {"bounty", "bounty", POS_STANDING, do_not_here, 0, 0},
  {"brief", "brief", POS_DEAD, do_gen_tog, 0, SCMD_BRIEF},
  {"buy", "buy", POS_STANDING, do_not_here, 0, 0},
  {"bug", "bug", POS_DEAD, do_gen_write, 0, SCMD_BUG},
  {"butcher", "butcher", POS_STANDING, do_not_here, 0, 0},
  {"buildwalk", "buildwalk", POS_STANDING, do_gen_togx, LVL_IMMORT, IMM_BUILDWALK},


  {"cast", "c", POS_SITTING, do_cast, 1, 0},
  {"castout", "castout", POS_SITTING, do_castout, 10, 0},
  {"caccept", "caccept", POS_SLEEPING, do_clan, 0, SCMD_CLAN_ACCEPT},
  {"capply", "capply", POS_SLEEPING, do_clan, 0, SCMD_CLAN_APPLY},
  {"cedit", "cedit", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_CEDIT},
  {"cdismiss", "cdismiss", POS_SLEEPING, do_clan, 0, SCMD_CLAN_DISMISS},
  {"change", "change", POS_RESTING, do_not_here, 0, 0},
  {"chopwood", "chopwood", POS_STANDING, do_chopwood, 0, 0},
  {"clear", "clear", POS_DEAD, do_gen_ps, 0, SCMD_CLEAR},
  {"clist", "clist", POS_SLEEPING, do_clan, 0, SCMD_CLAN_LIST},
  {"cpass", "cpass", POS_SLEEPING, do_clan, 0, SCMD_CLAN_PASSWORD},
  {"cmembers", "cmembers", POS_SLEEPING, do_clan, 0, SCMD_CLAN_MEMBER},
  {"cbalance", "cbalance", POS_SLEEPING, do_clan, 0, SCMD_CLAN_BALANCE},
  {"cdeposit", "cdeposit", POS_STANDING, do_clan, 0, SCMD_CLAN_DEPOSIT},
  {"cwithdraw", "cwithdraw", POS_STANDING, do_clan, 0, SCMD_CLAN_TAKEOUT},
  {"check", "check", POS_DEAD, do_check, 0, 0},
  {"close", "clo", POS_SITTING, do_gen_door, 0, SCMD_CLOSE},
  {"cls", "cls", POS_DEAD, do_gen_ps, 0, SCMD_CLEAR},
  {"compare", "compare", POS_RESTING, do_compare, 0, 0},
  {"consider", "con", POS_RESTING, do_consider, 0, 0},
  {"conspire", "conspire", POS_SLEEPING, do_conspire, 0, SCMD_CONSPIRE},
  {"color", "col", POS_DEAD, do_color, 0, 0},
  {"commands", "comm", POS_DEAD, do_commands, 0, SCMD_COMMANDS},
  {"compact", "comp", POS_DEAD, do_gen_tog, 0, SCMD_COMPACT},
  {"copyover", "copyover", POS_DEAD, do_copyover, LVL_GRGOD, 0},
  {"copyto", "copyto", POS_STANDING, do_immcmd, LVL_IMMORT, IMM_COPYTO},
  {"credits", "cred", POS_DEAD, do_gen_ps, 0, SCMD_CREDITS},
  {"creject", "creject", POS_SLEEPING, do_clan, 0, SCMD_CLAN_REJECT},
  {"cresign", "cresign", POS_SLEEPING, do_clan, 0, SCMD_CLAN_RESIGN},
  {"csay", "csay", POS_SLEEPING, do_clan, 0, SCMD_CLAN_TELL},
  {"ctell", "ctell", POS_SLEEPING, do_clan, 0, SCMD_CLAN_TELL},
  {"cwho", "cwho", POS_SLEEPING, do_clan, 0, SCMD_CLAN_WHO},
  {"cwithdraw", "cwithdraw", POS_SLEEPING, do_clan, 0, SCMD_CLAN_WITHDRAW},
  {"cuss", "cuss", POS_DEAD, do_cuss, 0, SCMD_CUSS},
  {"canmulti", "canmulti", POS_SLEEPING, do_immcmd, LVL_IMMORT, IMM_CANMULTI},
  {"canass", "canass", POS_SLEEPING, do_immcmd, LVL_IMMORT, IMM_CANASS},


  {"date", "date", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_DATE},
  {"dc", "dc", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_DC},
//  { "default"  , POS_STANDING, do_default  , LVL_IMPL, 0 },
  {"death", "death", POS_RESTING, do_not_here, 0, 0},
  {"deathtoall", "deathtoall", POS_RESTING, do_immcmd, LVL_IMMORT, IMM_DEATHTOALL},
  {"die", "die", POS_DEAD, do_die, 0, 0},
  {"depiss", "depiss", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_DEPISS},
  {"deposit", "depo", POS_STANDING, do_not_here, 1, 0},
  {"death", "death", POS_STANDING, do_not_here, 1, 0},
  {"diagnose", "diag", POS_RESTING, do_diagnose, 0, 0},
  {"dig", "dig", POS_STANDING, do_immcmd, LVL_IMMORT, IMM_DIG},
  {"disarm", "disarm", POS_FIGHTING, do_disarm, 0, 0},
  {"display", "disp", POS_DEAD, do_display, 0, 0},
  {"donate", "donate", POS_RESTING, do_drop, 0, SCMD_DONATE},
  {"drink", "drink", POS_RESTING, do_drink, 0, SCMD_DRINK},
  {"drop", "drop", POS_RESTING, do_drop, 0, SCMD_DROP},
  {"doubleexp", "doubleexp", POS_RESTING, do_immcmd, LVL_IMMORT, IMM_DEXP},
  {"doublegol", "doublegol", POS_RESTING, do_immcmd, LVL_IMMORT, IMM_DGOLD},

  {"eat", "eat", POS_RESTING, do_eat, 0, SCMD_EAT},
  {"echo", "echo", POS_SLEEPING, do_immcmd, LVL_IMMORT, IMM_ECHO},
  {"effects", "effects", POS_SLEEPING, do_affects, 0, 0},
  {"embalm", "embalm", POS_STANDING, do_not_here, 66, 0},
  {"emote", "emote", POS_RESTING, do_echo, 1, SCMD_EMOTE},
  {":", "emote", POS_RESTING, do_echo, 1, SCMD_EMOTE},
  {"enter", "enter", POS_STANDING, do_enter, 0, 0},
  {"equipment", "eq", POS_SLEEPING, do_equipment, 0, 0},
  {"exchange", "exchange", POS_RESTING, do_not_here, 60, 0},
  {"exits", "ex", POS_RESTING, do_exits, 0, 0},
  {"examine", "exa", POS_SITTING, do_examine, 0, 0},
  {"edit", "edit", POS_DEAD, do_edit, LVL_IMPL, 0},	/* Testing! */

  {"force", "f", POS_SLEEPING, do_immcmd, LVL_IMMORT, IMM_FORCE},
  {"forge", "forge", POS_STANDING, do_not_here, 0, 0},
  {"file", "file", POS_RESTING, do_immcmd, LVL_IMMORT, IMM_FILE},
  {"fill", "fill", POS_STANDING, do_pour, 0, SCMD_FILL},
  {"finger", "finger", POS_RESTING, do_finger, 1, 0},
  {"find", "find", POS_RESTING, do_immcmd, LVL_IMMORT, IMM_FIND},
  {"flee", "fl", POS_FIGHTING, do_flee, 1, 0},
  {"follow", "fol", POS_RESTING, do_follow, 0, 0},
  {"freeze", "fr", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_FREEZE},
  {"fullexit", "fullexit", POS_DEAD, do_gen_togx, 0, SCMD_FULLEXIT},

  {"get", "g", POS_RESTING, do_get, 0, 0},
  {"gecho", "gecho", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_GECHO},
  {"gemote", "gemote", POS_DEAD, do_gen_comm, 0, SCMD_GMOTE},	/*add this */
  {"ghistory", "ghistory", POS_DEAD, do_ghistory, 0, 0},
  {"give", "gi", POS_RESTING, do_give, 0, 0},
  {"gjob", "gjob", POS_SLEEPING, do_gjob, 0, 0},
  {"gms", "gms", POS_DEAD, do_gen_ps, 0, SCMD_GMS},
  {"gmlist", "gmlist", POS_DEAD, do_gen_ps, 0, SCMD_GMS},
  {"gossip", "go", POS_DEAD, do_gen_comm, 0, SCMD_GOSSIP},
  {"goto", "got", POS_SLEEPING, do_goto, LVL_IMMORT, 0},
  {"gold", "gol", POS_RESTING, do_gold, 0, 0},
  {"goldchip", "goldchip", POS_RESTING, do_immcmd, LVL_IMMORT, IMM_GOLDCHIP},
  {"group", "gr", POS_SLEEPING, do_group, 1, 0},
  {"grab", "grab", POS_RESTING, do_grab, 0, 0},
  {"grats", "grats", POS_SLEEPING, do_gen_comm, 0, SCMD_GRATZ},
  {"gsay", "gs", POS_SLEEPING, do_gsay, 0, 0},
  {"gtell", "gt", POS_SLEEPING, do_gsay, 0, 0},
  {"gtitle", "gtitle", POS_SLEEPING, do_gtitle, LVL_IMMORT, 0},
  {"gwho", "gwho", POS_SLEEPING, do_gwho, 0, 0},
  {"gain", "gain", POS_RESTING, do_not_here, 0, 0},

  {"haffinate", "haffinate", POS_STANDING, do_immcmd, LVL_IMMORT, IMM_HAFF},
  /*{"happystart", "happystart", POS_DEAD, do_happystart, LVL_GRGOD, 0},
  {"happyend", "happyend", POS_DEAD, do_happyend, LVL_GRGOD, 0},*/
  {"happytimes", "happytimes", POS_DEAD, do_happytimes, 0, 0},
  {"hearall", "hearall", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_HEARALL},
  {"hearlevel", "hearlevel", POS_DEAD, do_gen_togx, 0, SCMD_HEARLEVEL},
  {"help", "h", POS_DEAD, do_help, 0, 0},
  {"helpcheck", "helpcheck", POS_DEAD, do_helpcheck, LVL_IMMORT, 0},
  {"hedit", "hedit", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_HEDIT},
  {"handbook", "handbook", POS_DEAD, do_gen_ps, LVL_IMMORT, SCMD_HANDBOOK},
  {"hcontrol", "hcontrol", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_HOUSE},
  {"hide", "hide", POS_RESTING, do_hide, 1, 0},
  {"hit", "hit", POS_FIGHTING, do_hit, 0, SCMD_HIT},
  {"hold", "hold", POS_RESTING, do_grab, 1, 0},
  {"holler", "holler", POS_RESTING, do_gen_comm, 1, SCMD_HOLLER},
  {"holylight", "holy", POS_DEAD, do_gen_tog, LVL_IMMORT, SCMD_HOLYLIGHT},
  {"house", "house", POS_RESTING, do_house, 0, 0},
  {"hunt", "hunt", POS_STANDING, do_hunt, 0, 0},

  {"inventory", "i", POS_DEAD, do_inventory, 0, 0},
  {"ice", "ice", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_JAIL},
  {"iedit", "iedit", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_IEDIT},
  {"idea", "idea", POS_DEAD, do_gen_write, 0, SCMD_IDEA},
  {"imotd", "im", POS_DEAD, do_gen_ps, LVL_IMMORT, SCMD_IMOTD},
  {"gmlist", "gmlist", POS_DEAD, do_gen_ps, 0, SCMD_IMMLIST},
  {"info", "in", POS_SLEEPING, do_gen_ps, 0, SCMD_INFO},
  {"insult", "insult", POS_RESTING, do_insult, 0, 0},
  {"invis", "invis", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_INVIS},

  {"jail", "jail", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_JAIL},
  {"join", "join", POS_STANDING, do_not_here, 25, 0},
  {"junk", "j", POS_RESTING, do_drop, 0, SCMD_JUNK},

  {"kill", "ki", POS_FIGHTING, do_kill, 0, 0},
  {"kick", "kick", POS_FIGHTING, do_kick, 1, 0},

  {"look", "l", POS_RESTING, do_look, 0, SCMD_LOOK},
  {"land", "land", POS_STANDING, do_land, 1, 0},
  {"lag", "lag", POS_RESTING, do_immcmd, LVL_IMMORT, IMM_LAG},	/* hahaha nice */
  {"last", "last", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_LAST},
  {"leave", "leave", POS_STANDING, do_leave, 0, 0},
  {"levels", "lev", POS_SLEEPING, do_level, 0, 0},
  {"list", "li", POS_STANDING, do_not_here, 0, 0},
  {"lock", "lo", POS_SITTING, do_gen_door, 0, SCMD_LOCK},
  {"load", "load", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_LOAD},

  {"mark", "mark", POS_SITTING, do_mark, 50, 0},
  {"medit", "med", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_MEDIT},
  {"mine", "mine", POS_STANDING, do_mine, 0, 0},
  {"motd", "m", POS_DEAD, do_gen_ps, 0, SCMD_MOTD},
  {"mail", "mail", POS_STANDING, do_not_here, 1, 0},
  {"mute", "mu", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_MUTE},
  {"murder", "murder", POS_FIGHTING, do_hit, 0, SCMD_MURDER},

  {"news", "n", POS_SLEEPING, do_gen_ps, 0, SCMD_NEWS},
  {"newbie", "newbie", POS_DEAD, do_gen_comm, 0, SCMD_NEWBIE},
  {"noauction", "noauction", POS_DEAD, do_gen_tog, 0, SCMD_NOAUCTION},
  {"nocuss", "nocuss", POS_DEAD, do_gen_togx, 0, SCMD_CUSS},
  {"nogive", "nogive", POS_STANDING, do_gen_togx, 0, SCMD_NOGIVE},
  {"nogossip", "nogossip", POS_DEAD, do_gen_tog, 0, SCMD_NOGOSSIP},
  {"nograts", "nograts", POS_DEAD, do_gen_tog, 0, SCMD_NOGRATZ},
  {"nohassle", "nohassle", POS_DEAD, do_gen_tog, LVL_IMMORT, SCMD_NOHASSLE},
  {"norent", "norent", POS_DEAD, do_wizutil, LVL_DEITY, SCMD_NORENT},
  {"norepeat", "norepeat", POS_DEAD, do_gen_tog, 0, SCMD_NOREPEAT},
  {"noshout", "noshout", POS_SLEEPING, do_gen_tog, 1, SCMD_DEAF},
  {"nosummon", "nosummon", POS_DEAD, do_gen_tog, 1, SCMD_NOSUMMON},
  {"notell", "notell", POS_DEAD, do_gen_tog, 1, SCMD_NOTELL},
  {"notitle", "notitle", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_NOTITLE},
  {"nowiz", "nowiz", POS_DEAD, do_gen_tog, LVL_IMMORT, SCMD_NOWIZ},

  {"order", "o", POS_RESTING, do_order, 1, 0},
  {"open", "open", POS_SITTING, do_gen_door, 0, SCMD_OPEN},
  {"olc", "olc", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_OLC},
  {"oedit", "oed", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_OEDIT},
  {"oarchy", "oarchy", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_OARCHY},
  {"olevel", "olevel", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_OLEVEL},
  {"odelete", "odelete", POS_DEAD, do_odelete, LVL_IMMORT, 0},

  {"put", "p", POS_RESTING, do_put, 0, 0},
  {"password", "password", POS_STANDING, do_not_here, 0, 0},
  {"page", "pa", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_PAGE},
  {"pardon", "pard", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_PARDON},
  {"peace", "peace", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_PEACE},
  {"petition", "petition", POS_DEAD, do_petition, 1, 0},
  {"pick", "pick", POS_STANDING, do_gen_door, 1, SCMD_PICK},
  {"policy", "pol", POS_DEAD, do_gen_ps, 0, SCMD_POLICIES},
  {"poofin", "poofin", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_POOFIN},
  {"poofout", "poofout", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_POOFOUT},
  {"pop", "pop", POS_DEAD, do_pop, LVL_IMMORT, 0},
  {"pour", "pour", POS_STANDING, do_pour, 0, SCMD_POUR},
  {"prompt", "pr", POS_DEAD, do_display, 0, 0},
  {"practice", "practice", POS_RESTING, do_practice, 1, 0},
  {"pretitle", "pretitle", POS_RESTING, do_immcmd, LVL_IMMORT, IMM_PRETITLE},
  {"purge", "purge", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_PURGE},
  {"push", "push", POS_STANDING, do_push, 0, 0},
  {"pyoink", "pyoink", POS_RESTING, do_immcmd, LVL_IMMORT, IMM_PYOINK},
  {"pull", "pull", POS_STANDING, do_not_here, 0, 0},

  {"quaff", "q", POS_RESTING, do_use, 0, SCMD_QUAFF},
  {"qecho", "qecho", POS_DEAD, do_immcmd, LVL_IMMORT, SCMD_QECHO},
  {"quest", "quest", POS_DEAD, do_gen_tog, 0, SCMD_QUEST},
  {"queston", "queston", POS_RESTING, do_immcmd, LVL_IMMORT, IMM_QUESTON},
  {"qui", "qui", POS_RESTING, do_quit, 0, 0},
  {"quit", "quit", POS_RESTING, do_quit, 0, SCMD_QUIT},
  {"qsay", "qs", POS_RESTING, do_qcomm, 0, SCMD_QSAY},

  {"rdelete", "rdelete", POS_DEAD, do_rdelete, LVL_IMMORT, 0},
  {"recite", "rec", POS_RESTING, do_use, 0, SCMD_RECITE},
  {"recall", "recall", POS_STANDING, do_recall, 0, 0},
  {"recharge", "recharge", POS_RESTING, do_not_here, 0, 0},
  {"reelin", "reelin", POS_SITTING, do_reelin, 10, 0},
  {"reply", "r", POS_SLEEPING, do_reply, 0, 0},
  {"repiss", "repiss", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_REPISS},
  {"rest", "re", POS_RESTING, do_rest, 0, 0},
  {"read", "read", POS_RESTING, do_look, 0, SCMD_READ},
  {"reload", "rel", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_RELOAD},
  {"recite", "recite", POS_RESTING, do_use, 0, SCMD_RECITE},
  {"receive", "receive", POS_STANDING, do_not_here, 1, 0},
  {"remove", "rem", POS_RESTING, do_remove, 0, 0},
  {"report", "repo", POS_RESTING, do_report, 0, 0},
  {"rent", "rent", POS_STANDING, do_not_here, 0, 0},
  {"reroll", "reroll", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_REROLL},
  {"rescue", "rescue", POS_FIGHTING, do_rescue, 1, 0},
  {"restore", "restore", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_RESTORE},
  {"return", "return", POS_DEAD, do_return, 0, 0},
  {"redit", "red", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_REDIT},
  {"reimburse", "reimburse", POS_SLEEPING, do_reimbursal, LVL_IMPL, 0},
  {"rewardall", "rewardall", POS_SLEEPING, do_immcmd, LVL_IMMORT, IMM_REWARDALL},
  {"roomflags", "roomfl", POS_DEAD, do_gen_tog, LVL_IMMORT, SCMD_ROOMFLAGS},
  {"roomlink", "roomlink", POS_RESTING, do_roomlink, LVL_IMMORT, 0},

  {"sacrifice", "sacrifice", POS_RESTING, do_sacrifice, 0, 0},
  {"say", "s", POS_RESTING, do_say, 0, 0},
  {"'", "s", POS_RESTING, do_say, 0, 0},
  {"save", "sa", POS_SLEEPING, do_save, 0, 0},
  {"saveall", "saveall", POS_SLEEPING, do_immcmd, LVL_IMMORT, IMM_SAVEALL},
  {"score", "sc", POS_DEAD, do_score, 0, 0},
  {"scan", "scan", POS_STANDING, do_scan, 0, 0},
  {"sell", "sell", POS_STANDING, do_not_here, 0, 0},
  {"send", "send", POS_SLEEPING, do_immcmd, LVL_IMMORT, IMM_SEND},
  {"set", "set", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_SET},
  {"sedit", "sed", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_SEDIT},
  {"shout", "sh", POS_RESTING, do_gen_comm, 0, SCMD_SHOUT},
  {"shade", "shade", POS_STANDING, do_shade, 0, 0},
  {"shoot", "shoot", POS_STANDING, do_shoot, 0, 0},
  {"show", "show", POS_DEAD, do_show, 0, 0},
  {"shutdow", "shutdow", POS_DEAD, do_shutdown, LVL_IMPL, 0},
  {"shutdown", "shutdown", POS_DEAD, do_shutdown, LVL_IMPL, SCMD_SHUTDOWN},
  {"sip", "sip", POS_RESTING, do_drink, 0, SCMD_SIP},
  {"sit", "sit", POS_RESTING, do_sit, 0, 0},
  {"skillthief", "skillthief", POS_SLEEPING, do_skill, 0, SCMD_THIEF},
  {"skillmage", "skillmage", POS_SLEEPING, do_skill, 0, SCMD_MAGE},
  {"skillcleric", "skillcleric", POS_SLEEPING, do_skill, 0, SCMD_CLERIC},
  {"skillwarrior", "skillwarrior", POS_SLEEPING, do_skill, 0, SCMD_WARRIOR},
  {"skillset", "skillset", POS_SLEEPING, do_immcmd, LVL_IMMORT, IMM_SKILLSET},
  {"skillreset", "skillreset", POS_SLEEPING, do_immcmd, LVL_IMMORT, IMM_SKILLRESET},
  {"sleep", "sleep", POS_SLEEPING, do_sleep, 0, 0},
  {"slowns", "slowns", POS_DEAD, do_gen_tog, LVL_IMPL, SCMD_SLOWNS},
  {"sneak", "sn", POS_STANDING, do_sneak, 1, 0},
  {"snoop", "snoop", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_SNOOP},
  {"socials", "soc", POS_DEAD, do_commands, 0, SCMD_SOCIALS},
  {"split", "split", POS_SITTING, do_split, 1, 0},
  {"sparrank", "sparrank", POS_SLEEPING, do_sparrank, 0, 0},
  {"stab", "stab", POS_FIGHTING, do_stab, 0, 0},
  {"stand", "st", POS_SLEEPING, do_stand, 0, 0},
  {"stock", "stock", POS_RESTING, do_not_here, 0, 0},
  {"stat", "stat", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_STAT},
  {"statlist", "statlist", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_STATLIST},
  {"steal", "ste", POS_STANDING, do_steal, 1, 0},
  {"stop", "stop", POS_SLEEPING, do_stop, 0, 0},
  {"switch", "swit", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_SWITCH},
  {"syslog", "sys", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_SYSLOG},
//  { "subscribe", POS_DEAD    , do_subscribe, PERM_PLAYER, 0},

  {"tell", "t", POS_SLEEPING, do_tell, 0, 0},
  {"take", "take", POS_RESTING, do_get, 0, 0},
  {"taste", "taste", POS_RESTING, do_eat, 0, SCMD_TASTE},
  {"teleport", "tele", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_TELEPORT},
  {"tedit", "ted", POS_DEAD, do_immcmd, LVL_IMPL, IMM_TEDIT},	/* XXX: Oasisify */
  {"tether", "tether", POS_STANDING, do_tether, 0, 0},
  {"thistory", "thistory", POS_DEAD, do_thistory, 0, 0},
  {"thaw", "thaw", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_THAW},
  {"throw", "throw", POS_STANDING, do_throw, 0, 0},
  {"title", "ti", POS_DEAD, do_title, 0, 0},
  {"tick", "tick", POS_RESTING, do_tick, 0, 0},
  {"time", "time", POS_DEAD, do_time, 0, 0},
  {"toggle", "tog", POS_DEAD, do_toggle, 0, 0},
  {"track", "tr", POS_STANDING, do_track, 0, 0},
  {"trackthru", "trackthru", POS_DEAD, do_gen_tog, LVL_IMPL, SCMD_TRACK},
  {"train", "train", POS_STANDING, do_not_here, 0, 0},
  {"transfer", "trans", POS_SLEEPING, do_immcmd, LVL_IMMORT, IMM_TRANSFER},
  {"trip", "trip", POS_FIGHTING, do_trip, 0, 0},
  {"trigedit", "trig", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_TRIGEDIT},
  {"typo", "typo", POS_DEAD, do_gen_write, 0, SCMD_TYPO},

  {"unlock", "un", POS_SITTING, do_gen_door, 0, SCMD_UNLOCK},
  {"understand", "understand", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_UNDERSTAND},
  {"undig", "undig", POS_RESTING, do_immcmd, LVL_IMMORT, IMM_UNDIG},
  {"ungroup", "ungr", POS_DEAD, do_ungroup, 0, 0},
  {"unban", "unban", POS_DEAD, do_unban, LVL_IMPL, 0},
  {"unaffect", "unaff", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_UNAFFECT},
  {"unsaved", "unsaved", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_OLC},
  {"upgrade", "upgrade", POS_STANDING, do_not_here, 60, 0},
  {"uptime", "upt", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_UPTIME},
  {"use", "use", POS_SITTING, do_use, 1, SCMD_USE},
  {"users", "users", POS_DEAD, do_users, LVL_IMMORT, 0},

  {"value", "val", POS_STANDING, do_not_here, 0, 0},
  {"version", "version", POS_DEAD, do_gen_ps, 0, SCMD_VERSION},
  {"verify", "verify", POS_STANDING, do_immcmd, LVL_IMMORT, IMM_VERIFY},
  {"visible", "vis", POS_RESTING, do_visible, 1, 0},
  {"vnum", "vnum", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_VNUM},
  {"vstat", "vstat", POS_DEAD, do_vstat, LVL_IMMORT, 0},
  {"vlist", "vlist", POS_DEAD, do_vlist, LVL_IMMORT, 0},

  {"wake", "wa", POS_SLEEPING, do_wake, 0, 0},
  {"wear", "wea", POS_RESTING, do_wear, 0, 0},
  {"weather", "weather", POS_RESTING, do_weather, 0, 0},
  {"who", "wh", POS_DEAD, do_who, 0, 0},
  {"whoami", "whoami", POS_DEAD, do_gen_ps, 0, SCMD_WHOAMI},
  {"where", "where", POS_RESTING, do_where, 1, 0},
  {"whoinvis", "whoinvis", POS_RESTING, do_whoinvis, 50, 0},
  {"whisper", "whisp", POS_RESTING, do_spec_comm, 0, SCMD_WHISPER},
  {"whirlwind", "whirlwind", POS_FIGHTING, do_whirlwind, 0, 0},
  {"whatis", "whatis", POS_DEAD, do_not_here, 0, 0},
  {"wield", "wi", POS_RESTING, do_wield, 0, 0},
  {"wimpy", "wimp", POS_DEAD, do_wimpy, 0, 0},
  {"withdraw", "withdraw", POS_STANDING, do_not_here, 1, 0},
  {"wiznet", "wiz", POS_DEAD, do_wiznet, LVL_IMMORT, 0},
  {"architect", "wiz", POS_DEAD, do_wiznet, LVL_IMMORT, SCMD_WIZARCHI},
  {"creator", "wiz", POS_DEAD, do_wiznet, LVL_CREATOR, SCMD_WIZCREATOR},
  {"deity", "wiz", POS_DEAD, do_wiznet, LVL_DEITY, SCMD_WIZDEITY},
  {"coder", "wiz", POS_DEAD, do_wiznet, LVL_CODER, SCMD_WIZCODER},
  {"coimp", "wiz", POS_DEAD, do_wiznet, LVL_COIMP, SCMD_WIZCOIMP},
  {"imp", "wiz", POS_DEAD, do_wiznet, LVL_IMPL, SCMD_WIZIMP},
  {";", "wiz", POS_DEAD, do_wiznet, LVL_IMMORT, 0},
  {"wizhelp", "wizhelp", POS_SLEEPING, do_wizhelp, LVL_IMMORT, SCMD_WIZHELP},
  {"wizlist", "wizlist", POS_DEAD, do_gen_ps, 0, SCMD_WIZLIST},
  {"wizlock", "wizlock", POS_DEAD, do_immcmd, LVL_IMMORT, IMM_WIZLOCK},
  {"write", "wr", POS_RESTING, do_write, 1, 0},

  {"xapobjs", "xapobjs", POS_DEAD, do_gen_tog, LVL_IMPL, SCMD_XAP_OBJS},
  {"xname", "xname", POS_DEAD, do_xname, LVL_IMPL, 0},


  {"zedit", "zed", POS_DEAD, do_oasis, LVL_BUILDER, SCMD_OASIS_ZEDIT},
  {"zreset", "zreset", POS_DEAD, do_zreset, LVL_IMMORT, 0},
  {"zclear", "zclear", POS_DEAD, do_zclear, LVL_IMPL, 0},

  {"zsave", "zsave", POS_DEAD, do_zsave, LVL_IMMORT, 0},
  {"zdelete", "zdelete", POS_STANDING, do_zdelete, LVL_IMPL, 0},
  {"zhelp", "zhelp", POS_STANDING, do_zhelp, LVL_IMMORT, 0},

  /* Liblist */
  {"mlist", "mlist", POS_DEAD, do_liblist, LVL_IMMORT, SCMD_MLIST},
  {"olist", "olist", POS_DEAD, do_liblist, LVL_IMMORT, SCMD_OLIST},
  {"rlist", "rlist", POS_DEAD, do_liblist, LVL_IMMORT, SCMD_RLIST},
  {"zlist", "zlist", POS_DEAD, do_liblist, LVL_IMMORT, SCMD_ZLIST},

  /* DG trigger commands */
  {"attach", "attach", POS_DEAD, do_attach, LVL_CREATOR, 0},
  {"detach", "detach", POS_DEAD, do_detach, LVL_IMMORT, 0},
  {"tlist", "tlist", POS_DEAD, do_tlist, LVL_IMMORT, 0},
  {"tstat", "tstat", POS_DEAD, do_tstat, LVL_IMMORT, 0},
  {"masound", "masound", POS_DEAD, do_masound, -1, 0},
  {"mkill", "mkill", POS_STANDING, do_mkill, -1, 0},
  {"mdelete", "mdelete", POS_DEAD, do_mdelete, LVL_IMMORT, 0},
  {"mdamage", "mdamage", POS_DEAD, do_mdamage, -1, 0},
  {"mjunk", "mjunk", POS_SITTING, do_mjunk, -1, 0},
  {"mdoor", "mdoor", POS_DEAD, do_mdoor, -1, 0},
  {"mecho", "mecho", POS_DEAD, do_mecho, -1, 0},
  {"mechoaround", "mechoaround", POS_DEAD, do_mechoaround, -1, 0},
  {"msend", "msend", POS_DEAD, do_msend, -1, 0},
  {"mload", "mload", POS_DEAD, do_mload, -1, 0},
  {"mpurge", "mpurge", POS_DEAD, do_mpurge, -1, 0},
  {"mgoto", "mgoto", POS_DEAD, do_mgoto, -1, 0},
  {"mat", "mat", POS_DEAD, do_mat, -1, 0},
  {"mteleport", "mteleport", POS_DEAD, do_mteleport, -1, 0},
  {"mforce", "mforce", POS_DEAD, do_mforce, -1, 0},
  {"mexp", "mexp", POS_DEAD, do_mexp, -1, 0},
  {"mgold", "mgold", POS_DEAD, do_mgold, -1, 0},
  {"mhunt", "mhunt", POS_DEAD, do_mhunt, -1, 0},
  {"mremember", "mremember", POS_DEAD, do_mremember, -1, 0},
  {"mforget", "mforget", POS_DEAD, do_mforget, -1, 0},
  {"mtransform", "mtransform", POS_DEAD, do_mtransform, -1, 0},
  {"mskillset", "mskillset", POS_RESTING, do_mskillset, -1, 0},
  {"mteach", "mteach", POS_RESTING, do_mskillset, -1, 0},
  {"mzoneecho", "mzoneecho", POS_DEAD, do_mzoneecho, -1, 0},
  {"vdelete", "vdelete", POS_DEAD, do_vdelete, LVL_GRGOD, 0},

  /* Running Exec */
  {"swho", "swho", POS_RESTING, do_swho, LVL_IMPL, 0},
  {"slast", "slast", POS_RESTING, do_slast, LVL_IMPL, 0},
  {"grep", "grep", POS_RESTING, do_grep, LVL_IMPL, 0},
  {"sps", "sps", POS_RESTING, do_sps, LVL_IMPL, 0},
  {"ukill", "ukill", POS_RESTING, do_ukill, LVL_IMPL, 0},
  {"quota", "quota", POS_RESTING, do_quota, LVL_IMMORT, 0},

  /* Romance Module Command List */
  {"askout", "askout", POS_RESTING, do_askout, 1, 0},
  {"accept", "accept", POS_RESTING, do_accept, 1, 0},
  {"reject", "reject", POS_RESTING, do_reject, 1, 0},
  {"propose", "propose", POS_STANDING, do_propose, 1, 0},
  {"breakup", "breakup", POS_STANDING, do_breakup, 1, 0},
  {"marry", "marry", POS_STANDING, do_marry, LVL_IMPL, 0},
  {"divorce", "divorce", POS_STANDING, do_divorce, 1, 0},
  /* End Romance Module Command List */

  {"challenge", "challenge", POS_STANDING, do_challenge, 1, 0},
  {"acceptchallenge", "acceptchallenge", POS_STANDING, do_acceptchallenge, 1, 0},
  {"rejectchallenge", "rejectchallenge", POS_STANDING, do_rejectchallenge, 1, 0},


  /* Freeze tag if very run 
  {"addteam", "addteam", POS_STANDING, do_addteam, LVL_IMMORT, 0},
  {"startgame", "startgame", POS_STANDING, do_startgame, LVL_IMMORT, 0},
  {"endgame", "endgame", POS_STANDING, do_endgame, LVL_IMMORT, 0},
  {"tag", "tag", POS_STANDING, do_tag, 1, 0},
  {"tsay", "tsay", POS_DEAD, do_tsay, 1, 0}, */

  {"\n", "zzzzzzz", 0, 0, 0, 0} }; /* this must be last */


const char *fill[] = {
  "in",
  "from",
  "with",
  "the",
  "on",
  "at",
  "to",
  "\n"
};

const char *reserved[] = {
  "a",
  "an",
  "self",
  "me",
  "all",
  "room",
  "someone",
  "something",
  "\n"
};


/* COMMAND SEARCH TREE algorhitm ver 1.0
   Faster than light command searching by Yilard of VisionMUD 
   Copyright (C)1999 Electronic Phantasies */

#ifdef CMD_TREE_SEARCH

struct letter_info
{
  int cmd_index;
  int min_level;
  struct letter_dir *next;
};


struct letter_dir
{
  int num;
  char *letter_string;
  struct letter_info *letters;
};

struct letter_dir *cmd_tree = NULL;

#define CMD_LEVEL(cmd)      (cmd_info[cmd].minimum_level)

void
add_cmd (int cmd)
{
  struct letter_dir **tmpdir = &cmd_tree;
  int i = 0, inx;
  char buf[2], c;
  char *ptr;
  while (i < strlen (CMD_NAME))
    {
      c = tolower (CMD_NAME[i]);
      if (*tmpdir)
	{
	  ptr = strchr ((*tmpdir)->letter_string, c);
	  if (ptr)
	    {
	      inx = ptr - (*tmpdir)->letter_string;
	      if ((*tmpdir)->letters[inx].min_level > CMD_LEVEL (cmd))
		(*tmpdir)->letters[inx].min_level = CMD_LEVEL (cmd);
	      tmpdir = &((*tmpdir)->letters[inx].next);
	    }
	  else
	    {
	      ((*tmpdir)->num)++;
	      ptr =
		(char *) malloc (sizeof (char) *
				 (strlen ((*tmpdir)->letter_string) + 2));
	      strcpy (ptr, (*tmpdir)->letter_string);
	      ptr[strlen (ptr) + 1] = '\0';
	      ptr[strlen (ptr)] = c;
	      free ((*tmpdir)->letter_string);
	      (*tmpdir)->letter_string = ptr;
	      (*tmpdir)->letters =
		realloc ((*tmpdir)->letters,
			 (*tmpdir)->num * sizeof (struct letter_info));
	      (*tmpdir)->letters[(*tmpdir)->num - 1].cmd_index = cmd;
	      (*tmpdir)->letters[(*tmpdir)->num - 1].min_level =
		CMD_LEVEL (cmd);
	      (*tmpdir)->letters[(*tmpdir)->num - 1].next = NULL;
	      tmpdir = &((*tmpdir)->letters[(*tmpdir)->num - 1].next);
	    }
	}
      else
	{
	  buf[0] = c;
	  buf[1] = '\000';
	  *tmpdir = (struct letter_dir *) malloc (sizeof (struct letter_dir));
	  (*tmpdir)->letter_string = strdup (buf);
	  (*tmpdir)->num = 1;
	  (*tmpdir)->letters =
	    (struct letter_info *) malloc (sizeof (struct letter_info));
	  (*tmpdir)->letters[0].cmd_index = cmd;
	  (*tmpdir)->letters[0].min_level = CMD_LEVEL (cmd);
	  (*tmpdir)->letters[0].next = NULL;
	  tmpdir = &((*tmpdir)->letters[0].next);
	}
      i++;
    }
}

int
build_cmd_tree (void)
{
  int cmd = 1;
  while (CMD_NAME[0] != '\n')
    add_cmd (cmd++);
  return cmd - 1;
}

int
find_command_level (struct letter_dir *dir, int level)
{
  int i, r;
  for (i = 0; i < dir->num; i++)
    if (CMD_LEVEL (dir->letters[i].cmd_index) <= level)
      return dir->letters[i].cmd_index;

  for (i = 0; i < dir->num; i++)
    if (dir->letters[i].next)
      {
	r = find_command_level (dir->letters[i].next, level);
	if (r >= 0)
	  return r;
      }

  return -1;
}

int
tree_cmd_search (char *s, int level)
{
  register struct letter_dir *tmp = cmd_tree;
  register int i = 0;
  int inx = -1;
  char *ptr;
  while (i < strlen (s))
    {
      if (tmp)
	{
	  ptr = strchr (tmp->letter_string, s[i]);
	  if (ptr)
	    {
	      inx = ptr - tmp->letter_string;
	      if (tmp->letters[inx].min_level > level)
		return -1;
	      if (i + 1 >= strlen (s))
		{
		  if (CMD_LEVEL (tmp->letters[inx].cmd_index) <= level)
		    return tmp->letters[inx].cmd_index;
		  if (tmp->letters[inx].next)
		    return find_command_level (tmp->letters[inx].next, level);
		  else
		    return -1;
		}
	      tmp = tmp->letters[inx].next;
	    }
	  else
	    return -1;
	}
      else
	{
	  return -1;
	}
      i++;
    }
  return -1;
}

#endif
/*
 * This is the actual command interpreter called from game_loop() in comm.c
 * It makes sure you are the proper level and position to execute the command,
 * then calls the appropriate function.
 */
void
command_interpreter (struct char_data *ch, char *argument)
{
  int length, cmd;
  char *line;

  REMOVE_BIT (AFF_FLAGS (ch), AFF_HIDE);

  /* just drop to next line for hitting CR */
  skip_spaces (&argument);
  if (!*argument)
    return;

  /*
   * special case to handle one-character, non-alphanumeric commands;
   * requested by many people so "'hi" or ";godnet test" is possible.
   * Patch sent by Eric Green and Stefan Wasilewski.
   */
  if (!isalpha (*argument))
    {
      arg[0] = argument[0];
      arg[1] = '\0';
      line = argument + 1;
    }
  else
    line = any_one_arg (argument, arg);

  /* otherwise, find the command */
  if (GET_LEVEL (ch) < LVL_IMMORT)
    {
      int cont;			/* continue the command checks */
      cont = command_wtrigger (ch, arg, line);
      if (!cont)
	cont += command_mtrigger (ch, arg, line);
      if (!cont)
	cont = command_otrigger (ch, arg, line);
      if (cont)
	return;			/* command trigger took over */
    }

/* otherwise, find the command */
/* #ifdef CMD_TREE_SEARCH
  cmd = tree_cmd_search (arg, GET_LEVEL (ch));
#else */
  for (length = strlen (arg), cmd = 0; *complete_cmd_info[cmd].command != '\n'; cmd++)
    if (!strncmp (complete_cmd_info[cmd].command, arg, length))
      if (GET_LEVEL (ch) >= complete_cmd_info[cmd].minimum_level)
	break;
/* #endif */

  if (cmd == -1 || *complete_cmd_info[cmd].command == '\n')
    send_to_char ("Huh?!?\r\n", ch);

  else if (!IS_NPC (ch) && PLR_FLAGGED (ch, PLR_FROZEN)
	   && GET_LEVEL (ch) < LVL_IMPL)
    send_to_char ("You try, but the mind-numbing cold prevents you...\r\n",
		  ch);
  else if (complete_cmd_info[cmd].command_pointer == NULL)
    send_to_char ("Sorry, that command hasn't been implemented yet.\r\n", ch);
  else if (IS_NPC (ch) && complete_cmd_info[cmd].minimum_level >= LVL_IMMORT)
    send_to_char ("You can't use immortal commands while switched.\r\n", ch);
  else if (GET_POS (ch) < complete_cmd_info[cmd].minimum_position
	   && GET_LEVEL (ch) < LVL_IMMUNE)
    switch (GET_POS (ch))
      {
      case POS_DEAD:
	send_to_char ("Lie still; you are DEAD!!! :-(\r\n", ch);
	break;
      case POS_INCAP:
      case POS_MORTALLYW:
	send_to_char
	  ("You are in a pretty bad shape, unable to do anything!\r\n", ch);
	break;
      case POS_STUNNED:
	send_to_char
	  ("All you can do right now is think about the stars!\r\n", ch);
	break;
/*      case POS_FROZEN:
	send_to_char ("You've been frozen by an opponent!\r\n", ch);
	break;*/
      case POS_SLEEPING:
	send_to_char ("In your dreams, or what?\r\n", ch);
	break;
      case POS_RESTING:
	send_to_char ("Nah... You feel too relaxed to do that..\r\n", ch);
	break;
      case POS_SITTING:
	send_to_char ("Maybe you should get on your feet first?\r\n", ch);
	break;
      case POS_FIGHTING:
	send_to_char ("No way!  You're fighting for your life!\r\n", ch);
	break;
      }
  else if (no_specials || !special (ch, cmd, line))
    ((*complete_cmd_info[cmd].command_pointer) (ch, line, cmd, complete_cmd_info[cmd].subcmd));
}

/**************************************************************************
 * Routines to handle aliasing                                             *
  **************************************************************************/


struct alias_data *
find_alias (struct alias_data *alias_list, char *str)
{
  while (alias_list != NULL)
    {
      if (*str == *alias_list->alias)	/* hey, every little bit counts :-) */
	if (!strcmp (str, alias_list->alias))
	  return (alias_list);

      alias_list = alias_list->next;
    }

  return (NULL);
}


void
free_alias (struct alias_data *a)
{
  if (a->alias)
    free (a->alias);
  if (a->replacement)
    free (a->replacement);
  free (a);
}


/* The interface to the outside world: do_alias */
ACMD (do_alias)
{
  char *repl;
  struct alias_data *a, *temp;

  if (IS_NPC (ch))
    return;

  repl = any_one_arg (argument, arg);

  if (!*arg)
    {				/* no argument specified -- list currently defined aliases */
      send_to_char ("Currently defined aliases:\r\n", ch);
      if ((a = GET_ALIASES (ch)) == NULL)
	send_to_char (" None.\r\n", ch);
      else
	{
	  while (a != NULL)
	    {
	      sprintf (buf, "%-15s %s\r\n", a->alias, a->replacement);
	      send_to_char (buf, ch);
	      a = a->next;
	    }
	}
    }
  else
    {				/* otherwise, add or remove aliases */
      /* is this an alias we've already defined? */
      if ((a = find_alias (GET_ALIASES (ch), arg)) != NULL)
	{
	  REMOVE_FROM_LIST (a, GET_ALIASES (ch), next);
	  free_alias (a);
	}
      /* if no replacement string is specified, assume we want to delete */
      if (!*repl)
	{
	  if (a == NULL)
	    send_to_char ("No such alias.\r\n", ch);
	  else
	    send_to_char ("Alias deleted.\r\n", ch);
	}
      else
	{			/* otherwise, either add or redefine an alias */
	  if (!str_cmp (arg, "alias"))
	    {
	      send_to_char ("You can't alias 'alias'.\r\n", ch);
	      return;
	    }
	  CREATE (a, struct alias_data, 1);
	  a->alias = str_dup (arg);
	  delete_doubledollar (repl);
	  a->replacement = str_dup (repl);
	  if (strchr (repl, ALIAS_SEP_CHAR) || strchr (repl, ALIAS_VAR_CHAR))
	    a->type = ALIAS_COMPLEX;
	  else
	    a->type = ALIAS_SIMPLE;
	  a->next = GET_ALIASES (ch);
	  GET_ALIASES (ch) = a;
	  send_to_char ("Alias added.\r\n", ch);
	}
    }
}

/*
 * Valid numeric replacements are only $1 .. $9 (makes parsing a little
 * easier, and it's not that much of a limitation anyway.)  Also valid
 * is "$*", which stands for the entire original line after the alias.
 * ";" is used to delimit commands.
 */
#define NUM_TOKENS       9

void
perform_complex_alias (struct txt_q *input_q, char *orig,
		       struct alias_data *a)
{
  struct txt_q temp_queue;
  char *tokens[NUM_TOKENS], *temp, *write_point;
  int num_of_tokens = 0, num;

  /* First, parse the original string */
  temp = strtok (strcpy (buf2, orig), " ");
  while (temp != NULL && num_of_tokens < NUM_TOKENS)
    {
      tokens[num_of_tokens++] = temp;
      temp = strtok (NULL, " ");
    }

  /* initialize */
  write_point = buf;
  temp_queue.head = temp_queue.tail = NULL;

  /* now parse the alias */
  for (temp = a->replacement; *temp; temp++)
    {
      if (*temp == ALIAS_SEP_CHAR)
	{
	  *write_point = '\0';
	  buf[MAX_INPUT_LENGTH - 1] = '\0';
	  write_to_q (buf, &temp_queue, 1);
	  write_point = buf;
	}
      else if (*temp == ALIAS_VAR_CHAR)
	{
	  temp++;
	  if ((num = *temp - '1') < num_of_tokens && num >= 0)
	    {
	      strcpy (write_point, tokens[num]);
	      write_point += strlen (tokens[num]);
	    }
	  else if (*temp == ALIAS_GLOB_CHAR)
	    {
	      strcpy (write_point, orig);
	      write_point += strlen (orig);
	    }
	  else if ((*(write_point++) = *temp) == '$')	/* redouble $ for act safety */
	    *(write_point++) = '$';
	}
      else
	*(write_point++) = *temp;
    }

  *write_point = '\0';
  buf[MAX_INPUT_LENGTH - 1] = '\0';
  write_to_q (buf, &temp_queue, 1);

  /* push our temp_queue on to the _front_ of the input queue */
  if (input_q->head == NULL)
    *input_q = temp_queue;
  else
    {
      temp_queue.tail->next = input_q->head;
      input_q->head = temp_queue.head;
    }
}


/*
 * Given a character and a string, perform alias replacement on it.
 *
 * Return values:
 *   0: String was modified in place; call command_interpreter immediately.
 *   1: String was _not_ modified in place; rather, the expanded aliases
 *      have been placed at the front of the character's input queue.
 */
int
perform_alias (struct descriptor_data *d, char *orig)
{
  char first_arg[MAX_INPUT_LENGTH], *ptr;
  struct alias_data *a, *tmp;

  /* Mobs don't have alaises. */
  if (IS_NPC (d->character))
    return (0);

  /* bail out immediately if the guy doesn't have any aliases */
  if ((tmp = GET_ALIASES (d->character)) == NULL)
    return (0);

  /* find the alias we're supposed to match */
  ptr = any_one_arg (orig, first_arg);

  /* bail out if it's null */
  if (!*first_arg)
    return (0);

  /* if the first arg is not an alias, return without doing anything */
  if ((a = find_alias (tmp, first_arg)) == NULL)
    return (0);

  if (a->type == ALIAS_SIMPLE)
    {
      strcpy (orig, a->replacement);
      return (0);
    }
  else
    {
      perform_complex_alias (&d->input, ptr, a);
      return (1);
    }
}



/***************************************************************************
 * Various other parsing utilities                                         *
 **************************************************************************/

/*
 * searches an array of strings for a target string.  "exact" can be
 * 0 or non-0, depending on whether or not the match must be exact for
 * it to be returned.  Returns -1 if not found; 0..n otherwise.  Array
 * must be terminated with a '\n' so it knows to stop searching.
 */
int
search_block (char *arg, const char **list, int exact)
{
  register int i, l;

  /* Make into lower case, and get length of string */
  for (l = 0; *(arg + l); l++)
    *(arg + l) = LOWER (*(arg + l));

  if (exact)
    {
      for (i = 0; **(list + i) != '\n'; i++)
	if (!strcmp (arg, *(list + i)))
	  return (i);
    }
  else
    {
      if (!l)
	l = 1;			/* Avoid "" to match the first available
				 * string */
      for (i = 0; **(list + i) != '\n'; i++)
	if (!strncmp (arg, *(list + i), l))
	  return (i);
    }

  return (-1);
}


int
is_number (const char *str)
{
  while (*str)
    if (!isdigit (*(str++)))
      return (0);

  return (1);
}

/*
 * Function to skip over the leading spaces of a string.
 */
void
skip_spaces (char **string)
{
  for (; **string && isspace (**string); (*string)++);
}

void
strip_hyphan (char *string)
{
  for (; *string; (string)++)
    {
      if ((*string) == '-')
	(*string) = ' ';
    }
}

/*
 * Given a string, change all instances of double dollar signs ($$) to
 * single dollar signs ($).  When strings come in, all $'s are changed
 * to $$'s to avoid having users be able to crash the system if the
 * inputted string is eventually sent to act().  If you are using user
 * input to produce screen output AND YOU ARE SURE IT WILL NOT BE SENT
 * THROUGH THE act() FUNCTION (i.e., do_gecho, do_title, but NOT do_say),
 * you can call delete_doubledollar() to make the output look correct.
 *
 * Modifies the string in-place.
 */
char *
delete_doubledollar (char *string)
{
  char *read, *write;

  /* If the string has no dollar signs, return immediately */
  if ((write = strchr (string, '$')) == NULL)
    return (string);

  /* Start from the location of the first dollar sign */
  read = write;


  while (*read)			/* Until we reach the end of the string... */
    if ((*(write++) = *(read++)) == '$')	/* copy one char */
      if (*read == '$')
	read++;			/* skip if we saw 2 $'s in a row */

  *write = '\0';

  return (string);
}


int
fill_word (char *argument)
{
  return (search_block (argument, fill, TRUE) >= 0);
}


int
reserved_word (char *argument)
{
  return (search_block (argument, reserved, TRUE) >= 0);
}


/*
 * copy the first non-fill-word, space-delimited argument of 'argument'
 * to 'first_arg'; return a pointer to the remainder of the string.
 */
char *
one_argument (char *argument, char *first_arg)
{
  char *begin = first_arg;

  if (!argument)
    {
      log ("SYSERR: one_argument received a NULL pointer!");
      *first_arg = '\0';
      return (NULL);
    }

  do
    {
      skip_spaces (&argument);

      first_arg = begin;
      while (*argument && !isspace (*argument))
	{
	  *(first_arg++) = LOWER (*argument);
	  argument++;
	}

      *first_arg = '\0';
    }
  while (fill_word (begin));

  return (argument);
}


/*
 * one_word is like one_argument, except that words in quotes ("") are
 * considered one word.
 */
char *
one_word (char *argument, char *first_arg)
{
  char *begin = first_arg;

  do
    {
      skip_spaces (&argument);

      first_arg = begin;

      if (*argument == '\"')
	{
	  argument++;
	  while (*argument && *argument != '\"')
	    {
	      *(first_arg++) = LOWER (*argument);
	      argument++;
	    }
	  argument++;
	}
      else
	{
	  while (*argument && !isspace (*argument))
	    {
	      *(first_arg++) = LOWER (*argument);
	      argument++;
	    }
	}

      *first_arg = '\0';
    }
  while (fill_word (begin));

  return (argument);
}


/* same as one_argument except that it doesn't ignore fill words */
char *
any_one_arg (char *argument, char *first_arg)
{
  skip_spaces (&argument);

  while (*argument && !isspace (*argument))
    {
      *(first_arg++) = LOWER (*argument);
      argument++;
    }

  *first_arg = '\0';

  return (argument);
}


/*
 * Same as one_argument except that it takes two args and returns the rest;
 * ignores fill words
 */
char *
two_arguments (char *argument, char *first_arg, char *second_arg)
{
  return (one_argument (one_argument (argument, first_arg), second_arg));	/* :-) */
}



/*
 * determine if a given string is an abbreviation of another
 * (now works symmetrically -- JE 7/25/94)
 *
 * that was dumb.  it shouldn't be symmetrical.  JE 5/1/95
 * 
 * returnss 1 if arg1 is an abbreviation of arg2
 */
int
is_abbrev (const char *arg1, const char *arg2)
{
  if (!*arg1)
    return (0);

  for (; *arg1 && *arg2; arg1++, arg2++)
    if (LOWER (*arg1) != LOWER (*arg2))
      return (0);

  if (!*arg1)
    return (1);
  else
    return (0);
}


int
is_apart (const char *arg1, const char *arg2)
{
  int matchnum = 0;		/* how many letters have we matched */

  if (!*arg1 || !*arg2)
    return (0);

  while (*arg1 && *arg2)
    {
      if (LOWER (*arg1) != LOWER (*arg2))
	{
	  if (matchnum == 0)
	    arg2++;
	  else
	    {
	      while (matchnum > 0)
		{		/* need to backup and try again */
		  if (matchnum != 1)
		    arg2--;
		  arg1--;
		  matchnum--;
		}
	    }
	}
      else
	{
	  matchnum++;
	  arg1++;
	  arg2++;
	}
    }
  if (!*arg1 && !*arg2)
    return (1);
  else
    {
      if (!*arg2)
	return (0);
      else
	return (1);
    }
}

/* return first space-delimited token in arg1; remainder of string in arg2 */
void
half_chop (char *string, char *arg1, char *arg2)
{
  char *temp;

  temp = any_one_arg (string, arg1);
  skip_spaces (&temp);
  strcpy (arg2, temp);
}



/* Used in specprocs, mostly.  (Exactly) matches "command" to cmd number */
int
find_command (const char *command)
{
  int cmd;

  for (cmd = 0; *complete_cmd_info[cmd].command != '\n'; cmd++)
    if (!strcmp(complete_cmd_info[cmd].command, command))
      return (cmd);

  return (-1);
}


int
special (struct char_data *ch, int cmd, char *arg)
{
  register struct obj_data *i;
  register struct char_data *k;
  int j;

  /* special in room? */
  if (GET_ROOM_SPEC (ch->in_room) != NULL)
    if (GET_ROOM_SPEC (ch->in_room) (ch, world + ch->in_room, cmd, arg))
      return (1);

  /* special in equipment list? */
  for (j = 0; j < NUM_WEARS; j++)
    if (GET_EQ (ch, j) && GET_OBJ_SPEC (GET_EQ (ch, j)) != NULL)
      if (GET_OBJ_SPEC (GET_EQ (ch, j)) (ch, GET_EQ (ch, j), cmd, arg))
	return (1);

  /* special in inventory? */
  for (i = ch->carrying; i; i = i->next_content)
    if (GET_OBJ_SPEC (i) != NULL)
      if (GET_OBJ_SPEC (i) (ch, i, cmd, arg))
	return (1);

  /* special in mobile present? */

  for (k = world[ch->in_room].people; k; k = k->next_in_room)
    if (GET_MOB_SPEC (k) != NULL)
      if (GET_MOB_SPEC (k) (ch, k, cmd, arg))
	return (1);

  /* special in object present? */
  for (i = world[ch->in_room].contents; i; i = i->next_content)
    if (GET_OBJ_SPEC (i) != NULL)
      if (GET_OBJ_SPEC (i) (ch, i, cmd, arg))
	return (1);

  return (0);
}



/* *************************************************************************
*  Stuff for controlling the non-playing sockets (get name, pwd etc)       *
************************************************************************* */


/* locate entry in p_table with entry->name == name. -1 mrks failed search */
int
find_name (char *name)
{
  int i;

  for (i = 0; i <= top_of_p_table; i++)
    {
      if (!str_cmp ((player_table + i)->name, name))
	return (i);
    }

  return (-1);
}


int
_parse_name (char *arg, char *name)
{
  int i;

  /* skip whitespaces */
  for (; isspace (*arg); arg++);

  for (i = 0; (*name = *arg); arg++, i++, name++)
    if (!isalpha (*arg))
      return (1);

  if (!i)
    return (1);

  return (0);
}


#define RECON		1
#define USURP		2
#define UNSWITCH	3

/*
 * XXX: Make immortals 'return' instead of being disconnected when switched
 *      into person returns.  This function seems a bit over-extended too.
 */
int
perform_dupe_check (struct descriptor_data *d)
{
  struct descriptor_data *k, *next_k;
  struct char_data *target = NULL, *ch, *next_ch;
  int mode = 0;
  int pref_temp = 0;
  int id = GET_IDNUM (d->character);

  /*
   * Now that this descriptor has successfully logged in, disconnect all
   * other descriptors controlling a character with the same ID number.
   */

  for (k = descriptor_list; k; k = next_k)
    {
      next_k = k->next;

      if (k == d)
	continue;

      if (k->original && (GET_IDNUM (k->original) == id))
	{			/* switched char */
	  SEND_TO_Q ("\r\nMultiple login detected -- disconnecting.\r\n", k);
	  STATE (k) = CON_CLOSE;
	  pref_temp = GET_PREF (k->character);

	  if (!target)
	    {
	      target = k->original;
	      mode = UNSWITCH;
	    }
	  if (k->character)
	    k->character->desc = NULL;
	  k->character = NULL;
	  k->original = NULL;
	  pref_temp = GET_PREF (k->character);
	}
      else if (k->character && (GET_IDNUM (k->character) == id))
	{
	  if (!target && STATE (k) == CON_PLAYING)
	    {
	      SEND_TO_Q ("\r\nThis body has been usurped!\r\n", k);
	      target = k->character;
	      mode = USURP;
	    }
	  k->character->desc = NULL;
	  k->character = NULL;
	  k->original = NULL;
	  SEND_TO_Q ("\r\nMultiple login detected -- disconnecting.\r\n", k);
	  STATE (k) = CON_CLOSE;
	}
    }

  /*
   * now, go through the character list, deleting all characters that
   * are not already marked for deletion from the above step (i.e., in the
   * CON_HANGUP state), and have not already been selected as a target for
   * switching into.  In addition, if we haven't already found a target,
   * choose one if one is available (while still deleting the other
   * duplicates, though theoretically none should be able to exist).
   */

  for (ch = character_list; ch; ch = next_ch)
    {
      next_ch = ch->next;

      if (IS_NPC (ch))
	continue;
      if (GET_IDNUM (ch) != id)
	continue;

      /* ignore chars with descriptors (already handled by above step) */
      if (ch->desc)
	continue;

      /* don't extract the target char we've found one already */
      if (ch == target)
	continue;

      /* we don't already have a target and found a candidate for switching */
      if (!target)
	{
	  target = ch;
	  mode = RECON;
	  continue;
	}

      /* we've found a duplicate - blow him away, dumping his eq in limbo. */
      if (ch->in_room != NOWHERE)
	char_from_room (ch);
      char_to_room (ch, 1);
      extract_char (ch);
    }

  /* no target for swicthing into was found - allow login to continue */
  /* stupid-case rule for setting hostname on char:
     any time you set the char's pref .. */

  if (!target)
    {
      GET_HOST (d->character) = strdup (d->host);
      GET_PREF (d->character) = get_new_pref ();
      return 0;
    }
  else
    {
      if (GET_HOST (target))
	{
	  free (GET_HOST (target));
	}
      GET_HOST (target) = strdup (d->host);
      GET_PREF (target) = pref_temp;
      add_llog_entry (target, LAST_RECONNECT);
    }

  /* Okay, we've found a target.  Connect d to target. */
  free_char (d->character);	/* get rid of the old char */
  d->character = target;
  d->character->desc = d;
  d->original = NULL;
  d->character->char_specials.timer = 0;
  REMOVE_BIT (PLR_FLAGS (d->character), PLR_MAILING | PLR_WRITING);
  REMOVE_BIT (AFF_FLAGS (d->character), AFF_GROUP);
  STATE (d) = CON_PLAYING;

  switch (mode)
    {
    case RECON:
      SEND_TO_Q ("Reconnecting.\r\n", d);
      act ("$n has reconnected.", TRUE, d->character, 0, 0, TO_ROOM);
      sprintf (buf, "%s [%s] has reconnected.", GET_NAME (d->character),
	       d->host);
      mudlog (buf, NRM, MAX (LVL_IMMORT, GET_INVIS_LEV (d->character)), TRUE);
      break;
    case USURP:
      SEND_TO_Q ("You take over your own body, already in use!\r\n", d);
      if (PLR_FLAGGED
	  (d->character, PLR_DEAD | PLR_DEADI | PLR_DEADII | PLR_DEADIII))
	{
	  make_corpse (d->character);
	  extract_char (d->character);
	}
      act ("$n suddenly keels over in pain, surrounded by a white aura...\r\n"
	   "$n's body has been taken over by a new spirit!",
	   TRUE, d->character, 0, 0, TO_ROOM);
      sprintf (buf, "%s has re-logged in ... disconnecting old socket.",
	       GET_NAME (d->character));
      mudlog (buf, NRM, MAX (LVL_IMMORT, GET_INVIS_LEV (d->character)), TRUE);
      break;
    case UNSWITCH:
      SEND_TO_Q ("Reconnecting to unswitched char.", d);
      sprintf (buf, "%s [%s] has reconnected.", GET_NAME (d->character),
	       d->host);
      mudlog (buf, NRM, MAX (LVL_IMMORT, GET_INVIS_LEV (d->character)), TRUE);
      break;
    }
  return (1);
}

/* load the player, put them in the right room - used by copyover_recover too */
int
enter_player_game (struct descriptor_data *d)
{
  extern room_vnum mortal_start_room;
  extern sh_int r_immort_start_room;
  extern sh_int r_frozen_start_room;

  int load_result;
  sh_int load_room;
  struct clan_type *cptr = NULL;

  reset_char (d->character);
  read_aliases (d->character);
  read_saved_vars (d->character);

  if (PLR_FLAGGED (d->character, PLR_INVSTART))
    GET_INVIS_LEV (d->character) = GET_LEVEL (d->character);

  /* Check for new clans for leader */
  if (GET_CLAN (d->character) == PFDEF_CLAN)
    {
      for (cptr = clan_info; cptr; cptr = cptr->next)
	{
	  if (!strcmp (GET_NAME (d->character), cptr->leadersname))
	    GET_CLAN (d->character) = cptr->number;
	}
    }


  /* Now check to see if person's clan still exists */
  for (cptr = clan_info; cptr && cptr->number != GET_CLAN (d->character);
       cptr = cptr->next);

  if (cptr == NULL)
    {				/* Clan no longer exists */
      GET_CLAN (d->character) = PFDEF_CLAN;
      GET_CLAN_RANK (d->character) = PFDEF_CLANRANK;
      GET_HOME (d->character) = 1;
    }
  else
    {				/* Was there a change of leadership? */
      if (!strcmp (GET_NAME (d->character), cptr->leadersname))
	GET_CLAN_RANK (d->character) = CLAN_LEADER;
    }

  /*
   * We have to place the character in a room before equipping them
   * or equip_char() will gripe about the person in NOWHERE.
   */
  if ((load_room = GET_LOADROOM (d->character)) != NOWHERE)
    load_room = real_room (load_room);

  /* If char was saved with NOWHERE, or real_room above failed... */

  if (load_room == NOWHERE)
    {
      if (GET_LEVEL (d->character) >= LVL_IMMORT)
	load_room = r_immort_start_room;
      else
	load_room = real_room(mortal_start_room);
    }

  if (PLR_FLAGGED (d->character, PLR_FROZEN))
    load_room = r_frozen_start_room;

  if (GET_LEVEL (d->character) == 0)
    load_room = real_room (9083);	//New Star Room For New players

  d->character->next = character_list;
  character_list = d->character;
  char_to_room (d->character, load_room);
  load_result = Crash_load (d->character);

  if (d->character->player_specials->host)
    {
      free (d->character->player_specials->host);
      d->character->player_specials->host = NULL;
    }

  d->character->player_specials->host = str_dup (d->host);
  save_char (d->character, NOWHERE);

  return load_result;
}


void
roll_stats (struct descriptor_data *d)
{
  roll_real_abils (d->character);
  sprintf (buf,
	   "\r\n&WStrength:     [%2d] - Carrying Capacity, Hit Strength \r\n",
	   GET_STR (d->character));
  SEND_TO_Q (buf, d);
  sprintf (buf, "Intellegence: [%2d] - Mana Gain, Spell Success\r\n",
	   GET_INT (d->character));
  SEND_TO_Q (buf, d);
  sprintf (buf, "Wisdom:       [%2d] - Spell Success \r\n",
	   GET_WIS (d->character));
  SEND_TO_Q (buf, d);
  sprintf (buf, "Dexterity:    [%2d] - Evasion, Thief Skills, Move Gain\r\n",
	   GET_DEX (d->character));
  SEND_TO_Q (buf, d);
  sprintf (buf, "Constitution: [%2d] - HP Gain, Ressurection Chance \r\n",
	   GET_CON (d->character));
  SEND_TO_Q (buf, d);
  sprintf (buf,
	   "Charisma:     [%2d] - Mobile Agressivness, Charm abilities&n \r\n",
	   GET_CHA (d->character));
  SEND_TO_Q (buf, d);
  SEND_TO_Q ("\r\n\r\nKeep these stats? (y/N)", d);
}

/* deal with newcomers and other non-playing sockets */
void
nanny (struct descriptor_data *d, char *arg)
{
  char buf[128];		//int i;
  int player_i, load_result;
  //  char output[MAX_STRING_LENGTH];
  char tmp_name[MAX_INPUT_LENGTH];
  struct descriptor_data *dt = NULL;
  struct char_data *ch, *next_ch;


  /* OasisOLC states */
  struct
  {
    int state;
    void (*func) (struct descriptor_data *, char *);
  } olc_functions[] =
  {
    { CON_OEDIT, oedit_parse },
    { CON_ZEDIT, zedit_parse },
    { CON_SEDIT, sedit_parse },
    { CON_MEDIT, medit_parse },
    { CON_REDIT, redit_parse },
    { CON_ASSEDIT, assedit_parse },
    { CON_IEDIT, oedit_parse },
    { CON_TRIGEDIT, trigedit_parse },
    { CON_HEDIT, hedit_parse },
    { CON_CEDIT, cedit_parse },
    { CON_AEDIT, aedit_parse },
//  { CON_BEDIT, bedit_parse },
    { -1, NULL }
  };

  skip_spaces (&arg);

  /*
   * Quick check for the OLC states.
   */
  for (player_i = 0; olc_functions[player_i].state >= 0; player_i++)
    if (STATE (d) == olc_functions[player_i].state)
      {
	(*olc_functions[player_i].func) (d, arg);
	return;
      }

  if (d->character == NULL)
    {
      CREATE (d->character, struct char_data, 1);
      clear_char (d->character);
      CREATE (d->character->player_specials, struct player_special_data, 1);
      d->character->desc = d;
    }

  /* Not in OLC. */
  switch (STATE (d))
    {
    case CON_GET_NAME:
      if (!*arg || is_abbrev (arg, "quit"))
	STATE (d) = CON_CLOSE;
      else
	{
	  if (_parse_name (arg, tmp_name)
	      || strlen (tmp_name) < 2
	      || strlen (tmp_name) > MAX_NAME_LENGTH
	      || !Valid_Name (tmp_name)
	      || fill_word (strcpy (buf, tmp_name)) || reserved_word (buf))
	    {
	      SEND_TO_Q ("Invalid name, please try another.\r\n" "Name: ", d);
	      return;
	    }
	  if ((player_i = load_char (tmp_name, d->character)) > -1)
	    {
	      if (PRF_FLAGGED (d->character, PRF_COLOR_1))
		color = 1;
	      GET_PFILEPOS (d->character) = player_i;

	      if (PLR_FLAGGED (d->character, PLR_DELETED))
		{

		  /* make sure old files are removed in case of deleted player */
		  if ((player_i = find_name (tmp_name)) >= 0)
		    remove_player (player_i);

		  /* We get a false positive from the original deleted character. */
		  free_char (d->character);
		  /* Check for multiple creations... */
		  if (!Valid_Name (tmp_name))
		    {
		      SEND_TO_Q
			("Invalid name, please try another.\r\nName: ", d);
		      return;
		    }
		  CREATE (d->character, struct char_data, 1);
		  clear_char (d->character);
		  CREATE (d->character->player_specials,
			  struct player_special_data, 1);
		  d->character->desc = d;
		  CREATE (d->character->player.name, char,
			  strlen (tmp_name) + 1);
		  strcpy (d->character->player.name, CAP (tmp_name));
		  GET_PFILEPOS (d->character) = player_i;

		  if (color)
		    SET_BIT (PRF_FLAGS (d->character),
			     PRF_COLOR_1 | PRF_COLOR_2);
		  else
		    REMOVE_BIT (PRF_FLAGS (d->character),
				PRF_COLOR_1 | PRF_COLOR_2);

		  sprintf (buf,
			   "Create a new character by the name of %s (y/N)? ",
			   tmp_name);
		  SEND_TO_Q (buf, d);
		  STATE (d) = CON_NAME_CNFRM;
		}
	      else
		{
		  /* undo it just in case they are set */
		  REMOVE_BIT (PLR_FLAGS (d->character),
			      PLR_WRITING | PLR_MAILING | PLR_CRYO);
		  if (color)
		    SET_BIT (PRF_FLAGS (d->character),
			     PRF_COLOR_1 | PRF_COLOR_2);
		  else
		    REMOVE_BIT (PRF_FLAGS (d->character),
				PRF_COLOR_1 | PRF_COLOR_2);
		  REMOVE_BIT (AFF_FLAGS (d->character), AFF_GROUP);
		  SEND_TO_Q ("Password: ", d);
		  echo_off (d);
		  d->idle_tics = 0;
		  STATE (d) = CON_PASSWORD;
		}
	    }
	  else
	    {
	      /* player unknown -- make new character */

	      /* Check for multiple creations of a character. */
	      if (!Valid_Name (tmp_name))
					{
						SEND_TO_Q ("Invalid name, please try another.\r\nName: ",
											 d);
						return;
					}
	      CREATE (d->character->player.name, char, strlen (tmp_name) + 1);
	      strcpy (d->character->player.name, CAP (tmp_name));

	      sprintf (buf, "Did I get that right, %s (Y/N)? ", tmp_name);
	      SEND_TO_Q (buf, d);
	      STATE (d) = CON_NAME_CNFRM;
	    }
	}
      break;
    case CON_NAME_CNFRM:	/* wait for conf. of new name    */
      if (UPPER (*arg) == 'Y')
	{
	  if (isbanned (d->host) >= BAN_NEW)
	    {
	      sprintf (buf,
		       "Request for new char %s denied from [%s] (siteban)",
		       GET_PC_NAME (d->character), d->host);
	      mudlog (buf, NRM, LVL_GOD, TRUE);
	      SEND_TO_Q
		("Sorry, new characters are not allowed from your site!\r\n",
		 d);
	      STATE (d) = CON_CLOSE;
	      return;
	    }
	  if (circle_restrict)
	    {
	      SEND_TO_Q
		("Sorry, new players can't be created at the moment.\r\n", d);
	      sprintf (buf,
		       "Request for new char %s denied from [%s] (wizlock)",
		       GET_PC_NAME (d->character), d->host);
	      mudlog (buf, NRM, LVL_GOD, TRUE);
	      STATE (d) = CON_CLOSE;
	      return;
	    }
	  SEND_TO_Q ("New character.\r\n", d);
	  sprintf (buf, "Give me a password for %s: ",
		   GET_PC_NAME (d->character));
	  SEND_TO_Q (buf, d);
	  echo_off (d);
	  STATE (d) = CON_NEWPASSWD;
	}
      else if (*arg == 'n' || *arg == 'N')
	{
	  SEND_TO_Q ("Okay, what IS it, then? ", d);
	  free (d->character->player.name);
	  d->character->player.name = NULL;
	  STATE (d) = CON_GET_NAME;
	}
      else
	{
	  SEND_TO_Q ("Please type Yes or No: ", d);
	}
      break;
    case CON_PASSWORD:		/* get pwd for known player      */
      /*
       * To really prevent duping correctly, the player's record should
       * be reloaded from disk at this point (after the password has been
       * typed).  However I'm afraid that trying to load a character over
       * an already loaded character is going to cause some problem down the
       * road that I can't see at the moment.  So to compensate, I'm going to
       * (1) add a 15 or 20-second time limit for entering a password, and (2)
       * re-add the code to cut off duplicates when a player quits.  JE 6 Feb 96
       */

      echo_on (d);		/* turn echo back on */

      /* New echo_on() eats the return on telnet. Extra space better than none. */

      if (!*arg)
	STATE (d) = CON_CLOSE;
      else
	{
	  if (strncmp
	      (CRYPT (arg, GET_PASSWD (d->character)),
	       GET_PASSWD (d->character), MAX_PWD_LENGTH))
	    {
	      sprintf (buf, "Bad PW: %s [%s]", GET_NAME (d->character),
		       d->host);
	      mudlog (buf, BRF, LVL_GOD, TRUE);
	      GET_BAD_PWS (d->character)++;
	      save_char (d->character, NOWHERE);
	      save_player_index ();
	      if (++(d->bad_pws) >= max_bad_pws)
		{		/* 3 strikes and you're out. */
		  SEND_TO_Q ("Wrong password... disconnecting.\r\n", d);
		  STATE (d) = CON_CLOSE;
		}
	      else
		{
		  SEND_TO_Q ("Wrong password.\r\nPassword: ", d);
		  echo_off (d);
		}
	      return;
	    }

	  /* Password was correct. */
	  load_result = GET_BAD_PWS (d->character);
	  GET_BAD_PWS (d->character) = 0;
	  d->bad_pws = 0;

	  if (isbanned (d->host) == BAN_SELECT &&
	      !PLR_FLAGGED (d->character, PLR_SITEOK))
	    {
	      SEND_TO_Q
		("Sorry, this char has not been cleared for login from your site!\r\n",
		 d);
	      STATE (d) = CON_CLOSE;
	      sprintf (buf, "Connection attempt for %s denied from %s",
		       GET_NAME (d->character), d->host);
	      mudlog (buf, NRM, LVL_GOD, TRUE);
	      return;
	    }
	  if (GET_LEVEL (d->character) < circle_restrict)
	    {
	      SEND_TO_Q
		("The game is temporarily restricted.. try again later.\r\n",
		 d);
	      STATE (d) = CON_CLOSE;
	      sprintf (buf, "Request for login denied for %s [%s] (wizlock)",
		       GET_NAME (d->character), d->host);
	      mudlog (buf, NRM, LVL_GOD, TRUE);
	      return;
	    }
	  /* check and make sure no other copies of this player are logged in */
	  if (perform_dupe_check (d))
	    return;

	  sprintf (buf, "%s [%s] has connected.", GET_NAME (d->character),
		   d->host);
	  mudlog (buf, BRF, MAX (LVL_IMMORT, GET_INVIS_LEV (d->character)),
		  TRUE);

	  if (load_result)
	    {
	      sprintf (buf, "\r\n\r\n\007\007\007"
		       "%s%d LOGIN FAILURE%s SINCE LAST SUCCESSFUL LOGIN.%s\r\n",
		       CCRED (d->character, C_SPR), load_result,
		       (load_result > 1) ? "S" : "", CCNRM (d->character,
							    C_SPR));
	      SEND_TO_Q (buf, d);
	      GET_BAD_PWS (d->character) = 0;
	    }

	  for (dt = descriptor_list; dt; dt = dt->next)
	    {
	      //sprintf(buf, "%s(them) == %s(you)\r\n", dt->host, d->host);
	      //SEND_TO_Q(buf, d);
	      if (!d || !dt)
		break;
	      if (dt != d
		  && !str_cmp (dt->host, d->host)
		  && (!d->character || GET_LEVEL (d->character) < LVL_DEITY)
		  && (!dt->character
		      || GET_LEVEL (dt->character) < LVL_DEITY))
		{
		  if (!CANMULTIPLAY)
		    {
		      sprintf (buf, "%s Multiplay.", GET_NAME (d->character));
		      mudlog (buf, NRM, LVL_GOD, TRUE);
		      sprintf (buf,
			       "\r\n&RYou can't multiplay here. Sign off %s before logging on again.&n\r\n",
			       GET_NAME (dt->character));
		      SEND_TO_Q (buf, d);
		      STATE (d) = CON_CLOSE;
		    }
		  else
		    {
		      sprintf (buf, "%s Multiplay(Allowed).",
			       GET_NAME (d->character));
		      mudlog (buf, NRM, LVL_GOD, TRUE);
		      sprintf (buf,
			       "\r\n&RNormally you can't multiplay here, but for some reason you can right now.&n\r\n");
		      SEND_TO_Q (buf, d);
		    }
		  if (!dt->character)
		    STATE (dt) = CON_CLOSE;

		}
	    }

	  for (ch = character_list; ch; ch = next_ch)
	    {
	      next_ch = ch->next;
	      if (!d)
		break;

	      if (IS_NPC (ch))
		continue;
	      //if (GET_IDNUM(ch) != id) id not declared.
	      //  continue;
	      if (ch->desc)
		continue;
	      if (!ch->hostname || !d->host)
		continue;
	      if (str_cmp (ch->hostname, d->host) == 0
		  && (!d->character || GET_LEVEL (d->character) < LVL_DEITY)
		  && (GET_LEVEL (ch) < LVL_DEITY))
		{
		  if (!CANMULTIPLAY)
		    {
		      sprintf (buf, "%s Multiplay(linkless).",
			       GET_NAME (d->character));
		      mudlog (buf, NRM, LVL_GOD, TRUE);
		      sprintf (buf,
			       "You can't Multiplay while %s is linkless.\r\n",
			       GET_NAME (ch));
		      SEND_TO_Q (buf, d);
		      STATE (d) = CON_CLOSE;
		      if (!ch->desc
			  && (!PLR_FLAGGED (ch, PLR_ASSASSIN) || !ch->in_room
			      || GET_ROOM_VNUM (ch->in_room) <= 1))
			{
			  //Remove linkless character here if you want.
			  ch->char_specials.timer = idle_rent_time + 1;
			  check_idling (ch);
			}
		    }
		  else
		    {
		      sprintf (buf, "%s Multiplay(Allowed)",
			       GET_NAME (d->character));
		      mudlog (buf, NRM, LVL_GOD, TRUE);
		      sprintf (buf,
			       "\r\n&RNormally you can't multiplay here, but for some reason you can right now.&n\r\n");
		      SEND_TO_Q (buf, d);
		    }


		}
	    }


	  if (STATE (d) != CON_CLOSE)
	    {
	      add_llog_entry (d->character, LAST_CONNECT);
	      SEND_TO_Q (MENU, d);
	      STATE (d) = CON_MENU;
	    }

	}
      break;

    case CON_NEWPASSWD:
    case CON_CHPWD_GETNEW:
      if (!*arg || strlen (arg) > MAX_PWD_LENGTH || strlen (arg) < 3 ||
	  !str_cmp (arg, GET_PC_NAME (d->character)))
	{
	  SEND_TO_Q ("\r\nIllegal password.\r\n", d);
	  SEND_TO_Q ("Password: ", d);
	  return;
	}
      strncpy (GET_PASSWD (d->character),
	       CRYPT (arg, GET_PC_NAME (d->character)), MAX_PWD_LENGTH);
      *(GET_PASSWD (d->character) + MAX_PWD_LENGTH) = '\0';

      SEND_TO_Q ("\r\nPlease retype password: ", d);
      if (STATE (d) == CON_NEWPASSWD)
	STATE (d) = CON_CNFPASSWD;
      else
	STATE (d) = CON_CHPWD_VRFY;

      break;

    case CON_CNFPASSWD:
    case CON_CHPWD_VRFY:
      if (strncmp
	  (CRYPT (arg, GET_PASSWD (d->character)), GET_PASSWD (d->character),
	   MAX_PWD_LENGTH))
	{
	  SEND_TO_Q ("\r\nPasswords don't match... start over.\r\n", d);
	  SEND_TO_Q ("Password: ", d);
	  if (STATE (d) == CON_CNFPASSWD)
	    STATE (d) = CON_NEWPASSWD;
	  else
	    STATE (d) = CON_CHPWD_GETNEW;
	  return;
	}
      echo_on (d);

      if (STATE (d) == CON_CNFPASSWD)
	{
	  SEND_TO_Q ("\r\nWhat is your sex (M/F)? ", d);
	  STATE (d) = CON_QSEX;
	}
      else
	{
	  save_char (d->character, NOWHERE);
	  echo_on (d);
	  SEND_TO_Q ("\r\nDone.\r\n", d);
	  SEND_TO_Q (MENU, d);
	  STATE (d) = CON_MENU;
	}

      break;

    case CON_QSEX:		/* query sex of new user         */
      switch (*arg)
	{
	case 'm':
	case 'M':
	  d->character->player.sex = SEX_MALE;
	  break;
	case 'f':
	case 'F':
	  d->character->player.sex = SEX_FEMALE;
	  break;
	default:
	  SEND_TO_Q ("That is not a sex..\r\n" "What IS your sex? ", d);
	  return;
	}

      SEND_TO_Q (class_menu, d);
      SEND_TO_Q ("\r\nClass: ", d);
      STATE (d) = CON_QCLASS;
      break;

    case CON_QCLASS:
      load_result = parse_class (*arg);
      if (load_result == CLASS_UNDEFINED)
				{
					SEND_TO_Q ("\r\nThat's not a class.\r\nClass: ", d);
					return;
				}
      else
				GET_CLASS (d->character) = load_result;
			
      roll_stats (d);
      STATE (d) = CON_QROLLSTATS;
      break;
			
    case CON_QROLLSTATS:
      switch (*arg)
				{
				case 'y':
				case 'Y':
					break;
				case 'n':
				case 'N':
				default:
					roll_stats (d);
					return;
				}
			
      if (GET_PFILEPOS (d->character) < 0)
				GET_PFILEPOS (d->character) =
					create_entry (GET_PC_NAME (d->character));
      /* Now GET_NAME() will work properly. */
      init_char (d->character);
      save_char (d->character, NOWHERE);
      SEND_TO_Q (MENU, d);
      STATE (d) = CON_MENU;
      sprintf (buf, "%s [%s] new player.", GET_NAME (d->character), d->host);
      mudlog (buf, NRM, LVL_IMMORT, TRUE);
      break;
    case CON_MENU:		/* get selection from main menu  */
      switch (*arg)
	{
	case '0':
	  SEND_TO_Q ("Goodbye.\r\n", d);
	  STATE (d) = CON_CLOSE;
	  add_llog_entry (d->character, LAST_QUIT);
	  break;

	case '1':
	  load_result = enter_player_game (d);

	  if (d->character->player_specials->host)
	    {
	      free (d->character->player_specials->host);
	      d->character->player_specials->host = NULL;
	    }
	  d->character->player_specials->host = str_dup (d->host);
	  d->character->player.time.logon = time (0);
	  save_char (d->character, NOWHERE);


	  send_to_char (WELC_MESSG, d->character);

	  act ("$n has entered the game.", TRUE, d->character, 0, 0, TO_ROOM);

	  /* with the copyover patch, this next line goes in enter_player_game() */
	  read_saved_vars (d->character);

	  if (GET_ID (d->character) != 0)
	    {
	      greet_mtrigger (d->character, -1);
	      greet_memory_mtrigger (d->character);
	    }


	  STATE (d) = CON_PLAYING;

	  if (GET_LEVEL (d->character) == 0)
	    {
	      do_start (d->character);
	      send_to_char (START_MESSG, d->character);
	    }

	  look_at_room (d->character, 0);
	  if (has_mail (GET_IDNUM (d->character)))
	    send_to_char ("You have mail waiting.\r\n", d->character);
	  if (load_result == 2)
	    {			/* rented items lost */
	      send_to_char ("\r\n\007You could not afford your rent!\r\n"
			    "Your possesions have been donated to the Salvation Army!\r\n",
			    d->character);
	    }
	  d->has_prompt = 0;

	  /* Just in case.. to prevent building accidents */
	  REMOVE_BIT (PRF_FLAGS2 (d->character), PRF2_BUILDWALK);

	  break;

	case '2':
	  if (d->character->player.description)
	    {
	      SEND_TO_Q ("Current description:\r\n", d);
	      SEND_TO_Q (d->character->player.description, d);
	      d->backstr = str_dup (d->character->player.description);
	    }
	  SEND_TO_Q
	    ("Enter the new text you'd like others to see when they look at you.\r\n",
	     d);
	  SEND_TO_Q ("(/s saves /h for help)\r\n", d);
	  d->str = &d->character->player.description;
	  d->max_str = EXDSCR_LENGTH;
	  STATE (d) = CON_EXDESC;
	  break;

	case '3':
	  page_string (d, background, 0);
	  STATE (d) = CON_RMOTD;
	  break;

	case '4':
	  SEND_TO_Q ("\r\nEnter your old password: ", d);
	  echo_off (d);
	  STATE (d) = CON_CHPWD_GETOLD;
	  break;

	case '5':
	  SEND_TO_Q ("\r\nEnter your password for verification: ", d);
	  echo_off (d);
	  STATE (d) = CON_DELCNF1;
	  break;

	default:
	  SEND_TO_Q ("\r\nThat's not a menu choice!\r\n", d);
	  SEND_TO_Q (MENU, d);
	  break;
	}

      break;

    case CON_CHPWD_GETOLD:
      if (strncmp
	  (CRYPT (arg, GET_PASSWD (d->character)), GET_PASSWD (d->character),
	   MAX_PWD_LENGTH))
	{
	  echo_on (d);
	  SEND_TO_Q ("\r\nIncorrect password.\r\n", d);
	  SEND_TO_Q (MENU, d);
	  STATE (d) = CON_MENU;
	}
      else
	{
	  SEND_TO_Q ("\r\nEnter a new password: ", d);
	  STATE (d) = CON_CHPWD_GETNEW;
	}
      return;

    case CON_DELCNF1:
      echo_on (d);
      if (strncmp
	  (CRYPT (arg, GET_PASSWD (d->character)), GET_PASSWD (d->character),
	   MAX_PWD_LENGTH))
	{
	  SEND_TO_Q ("\r\nIncorrect password.\r\n", d);
	  SEND_TO_Q (MENU, d);
	  STATE (d) = CON_MENU;
	}
      else
	{
	  SEND_TO_Q
	    ("\r\nYOU ARE ABOUT TO DELETE THIS CHARACTER PERMANENTLY.\r\n"
	     "ARE YOU ABSOLUTELY SURE?\r\n\r\n"
	     "Please type \"yes\" to confirm: ", d);
	  STATE (d) = CON_DELCNF2;
	}
      break;

    case CON_DELCNF2:
      if (!strcmp (arg, "yes") || !strcmp (arg, "YES"))
	{
	  if (PLR_FLAGGED (d->character, PLR_FROZEN))
	    {
	      SEND_TO_Q
		("You try to kill yourself, but the ice stops you.\r\n", d);
	      SEND_TO_Q ("Character not deleted.\r\n\r\n", d);
	      STATE (d) = CON_CLOSE;
	      return;
	    }
	  if (GET_LEVEL (d->character) < LVL_GRGOD)
	    SET_BIT (PLR_FLAGS (d->character), PLR_DELETED);
	  save_char (d->character, NOWHERE);
	  Crash_delete_file (GET_NAME (d->character));
	  if (selfdelete_fastwipe)
	    if ((player_i = find_name (GET_NAME (d->character))) >= 0)
	      {
		SET_BIT (player_table[player_i].flags, PINDEX_SELFDELETE);
		remove_player (player_i);
	      }
	  sprintf (buf, "Character '%s' deleted!\r\n"
		   "Goodbye.\r\n", GET_NAME (d->character));
	  SEND_TO_Q (buf, d);
	  sprintf (buf, "%s (lev %d) has self-deleted.",
		   GET_NAME (d->character), GET_LEVEL (d->character));
	  mudlog (buf, NRM, LVL_GOD, TRUE);
	  STATE (d) = CON_CLOSE;
	  return;
	}
      else
	{
	  SEND_TO_Q ("\r\nCharacter not deleted.\r\n", d);
	  SEND_TO_Q (MENU, d);
	  STATE (d) = CON_MENU;
	}
      break;

      /*
       * It's possible, if enough pulses are missed, to kick someone off
       * while they are at the password prompt. We'll just defer to let
       * the game_loop() axe them.
       */
    case CON_CLOSE:
      break;

    case CON_LINE_INPUT:
      {
	if (d->callback)
	  {
	    d->callback (d, arg, d->c_data);
	    if (--(d->callback_depth) <= 0)
	      {
		d->callback = NULL;	// if the function wasn't chained, clean up
		d->callback_depth = 0;	// AND wasn't recursive
		d->c_data = NULL;
		STATE (d) = CON_PLAYING;
	      }
	  }
	else
	  {
	    log
	      ("SYSERR: No callback function specified for state CON_LINE_INPUT");
	    STATE (d) = CON_PLAYING;
	  }
      }
      break;

    case CON_ASSEDIT:
      assedit_parse (d, arg);
      break;

    default:
      log
	("SYSERR: Nanny: illegal state of con'ness (%d) for '%s'; closing connection.",
	 STATE (d), d->character ? GET_NAME (d->character) : "<unknown>");
      STATE (d) = CON_DISCONNECT;	/* Safest to do. */
      break;
    }
}
