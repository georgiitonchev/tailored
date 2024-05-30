#include "engine/t_core.h"
#include "engine/t_input.h"
#include "engine/t_sprite.h"
#include "engine/t_ui.h"
#include "screens.h"
#include "./engine/tailored.h"

#include "game.h"
#include "cJSON.h"

#define CELL_SIZE 48

#define H_GREEN (t_color) { 151, 190, 90, 255 }
#define H_YELLOW (t_color) { 243, 202, 82, 255 }

typedef enum item_effect_condition {

  PASSIVE, 
  ON_ATTACK, 
  ON_HIT, 
  ON_CAST,
  AT_HEALTH,

} item_effect_condition;

typedef enum item_effect_type { 

  TYPE_EMTPY,
  GAIN_STATS

} item_effect_type;

typedef enum item_effect_repeat { 

  ONCE,
  REPEAT,

} item_effect_repeat;

typedef enum item_effect_lifetime { 

  TIME, //example 5 seconds
  END_OF_COMBAT, //until end of combat

} item_effect_lifetime;

typedef struct item_effect { 

  item_effect_condition condition;
  item_effect_type type;
  item_effect_repeat repeat;
  item_effect_lifetime lifetime;
  float limit;

  float attack_speed;

} item_effect;
//bow = on attack, gain stats, 25 times

typedef enum game_phase { SHOP, PLAN, COMBAT, RECAP } game_phase;
typedef enum character_class { TANK, FIGHTER, RANGER } character_class;

static t_sprite s_sprite_panel;
static t_sprite s_sprite_shop_slot;
static t_sprite s_sprite_triangle;
static t_sprite s_sprite_gear;
static t_sprite s_sprite_panel_border_1;

static t_sprite s_sprite_border_2;

static t_sprite s_sprite_circling_particles;

static t_sprite s_sprite_sword;
static t_sprite s_sprite_shield;
static t_sprite s_sprite_bow;
static t_sprite s_sprite_shot;

static t_sprite s_sprite_plus_sign;
static t_sprite s_sprite_swap_sign;
static t_sprite s_sprite_border;
static t_sprite s_sprite_arrow;

static t_ui_button s_button_sell;

static t_ui_button s_button_play;
static t_ui_button s_button_plan;

static t_ui_button s_button_toggle_shop;
static t_ui_button s_button_refresh_shop;
static t_ui_button s_button_lock_shop;

// STATE
static unsigned int s_currency = 2;
static unsigned int s_level = 1;
static unsigned int s_stage = 0;

typedef struct item {

  const char* name;
  const char* description; 

  unsigned int cost;
  unsigned int attack_damage;
  float attack_speed;
  unsigned int crit_chance;
  unsigned int health;

  t_sprite* sprite;

  item_effect effect;

} item;

typedef struct character {

  const char* name;
  t_sprite* sprite;

  unsigned int cost;

  unsigned int damage;
  unsigned int health;
  unsigned int energy;
  unsigned int range;
  unsigned int crit_chance;
  unsigned int crit_damage;

  unsigned int attack_resist;
  unsigned int ability_resist;

  float attack_speed;
  float movement_speed;
  
  character_class class;


} character;

typedef struct a_cell {

  t_vec2 position;

  int grid_x;
  int grid_y;

  float g_cost; // distance from start
  float h_cost; // distance from end

  bool is_occupied;
  bool is_open;
  bool is_closed;

  struct a_cell** neighbours;
  struct a_cell* parent;
} a_cell;

typedef struct character_instance {

  character* character;

  unsigned int damage;

  int health_max;
  unsigned int energy_max;

  unsigned int range;
  unsigned int crit_chance;
  unsigned int crit_damage;

  unsigned int attack_resist;
  unsigned int ability_resist;

  float attack_speed;
  float movement_speed;

  t_ui_button* item_slot_buttons;

} character_instance;

typedef struct character_combat_instance { 

  character_instance* instance;

  float current_movement_timer_x;
  float current_movement_timer_y;
  bool performing_movement;

  float move_timer;
  float attack_timer;

  a_cell* current_cell;
  a_cell* destination_cell;

  bool is_alive;

  t_vec2 position;

  unsigned int damage;

  int health_max;
  int health_current;

  unsigned int energy_max;
  unsigned int energy_current;

  unsigned int range;
  unsigned int crit_chance;
  unsigned int crit_damage;

  unsigned int attack_resist;
  unsigned int ability_resist;

  float attack_speed;
  float movement_speed;

  t_list* team;

  struct character_combat_instance* target;

} character_combat_instance;

typedef struct character_slot {
  character_instance* instance;

} character_slot;

typedef struct board_slot {
  t_vec2 position;
  character_slot character_slot;
  a_cell* cell;

} board_slot;

typedef struct item_slot {

  item* item;
} item_slot;

typedef struct shop_slot {

  character* character;
  item* item;

} shop_slot;

static t_ui_button* s_shop_slot_mouseover;
static t_ui_button* s_character_slot_mouseover;
static t_ui_button* s_board_slot_mouseover;

static character_slot* s_selected_character_slot;
static board_slot* s_selected_board_slot;

static character_instance* s_selected_character_instance = NULL;
static character_combat_instance* s_selected_character_combat_instance = NULL;
static item* s_selected_item = NULL;

#define SHOP_SIZE 4
static t_list* s_list_shop_slots;

#define CHARACTER_SLOTS_SIZE (2 * 6)
static t_list* s_list_character_slots;

#define ITEM_SLOTS_SIZE (6)
static t_list* s_list_item_slots;

#define BOARD_ROWS 6
#define BOARD_COLS 6
#define BOARD_SIZE (BOARD_ROWS * BOARD_COLS)
static t_list* s_list_board_slots;

static t_list* s_list_characters;
static t_list* s_list_items;

static bool s_ability_active = false;
static float s_ability_timer = 0;

static t_list* s_list_active_abilities;

typedef struct ability_instance { 

  void (*ability_start)(struct ability_instance*);
  void (*ability_loop)(struct ability_instance*);
  void (*ability_end)(struct ability_instance*);

  character_combat_instance* character_combat_instance;

  float timer;
  float timer_limit;

} ability_instance;

static void s_add_ability_instance(ability_instance* instance) { 
  if (s_list_active_abilities == NULL)
    s_list_active_abilities = create_list(sizeof(ability_instance));

  instance->ability_start(instance);
  add_to_list(s_list_active_abilities, instance);
}

static void s_update_active_abilities() { 

  if (s_list_active_abilities != NULL) {
    for (int i = 0; i < s_list_active_abilities->size; i++) { 

      ability_instance* instance = element_at_list(s_list_active_abilities, i);

      instance->ability_loop(instance);
      instance->timer += t_delta_time();

      if (instance->timer >= instance->timer_limit) {
        instance->ability_end(instance);
        remove_from_list(s_list_active_abilities, i);
        i--;
      }
    }
  }
}

static void s_ranger_ability_start(ability_instance* instance) {
  for (int i = 0; i < instance->character_combat_instance->team->size; i++) { 

    t_ui_button* btn = element_at_list(instance->character_combat_instance->team, i);
    character_combat_instance* team_member_instance = btn->user_data;

    team_member_instance->attack_speed += 0.5;
  }
}

static void s_ranger_ability_loop(ability_instance* instance) {

}

static void s_ranger_ability_end(ability_instance* instance) { 
  for (int i = 0; i < instance->character_combat_instance->team->size; i++) { 

    t_ui_button* btn = element_at_list(instance->character_combat_instance->team, i);
    character_combat_instance* team_member_instance = btn->user_data;

    team_member_instance->attack_speed -= 0.5;
  }
}

static void s_ranger_ability(character_combat_instance* instance) { 

  ability_instance* abi_instance = malloc(sizeof(ability_instance));
  abi_instance->ability_start = s_ranger_ability_start;
  abi_instance->ability_loop = s_ranger_ability_loop;
  abi_instance->ability_end = s_ranger_ability_end;

  abi_instance->character_combat_instance = instance;
  abi_instance->timer = 0;
  abi_instance->timer_limit = 5;

  s_add_ability_instance(abi_instance);
}

static t_list* s_list_ally_characters;
static t_list* s_list_enemy_characters;

static t_array* s_cells;
static t_array* s_cells_open;
static t_array* s_cells_closed;

static void s_init_path() {

  if (s_cells_open != NULL)
    clear_array(s_cells_open);
  else
    s_cells_open = create_array();

  if (s_cells_closed != NULL)
    clear_array(s_cells_closed);
  else
    s_cells_closed = create_array();

  if (s_cells != NULL) {

    for (int i = 0; i < s_cells->size; i++) {

      a_cell* cell = element_at_array(s_cells, i);
      cell->is_closed = false;
      cell->is_open = false;
      cell->parent = NULL;
      cell->g_cost = 0;
      cell->h_cost = 0;
    }

  }
  else
    s_cells = create_array();
}

static float s_get_cell_distance(a_cell* cell_a, a_cell* cell_b) {
  float dist_x = abs(cell_a->grid_x - cell_b->grid_x);
  float dist_y = abs(cell_a->grid_y - cell_b->grid_y);

  if (dist_x > dist_y) {
    return 1.4f * dist_y + 1.0f * (dist_x - dist_y);
  } else {
    return 1.4f * dist_x + 1.0f * (dist_y - dist_x);
  }
}

static a_cell* s_find_path(a_cell* cell_from, a_cell* cell_to) {
  t_log_debug("Search path from (%d, %d) to (%d, %d)", cell_from->grid_x, cell_from->grid_y, cell_to->grid_x, cell_to->grid_y);

  // mark start cell as open and add to list of open
  cell_from->is_open = true;
  add_to_array(s_cells_open, cell_from);

  while (true) {

    a_cell* current = NULL;
    t_log_debug("Check open cells: %d", s_cells_open->size);
    for (int i = 0; i < s_cells_open->size; i++) {
      a_cell* open_cell = element_at_array(s_cells_open, i);
      t_log_debug("Comapre cell: (%d, %d) - %d", open_cell->grid_x, open_cell->grid_y, open_cell->is_open);
      //temp check if remove from list does not work
      if (!open_cell->is_open) continue;
      //t_log_debug("Compare cells: (%d, %d)[%f] - (%d, %d)[%f]", current->grid_x, current->grid_y, current->g_cost + current->h_cost,
      //   open_cell->grid_x, open_cell->grid_y, open_cell->g_cost + open_cell->h_cost);

      if (current == NULL || ((open_cell->g_cost + open_cell->h_cost) < (current->g_cost + current->h_cost) ||
          ((open_cell->g_cost + open_cell->h_cost) == (current->g_cost + current->h_cost) && open_cell->h_cost < current->h_cost))) {
        current = open_cell;
      }
    }

    if (current == NULL) return cell_from;

    t_log_debug("Current cell: (%d, %d)", current->grid_x, current->grid_y);
    remove_from_array(s_cells_open, current);
    add_to_array(s_cells_closed, current);

    // mark current as closed
    current->is_open = false;
    current->is_closed = true;

    if (current == cell_to) {
      t_log_debug("Path found, retracing");
      a_cell* cell_parent = current;
      while (true) {
        if (cell_parent->parent == cell_from)
            return cell_parent;

        cell_parent = cell_parent->parent;
      }
    }

    for (int i = 0; i < 8; i ++) {
      a_cell* neighbour_cell = current->neighbours[i];
      if (neighbour_cell == NULL) continue;

      if (neighbour_cell == cell_to) {
          t_log_debug("Path found, retracing");
          a_cell* cell_parent = current;
          while (true) {
              if (cell_parent == NULL) return NULL;
              if (cell_parent->parent == cell_from)
                  return cell_parent;

              cell_parent = cell_parent->parent;
          }
      }

      t_log_debug("Evaluate neighbour: %d", i);
      if (neighbour_cell->is_occupied || neighbour_cell->is_closed) {
        t_log_debug("Neighbour occupied or closed. Skipping");
        continue;
      }
      t_log_debug("Calculating neighbour movement cost.");
      float movement_cost = current->g_cost + s_get_cell_distance(current, neighbour_cell);
      t_log_debug("Movement cost: %f", movement_cost);
      if (movement_cost < neighbour_cell->g_cost || !neighbour_cell->is_open) {
        neighbour_cell->g_cost = movement_cost;
        neighbour_cell->h_cost = s_get_cell_distance(neighbour_cell, cell_to);
        neighbour_cell->parent = current;

        if (!neighbour_cell->is_open) {
          neighbour_cell->is_open = true;
          add_to_array(s_cells_open, neighbour_cell);

          t_log_debug("Add to open: (%d, %d) %d", neighbour_cell->grid_x, neighbour_cell->grid_y, neighbour_cell->is_open);
        }
      }
    }
  }

  return NULL;
}

static void s_on_shop_slot_mouse_enter(t_ui_button* button) {
  s_shop_slot_mouseover = button;
}

static void s_on_shop_slot_mouse_exit(t_ui_button* button) {
  s_shop_slot_mouseover = NULL;
}

static character_combat_instance* s_create_character_combat_instance(character_instance* instance, t_list* team) { 

  character_combat_instance* combat_instance = malloc(sizeof(character_combat_instance));

  unsigned int total_damage = instance->damage;
  unsigned int total_crit_chance = instance->crit_chance;
  float total_attack_speed = instance->attack_speed;

  for (int i = 0; i < 3; i ++) {

    t_ui_button* slot_button = &instance->item_slot_buttons[i];
    item_slot* item_slot = slot_button->user_data;

    if (item_slot != NULL && item_slot->item != NULL) { 
      total_damage += item_slot->item->attack_damage;
      total_crit_chance += item_slot->item->crit_chance;
      total_attack_speed += item_slot->item->attack_speed;
    }
  }

  combat_instance->instance = instance;
  combat_instance->damage = total_damage;
  combat_instance->health_max = instance->health_max;
  combat_instance->health_current = combat_instance->health_max;

  combat_instance->energy_max = instance->energy_max;
  combat_instance->energy_current = 0;

  combat_instance->range = instance->range;
  combat_instance->crit_chance = total_crit_chance;
  combat_instance->crit_damage = instance->crit_damage;

  combat_instance->attack_resist = instance->attack_resist;
  combat_instance->ability_resist = instance->ability_resist;

  combat_instance->attack_speed = total_attack_speed;
  combat_instance->movement_speed = instance->movement_speed;

  combat_instance->attack_timer = 0;
  combat_instance->move_timer = 0;
  combat_instance->position = VEC2_ZERO;
  combat_instance->target = NULL;
  combat_instance->is_alive = true;
  combat_instance->current_cell = NULL;
  combat_instance->destination_cell = NULL;
  combat_instance->current_movement_timer_x = 0;
  combat_instance->current_movement_timer_y = 0;
  combat_instance->performing_movement = false;
  combat_instance->team = team;

  return combat_instance;
}

static character_instance* s_create_character_instance(character* character) { 

  character_instance* instance = malloc(sizeof(character_instance));
  instance->character = character;

  instance->damage = character->damage;
  instance->health_max = character->health;
  instance->energy_max = character->energy;

  instance->range = character->range;
  instance->crit_chance = character->crit_chance;
  instance->crit_damage = character->crit_damage;

  instance->attack_resist = character->attack_resist;
  instance->ability_resist = character->ability_resist;

  instance->attack_speed = character->attack_speed;
  instance->movement_speed = character->movement_speed;

  instance->item_slot_buttons = malloc(3 * sizeof(t_ui_button));

  for (int i = 0; i < 3; i++) { 
    instance->item_slot_buttons[i] = create_ui_button(&s_sprite_border_2);

    item_slot* slot = malloc(sizeof(item_slot));
    slot->item = NULL;
    instance->item_slot_buttons[i].user_data = slot;
    instance->item_slot_buttons[i].user_data_1 = slot;
  }

  return instance;
}

static void s_on_combat_instance_clicked(t_ui_button* button) { 

  character_combat_instance* instance = button->user_data;

  if (s_selected_character_combat_instance != instance) {
    s_selected_character_combat_instance = instance;
    s_selected_character_instance = NULL;
  }
  else
    s_selected_character_combat_instance = NULL;
}

static void s_on_shop_slot_clicked(t_ui_button* button) {

  shop_slot* s_slot = (shop_slot*)button->user_data;
  bool shop_slot_bought = false;

  if (s_slot->character != NULL) {
    t_log_debug("Attempt buy character");

    if ((int)s_currency - (int)s_slot->character->cost < 0) {
      t_log_debug("Not enough currency");
      return;
    }

    for (int i = 0; i < CHARACTER_SLOTS_SIZE; i ++) {
      t_ui_button* character_slot_button = element_at_list(s_list_character_slots, i);
      character_slot* slot = (character_slot*)character_slot_button->user_data;

      if (slot->instance == NULL) {
        slot->instance = s_create_character_instance(s_slot->character);
        t_log_debug("Empty bench index: %d", i);
        t_log_debug("Bought");
        s_currency -= s_slot->character->cost;
        button->is_disabled = true;
        s_shop_slot_mouseover = NULL;
        character_slot_button->sprite = slot->instance->character->sprite;
        break;
      }
    }
  }
  else {
    t_log_debug("Attempt buy item.");

    if ((int)s_currency - (int)s_slot->item->cost < 0) {
      t_log_debug("Not enough currency");
      return;
    }

    for (int i = 0; i < ITEM_SLOTS_SIZE; i ++) {
      t_ui_button* item_slot_button = element_at_list(s_list_item_slots, i);
      item_slot* slot = (item_slot*)item_slot_button->user_data;

      if (slot->item == NULL) {
        slot->item = s_slot->item;
        t_log_debug("Empty bench index: %d", i);
        t_log_debug("Bought");
        s_currency -= s_slot->item->cost;
        button->is_disabled = true;
        s_shop_slot_mouseover = NULL;
        item_slot_button->sprite = slot->item->sprite;
        break;
      }
    }
  }
}


static bool s_slot_pressed = false;
static float s_slot_pressed_timer = 0;
static t_vec2 s_slot_pressed_at;

static bool s_dragging_item = false;
static t_ui_button* s_dragged_button = NULL;

static void s_on_item_slot_pressed(t_ui_button* button) { 

  if (s_dragging_item) return;

    if (!s_slot_pressed) {
        s_slot_pressed = true;
        s_slot_pressed_timer = 0;
        s_slot_pressed_at = mouse_position();
    } else {
        s_slot_pressed_timer += t_delta_time();

        const float distance = t_vec2_distance(s_slot_pressed_at, mouse_position());
        if (s_slot_pressed_timer >= 0.5f ||
             distance > 10) {

            s_dragging_item = true;
            s_slot_pressed = false;
        }
    }

    if (s_dragging_item) {
        s_dragged_button = button;
    }
}

static void s_on_item_slot_clicked(t_ui_button* button) {

  if (s_dragging_item) return;

  item_slot* slot = (item_slot*)button->user_data_1;
  s_selected_item = slot->item;
  s_selected_character_instance = NULL;

  s_slot_pressed = false;
}

static bool s_dragging_character = false;

static void s_on_character_slot_pressed(t_ui_button* button) {

    if (s_dragging_character) return;

    if (!s_slot_pressed) {
        s_slot_pressed = true;
        s_slot_pressed_timer = 0;
        s_slot_pressed_at = mouse_position();
    } else {
        s_slot_pressed_timer += t_delta_time();

        const float distance = t_vec2_distance(s_slot_pressed_at, mouse_position());
        if (s_slot_pressed_timer >= 0.5f ||
             distance > 10) {

            s_dragging_character = true;
            s_slot_pressed = false;
        }
    }

    if (s_dragging_character) {
        s_dragged_button = button;
    }
}

static void s_on_character_slot_clicked(t_ui_button* button) {

  if (s_dragging_character) return;

  character_slot* slot = (character_slot*)button->user_data_1;
  s_selected_character_instance = slot->instance;
  s_selected_character_combat_instance = NULL;
  s_selected_item = NULL;

  s_slot_pressed = false;
}

static void s_on_board_slot_mouse_enter(t_ui_button* button) {
  s_board_slot_mouseover = button;
}

static void s_on_board_slot_mouse_exit(t_ui_button* button) {
  s_board_slot_mouseover = NULL;
}

static void s_on_button_sell_clicked(t_ui_button* button) {
  UNUSED(button);

  // if (s_selected_board_slot != NULL) {
  //   s_currency += s_selected_board_slot->character->cost;
  //   s_selected_board_slot->character = NULL;
  //   s_selected_board_slot = NULL;
  // } else if (s_selected_character_slot != NULL) {
  //   s_currency += s_selected_character_slot->character->cost;
  //   s_selected_character_slot->character = NULL;
  //   s_selected_character_slot = NULL;
  // }
}

static bool s_is_playing = false;
static game_phase s_phase = SHOP;

static float s_offset_y_shop = 0;
static bool s_ease_out_shop = false;
static bool s_ease_in_shop = false;
static float s_ease_timer_shop = 0;

static void s_on_button_plan_clicked(t_ui_button* button) {
  UNUSED(button);

  s_phase = PLAN;
  s_ease_out_shop = true;

   for (int i = 0; i < s_list_board_slots->size / 2; i++) {

    t_ui_button* button = element_at_list(s_list_board_slots, i);
    board_slot* slot = (board_slot*)button->user_data;

    if (t_random_float(0, 1) > 0.8f) {
      slot->character_slot.instance = s_create_character_instance((character*)element_at_list(s_list_characters, t_random_int(0, s_list_characters->size)));
    }
  }
}

static void s_on_button_play_clicked(t_ui_button* button) {
  UNUSED(button);

  s_phase = COMBAT;

  if (s_list_ally_characters != NULL)
    destroy_list(s_list_ally_characters);

  if (s_list_enemy_characters != NULL)
    destroy_list(s_list_enemy_characters);

  s_list_ally_characters = create_list(sizeof(t_ui_button));
  s_list_enemy_characters = create_list(sizeof(t_ui_button));

  // Allies
  for (int i = s_list_board_slots->size / 2; i < s_list_board_slots->size; i ++) {
    t_ui_button* button = element_at_list(s_list_board_slots, i);
    board_slot* slot = (board_slot*)button->user_data;

    if (slot->character_slot.instance != NULL) {

      t_ui_button* instance_button = malloc(sizeof(t_ui_button));
      *instance_button = create_ui_button(slot->character_slot.instance->character->sprite);

      character_combat_instance* instance = s_create_character_combat_instance(slot->character_slot.instance, s_list_ally_characters);
      instance->position = slot->position;
      instance->current_cell = slot->cell;
      instance->destination_cell = slot->cell;

      instance_button->user_data = instance;
      instance_button->on_released = s_on_combat_instance_clicked;

      slot->cell->is_occupied = true;

      add_to_list(s_list_ally_characters, instance_button);
    }
  }

  for (int i = 0; i < s_list_board_slots->size / 2; i++) {

    t_ui_button* button = element_at_list(s_list_board_slots, i);
    board_slot* slot = (board_slot*)button->user_data;

    if (slot->character_slot.instance != NULL)
    {

      t_ui_button* instance_button = malloc(sizeof(t_ui_button));
      *instance_button = create_ui_button(slot->character_slot.instance->character->sprite);

      character_combat_instance* instance = s_create_character_combat_instance(slot->character_slot.instance, s_list_enemy_characters);
      instance->position = slot->position;
      instance->current_cell = slot->cell;
      instance->destination_cell = slot->cell;

      instance_button->user_data = instance;
      instance_button->on_released = s_on_combat_instance_clicked;

      slot->cell->is_occupied = true;

      add_to_list(s_list_enemy_characters, instance_button);
    }
  }

  s_is_playing = true;
}

static void s_on_button_pause_cliced(t_ui_button* button) {
  UNUSED(button);
  s_is_playing = false;
}

typedef struct instance_shot {

  t_vec2 shot_from;
  character_combat_instance* from;
  character_combat_instance* to;
  
  t_vec2 pos;

} instance_shot;

typedef struct instance_text { 
  
  t_vec2 spawned_at;
  char text[15];

  bool crit;
  float timer;
} instance_text;

static t_list* s_list_shots = NULL; 
static t_list* s_list_texts = NULL;

static void s_spawn_text(int value, int x, int y, bool crit) { 

  if (s_list_texts == NULL)
    s_list_texts = create_list(sizeof(instance_text));

  instance_text* instance = malloc(sizeof(instance_text));
  instance->spawned_at = (t_vec2) {x, y};
  instance->crit = crit;
  instance->timer = 0;

  sprintf(instance->text, "%d", value);

  add_to_list(s_list_texts, instance);
}

static void s_spawn_shot(character_combat_instance* from, character_combat_instance* to) {

  if (s_list_shots == NULL)
    s_list_shots = create_list(sizeof(instance_shot));

  instance_shot* instance = malloc(sizeof(instance_shot));
  instance->from = from;
  instance->to = to;
  instance->pos = from->position;
  instance->pos.x += 20;
  instance->pos.y += 20;

  add_to_list(s_list_shots, instance);
}

static void s_draw_shots() { 

  if (s_list_shots != NULL) { 
    for (int i = 0; i < s_list_shots->size; i++) { 

      instance_shot* shot = element_at_list(s_list_shots, i);

      t_vec2 pos_to = shot->to->position;
      pos_to.x += 20;
      pos_to.y += 20;

      float distance = t_vec2_distance(shot->pos, pos_to);
      t_vec2 dir = t_vec2_dir(shot->pos, pos_to);

      shot->pos.x += dir.x / 5;
      shot->pos.y += dir.y / 5;

      t_draw_sprite(&s_sprite_shot, shot->pos.x, shot->pos.y, 8, 8, WHITE);

      if (distance <= 10) {

        bool is_crit = t_random_int(0, 100) < shot->from->crit_chance;
        int damage = is_crit ? shot->from->damage * (1 + ((float)shot->from->crit_damage / 100)) : shot->from->damage;
        shot->to->health_current -= damage;
        
        s_spawn_text(damage, shot->pos.x, shot->pos.y, is_crit);

        if (shot->to->health_current <= 0) {
          shot->to->is_alive = false;
          shot->to->destination_cell->is_occupied = false;
        }

        remove_from_list(s_list_shots, i);
        i--;
      }
    }
  }
}

static void s_draw_combat_characters(t_list* allies, t_list* enemies, bool move) {

  int allies_alive = allies->size;
  for (int i = 0; i < allies->size; i++) { 

    t_ui_button* combat_instance_button = element_at_list(allies, i);
    character_combat_instance* combat_instance = combat_instance_button->user_data;

    // skip if not alive
    if (!combat_instance->is_alive) { allies_alive --; continue; } 

    // find new target
    if (combat_instance->target == NULL || !combat_instance->target->is_alive) {

      character_combat_instance* closest_enemy = NULL;

      for (int e = 0; e < enemies->size; e++) { 

        character_combat_instance* enemy_combat_instance = ((t_ui_button*)element_at_list(enemies, e))->user_data;

        // skip if not alive
        if (!enemy_combat_instance->is_alive) continue;

        if (closest_enemy == NULL || 
              s_get_cell_distance(combat_instance->destination_cell, enemy_combat_instance->destination_cell) < 
              s_get_cell_distance(combat_instance->destination_cell, closest_enemy->destination_cell)) {

          closest_enemy = enemy_combat_instance;
        }
      }

      combat_instance->target = closest_enemy;
    }

    draw_ui_button(combat_instance_button, combat_instance->position.x, combat_instance->position.y, 48, 48);
    // t_draw_sprite(combat_instance->instance->character->sprite, combat_instance->position.x, combat_instance->position.y, 48, 48, WHITE);

    if (move && combat_instance->target != NULL) { 

      if (combat_instance->performing_movement) { 

        t_ease_out_quint(&combat_instance->current_movement_timer_x, &combat_instance->position.x, combat_instance->current_cell->position.x, combat_instance->destination_cell->position.x, 1.0f);
        float p = t_ease_out_quint(&combat_instance->current_movement_timer_y, &combat_instance->position.y, combat_instance->current_cell->position.y, combat_instance->destination_cell->position.y, 1.0f);

        if (p >= 1) {
          combat_instance->performing_movement = false;
          combat_instance->current_cell = combat_instance->destination_cell;
          combat_instance->current_movement_timer_x = 0;
          combat_instance->current_movement_timer_y = 0;
          combat_instance->destination_cell->is_occupied = true;
        }
      } 
      else { 

        float distance_from_target = s_get_cell_distance(combat_instance->current_cell, combat_instance->target->destination_cell);

        if (distance_from_target - 0.41f > combat_instance->range) {

          s_init_path();
          a_cell* next_move_cell = s_find_path(combat_instance->current_cell, combat_instance->target->destination_cell);
          if (next_move_cell != NULL) {
            combat_instance->performing_movement = true;
            combat_instance->destination_cell = next_move_cell;
            combat_instance->destination_cell->is_occupied = true;
            combat_instance->current_cell->is_occupied = false;
          }        
        }
        else {
          combat_instance->attack_timer += t_delta_time() * combat_instance->attack_speed;

          if (combat_instance->attack_timer >= 1.0f) {

            s_spawn_shot(combat_instance, combat_instance->target);

            combat_instance->attack_timer = 1.0f - combat_instance->attack_timer;
            combat_instance->energy_current += 1;

            if (combat_instance->energy_current >= combat_instance->energy_max) {
              combat_instance->energy_current = 0;

              s_ranger_ability(combat_instance);
            }
          }
        }
      }
    }
    
    // health
    draw_rect(combat_instance->position.x, combat_instance->position.y, ((float)combat_instance->health_current / combat_instance->health_max) * 48, 4, H_GREEN);

    // energy
    draw_rect(combat_instance->position.x, combat_instance->position.y + 4, ((float)combat_instance->energy_current / combat_instance->energy_max) * 48, 2, H_YELLOW);
  }

  if (allies_alive == 0) s_is_playing = false;
}

static void s_draw_combat() {

  s_draw_combat_characters(s_list_ally_characters, s_list_enemy_characters, true);
  s_draw_combat_characters(s_list_enemy_characters, s_list_ally_characters, false);
}

static void s_create_board() {

  s_list_board_slots = create_list(sizeof(t_ui_button));
  for (int i = 0; i < BOARD_SIZE; i++) {

    const int x = (i % BOARD_COLS);
    const int y = (i / BOARD_COLS);

    const int pos_x = 16 + x * (48 + 8);
    const int pos_y = 16 + y * (48 + 8);

    board_slot* slot = malloc(sizeof(board_slot));
    slot->character_slot.instance = NULL;
    slot->position = (t_vec2) { pos_x, pos_y };

    a_cell* cell = malloc(sizeof(a_cell));
    cell->position = slot->position;
    cell->grid_x = x;
    cell->grid_y = y;
    cell->g_cost = 0;
    cell->h_cost = 0;
    cell->is_occupied = false;
    cell->is_open = false;
    cell->is_closed = false;
    cell->parent = NULL;
    cell->neighbours = calloc(8, sizeof(a_cell*));
    slot->cell = cell;
    add_to_array(s_cells, cell);

    // set cell neighbours from top left
    if (x > 0) {
      // left
      t_ui_button* slot_left_btn = element_at_list(s_list_board_slots, i - 1);
      board_slot* slot_left = slot_left_btn->user_data;
      cell->neighbours[1] = slot_left->cell;
      slot_left->cell->neighbours[5] = cell;
    }

    if (y > 0) {
      // top
      t_ui_button* slot_top_btn = element_at_list(s_list_board_slots, i - BOARD_COLS);
      board_slot* slot_top = slot_top_btn->user_data;
      slot_top->cell->neighbours[7] = cell;
      cell->neighbours[3] = slot_top->cell;

      // top right
      if (x < BOARD_COLS - 1) {
        t_ui_button* slot_top_right_btn = element_at_list(s_list_board_slots, i - (BOARD_COLS - 1));
        board_slot* slot_top_right = slot_top_right_btn->user_data;
        slot_top_right->cell->neighbours[0] = cell;
        cell->neighbours[4] = slot_top_right->cell;
      }
      // top left
      if (x > 0) {
        t_ui_button* slot_top_left_btn = element_at_list(s_list_board_slots, i - (BOARD_COLS + 1));
        board_slot* slot_top_left = slot_top_left_btn->user_data;
        slot_top_left->cell->neighbours[6] = cell;
        cell->neighbours[2] = slot_top_left->cell;
      }
    }


    t_ui_button* slot_button = malloc(sizeof(t_ui_button));

    *slot_button = create_ui_button(&s_sprite_border);
    slot_button->user_data = slot;
    slot_button->user_data_1 = &slot->character_slot;
    slot_button->on_mouse_enter = s_on_board_slot_mouse_enter;
    slot_button->on_mouse_exit = s_on_board_slot_mouse_exit;
    slot_button->on_released = s_on_character_slot_clicked;
    slot_button->on_pressed = s_on_character_slot_pressed;

    add_to_list(s_list_board_slots, slot_button);
  }
}

static void s_create_characters_inventory() {

  s_list_character_slots = create_list(sizeof(t_ui_button));
  for (int i = 0; i < CHARACTER_SLOTS_SIZE; i++) {

    character_slot* slot = malloc(sizeof(character_slot));
    slot->instance = NULL;

    t_ui_button* slot_button = malloc(sizeof(t_ui_button));

    *slot_button = create_ui_button(&s_sprite_border);
    slot_button->user_data = slot;
    slot_button->user_data_1 = slot;
    slot_button->on_released = s_on_character_slot_clicked;
    slot_button->on_pressed = s_on_character_slot_pressed;

    add_to_list(s_list_character_slots, slot_button);
  }
}

static void s_create_items_inventory() { 
  
  s_list_item_slots = create_list(sizeof(t_ui_button));

  for (int i = 0; i < ITEM_SLOTS_SIZE; i++) {

    item_slot* slot = malloc(sizeof(item_slot));
    slot->item = NULL;

    t_ui_button* slot_button = malloc(sizeof(t_ui_button));

    *slot_button = create_ui_button(&s_sprite_border);
    slot_button->user_data = slot;
    slot_button->user_data_1 = slot;
    slot_button->on_released = s_on_item_slot_clicked;
    slot_button->on_pressed = s_on_item_slot_pressed;

    add_to_list(s_list_item_slots, slot_button);
  }
}

static void s_load_items() { 
  s_list_items = create_list(sizeof(item));

  long file_size;
  const char* items_data = t_read_file("./res/temp/items.json", &file_size);

  if (items_data != NULL) { 
    t_log_debug("Loading items.");

    cJSON* json_data = cJSON_Parse(items_data);
    cJSON* json_files_array = cJSON_GetObjectItem(json_data, "items");
    int json_files_array_size = cJSON_GetArraySize(json_files_array);
    t_log_debug("%d items found", json_files_array_size);

    for (int i = 0; i < json_files_array_size; i++) {
      cJSON* json_array_item = cJSON_GetArrayItem(json_files_array, i);

      item* itm = malloc(sizeof(item));
      cJSON* json_name = cJSON_GetObjectItem(json_array_item, "name");
      itm->name = json_name->valuestring;

      cJSON* json_cost = cJSON_GetObjectItem(json_array_item, "cost");
      itm->cost = json_cost->valueint;

      itm->attack_damage = cJSON_GetObjectItem(json_array_item, "attack_damage")->valueint;
      itm->attack_speed = (float)cJSON_GetObjectItem(json_array_item, "attack_speed")->valuedouble;
      itm->crit_chance = cJSON_GetObjectItem(json_array_item, "crit_chance")->valueint;
      itm->health = cJSON_GetObjectItem(json_array_item, "health")->valueint;
      itm->description = cJSON_GetObjectItem(json_array_item, "description")->valuestring;

      cJSON* json_effect = cJSON_GetObjectItem(json_array_item, "effect");
      if (json_effect != NULL) { 

        item_effect effect;
        effect.condition = cJSON_GetObjectItem(json_effect, "condition")->valueint;
        effect.type = cJSON_GetObjectItem(json_effect, "type")->valueint;
        effect.repeat = cJSON_GetObjectItem(json_effect, "repeat")->valueint;
        effect.lifetime = cJSON_GetObjectItem(json_effect, "lifetime")->valueint;
        effect.limit = (float)cJSON_GetObjectItem(json_effect, "limit")->valuedouble;

        itm->effect = effect;
      }

      const char* texture_name = cJSON_GetObjectItem(json_array_item, "texture")->valuestring;
      t_sprite* sprite = malloc(sizeof(t_sprite));

      char texture_path[200];
      sprintf(texture_path, "./res/textures/%s", texture_name);
      t_load_texture_data_s(sprite, texture_path);
      t_init_sprite(sprite);

      itm->sprite = sprite;

      add_to_list(s_list_items, itm);
    }
  }
}

static void s_load_characters() {
  s_list_characters = create_list(sizeof(character));

  long file_size;
  const char* characters_data = t_read_file("./res/temp/characters.json", &file_size);

  if (characters_data != NULL) {
      t_log_debug("Loading characters.");
      cJSON* json_data = cJSON_Parse(characters_data);
      cJSON* json_files_array = cJSON_GetObjectItem(json_data, "characters");
      int json_files_array_size = cJSON_GetArraySize(json_files_array);
      t_log_debug("%d characters found", json_files_array_size);

      for (int i = 0; i < json_files_array_size; i++) {
        cJSON* json_array_item = cJSON_GetArrayItem(json_files_array, i);

        character* ch = (character*)malloc(sizeof(character));
        cJSON* json_name = cJSON_GetObjectItem(json_array_item, "name");
        ch->name = json_name->valuestring;

        cJSON* json_cost = cJSON_GetObjectItem(json_array_item, "cost");
        ch->cost = json_cost->valueint;

        ch->damage = cJSON_GetObjectItem(json_array_item, "damage")->valueint;
        ch->health = cJSON_GetObjectItem(json_array_item, "health")->valueint;
        ch->energy = cJSON_GetObjectItem(json_array_item, "energy")->valueint;
        ch->range = cJSON_GetObjectItem(json_array_item, "range")->valueint;
        ch->crit_chance = cJSON_GetObjectItem(json_array_item, "crit_chance")->valueint;
        ch->crit_damage = cJSON_GetObjectItem(json_array_item, "crit_damage")->valueint;
        ch->attack_resist = cJSON_GetObjectItem(json_array_item, "attack_resist")->valueint;
        ch->ability_resist = cJSON_GetObjectItem(json_array_item, "ability_resist")->valueint;
        ch->attack_speed = (float)cJSON_GetObjectItem(json_array_item, "attack_speed")->valuedouble;
        ch->movement_speed = (float)cJSON_GetObjectItem(json_array_item, "movement_speed")->valuedouble;
        ch->class = cJSON_GetObjectItem(json_array_item, "class")->valueint;

        const char* texture_name = cJSON_GetObjectItem(json_array_item, "texture")->valuestring;
        t_sprite* sprite = malloc(sizeof(t_sprite));

        char texture_path[200];
        sprintf(texture_path, "./res/textures/%s", texture_name);
        t_load_texture_data_s(sprite, texture_path);
        t_init_sprite(sprite);

        ch->sprite = sprite;

        add_to_list(s_list_characters, ch);
      }

      t_log_debug("Characters successfuly loaded.");

  } else {
      t_log_debug("Problem loading characters.");
  }

  free(characters_data);
}

static void s_create_shop() {

    s_list_shop_slots = create_list(sizeof(t_ui_button));
    for (int i = 0; i < SHOP_SIZE; i++) {

      shop_slot* slot = malloc(sizeof(shop_slot));
      t_ui_button* slot_button = malloc(sizeof(t_ui_button));
      slot->character = NULL;
      slot->item = NULL;

      slot->character = (character*)element_at_list(s_list_characters, t_random_int(0, s_list_characters->size));
      *slot_button = create_ui_button(slot->character->sprite);

      slot_button->user_data = slot;
      slot_button->on_mouse_enter = s_on_shop_slot_mouse_enter;
      slot_button->on_mouse_exit = s_on_shop_slot_mouse_exit;
      slot_button->on_released = s_on_shop_slot_clicked;

      add_to_list(s_list_shop_slots, slot_button);
    }
}

static t_array* s_sprite_queue;

typedef struct t_sprite_queue_item { 

  t_sprite* sprite;
  const char* texture_path;
} t_sprite_load_queue_item;

static void s_load_queue_sprites() { 

}

static void s_init_queue_sprites() { 

}

int load_setup_screen(void* args) {
    UNUSED(args);

    t_load_texture_data_s(&s_sprite_plus_sign, "./res/textures/plus_sign.png");
    t_load_texture_data_s(&s_sprite_swap_sign, "./res/textures/swap_sign.png");
    t_load_texture_data_s(&s_sprite_border, "./res/textures/1px_border.png");

    t_load_texture_data_s(&s_sprite_panel, "./res/textures/panel-001.png");
    t_load_texture_data_s(&s_sprite_shop_slot, "./res/textures/panel-015.png");
    t_load_texture_data_s(&s_sprite_triangle, "./res/textures/triangle.png");
    t_load_texture_data_s(&s_sprite_gear, "./res/textures/gear.png");

    t_load_texture_data_s(&s_sprite_border_2, "./res/textures/border_2.png");
    t_load_texture_data_s(&s_sprite_panel_border_1, "./res/textures/panel-border-001.png");

    t_load_texture_data_s(&s_sprite_circling_particles, "./res/textures/circling_particles.png");

    t_load_texture_data_s(&s_sprite_sword, "./res/textures/sword.png");
    t_load_texture_data_s(&s_sprite_shield, "./res/textures/shield.png");
    t_load_texture_data_s(&s_sprite_bow, "./res/textures/bow.png");
    t_load_texture_data_s(&s_sprite_arrow, "./res/textures/arrow.png");

    t_load_texture_data_s(&s_sprite_shot, "./res/textures/shot.png");

    set_loading_progress(1);
    set_loading_finished();
    return 0;
}

void unload_setup_screen() {

}

t_scene *scenes = NULL;
unsigned int shader_program = 0;

static t_font s_font;
static t_font s_font_s;
static t_font s_font_l;

void init_setup_screen() {
  process_gltf_file("./res/models/cube/Cube.gltf", &scenes);

  shader_program = t_create_shader_program("./res/shaders/model_shader.vs", "./res/shaders/model_shader.fs");

  s_font = load_ttf_font("./res/fonts/Eczar-Regular.ttf", 28);
  s_font_s = load_ttf_font("./res/fonts/Eczar-Regular.ttf", 24);
  s_font_l = load_ttf_font("./res/fonts/Eczar-Regular.ttf", 34);

  t_init_sprite(&s_sprite_plus_sign);
  t_init_sprite(&s_sprite_swap_sign);
  t_init_sprite(&s_sprite_triangle);
  t_init_sprite(&s_sprite_gear);

  t_init_sprite(&s_sprite_border_2);
  s_sprite_border_2.slice_borders = (t_vec4) { 2, 2, 2, 2 };

  t_init_sprite(&s_sprite_circling_particles);

  t_init_sprite(&s_sprite_panel_border_1);
  s_sprite_panel_border_1.slice_borders = (t_vec4) { 16, 16, 16, 16 };

  t_init_sprite(&s_sprite_sword);
  t_init_sprite(&s_sprite_shield);
  t_init_sprite(&s_sprite_bow);
  t_init_sprite(&s_sprite_arrow);

  t_init_sprite(&s_sprite_shot);

  t_init_sprite(&s_sprite_border);
  s_sprite_border.slice_borders = VEC4_ONE;

  t_init_sprite(&s_sprite_panel);
  s_sprite_panel.slice_borders = (t_vec4) { 16, 16, 16, 16 };

  t_init_sprite(&s_sprite_shop_slot);
  s_sprite_shop_slot.slice_borders = (t_vec4) { 16, 16, 16, 16 };

  s_button_sell = create_ui_button(&s_sprite_border);
  s_button_sell.on_released = s_on_button_sell_clicked;

  s_button_play = create_ui_button(&s_sprite_panel);
  s_button_play.on_released = s_on_button_play_clicked;
  s_button_play.color_default = CC_RED;

  s_button_plan = create_ui_button(&s_sprite_panel);
  s_button_plan.on_released = s_on_button_plan_clicked;
  s_button_plan.color_default = CC_RED;

  s_button_toggle_shop = create_ui_button(&s_sprite_triangle);
  s_button_toggle_shop.color_default = CC_RED;

  s_button_refresh_shop = create_ui_button(&s_sprite_shop_slot);
  s_button_refresh_shop.color_default = CC_DARK_RED;
  s_button_refresh_shop.color_mouseover = CC_DARK_RED;
  s_button_refresh_shop.color_clicked = CC_RED;

  s_button_lock_shop = create_ui_button(&s_sprite_shop_slot);
  s_button_lock_shop.color_default = CC_DARK_RED;
  s_button_lock_shop.color_mouseover = CC_DARK_RED;
  s_button_lock_shop.color_clicked = CC_RED;

  s_load_items();
  s_load_characters();
  s_create_shop();
  s_create_characters_inventory();
  s_create_items_inventory();

  s_init_path();
  s_create_board();
}

vec3 cam_pos = {0, 0, 5};
vec3 cam_dir = {0, 0, -1};
vec3 cam_up = {0, 1, 0};

static void s_draw_tooltip(const char* text) {

  t_vec2 size_text = measure_text_size_ttf(text, &s_font_s);
  draw_rect(mouse_position().x + 8, mouse_position().y - 32, size_text.x + 16, 24, (t_color) { 25, 25, 25, 200 });
  t_draw_text(text, &s_font_s, mouse_position().x + 8 + (size_text.x + 16 - size_text.x) / 2, mouse_position().y - 32 + (24 + size_text.y) / 2, WHITE, 0);
}

static void s_draw_texts() {

  if (s_list_texts != NULL) {

    for (int i = 0; i < s_list_texts->size; i++) { 

      instance_text* instance = element_at_list(s_list_texts, i);
      t_color color = instance->crit ? RED : WHITE;

      color.a *= 1 - instance->timer;

      float ease_out_pos_y = 0;
      t_ease_out_quint(&instance->timer, &ease_out_pos_y, 0, -25, 1.0f);

      t_vec2 position = instance->spawned_at;
      position.y += ease_out_pos_y;

      t_font* font = &s_font_s;
      if (instance->crit)
        font = &s_font_l;

      t_draw_text(instance->text, font, position.x, position.y, color, 0);

      if (instance->timer >= 1.0f) {

        remove_from_list(s_list_texts, i);
        i--;
      }
    }
  }
}

static t_ui_button* button_slot_over = NULL;

static void s_draw_board() { 
  unsigned int board_slot_index = 0;

  button_slot_over = NULL;
  // board
  for (int y = 0; y < BOARD_COLS; y++) {
    for (int x = 0; x < BOARD_ROWS; x++) {

      t_ui_button* button = element_at_list(s_list_board_slots, board_slot_index);
      board_slot* slot = (board_slot*)button->user_data;
      const int pos_x = slot->position.x;
      const int pos_y = slot->position.y;

      t_color color = y >= 3 ? CC_RED : CC_DARK_RED;
      bool is_board_slot = y >= 3;
      draw_rect_lines(pos_x, pos_y, 48, 48, color);

      if (!is_board_slot && !s_is_playing) {

        if (slot->character_slot.instance != NULL) {

          t_sprite* sprite_class = NULL;

          switch (slot->character_slot.instance->character->class) {
            case TANK:
              sprite_class = &s_sprite_shield;
              break;
            case FIGHTER:
              sprite_class = &s_sprite_sword;
              break;
            case RANGER:
              sprite_class = &s_sprite_bow;
              break;
          }

          t_draw_sprite(sprite_class, pos_x + 8, pos_y + 8, 32, 32, CC_DARK_RED);
        }
      }
      else if (is_board_slot && !s_is_playing) {

        t_rect rect_slot = {pos_x, pos_y, 48, 48};

        if (s_dragging_character && s_dragged_button != button && is_point_in_rect(mouse_position(), rect_slot)) {

            t_draw_sprite(&s_sprite_panel_border_1, pos_x, pos_y, 48, 48, color);
            button_slot_over = button;
        }

        if (s_dragged_button != button && slot->character_slot.instance != NULL)
          draw_ui_button(button, pos_x, pos_y, 48, 48);
      }
        board_slot_index++;
    }
  }
}

static void s_draw_shop() {

  if (s_ease_out_shop) {

        float progress = t_ease_out_quint(&s_ease_timer_shop, &s_offset_y_shop, 0, -160, .5f);

        if (progress >= 1) {
            s_ease_timer_shop = 0;
            s_ease_out_shop = false;
        }
  }

  t_draw_sprite(&s_sprite_panel, 64, 32 + s_offset_y_shop, 232, 128, CC_RED);

  for (int i = 0; i < SHOP_SIZE; i++) {

    t_ui_button* shop_slot_button = element_at_list(s_list_shop_slots, i);
    shop_slot* shop_slot = shop_slot_button->user_data;

    const int pos_x = 80 + i * (38 + 16);
    const int pos_y = 48  + s_offset_y_shop;

    t_draw_sprite(&s_sprite_border_2, pos_x, pos_y, 38, 38, CC_DARK_RED);
    draw_ui_button(shop_slot_button, pos_x + 2, pos_y + 2, 32, 32);

    // get cost from shop_slot
    unsigned int cost = 0;
    if (shop_slot->character != NULL)
      cost = shop_slot->character->cost;
    else if (shop_slot->item != NULL)
      cost = shop_slot->item->cost;

    char text_cost[4];
    sprintf(text_cost, "%d", cost);

    t_vec2 size_text_cost = measure_text_size_ttf(text_cost, &s_font);
    t_draw_text(text_cost, &s_font, pos_x + (38 - size_text_cost.x) / 2, pos_y + 38 + size_text_cost.y + 6, CC_BLACK, 0);
  }

  draw_ui_button(&s_button_refresh_shop, 80, 121 + s_offset_y_shop, 96, 32);
  t_vec2 size_text_refresh = measure_text_size_ttf("Refresh", &s_font);
  t_draw_text("Refresh", &s_font, 80 + (96 - size_text_refresh.x) / 2, 121 + (32 + size_text_refresh.y) / 2 + s_offset_y_shop, CC_RED, 0);

  draw_ui_button(&s_button_lock_shop, 184, 121 + s_offset_y_shop, 96, 32);
  t_vec2 size_text_lock = measure_text_size_ttf("Lock", &s_font);
  t_draw_text("Lock", &s_font, 184 + (96 - size_text_lock.x) / 2, 121 + (32 + size_text_lock.y) / 2 + s_offset_y_shop, CC_RED, 0);
}

static void s_draw_inventory() {

    t_draw_sprite(&s_sprite_shop_slot, 368, 24, 248, 132, CC_RED);

    // character slots
    int character_slot_index = 0;

    for (int x = 0; x < 2; x++) {
      for (int y = 0; y < 6; y++) {
        int pos_x = 376 + y * (32 + 8);
        int pos_y = 32 + x * (32 + 8);

        t_draw_sprite(&s_sprite_border_2, pos_x, pos_y, 32, 32, CC_DARK_RED);

        t_ui_button* slot_button = element_at_list(s_list_character_slots, character_slot_index);
        character_slot* slot = slot_button->user_data;

        if (s_dragged_button != slot_button && slot->instance != NULL)
          draw_ui_button(slot_button, pos_x, pos_y - 2, 32, 32);

        t_rect rect_slot = (t_rect) { pos_x, pos_y, 32, 32 };

        if (s_dragging_character && s_dragged_button != slot_button && is_point_in_rect(mouse_position(), rect_slot)) {

            t_draw_sprite(&s_sprite_panel_border_1, pos_x, pos_y, 32, 32, CC_DARK_RED);
            button_slot_over = slot_button;
        }

        character_slot_index++;
      }
    }

    draw_line(376, 110, 608, 110, 1, CC_DARK_RED);

    // item slots
    for (int i = 0; i < ITEM_SLOTS_SIZE; i++) {
      const int pos_x = 376 + i * (32 + 8);
      const int pos_y = 116;

      t_draw_sprite(&s_sprite_border_2, pos_x, pos_y, 32, 32, CC_DARK_RED);

      t_ui_button* slot_button = (t_ui_button*)element_at_list(s_list_item_slots, i);
      item_slot* slot = slot_button->user_data;

      if (s_dragged_button != slot_button && slot->item != NULL)
        draw_ui_button(slot_button, pos_x, pos_y - 2, 32, 32);

      t_rect rect_slot = (t_rect) { pos_x, pos_y, 32, 32 };

      if (s_dragging_item && s_dragged_button != slot_button && is_point_in_rect(mouse_position(), rect_slot)) {

          t_draw_sprite(&s_sprite_panel_border_1, pos_x, pos_y, 32, 32, CC_DARK_RED);
          button_slot_over = slot_button;
      }
    }
}

static void s_draw_character_combat_instance_info() {

  if (s_selected_character_combat_instance != NULL) {

    // sprite
    t_draw_sprite(&s_sprite_panel, 376, 172, 64, 64, CC_DARK_RED);
    t_draw_sprite(s_selected_character_combat_instance->instance->character->sprite, 376 + 8, 172 + 8, 48, 48, WHITE);

    // health
    draw_rect(376, 247, ((float)s_selected_character_combat_instance->health_current / s_selected_character_combat_instance->health_max) * 64, 16, H_GREEN);
    draw_rect_lines(376, 247, 64, 16, CC_BLACK);

    t_rect rect_health = (t_rect) { 376, 247, 64, 16 };
    if (is_point_in_rect(mouse_position(), rect_health))
      s_draw_tooltip("Health");

    char text_health[15];
    sprintf(text_health, "%d/%d", s_selected_character_combat_instance->health_current, s_selected_character_combat_instance->health_max);

    t_vec2 size_text_health = measure_text_size_ttf(text_health, &s_font_s);
    t_draw_text(text_health, &s_font_s, 376 + (64 - size_text_health.x) / 2, 247 + (16 + size_text_health.y) / 2, CC_BLACK, 0);

    // energy
    draw_rect(376, 269, ((float)s_selected_character_combat_instance->energy_current / s_selected_character_combat_instance->energy_max) * 64, 16, (t_color) { 177, 160, 1, 255 });
    draw_rect_lines(376, 269, 64, 16, CC_BLACK);

    t_rect rect_energy = (t_rect) { 376, 269, 64, 16 };
    if (is_point_in_rect(mouse_position(), rect_energy))
      s_draw_tooltip("Energy");

    char text_energy[15];
    sprintf(text_energy, "%d/%d", s_selected_character_combat_instance->energy_current, s_selected_character_combat_instance->energy_max);

    t_vec2 size_text_energy = measure_text_size_ttf(text_energy, &s_font_s);
    t_draw_text(text_energy, &s_font_s, 376 + (64 - size_text_energy.x) / 2, 269 + (16 + size_text_energy.y) / 2, CC_BLACK, 0);

    // stats
    //  attack damage
    t_draw_sprite(&s_sprite_sword, 451, 183, 16, 16, CC_DARK_RED);

    char text_ad[5];
    sprintf(text_ad, "%u", s_selected_character_combat_instance->damage);
    t_draw_text(text_ad, &s_font_s, 468, 183 + 12, CC_BLACK, 0);

    t_rect rect_ad = (t_rect) { 451, 183, 32, 16};
    if (is_point_in_rect(mouse_position(), rect_ad)) {
      s_draw_tooltip("Attack damage");
    }

    //  attack speed
    t_draw_sprite(&s_sprite_sword, 507, 183, 16, 16, CC_DARK_RED);

    char text_as[5];
    sprintf(text_as, "%.1f", s_selected_character_combat_instance->attack_speed);
    t_draw_text(text_as, &s_font_s, 526, 183 + 12 , CC_BLACK, 0);

    t_rect rect_as = (t_rect) { 507, 183, 32, 16};
    if (is_point_in_rect(mouse_position(), rect_as)) {
      s_draw_tooltip("Attack speed");
    }

    //  crit chance
    t_draw_sprite(&s_sprite_sword, 563, 183, 16, 16, CC_DARK_RED);

    char text_crit_chance[5];
    sprintf(text_crit_chance, "%d%%", s_selected_character_combat_instance->crit_chance);
    t_draw_text(text_crit_chance, &s_font_s, 582, 183 + 12 , CC_BLACK, 0);

    t_rect rect_cc = (t_rect) { 563, 183, 32, 16};
    if (is_point_in_rect(mouse_position(), rect_cc)) {
      s_draw_tooltip("Crit chance");
    }

    //  armor
    t_draw_sprite(&s_sprite_sword, 451, 209, 16, 16, CC_DARK_RED);

    char text_armor[5];
    sprintf(text_armor, "%d", s_selected_character_combat_instance->attack_resist);
    t_draw_text(text_armor, &s_font_s, 468, 209 + 12 , CC_BLACK, 0);

    t_rect rect_ar = (t_rect) { 451, 209, 32, 16};
    if (is_point_in_rect(mouse_position(), rect_ar)) {
      s_draw_tooltip("Attack resist");
    }

    //  magic resist
    t_draw_sprite(&s_sprite_sword, 507, 209, 16, 16, CC_DARK_RED);

    char text_magic_resist[5];
    sprintf(text_magic_resist, "%d", s_selected_character_combat_instance->ability_resist);
    t_draw_text(text_magic_resist, &s_font_s, 526, 209 + 12, CC_BLACK, 0);

    t_rect rect_mr = (t_rect) { 507, 209, 32, 16};
    if (is_point_in_rect(mouse_position(), rect_mr)) {
      s_draw_tooltip("Ability resist");
    }

    //  range
    t_draw_sprite(&s_sprite_sword, 563, 209, 16, 16, CC_DARK_RED);

    char text_range[5];
    sprintf(text_range, "%d", s_selected_character_combat_instance->range);
    t_draw_text(text_range, &s_font_s, 582, 209 + 12, CC_BLACK, 0);

    t_rect rect_rng = (t_rect) { 563, 209, 32, 16};
    if (is_point_in_rect(mouse_position(), rect_rng)) {
      s_draw_tooltip("Range");
    }

    // items

    for (int i = 0; i < 3; i ++) {
      const int pos_x = 465 + i * (38 + 8);
      const int pos_y = 247;

      draw_rect_lines(pos_x, pos_y, 38, 38, CC_BLACK);

      t_ui_button* slot_button = &s_selected_character_combat_instance->instance->item_slot_buttons[i];
      item_slot* item_slot = slot_button->user_data;

       if (item_slot != NULL && item_slot->item != NULL) { 
        t_draw_sprite(item_slot->item->sprite, pos_x + 2, pos_y + 2, 32, 32, WHITE);
      }
    }
  }
}

static void s_draw_character_instance_info() {

  t_draw_sprite(&s_sprite_shop_slot, 368, 164, 248, 132, CC_RED);

  if (s_selected_character_instance != NULL) {

    unsigned int total_damage = s_selected_character_instance->damage;
    float total_attack_speed = s_selected_character_instance->attack_speed;

    // items
    for (int i = 0; i < 3; i ++) {
      const int pos_x = 465 + i * (38 + 8);
      const int pos_y = 247;

      t_ui_button* slot_button = &s_selected_character_instance->item_slot_buttons[i];
      item_slot* item_slot = slot_button->user_data;

      t_rect rect_slot = (t_rect) { pos_x, pos_y, 32, 32 };

      draw_rect_lines(pos_x, pos_y, 38, 38, CC_BLACK);

      if (item_slot != NULL && item_slot->item != NULL) { 
        t_draw_sprite(item_slot->item->sprite, pos_x + 2, pos_y + 2, 32, 32, WHITE);
        total_damage += item_slot->item->attack_damage;
        total_attack_speed += item_slot->item->attack_speed;
      }

      if (s_dragging_item && is_point_in_rect(mouse_position(), rect_slot)) {
          t_draw_sprite(&s_sprite_panel_border_1, pos_x, pos_y, 38, 38, CC_DARK_RED);
          button_slot_over = slot_button;
      }
    }

    // sprite
    t_draw_sprite(&s_sprite_panel, 376, 172, 64, 64, CC_DARK_RED);
    t_draw_sprite(s_selected_character_instance->character->sprite, 376 + 8, 172 + 8, 48, 48, WHITE);

    // health
    draw_rect(376, 247, 64, 16, H_GREEN);
    draw_rect_lines(376, 247, 64, 16, CC_BLACK);

    t_rect rect_health = (t_rect) { 376, 247, 64, 16 };
    if (is_point_in_rect(mouse_position(), rect_health))
      s_draw_tooltip("Health");

    char text_health[10];
    sprintf(text_health, "%d", s_selected_character_instance->health_max);

    t_vec2 size_text_health = measure_text_size_ttf(text_health, &s_font_s);
    t_draw_text(text_health, &s_font_s, 376 + (64 - size_text_health.x) / 2, 247 + (16 + size_text_health.y) / 2, CC_BLACK, 0);

    // energy
    draw_rect(376, 269, 64, 16, (t_color) { 177, 160, 1, 255 });
    draw_rect_lines(376, 269, 64, 16, CC_BLACK);

    t_rect rect_energy = (t_rect) { 376, 269, 64, 16 };
    if (is_point_in_rect(mouse_position(), rect_energy))
      s_draw_tooltip("Energy");

    char text_energy[10];
    sprintf(text_energy, "%d", s_selected_character_instance->energy_max);

    t_vec2 size_text_energy = measure_text_size_ttf(text_energy, &s_font_s);
    t_draw_text(text_energy, &s_font_s, 376 + (64 - size_text_energy.x) / 2, 269 + (16 + size_text_energy.y) / 2, CC_BLACK, 0);

    // stats
    //  attack damage
    t_draw_sprite(&s_sprite_sword, 451, 183, 16, 16, CC_DARK_RED);

    char text_ad[5];
    sprintf(text_ad, "%u", total_damage);
    t_draw_text(text_ad, &s_font_s, 468, 183 + 12, CC_BLACK, 0);

    t_rect rect_ad = (t_rect) { 451, 183, 32, 16};
    if (is_point_in_rect(mouse_position(), rect_ad)) {
      s_draw_tooltip("Attack damage");
    }

    //  attack speed
    t_draw_sprite(&s_sprite_sword, 507, 183, 16, 16, CC_DARK_RED);

    char text_as[5];
    sprintf(text_as, "%.1f", total_attack_speed);
    t_draw_text(text_as, &s_font_s, 526, 183 + 12 , CC_BLACK, 0);

    t_rect rect_as = (t_rect) { 507, 183, 32, 16};
    if (is_point_in_rect(mouse_position(), rect_as)) {
      s_draw_tooltip("Attack speed");
    }

    //  crit chance
    t_draw_sprite(&s_sprite_sword, 563, 183, 16, 16, CC_DARK_RED);

    char text_crit_chance[5];
    sprintf(text_crit_chance, "%d%%", s_selected_character_instance->crit_chance);
    t_draw_text(text_crit_chance, &s_font_s, 582, 183 + 12 , CC_BLACK, 0);

    t_rect rect_cc = (t_rect) { 563, 183, 32, 16};
    if (is_point_in_rect(mouse_position(), rect_cc)) {
      s_draw_tooltip("Crit chance");
    }

    //  armor
    t_draw_sprite(&s_sprite_sword, 451, 209, 16, 16, CC_DARK_RED);

    char text_armor[5];
    sprintf(text_armor, "%d", s_selected_character_instance->attack_resist);
    t_draw_text(text_armor, &s_font_s, 468, 209 + 12 , CC_BLACK, 0);

    t_rect rect_ar = (t_rect) { 451, 209, 32, 16};
    if (is_point_in_rect(mouse_position(), rect_ar)) {
      s_draw_tooltip("Attack resist");
    }

    //  magic resist
    t_draw_sprite(&s_sprite_sword, 507, 209, 16, 16, CC_DARK_RED);

    char text_magic_resist[5];
    sprintf(text_magic_resist, "%d", s_selected_character_instance->ability_resist);
    t_draw_text(text_magic_resist, &s_font_s, 526, 209 + 12, CC_BLACK, 0);

    t_rect rect_mr = (t_rect) { 507, 209, 32, 16};
    if (is_point_in_rect(mouse_position(), rect_mr)) {
      s_draw_tooltip("Ability resist");
    }

    //  range
    t_draw_sprite(&s_sprite_sword, 563, 209, 16, 16, CC_DARK_RED);

    char text_range[5];
    sprintf(text_range, "%d", s_selected_character_instance->range);
    t_draw_text(text_range, &s_font_s, 582, 209 + 12, CC_BLACK, 0);

    t_rect rect_rng = (t_rect) { 563, 209, 32, 16};
    if (is_point_in_rect(mouse_position(), rect_rng)) {
      s_draw_tooltip("Range");
    }

    
  }
}

static void s_draw_item_info() {

  if (s_selected_item != NULL) {

    t_draw_sprite(&s_sprite_panel, 376, 172, 64, 64, CC_DARK_RED);
    t_draw_sprite(s_selected_item->sprite, 376 + 8, 172 + 8, 48, 48, WHITE);

    t_draw_text(s_selected_item->name, &s_font, 376 + 64 + 12, 172 + 16, CC_BLACK, 0);

    t_draw_text(s_selected_item->description, &s_font_s, 376, 172 + 64 + 16, CC_BLACK, 232);

    int x = 0, y = 0;
    int pos_x_stat = 451 + x * (56);
    int pos_y_stat = 199 + y * (26);

    if (s_selected_item->attack_damage > 0) { 
      t_draw_sprite(&s_sprite_sword, pos_x_stat, pos_y_stat, 16, 16, CC_DARK_RED);

      char text_ad[5];
      sprintf(text_ad, "%u", s_selected_item->attack_damage);
      t_draw_text(text_ad, &s_font_s, pos_x_stat + 16, pos_y_stat + 12, CC_BLACK, 0);

      t_rect rect_ad = (t_rect) { pos_x_stat, pos_y_stat, 32, 16};
      if (is_point_in_rect(mouse_position(), rect_ad)) {
        s_draw_tooltip("Attack damage");
      }

      x++;
      pos_x_stat = 451 + x * (56);
    }

    if (s_selected_item->attack_speed > 0.0f) { 
      t_draw_sprite(&s_sprite_sword, pos_x_stat, pos_y_stat, 16, 16, CC_DARK_RED);

      char text_as[5];
      sprintf(text_as, "%f", s_selected_item->attack_speed);
      t_draw_text(text_as, &s_font_s, pos_x_stat + 16, pos_y_stat + 12, CC_BLACK, 0);

      t_rect rect_ad = (t_rect) { pos_x_stat, pos_y_stat, 32, 16};
      if (is_point_in_rect(mouse_position(), rect_ad)) {
        s_draw_tooltip("Attack speed");
      }

      x++;
      pos_x_stat = 451 + x * (56);
    }

    if (s_selected_item->crit_chance > 0) { 
      t_draw_sprite(&s_sprite_sword, pos_x_stat, pos_y_stat, 16, 16, CC_DARK_RED);

      char text_cc[5];
      sprintf(text_cc, "%d%%", s_selected_item->crit_chance);
      t_draw_text(text_cc, &s_font_s, pos_x_stat + 16, pos_y_stat + 12, CC_BLACK, 0);

      t_rect rect_ad = (t_rect) { pos_x_stat, pos_y_stat, 32, 16};
      if (is_point_in_rect(mouse_position(), rect_ad)) {
        s_draw_tooltip("Crit chance");
      }

      x++;

      if (x == 2) {
        x = 0;
        y = 1;
      }

      pos_x_stat = 451 + x * (56);
      pos_y_stat = 183 + y * (26);
    }
  }
}

static void s_draw_game_info() {

    // currency
    char currency_str[10];
    sprintf(currency_str, "G: %u", s_currency);
    t_draw_text(currency_str, &s_font, 376, 306 + 12, CC_RED, 0);

    // level
    char level_str[10];
    sprintf(level_str, "L: %u", s_level);
    t_draw_text(level_str, &s_font, 427, 306 + 12, CC_RED, 0);

    // stage
    char stage_str[10];
    sprintf(stage_str, "S: %u", s_stage);
    t_draw_text(stage_str, &s_font, 474, 306 + 12, CC_RED, 0);

    t_draw_sprite(&s_sprite_border_2, 376, 326, 128, 8, CC_RED);
}

static void s_draw_phase_controls() {

   if (s_phase == SHOP) {
      draw_ui_button(&s_button_plan, 520, 304, 96, 32);

      t_vec2 text_plan_size = measure_text_size_ttf("Plan", &s_font_l);
      t_draw_text("Plan", &s_font_l, 520 + (96 - text_plan_size.x) / 2, 304 + (32 + text_plan_size.y) / 2, CC_BLACK, 0);
    }
    else if (s_phase == PLAN) {
      draw_ui_button(&s_button_play, 520, 304, 96, 32);

      t_vec2 text_start_size = measure_text_size_ttf("Start", &s_font_l);
      t_draw_text("Start", &s_font_l, 520 + (96 - text_start_size.x) / 2, 304 + (32 + text_start_size.y) / 2, CC_BLACK, 0);
    }
}

static void s_draw_settings() { 

    t_draw_sprite(&s_sprite_gear, 1, 1, 32, 32, CC_RED);
}

static void s_draw_slot_dragging() { 

  if (s_dragged_button != NULL) {
  
    const int pos_x = mouse_position().x - s_dragged_button->mouse_clicked_at.x;
    const int pos_y = mouse_position().y - s_dragged_button->mouse_clicked_at.y;

    draw_ui_button(s_dragged_button, pos_x, pos_y, 48, 48);

    if (is_mouse_button_released(MOUSE_BUTTON_LEFT)) { 

      if (button_slot_over != NULL) {

        if (s_dragging_character) {

          character_slot* slot_from = s_dragged_button->user_data_1;
          character_slot* slot_to = button_slot_over->user_data_1;

           // Place
          if (slot_to->instance == NULL) {
            slot_to->instance = slot_from->instance;
            slot_from->instance = NULL;
          }
          // Swap
          else {
            character_instance* on_board = slot_to->instance;
            slot_to->instance = slot_from->instance;
            slot_from->instance = on_board;
            s_dragged_button->sprite = on_board->character->sprite;
          }

          button_slot_over->sprite = slot_to->instance->character->sprite; 

        } else { 
          item_slot* slot_from = s_dragged_button->user_data_1;
          item_slot* slot_to = button_slot_over->user_data_1;

          // Place
          if (slot_to->item == NULL) { 
            slot_to->item = slot_from->item;
            slot_from->item = NULL;
          }
          // Swap 
          else { 
            item* on_slot = slot_to->item;
            slot_to->item = slot_from->item;
            slot_from->item = on_slot;
            s_dragged_button->sprite = on_slot->sprite;
          }

          button_slot_over->sprite = slot_to->item->sprite;
        }
      }

      s_dragged_button = NULL;
      s_dragging_character = false;
      s_dragging_item = false;
    }
  }
}

static void s_draw_combat_phase() { 

  if (s_is_playing) { 
    s_draw_combat();
    s_draw_shots();
    s_draw_texts();

    s_update_active_abilities();
  }
}

void draw_setup_screen() {

  s_draw_board();
  s_draw_shop();

  draw_rect_lines(360, 16, 264, 328, CC_RED);
  s_draw_inventory();
  s_draw_character_instance_info();
  s_draw_character_combat_instance_info();
  s_draw_item_info();
  s_draw_game_info();
  s_draw_phase_controls();
  s_draw_settings();
  s_draw_slot_dragging();
  s_draw_combat_phase();
}
