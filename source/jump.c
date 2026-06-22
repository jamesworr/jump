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
    // need to pass current map somehow
    // need to account for the weird 64x64 tile vram layout

    // quantize player location into 8x8 BG tiles
    u8 upper_tile_x = (player->x) >> 3;
    u8 upper_tile_y = (player->y) >> 3;
    u8 lower_tile_x = (player->x + PLAYER_WIDTH ) >> 3;
    u8 lower_tile_y = (player->y + PLAYER_HEIGHT) >> 3;
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
	// Load map into SBB 30
	memcpy32(&se_mem[8][0], left_house_map, left_house_map_len / sizeof(u32));

    // TODO copy center and right
    // TODO see if we can fit all 3 BG maps into separate SBB
    // but keep common tileset

    // Setup background and display registers
    REG_BG0CNT  = BG_CBB(0) | BG_SBB(8) | BG_8BPP | BG_REG_64x64;
}

void sprite_loop() {
    OBJ_ATTR *block_obj;

    volatile int frame_counter = 0;
    volatile u8 collision = 0;
    volatile u16 bg_horz = 100;
    volatile u16 bg_vert = 100;

    while(1) {
        vid_vsync();
        key_poll();
        if (key_hit(KEY_SELECT)) {
            break; // Break out of waiting loop and restart
        }
        // Game Loop
        // Check collision
        // TODO

        // FIXME test control for map scrolling
        if (key_is_down(KEY_UP)    && (bg_vert > BG_SCROLL_X_LOWER)) {
            bg_vert--;
        }
        if (key_is_down(KEY_DOWN)  && (bg_vert < BG_SCROLL_X_UPPER)) {
            bg_vert++;
        }
        if (key_is_down(KEY_LEFT)  && (bg_horz > BG_SCROLL_Y_LOWER)) {
            bg_horz--;
        }
        if (key_is_down(KEY_RIGHT) && (bg_horz < BG_SCROLL_Y_UPPER)) {
            bg_horz++;
        }

        REG_BG0HOFS = bg_horz;
        REG_BG0VOFS = bg_vert;

        oam_copy(oam_mem, obj_buffer, 1);
        frame_counter++;
    }
}

int main() {
    while(1) {
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
        obj_set_pos(&obj_buffer[0], 100, 120);

        REG_DISPCNT = DCNT_MODE0 | DCNT_BG0 | DCNT_OBJ | DCNT_OBJ_1D;

        sprite_loop();

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
