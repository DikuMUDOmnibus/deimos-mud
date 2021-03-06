/* ************************************************************************
*   File: spec_assign.c                                 Part of CircleMUD *
*  Usage: Functions to assign function pointers to objs/mobs/rooms        *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "db.h"
#include "interpreter.h"
#include "utils.h"

extern int dts_are_dumps;
extern int mini_mud;
extern struct room_data *world;
extern struct index_data *mob_index;
extern struct index_data *obj_index;

SPECIAL(sholo);
SPECIAL(assemble);
SPECIAL(dump);
SPECIAL(pet_shops);
SPECIAL(postmaster);
SPECIAL(guild_guard);
SPECIAL(guild);
SPECIAL(puff);
SPECIAL(fido);
SPECIAL(janitor);
SPECIAL(mayor);
SPECIAL(snake);
SPECIAL(thief);
SPECIAL(magic_user);
SPECIAL(super_magic_user);
SPECIAL(warrior_class);
SPECIAL(thief_class);
SPECIAL(bank);
SPECIAL(gen_board);
SPECIAL(recharger);
SPECIAL(pop_dispenser);
SPECIAL(wiseman);
SPECIAL(meta_physician);
SPECIAL(pooky);
SPECIAL(level);
SPECIAL(assassin_guild);
SPECIAL(embalmer);
SPECIAL(thief_guild);
SPECIAL(warrior_guild);
SPECIAL(mage_guild);
SPECIAL(cleric_guild);
SPECIAL(butcher);
SPECIAL(bounty);
SPECIAL(stableshop);
SPECIAL(stable);
SPECIAL(gmaster);
SPECIAL(gm_zone);
SPECIAL(fear_maker);
SPECIAL(cussgiver);
SPECIAL(casino);
SPECIAL(troll);
SPECIAL(rental);
SPECIAL(warehouse);
SPECIAL(cestadeath);
SPECIAL(firestation);
SPECIAL(calise_hunter);
SPECIAL(maiden);
SPECIAL(newspaper_stand);
SPECIAL(gm_plaque);
SPECIAL(block_north);
SPECIAL(block_east);
SPECIAL(block_south);
SPECIAL(block_west);
SPECIAL(gm_guild);
SPECIAL(cap_flag);
SPECIAL(reincarnation);
SPECIAL(religion);
SPECIAL(clan_guard);
SPECIAL(slots);

/* local functions */

const struct specproc_info mob_procs[] = {
  {"NONE",              NULL,                   LVL_IMMORT},
  {"postmaster",        postmaster,             LVL_IMPL},
  {"fido",              fido,                   LVL_IMMORT},
  {"janitor",           janitor,                LVL_CREATOR},
  {"mayor",             mayor,                  LVL_DEITY},
  {"puff",              puff,                   LVL_IMPL},
  {"wiseman",           wiseman,                LVL_IMPL},
  {"meta_physician",    meta_physician,         LVL_IMPL},
  {"assassin_guild",    assassin_guild,         LVL_IMPL},
  {"embalmer",          embalmer,               LVL_DEITY},
  {"magic_user",        magic_user,             LVL_IMMORT},
  {"super_magic_user",  super_magic_user,       LVL_IMMORT},
  {"butcher",           butcher,                LVL_DEITY},
  {"bank",              bank,                   LVL_DEITY},
  {"calise_hunter",     calise_hunter,          LVL_IMPL},
  {"maiden",            maiden,                 LVL_IMPL},
  {"block_north",       block_north,            LVL_IMMORT},
  {"block_east",        block_east,             LVL_IMMORT},
  {"block_south",       block_south,            LVL_IMMORT},
  {"block_west",        block_west,             LVL_IMMORT},
  {"warrior_class",     warrior_class,          LVL_IMMORT},
  {"thief_class",       thief_class,            LVL_IMMORT},
  {"Troll",             troll,                  LVL_IMMORT},
  {"Fear-Maker",        fear_maker,             LVL_IMMORT},
  {"clan_guard",        clan_guard,             LVL_IMPL},
  {"gmzone",            gm_zone,                LVL_IMPL},
  {"\n",                0,                      0}
};

const struct specproc_info obj_procs[] = {
  {"NONE",              NULL,                   LVL_IMMORT},
  {"bank",              bank,                   LVL_DEITY},
  {"gen_board",         gen_board,              LVL_IMPL},
  {"recharger",         recharger,              LVL_DEITY},
  {"pop_dispenser",     pop_dispenser,          LVL_IMMORT},
  {"ankh",              reincarnation,          LVL_IMPL},
  {"gm_plaque",         gm_plaque,              LVL_IMPL},
	{"slot_machine",      slots,                  LVL_IMMORT},
  {"\n",                0,                      0}
};

const struct specproc_info room_procs[] = {
  {"NONE",              NULL,                   LVL_IMMORT},
  {"dump",              dump,                   LVL_IMMORT},
  {"pet_shop",          pet_shops,              LVL_IMPL},
  {"stableshop",        stableshop,             LVL_IMPL},
  {"stable",            stable,                 LVL_IMPL},
  {"casino",            casino,                 LVL_IMMORT},
  {"rental",            rental,                 LVL_IMPL},
  {"warehouse",         warehouse,              LVL_IMPL},
  {"firestation",       firestation,            LVL_IMPL},
  {"newspaper",         newspaper_stand,        LVL_IMPL},
  {"forge",             assemble,               LVL_IMPL},
  {"religion",          religion,               LVL_IMPL},
  {"sholo",             sholo,                  LVL_IMPL},

  {"gm_wall",           gmaster,                LVL_IMPL},
  {"cleric_guild",      cleric_guild,           LVL_IMPL},
  {"mage_guild",        mage_guild,             LVL_IMPL},
  {"warrior_guild",     warrior_guild,          LVL_IMPL},
  {"thief_guild",       thief_guild,            LVL_IMPL},
  {"rental",            rental,                 LVL_IMPL},
  {"gm_guild",          gm_guild,               LVL_IMPL},
  {"\n",                0,                      0}
};

void assign_mobiles(void);
void assign_objects(void);
void assign_rooms(void);
void ASSIGNROOM(room_vnum room, SPECIAL(fname));
void ASSIGNMOB(mob_vnum mob, SPECIAL(fname));
void ASSIGNOBJ(obj_vnum obj, SPECIAL(fname));

/* functions to perform assignments */
void ASSIGNMOB(mob_vnum mob, SPECIAL(fname))
{
  if (real_mobile(mob) >= 0) {
    mob_index[real_mobile(mob)].func = fname;
    if (get_spec_name(mob_procs, fname) == 0) {
      sprintf(buf, "SYSWAR: Assign spec not in table to mob #%d",
            mob);
      log(buf);

    }
  }
  else if (!mini_mud) {
    sprintf(buf, "SYSERR: Attempt to assign spec to non-existant mob #%d",
            mob);
    log(buf);
  }
}

void ASSIGNOBJ(obj_vnum obj, SPECIAL(fname))
{
  if (real_object(obj) >= 0) {
    obj_index[real_object(obj)].func = fname;
    if (get_spec_name(obj_procs, fname) == 0) {
      sprintf(buf, "SYSWAR: Assign spec not in table to obj #%d",
            obj);
      log(buf);

    }
  }
  else if (!mini_mud) {
    sprintf(buf, "SYSERR: Attempt to assign spec to non-existant obj #%d",
            obj);
    log(buf);
  }
}

void ASSIGNROOM(room_vnum room, SPECIAL(fname))
{
  if (real_room(room) >= 0) {
    world[real_room(room)].func = fname;
    if (get_spec_name(room_procs, fname) == 0) {
      sprintf(buf, "SYSWAR: Assign spec not in table to room #%d",
            room);
      log(buf);

    }
  }
  else if (!mini_mud) {
    sprintf(buf, "SYSERR: Attempt to assign spec to non-existant rm. #%d",
            room);
    log(buf);
  }
}

/* ********************************************************************
*  Assignments                                                        *
******************************************************************** */

/* assign special procedures to mobiles */
void assign_mobiles(void)
{

  ASSIGNMOB(1, puff);

}



/* assign special procedures to objects */
void assign_objects(void)
{
 // ASSIGNOBJ(3034, bank);	/* atm */
 // ASSIGNOBJ(3036, bank);	/* cashcard */
 // ASSIGNMOB(3035, bank);        /* Sarah, the bank teller */
}



/* assign special procedures to rooms */
void assign_rooms(void)
{
 // ASSIGNROOM(3030, dump);
 // ASSIGNROOM(3031, pet_shops);
  //ASSIGNROOM(256, stableshop);
  //ASSIGNROOM(255, stable);
  ASSIGNROOM(271, gmaster);
  ASSIGNROOM(276, mage_guild);
  ASSIGNROOM(258, warrior_guild);  
  ASSIGNROOM(260, cleric_guild);  
  ASSIGNROOM(298, thief_guild);  
  //ASSIGNROOM(200, casino); 
  ASSIGNROOM(1513, rental);
  ASSIGNROOM(1512, gm_guild);
}

room_rnum get_holo_room(room_vnum vnum)
{
/*      sprintf(buf, "SYSLOG: vnum holo : %d ",
            vnum);
      log(buf);
*/
    if  (1527 == vnum)
    {
        return real_room(1570);
    }
    if  (1528 == vnum)
    {
        return real_room(1571);
    }
    if  (1529 == vnum)
    {
        return real_room(1572);
    }
    if  (1530 == vnum)
    {
        return real_room(1573);
    }
    if  (1531 == vnum)
    {
        return real_room(1574);
    }

    return NOWHERE;
}

