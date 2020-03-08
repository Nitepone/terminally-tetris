/*
 * tetris-game.h
 * Copyright (C) 2019 nitepone <admin@night.horse>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef TETRIS_GAME_H
#define TETRIS_GAME_H

#define MAX_BLOCK_UNITS 4
#define BLOCK_START_POSITION   \
	{ 4, 21 }

#define MAX_AUTO_LOWER 3
#define MAX_SWAP_H 1

#define BOARD_HEIGHT 24
#define BOARD_PLAY_HEIGHT 20
#define BOARD_WIDTH 10

#define NO_BLOCK_VAL -1

enum rotation {
	none,
	right,
	invert,
	left,
};
#define ROT_COUNT 4

enum block_type {
	orange,
	blue,
	cleve,
	rhode,
	teewee,
	smashboy,
	hero,
};
#define BLOCK_TYPE_COUNT 7

struct position {
	int x;
	int y;
};

struct active_block {
	enum block_type block_type;
	enum rotation rotation;
	/* position of block center */
	struct position position;
	struct position board_units[MAX_BLOCK_UNITS];
};

static const struct position block_offsets[BLOCK_TYPE_COUNT][MAX_BLOCK_UNITS] =
    {
        {{0, 0}, {0, -1}, {1, -1}, {0, 1}},  // orange
        {{0, 0}, {0, -1}, {-1, -1}, {0, 1}}, // blue
        {{0, 0}, {-1, 0}, {-1, 1}, {0, -1}}, // cleve
        {{0, 0}, {-1, 0}, {-1, -1}, {0, 1}}, // rhode
        {{0, 0}, {1, 0}, {0, 1}, {-1, 0}},   // teewee
        // the following 2 pieces should not rely on this for rotation
        {{0, -1}, {0, 0}, {0, 1}, {0, 2}}, // hero
        {{0, 0}, {0, 1}, {1, 0}, {1, 1}},  // square
};

struct game_contents {
	int points;
	int lines_cleared;
	int auto_lower_count;
	int swap_h_block_count;
	int board[BOARD_HEIGHT][BOARD_WIDTH];
	unsigned int seed;
	enum block_type next_block;
	enum block_type hold_block;
	struct active_block *active_block;
};

struct game_view_data {
	int points;
	int lines_cleared;
	enum block_type next_block;
	enum block_type hold_block;
	int board[BOARD_HEIGHT][BOARD_WIDTH];
};

/*
 * Lowers the block down the board by 1
 * @param forced - 0 if move is done by client, non-zero if by game
 * @return - 0 if game is still good, -1 on game over
 */
int lower_block(int forced, struct game_contents *game_contents);

/*
 * Translates a block left or right a unit
 * @param rightward - zero if block movement is leftward, else rightward
 * @return - 0 if piece moved, else non-zero
 */
int translate_block(int rightward, struct game_contents *game_contents);

/*
 * Rotates a block left or right
 * @param clockwise - zero rotates the block counter-clockwise, else clockwise
 * @return - 0 if piece rotated, else non-zero
 */
int rotate_block(int clockwise, struct game_contents *game_contents);

/*
 * Starts a game and takes a double pointer for game contents data
 * @return 0
 */
int new_game(struct game_contents **game_contents);

/*
 * Starts a game and takes a double pointer for game contents data.
 * Also takes a seed to base the number generation from.
 * @return 0
 */
int new_seeded_game(struct game_contents **game_contents);

/*
 * Destroy old game data
 * Frees the memory and sets the second pointer to NULL
 * @return 0
 */
int destroy_game(struct game_contents **game_contents);

/*
 * Makes a game_view_data with a representation of all active board pieces
 */
int generate_game_view_data(struct game_view_data **gvd,
                            struct game_contents *gc);

/*
 * Returns 0 if game is still ongoing
 */
int game_over(struct game_contents *game_contents);

/*
 * Forces the current piece to drop until placed immediately
 */
int hard_drop(struct game_contents *game_contents);

/*
 * Swaps the currently used block with the one in holding
 */
int swap_hold_block(struct game_contents *game_contents);

#endif /* !TETRIS_GAME_H */
