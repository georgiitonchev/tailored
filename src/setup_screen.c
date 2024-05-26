#include "screens.h"
#include "./engine/tailored.h"

#include "game.h"
#include "cJSON.h"

#define CELL_SIZE 48

typedef enum game_phase { SHOP, PLAN, COMBAT } game_phase;
typedef enum character_class { TANK, FIGHTER, RANGER} character_class;

static t_sprite s_sprite_panel;
static t_sprite s_sprite_shop_slot;
static t_sprite s_sprite_triangle;
static t_sprite s_sprite_gear;

static t_sprite s_sprite_sword;
static t_sprite s_sprite_shield;
static t_sprite s_sprite_bow;

static t_sprite s_sprite_plus_sign;
static t_sprite s_sprite_swap_sign;
static t_sprite s_sprite_border;

static t_ui_button s_button_sell;

static t_ui_button s_button_play;
static t_ui_button s_button_plan;

static t_ui_button s_button_toggle_shop;
static t_ui_button s_button_refresh_shop;
static t_ui_button s_button_lock_shop;

// STATE
static unsigned int s_currency = 1;
static unsigned int s_level = 1;
static unsigned int s_stage = 0;

typedef struct item {

  const char* name;
  unsigned int cost;
} item;

typedef struct character { 

  const char* name;

  unsigned int cost;
  unsigned int base_dmg;
  unsigned int base_hp;
  unsigned int base_range;
  unsigned int base_crit_chance;

  unsigned int base_armor;
  unsigned int base_magic_resist;

  character_class class;

  float base_as;
  float base_ms;
  t_sprite* sprite;

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

  float current_movement_timer_x;
  float current_movement_timer_y;
  bool performing_movement;

  float move_timer;
  float attack_timer;
  float energy;

  int health_points_max;
  int health_points_current;

  a_cell* current_cell;
  a_cell* destination_cell;

  bool is_alive;

  t_vec2 position;

  struct character_instance* target;

} character_instance;

typedef struct board_slot {

  unsigned int index;
  t_vec2 position;
  character* character;

  a_cell* cell;

} board_slot;

typedef struct character_slot { 

  unsigned int index;
  character* character;
} character_slot;

typedef struct item_slot {

  unsigned int index;
  item* item;
} item_slot;

typedef struct shop_slot {

  unsigned int index;
  character* character;
  item* item;

} shop_slot;

static t_ui_button* s_shop_slot_mouseover;
static t_ui_button* s_character_slot_mouseover;
static t_ui_button* s_board_slot_mouseover;

static character_slot* s_selected_character_slot;
static board_slot* s_selected_board_slot;

static character* s_selected_character = NULL;

#define SHOP_SIZE 4
static t_list* s_list_shop_slots;

#define CHARACTER_SLOTS_SIZE (2 * 6)
static t_list* s_list_character_slots;

#define BOARD_ROWS 6
#define BOARD_COLS 6
#define BOARD_SIZE (BOARD_ROWS * BOARD_COLS)
static t_list* s_list_board_slots;

static t_list* s_list_characters;

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

      if (slot->character == NULL) {
        slot->character = s_slot->character;
        t_log_debug("Empty bench index: %d", i);
        t_log_debug("Bought");
        s_currency -= s_slot->character->cost;
        button->is_disabled = true;
        s_shop_slot_mouseover = NULL;
        character_slot_button->sprite = slot->character->sprite;
        break;
      }
    }
  }
  else { 

  }
}

static void s_on_character_slot_mouse_enter(t_ui_button* button) { 
  s_character_slot_mouseover = button;
}

static void s_on_character_slot_mouse_exit(t_ui_button* button) { 
  s_character_slot_mouseover = NULL;
}

static void s_on_character_slot_clicked(t_ui_button* button) { 
  character_slot* slot = (character_slot*)button->user_data;

  if (s_selected_board_slot != NULL) { 
     // Place
    if (slot->character == NULL) { 
      slot->character = s_selected_board_slot->character;
      s_selected_board_slot->character = NULL;
      s_selected_board_slot = NULL;
      s_selected_character_slot = slot;
    }
    // Swap
    else {
      character* on_bench = slot->character;
      slot->character = s_selected_board_slot->character;
      s_selected_board_slot->character = on_bench;
      s_selected_board_slot = NULL;
      s_selected_character_slot = slot;
    }
  }
  else { 
    if (s_selected_character_slot != slot) {
      s_selected_character_slot = slot;
    }
    else
      s_selected_character_slot = NULL;
  }
}

static void s_on_board_slot_mouse_enter(t_ui_button* button) { 
  s_board_slot_mouseover = button;
}

static void s_on_board_slot_mouse_exit(t_ui_button* button) { 
  s_board_slot_mouseover = NULL;
}

static void s_on_board_slot_clicked(t_ui_button* button) { 

  board_slot* slot = (board_slot*)button->user_data;

  // Swap or place
  if (s_selected_character_slot != NULL) { 

    // Place
    if (slot->character == NULL) { 
      slot->character = s_selected_character_slot->character;
      s_selected_character_slot->character = NULL;
      s_selected_character_slot = NULL;
      s_selected_board_slot = slot;
    }
    // Swap
    else {
      character* on_board = slot->character;
      slot->character = s_selected_character_slot->character;
      s_selected_character_slot->character = on_board;
      s_selected_character_slot = NULL;
      s_selected_board_slot = slot;
    }
  }
  else if (s_selected_board_slot != NULL && s_selected_board_slot != slot) { 

    if (slot->character == NULL) { 
      slot->character = s_selected_board_slot->character;
      s_selected_board_slot->character = NULL;
      s_selected_board_slot = slot;
    }
    // Swap
    else {
      character* on_board = slot->character;
      slot->character = s_selected_board_slot->character;
      s_selected_board_slot->character = on_board;
      s_selected_board_slot = slot;
    }
  }
  //PREVIEW
  else { 
    if (s_selected_board_slot != slot) { 
      s_selected_board_slot = slot;
    } else { 
      s_selected_board_slot = NULL;
    }
  }
}

static void s_on_button_sell_clicked(t_ui_button* button) { 
  UNUSED(button);

  if (s_selected_board_slot != NULL) { 
    s_currency += s_selected_board_slot->character->cost;
    s_selected_board_slot->character = NULL;
    s_selected_board_slot = NULL;
  } else if (s_selected_character_slot != NULL) { 
    s_currency += s_selected_character_slot->character->cost;
    s_selected_character_slot->character = NULL;
    s_selected_character_slot = NULL;
  }
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
      slot->character = (character*)element_at_list(s_list_characters, t_random_int(0, s_list_characters->size));
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

  s_list_ally_characters = create_list(sizeof(character_instance));
  s_list_enemy_characters = create_list(sizeof(character_instance));

  // Allies
  for (int i = s_list_board_slots->size / 2; i < s_list_board_slots->size; i ++) { 
    t_ui_button* button = element_at_list(s_list_board_slots, i);
    board_slot* slot = (board_slot*)button->user_data;

    if (slot->character != NULL) { 
      
      character_instance* instance = malloc(sizeof(character_instance));
      instance->character = slot->character;
      instance->health_points_max = instance->character->base_hp;
      instance->health_points_current = instance->health_points_max;
      instance->attack_timer = 0;
      instance->move_timer = 0;
      instance->energy = 0;
      instance->position = slot->position;
      instance->target = NULL;
      instance->is_alive = true;
      instance->current_cell = slot->cell;
      instance->destination_cell = slot->cell;
      instance->current_movement_timer_x = 0;
      instance->current_movement_timer_y = 0;
      instance->performing_movement = false;

      slot->cell->is_occupied = true;

      add_to_list(s_list_ally_characters, instance);
    }
  }

  for (int i = 0; i < s_list_board_slots->size / 2; i++) {

    t_ui_button* button = element_at_list(s_list_board_slots, i);
    board_slot* slot = (board_slot*)button->user_data;

    if (slot->character != NULL) 
    { 
      character_instance* instance = malloc(sizeof(character_instance));
      instance->character = slot->character;
      instance->health_points_max = instance->character->base_hp;
      instance->health_points_current = instance->health_points_max;
      instance->attack_timer = 0;
      instance->move_timer = 0;
      instance->energy = 0;
      instance->position = slot->position;
      instance->target = NULL;
      instance->is_alive = true;
      instance->current_cell = slot->cell;
      instance->destination_cell = slot->cell;
      instance->current_movement_timer_x = 0;
      instance->current_movement_timer_y = 0;
      instance->performing_movement = false;

      slot->cell->is_occupied = true;

      add_to_list(s_list_enemy_characters, instance);
    }
  }

  s_is_playing = true;
}

static void s_on_button_pause_cliced(t_ui_button* button) { 
  UNUSED(button);
  s_is_playing = false;
}

static void play() { 

  for (int i = 0; i < s_list_ally_characters->size; i++) {
    character_instance* ally_instance = element_at_list(s_list_ally_characters, i);
    if(!ally_instance->is_alive) continue;
    // find target
    if (ally_instance->target == NULL || !ally_instance->target->is_alive) {
      // t_log_debug("Ally selecting target.");
      // t_log_debug("Enemies count: %d", s_list_enemy_characters->size);

      // find closest
      character_instance* closest_enemy = NULL;

      for (int ei = 0; ei < s_list_enemy_characters->size; ei++) { 
        character_instance* enemy_instance = element_at_list(s_list_enemy_characters, ei);

        if (enemy_instance->is_alive && (closest_enemy == NULL || s_get_cell_distance(ally_instance->destination_cell, enemy_instance->destination_cell) < s_get_cell_distance(ally_instance->destination_cell, closest_enemy->destination_cell)))
          closest_enemy = enemy_instance;
      }

      ally_instance->target = closest_enemy;
    }

    t_draw_sprite(ally_instance->character->sprite, ally_instance->position.x, ally_instance->position.y, 48, 48, WHITE);

    if (ally_instance->target != NULL) {

      draw_line(ally_instance->position.x + 48 / 2, ally_instance->position.y + 48 / 2, ally_instance->target->position.x + 48 / 2, ally_instance->target->position.y + 48 / 2, 1, WHITE);
      
      if (ally_instance->performing_movement) {
        t_ease_out_quint(&ally_instance->current_movement_timer_x, &ally_instance->position.x, ally_instance->current_cell->position.x, ally_instance->destination_cell->position.x, 1.0f);
        float p = t_ease_out_quint(&ally_instance->current_movement_timer_y, &ally_instance->position.y, ally_instance->current_cell->position.y, ally_instance->destination_cell->position.y, 1.0f);

        if (p >= 1) 
        {
          ally_instance->performing_movement = false;
          ally_instance->current_cell = ally_instance->destination_cell;
          ally_instance->current_movement_timer_x = 0;
          ally_instance->current_movement_timer_y = 0;
          //??
          ally_instance->destination_cell->is_occupied = true;

        }
      }
      else {
        // t_log_debug("Pre distance check.");
        float distance_from_target = s_get_cell_distance(ally_instance->current_cell, ally_instance->target->destination_cell);
        // t_log_debug("Distance check: %f", distance_from_target);
        if (distance_from_target - 0.41f > ally_instance->character->base_range) { 
          // t_log_debug("Not withing range.");
          s_init_path();
          a_cell* next_move_cell = s_find_path(ally_instance->current_cell, ally_instance->target->destination_cell);
          if (next_move_cell != NULL) {
              ally_instance->performing_movement = true;
              ally_instance->destination_cell = next_move_cell;
              ally_instance->destination_cell->is_occupied = true;
              ally_instance->current_cell->is_occupied = false;
          }
        }
        else { 
          ally_instance->attack_timer += t_delta_time() * ally_instance->character->base_as;
        
          if (ally_instance->attack_timer >= 1.0f) {

            ally_instance->attack_timer = 1.0f - ally_instance->attack_timer;
            ally_instance->energy += 0.1f;

            ally_instance->target->health_points_current -= ally_instance->character->base_dmg;

            if (ally_instance->target->health_points_current <= 0) { 
              ally_instance->target->is_alive = false;
              ally_instance->destination_cell->is_occupied = false;
            }

            if (ally_instance->energy >= 1.0f) { 
              ally_instance->energy = 0;
            }
          }
        }
      }

      // health
      draw_rect(ally_instance->position.x, ally_instance->position.y, ((float)ally_instance->health_points_current / ally_instance->health_points_max) * 48, 2, GREEN);

      // attack
      draw_rect(ally_instance->position.x, ally_instance->position.y + 48 - 4, ally_instance->attack_timer * 48, 2, BLUE);

      // energy
      draw_rect(ally_instance->position.x, ally_instance->position.y + 48 - 2, ally_instance->energy * 48, 2, YELLOW);
    }
  }

  for (int i = 0; i < s_list_enemy_characters->size; i++) {

    character_instance* enemy_instance = element_at_list(s_list_enemy_characters, i);
    if (!enemy_instance->is_alive) continue;
    // find target
    if (enemy_instance->target == NULL || !enemy_instance->target->is_alive) {
      // t_log_debug("Ally selecting target.");
      // t_log_debug("Enemies count: %d", s_list_enemy_characters->size);

      // find closest
      character_instance* closest_enemy = NULL;

      for (int ei = 0; ei < s_list_ally_characters->size; ei++) { 
        character_instance* enemy_instance = element_at_list(s_list_ally_characters, ei);

        if (enemy_instance->is_alive && (closest_enemy == NULL || s_get_cell_distance(enemy_instance->destination_cell, enemy_instance->destination_cell) < s_get_cell_distance(enemy_instance->destination_cell, closest_enemy->destination_cell)))
          closest_enemy = enemy_instance;
      }

      enemy_instance->target = closest_enemy;
    }

    t_draw_sprite(enemy_instance->character->sprite, enemy_instance->position.x, enemy_instance->position.y, 48, 48, WHITE);

    if (enemy_instance->target != NULL) {

      draw_line(enemy_instance->position.x + 48 / 2, enemy_instance->position.y + 48 / 2, enemy_instance->target->position.x + 48 / 2, enemy_instance->target->position.y + 48 / 2, 1, WHITE);
      
      if (enemy_instance->performing_movement) {
        t_ease_out_quint(&enemy_instance->current_movement_timer_x, &enemy_instance->position.x, enemy_instance->current_cell->position.x, enemy_instance->destination_cell->position.x, 1.0f);
        float p = t_ease_out_quint(&enemy_instance->current_movement_timer_y, &enemy_instance->position.y, enemy_instance->current_cell->position.y, enemy_instance->destination_cell->position.y, 1.0f);

        if (p >= 1) 
        {
          enemy_instance->performing_movement = false;
          enemy_instance->current_cell = enemy_instance->destination_cell;
          enemy_instance->current_movement_timer_x = 0;
          enemy_instance->current_movement_timer_y = 0;
        }
      }
      else {
        // t_log_debug("Pre distance check.");
        float distance_from_target = s_get_cell_distance(enemy_instance->current_cell, enemy_instance->target->destination_cell);
        // t_log_debug("Distance check: %f", distance_from_target);
        if (distance_from_target - 0.41f > enemy_instance->character->base_range) { 
          // t_log_debug("Not withing range.");
          s_init_path();
          a_cell* next_move_cell = s_find_path(enemy_instance->current_cell, enemy_instance->target->destination_cell);

          if (next_move_cell != NULL) {
              enemy_instance->performing_movement = true;
              enemy_instance->destination_cell = next_move_cell;
              enemy_instance->destination_cell->is_occupied = true;
              enemy_instance->current_cell->is_occupied = false;
          }
        }
        else 
        { 
          enemy_instance->attack_timer += t_delta_time() * enemy_instance->character->base_as;
        
          if (enemy_instance->attack_timer >= 1.0f) {

            enemy_instance->attack_timer = 1.0f - enemy_instance->attack_timer;
            enemy_instance->energy += 0.1f;

            enemy_instance->target->health_points_current -= enemy_instance->character->base_dmg;

            if (enemy_instance->target->health_points_current <= 0) { 
              enemy_instance->target->is_alive = false;
              enemy_instance->destination_cell->is_occupied = false;
            }

            if (enemy_instance->energy >= 1.0f) { 
              enemy_instance->energy = 0;
            }
          }
        }
      }

      // health
      draw_rect(enemy_instance->position.x, enemy_instance->position.y, ((float)enemy_instance->health_points_current / enemy_instance->health_points_max) * 48, 2, GREEN);

      // attack
      draw_rect(enemy_instance->position.x, enemy_instance->position.y + 48 - 4, enemy_instance->attack_timer * 48, 2, BLUE);

      // energy
      draw_rect(enemy_instance->position.x, enemy_instance->position.y + 48 - 2, enemy_instance->energy * 48, 2, YELLOW);
    }
  }
}

static void s_create_board() {

  s_list_board_slots = create_list(sizeof(t_ui_button));
  for (int i = 0; i < BOARD_SIZE; i++) {

    const int x = (i % BOARD_COLS);
    const int y = (i / BOARD_COLS);

    const int pos_x = 16 + x * (48 + 8);
    const int pos_y = 16 + y * (48 + 8);

    board_slot* slot = malloc(sizeof(board_slot));
    slot->index = i;
    slot->character = NULL;
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
    slot_button->on_mouse_enter = s_on_board_slot_mouse_enter;
    slot_button->on_mouse_exit = s_on_board_slot_mouse_exit;
    slot_button->on_released = s_on_board_slot_clicked;

    add_to_list(s_list_board_slots, slot_button);
  }
}

static void s_create_characters_bench() {
  s_list_character_slots = create_list(sizeof(t_ui_button));
  for (int i = 0; i < CHARACTER_SLOTS_SIZE; i++) {

    character_slot* slot = malloc(sizeof(character_slot));
    slot->index = i;
    slot->character = NULL;

    t_ui_button* slot_button = malloc(sizeof(t_ui_button));

    *slot_button = create_ui_button(&s_sprite_border);
    slot_button->user_data = slot;
    slot_button->on_mouse_enter = s_on_character_slot_mouse_enter;
    slot_button->on_mouse_exit = s_on_character_slot_mouse_exit;
    slot_button->on_released = s_on_character_slot_clicked;

    add_to_list(s_list_character_slots, slot_button);
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

        ch->base_dmg = cJSON_GetObjectItem(json_array_item, "base_dmg")->valueint;
        ch->base_hp = cJSON_GetObjectItem(json_array_item, "base_hp")->valueint;
        ch->base_range = cJSON_GetObjectItem(json_array_item, "base_range")->valueint;
        ch->base_as = (float) cJSON_GetObjectItem(json_array_item, "base_as")->valuedouble;
        ch->base_ms = (float) cJSON_GetObjectItem(json_array_item, "base_ms")->valuedouble;
        ch->base_armor = 100;
        ch->base_crit_chance = 25;
        ch->base_magic_resist = 100;
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
      slot->index = i;
      slot->character = (character*)element_at_list(s_list_characters, t_random_int(0, s_list_characters->size));

      t_ui_button* slot_button = malloc(sizeof(t_ui_button));

      *slot_button = create_ui_button(slot->character->sprite);
      slot_button->user_data = slot;
      slot_button->on_mouse_enter = s_on_shop_slot_mouse_enter;
      slot_button->on_mouse_exit = s_on_shop_slot_mouse_exit;
      slot_button->on_released = s_on_shop_slot_clicked;

      add_to_list(s_list_shop_slots, slot_button);
    }
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

    t_load_texture_data_s(&s_sprite_sword, "./res/textures/sword.png");
    t_load_texture_data_s(&s_sprite_shield, "./res/textures/shield.png");
    t_load_texture_data_s(&s_sprite_bow, "./res/textures/bow.png");

    set_loading_progress(1);
    set_loading_finished();
    return 0;
}

void unload_setup_screen() {

}

t_scene *scenes = NULL;
unsigned int shader_program = 0;

static t_font s_font;
static t_font s_font_l;

void init_setup_screen() {
  process_gltf_file("./res/models/cube/Cube.gltf", &scenes);

  shader_program = t_create_shader_program("./res/shaders/model_shader.vs", "./res/shaders/model_shader.fs");

  s_font = load_ttf_font("./res/fonts/Eczar-Regular.ttf", 28);
  s_font_l = load_ttf_font("./res/fonts/Eczar-Regular.ttf", 34);

  t_init_sprite(&s_sprite_plus_sign);
  t_init_sprite(&s_sprite_swap_sign);
  t_init_sprite(&s_sprite_triangle);
  t_init_sprite(&s_sprite_gear);

  t_init_sprite(&s_sprite_sword);
  t_init_sprite(&s_sprite_shield);
  t_init_sprite(&s_sprite_bow);

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

  s_load_characters();
  s_create_shop();
  s_create_characters_bench();

  s_init_path();
  s_create_board();
}

vec3 cam_pos = {0, 0, 5};
vec3 cam_dir = {0, 0, -1};
vec3 cam_up = {0, 1, 0};

static float s_text_pos_timer;
static float s_text_fade_timer;

static t_vec2 s_text_origin;

static float s_text_pos_y_offset = 0;
static float s_text_alpha_offset = 1;

static bool s_crit = false;
static bool s_drawing_text = false;
static int s_random_number = 120;

static void s_draw_text() { 

  if (s_drawing_text) {

    t_color color = s_crit ? RED : WHITE;
    color.a *= s_text_alpha_offset;
    t_vec2 pos = s_text_origin;

    pos.y += s_text_pos_y_offset;

    t_ease_out_quint(&s_text_pos_timer, &s_text_pos_y_offset, 0, -50, 1.5f);
    t_ease_out_quint(&s_text_fade_timer, &s_text_alpha_offset, 1, 0, 1.5f);

    if (color.a == 0) { 

      s_drawing_text = false;

      s_text_pos_timer = 0;
      s_text_fade_timer = 0;
      s_text_pos_y_offset = 0;
      s_text_alpha_offset = 1;
    }

    char str_num[20];
    sprintf(str_num, "%d", s_random_number + (s_crit ? s_random_number / 2 : 0));
    if (!s_crit)
      draw_text_ttf(str_num, &s_font, pos, color, 0);
    else 
      draw_text_ttf(str_num, &s_font_l, pos, color, 0);
  }
  else {

    if (is_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
      s_drawing_text = true;
      s_text_origin = mouse_position();

      s_crit = t_random_float(0, 1) > .5f;
    }
  }
}

static void s_draw_shop() {

  t_draw_sprite(&s_sprite_panel, 64, 32 + s_offset_y_shop, 232, 128, CC_RED);

  for (int i = 0; i < SHOP_SIZE; i++) { 

    t_ui_button* shop_slot_button = element_at_list(s_list_shop_slots, i);
    shop_slot* shop_slot = shop_slot_button->user_data;

    const int pos_x = 80 + i * (38 + 16);
    const int pos_y = 48  + s_offset_y_shop;

    t_draw_sprite(&s_sprite_shop_slot, pos_x, pos_y, 38, 38, CC_DARK_RED);
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
    draw_text_ttf(text_cost, &s_font, (t_vec2) { pos_x + (38 - size_text_cost.x) / 2, pos_y + 38 + size_text_cost.y + 6}, CC_BLACK, 0);
  }

  draw_ui_button(&s_button_refresh_shop, 80, 121 + s_offset_y_shop, 96, 32);
  t_vec2 size_text_refresh = measure_text_size_ttf("Refresh", &s_font);
  draw_text_ttf("Refresh", &s_font, (t_vec2) {80 + (96 - size_text_refresh.x) / 2, 121 + (32 + size_text_refresh.y) / 2 + s_offset_y_shop}, CC_RED, 0);

  draw_ui_button(&s_button_lock_shop, 184, 121 + s_offset_y_shop, 96, 32);
  t_vec2 size_text_lock = measure_text_size_ttf("Lock", &s_font);
  draw_text_ttf("Lock", &s_font, (t_vec2) {184 + (96 - size_text_lock.x) / 2, 121 + (32 + size_text_lock.y) / 2 + s_offset_y_shop}, CC_RED, 0);
}

static void s_draw_inventory() { 

    t_draw_sprite(&s_sprite_shop_slot, 368, 24, 248, 132, CC_RED);

    // character slots
    int character_slot_index = 0;

    for (int x = 0; x < 2; x++) { 
      for (int y = 0; y < 6; y++) { 
        const int pos_x = 376 + y * (32 + 8);
        const int pos_y = 32 + x * (32 + 8);

        t_draw_sprite(&s_sprite_shop_slot, pos_x, pos_y, 32, 32, CC_DARK_RED);

        t_ui_button* slot_button = element_at_list(s_list_character_slots, character_slot_index);
        character_slot* slot = slot_button->user_data;

        if (slot->character != NULL)
          draw_ui_button(slot_button, pos_x, pos_y - 2, 32, 32);        

        character_slot_index++;
      }
    }

    draw_line(376, 110, 608, 110, 1, CC_DARK_RED);

    // item slots
    for (int i = 0; i < 6; i++) { 
      const int pos_x = 376 + i * (32 + 8);
      const int pos_y = 116;

      draw_rect_lines(pos_x, pos_y, 32, 32, CC_DARK_RED);
    }
}

static void s_draw_character_info() { 

    t_draw_sprite(&s_sprite_shop_slot, 368, 164, 248, 132, CC_RED);

    if (s_selected_character != NULL) { 

      // sprite
      t_draw_sprite(&s_sprite_panel, 376, 172, 64, 64, CC_DARK_RED);
      t_draw_sprite(s_selected_character->sprite, 376 + 8, 172 + 8, 48, 48, WHITE);

      // health
      draw_rect(376, 247, 58, 16, (t_color) { 74, 126, 49, 255 });
      draw_rect_lines(376, 247, 64, 16, CC_BLACK);

      char text_health[10];
      sprintf(text_health, "%d/%d", 673, 673);

      t_vec2 size_text_health = measure_text_size_ttf(text_health, &s_font);
      draw_text_ttf(text_health, &s_font, (t_vec2) { 376 + (64 - size_text_health.x) / 2, 247 + (16 + size_text_health.y) / 2}, CC_BLACK, 0);

      // energy
      draw_rect(376, 269, 58, 16, (t_color) { 177, 160, 1, 255 });
      draw_rect_lines(376, 269, 64, 16, CC_BLACK);

      char text_energy[10];
      sprintf(text_energy, "%d/%d", 100, 100);

      t_vec2 size_text_energy = measure_text_size_ttf(text_energy, &s_font);
      draw_text_ttf(text_energy, &s_font, (t_vec2) { 376 + (64 - size_text_energy.x) / 2, 269 + (16 + size_text_energy.y) / 2}, CC_BLACK, 0);

      // stats
      //  attack damage
      t_draw_sprite(&s_sprite_sword, 448, 178, 24, 24, CC_DARK_RED);
      
      char text_ad[5];
      sprintf(text_ad, "%u", s_selected_character->base_dmg);
      draw_text_ttf(text_ad, &s_font, (t_vec2) { 472, 178 + 20 }, CC_BLACK, 0);

      //  attack speed
      t_draw_sprite(&s_sprite_sword, 504, 178, 24, 24, CC_DARK_RED);

      char text_as[5];
      sprintf(text_as, "%.0f", s_selected_character->base_as);
      draw_text_ttf(text_as, &s_font, (t_vec2) { 530, 178 + 20 }, CC_BLACK, 0);

      //  crit chance
      t_draw_sprite(&s_sprite_sword, 560, 178, 24, 24, CC_DARK_RED);

      char text_crit_chance[5];
      sprintf(text_crit_chance, "%d%", s_selected_character->base_crit_chance);
      draw_text_ttf(text_crit_chance, &s_font, (t_vec2) { 586, 178 + 20 }, CC_BLACK, 0);

      //  armor
      t_draw_sprite(&s_sprite_sword, 448, 204, 24, 24, CC_DARK_RED);

      char text_armor[5];
      sprintf(text_armor, "%d", s_selected_character->base_armor);
      draw_text_ttf(text_armor, &s_font, (t_vec2) { 472, 204 + 20 }, CC_BLACK, 0);

      //  magic resist
      t_draw_sprite(&s_sprite_sword, 504, 204, 24, 24, CC_DARK_RED);

      char text_magic_resist[5];
      sprintf(text_magic_resist, "%d", s_selected_character->base_magic_resist);
      draw_text_ttf(text_magic_resist, &s_font, (t_vec2) { 530, 204 + 20 }, CC_BLACK, 0);

      //  range
      t_draw_sprite(&s_sprite_sword, 560, 204, 24, 24, CC_DARK_RED);

      char text_range[5];
      sprintf(text_range, "%d", s_selected_character->base_range);
      draw_text_ttf(text_range, &s_font, (t_vec2) { 586, 204 + 20 }, CC_BLACK, 0);

      // items

      for (int i = 0; i < 3; i ++) { 
        const int pos_x = 465 + i * (38 + 8);
        const int pos_y = 247;

        draw_rect_lines(pos_x, pos_y, 38, 38, CC_BLACK);
      }
    }
}

static void s_draw_game_info() {

   // currency
    char currency_str[10];
    sprintf(currency_str, "G: %u", s_currency);
    draw_text_ttf(currency_str, &s_font, (t_vec2) { 376, 306 + 18 }, CC_RED, 0);

    // level
    char level_str[10];
    sprintf(level_str, "L: %u", s_level);
    draw_text_ttf(level_str, &s_font, (t_vec2) { 427, 306 + 18 }, CC_RED, 0);

    // stage
    char stage_str[10];
    sprintf(stage_str, "S: %u", s_stage);
    draw_text_ttf(stage_str, &s_font, (t_vec2) { 474, 306 + 18 }, CC_RED, 0);
}

void draw_setup_screen() {

    unsigned int board_slot_index = 0;

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

          if (slot->character != NULL) {

            t_sprite* sprite_class = NULL;

            switch (slot->character->class) { 
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

          if (slot->character != NULL || s_selected_character_slot != NULL || s_selected_board_slot != NULL) {
            draw_ui_button(button, pos_x, pos_y, 48, 48);
          } else {
            draw_rect_lines(pos_x, pos_y, 48, 48, color);
          }

          if (s_selected_character_slot != NULL || s_selected_board_slot != NULL) {

            bool from_this_board_slot = s_selected_board_slot != NULL && s_selected_board_slot->character == slot->character;

            if (!from_this_board_slot) { 
              if (slot->character == NULL)
                t_draw_sprite(&s_sprite_plus_sign, pos_x, pos_y, 48, 48, color);
              else 
                t_draw_sprite(&s_sprite_swap_sign, pos_x, pos_y, 48, 48, color);
            }
          }

          if (slot->character != NULL) {
            t_draw_sprite(slot->character->sprite, pos_x, pos_y, 48, 48, WHITE);
          }
        } 

          board_slot_index++;
      }
    }
    
    draw_rect_lines(360, 16, 264, 328, CC_RED);

    unsigned int character_slot_index = 0;
    // // character slots
    // for (int y = 0; y < 2; y++) {
    //   for (int x = 0; x < 4; x++) {

    //     const int pos_x = 472 + x * (32 + 6);
    //     const int pos_y = 20 + y * (32 + 6);
    //     t_ui_button* character_slot_button = (t_ui_button*)element_at_list(s_list_character_slots, character_slot_index);
    //     character_slot* slot = (character_slot*)character_slot_button->user_data;

    //     if (slot->character != NULL || s_selected_board_slot != NULL) {
    //       if (slot->character != NULL)
    //         t_draw_sprite(slot->character->sprite, pos_x, pos_y, 32, 32, WHITE);

    //       draw_ui_button(character_slot_button, pos_x, pos_y, 32, 32);
    //     } else {
    //       draw_rect_lines(pos_x, pos_y, 32, 32, CC_RED);
    //     }

    //     if (s_selected_board_slot != NULL) {
    //       if (slot->character == NULL)
    //         t_draw_sprite(&s_sprite_plus_sign, pos_x, pos_y, 32, 32, CC_RED);
    //       else 
    //         t_draw_sprite(&s_sprite_swap_sign, pos_x, pos_y, 32, 32, CC_RED);
    //     }

    //     character_slot_index++;
    //   }
    // }

    // PREVIEW Rect

    if (s_is_playing) {
      play();
    }

    if (s_ease_out_shop) {

        float progress = t_ease_out_quint(&s_ease_timer_shop, &s_offset_y_shop, 0, -160, .5f);

        if (progress >= 1) {
            s_ease_timer_shop = 0;
            s_ease_out_shop = false;
        }
    }

    s_draw_shop();
    s_draw_inventory();
    s_draw_character_info();
    s_draw_game_info();

    if (s_phase == SHOP) { 
      draw_ui_button(&s_button_plan, 520, 304, 96, 32);

      t_vec2 text_plan_size = measure_text_size_ttf("Plan", &s_font_l);
      draw_text_ttf("Plan", &s_font_l, (t_vec2) { 520 + (96 - text_plan_size.x) / 2, 304 + (32 + text_plan_size.y) / 2 }, CC_BLACK, 0);
    }
    else if (s_phase == PLAN) { 
      draw_ui_button(&s_button_play, 520, 304, 96, 32);

      t_vec2 text_start_size = measure_text_size_ttf("Start", &s_font_l);
      draw_text_ttf("Start", &s_font_l, (t_vec2) { 520 + (96 - text_start_size.x) / 2, 304 + (32 + text_start_size.y) / 2 }, CC_BLACK, 0);
    }

    t_draw_sprite(&s_sprite_gear, 1, 1, 32, 32, CC_RED);
}