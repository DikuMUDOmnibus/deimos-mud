/************************************************************************
 * OasisOLC - Objects / oedit.c					v2.0	*
 * Original author: Levork						*
 * Copyright 1996 by Harvey Gilpin					*
 * Copyright 1997-1999 by George Greer (greerga@circlemud.org)		*
 ************************************************************************/

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "comm.h"
#include "interpreter.h"
#include "spells.h"
#include "utils.h"
#include "db.h"
#include "boards.h"
#include "constants.h"
#include "shop.h"
#include "genolc.h"
#include "genobj.h"
#include "oasis.h"
#include "improved-edit.h"
#include "dg_olc.h"
#include "handler.h"

/*------------------------------------------------------------------------*/

/*
 * External variable declarations.
 */

extern struct obj_data *obj_proto;
extern struct index_data *obj_index;
extern struct obj_data *object_list;
extern obj_rnum top_of_objt;
extern struct zone_data *zone_table;
extern zone_rnum top_of_zone_table;
extern struct room_data *world;
extern struct shop_data *shop_index;
extern struct attack_hit_type attack_hit_text[];
extern struct spell_info_type spell_info[];
extern struct board_info_type board_info[];
extern struct descriptor_data *descriptor_list;
extern struct specproc_info obj_procs[];

/*------------------------------------------------------------------------*/

/*
 * Handy macros.
 */
#define S_PRODUCT(s, i) ((s)->producing[(i)])
/* Function externs */
void olc_disp_spec_proc_menu(struct descriptor_data * d,
  struct specproc_info table[]);
/*------------------------------------------------------------------------*\
  Utility and exported functions
\*------------------------------------------------------------------------*/

void oedit_setup_new(struct descriptor_data *d)
{
  CREATE(OLC_OBJ(d), struct obj_data, 1);

  clear_object(OLC_OBJ(d));
  OLC_OBJ(d)->name = str_dup("unfinished object");
  OLC_OBJ(d)->description = str_dup("An unfinished object is lying here.");
  OLC_OBJ(d)->short_description = str_dup("an unfinished object");
  GET_OBJ_WEAR(OLC_OBJ(d)) = ITEM_WEAR_TAKE;
  OLC_SPEC(d) = 0;
  OLC_VAL(d) = 0;
  OLC_ITEM_TYPE(d) = OBJ_TRIGGER;

  SCRIPT(OLC_OBJ(d)) = NULL;
  OLC_OBJ(d)->proto_script = OLC_SCRIPT(d) = NULL;

  oedit_disp_menu(d);
}

/*------------------------------------------------------------------------*/

void oedit_setup_existing(struct descriptor_data *d, int real_num)
{
  struct obj_data *obj;

  /*
   * Allocate object in memory.
   */
  CREATE(obj, struct obj_data, 1);
  copy_object(obj, &obj_proto[real_num]);

//  if (SCRIPT(obj))
//    script_copy(obj, &obj_proto[real_num], OBJ_TRIGGER);


  OLC_SPEC(d) = get_spec_name(obj_procs, obj_index[real_num].func);
  /*
   * Attach new object to player's descriptor.
   */
  OLC_OBJ(d) = obj;
  OLC_VAL(d) = 0;
  OLC_ITEM_TYPE(d) = OBJ_TRIGGER;
  dg_olc_script_copy(d);
  /* The edited obj must not have a script. It will be assigned to the updated
   * obj later, after editing. */
  SCRIPT(obj) = NULL;
  OLC_OBJ(d)->proto_script = NULL;

  oedit_disp_menu(d);
}

/*------------------------------------------------------------------------*/

void oedit_save_internally(struct descriptor_data *d)
{
  int i;
  obj_rnum robj_num;
  struct descriptor_data *dsc;
  struct obj_data *obj;

  i = (real_object(OLC_NUM(d)) == NOTHING);

  if ((robj_num = add_object(OLC_OBJ(d), OLC_NUM(d))) < 0) {
    log("oedit_save_internally: add_object failed.");
    return;
  }

  /* Update triggers and free old proto list  */
  if (obj_proto[robj_num].proto_script &&
      obj_proto[robj_num].proto_script != OLC_SCRIPT(d))
    free_proto_script(&obj_proto[robj_num], OBJ_TRIGGER);
  /* this will handle new instances of the object: */
  obj_proto[robj_num].proto_script = OLC_SCRIPT(d);

  /* this takes care of the objects currently in-game */
  for (obj = object_list; obj; obj = obj->next) {
    if (obj->item_number != robj_num)
      continue;
    /* remove any old scripts */
    if (SCRIPT(obj))
      extract_script(obj, OBJ_TRIGGER);

    free_proto_script(obj, OBJ_TRIGGER);
    copy_proto_script(&obj_proto[robj_num], obj, OBJ_TRIGGER);
    assign_triggers(obj, OBJ_TRIGGER);
  }
  /* end trigger update */


  if (!i)	/* If it's not a new object, don't renumber. */
    return;

  /*. Update spec procs .*/
  robj_num = real_object(OLC_NUM(d));
    obj_index[robj_num].func = obj_procs[OLC_SPEC(d)].sp_pointer;

  /*
   * Renumber produce in shops being edited.
   */
  for (dsc = descriptor_list; dsc; dsc = dsc->next)
    if (STATE(dsc) == CON_SEDIT)
      for (i = 0; S_PRODUCT(OLC_SHOP(dsc), i) != -1; i++)
	if (S_PRODUCT(OLC_SHOP(dsc), i) >= robj_num)
	  S_PRODUCT(OLC_SHOP(dsc), i)++;


  /* Update other people in zedit too. From: C.Raehl 4/27/99 */
  for (dsc = descriptor_list; dsc; dsc = dsc->next)
    if (STATE(dsc) == CON_ZEDIT)
      for (i = 0; OLC_ZONE(dsc)->cmd[i].command != 'S'; i++)
        switch (OLC_ZONE(dsc)->cmd[i].command) {
          case 'P':
            OLC_ZONE(dsc)->cmd[i].arg3 += (OLC_ZONE(dsc)->cmd[i].arg3 >= robj_num);
            /* Fall through. */
          case 'E':
          case 'G':
          case 'O':
            OLC_ZONE(dsc)->cmd[i].arg1 += (OLC_ZONE(dsc)->cmd[i].arg1 >= robj_num);
            break;
          case 'R':
            OLC_ZONE(dsc)->cmd[i].arg2 += (OLC_ZONE(dsc)->cmd[i].arg2 >= robj_num);
            break;
          default:
          break;
        }
}

/*------------------------------------------------------------------------*/

void oedit_save_to_disk(int zone_num)
{
  save_objects(zone_num);
}

/**************************************************************************
 Menu functions 
 **************************************************************************/

/*
 * For container flags.
 */
void oedit_disp_container_flags_menu(struct descriptor_data *d)
{
  get_char_colors(d->character);
  clear_screen(d);

  sprintbit(GET_OBJ_VAL(OLC_OBJ(d), 1), container_bits, buf1);
  sprintf(buf,
	  "%s1%s) CLOSEABLE\r\n"
	  "%s2%s) PICKPROOF\r\n"
	  "%s3%s) CLOSED\r\n"
	  "%s4%s) LOCKED\r\n"
	  "Container flags: %s%s%s\r\n"
	  "Enter flag, 0 to quit : ",
	  grn, nrm, grn, nrm, grn, nrm, grn, nrm, cyn, buf1, nrm);
  SEND_TO_Q(buf, d);
}

/*
 * For extra descriptions.
 */
void oedit_disp_extradesc_menu(struct descriptor_data *d)
{
  struct extra_descr_data *extra_desc = OLC_DESC(d);

  strcpy(buf1, !extra_desc->next ? "<Not set>\r\n" : "Set.");

  get_char_colors(d->character);
  clear_screen(d);
  sprintf(buf,
	  "Extra desc menu\r\n"
	  "%s1%s) Keyword: %s%s\r\n"
	  "%s2%s) Description:\r\n%s%s\r\n"
	  "%s3%s) Goto next description: %s\r\n"
	  "%s0%s) Quit\r\n"
	  "Enter choice : ",

     	  grn, nrm, yel, (extra_desc->keyword && *extra_desc->keyword) ? extra_desc->keyword : "<NONE>",
	  grn, nrm, yel, (extra_desc->description && *extra_desc->description) ? extra_desc->description : "<NONE>",
	  grn, nrm, buf1,
          grn, nrm
          );
  SEND_TO_Q(buf, d);
  OLC_MODE(d) = OEDIT_EXTRADESC_MENU;
}

/*
 * Ask for *which* apply to edit.
 */
void oedit_disp_prompt_apply_menu(struct descriptor_data *d)
{
  int counter;

  get_char_colors(d->character);
  clear_screen(d);

  for (counter = 0; counter < MAX_OBJ_AFFECT; counter++) {
    if (OLC_OBJ(d)->affected[counter].modifier) {
      sprinttype(OLC_OBJ(d)->affected[counter].location, apply_types, buf2);
      sprintf(buf, " %s%d%s) %+d to %s\r\n", grn, counter + 1, nrm,
	      OLC_OBJ(d)->affected[counter].modifier, buf2);
      SEND_TO_Q(buf, d);
    } else {
      sprintf(buf, " %s%d%s) None.\r\n", grn, counter + 1, nrm);
      SEND_TO_Q(buf, d);
    }
  }
  SEND_TO_Q("\r\nEnter affection to modify (0 to quit) : ", d);
  OLC_MODE(d) = OEDIT_PROMPT_APPLY;
}

/*
 * Ask for liquid type.
 */
void oedit_liquid_type(struct descriptor_data *d)
{
  int counter, columns = 0;

  get_char_colors(d->character);
  clear_screen(d);

  for (counter = 0; counter < NUM_LIQ_TYPES; counter++) {
    sprintf(buf, " %s%2d%s) %s%-20.20s %s", grn, counter, nrm, yel,
	    drinks[counter], !(++columns % 2) ? "\r\n" : "");
    SEND_TO_Q(buf, d);
  }
  sprintf(buf, "\r\n%sEnter drink type : ", nrm);
  SEND_TO_Q(buf, d);
  OLC_MODE(d) = OEDIT_VALUE_3;
}

/*
 * The actual apply to set.
 */
void oedit_disp_apply_menu(struct descriptor_data *d)
{
  int counter, columns = 0;

  get_char_colors(d->character);
  clear_screen(d);

  for (counter = 0; counter < NUM_APPLIES; counter++) {
    sprintf(buf, "%s%2d%s) %-20.20s %s", grn, counter, nrm,
		apply_types[counter], !(++columns % 2) ? "\r\n" : "");
    SEND_TO_Q(buf, d);
  }
  SEND_TO_Q("\r\nEnter apply type (0 is no apply) : ", d);
  OLC_MODE(d) = OEDIT_APPLY;
}

/*
 * Weapon type.
 */
void oedit_disp_weapon_menu(struct descriptor_data *d)
{
  int counter, columns = 0;

  get_char_colors(d->character);
  clear_screen(d);

  for (counter = 0; counter < NUM_ATTACK_TYPES; counter++) {
    sprintf(buf, "%s%2d%s) %-20.20s %s", grn, counter, nrm,
		attack_hit_text[counter].singular,
		!(++columns % 2) ? "\r\n" : "");
    SEND_TO_Q(buf, d);
  }
  SEND_TO_Q("\r\nEnter weapon type : ", d);
}

/*
 * Spell type.
 */
void oedit_disp_spells_menu(struct descriptor_data *d)
{
  int counter, columns = 0;

  get_char_colors(d->character);
  clear_screen(d);

  for (counter = 0; counter < NUM_SPELLS; counter++) {
    sprintf(buf, "%s%2d%s) %s%-20.20s %s", grn, counter, nrm, yel,
		spell_info[counter].name, !(++columns % 3) ? "\r\n" : "");
    SEND_TO_Q(buf, d);
  }
  sprintf(buf, "\r\n%sEnter spell choice (0 for none) : ", nrm);
  SEND_TO_Q(buf, d);
}

/*
 * Object value #1
 */
void oedit_disp_val1_menu(struct descriptor_data *d)
{
  OLC_MODE(d) = OEDIT_VALUE_1;
  switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
  case ITEM_LIGHT:
    /*
     * values 0 and 1 are unused.. jump to 2 
     */
    oedit_disp_val3_menu(d);
    break;
  case ITEM_SCROLL:
  case ITEM_WAND:
  case ITEM_STAFF:
  case ITEM_POTION:
    SEND_TO_Q("Spell level : ", d);
    break;
  case ITEM_WEAPON:
    /*
     * This doesn't seem to be used if I remembe right.
     */
    oedit_disp_val2_menu(d);
    //SEND_TO_Q("Modifier to Hitroll : ", d);
    break;
  case ITEM_ARMOR:
    SEND_TO_Q("Apply to AC : ", d);
    break;
  case ITEM_CONTAINER:
    SEND_TO_Q("Max weight to contain : ", d);
    break;
  case ITEM_DRINKCON:
  case ITEM_FOUNTAIN:
    SEND_TO_Q("Max drink units : ", d);
    break;
  case ITEM_FOOD:
    SEND_TO_Q("Hours to fill stomach : ", d);
    break;
  case ITEM_MONEY:
    SEND_TO_Q("Number of gold coins : ", d);
    break;
  case ITEM_NOTE:
    /*
     * This is supposed to be language, but it's unused.
     */
    oedit_disp_menu(d);
    break;
  case ITEM_PORTAL:
    SEND_TO_Q("Room VNUM to portal to: ", d);
    break;
  default:
    oedit_disp_menu(d);
  }
}

/*
 * Object value #2
 */
void oedit_disp_val2_menu(struct descriptor_data *d)
{
  OLC_MODE(d) = OEDIT_VALUE_2;
  switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
  case ITEM_SCROLL:
  case ITEM_POTION:
    oedit_disp_spells_menu(d);
    break;
  case ITEM_WAND:
  case ITEM_STAFF:
    SEND_TO_Q("Max number of charges : ", d);
    break;
  case ITEM_WEAPON:
    SEND_TO_Q("Number of damage dice : ", d);
    break;
  case ITEM_FOOD:
    /*
     * Values 2 and 3 are unused, jump to 4...Odd.
     */
    oedit_disp_val4_menu(d);
    break;
  case ITEM_CONTAINER:
    /*
     * These are flags, needs a bit of special handling.
     */
    oedit_disp_container_flags_menu(d);
    break;
  case ITEM_DRINKCON:
  case ITEM_FOUNTAIN:
    SEND_TO_Q("Initial drink units : ", d);
    break;
  default:
    oedit_disp_menu(d);
  }
}

/*
 * Object value #3
 */
void oedit_disp_val3_menu(struct descriptor_data *d)
{
  OLC_MODE(d) = OEDIT_VALUE_3;
  switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
  case ITEM_LIGHT:
    SEND_TO_Q("Number of hours (0 = burnt, -1 is infinite) : ", d);
    break;
  case ITEM_SCROLL:
  case ITEM_POTION:
    oedit_disp_spells_menu(d);
    break;
  case ITEM_WAND:
  case ITEM_STAFF:
    SEND_TO_Q("Number of charges remaining : ", d);
    break;
  case ITEM_WEAPON:
    SEND_TO_Q("Size of damage dice : ", d);
    break;
  case ITEM_CONTAINER:
    SEND_TO_Q("Vnum of key to open container (-1 for no key) : ", d);
    break;
  case ITEM_DRINKCON:
  case ITEM_FOUNTAIN:
    oedit_liquid_type(d);
    break;
  default:
    oedit_disp_menu(d);
  }
}

/*
 * Object value #4
 */
void oedit_disp_val4_menu(struct descriptor_data *d)
{
  OLC_MODE(d) = OEDIT_VALUE_4;
  switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
  case ITEM_SCROLL:
  case ITEM_POTION:
  case ITEM_WAND:
  case ITEM_STAFF:
    oedit_disp_spells_menu(d);
    break;
  case ITEM_WEAPON:
    oedit_disp_weapon_menu(d);
    break;
  case ITEM_DRINKCON:
  case ITEM_FOUNTAIN:
  case ITEM_FOOD:
    SEND_TO_Q("Poisoned (0 = not poison) : ", d);
    break;
  default:
    oedit_disp_menu(d);
  }
}

/*
 * Object type.
 */
void oedit_disp_type_menu(struct descriptor_data *d)
{
  int counter, columns = 0;

  get_char_colors(d->character);
  clear_screen(d);

  for (counter = 0; counter < NUM_ITEM_TYPES; counter++) {
    sprintf(buf, "%s%2d%s) %-20.20s %s", grn, counter, nrm,
		item_types[counter], !(++columns % 2) ? "\r\n" : "");
    SEND_TO_Q(buf, d);
  }
  SEND_TO_Q("\r\nEnter object type : ", d);
}

/*
 * Object extra flags.
 */
void oedit_disp_extra_menu(struct descriptor_data *d)
{
  int counter, columns = 0;

  get_char_colors(d->character);
  clear_screen(d);

  for (counter = 0; counter < NUM_ITEM_FLAGS; counter++) {
    sprintf(buf, "%s%2d%s) %-20.20s %s", grn, counter + 1, nrm,
		extra_bits[counter], !(++columns % 2) ? "\r\n" : "");
    SEND_TO_Q(buf, d);
  }
  sprintbit(GET_OBJ_EXTRA(OLC_OBJ(d)), extra_bits, buf1);
  sprintf(buf, "\r\nObject flags: %s%s%s\r\n"
	  "Enter object extra flag (0 to quit) : ",
	  cyn, buf1, nrm);
  SEND_TO_Q(buf, d);
}

/*
 * Object perm flags.
 */
void oedit_disp_perm_menu(struct descriptor_data *d)
{
  int counter, columns = 0;

  get_char_colors(d->character);
  clear_screen(d);

  for (counter = 0; counter < NUM_AFF_FLAGS; counter++) {
    sprintf(buf, "%s%2d%s) %-20.20s %s", grn, counter + 1, nrm, affected_bits[counter], !(++columns % 2) ? "\r\n" : "");
    SEND_TO_Q(buf, d);
  }
  sprintbit(GET_OBJ_PERM(OLC_OBJ(d)), affected_bits, buf1);
  sprintf(buf, "\r\nObject permanent flags: %s%s%s\r\n"
          "Enter object perm flag (0 to quit) : ", cyn, buf1, nrm);
  SEND_TO_Q(buf, d);
}

/*
 * Object wear flags.
 */
void oedit_disp_wear_menu(struct descriptor_data *d)
{
  int counter, columns = 0;

  get_char_colors(d->character);
  clear_screen(d);

  for (counter = 0; counter < NUM_ITEM_WEARS; counter++) {
    sprintf(buf, "%s%2d%s) %-20.20s %s", grn, counter + 1, nrm,
		wear_bits[counter], !(++columns % 2) ? "\r\n" : "");
    SEND_TO_Q(buf, d);
  }
  sprintbit(GET_OBJ_WEAR(OLC_OBJ(d)), wear_bits, buf1);
  sprintf(buf, "\r\nWear flags: %s%s%s\r\n"
	  "Enter wear flag, 0 to quit : ", cyn, buf1, nrm);
  SEND_TO_Q(buf, d);
}

/*
 * Display main menu.
 */
void oedit_disp_menu(struct descriptor_data *d)
{
  struct obj_data *obj;
  int value_zero_display =0;
  obj = OLC_OBJ(d);
  get_char_colors(d->character);
  clear_screen(d);

  /*
   * Build buffers for first part of menu.
   */
  sprinttype(GET_OBJ_TYPE(obj), item_types, buf1);
  sprintbit(GET_OBJ_EXTRA(obj), extra_bits, buf2);

  /*
   * Build first half of menu.
   */
  sprintf(buf,
	  "-- Item number : [%s%d%s]\r\n"
	  "%s1%s) Keywords : %s%s\r\n"
	  "%s2%s) S-Desc   : %s%s\r\n"
	  "%s3%s) L-Desc   :-\r\n%s%s\r\n"
	  "%s4%s) A-Desc   :-\r\n%s%s"
	  "%s5%s) Type        : %s%s\r\n"
	  "%s6%s) Extra flags : %s%s\r\n",

	  cyn, OLC_NUM(d), nrm,
	  grn, nrm, yel, (obj->name && *obj->name) ? obj->name : "undefined",
	  grn, nrm, yel, (obj->short_description && *obj->short_description) ? obj->short_description : "undefined",
	  grn, nrm, yel, (obj->description && *obj->description) ? obj->description : "undefined",
	  grn, nrm, yel, (obj->action_description && *obj->action_description) ? obj->action_description : "<not set>\r\n",
	  grn, nrm, cyn, buf1,
	  grn, nrm, cyn, buf2
	  );
  /*
   * Send first half.
   */
  SEND_TO_Q(buf, d);

  /*
   * Build second half of menu.
   */
  sprintbit(GET_OBJ_WEAR(obj), wear_bits, buf1);
  sprintbit(GET_OBJ_PERM(obj), affected_bits, buf2);

  if (GET_OBJ_TYPE(OLC_OBJ(d)) == ITEM_PORTAL)
    value_zero_display = GET_ROOM_VNUM(GET_OBJ_VAL(obj,0));
  else
    value_zero_display = GET_OBJ_VAL(obj,0);

  sprintf(buf,
	  "%s7%s) Wear flags  : %s%s\r\n"
	  "%s8%s) Weight      : %s%d\r\n"
	  "%s9%s) Cost        : %s%d\r\n"
	  "%sA%s) Cost/Day    : %s%d\r\n"
	  "%sB%s) Timer       : %s%d\r\n"
	  "%sC%s) Values      : %s%d %d %d %d\r\n"
	  "%sD%s) Applies menu\r\n"
	  "%sE%s) Extra descriptions menu\r\n"
          "%sM%s) Min Level   : %s%d\r\n"
          "%sP%s) Perm Affects: %s%s\r\n"
          "%sS%s) Script      : %s%s\r\n"
          "%sH%s) Spec-proc: %s%s\r\n"
	  "%sQ%s) Quit\r\n"
	  "Enter choice : ",

	  grn, nrm, cyn, buf1,
	  grn, nrm, cyn, GET_OBJ_WEIGHT(obj),
	  grn, nrm, cyn, GET_OBJ_COST(obj),
	  grn, nrm, cyn, GET_OBJ_RENT(obj),
	  grn, nrm, cyn, GET_OBJ_TIMER(obj),
	  grn, nrm, cyn, value_zero_display,
	  GET_OBJ_VAL(obj, 1),
	  GET_OBJ_VAL(obj, 2),
	  GET_OBJ_VAL(obj, 3),
	  grn, nrm, grn, nrm,
          grn, nrm, cyn, GET_OBJ_LEVEL(obj),
          grn, nrm, cyn, buf2,
          grn, nrm, cyn, OLC_SCRIPT(d)?"Set.":"Not Set.",
          grn, nrm, cyn, obj_procs[OLC_SPEC(d)].name,
          grn, nrm
  );
  SEND_TO_Q(buf, d);
  OLC_MODE(d) = OEDIT_MAIN_MENU;
}

/***************************************************************************
 main loop (of sorts).. basically interpreter throws all input to here
 ***************************************************************************/

void oedit_parse(struct descriptor_data *d, char *arg)
{
  int number, max_val, min_val, robj_num;
  char *oldtext = NULL;

  robj_num = real_object(OLC_NUM(d));

  switch (OLC_MODE(d)) {

  case OEDIT_CONFIRM_SAVESTRING:
    switch (*arg) {
    case 'y':
    case 'Y':
      if(STATE(d) == CON_OEDIT) {
        /*. Update spec procs .*/
        obj_index[robj_num].func = obj_procs[OLC_SPEC(d)].sp_pointer;
        oedit_save_internally(d);
        oedit_save_to_disk(real_zone_by_thing(OLC_NUM(d)));
        SEND_TO_Q("Object saved to disk.\r\n", d);

        sprintf(buf, "OLC: %s edits obj %d", GET_NAME(d->character), OLC_NUM(d));
        mudlog(buf, CMP, MAX(LVL_BUILDER, GET_INVIS_LEV(d->character)), TRUE);
        cleanup_olc(d, CLEANUP_STRUCTS);
      } else {  /* case is iedit */
        send_to_char("Commiting iedit changes.\r\n", d->character);
        *(OLC_IEDIT(d)) = *(OLC_OBJ(d));
  /* Xap - ought to save the old pointer, free after assignment I suppose */
        sprintf(buf, "OLC: %s uses iedit", GET_NAME(d->character));
        mudlog(buf, CMP, MAX(LVL_BUILDER, GET_INVIS_LEV(d->character)), TRUE);
        if (d->character) {
          REMOVE_BIT(PLR_FLAGS(d->character), PLR_WRITING);
          STATE(d) = CON_PLAYING;
          act("$n stops using OLC.", TRUE, d->character, 0, 0, TO_ROOM);
        }
        free(d->olc);
       }
        return;

      /* Fall through. */
    case 'n':
    case 'N':
      if(STATE(d) == CON_OEDIT) {
        /* If not saving, we must free the script_proto list. */
        OLC_OBJ(d)->proto_script = OLC_SCRIPT(d);
        free_proto_script(OLC_OBJ(d), OBJ_TRIGGER);
        cleanup_olc(d, CLEANUP_ALL);
      } else {
        if (d->character) {
          REMOVE_BIT(PLR_FLAGS(d->character), PLR_WRITING);
          STATE(d) = CON_PLAYING;
          act("$n stops using OLC.", TRUE, d->character, 0, 0, TO_ROOM);
        }
        free(d->olc);
      }
      return;
    default:
      SEND_TO_Q("Invalid choice!\r\nDo you wish to save your changes? : ", d);
      return;
    }

  case OEDIT_MAIN_MENU:
    /*
     * Throw us out to whichever edit mode based on user input.
     */
    switch (*arg) {
    case 'q':
    case 'Q':
      if (OLC_VAL(d)) {	/* Something has been modified. */
	SEND_TO_Q("Do you wish to save your changes? : ", d);
	OLC_MODE(d) = OEDIT_CONFIRM_SAVESTRING;
      } else
	cleanup_olc(d, CLEANUP_ALL);
      return;
    case '1':
      SEND_TO_Q("Enter namelist : ", d);
      OLC_MODE(d) = OEDIT_EDIT_NAMELIST;
      break;
    case '2':
      SEND_TO_Q("Enter short desc : ", d);
      OLC_MODE(d) = OEDIT_SHORTDESC;
      break;
    case '3':
      SEND_TO_Q("Enter long desc :-\r\n| ", d);
      OLC_MODE(d) = OEDIT_LONGDESC;
      break;
    case '4':
      OLC_MODE(d) = OEDIT_ACTDESC;
      send_editor_help(d);
      SEND_TO_Q("Enter action description:\r\n\r\n", d);
      if (OLC_OBJ(d)->action_description) {
	SEND_TO_Q(OLC_OBJ(d)->action_description, d);
	oldtext = str_dup(OLC_OBJ(d)->action_description);
      }
      string_write(d, &OLC_OBJ(d)->action_description, MAX_MESSAGE_LENGTH, 0, oldtext);
      OLC_VAL(d) = 1;
      break;
    case '5':
      oedit_disp_type_menu(d);
      OLC_MODE(d) = OEDIT_TYPE;
      break;
    case '6':
      oedit_disp_extra_menu(d);
      OLC_MODE(d) = OEDIT_EXTRAS;
      break;
    case '7':
      oedit_disp_wear_menu(d);
      OLC_MODE(d) = OEDIT_WEAR;
      break;
    case '8':
      SEND_TO_Q("Enter weight : ", d);
      OLC_MODE(d) = OEDIT_WEIGHT;
      break;
    case '9':
      SEND_TO_Q("Enter cost : ", d);
      OLC_MODE(d) = OEDIT_COST;
      break;
    case 'a':
    case 'A':
      SEND_TO_Q("Enter cost per day : ", d);
      OLC_MODE(d) = OEDIT_COSTPERDAY;
      break;
    case 'b':
    case 'B':
      SEND_TO_Q("Enter timer : ", d);
      OLC_MODE(d) = OEDIT_TIMER;
      break;
    case 'c':
    case 'C':
      /*
       * Clear any old values  
       */
      GET_OBJ_VAL(OLC_OBJ(d), 0) = 0;
      GET_OBJ_VAL(OLC_OBJ(d), 1) = 0;
      GET_OBJ_VAL(OLC_OBJ(d), 2) = 0;
      GET_OBJ_VAL(OLC_OBJ(d), 3) = 0;
      oedit_disp_val1_menu(d);
      break;
    case 'd':
    case 'D':
      oedit_disp_prompt_apply_menu(d);
      break;
    case 'e':
    case 'E':
      /*
       * If extra descriptions don't exist.
       */
      if (OLC_OBJ(d)->ex_description == NULL) {
	CREATE(OLC_OBJ(d)->ex_description, struct extra_descr_data, 1);
	OLC_OBJ(d)->ex_description->next = NULL;
      }
      OLC_DESC(d) = OLC_OBJ(d)->ex_description;
      oedit_disp_extradesc_menu(d);
      break;
    case 'm':
    case 'M':
      SEND_TO_Q("Enter new minimum level: ", d);
      OLC_MODE(d) = OEDIT_LEVEL;
      break;
    case 'p':
    case 'P':
      oedit_disp_perm_menu(d);
      OLC_MODE(d) = OEDIT_PERM;
      break;
    case 's':
    case 'S':
      OLC_SCRIPT_EDIT_MODE(d) = SCRIPT_MAIN_MENU;
      dg_script_menu(d);
      return;
    case 'h':
    case 'H':
      OLC_MODE(d) = OEDIT_SPEC_PROC;
      olc_disp_spec_proc_menu(d, obj_procs);
      return;
    default:
      oedit_disp_menu(d);
      break;
    }
    return;			/*
				 * end of OEDIT_MAIN_MENU 
				 */

  case OLC_SCRIPT_EDIT:
    if (dg_script_edit_parse(d, arg)) return;
    break;

  case OEDIT_EDIT_NAMELIST:
    if (!genolc_checkstring(d, arg))
      break;
    if (OLC_OBJ(d)->name)
      free(OLC_OBJ(d)->name);
    OLC_OBJ(d)->name = str_udup(arg);
    break;

  case OEDIT_SHORTDESC:
    if (!genolc_checkstring(d, arg))
      break;
    if (OLC_OBJ(d)->short_description)
      free(OLC_OBJ(d)->short_description);
    OLC_OBJ(d)->short_description = str_udup(arg);
    break;

  case OEDIT_LONGDESC:
    if (!genolc_checkstring(d, arg))
      break;
    if (OLC_OBJ(d)->description)
      free(OLC_OBJ(d)->description);
    OLC_OBJ(d)->description = str_udup(arg);
    break;

  case OEDIT_TYPE:
    number = atoi(arg);
    if ((number < 1) || (number >= NUM_ITEM_TYPES)) {
      SEND_TO_Q("Invalid choice, try again : ", d);
      return;
    } else
      GET_OBJ_TYPE(OLC_OBJ(d)) = number;
    break;

  case OEDIT_EXTRAS:
    number = atoi(arg);
    if ((number < 0) || (number > NUM_ITEM_FLAGS)) {
      oedit_disp_extra_menu(d);
      return;
    } else if (number == 0)
      break;
    else {
      TOGGLE_BIT(GET_OBJ_EXTRA(OLC_OBJ(d)), 1 << (number - 1));
      oedit_disp_extra_menu(d);
      return;
    }

  case OEDIT_WEAR:
    number = atoi(arg);
    if ((number < 0) || (number > NUM_ITEM_WEARS)) {
      SEND_TO_Q("That's not a valid choice!\r\n", d);
      oedit_disp_wear_menu(d);
      return;
    } else if (number == 0)	/* Quit. */
      break;
    else {
      TOGGLE_BIT(GET_OBJ_WEAR(OLC_OBJ(d)), 1 << (number - 1));
      oedit_disp_wear_menu(d);
      return;
    }

  case OEDIT_WEIGHT:
    GET_OBJ_WEIGHT(OLC_OBJ(d)) = atoi(arg);
    break;

  case OEDIT_COST:
    GET_OBJ_COST(OLC_OBJ(d)) = atoi(arg);
    break;

  case OEDIT_COSTPERDAY:
    GET_OBJ_RENT(OLC_OBJ(d)) = atoi(arg);
    break;

  case OEDIT_TIMER:
    GET_OBJ_TIMER(OLC_OBJ(d)) = atoi(arg);
    break;

  case OEDIT_LEVEL:
    GET_OBJ_LEVEL(OLC_OBJ(d)) = atoi(arg);
    break;

  case OEDIT_PERM:
    if ((number = atoi(arg)) == 0)
      break;
    if (number > 0 && number <= NUM_AFF_FLAGS)
      TOGGLE_BIT(GET_OBJ_PERM(OLC_OBJ(d)), 1 << (number - 1));
    oedit_disp_perm_menu(d);
    return;

  case OEDIT_SPEC_PROC:
    if (*arg == '\0') break;
    OLC_SPEC(d) = get_spec_proc_index(obj_procs, GET_LEVEL(d->character), atoi(arg));;
    break;

  case OEDIT_VALUE_1:
    /*
     * Lucky, I don't need to check any of these for out of range values.
     * Hmm, I'm not so sure - Rv  
     */

    if (GET_OBJ_TYPE(OLC_OBJ(d)) == ITEM_PORTAL)
    {
       if (VALID_RNUM(real_room(atoi(arg))))
         GET_OBJ_VAL(OLC_OBJ(d), 0) = real_room(atoi(arg));       
       else
         GET_OBJ_VAL(OLC_OBJ(d),0) = 0;
    }
    else
    {
       GET_OBJ_VAL(OLC_OBJ(d), 0) = atoi(arg);
    }
    /*
     * proceed to menu 2 
     */
    oedit_disp_val2_menu(d);
    return;
  case OEDIT_VALUE_2:
    /*
     * Here, I do need to check for out of range values.
     */
    number = atoi(arg);
    switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
    case ITEM_SCROLL:
    case ITEM_POTION:
      if (number < 0 || number >= NUM_SPELLS)
	oedit_disp_val2_menu(d);
      else {
	GET_OBJ_VAL(OLC_OBJ(d), 1) = number;
	oedit_disp_val3_menu(d);
      }
      break;
    case ITEM_CONTAINER:
      /*
       * Needs some special handling since we are dealing with flag values
       * here.
       */
      if (number < 0 || number > 4)
	oedit_disp_container_flags_menu(d);
      else if (number != 0) {
        TOGGLE_BIT(GET_OBJ_VAL(OLC_OBJ(d), 1), 1 << (number - 1));
        OLC_VAL(d) = 1;
	oedit_disp_val2_menu(d);
      } else
	oedit_disp_val3_menu(d);
      break;

    default:
      GET_OBJ_VAL(OLC_OBJ(d), 1) = number;
      oedit_disp_val3_menu(d);
    }
    return;

  case OEDIT_VALUE_3:
    number = atoi(arg);
    /*
     * Quick'n'easy error checking.
     */
    switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
    case ITEM_SCROLL:
    case ITEM_POTION:
      min_val = 0;
      max_val = NUM_SPELLS - 1;
      break;
    case ITEM_WEAPON:
      min_val = 1;
      max_val = 50;
      break;
    case ITEM_WAND:
    case ITEM_STAFF:
      min_val = 0;
      max_val = 20;
      break;
    case ITEM_DRINKCON:
    case ITEM_FOUNTAIN:
      min_val = 0;
      max_val = NUM_LIQ_TYPES - 1;
      break;
    default:
      min_val = -32000;
      max_val = 32000;
    }
    GET_OBJ_VAL(OLC_OBJ(d), 2) = LIMIT(number, min_val, max_val);
    oedit_disp_val4_menu(d);
    return;

  case OEDIT_VALUE_4:
    number = atoi(arg);
    switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
    case ITEM_SCROLL:
    case ITEM_POTION:
      min_val = 0;
      max_val = NUM_SPELLS - 1;
      break;
    case ITEM_WAND:
    case ITEM_STAFF:
      min_val = 1;
      max_val = NUM_SPELLS - 1;
      break;
    case ITEM_WEAPON:
      min_val = 0;
      max_val = NUM_ATTACK_TYPES - 1;
      break;
    default:
      min_val = -32000;
      max_val = 32000;
      break;
    }
    GET_OBJ_VAL(OLC_OBJ(d), 3) = LIMIT(number, min_val, max_val);
    break;

  case OEDIT_PROMPT_APPLY:
    if ((number = atoi(arg)) == 0)
      break;
    else if (number < 0 || number > MAX_OBJ_AFFECT) {
      oedit_disp_prompt_apply_menu(d);
      return;
    }
    OLC_VAL(d) = number - 1;
    OLC_MODE(d) = OEDIT_APPLY;
    oedit_disp_apply_menu(d);
    return;

  case OEDIT_APPLY:
    if ((number = atoi(arg)) == 0) {
      OLC_OBJ(d)->affected[OLC_VAL(d)].location = 0;
      OLC_OBJ(d)->affected[OLC_VAL(d)].modifier = 0;
      oedit_disp_prompt_apply_menu(d);
    } else if (number < 0 || number >= NUM_APPLIES)
      oedit_disp_apply_menu(d);
    else {
      OLC_OBJ(d)->affected[OLC_VAL(d)].location = number;
      SEND_TO_Q("Modifier : ", d);
      OLC_MODE(d) = OEDIT_APPLYMOD;
    }
    return;

  case OEDIT_APPLYMOD:
    OLC_OBJ(d)->affected[OLC_VAL(d)].modifier = atoi(arg);
    oedit_disp_prompt_apply_menu(d);
    return;

  case OEDIT_EXTRADESC_KEY:
    if (genolc_checkstring(d, arg)) {
      if (OLC_DESC(d)->keyword)
        free(OLC_DESC(d)->keyword);
      OLC_DESC(d)->keyword = str_udup(arg);
    }
    oedit_disp_extradesc_menu(d);
    return;

  case OEDIT_EXTRADESC_MENU:
    switch ((number = atoi(arg))) {
    case 0:
      if (!OLC_DESC(d)->keyword || !OLC_DESC(d)->description) {
	struct extra_descr_data **tmp_desc;

	if (OLC_DESC(d)->keyword)
	  free(OLC_DESC(d)->keyword);
	if (OLC_DESC(d)->description)
	  free(OLC_DESC(d)->description);

	/*
	 * Clean up pointers  
	 */
	for (tmp_desc = &(OLC_OBJ(d)->ex_description); *tmp_desc; tmp_desc = &((*tmp_desc)->next)) {
	  if (*tmp_desc == OLC_DESC(d)) {
	    *tmp_desc = NULL;
	    break;
	  }
	}
	free(OLC_DESC(d));
      }
    break;

    case 1:
      OLC_MODE(d) = OEDIT_EXTRADESC_KEY;
      SEND_TO_Q("Enter keywords, separated by spaces :-\r\n| ", d);
      return;

    case 2:
      OLC_MODE(d) = OEDIT_EXTRADESC_DESCRIPTION;
      send_editor_help(d);
      SEND_TO_Q("Enter the extra description:\r\n\r\n", d);
      if (OLC_DESC(d)->description) {
	SEND_TO_Q(OLC_DESC(d)->description, d);
	oldtext = str_dup(OLC_DESC(d)->description);
      }
      string_write(d, &OLC_DESC(d)->description, MAX_MESSAGE_LENGTH, 0, oldtext);
      OLC_VAL(d) = 1;
      return;

    case 3:
      /*
       * Only go to the next description if this one is finished.
       */
      if (OLC_DESC(d)->keyword && OLC_DESC(d)->description) {
	struct extra_descr_data *new_extra;

	if (OLC_DESC(d)->next)
	  OLC_DESC(d) = OLC_DESC(d)->next;
	else {	/* Make new extra description and attach at end. */
	  CREATE(new_extra, struct extra_descr_data, 1);
	  OLC_DESC(d)->next = new_extra;
	  OLC_DESC(d) = OLC_DESC(d)->next;
	}
      }
      /*
       * No break - drop into default case.
       */
    default:
      oedit_disp_extradesc_menu(d);
      return;
    }
    break;
  default:
    mudlog("SYSERR: OLC: Reached default case in oedit_parse()!", BRF, LVL_BUILDER, TRUE);
    SEND_TO_Q("Oops...\r\n", d);
    break;
  }

  /*
   * If we get here, we have changed something.  
   */
  OLC_VAL(d) = 1;
  oedit_disp_menu(d);
}

void oedit_string_cleanup(struct descriptor_data *d, int terminator)
{
  switch (OLC_MODE(d)) {
  case OEDIT_ACTDESC:
    oedit_disp_menu(d);
    break;
  case OEDIT_EXTRADESC_DESCRIPTION:
    oedit_disp_extradesc_menu(d);
    break;
  }
}
/* this is all iedit stuff */
void iedit_setup_existing(struct descriptor_data *d,
  struct obj_data *real_num) {
  struct obj_data *obj=NULL;

  OLC_IEDIT(d) = real_num;
  CREATE(obj, struct obj_data, 1);
  clear_object(obj);
  copy_object(obj,real_num);

  OLC_OBJ(d) = obj;
  OLC_IEDIT(d) = real_num;
  OLC_VAL(d) = 0;
  oedit_disp_menu(d);
}

ACMD(do_iedit) {
  struct obj_data *k;
  int found=0;
  int tmp;
  extern struct room_data *world;

  one_argument(argument, arg);

  if(!*arg || !*argument) {
    send_to_char("You must supply an object name.\r\n",ch);
  return;
  }

  if ((k = get_object_in_equip_vis(ch, arg, ch->equipment, &tmp))) {
    found=1;
  } else if ((k = get_obj_in_list_vis(ch, arg, ch->carrying))) {
    found=1;
  } else if ((k = get_obj_in_list_vis(ch, arg,
        world[ch->in_room].contents))) {
    found =1;
  } else if ((k = get_obj_vis(ch, arg))) {
    found=1;
  }

  if (!found) {
    send_to_char("Couldn't find that object. Sorry.\r\n",ch);
    return;
  }

                /* set up here */
  CREATE((ch->desc)->olc, struct oasis_olc_data, 1);
  SET_BIT(GET_OBJ_EXTRA(k), ITEM_UNIQUE_SAVE);

  SET_BIT(PLR_FLAGS(ch), PLR_WRITING);
  iedit_setup_existing(ch->desc,k);
  OLC_VAL(ch->desc) = 0;

  act("$n starts using OLC.", TRUE, ch, 0, 0, TO_ROOM);

  STATE(ch->desc) = CON_IEDIT;

  return;
}

