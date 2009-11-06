/* ************************************************************************
*   File: utils.h                                       Part of CircleMUD *
*  Usage: header file: utility macros and prototypes of utility funcs     *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */


/* external declarations and prototypes **********************************/

extern struct weather_data weather_info;
extern FILE *logfile;

#define log			basic_mud_log

/* public functions in utils.c */
int     can_murder(struct char_data *ch, struct char_data *victim);
char	*str_dup(const char *source);
int	str_cmp(const char *arg1, const char *arg2);
int	strn_cmp(const char *arg1, const char *arg2, int n);
void	basic_mud_log(const char *format, ...) __attribute__ ((format (printf, 1, 2)));
int	touch(const char *path);
void	mudlog(const char *str, int type, int level, int file);
void	showplayer(const char *str, int type, int level, int file);
void	log_death_trap(struct char_data *ch);
int	number(int from, int to);
int	dice(int number, int size);
void	sprintbit(bitvector_t vektor, const char *names[], char *result);
void	sprinttype(int type, const char *names[], char *result);
int	get_line(FILE *fl, char *buf);
int	get_filename(char *orig_name, char *filename, int mode);
struct time_info_data *age(struct char_data *ch);
int	num_pc_in_room(struct room_data *room);
void    line_input(struct descriptor_data *d, const char *prompt,
                  C_FUNC(*callback), void *info);
void    show_spec_proc_table(struct specproc_info table[], int level, char *result);
int     get_spec_proc_index(struct specproc_info table[], int level, int arg);
int     get_spec_proc(struct specproc_info table[], const char* arg);
int     get_spec_name(const struct specproc_info table[], SPECIAL(fname));
void    core_dump_real(const char *, int);
void	core_dump_real(const char *, int);
void    line_input(struct descriptor_data *d, const char *prompt,
		   C_FUNC(*callback), void *info);

void set_arank(struct char_data *killer, struct char_data *ch);
byte get_class_level(struct char_data *ch, byte class);

#define core_dump()		core_dump_real(__FILE__, __LINE__)

/* random functions in random.c */
void circle_srandom(unsigned long initial_seed);
unsigned long circle_random(void);

/* undefine MAX and MIN so that our functions are used instead */
#ifdef MAX
#undef MAX
#endif

#ifdef MIN
#undef MIN
#endif

int MAX(int a, int b);
int MIN(int a, int b);
char *CAP(char *txt);

#define LIMIT(var, low, high)	MIN(high, MAX(var, low))

/* in magic.c */
bool	circle_follow(struct char_data *ch, struct char_data * victim);

/* in act.informative.c */
void	look_at_room(struct char_data *ch, int mode);

/* in act.movmement.c */
int	do_simple_move(struct char_data *ch, int dir, int following);
int	perform_move(struct char_data *ch, int dir, int following);

/* in limits.c */
int	mana_gain(struct char_data *ch);
int	hit_gain(struct char_data *ch);
int	move_gain(struct char_data *ch);
void	advance_level(struct char_data *ch);
void    set_pretitle(struct char_data *ch, char *pretitle);
void	set_title(struct char_data *ch, char *title);
void	gain_exp(struct char_data *ch, int gain);
void	gain_condition(struct char_data *ch, int condition, int value);
void	check_idling(struct char_data *ch);
void	point_update(void);
void	harvest_ticker(void);
void	update_pos(struct char_data *victim);
void set_killer(struct char_data *victim, struct char_data *killer, int amt);
void    tracker(void);
void    goldchip(void);
void    othergoldchip(void);
void    check_fishing(void);

/* various constants *****************************************************/

/* Defines for multiplier for gold */
#define MULTI1_CONST (1.5)
#define MULTI2_CONST (2)


/* defines for mudlog() */
#define OFF	0
#define BRF	1
#define NRM	2
#define CMP	3

/* get_filename() */
#define CRASH_FILE	0
#define ETEXT_FILE	1
#define ALIAS_FILE	2
#define NEW_OBJ_FILES   3
#define SCRIPT_VARS_FILE 4
#define BACKUP_FILES   5


/* breadth-first searching */
#define BFS_ERROR		-1
#define BFS_ALREADY_THERE	-2
#define BFS_NO_PATH		-3

/*
 * XXX: These constants should be configurable. See act.informative.c
 *	and utils.c for other places to change.
 */
/* mud-life time */
#define SECS_PER_MUD_HOUR	70
#define SECS_PER_MUD_DAY	(24*SECS_PER_MUD_HOUR)
#define SECS_PER_MUD_MONTH	(35*SECS_PER_MUD_DAY)
#define SECS_PER_MUD_YEAR	(17*SECS_PER_MUD_MONTH)

/* real-life time (remember Real Life?) */
#define SECS_PER_REAL_MIN	60
#define SECS_PER_REAL_HOUR	(60*SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY	(24*SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR	(365*SECS_PER_REAL_DAY)

/* string utils **********************************************************/


#define YESNO(a) ((a) ? "YES" : "NO")
#define ONOFF(a) ((a) ? "ON" : "OFF")

#define LOWER(c)   (((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))
#define UPPER(c)   (((c)>='a'  && (c) <= 'z') ? ((c)+('A'-'a')) : (c) )

#define ISNEWL(ch) ((ch) == '\n' || (ch) == '\r') 
#define IF_STR(st) ((st) ? (st) : "\0")

#define AN(string) (strchr("aeiouAEIOU", *string) ? "an" : "a")


/* memory utils **********************************************************/

#define CREATE(result, type, number)  do {\
	if ((number) * sizeof(type) <= 0)	\
		log("SYSERR: Zero bytes or less requested at %s:%d.", __FILE__, __LINE__);	\
	if (!((result) = (type *) calloc ((number), sizeof(type))))	\
		{ perror("SYSERR: malloc failure"); abort(); } } while(0)

#define RECREATE(result,type,number) do {\
  if (!((result) = (type *) realloc ((result), sizeof(type) * (number))))\
		{ perror("SYSERR: realloc failure"); abort(); } } while(0)

/*
 * the source previously used the same code in many places to remove an item
 * from a list: if it's the list head, change the head, else traverse the
 * list looking for the item before the one to be removed.  Now, we have a
 * macro to do this.  To use, just make sure that there is a variable 'temp'
 * declared as the same type as the list to be manipulated.  BTW, this is
 * a great application for C++ templates but, alas, this is not C++.  Maybe
 * CircleMUD 4.0 will be...
 */
#define REMOVE_FROM_LIST(item, head, next)	\
   if ((item) == (head))		\
      head = (item)->next;		\
   else {				\
      temp = head;			\
      while (temp && (temp->next != (item))) \
	 temp = temp->next;		\
      if (temp)				\
         temp->next = (item)->next;	\
   }					\


/* basic bitvector utils *************************************************/
#define Q_FIELD(x)  ((int) (x) / 32)
#define Q_BIT(x)    (1 << ((x) % 32))
#define IS_SET_AR(var, bit)       ((var)[Q_FIELD(bit)] & Q_BIT(bit))
#define SET_BIT_AR(var, bit)      ((var)[Q_FIELD(bit)] |= Q_BIT(bit))
#define REMOVE_BIT_AR(var, bit)   ((var)[Q_FIELD(bit)] &= ~Q_BIT(bit))
#define TOGGLE_BIT_AR(var, bit)   ((var)[Q_FIELD(bit)] = (var)[Q_FIELD(bit)] ^ Q_BIT(bit))  
#define IS_SET(flag,bit)  ((flag) & (bit))
#define SET_BIT(var,bit)  ((var) |= (bit))
#define REMOVE_BIT(var,bit)  ((var) &= ~(bit))
#define TOGGLE_BIT(var,bit) ((var) = (var) ^ (bit))

/*
 * Accessing player specific data structures on a mobile is a very bad thing
 * to do.  Consider that changing these variables for a single mob will change
 * it for every other single mob in the game.  If we didn't specifically check
 * for it, 'wimpy' would be an extremely bad thing for a mob to do, as an
 * example.  If you really couldn't care less, change this to a '#if 0'.
 */
#if 0
/* Subtle bug in the '#var', but works well for now. */
#define CHECK_PLAYER_SPECIAL(ch, var) \
	(*(((ch)->player_specials == &dummy_mob) ? (log("SYSERR: Mob using '"#var"' at %s:%d.", __FILE__, __LINE__), &(var)) : &(var)))
#else
#define CHECK_PLAYER_SPECIAL(ch, var)	(var)
#endif

#define MOB_FLAGS(ch)	((ch)->char_specials.saved.act)
#define PLR_FLAGS(ch)	((ch)->char_specials.saved.act)
#define PLR_FLAGS2(ch)  ((ch)->char_specials.saved.act2) 
#define PRF_FLAGS(ch)  CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.pref))
#define PRF_FLAGS2(ch) CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.pref2))
#define AFF_FLAGS(ch)	((ch)->char_specials.saved.affected_by)
#define ROOM_FLAGS(loc)	(world[(loc)].room_flags)
#define SPELL_ROUTINES(spl)	(spell_info[spl].routines)

/* See http://www.circlemud.org/~greerga/todo.009 to eliminate MOB_ISNPC. */
#define IS_NPC(ch)	(IS_SET(MOB_FLAGS(ch), MOB_ISNPC))
#define IS_PC(ch)   (!IS_NPC(ch))

#define IS_MOB(ch)  (IS_NPC(ch) && GET_MOB_RNUM(ch) >= 0)

#define MOB_FLAGGED(ch, flag) (IS_NPC(ch) && IS_SET(MOB_FLAGS(ch), (flag)))
#define PLR_FLAGGED(ch, flag) (!IS_NPC(ch) && IS_SET(PLR_FLAGS(ch), (flag)))
#define MOB_FLAGGED2(ch, flag) (IS_NPC(ch) && IS_SET(PLR_FLAGS2(ch), (flag)))
#define PLR_FLAGGED2(ch, flag) (!IS_NPC(ch) && IS_SET(PLR_FLAGS2(ch), (flag)))
#define AFF_FLAGGED(ch, flag) (IS_SET(AFF_FLAGS(ch), (flag)))
#define PRF_FLAGGED(ch, flag) (IS_SET(PRF_FLAGS(ch), (flag)))
#define PRF_FLAGGED2(ch, flag) (IS_SET(PRF_FLAGS2(ch), (flag)))
#define ROOM_FLAGGED(loc, flag) (IS_SET(ROOM_FLAGS(loc), (flag)))
#define EXIT_FLAGGED(exit, flag) (IS_SET((exit)->exit_info, (flag)))
#define OBJVAL_FLAGGED(obj, flag) (IS_SET(GET_OBJ_VAL((obj), 1), (flag)))
#define OBJWEAR_FLAGGED(obj, flag) (IS_SET((obj)->obj_flags.wear_flags, (flag)))
#define OBJ_FLAGGED(obj, flag) (IS_SET(GET_OBJ_EXTRA(obj), (flag)))
#define HAS_SPELL_ROUTINE(spl, flag) (IS_SET(SPELL_ROUTINES(spl), (flag)))

/* IS_AFFECTED for backwards compatibility */
#define IS_AFFECTED(ch, skill) (AFF_FLAGGED((ch), (skill)))

#define PLR_TOG_CHK(ch,flag) ((TOGGLE_BIT(PLR_FLAGS(ch), (flag))) & (flag))
#define PLR_TOG_CHK2(ch,flag) ((TOGGLE_BIT(PLR_FLAGS2(ch), (flag))) & (flag)) 
#define PRF_TOG_CHK(ch,flag) ((TOGGLE_BIT(PRF_FLAGS(ch), (flag))) & (flag))
#define PRF_TOG_CHK2(ch,flag) ((TOGGLE_BIT(PRF_FLAGS2(ch), (flag))) & (flag))


/* room utils ************************************************************/


#define SECT(room)	(world[(room)].sector_type)

#define IS_DARK(room)  ( !world[room].light && \
                         (ROOM_FLAGGED(room, ROOM_DARK) || ROOM_FLAGGED(room, ROOM_PERFECTDARK)))
/*                          ((SECT(room) != SECT_INSIDE && \
                            SECT(room) != SECT_CITY  && \
                            SECT(room) != SECT_FIELD  && \
                            SECT(room) != SECT_HILLS  && \
                            SECT(room) != SECT_MOUNTAIN  && \
                            SECT(room) != SECT_WATER_SWIM  && \
                            SECT(room) != SECT_WATER_NOSWIM  && \
                            SECT(room) != SECT_UNDERWATER  && \
                            SECT(room) != SECT_FLYING ) && \
			    (weather_info.sunlight == SUN_SET || \
			     weather_info.sunlight == SUN_DARK)) ) )
*/
#define IS_LIGHT(room)  (!IS_DARK(room))

#define VALID_RNUM(rnum)	((rnum) >= 0 && (rnum) <= top_of_world)
#define GET_ROOM_VNUM(rnum) \
	((room_vnum)(VALID_RNUM(rnum) ? world[(rnum)].number : NOWHERE))
#define GET_ROOM_SPEC(room) (VALID_RNUM(room) ? world[(room)].func : NULL)

/* char utils ************************************************************/

#define IN_ROOM_VNUM(ch) (GET_ROOM_VNUM(IN_ROOM(ch)))
#define IN_ROOM(ch)	((ch)->in_room)
#define GET_WAS_IN(ch)	((ch)->was_in_room)
#define GET_AGE(ch)     (age(ch)->year)

#define GET_PC_NAME(ch)	((ch)->player.name)
#define GET_NAME(ch)    (IS_NPC(ch) ? (ch)->player.short_descr : GET_PC_NAME(ch))
#define SET_NAME(ch, value)    if (IS_NPC(ch)) { (ch)->player.short_descr = value; } else { GET_PC_NAME(ch) = value; } 
#define GET_TITLE(ch)   ((ch)->player.title)
#define GET_PRETITLE(ch) ((ch)->player.pretitle)
#define GET_LNAME(ch)    ((ch)->player.lastname)
#define IS_LORD(ch)      ((ch)->player.islord)
#define GET_GJOB(ch) ((ch)->player.gjob)
#define GET_GTITLE(ch) ((ch)->player.gtitle)

#define GET_LEVEL(ch)   ((ch)->player.level)

#define GET_THIEF_LEVEL(ch)  (IS_NPC(ch) ? (ch)->player.level : ((ch)->player.levelthief))
#define GET_WARRIOR_LEVEL(ch)(IS_NPC(ch) ? (ch)->player.level : ((ch)->player.levelwarrior))
#define GET_MAGE_LEVEL(ch)   (IS_NPC(ch) ? (ch)->player.level : ((ch)->player.levelmage))
#define GET_CLERIC_LEVEL(ch) (IS_NPC(ch) ? (ch)->player.level : ((ch)->player.levelcleric))

#define SET_THIEF_LEVEL(ch,value)   if (IS_NPC(ch)) { (ch)->player.level = value; } else { (ch)->player.levelthief = value;}
#define SET_WARRIOR_LEVEL(ch,value) if (IS_NPC(ch)) { (ch)->player.level = value; } else { (ch)->player.levelwarrior = value;}
#define SET_MAGE_LEVEL(ch,value)    if (IS_NPC(ch)) { (ch)->player.level = value; } else { (ch)->player.levelmage = value;}
#define SET_CLERIC_LEVEL(ch,value)  if (IS_NPC(ch)) { (ch)->player.level = value; } else { (ch)->player.levelcleric = value;}


#define GET_HORSELEVEL(ch)   ((ch)->player.levelhorse)
#define GET_PASSWD(ch)	 ((ch)->player.passwd)
#define GET_PFILEPOS(ch) ((ch)->pfilepos)
#define GET_TOTAL_LEVEL(ch) (((ch)->player.levelthief) + \
                            ((ch)->player.levelmage) +   \
                            ((ch)->player.levelcleric) + \
                            ((ch)->player.levelwarrior))

#define GET_GM_LEVEL(ch) ((ch)->player.levelgm)



// These two defines control every aspect of this game.
#define GET_MOB_EXP(ch)  (GET_LEVEL(ch) * GET_LEVEL(ch) * 1000)
#define GET_MOB_GOLD(ch) (GET_LEVEL(ch) * GET_LEVEL(ch) * 175)

#define MAX_BS_DAMAGE           1000
#define MAX_PC_DAMAGE           1000
#define MAX_NPC_DAMAGE          1400

#define RANDOMIZE_LEVEL         15
#define MAX_ARMOR               200
#define MIN_ARMOR               0


/*
 * I wonder if this definition of GET_REAL_LEVEL should be the definition
 * of GET_LEVEL?  JE
 */
#define GET_REAL_LEVEL(ch) \
   (ch->desc && ch->desc->original ? GET_LEVEL(ch->desc->original) : \
    GET_LEVEL(ch))

#define GET_CLASS(ch)   ((ch)->player.chclass)

#define GET_HOME(ch)	((ch)->player.hometown)
#define GET_HEIGHT(ch)	((ch)->player.height)
#define GET_WEIGHT(ch)	((ch)->player.weight)
#define GET_SEX(ch)	((ch)->player.sex)
#define GET_RELIGION(ch) ((ch)->player.religion)

#define GET_STR(ch)     ((ch)->aff_abils.str)
#define GET_DEX(ch)     ((ch)->aff_abils.dex)
#define GET_INT(ch)     ((ch)->aff_abils.intel)
#define GET_WIS(ch)     ((ch)->aff_abils.wis)
#define GET_CON(ch)     ((ch)->aff_abils.con)
#define GET_CHA(ch)     ((ch)->aff_abils.cha)

#define GET_EXP(ch)	  ((ch)->points.exp)
#define GET_HORSEEXP(ch)  ((ch)->points.horseexp)
#define GET_AC(ch)        ((ch)->points.armor)
#define GET_HIT(ch)	  ((ch)->points.hit)
#define GET_MAX_HIT(ch)	  ((ch)->points.max_hit)
#define GET_MOVE(ch)	  ((ch)->points.move)
#define GET_MAX_MOVE(ch)  ((ch)->points.max_move)
#define GET_MANA(ch)	  ((ch)->points.mana)
#define GET_MAX_MANA(ch)  ((ch)->points.max_mana)
#define GET_GOLD(ch)	  ((ch)->points.gold)
#define GET_BANK_GOLD(ch) ((ch)->points.bank_gold)
#define GET_HITROLL(ch)   ((ch)->points.hitroll)
#define GET_DAMROLL(ch)   ((ch)->points.damroll)
#define GET_HORSEHAPPY(ch)((ch)->points.horsehappy)
#define GET_MAXHAPPY(ch)  ((ch)->points.maxhappy)
#define GET_SPARRANK(ch)  ((ch)->points.sparrank)
#define GET_ECONRANK(ch)  ((ch)->points.econrank)
#define GET_ASSASSINRANK(ch)((ch)->points.assassinrank)
#define GET_CAREERRANK(ch)((ch)->points.careerrank)
#define GET_KILLS(ch)     ((ch)->points.kills)
#define GET_DEATHS(ch)    ((ch)->points.deaths)
#define GET_SACRIFICE(ch) ((ch)->points.sacrifices)

#define GET_POS(ch)	  ((ch)->char_specials.position)
#define GET_HINDEX(ch)	  ((ch)->char_specials.temp_help_index)
#define GET_TOP_HINDEX(ch)((ch)->char_specials.top_of_this_index)
#define GET_IDNUM(ch)	  ((ch)->char_specials.saved.idnum)
#define GET_ID(x)         ((x)->id)
#define IS_CARRYING_W(ch) ((ch)->char_specials.carry_weight)
#define IS_CARRYING_N(ch) ((ch)->char_specials.carry_items)
#define FIGHTING(ch)	  ((ch)->char_specials.fighting)
#define GET_KILLER(ch)	  ((ch)->char_specials.killer)
#define HUNTING(ch)	  ((ch)->char_specials.hunting)
#define TRACKING(ch)      ((ch)->player.tracking)
#define GET_SAVE(ch, i)	  ((ch)->char_specials.saved.apply_saving_throw[i])
#define GET_ALIGNMENT(ch) ((ch)->char_specials.saved.alignment)
#define GET_FREEZE_TEAM(ch) ((ch)->player_specials->saved.tag_team)

#define GET_COND(ch, i)		CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.conditions[(i)]))
#define GET_LOADROOM(ch)	CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.load_room))
#define GET_HORSEROOM(ch)	CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.horseroom))
#define GET_PRACTICES(ch)	CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.spells_to_learn))
#define GET_INVIS_LEV(ch)	CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.invis_level))
#define GET_WIMP_LEV(ch)	CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.wimp_level))
#define GET_FREEZE_LEV(ch)	CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.freeze_level))
#define GET_BAD_PWS(ch)		CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.bad_pws))
#define GET_TALK(ch, i)		CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.talks[i]))
#define POOFIN(ch)		CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->poofin))
#define POOFOUT(ch)		CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->poofout))
#define GET_LAST_OLC_TARG(ch)	CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->last_olc_targ))
#define GET_LAST_OLC_MODE(ch)	CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->last_olc_mode))
#define GET_ALIASES(ch)		CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->aliases))
#define GET_LAST_TELL(ch)	CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->last_tell))
#define GET_CLAN(ch)  		((ch)->player.clan)
#define GET_CLAN_RANK(ch)	((ch)->player.rank)
#define GET_CLAN_NAME(ch)       (((ch)->player.clan)->name)

#define GET_HORSENAME(ch) 	((ch)->player.horsename)
#define GET_HORSEEQ(ch)  	((ch)->player.horseeq)

#define GET_SKILL(ch, i)	CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.skills[i]))
#define SET_SKILL(ch, i, pct)	do { CHECK_PLAYER_SPECIAL((ch), (ch)->player_specials->saved.skills[i]) = pct; } while(0)

#define GET_EQ(ch, i)		((ch)->equipment[i])

#define GET_MOB_SPEC(ch)	(IS_MOB(ch) ? mob_index[(ch)->nr].func : NULL)
#define GET_MOB_SPEC_INDEX(ch)  (IS_MOB(ch) ? (get_spec_name(mob_procs, mob_index[(ch->nr)].func)) : 0)
#define SET_MOB_SPEC(ch, i)     {if (IS_MOB(ch)) mob_index[(ch->nr)].func = i;}
#define SET_MOB_SPEC_INDEX(ch, i) {if (IS_MOB(ch)) mob_index[(ch->nr)].func_index = i;}
#define GET_MOB_RNUM(mob)	((mob)->nr)
#define GET_MOB_VNUM(mob)	(IS_MOB(mob) ? \
				 mob_index[GET_MOB_RNUM(mob)].vnum : -1)

#define GET_DEFAULT_POS(ch)	((ch)->mob_specials.default_pos)
#define MEMORY(ch)		((ch)->mob_specials.memory)

#define STRENGTH_APPLY_INDEX(ch) (GET_STR(ch))
   

#define CAN_CARRY_W(ch) (50 * GET_STR(ch))
#define CAN_CARRY_N(ch) (GET_LEVEL(ch) > LVL_IMMORT ? 2000 : (5 + (GET_DEX(ch) >> 1) + (GET_LEVEL(ch) >> 1))) 
#define AWAKE(ch) (GET_POS(ch) > POS_SLEEPING)
#define CAN_SEE_IN_DARK(ch) \
    ((AFF_FLAGGED(ch, AFF_INFRAVISION) && !ROOM_FLAGGED(ch->in_room, ROOM_PERFECTDARK)) \
    || (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_HOLYLIGHT)))

#define IS_GOOD(ch)    (GET_ALIGNMENT(ch) >= 350)
#define IS_EVIL(ch)    (GET_ALIGNMENT(ch) <= -350)
#define IS_NEUTRAL(ch) (!IS_GOOD(ch) && !IS_EVIL(ch))

/* These three deprecated. */
#define WAIT_STATE(ch, cycle) do { GET_WAIT_STATE(ch) = (cycle); } while(0)
#define CHECK_WAIT(ch)                ((ch)->wait > 0)
#define GET_MOB_WAIT(ch)      GET_WAIT_STATE(ch)
/* New, preferred macro. */
#define GET_WAIT_STATE(ch)    ((ch)->wait)

/* Romance Macros */
#define ROMANCE(ch) ((ch)->player.romance)
#define PARTNER(ch) ((ch)->player.partner)
#define DPARTNER(ch) ((ch)->player.dpartner)

/* Dueling Macros */
#define DUEL(ch) ((ch)->player.duel)

#define GET_TETHER(ch, num) ((ch)->player.tether_room[num])

/* MatingMod Macros */

/* descriptor-based utils ************************************************/

/* Hrm, not many.  We should make more. -gg 3/4/99 */
#define STATE(d)	((d)->connected)


/* object utils **********************************************************/


#define GET_OBJ_TYPE(obj)	((obj)->obj_flags.type_flag)
#define GET_OBJ_COST(obj)	((obj)->obj_flags.cost)
#define GET_OBJ_RENT(obj)	((obj)->obj_flags.cost_per_day)
#define GET_OBJ_EXTRA(obj)	((obj)->obj_flags.extra_flags)
#define GET_OBJ_WEAR(obj)	((obj)->obj_flags.wear_flags)
#define GET_OBJ_VAL(obj, val)	((obj)->obj_flags.value[(val)])
#define GET_OBJ_WEIGHT(obj)	((obj)->obj_flags.weight)
#define GET_OBJ_TIMER(obj)	((obj)->obj_flags.timer)
#define GET_OBJ_LEVEL(obj)      ((obj)->obj_flags.level)
#define GET_OBJ_PERM(obj)       ((obj)->obj_flags.bitvector)
#define GET_OBJ_RNUM(obj)	((obj)->item_number)
#define GET_OBJ_VNUM(obj)	(GET_OBJ_RNUM(obj) >= 0 ? \
				 obj_index[GET_OBJ_RNUM(obj)].vnum : -1)
#define IS_OBJ_STAT(obj,stat)	(IS_SET((obj)->obj_flags.extra_flags,stat))
#define IS_CORPSE(obj)		(GET_OBJ_TYPE(obj) == ITEM_CONTAINER && \
					GET_OBJ_VAL((obj), 3) == 1)

#define GET_OBJ_SPEC(obj) ((obj)->item_number >= 0 ? \
	(obj_index[(obj)->item_number].func) : NULL)

#define CAN_WEAR(obj, part) (IS_SET((obj)->obj_flags.wear_flags, (part)))


/* compound utilities and other macros **********************************/

/*
 * Used to compute CircleMUD version. To see if the code running is newer
 * than 3.0pl13, you would use: #if _CIRCLEMUD > CIRCLEMUD_VERSION(3,0,13)
 */
#define CIRCLEMUD_VERSION(major, minor, patchlevel) \
	(((major) << 16) + ((minor) << 8) + (patchlevel))

#define HSHR(ch) (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "his":"her") :"its")
#define HESH(ch) (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "he" :"she") : "it")
#define HMHR(ch) (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "him":"her") : "it")

#define ANA(obj) (strchr("aeiouyAEIOUY", *(obj)->name) ? "An" : "A")
#define SANA(obj) (strchr("aeiouyAEIOUY", *(obj)->name) ? "an" : "a")


/* Various macros building up to CAN_SEE */

#define LIGHT_OK(sub)	(!AFF_FLAGGED(sub, AFF_BLIND) && \
   (IS_LIGHT((sub)->in_room) || AFF_FLAGGED((sub), AFF_INFRAVISION)))

#define INVIS_OK(sub, obj) \
 ((!AFF_FLAGGED((obj),AFF_INVISIBLE) || AFF_FLAGGED(sub,AFF_DETECT_INVIS)) && \
 (!AFF_FLAGGED((obj), AFF_HIDE) || AFF_FLAGGED(sub, AFF_SENSE_LIFE)))

#define MORT_CAN_SEE(sub, obj) (LIGHT_OK(sub) && INVIS_OK(sub, obj))

#define IMM_CAN_SEE(sub, obj) \
   (MORT_CAN_SEE(sub, obj) || (!IS_NPC(sub) && PRF_FLAGGED(sub, PRF_HOLYLIGHT)))

#define SELF(sub, obj)  ((sub) == (obj))

/* Can subject see character "obj"? */
#define CAN_SEE(sub, obj) (SELF(sub, obj) || \
   ((GET_REAL_LEVEL(sub) >= (IS_NPC(obj) ? 0 : GET_INVIS_LEV(obj))) && \
   IMM_CAN_SEE(sub, obj)))

/* End of CAN_SEE */


#define INVIS_OK_OBJ(sub, obj) \
  (!IS_OBJ_STAT((obj), ITEM_INVISIBLE) || AFF_FLAGGED((sub), AFF_DETECT_INVIS))

/* Is anyone carrying this object and if so, are they visible? */
#define CAN_SEE_OBJ_CARRIER(sub, obj) \
  ((!obj->carried_by || CAN_SEE(sub, obj->carried_by)) &&	\
   (!obj->worn_by || CAN_SEE(sub, obj->worn_by)))

#define MORT_CAN_SEE_OBJ(sub, obj) \
  (LIGHT_OK(sub) && INVIS_OK_OBJ(sub, obj) && CAN_SEE_OBJ_CARRIER(sub, obj))

#define CAN_SEE_OBJ(sub, obj) \
  ((!IS_NPC(sub) && PRF_FLAGGED((sub), PRF_HOLYLIGHT)) || MORT_CAN_SEE_OBJ(sub, obj))

#define CAN_CARRY_OBJ(ch,obj)  \
   (((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) <= CAN_CARRY_W(ch)) &&   \
    ((IS_CARRYING_N(ch) + 1) <= CAN_CARRY_N(ch)))

#define CAN_GET_OBJ(ch, obj)   \
   (CAN_WEAR((obj), ITEM_WEAR_TAKE) && CAN_CARRY_OBJ((ch),(obj)) && \
    CAN_SEE_OBJ((ch),(obj)))

#define PERS(ch, vict)   (CAN_SEE(vict, ch) ? GET_NAME(ch) : ( \
             GET_LEVEL(vict) > LVL_IMMORT ? (IS_NPC(vict) ? "someone" : "an Immortal") : "someone"))

#define OBJS(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
	((obj)->short_description == NULL ? "buggy" : (obj)->short_description)  : "something")

#define OBJN(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
	fname((obj)->name) : "something")


#define EXIT(ch, door)		(world[(ch)->in_room].dir_option[door])
#define _2ND_EXIT(ch, door)     (world[EXIT(ch, door)->to_room].dir_option[door])
#define _3RD_EXIT(ch, door) (world[_2ND_EXIT(ch, door)->to_room].dir_option[door])
#define W_EXIT(room, num)	(world[(room)].dir_option[(num)])
#define R_EXIT(room, num)	((room)->dir_option[(num)])

#define CAN_GO(ch, door) (EXIT(ch,door) && \
			 (EXIT(ch,door)->to_room != NOWHERE) && \
			 !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))


#define CLASS_ABBR(ch) (IS_NPC(ch) ? "--" : class_abbrevs[(int)GET_CLASS(ch)])

#define IS_MAGIC_USER(ch)	(!IS_NPC(ch) && \
				(GET_CLASS(ch) == CLASS_MAGIC_USER))
#define IS_CLERIC(ch)		(!IS_NPC(ch) && \
				(GET_CLASS(ch) == CLASS_CLERIC))
#define IS_THIEF(ch)		(!IS_NPC(ch) && \
				(GET_CLASS(ch) == CLASS_THIEF))
#define IS_WARRIOR(ch)		(!IS_NPC(ch) && \
				(GET_CLASS(ch) == CLASS_WARRIOR))

#define OUTSIDE(ch) (!ROOM_FLAGGED((ch)->in_room, ROOM_INDOORS))


/* OS compatibility ******************************************************/


/* there could be some strange OS which doesn't have NULL... */
#ifndef NULL
#define NULL (void *)0
#endif

#if !defined(FALSE)
#define FALSE 0
#endif

#if !defined(TRUE)
#define TRUE  (!FALSE)
#endif

/* defines for fseek */
#ifndef SEEK_SET
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2
#endif

/*
 * NOCRYPT can be defined by an implementor manually in sysdep.h.
 * CIRCLE_CRYPT is a variable that the 'configure' script
 * automatically sets when it determines whether or not the system is
 * capable of encrypting.
 */
#if defined(NOCRYPT) || !defined(CIRCLE_CRYPT)
#define CRYPT(a,b) (a)
#else
#define CRYPT(a,b) ((char *) crypt((a),(b)))
#endif

#define GET_PREF(ch) 		(ch->pref)
#define GET_HOST(ch)		(ch->hostname)
#define SENDOK(ch)	(((ch)->desc || SCRIPT_CHECK((ch), MTRIG_ACT)) && \
			(to_sleeping || AWAKE(ch)) && \
			!PLR_FLAGGED((ch), PLR_WRITING))

#define NUM_MOB_PROCS   (num_mob_procs)
#define NUM_OBJ_PROCS   (num_obj_procs)
#define NUM_ROOM_PROCS  (num_room_procs)
bool CAN_ASSASSIN(struct char_data *ch, struct char_data *victim);
bool CAN_STEAL(struct char_data *ch, struct char_data *victim);

room_rnum get_holo_room(room_rnum rnum);

