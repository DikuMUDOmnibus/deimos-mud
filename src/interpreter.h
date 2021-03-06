/* ************************************************************************
*   File: interpreter.h                                 Part of CircleMUD *
*  Usage: header file: public procs, macro defs, subcommand defines       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#define ACMD(name)  \
   void name(struct char_data *ch, char *argument, int cmd, int subcmd)

ACMD(do_move);

#define CMD_NAME (complete_cmd_info[cmd].command)
#define CMD_IS(cmd_name) (!strcmp(cmd_name, complete_cmd_info[cmd].command))
#define IS_MOVE(cmdnum) (complete_cmd_info[cmdnum].command_pointer == do_move)
/* #define CMD_TREE_SEARCH */

void	command_interpreter(struct char_data *ch, char *argument);
int	search_block(char *arg, const char **list, int exact);
char	lower( char c );
char	*one_argument(char *argument, char *first_arg);
char	*one_word(char *argument, char *first_arg);
char	*any_one_arg(char *argument, char *first_arg);
char	*two_arguments(char *argument, char *first_arg, char *second_arg);
int	fill_word(char *argument);
void	half_chop(char *string, char *arg1, char *arg2);
void	nanny(struct descriptor_data *d, char *arg);
int	is_apart(const char *arg1, const char *arg2);
int	is_abbrev(const char *arg1, const char *arg2);
int	is_number(const char *str);
int	find_command(const char *command);
void	skip_spaces(char **string);
void    strip_hyphan(char *string);
char	*delete_doubledollar(char *string);

/* for compatibility with 2.20: */
#define argument_interpreter(a, b, c) two_arguments(a, b, c)


struct command_info {
   const char *command;
   const char *sort_as;
   byte minimum_position;
   void	(*command_pointer)
	   (struct char_data *ch, char * argument, int cmd, int subcmd);
   sh_int minimum_level;
   int	subcmd;
};

/*
 * Necessary for CMD_IS macro.  Borland needs the structure defined first
 * so it has been moved down here.
 */
#ifndef __INTERPRETER_C__
extern struct command_info *complete_cmd_info;
#endif

ACMD(do_immcmd);
ACMD(do_wizhelp);
ACMD(do_question);
ACMD(do_action);
ACMD(do_advance);
ACMD(do_alias);
ACMD(do_affects);
ACMD(do_assemble);
ACMD(do_assedit);
ACMD(do_assist);
ACMD(do_at);
ACMD(do_autoall);
ACMD(do_backstab);
ACMD(do_ban);
ACMD(do_boom);
ACMD(do_bandage);
ACMD(do_bash);
ACMD(do_cast);
ACMD(do_castout);
ACMD(do_clan);
ACMD(do_chopwood);
ACMD(do_conspire);
ACMD(do_cuss);
ACMD(do_check);
ACMD(do_die);
ACMD(do_depiss);
ACMD(do_wiztog);
ACMD(do_push);
ACMD(do_pyoink);
ACMD(do_compare);
ACMD(do_deathtoall);
ACMD(do_color);
ACMD(do_copyover);
ACMD(do_compare);
ACMD(do_commands);
ACMD(do_copyto);
ACMD(do_consider);
ACMD(do_credits);
ACMD(do_date);
ACMD(do_dc);ACMD(do_date);
ACMD(do_dc);
// ACMD(do_default);
ACMD(do_diagnose);
ACMD(do_dig);
ACMD(do_disarm);
ACMD(do_display);
ACMD(do_dismount);
ACMD(do_drink);
ACMD(do_drop);
ACMD(do_doh);
ACMD(do_eat);
ACMD(do_echo);
ACMD(do_edit);
ACMD(do_enter);
ACMD(do_equipment);
ACMD(do_examine);
ACMD(do_exit);
ACMD(do_exits);ACMD(do_eyebrow);
ACMD(do_file);
ACMD(do_firefight);
ACMD(do_find);
ACMD(do_finger);
ACMD(do_flee);
ACMD(do_follow);
ACMD(do_force);
ACMD(do_gecho);
ACMD(do_gmote);  /* add this line */
ACMD(do_gen_comm);
ACMD(do_gen_door);
ACMD(do_gen_ps);
ACMD(do_gen_tog);
ACMD(do_gen_togx);
ACMD(do_gen_write);
ACMD(do_gen_vfile);ACMD(do_get);
ACMD(do_ghistory);
ACMD(do_thistory);
ACMD(do_give);
ACMD(do_gold);
ACMD(do_goto);
ACMD(do_grab);
ACMD(do_group);
ACMD(do_gsay);
ACMD(do_gtitle);
ACMD(do_gjob);
ACMD(do_gwho);
ACMD(do_haffinate);
/*ACMD(do_happystart);
ACMD(do_happyend);*/
ACMD(do_happytimes);
ACMD(do_hcontrol);
ACMD(do_help);
ACMD(do_helpcheck);
ACMD(do_hide);
ACMD(do_hit);
ACMD(do_house);
ACMD(do_hunt);ACMD(do_iedit);
ACMD(do_info);
ACMD(do_insult);
ACMD(do_inventory);
ACMD(do_invis);
ACMD(do_kick);
ACMD(do_kill);
ACMD(do_liblist);
ACMD(do_lag);
ACMD(do_last);
ACMD(do_leave);
ACMD(do_land);
ACMD(do_level);
ACMD(do_load);
ACMD(do_load_weapon);
ACMD(do_look);
ACMD(do_mine);
/* ACMD(do_move); -- interpreter.h */
ACMD(do_not_here);
ACMD(do_oasis);
ACMD(do_oarchy);
ACMD(do_olevel);
ACMD(do_odelete);
ACMD(do_offer);
/* ACMD(do_olc); */
ACMD(do_order);
ACMD(do_page);
ACMD(do_peace);
ACMD(do_repiss);
ACMD(do_poofin);
ACMD(do_poofout);
ACMD(do_pop);
ACMD(do_pour);
ACMD(do_practice);
ACMD(do_pretitle);
ACMD(do_purge);
ACMD(do_put);
ACMD(do_qcomm);
ACMD(do_quit);

ACMD(do_rdelete);
ACMD(do_reboot);
ACMD(do_recall);
ACMD(do_recharge);
ACMD(do_reelin);
ACMD(do_remove);
ACMD(do_reply);
ACMD(do_report);
ACMD(do_rent);
ACMD(do_rescue);
ACMD(do_rest);
ACMD(do_restore);
ACMD(do_return);
ACMD(do_ride);
ACMD(do_reimbursal);
ACMD(do_rewardall);
ACMD(do_roomlink);
ACMD(do_sacrifice);
ACMD(do_save);
ACMD(do_subscribe);
ACMD(do_saveall);
ACMD(do_say);
ACMD(do_score);
ACMD(do_scan);
ACMD(do_send);
ACMD(do_set);
ACMD(do_shade);
ACMD(do_shoot);
ACMD(do_show);
ACMD(do_shutdown);
ACMD(do_simulatecrash);
ACMD(do_sit);
ACMD(do_skill);
ACMD(do_skillthief);ACMD(do_skillmage);
ACMD(do_skillcleric);
ACMD(do_skillwarrior);
ACMD(do_skillset);
ACMD(do_skillreset);
ACMD(do_sleep);
ACMD(do_smartmud);
ACMD(do_sneak);
ACMD(do_snoop);
ACMD(do_sparrank);
ACMD(do_econrank);
ACMD(do_economyreset);
ACMD(do_spec_comm);
ACMD(do_split);
ACMD(do_stab);
ACMD(do_stand);
ACMD(do_stat);ACMD(do_statlist);
ACMD(do_steal);
ACMD(do_stop);
ACMD(do_switch);
ACMD(do_syslog);
ACMD(do_teleport);
ACMD(do_tell);
ACMD(do_time);
ACMD(do_title);
ACMD(do_tedit);
ACMD(do_tick);
ACMD(do_trip);
ACMD(do_toggle);
ACMD(do_track);
ACMD(do_trans);
ACMD(do_throw);
ACMD(do_tether);
ACMD(do_unban);ACMD(do_understand);
ACMD(do_undig);
ACMD(do_ungroup);
ACMD(do_upgrade);
ACMD(do_use);
ACMD(do_users);
ACMD(do_verify);
ACMD(do_visible);
ACMD(do_vnum);
ACMD(do_vstat);
ACMD(do_vlist);
ACMD(do_wake);
ACMD(do_wear);
ACMD(do_weather);
ACMD(do_where);
ACMD(do_who);
ACMD(do_whirlwind);ACMD(do_wield);
ACMD(do_wimpy);
ACMD(do_$);
ACMD(do_test);
ACMD(do_wizlock);
ACMD(do_wiznet);
ACMD(do_wizutil);
ACMD(do_write);
ACMD(do_xname);
ACMD(do_zreset);
ACMD(do_zclear);
ACMD(do_objconv);
ACMD(do_whoinvis);
ACMD(do_zsave);
ACMD(do_zdelete);
ACMD(do_zhelp);
/* DG Script ACMD's */ACMD(do_attach);
ACMD(do_detach);
ACMD(do_tlist);
ACMD(do_tstat);
ACMD(do_masound);
ACMD(do_mdamage);
ACMD(do_mdelete);
ACMD(do_mkill);
ACMD(do_mjunk);
ACMD(do_mdoor);
ACMD(do_mechoaround);
ACMD(do_mzoneecho);
ACMD(do_msend);
ACMD(do_mecho);
ACMD(do_mload);
ACMD(do_mpurge);
ACMD(do_mgoto);
ACMD(do_mat);
ACMD(do_mteleport);
ACMD(do_mforce);ACMD(do_mexp);
ACMD(do_mgold);
ACMD(do_mhunt);
ACMD(do_mremember);
ACMD(do_mforget);
ACMD(do_mtransform);
ACMD(do_mskillset);
ACMD(do_mteach);
ACMD(do_vdelete);
/* Runexecs */
ACMD(do_swho);
ACMD(do_slast);
ACMD(do_grep);
ACMD(do_sps);
ACMD(do_ukill);
ACMD(do_quota);
/* Romance Module */ACMD(do_askout);
ACMD(do_accept);
ACMD(do_reject);
ACMD(do_propose);
ACMD(do_breakup);
ACMD(do_marry);
ACMD(do_divorce);
/* End Romance Module (7 commands) */
/* MatingMod */
ACMD(do_agree);
ACMD(do_deny);
ACMD(mate_toggle);
/* End Mating Module (5 commands) *//* Some new shit I put in for no
particular point, bitch... */
/* I who? wheee! */
ACMD(do_addteam);
ACMD(do_startgame);
ACMD(do_endgame);
ACMD(do_tag);
ACMD(do_tsay);

// You know what I say Jon? DIEIDIDIDIDEI! Ryan
ACMD(do_challenge);
ACMD(do_acceptchallenge);
ACMD(do_rejectchallenge);

// RELIGION STUFFS
ACMD(do_atheist);
ACMD(do_phobos);
ACMD(do_deimos);
ACMD(do_calim);
ACMD(do_rulek);


/*
 * Alert! Changed from 'struct alias' to 'struct alias_data' in bpl15
 * because a Windows 95 compiler gives a warning about it having similiar
 * named member.
 */
struct alias_data {
  char *alias;
  char *replacement;
  int type;
  struct alias_data *next;
};

#define ALIAS_SIMPLE	0
#define ALIAS_COMPLEX	1

#define ALIAS_SEP_CHAR	';'
#define ALIAS_VAR_CHAR	'$'
#define ALIAS_GLOB_CHAR	'*'

/*
 * SUBCOMMANDS
 *   You can define these however you want to, and the definitions of the
 *   subcommands are independent from function to function.
 */

/* directions */
#define SCMD_NORTH	1
#define SCMD_EAST	2
#define SCMD_SOUTH	3
#define SCMD_WEST	4
#define SCMD_UP		5
#define SCMD_DOWN	6

/* do_gen_ps */
#define SCMD_INFO        0
#define SCMD_HANDBOOK    1 
#define SCMD_CREDITS     2
#define SCMD_NEWS        3
#define SCMD_WIZLIST     4
#define SCMD_POLICIES    5
#define SCMD_VERSION     6
#define SCMD_IMMLIST     7
#define SCMD_MOTD	 8
#define SCMD_IMOTD	 9
#define SCMD_CLEAR	 10
#define SCMD_WHOAMI	 11
#define SCMD_BUILDERCODE 12
#define SCMD_GMS         13

/* do_gen_tog */
#define SCMD_NOSUMMON   0
#define SCMD_NOHASSLE   1
#define SCMD_BRIEF      2
#define SCMD_COMPACT    3
#define SCMD_NOTELL	4
#define SCMD_NOAUCTION	5
#define SCMD_DEAF	6
#define SCMD_NOGOSSIP	7
#define SCMD_NOGRATZ	8
#define SCMD_NOWIZ	9
#define SCMD_QUEST	10
#define SCMD_ROOMFLAGS	11
#define SCMD_NOREPEAT	12
#define SCMD_HOLYLIGHT	13
#define SCMD_SLOWNS	14
#define SCMD_AUTOEXIT	15
#define SCMD_AUTOSPLIT  16
#define SCMD_AUTOLOOT   17
#define SCMD_TRACK	18
#define SCMD_AUTOASSIST 19
#define SCMD_AFK        20
#define SCMD_AUTODIAG   21
#define SCMD_XAP_OBJS   22
#define SCMD_AUTOGOLD   23
#define SCMD_ANON       24
#define SCMD_WHOINVIS   25
#define SCMD_AUTOSAC    26
#define SCMD_CONSPIRE   27

/* do_gen_togx  */
#define SCMD_AUTOAFFECTS 0
#define SCMD_AUTOTICK    1
#define SCMD_AUTOLOGON   2
#define SCMD_BUILDWALK   3
#define SCMD_HEARALLTELL 4
#define SCMD_CUSS        5
#define SCMD_LYRIC       6
#define SCMD_HEARLEVEL   7
#define SCMD_FULLEXIT    8
#define SCMD_NOGIVE      9

/* do_wizutil */
#define SCMD_REROLL	0
#define SCMD_PARDON     1
#define SCMD_NOTITLE    2
#define SCMD_SQUELCH    3
#define SCMD_FREEZE	4
#define SCMD_THAW	5
#define SCMD_UNAFFECT	6
#define SCMD_JAIL       7
#define SCMD_NORENT     8

/* Liblist */
#define SCMD_OLIST      0
#define SCMD_MLIST      1
#define SCMD_RLIST      2
#define SCMD_ZLIST      3

/* do_spec_com */
#define SCMD_WHISPER	0
#define SCMD_ASK	1

/* do_gen_com */
#define SCMD_HOLLER	0
#define SCMD_SHOUT	1
#define SCMD_GOSSIP	2
#define SCMD_AUCTION	3
#define SCMD_GRATZ	4
#define SCMD_GMOTE      5
#define SCMD_NEWBIE     6


/* do_shutdown */
#define SCMD_SHUTDOW	0
#define SCMD_SHUTDOWN   1

/* do_quit */
#define SCMD_QUI	0
#define SCMD_QUIT	1

/* do_date */
#define SCMD_DATE	0
#define SCMD_UPTIME	1

/* do_commands */
#define SCMD_COMMANDS	0
#define SCMD_SOCIALS	1
#define SCMD_WIZHELP	2

/* do_drop */
#define SCMD_DROP	0
#define SCMD_JUNK	1
#define SCMD_DONATE	2

/* do_gen_write */
#define SCMD_BUG	0
#define SCMD_TYPO	1
#define SCMD_IDEA	2

/* do_look */
#define SCMD_LOOK	0
#define SCMD_READ	1

/* do_qcomm */
#define SCMD_QSAY	0
#define SCMD_QECHO	1

/* do_pour */
#define SCMD_POUR	0
#define SCMD_FILL	1

/* do_poof */
#define SCMD_POOFIN	0
#define SCMD_POOFOUT	1

/* do_hit */
#define SCMD_HIT	0
#define SCMD_MURDER	1

/* do_eat */
#define SCMD_EAT	0
#define SCMD_TASTE	1
#define SCMD_DRINK	2
#define SCMD_SIP	3

/* do_use */
#define SCMD_USE	0
#define SCMD_QUAFF	1
#define SCMD_RECITE	2

/* do_echo */
#define SCMD_ECHO	0
#define SCMD_EMOTE	1

/* do_gen_door */
#define SCMD_OPEN       0
#define SCMD_CLOSE      1
#define SCMD_UNLOCK     2
#define SCMD_LOCK       3
#define SCMD_PICK       4

/*. do_olc .*/
#define SCMD_OASIS_REDIT	0
#define SCMD_OASIS_OEDIT	1
#define SCMD_OASIS_ZEDIT	2
#define SCMD_OASIS_MEDIT	3
#define SCMD_OASIS_SEDIT	4
#define SCMD_OASIS_TRIGEDIT     5
#define SCMD_OASIS_HEDIT        6
#define SCMD_OASIS_CEDIT        7
#define SCMD_OASIS_AEDIT  8
#define SCMD_OLC_SAVEINFO	9

 /* do_clan */
#define SCMD_CLAN_WHO      0
#define SCMD_CLAN_TELL     1
#define SCMD_CLAN_APPLY    2
#define SCMD_CLAN_REJECT   3
#define SCMD_CLAN_ACCEPT   4
#define SCMD_CLAN_DISMISS  5
#define SCMD_CLAN_WITHDRAW 6
#define SCMD_CLAN_RESIGN   7
#define SCMD_CLAN_LIST     8
#define SCMD_CLAN_PASSWORD 9
#define SCMD_CLAN_MEMBER   10
#define SCMD_CLAN_BALANCE  11
#define SCMD_CLAN_DEPOSIT  12
#define SCMD_CLAN_TAKEOUT  13

#define SCMD_THIEF   0 
#define SCMD_MAGE    1 
#define SCMD_CLERIC  2 
#define SCMD_WARRIOR 3 

/* * do_assemble * These constants *must* corespond with
     the ASSM_xxx constants in * assemblies.h. */
#define SCMD_ASSEMBLE	0

/* do_mskillset */
#define SCMD_SKILLSET   0
#define SCMD_TEACH	1

/* do_gen_vfile */
#define SCMD_V_BUGS 0
#define SCMD_V_IDEAS 1
#define SCMD_V_TYPOS 2

/* wiznet level specific */
#define SCMD_WIZARCHI    0
#define SCMD_WIZCREATOR  1
#define SCMD_WIZDEITY    2
#define SCMD_WIZCODER    3
#define SCMD_WIZCOIMP    4
#define SCMD_WIZIMP      5

/*Double tog */
#define SCMD_GOLD        0
#define SCMD_EXP         1
#define SCMD_CHIP        2
#define SCMD_QUESTON     3
#define SCMD_MULTIPLAY   4
#define SCMD_ASSASSIN    5

/* economy */
#define SCMD_UPDATEECON    0
#define SCMD_DISPLAYECON   1

/* Real Imm */
#define IMM_AT           1
#define IMM_ASSEDIT       2
#define IMM_BUILDWALK     3
#define IMM_CEDIT         4
#define IMM_DEATHTOALL    5
#define IMM_COPYTO        6
#define IMM_CANMULTI      7
#define IMM_CANASS        8
#define IMM_DATE          9
#define IMM_DC           10
#define IMM_DEPISS       11
#define IMM_DIG          12
#define IMM_DEXP         13
#define IMM_DGOLD        14
#define IMM_ECHO         15
#define IMM_FORCE        16
#define IMM_FILE         17
#define IMM_FIND         18
#define IMM_FREEZE       19
#define IMM_GECHO        20
#define IMM_GOLDCHIP     21
#define IMM_HAFF         22
#define IMM_HEARALL      23
#define IMM_HEDIT        24
#define IMM_HOUSE        25
#define IMM_IEDIT        26
#define IMM_INVIS        27
#define IMM_JAIL         28
#define IMM_LAG          29
#define IMM_LAST         30
#define IMM_LOAD         31
#define IMM_MEDIT        32
#define IMM_MUTE         33
#define IMM_NOTITLE      34
#define IMM_OLC          35
#define IMM_OEDIT        36
#define IMM_PAGE         37
#define IMM_PARDON       38
#define IMM_PEACE        39
#define IMM_POOFIN       40
#define IMM_POOFOUT      41
#define IMM_PRETITLE     42
#define IMM_PURGE        43
#define IMM_PYOINK       44
#define IMM_QECHO        45
#define IMM_QUESTON      46
#define IMM_REPISS       47
#define IMM_RELOAD       48
#define IMM_REROLL       49
#define IMM_RESTORE      50
#define IMM_REDIT        51
#define IMM_REWARDALL    52
#define IMM_SAVEALL      53
#define IMM_SEND         54
#define IMM_SET          55
#define IMM_SEDIT        56
#define IMM_SKILLSET     57
#define IMM_SKILLRESET   58
#define IMM_SNOOP        59
#define IMM_STAT         60
#define IMM_STATLIST     61
#define IMM_SWITCH       62
#define IMM_SYSLOG       63
#define IMM_TELEPORT     64
#define IMM_TEDIT        65
#define IMM_THAW         66
#define IMM_TRANSFER     67
#define IMM_TRIGEDIT     68
#define IMM_UNDERSTAND   69
#define IMM_UNDIG        70
#define IMM_UNAFFECT     71
#define IMM_UPTIME       72
#define IMM_USERS        73
#define IMM_VERIFY       74
#define IMM_VNUM         75
#define IMM_VSTAT        76
#define IMM_VLIST        77
#define IMM_WIZLOCK      78
//#define IMM_VNUM         79
#define IMM_ZEDIT        80
#define IMM_ZRESET       81
#define IMM_ZCLEAR       82
#define IMM_ZSAVE        83
#define IMM_ZHELP        84
#define IMM_OARCHY       85
#define IMM_OLEVEL       86




#define REALIMMONLY 999
