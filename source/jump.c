//
// Will 4/21/26
//

#include <string.h>
#include <tonc.h>
#include "man.h"
#include "left_house.h"
#include "center_house.h"
#include "right_house.h"
#include "scorpion.h"

OBJ_ATTR obj_buffer[128];
OBJ_AFFINE *obj_aff_buffer= (OBJ_AFFINE*)obj_buffer;

#define CBB_0  0
#define SBB_0 28

// FIXME refactor the flipped names
#define BG_SCROLL_X_LOWER 0
#define BG_SCROLL_X_UPPER 352
#define BG_SCROLL_Y_LOWER 0
#define BG_SCROLL_Y_UPPER 272

// 512 px map / 2 = 256
// 240 px screen / 2 = 120
// 8 px wide sprite / 2 = 4
// 256 - 120 - 4 = 132
#define BG_START_COORD_X 132
// 512 px map / 2 = 256
// 160 px screen / 2 = 80
// 16 px high sprite / 2 = 8
// 256 - 80 - 8 = 168
#define BG_START_COORD_Y 172

#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 120

#define CENTER_X 120
#define CENTER_Y 80

#define PLAYER_HEIGHT 16
#define PLAYER_WIDTH   8
#define PLAYER_TID     0

#define PLAYER_SWITCH_MAP_LEFT    8
#define PLAYER_SWITCH_MAP_RIGHT 505

#define SCORPION_HEIGHT 32
#define SCORPION_WIDTH  16
#define SCORPION_TID     4

// Collision stuff
#define WALL_TID 0

void set_scorpion_visibility(u8 visible);
// player object
typedef struct {
    // absolute position within the current map
    u16 x;
    u16 y;

    // relative position within the screen
    // needed for movement on edges when
    // background scrolling runs out
    // normally centered (120, 80)
    u8 screen_x;
    u8 screen_y;

    // Tracks background scrolling location
    u16 bg_horz;
    u16 bg_vert;

    // 0: left
    // 1: center
    // 2: right
    u8 current_map;

    // Tile ID for sprite animation
    u8 tid;

} player_t;

// scorpion object
typedef struct {
    // absolute position within the current map
    u16 x;
    u16 y;

    // relative position within the screen
    // x = 255 means outsode current screen
    // and do not render or collide
    u8 screen_x;
    u8 screen_y;

    // Tile ID for sprite animation
    u8 tid;

} scorpion_t;

// Returns wall collision status
// 0x01: right
// 0x02: left
// 0x04: up
// 0x08: down
u8 check_player_wall_collision(player_t* player) {
    // TODO
    // need to account for the weird 64x64 tile vram layout
    // OR just be lazy and have PIG generate a collision map

    // quantize player location into 8x8 BG tiles
    u8 upper_tile_x = (player->x) >> 3;
    u8 upper_tile_y = (player->y) >> 3;
    u8 lower_tile_x = (player->x + PLAYER_WIDTH ) >> 3;
    u8 lower_tile_y = (player->y + PLAYER_HEIGHT) >> 3;

    volatile u16 tile_map_offset = (upper_tile_y<<6)+upper_tile_x;
    //volatile u16 tile_value = left_house_collision_map[tile_map_offset];
    //volatile u16 tile_value = center_house_collision_map[tile_map_offset];

    // TODO store pointer to current collision map in player struct
    // so we don't have to use the switch every time
    volatile u16 tile_value;
    switch (player->current_map) {
        case 0:
            tile_value = left_house_collision_map[tile_map_offset];
            break;
        case 1:
            tile_value = center_house_collision_map[tile_map_offset];
            break;
        case 2:
            tile_value = right_house_collision_map[tile_map_offset];
            break;
    }
    volatile u16 tile_value3 = left_house_collision_map[0]; // TODO delete me placeholder for breakpoint

    // TODO also need to check the 2nd tile
    if (tile_value == WALL_TID) {
        return 1;
    }
    else {
        return 0;
    }
}

void swap_player_map(player_t* player, u8 new_map, u16 new_x, u16 new_y, u16 new_screen_x, u16 new_screen_y, u16 new_bg_horz, u16 new_bg_vert) {
    player->current_map = new_map;
    player->x = new_x;
    player->y = new_y;
    player->screen_x = new_screen_x;
    player->screen_y = new_screen_y;
    player->bg_horz = new_bg_horz;
    player->bg_vert = new_bg_vert;
}

// FIXME move the bg control reg update into swap_player_map
void check_player_map_swap(player_t* player) {
    if (player->x < PLAYER_SWITCH_MAP_LEFT) {
        if (player->current_map == 1) {
            // center moving to left
            swap_player_map(player, 0, 505, 256, 229, 80, 272, 172);
            REG_BG0CNT = BG_CBB(0) | BG_SBB(8) | BG_8BPP | BG_REG_64x64;
        }
        else {
            // implied player->current_map == 2
            // right moving to center
            swap_player_map(player, 1, 502, 376, 226, 81, 272, 291);
            REG_BG0CNT = BG_CBB(0) | BG_SBB(12) | BG_8BPP | BG_REG_64x64;
        }
    }
    else if (player->x > PLAYER_SWITCH_MAP_RIGHT) {
        if (player->current_map == 0) {
            // left moving to center
            swap_player_map(player, 1, 13, 170, 9, 79, 0, 87);
            REG_BG0CNT = BG_CBB(0) | BG_SBB(12) | BG_8BPP | BG_REG_64x64;
        }
        else {
            // implied player->current_map == 1
            // center moving to right
            swap_player_map(player, 2, 10, 249, 6, 79, 0, 166);
            REG_BG0CNT = BG_CBB(0) | BG_SBB(16) | BG_8BPP | BG_REG_64x64;
        }
    }
}

void update_player_position(player_t* player) {
    // Moving up
    if (key_is_down(KEY_UP) && ((player->bg_vert <= BG_SCROLL_X_LOWER) || (player->screen_y >= CENTER_Y))) {
        player->screen_y--;
        player->y--;
        // Check collison on new move TODO find better way
        if (check_player_wall_collision(player) == 1) {
            // revert
            player->screen_y++;
            player->y++;
        }
    }
    else if (key_is_down(KEY_UP) && (player->bg_vert >= BG_SCROLL_X_LOWER)) {
        player->bg_vert--;
        player->y--;
        // Check collison on new move TODO find better way
        if (check_player_wall_collision(player) == 1) {
            // revert
            player->bg_vert++;
            player->y++;
        }
    }

    // Moving down
    if (key_is_down(KEY_DOWN) && ((player->bg_vert >= BG_SCROLL_X_UPPER) || (player->screen_y <= CENTER_Y))) {
        player->screen_y++;
        player->y++;
        // Check collison on new move TODO find better way
        if (check_player_wall_collision(player) == 1) {
            // revert
            player->screen_y--;
            player->y--;
        }
    }
    else if (key_is_down(KEY_DOWN) && (player->bg_vert <= BG_SCROLL_X_UPPER)) {
        player->bg_vert++;
        player->y++;
        // Check collison on new move TODO find better way
        if (check_player_wall_collision(player) == 1) {
            // revert
            player->bg_vert--;
            player->y--;
        }
    }

    // Moving left
    if (key_is_down(KEY_LEFT) && ((player->bg_horz <= BG_SCROLL_Y_LOWER) || (player->screen_x >= CENTER_X))) {
        player->screen_x--;
        player->x--;
        // Check collison on new move TODO find better way
        if (check_player_wall_collision(player) == 1) {
            // revert
            player->screen_x++;
            player->x++;
        }
    }
    else if (key_is_down(KEY_LEFT) && (player->bg_horz >= BG_SCROLL_Y_LOWER)) {
        player->bg_horz--;
        player->x--;
        // Check collison on new move TODO find better way
        if (check_player_wall_collision(player) == 1) {
            // revert
            player->bg_horz++;
            player->x++;
        }
    }

    // Moving right
    if (key_is_down(KEY_RIGHT) && ((player->bg_horz >= BG_SCROLL_Y_UPPER) || (player->screen_x <= CENTER_X))) {
        player->screen_x++;
        player->x++;
        // Check collison on new move TODO find better way
        if (check_player_wall_collision(player) == 1) {
            // revert
            player->screen_x--;
            player->x--;
        }
    }
    else if (key_is_down(KEY_RIGHT) && (player->bg_horz <= BG_SCROLL_Y_UPPER)) {
        player->bg_horz++;
        player->x++;
        // Check collison on new move TODO find better way
        if (check_player_wall_collision(player) == 1) {
            // revert
            player->bg_horz--;
            player->x--;
        }
    }

    check_player_map_swap(player);

    REG_BG0HOFS = player->bg_horz;
    REG_BG0VOFS = player->bg_vert;

    obj_set_pos(&obj_buffer[0], player->screen_x, player->screen_y);
}

// Returns wall collision status
// 0x01: right
// 0x02: left
// 0x04: up
// 0x08: down
u8 check_player_scorpion_collision(player_t* player, scorpion_t* scorpion) {
    // FIXME need to check the ordering on big vs small once final player sprite in place

    // a = x min scorpion
    // b = x max scorpion
    // c = x min player
    // d = x max player

    volatile u8 x_overlap = 0;
    volatile u8 y_overlap = 0;
    // Check early bail out scenarios first
    // A > D
    if (scorpion->x > (player->x + PLAYER_WIDTH)) {
        return 0;
    }
    else if (scorpion->x >= player->x) {
        // Check actual overlap
        // A >= C
        x_overlap = 1;
    }
    // B < C
    // TODO need to refactor the bail out logic but Im way too lazy and tired
    if (x_overlap == 0) {
        if ((scorpion->x + SCORPION_WIDTH) < player->x) {
            return 0;
        }
        else { 
            // Check actual overlap
            // B <= D
            if ((scorpion->x + SCORPION_WIDTH) <= (player->x + PLAYER_WIDTH)) {
                x_overlap = 1;
            }
        }
    }
    if (x_overlap == 0) {
        return 0;
    }

    // Vertical
    // Check early bail out scenarios first
    // A > D
    if (scorpion->y > (player->y + SCREEN_HEIGHT)) {
        return 0;
    }
    else if (scorpion->y >= player->y) {
        // Check actual overlap
        // A >= C
        y_overlap = 1;
    }
    // B < C
    // TODO need to refactor the bail out logic but Im way too lazy and tired
    if (y_overlap == 0) {
        if ((scorpion->y + SCORPION_HEIGHT) < player->y) {
            return 0;
        }
        else { 
            // Check actual overlap
            // B <= D
            if ((scorpion->y + SCORPION_HEIGHT) <= (player->y + SCREEN_HEIGHT)) {
                y_overlap = 1;
            }
        }
    }
    if (y_overlap == 0) {
        return 0;
    }

    return 1;
}

void set_scorpion_visibility(u8 visible) {
    // TODO add pointer inside scorpion struct to the
    // OAM object buffer so this is reusable
    if (visible == 1) {
        obj_buffer[1].attr0 = obj_buffer[0].attr0 & 0xFCFF;
    }
    else {
        obj_buffer[1].attr0 = obj_buffer[0].attr0 | ATTR0_HIDE;
    }
}

u8 check_scorpion_visible(player_t* player, scorpion_t* scorpion) {
    // basically need to check collision of the bg scroll area
    // with the outline of the scorpion. If "collision" then
    // we enable scopion visibility and give it a valid screen_x/y
    // otherwise keep the visibility disabled
    //
    // a = x min scorpion
    // b = x max scorpion
    // c = x min screen
    // d = x max screen

    volatile u8 x_overlap = 0;
    volatile u8 y_overlap = 0;
    // Check early bail out scenarios first
    // A > D
    if (scorpion->x > (player->bg_horz + SCREEN_WIDTH)) {
        set_scorpion_visibility(0);
        return 0;
    }
    else if (scorpion->x >= player->bg_horz) {
        // Check actual overlap
        // A >= C
        x_overlap = 1;
    }
    // B < C
    // TODO need to refactor the bail out logic but Im way too lazy and tired
    if (x_overlap == 0) {
        if ((scorpion->x + SCORPION_WIDTH) < player->bg_horz) {
            set_scorpion_visibility(0);
            return 0;
        }
        else { 
            // Check actual overlap
            // B <= D
            if ((scorpion->x + SCORPION_WIDTH) <= (player->bg_horz + SCREEN_WIDTH)) {
                x_overlap = 1;
            }
        }
    }
    if (x_overlap == 0) {
        set_scorpion_visibility(0);
        return 0;
    }

    // Vertical
    // Check early bail out scenarios first
    // A > D
    if (scorpion->y > (player->bg_horz + SCREEN_HEIGHT)) {
        set_scorpion_visibility(0);
        return 0;
    }
    else if (scorpion->y >= player->bg_horz) {
        // Check actual overlap
        // A >= C
        y_overlap = 1;
    }
    // B < C
    // TODO need to refactor the bail out logic but Im way too lazy and tired
    if (y_overlap == 0) {
        if ((scorpion->y + SCORPION_HEIGHT) < player->bg_horz) {
            set_scorpion_visibility(0);
            return 0;
        }
        else { 
            // Check actual overlap
            // B <= D
            if ((scorpion->y + SCORPION_HEIGHT) <= (player->bg_horz + SCREEN_HEIGHT)) {
                y_overlap = 1;
            }
        }
    }
    if (y_overlap == 0) {
        set_scorpion_visibility(0);
        return 0;
    }

    set_scorpion_visibility(1);
    return 1;
}

void update_scorpion_screen_position(player_t* player, scorpion_t* scorpion) {
    volatile short scorpion_offset_x = scorpion->x - player->x;
    volatile short scorpion_offset_y = scorpion->y - player->y;

    volatile short scorpion_screen_x = player->screen_x + scorpion_offset_x;
    volatile short scorpion_screen_y = player->screen_y + scorpion_offset_y;

    obj_set_pos(&obj_buffer[1], scorpion_screen_x, scorpion_screen_y);
}

//BG_POINT bg0_pt= { 0, 0 };
//SCR_ENTRY *bg0_map= se_mem[SBB_0];

void wait_any_key(void) {
    while(1) {
        vid_vsync();
        key_poll();
        if(key_hit(KEY_ANY)) {
            break;
        }
    }
}

void init_bg() {
    // Copy background tiles
	// Load shared palette
	memcpy16(pal_bg_mem, left_house_pal, left_house_pal_len / sizeof(u16));
	// Load shared tiles into CBB 0
	memcpy32(&tile_mem[0][0], left_house_tiles, left_house_tiles_len / sizeof(u32));
	// Load left map into SBB 8
	memcpy32(&se_mem[8][0], left_house_map, left_house_map_len / sizeof(u32));
	// Load center map into SBB 12
	memcpy32(&se_mem[12][0], center_house_map, center_house_map_len / sizeof(u32));
	// Load right map into SBB 16
	memcpy32(&se_mem[16][0], right_house_map, right_house_map_len / sizeof(u32));

    // TODO copy center and right
    // TODO see if we can fit all 3 BG maps into separate SBB
    // but keep common tileset

    // Setup background and display registers
    // REG_BG0CNT = BG_CBB(0) | BG_SBB(8) | BG_8BPP | BG_REG_64x64; FIXME
    REG_BG0CNT = BG_CBB(0) | BG_SBB(12) | BG_8BPP | BG_REG_64x64;
}

void sprite_loop(player_t* player, scorpion_t* scorpion) {
    OBJ_ATTR *block_obj;

    volatile int frame_counter = 0;
    volatile u8 collision = 0;
    volatile u8 scorpion_collision = 0;

    while(1) {
        vid_vsync();
        key_poll();
        if (key_hit(KEY_SELECT)) {
            break; // Break out of waiting loop and restart
        }
        // Game Loop
        // Check collision
        // TODO
        check_player_wall_collision(player);

        if (check_scorpion_visible(player, scorpion)) {
            // Only check collision when visible
            scorpion_collision = check_player_scorpion_collision(player, scorpion);
            // TODO move scorpion XY
            update_scorpion_screen_position(player, scorpion);
        }

        // TODO fix off by one issue when changing direction
        update_player_position(player);

        oam_copy(oam_mem, obj_buffer, 2);
        frame_counter++;
    }
}

int main() {
    while(1) {
        // Init player struct
        // TODO have init locations per room
        volatile player_t player = {
            .x = 256,
            .y = 256,
            .screen_x = CENTER_X,
            .screen_y = CENTER_Y,
            .bg_horz = BG_START_COORD_X,
            .bg_vert = BG_START_COORD_Y,
            .current_map = 1,
            .tid = PLAYER_TID
        };

        volatile scorpion_t scorpion = {
            .x = 20, // TODO randomize
            .y = 20,
            .screen_x = 255,
            .screen_y = 0,
            .tid = SCORPION_TID
        };

        //opening_sequence();
        init_bg();

        // TODO move into separate init function
        //oam_init(obj_buffer, 128);
        oam_init(obj_buffer, 128);

        // Copy sprite tiles
        memcpy32(&tile_mem[4][0], man_tiles, man_tiles_len / sizeof(u32));
        memcpy32(&tile_mem[4][4], scorpion_tiles, scorpion_tiles_len / sizeof(u32));

        // Copy shared sprite palette
        memcpy16(pal_obj_mem, man_pal, man_pal_len / sizeof(u16));

        // Initialize the person
        obj_set_attr(&obj_buffer[0],
            ATTR0_TALL | ATTR0_8BPP,
            ATTR1_SIZE_8x16,
            ATTR2_PALBANK(0) | PLAYER_TID);
        obj_set_pos(&obj_buffer[0], player.screen_x, player.screen_y);
        //obj_set_pos(&obj_buffer[0], 100, 100);

        // Initialize the scorpion
        // TODO make this a loop for multiple
        obj_set_attr(&obj_buffer[1],
            ATTR0_TALL | ATTR0_8BPP,
            ATTR1_SIZE_16x32,
            ATTR2_PALBANK(0) | SCORPION_TID);
        obj_set_pos(&obj_buffer[1], scorpion.screen_x,scorpion.screen_y);
        set_scorpion_visibility(0);
        //obj_set_pos(&obj_buffer[1], 100, 100);

        REG_DISPCNT = DCNT_MODE0 | DCNT_BG0 | DCNT_OBJ | DCNT_OBJ_1D;

        sprite_loop(&player, &scorpion);

        // Waiting for new commands
        while(1) {
            vid_vsync();
            key_poll();
            if (key_hit(KEY_SELECT)) {
                break; // Break out of waiting loop and restart
            }
        }
    }
    return 0;
}
