//
// Will 4/21/26
//

#include <string.h>
#include <tonc.h>
#include "block.h"
#include "man.h"
#include "left_house.h"

OBJ_ATTR obj_buffer[128];
OBJ_AFFINE *obj_aff_buffer= (OBJ_AFFINE*)obj_buffer;

#define CBB_0  0
#define SBB_0 28

#define FPID 0
#define WPID 1

// [7:4] is Palette ID
// [3:0] is Tile ID
#define GRID_TL_W  0x30
#define GRID_TR_W  0x31
#define GRID_BL_W  0x33
#define GRID_BR_W  0x32
#define GRID_TL_G  0x50
#define GRID_TR_G  0x51
#define GRID_BL_G  0x53
#define GRID_BR_G  0x52
#define BLANK_BG   0x04

#define BLOCK_WIDTH  8
#define BLOCK_HEIGHT 8

#define NUM_COLS 30
#define NUM_ROWS 30
#define NUM_PIECES 1
#define MAX_BLOCKS 8 // FIXME

#define BLOCK_TILE_OFFSET 6

BG_POINT bg0_pt= { 0, 0 };
SCR_ENTRY *bg0_map= se_mem[SBB_0];


// World block
typedef struct {
    // block position
    u8 x; // init to 120
    u8 y;

    u8 tid;

} block_t;

// TODO get rid of me
volatile u8 board_state[NUM_COLS][NUM_ROWS] = {
    {0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0},
};

// FIXME dummy for now
// TODO make it a struct to handle multiple pieces
//const piece_map_t* live_piece = &(piece_library[0]);
volatile u8 live_piece_idx = 0;
volatile u8 live_piece_x   = 0;
volatile u8 live_piece_y   = 0;

void wait_any_key(void) {
    while(1) {
        vid_vsync();
        key_poll();
        if(key_hit(KEY_ANY)) {
            break;
        }
    }
}



//void render_blocks(void) {
//    OBJ_ATTR *block_obj;
//    for (int col = 0; col < NUM_COLS; col++) {
//        for (int row = 0; row < NUM_ROWS; row++) {
//            block_obj = &obj_buffer[(col*NUM_COLS)+row];
//            if (board_state[row][col]) {
//                // Set sprite visibility
//                block_obj->attr0 = block_obj->attr0 & 0xFCFF;
//
//                // Set sprite palette
//                u8 pal_bank;
//                switch (board_state[row][col]) {
//                    case 4:
//                        pal_bank = 2;
//                        break;
//                    case 6:
//                        pal_bank = 3;
//                        break;
//                    default:
//                        pal_bank = 1;
//                        break;
//                }
//                //if (board_state[row][col] < 4) {
//                //    pal_bank = 1;
//                //}
//                //else if (board_state[row][col] == 4) {
//                //    pal_bank = 2;
//                //}
//                //else if (board_state[row][col] == 6) {
//                //    pal_bank = 3;
//                //}
//                //else {
//                //    pal_bank = 1;
//                //}
//
//
//                block_obj->attr2 = ATTR2_BUILD(BLOCK_TILE_OFFSET, pal_bank, 0);
//            }
//            else {
//                // Set sprite visibility
//                block_obj->attr0 = block_obj->attr0 | ATTR0_HIDE;
//            }
//        }
//    }
//}


void init_bg() {
    // Initialize background 0
    //REG_BG0CNT = BG_CBB(CBB_0) | BG_SBB(SBB_0) | BG_REG_32x32;
    //REG_BG0HOFS = 0;
    //REG_BG0VOFS = 0;
    // TODO make background with PIG

    // Copy background tiles
	// Load palette
	memcpy16(pal_bg_mem, left_house_pal, left_house_pal_len / sizeof(u16));
	// Load tiles into CBB 0
	memcpy32(&tile_mem[0][0], left_house_tiles, left_house_tiles_len / sizeof(u32));
	// Load map into SBB 30
	memcpy32(&se_mem[30][0], left_house_map, left_house_map_len / sizeof(u32));

    // Setup background and display registers
    REG_BG0CNT  = BG_CBB(0) | BG_SBB(30) | BG_8BPP | BG_REG_64x64;
    REG_DISPCNT = DCNT_MODE0 | DCNT_BG0 | DCNT_OBJ | DCNT_OBJ_1D;
}

void sprite_loop() {
    OBJ_ATTR *block_obj;
    // Initialize the person
    // TODO move into separate init function
    block_obj = &obj_buffer[0];
    obj_set_attr(block_obj,
        ATTR0_TALL | ATTR0_8BPP,
        ATTR1_SIZE_8x16,
        ATTR2_PALBANK(0) | 0);
    obj_set_pos(block_obj, 100, 120);

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
        if (key_is_down(KEY_UP) && (bg_vert > 0)) {
            bg_vert--;
        }
        if (key_is_down(KEY_DOWN) && (bg_vert < 352)) {
            bg_vert++;
        }
        if (key_is_down(KEY_LEFT) && (bg_horz > 0)) {
            bg_horz--;
        }
        if (key_is_down(KEY_RIGHT) && (bg_horz < 272)) {
            bg_horz++;
        }

        REG_BG0HOFS = bg_horz;
        REG_BG0VOFS = bg_vert;

        //collision = copy_piece_to_board_state(live_piece_idx, live_piece_x, live_piece_y, 1);
        // TODO update colors of live piece placement
        // make collision 1 color and valid placement another
        // FIXME prob need to handle color indpenedant of directly from the block state
        // having trouble coloring all live blocks this way
        // problem is we can't just subtract after coloring all live pieces together


        // FIXME update all touched blocks
        //mr_env->attr2= ATTR2_BUILD(tid, pb, 0);
        //obj_set_pos(mr_env, x, y);


        //render_blocks();
        oam_copy(oam_mem, obj_buffer,  MAX_BLOCKS /* + other things??? TODO */);
        frame_counter++;
    }
}

int main() {
    while(1) {
        //opening_sequence();

        // Setup for tiled mode
        // Places the glyphs of a 4bpp Mr. Envelope
        //   into LOW obj memory (cbb == 4)
        // TODO need to either merge palettes
        // or have PIG generate separate palettes
        memcpy32(&tile_mem[4][0], man_tiles, man_tiles_len / sizeof(u32));
        memcpy16(pal_obj_mem, man_pal, man_pal_len / sizeof(u16));

        oam_init(obj_buffer, 128);
        REG_DISPCNT= DCNT_MODE0 | DCNT_BG0 | DCNT_OBJ | DCNT_OBJ_1D;

        init_bg(); // FIXME get rid of this BG
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
