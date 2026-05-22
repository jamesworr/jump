//
// Will 4/21/26
//

#include <string.h>
#include <tonc.h>
//#include "block_sprite.h"

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

#define BLOCK_WIDTH  16
#define BLOCK_HEIGHT 16

#define NUM_COLS 30
#define NUM_ROWS 30
#define NUM_PIECES 1
#define MAX_BLOCKS 32 // FIXME
#define BLOCK_GAP_WddIDTH 0

#define BLOCK_TILE_OFFSET 0

BG_POINT bg0_pt= { 0, 0 };
SCR_ENTRY *bg0_map= se_mem[SBB_0];

const TILE tiles[5] =
{
    {{0x00000000,
      0x11111110,
      0x11111110,
      0x11111110,
      0x11111110,
      0x11111110,
      0x11111110,
      0x11111110}},

    {{0x00000000,
      0x01111111,
      0x01111111,
      0x01111111,
      0x01111111,
      0x01111111,
      0x01111111,
      0x01111111}},

    {{0x01111111,
      0x01111111,
      0x01111111,
      0x01111111,
      0x01111111,
      0x01111111,
      0x01111111,
      0x00000000}},

    {{0x11111110,
      0x11111110,
      0x11111110,
      0x11111110,
      0x11111110,
      0x11111110,
      0x11111110,
      0x00000000}},

    {{0x11111111,
      0x11111111,
      0x11111111,
      0x11111111,
      0x11111111,
      0x11111111,
      0x11111111,
      0x11111111}},

};

const SCR_ENTRY my_map[20][32] = {
    {GRID_TL_W,GRID_TR_W,GRID_TL_W,GRID_TR_W,GRID_TL_W,GRID_TR_W,GRID_TL_G,GRID_TR_G,GRID_TL_G,GRID_TR_G,GRID_TL_G,GRID_TR_G,GRID_TL_W,GRID_TR_W,GRID_TL_W,GRID_TR_W,GRID_TL_W,GRID_TR_W,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG},
    {GRID_BL_W,GRID_BR_W,GRID_BL_W,GRID_BR_W,GRID_BL_W,GRID_BR_W,GRID_BL_G,GRID_BR_G,GRID_BL_G,GRID_BR_G,GRID_BL_G,GRID_BR_G,GRID_BL_W,GRID_BR_W,GRID_BL_W,GRID_BR_W,GRID_BL_W,GRID_BR_W,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG},
    {GRID_TL_W,GRID_TR_W,GRID_TL_W,GRID_TR_W,GRID_TL_W,GRID_TR_W,GRID_TL_G,GRID_TR_G,GRID_TL_G,GRID_TR_G,GRID_TL_G,GRID_TR_G,GRID_TL_W,GRID_TR_W,GRID_TL_W,GRID_TR_W,GRID_TL_W,GRID_TR_W,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG},
    {GRID_BL_W,GRID_BR_W,GRID_BL_W,GRID_BR_W,GRID_BL_W,GRID_BR_W,GRID_BL_G,GRID_BR_G,GRID_BL_G,GRID_BR_G,GRID_BL_G,GRID_BR_G,GRID_BL_W,GRID_BR_W,GRID_BL_W,GRID_BR_W,GRID_BL_W,GRID_BR_W,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG},
    {GRID_TL_W,GRID_TR_W,GRID_TL_W,GRID_TR_W,GRID_TL_W,GRID_TR_W,GRID_TL_G,GRID_TR_G,GRID_TL_G,GRID_TR_G,GRID_TL_G,GRID_TR_G,GRID_TL_W,GRID_TR_W,GRID_TL_W,GRID_TR_W,GRID_TL_W,GRID_TR_W,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG},
    {GRID_BL_W,GRID_BR_W,GRID_BL_W,GRID_BR_W,GRID_BL_W,GRID_BR_W,GRID_BL_G,GRID_BR_G,GRID_BL_G,GRID_BR_G,GRID_BL_G,GRID_BR_G,GRID_BL_W,GRID_BR_W,GRID_BL_W,GRID_BR_W,GRID_BL_W,GRID_BR_W,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG},
    {GRID_TL_G,GRID_TR_G,GRID_TL_G,GRID_TR_G,GRID_TL_G,GRID_TR_G,GRID_TL_W,GRID_TR_W,GRID_TL_W,GRID_TR_W,GRID_TL_W,GRID_TR_W,GRID_TL_G,GRID_TR_G,GRID_TL_G,GRID_TR_G,GRID_TL_G,GRID_TR_G,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG},
    {GRID_BL_G,GRID_BR_G,GRID_BL_G,GRID_BR_G,GRID_BL_G,GRID_BR_G,GRID_BL_W,GRID_BR_W,GRID_BL_W,GRID_BR_W,GRID_BL_W,GRID_BR_W,GRID_BL_G,GRID_BR_G,GRID_BL_G,GRID_BR_G,GRID_BL_G,GRID_BR_G,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG},
    {GRID_TL_G,GRID_TR_G,GRID_TL_G,GRID_TR_G,GRID_TL_G,GRID_TR_G,GRID_TL_W,GRID_TR_W,GRID_TL_W,GRID_TR_W,GRID_TL_W,GRID_TR_W,GRID_TL_G,GRID_TR_G,GRID_TL_G,GRID_TR_G,GRID_TL_G,GRID_TR_G,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG},
    {GRID_BL_G,GRID_BR_G,GRID_BL_G,GRID_BR_G,GRID_BL_G,GRID_BR_G,GRID_BL_W,GRID_BR_W,GRID_BL_W,GRID_BR_W,GRID_BL_W,GRID_BR_W,GRID_BL_G,GRID_BR_G,GRID_BL_G,GRID_BR_G,GRID_BL_G,GRID_BR_G,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG},
    {GRID_TL_G,GRID_TR_G,GRID_TL_G,GRID_TR_G,GRID_TL_G,GRID_TR_G,GRID_TL_W,GRID_TR_W,GRID_TL_W,GRID_TR_W,GRID_TL_W,GRID_TR_W,GRID_TL_G,GRID_TR_G,GRID_TL_G,GRID_TR_G,GRID_TL_G,GRID_TR_G,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG},
    {GRID_BL_G,GRID_BR_G,GRID_BL_G,GRID_BR_G,GRID_BL_G,GRID_BR_G,GRID_BL_W,GRID_BR_W,GRID_BL_W,GRID_BR_W,GRID_BL_W,GRID_BR_W,GRID_BL_G,GRID_BR_G,GRID_BL_G,GRID_BR_G,GRID_BL_G,GRID_BR_G,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG},
    {GRID_TL_W,GRID_TR_W,GRID_TL_W,GRID_TR_W,GRID_TL_W,GRID_TR_W,GRID_TL_G,GRID_TR_G,GRID_TL_G,GRID_TR_G,GRID_TL_G,GRID_TR_G,GRID_TL_W,GRID_TR_W,GRID_TL_W,GRID_TR_W,GRID_TL_W,GRID_TR_W,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG},
    {GRID_BL_W,GRID_BR_W,GRID_BL_W,GRID_BR_W,GRID_BL_W,GRID_BR_W,GRID_BL_G,GRID_BR_G,GRID_BL_G,GRID_BR_G,GRID_BL_G,GRID_BR_G,GRID_BL_W,GRID_BR_W,GRID_BL_W,GRID_BR_W,GRID_BL_W,GRID_BR_W,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG},
    {GRID_TL_W,GRID_TR_W,GRID_TL_W,GRID_TR_W,GRID_TL_W,GRID_TR_W,GRID_TL_G,GRID_TR_G,GRID_TL_G,GRID_TR_G,GRID_TL_G,GRID_TR_G,GRID_TL_W,GRID_TR_W,GRID_TL_W,GRID_TR_W,GRID_TL_W,GRID_TR_W,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG},
    {GRID_BL_W,GRID_BR_W,GRID_BL_W,GRID_BR_W,GRID_BL_W,GRID_BR_W,GRID_BL_G,GRID_BR_G,GRID_BL_G,GRID_BR_G,GRID_BL_G,GRID_BR_G,GRID_BL_W,GRID_BR_W,GRID_BL_W,GRID_BR_W,GRID_BL_W,GRID_BR_W,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG},
    {GRID_TL_W,GRID_TR_W,GRID_TL_W,GRID_TR_W,GRID_TL_W,GRID_TR_W,GRID_TL_G,GRID_TR_G,GRID_TL_G,GRID_TR_G,GRID_TL_G,GRID_TR_G,GRID_TL_W,GRID_TR_W,GRID_TL_W,GRID_TR_W,GRID_TL_W,GRID_TR_W,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG},
    {GRID_BL_W,GRID_BR_W,GRID_BL_W,GRID_BR_W,GRID_BL_W,GRID_BR_W,GRID_BL_G,GRID_BR_G,GRID_BL_G,GRID_BR_G,GRID_BL_G,GRID_BR_G,GRID_BL_W,GRID_BR_W,GRID_BL_W,GRID_BR_W,GRID_BL_W,GRID_BR_W,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG},
    {BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG},
    {BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG,BLANK_BG},
};

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

// TODO DFS on piece map to only touch live piece
// returns collision state
//u8 copy_piece_to_board_state(u8 piece_idx, u8 block_idx_x, u8 block_idx_y, u8 pal_offset) {
//    volatile u8 collision_found = 0;
//    for (int i = 0; i < piece_library[piece_idx].y_len; i++) {
//        for (int j = 0; j < piece_library[piece_idx].x_len; j++) {
//            board_state[block_idx_y+i][block_idx_x+j] = board_state[block_idx_y+i][block_idx_x+j] + (piece_library[piece_idx].map[i][j] << pal_offset);
//            if (board_state[block_idx_y+i][block_idx_x+j] > 4) {
//                collision_found = 1;
//            }
//        }
//    }
//    return collision_found;
//}

// TODO DFS on piece map to only touch live piece
//void remove_piece_from_board_state(u8 piece_idx, u8 block_idx_x, u8 block_idx_y, u8 pal_offset) {
//    for (int i = 0; i < piece_library[piece_idx].y_len; i++) {
//        for (int j = 0; j < piece_library[piece_idx].x_len; j++) {
//            if (board_state[block_idx_y+i][block_idx_x+j] > 0) {
//                board_state[block_idx_y+i][block_idx_x+j] = board_state[block_idx_y+i][block_idx_x+j] - (piece_library[piece_idx].map[i][j] << pal_offset);
//            }
//        }
//    }
//}

//u8 square_check(u8 x, u8 y) {
//    // Find which squares the live piece touches
//    // could theoretically be 4 ):
//    // maybe check the topl and if that clears then check the other 3?
//    // TODO handle botr
//
//    volatile u8 valid_structure = 1;
//
//    // Find the corner of the square to check
//    volatile u8 x_square = (x > 8) ? 6 : (3 * (x / 3));
//    volatile u8 y_square = (y > 8) ? 6 : (3 * (y / 3));
//
//    for (int i = 0; i < 3; i++) {
//        for (int j = 0; j < 3; j++) {
//            if (board_state[y_square+j][x_square+i] == 0) {
//                valid_structure = 0;
//                break;
//            }
//        }
//    }
//    return valid_structure;
//}

//void square_clear(u8 x, u8 y) {
//    // Find the corner of the square to check
//    volatile u8 x_square = (x > 8) ? 6 : (3 * (x / 3));
//    volatile u8 y_square = (y > 8) ? 6 : (3 * (y / 3));
//
//    for (int i = 0; i < 3; i++) {
//        for (int j = 0; j < 3; j++) {
//            board_state[y_square+j][x_square+i] = 0;
//        }
//    }
//}

// TODO optimize to only run live piece shape, not n^2
// Checks for vertical 9, horizontal 9, or square 9
// Check east west for each Y in the shape
// Check north south for each X in the shape
// returns points scored
//u8 find_complete_block_structure(u8 piece_idx, u8 block_idx_x, u8 block_idx_y) {
//    u8 valid_structure = 1;
//    u8 total_clears = 0; // TODO use me
//
//    // East West for each Y in live piece
//    for (int i = 0; i < MAX_DIM; i++) {
//        for (int j = 0; j < NUM_ROWS; j++) {
//            if (board_state[block_idx_y+i][j] == 0) {
//                valid_structure = 0;
//                break;
//            }
//        }
//        if (valid_structure == 1) {
//            // Tag row for removal
//            board_state[block_idx_y+i][0] = 20;
//        }
//        valid_structure = 1;
//    }
//    valid_structure = 1;
//    // North South for each X in live piece
//    for (int i = 0; i < MAX_DIM; i++) {
//        for (int j = 0; j < NUM_COLS; j++) {
//            if (board_state[j][block_idx_x+i] == 0) {
//                valid_structure = 0;
//                break;
//            }
//        }
//        if (valid_structure == 1) {
//            // Have to create different tag in top left corner where you can have both
//            // horizontal and vertical clears starting from the same block
//            board_state[0][block_idx_x+i] = (board_state[0][block_idx_x+i] == 20)? 22:21;
//        }
//        valid_structure = 1;
//    }
//
//    // Square detect
//    volatile u8 squares_state = 0;
//    squares_state |= square_check(block_idx_x + piece_library[piece_idx].x_len, block_idx_y + piece_library[piece_idx].y_len);
//    squares_state <<= 1;
//    squares_state |= square_check(block_idx_x, block_idx_y + piece_library[piece_idx].y_len);
//    squares_state <<= 1;
//    squares_state |= square_check(block_idx_x + piece_library[piece_idx].x_len, block_idx_y);
//    squares_state <<= 1;
//    squares_state |=  square_check(block_idx_x, block_idx_y);
//
//    // Vertical removal
//    for (int i = 0; i < NUM_COLS; i++) {
//        if (board_state[0][i] == 21) { // FIXME handle collision
//            for (int j = 0; j < NUM_ROWS; j++) {
//                board_state[j][i] = 0;
//            }
//        }
//        // Both horizontal and vertical
//        else if (board_state[0][i] == 22) {
//            for (int j = 0; j < NUM_ROWS; j++) {
//                board_state[j][i] = 0;
//            }
//            board_state[0][i] = 20; // put tag back for horizontal
//        }
//    }
//    
//    // Horizontal removal
//    for (int i = 0; i < NUM_ROWS; i++) {
//        if (board_state[i][0] == 20) {
//            for (int j = 0; j < NUM_COLS; j++) {
//                board_state[i][j] = 0;
//            }
//        }
//    }
//
//
//    // Square removal
//    if (squares_state & 0x01) {
//        square_clear(block_idx_x, block_idx_y);
//    }
//    if (squares_state & 0x02) {
//        square_clear(block_idx_x + piece_library[piece_idx].x_len, block_idx_y);
//    }
//    if (squares_state & 0x04) {
//        square_clear(block_idx_x, block_idx_y + piece_library[piece_idx].y_len);
//    }
//    if (squares_state & 0x04) {
//        square_clear(block_idx_x + piece_library[piece_idx].x_len, block_idx_y + piece_library[piece_idx].y_len);
//    }
//
//
//
//    // TODO save all clears and remove block at the end
//
//    return 0;
//}

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

void demo_animation(void) {
    u8 piece_idx = 0;
    u8 pal_bank  = 0;
    while(1) {
        vid_vsync();
        key_poll();
        if (key_hit(KEY_SELECT)) {
            break; // Break out of waiting loop and restart
        }

        //remove_piece_from_board_state(piece_idx, 4, 4, 0);

        if (key_hit(KEY_RIGHT) && (piece_idx < NUM_PIECES-1)) {
            piece_idx++;
        }
        if (key_hit(KEY_LEFT) && (piece_idx > 0)) {
            piece_idx--;
        }
        if (key_hit(KEY_UP)) {
            pal_bank++;
            for (int i = 0; i < MAX_BLOCKS; i++) {
                obj_buffer[i].attr2 = ATTR2_BUILD(BLOCK_TILE_OFFSET, pal_bank, 0);
            }
        }
        if (key_hit(KEY_DOWN)) {
            pal_bank--;
            for (int i = 0; i < MAX_BLOCKS; i++) {
                obj_buffer[i].attr2 = ATTR2_BUILD(BLOCK_TILE_OFFSET, pal_bank, 0);
            }
        }
        
        //copy_piece_to_board_state(piece_idx, 4, 4, 0);

        //render_blocks();
        oam_copy(oam_mem, obj_buffer,  MAX_BLOCKS /* + other things??? TODO */);
    }
}

void init_bg() {
    // Initialize background 0
    REG_BG0CNT = BG_CBB(CBB_0) | BG_SBB(SBB_0) | BG_REG_32x32;
    REG_BG0HOFS = 0;
    REG_BG0VOFS = 0;

    tile_mem[CBB_0][0] = tiles[0];
    tile_mem[CBB_0][1] = tiles[1];
    tile_mem[CBB_0][2] = tiles[2];
    tile_mem[CBB_0][3] = tiles[3];

    // Create the palette
    pal_bg_bank[0][1] = RGB15(31,  0,  0);
    pal_bg_bank[1][1] = RGB15( 0, 31,  0);
    pal_bg_bank[2][1] = RGB15( 0,  0, 31);
    pal_bg_bank[3][1] = RGB15(16, 16, 16);
    pal_bg_bank[4][1] = RGB15( 0,  0,  0);
    pal_bg_bank[5][1] = RGB15(31, 31, 31);

    SCR_ENTRY *pse = bg0_map;
    for(int jj=0; jj<32*20; jj++) {
        *pse++ = SE_PALBANK((my_map[jj/32][jj%32] & 0xF0)>>4) | SE_ID(my_map[jj/32][jj%32] & 0x0F);
    }
}

void sprite_loop() {
    OBJ_ATTR *block_obj;
    for (int i = 0; i < MAX_BLOCKS; i++) {
        block_obj = &obj_buffer[i];
        obj_set_attr(block_obj,
            ATTR0_SQUARE | ATTR0_HIDE,
            ATTR1_SIZE_16x16,
            ATTR2_PALBANK(0) | BLOCK_TILE_OFFSET);
        obj_set_pos(block_obj, (i/NUM_COLS)*BLOCK_HEIGHT, (i%NUM_ROWS)*BLOCK_WIDTH);
    }

    // TODO remove demo
    //demo_animation();
    
    // Copy initial piece to board
    //copy_piece_to_board_state(live_piece_idx, live_piece_x, live_piece_y, 1);

    volatile int frame_counter = 0;
    volatile u8 collision = 0;
    while(1) {
        vid_vsync();
        key_poll();
        if (key_hit(KEY_SELECT)) {
            break; // Break out of waiting loop and restart
        }
        // Game Loop
        // move live piece or swap live piece
        // check collisons
        // Can place?
        // Does legal placement clear any blocks? (vertical, horizontal, square)


        // Check collision

        // Check for "place" command
        // Double copy on placement to leave blocks in the game state
        // Check for no collision
        // Generate new live piece
        if ((key_hit(KEY_A)) && (collision == 0)) {
            // reduce live state from 4 to "placed" 2
            //remove_piece_from_board_state(live_piece_idx, live_piece_x, live_piece_y, 0);

            // TODO check for cleared blocks
            //find_complete_block_structure(live_piece_idx, live_piece_x, live_piece_y);
            live_piece_idx = qran_range(0, NUM_PIECES-1);

            // FIXME just temporary until I implement botr
            //while (piece_library[live_piece_idx].topl_botr) {
            //    live_piece_idx = qran_range(0, NUM_PIECES-1);
            //}

            live_piece_x   = 0;
            live_piece_y   = 0;
        }
        else {
            //remove_piece_from_board_state(live_piece_idx, live_piece_x, live_piece_y, 1);
        }
        
        // FIXME redo the bounds checking to account for different piece shapes
        //if (key_hit(KEY_UP) && (live_piece_y > 0)) {
        //    live_piece_y--;
        //}
        //if (key_hit(KEY_DOWN) && (live_piece_y < (NUM_ROWS - piece_library[live_piece_idx].y_len))) {
        //    live_piece_y++;
        //}
        //if (key_hit(KEY_LEFT) && (live_piece_x > 0)) {
        //    live_piece_x--;
        //}
        //if (key_hit(KEY_RIGHT) && (live_piece_x < (NUM_ROWS - piece_library[live_piece_idx].x_len))) {
        //    live_piece_x++;
        //}

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
        // TODO copy block
        // FIXME
        //memcpy32(&tile_mem[4][0], block_spriteTiles, block_spriteTilesLen / sizeof(u32));
        //memcpy16(pal_obj_mem, block_spritePal, block_spritePalLen / sizeof(u16));
        oam_init(obj_buffer, 128);
        REG_DISPCNT= DCNT_MODE0 | DCNT_BG0 | DCNT_OBJ | DCNT_OBJ_1D;

        init_bg(); // FIXME create basic BG
        //sprite_loop();

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
