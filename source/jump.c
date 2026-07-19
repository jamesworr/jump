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
#include "turby.h"

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

#define PLAYER_HEIGHT 32
#define PLAYER_WIDTH  16
#define PLAYER_TID    68
#define WALK_1_TID    84
#define WALK_2_TID   100
#define PLAYER_PUNCH 132

#define PLAYER_SWITCH_MAP_LEFT    8
#define PLAYER_SWITCH_MAP_RIGHT 500

#define SCORPION_HEIGHT  32
#define SCORPION_WIDTH   16
#define SCORPION_TID      4
#define CLAW_LEFT_TID    10
#define CLAW_RIGHT_TID   36
#define SCORPION_RED_TID 52

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
    //u16 x;
    //u16 y;
    short x;
    short y;

    // relative position within the screen
    // x = 255 means outsode current screen
    // and do not render or collide
    u8 screen_x;
    u8 screen_y;

    // 0: left
    // 1: center
    // 2: right
    u8 current_map;

    // Tile ID for sprite animation
    u8 tid;

    // Movement directions
    short move_x;
    short move_y;

} scorpion_t;

// Returns wall collision status
// 0x01: right
// 0x02: left
// 0x04: up
// 0x08: down
u8 check_player_wall_collision(player_t* player) {
    const unsigned short* collision_map;
    switch (player->current_map) {
        case 0:
            collision_map = left_house_collision_map;
            break;
        case 1:
            collision_map = center_house_collision_map;
            break;
        case 2:
            collision_map = right_house_collision_map;
            break;
    }

    // quantize player location into 8x8 BG tiles
    volatile u8 tile_x = (player->x) >> 3;
    volatile u8 tile_y = (player->y) >> 3;

    volatile u16 tile_value; // TODO get rid of me and put the collision_map[] into if()
    volatile u16 tile_offset;
    tile_offset = (tile_y<<6)+tile_x;
    tile_value = collision_map[tile_offset];
    if (tile_value == WALL_TID) {
        return 1;
    }
    tile_offset = ((tile_y+3)<<6)+tile_x;
    tile_value = collision_map[tile_offset];
    if (tile_value == WALL_TID) {
        return 1;
    }
    tile_offset = (tile_y<<6)+(tile_x+1);
    tile_value = collision_map[tile_offset];
    if (tile_value == WALL_TID) {
        return 1;
    }
    tile_offset = ((tile_y+3)<<6)+(tile_x+1);
    tile_value = collision_map[tile_offset];
    if (tile_value == WALL_TID) {
        return 1;
    }

    return 0;
}

// Returns wall collision status
// 0x01: right
// 0x02: left
// 0x04: up
// 0x08: down
// FIXME make this shared with player but I'm out of time
u8 check_scorpion_wall_collision(scorpion_t* scorpion) {
    const unsigned short* collision_map = center_house_collision_map;
    /*
    switch (scorpion->current_map) {
        case 0:
            collision_map = left_house_collision_map;
            break;
        case 1:
            collision_map = center_house_collision_map;
            break;
        case 2:
            collision_map = right_house_collision_map;
            break;
    }
    */

    // quantize scorpion location into 8x8 BG tiles
    volatile u8 tile_x = (scorpion->x) >> 3;
    volatile u8 tile_y = (scorpion->y) >> 3;

    volatile u16 tile_value; // TODO get rid of me and put the collision_map[] into if()
    volatile u16 tile_offset;
    tile_offset = (tile_y<<6)+tile_x;
    tile_value = collision_map[tile_offset];
    if (tile_value == WALL_TID) {
        return 1;
    }
    tile_offset = ((tile_y+3)<<6)+tile_x;
    tile_value = collision_map[tile_offset];
    if (tile_value == WALL_TID) {
        return 2;
    }
    tile_offset = (tile_y<<6)+(tile_x+1);
    tile_value = collision_map[tile_offset];
    if (tile_value == WALL_TID) {
        return 3;
    }
    tile_offset = ((tile_y+3)<<6)+(tile_x+1);
    tile_value = collision_map[tile_offset];
    if (tile_value == WALL_TID) {
        return 4;
    }

    return 0;
}

u8 update_scorpion_position(scorpion_t* scorpion) {
    scorpion->x += scorpion->move_x;
    scorpion->y += scorpion->move_y;
    volatile u8 collision_side = check_scorpion_wall_collision(scorpion);
    if (collision_side) {
        // revert move and change direction on collision
        scorpion->x -= scorpion->move_x;
        scorpion->y -= scorpion->move_x;
        switch (collision_side) {
            case 1:
                scorpion->move_x =  0;
                scorpion->move_y =  1;
                break;
            case 2:
                scorpion->move_x =  0;
                scorpion->move_y = -1;
                break;
            case 3:
                scorpion->move_x =  0;
                scorpion->move_y =  1;
                break;
            case 4:
                scorpion->move_x =  0;
                scorpion->move_y = -1;
                break;
        }
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

void check_player_map_swap(player_t* player) {
    if (player->x < PLAYER_SWITCH_MAP_LEFT) {
        if (player->current_map == 1) {
            // center moving to left
            swap_player_map(player, 0, 497, 246, 221, 79, 272, 163);
            //swap_player_map(player, 0, 256, 256, CENTER_X, CENTER_Y, BG_START_COORD_X, BG_START_COORD_Y);
            REG_BG0CNT = BG_CBB(0) | BG_SBB(8) | BG_8BPP | BG_REG_64x64;
        }
        else {
            // implied player->current_map == 2
            // right moving to center
            swap_player_map(player, 1, 496, 364, 220, 81, 272, 279);
            //swap_player_map(player, 1, 256, 256, CENTER_X, CENTER_Y, BG_START_COORD_X, BG_START_COORD_Y);
            REG_BG0CNT = BG_CBB(0) | BG_SBB(12) | BG_8BPP | BG_REG_64x64;
        }
    }
    else if (player->x > PLAYER_SWITCH_MAP_RIGHT) {
        if (player->current_map == 0) {
            // left moving to center
            swap_player_map(player, 1, 13, 170, 9, 79, 0, 87);
            //swap_player_map(player, 1, 8, 157, 4, 79, 0, 74);
            //swap_player_map(player, 1, 256, 256, CENTER_X, CENTER_Y, BG_START_COORD_X, BG_START_COORD_Y);
            REG_BG0CNT = BG_CBB(0) | BG_SBB(12) | BG_8BPP | BG_REG_64x64;
        }
        else {
            // implied player->current_map == 1
            // center moving to right
            swap_player_map(player, 2, 8, 238, 4, 79, 0, 155);
            //swap_player_map(player, 2, 256, 256, CENTER_X, CENTER_Y, BG_START_COORD_X, BG_START_COORD_Y);
            REG_BG0CNT = BG_CBB(0) | BG_SBB(16) | BG_8BPP | BG_REG_64x64;
        }
    }
}

u8 update_player_position(player_t* player) {
    // return moving state
    u8 walking = 0;
    // Moving up
    if (key_is_down(KEY_UP) && ((player->bg_vert <= BG_SCROLL_X_LOWER) || (player->screen_y >= CENTER_Y))) {
        player->screen_y--;
        player->y--;
        walking = 1;
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
        walking = 1;
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
        walking = 1;
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
        walking = 1;
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
        walking = 1;
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
        walking = 1;
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
        walking = 1;
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
        walking = 1;
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

    return walking;
}

// Returns wall collision status
// 0x01: right
// 0x02: left
// 0x04: up
// 0x08: down
u8 check_player_scorpion_collision(player_t* player, scorpion_t* scorpion) {
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
    if (visible == 1) {
        obj_buffer[1].attr0 = (obj_buffer[0].attr0 & 0xFCFF) | ATTR0_AFF_DBL;
    }
    else {
        obj_buffer[1].attr0 = (obj_buffer[0].attr0 & 0xFCFF) | ATTR0_HIDE;
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

    // Check if we're even in the same map room
    if (scorpion->current_map != player->current_map) {
        set_scorpion_visibility(0);
        return 0;
    }

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
    if (scorpion->y > (player->bg_vert + SCREEN_HEIGHT)) {
        set_scorpion_visibility(0);
        return 0;
    }
    else if (scorpion->y >= player->bg_vert) {
        // Check actual overlap
        // A >= C
        y_overlap = 1;
    }
    // B < C
    // TODO need to refactor the bail out logic but Im way too lazy and tired
    if (y_overlap == 0) {
        if ((scorpion->y + SCORPION_HEIGHT) < player->bg_vert) {
            set_scorpion_visibility(0);
            return 0;
        }
        else { 
            // Check actual overlap
            // B <= D
            if ((scorpion->y + SCORPION_HEIGHT) <= (player->bg_vert + SCREEN_HEIGHT)) {
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

    // Setup background and display registers
    REG_BG0CNT = BG_CBB(0) | BG_SBB(12) | BG_8BPP | BG_REG_64x64;
}

void update_player_direction(void) {
    // Pre-calculated LUTs for affine rotations
    if (key_is_down(KEY_UP))  {
        if (key_is_down(KEY_RIGHT)) {
            obj_aff_set(&obj_aff_buffer[0], 160, 200, -200, 160);
        }
        else if (key_is_down(KEY_LEFT)) {
            obj_aff_set(&obj_aff_buffer[0], 160, -200, 200, 160);
        }
        else {
            obj_aff_identity(&obj_aff_buffer[0]);
        }
    }
    else if (key_is_down(KEY_DOWN))  {
        if (key_is_down(KEY_RIGHT)) {
            obj_aff_set(&obj_aff_buffer[0], -160, 200, -200, -160);
        }
        else if (key_is_down(KEY_LEFT)) {
            obj_aff_set(&obj_aff_buffer[0], -160, -200, 200, -160);
        }
        else {
            obj_aff_set(&obj_aff_buffer[0], -256, 0, 0, -256);
        }
    }
    else if (key_is_down(KEY_RIGHT))  {
        obj_aff_set(&obj_aff_buffer[0], 0, 256, -256, 0);
    }
    else if (key_is_down(KEY_LEFT))  {
        obj_aff_set(&obj_aff_buffer[0], 0, -256, 256, 0);
    }
}

void player_animation(player_t* player, u8 walking, unsigned int* frame_counter, u8 collision) {
    if (key_is_down(KEY_R)) {
        // Turby punching animation
        obj_buffer[0].attr2 = PLAYER_PUNCH;
        if (collision) {
            // Make scorpion red on contact
            // I know this would be cheaper with palette swapping but I'm lazy
            obj_buffer[1].attr2 = SCORPION_RED_TID;
        }
    }
    else if (walking == 1) {
        // Cycle through walking animation frames on counter
        // 15 frames should be 4Hz
        if (*frame_counter > 15) {
            *frame_counter = 0;
            player->tid++;
            player->tid %= 4;
        }
        switch (player->tid) {
            case 0:
                obj_buffer[0].attr2 = PLAYER_TID;
                break;
            case 1:
                obj_buffer[0].attr2 = WALK_1_TID;
                break;
            case 2:
                obj_buffer[0].attr2 = PLAYER_TID;
                break;
            case 3:
                obj_buffer[0].attr2 = WALK_2_TID;
                break;
        }
        obj_buffer[1].attr2 = SCORPION_TID;
    }
    else {
        // reset to default frames
        obj_buffer[0].attr2 = PLAYER_TID;
        obj_buffer[1].attr2 = SCORPION_TID;
    }
}

void sprite_loop(player_t* player, scorpion_t* scorpion) {
    OBJ_ATTR *block_obj;

    volatile unsigned int frame_counter = 0;
    volatile u8 collision = 0;
    volatile u8 scorpion_collision = 0;
    volatile u8 walking = 0;
    volatile unsigned int rotation = 0;

    // Open Items
    // TODO move scorpion XY
    // TODO fix off by one issue when changing direction
    // TODO fix janky door crossing with affine

    while(1) {
        vid_vsync();
        key_poll();
        if (key_hit(KEY_SELECT)) {
            break; // Break out of waiting loop and restart
        }
        // Game Loop
        // Check collision
        check_player_wall_collision(player);

        if (frame_counter & 0x01) {
            update_scorpion_position(scorpion);
        }
        if (check_scorpion_visible(player, scorpion)) {
            // Only check collision when visible
            scorpion_collision = check_player_scorpion_collision(player, scorpion);
            update_scorpion_screen_position(player, scorpion);
        }

        walking = update_player_position(player);
        update_player_direction();
        player_animation(player, walking, &frame_counter, scorpion_collision);

        oam_copy(oam_mem, obj_buffer, 2);
		obj_aff_copy(obj_aff_mem, obj_aff_buffer, 2);
        frame_counter++;
    }
}

int main() {
    while(1) {
        // Init player struct
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
            .current_map = 1, // TODO randomize
            .tid = SCORPION_TID,
            .move_x =  0,
            .move_y = -1
        };

        //opening_sequence();
        init_bg();

        oam_init(obj_buffer, 128);

        // Copy sprite tiles
        memcpy32(&tile_mem[4][0], man_tiles, man_tiles_len / sizeof(u32));
        memcpy32(&tile_mem[4][4], scorpion_tiles, scorpion_tiles_len / sizeof(u32));
        memcpy32(&tile_mem[4][68], turby_tiles, turby_tiles_len / sizeof(u32));

        // Copy shared sprite palette
        memcpy16(pal_obj_mem, scorpion_pal, scorpion_pal_len / sizeof(u16));

        // Initialize the person
        obj_set_attr(&obj_buffer[0],
            ATTR0_TALL | ATTR0_8BPP | ATTR0_AFF_DBL,
            ATTR1_SIZE_16x32 | ATTR1_AFF_ID(0),
            ATTR2_PALBANK(0) | PLAYER_TID);
        obj_set_pos(&obj_buffer[0], player.screen_x, player.screen_y);
        obj_aff_identity(&obj_aff_buffer[0]);

        // Initialize the scorpion
        obj_set_attr(&obj_buffer[1],
            ATTR0_TALL | ATTR0_8BPP | ATTR0_AFF_DBL,
            ATTR1_SIZE_16x32 | ATTR1_AFF_ID(1),
            ATTR2_PALBANK(0) | SCORPION_TID);
        obj_set_pos(&obj_buffer[1], scorpion.screen_x,scorpion.screen_y);
        obj_aff_identity(&obj_aff_buffer[1]);
        set_scorpion_visibility(0);

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
