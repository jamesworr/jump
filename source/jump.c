//
// Will 4/21/26
//

#include <string.h>
#include <tonc.h>
#include "man.h"
#include "left_house.h"

OBJ_ATTR obj_buffer[128];
OBJ_AFFINE *obj_aff_buffer= (OBJ_AFFINE*)obj_buffer;

#define CBB_0  0
#define SBB_0 28

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

#define CENTER_X 120
#define CENTER_Y 80

#define PLAYER_HEIGHT 16
#define PLAYER_WIDTH   8

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

    // Tile ID for sprite animation
    u8 tid;

} player_t;

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

    // TODO figure out the offsets with the weird 64x64 tile vram layout
    volatile u16 tile_value = left_house_collision_map[0];
    volatile u16 tile_map_offset = (upper_tile_y<<6)+upper_tile_x;
    volatile u16 tile_value2 = left_house_collision_map[tile_map_offset];
    volatile u16 tile_value3 = left_house_collision_map[0]; // TODO delete me placeholder for breakpoint
    return 0;
}

void update_player_position(player_t* player) {
    // TODO write movement update function that 
    // either scrolls the BG when possible or moves sprite on edges
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
	// Load palette
	memcpy16(pal_bg_mem, left_house_pal, left_house_pal_len / sizeof(u16));
	// Load tiles into CBB 0
	memcpy32(&tile_mem[0][0], left_house_tiles, left_house_tiles_len / sizeof(u32));
	// Load map into SBB 8
	memcpy32(&se_mem[8][0], left_house_map, left_house_map_len / sizeof(u32));

    // TODO copy center and right
    // TODO see if we can fit all 3 BG maps into separate SBB
    // but keep common tileset

    // Setup background and display registers
    REG_BG0CNT  = BG_CBB(0) | BG_SBB(8) | BG_8BPP | BG_REG_64x64;
}

void sprite_loop(player_t* player) {
    OBJ_ATTR *block_obj;

    volatile int frame_counter = 0;
    volatile u8 collision = 0;

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

        // TODO fix off by one issue when changing direction
        // Moving up
        if (key_is_down(KEY_UP) && ((player->bg_vert <= BG_SCROLL_X_LOWER) || (player->screen_y >= CENTER_Y))) {
            player->screen_y--;
            player->y--;
        }
        else if (key_is_down(KEY_UP) && (player->bg_vert >= BG_SCROLL_X_LOWER)) {
            player->bg_vert--;
            player->y--;
        }

        // Moving down
        if (key_is_down(KEY_DOWN) && ((player->bg_vert >= BG_SCROLL_X_UPPER) || (player->screen_y <= CENTER_Y))) {
            player->screen_y++;
            player->y++;
        }
        else if (key_is_down(KEY_DOWN) && (player->bg_vert <= BG_SCROLL_X_UPPER)) {
            player->bg_vert++;
            player->y++;
        }

        // Moving left
        if (key_is_down(KEY_LEFT) && ((player->bg_horz <= BG_SCROLL_Y_LOWER) || (player->screen_x >= CENTER_X))) {
            player->screen_x--;
            player->x--;
        }
        else if (key_is_down(KEY_LEFT) && (player->bg_horz >= BG_SCROLL_Y_LOWER)) {
            player->bg_horz--;
            player->x--;
        }

        // Moving right
        if (key_is_down(KEY_RIGHT) && ((player->bg_horz >= BG_SCROLL_Y_UPPER) || (player->screen_x <= CENTER_X))) {
            player->screen_x++;
            player->x++;
        }
        else if (key_is_down(KEY_RIGHT) && (player->bg_horz <= BG_SCROLL_Y_UPPER)) {
            player->bg_horz++;
            player->x++;
        }

        REG_BG0HOFS = player->bg_horz;
        REG_BG0VOFS = player->bg_vert;

        obj_set_pos(&obj_buffer[0], player->screen_x, player->screen_y);
        oam_copy(oam_mem, obj_buffer, 1);
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
            .tid = 0
        };

        //opening_sequence();
        init_bg();

        // Initialize the person
        // TODO move into separate init function
        //oam_init(obj_buffer, 128);
        oam_init(obj_buffer, 128);

        memcpy32(&tile_mem[4][0], man_tiles, man_tiles_len / sizeof(u32));
        memcpy16(pal_obj_mem, man_pal, man_pal_len / sizeof(u16));

        obj_set_attr(&obj_buffer[0],
            ATTR0_TALL | ATTR0_8BPP,
            ATTR1_SIZE_8x16,
            ATTR2_PALBANK(0) | 0);
        obj_set_pos(&obj_buffer[0], player.screen_x, player.screen_y);
        //obj_set_pos(&obj_buffer[0], 100, 100);

        REG_DISPCNT = DCNT_MODE0 | DCNT_BG0 | DCNT_OBJ | DCNT_OBJ_1D;

        sprite_loop(&player);

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
