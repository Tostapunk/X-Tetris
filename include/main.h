#ifndef X_TETRIS_MAIN_H
#define X_TETRIS_MAIN_H

/* Matrix width. */
extern const int MATRIX_W;
/* Matrix height. */
extern const int MATRIX_H;
/* Min matrix width. */
extern const int MATRIX_X_MIN;
/* Min matrix height. */
extern const int MATRIX_Y_MIN;
/* Number of tetraminos for each type. */
extern const int MAX_TETRAMINOS;
/* Max tetramino width. */
extern const int MAX_TETRAMINOS_SIDE;
/* Number of tetraminos to preview. */
extern const int MAX_TETRAMINOS_PREVIEW;

typedef enum { FALSE, TRUE } bool_t;

/* Game moves. */
typedef enum {
    LEFT = 'a',
	RIGHT = 'd',
	DOWN = 's',
	ROTATE = 'r',
	HARD_DROP = ' ',
	CLEAR = 'c',
	QUIT = 'q'
} move_t;

/* Game modes. */
typedef enum {
	SINGLE,
	MULTI,
	CPU
} game_t;

/* Player/turn types. */
typedef enum {
	PLAYER_1,
	PLAYER_2,
	PLAYER_CPU
} turn_t;

/* Tetraminos types. */
typedef enum {
	I_TETRAMINO,
	J_TETRAMINO,
	L_TETRAMINO,
	O_TETRAMINO,
	S_TETRAMINO,
	Z_TETRAMINO,
	T_TETRAMINO
} tetramino_t;

/* Tetramino info. */
struct tetraminos {
	int type;
	char mino[5][5];
	int w;
	int h;
	char *color;
};

struct game_data {
	/* Matrix Player 1. */
    char **matrix1;
	/* Matrix Player 2/CPU. */
    char **matrix2;
	/* Gameover flag value. */
    int gameover;
	/* Score Player 1. */
    int score1;
	/* Score Player 2/CPU. */
    int score2;
	/* Level player 1. */
	int level1;
	/* Level player 2/CPU. */
	int level2;
	/* Current game mode. */
    game_t game_mode;
	/* Defines who's playing in the current turn. */
    turn_t turn;
	/* Starting number of tetraminos available. */
	int total_tetraminos;
	/* Remaining tetraminos. */
    int remaining_tetraminos;
	/* X position (current column). */
    int x;
	/* Y position (current row). */
    int y;
	/* Collision flag (usually updated after a movement). */
    int collision;
	/* Fullrows obtained with the latest move. */
	int fullrows;
	/* Current move. */
    move_t move;
	/* Current tetramino. */
	struct tetraminos current_tetramino;
	/* Next tetraminos of player 1. */
    struct tetraminos next_tetraminos1[4];
	/* Next tetraminos of player 2/CPU. */
    struct tetraminos next_tetraminos2[4];
	/* Terminal window width. */
	int window_w;
};

/* Tetraminos definition. */
extern struct tetraminos tetramino[7];

/*
 * Shows the winner and the players score.
 */
void game_over(struct game_data *g);

/*
 * Generates new tetraminos.
 */ 
void new_tetramino(struct game_data *g);

/*
 * Game data init.
 */ 
void init(struct game_data *g);

/*
 * If game_data.fullrows >= 3 and player mode is not SINGLE then it 
 * inverts game_data.fullrows rows on the opponent matrix.
 */ 
void rows_invert(struct game_data *g);

/*
 * Checks for fullrows.
 */ 
void fullrow_check(struct game_data *g, char **matrix);

/*
 * Checks if the latest move have generated a collision.
 */ 
int collision_check(struct game_data *g);

void help(void);

/*
 * Tries to fix a rotation that have generated a collision (wall kicks).
 */ 
bool_t rotate_fix(struct game_data *g, char **matrix);

/*
 * Rotates the current tetramino.
 */ 
void rotate(struct game_data *g, bool_t check_collision);

/*
 * Shows the upcoming tetraminos.
 */ 
void print_next_tetraminos(struct game_data *g, int screen_x, turn_t player);

/*
 * Shows the remaining number of tetraminos and the player turn.
 */ 
void print_stats(struct game_data *g);

/*
 * Shows the score and the level for each player.
 */ 
void print_score(int x, int y, int level, int score);

/*
 * Shows the player matrix
 */ 
void print_matrix(struct game_data *g, char **aux_matrix);

/*
 * Inserts a tetramino in the current player matrix.
 */ 
void tetramino_insert(struct game_data *g, char **aux_matrix, bool_t is_ghost);

/*
 * Ghost piece.
 */ 
void ghost(struct game_data *g, char **aux_matrix);

/*
 * Updates the matrix after a valid move.
 */ 
void matrix(struct game_data *g);

/*
 * Moves down the tetramino.
 */ 
void drop(struct game_data *g);

/*
 * Makes a tetramino fall down until a collision is detected.
 */ 
void hard_drop(struct game_data *g, bool_t check_collision);

/*
 * Fall speed (2009 design guideline).
 */ 
float fall_speed(struct game_data *g);

/*
 * CPU strategy.
 * Tries to get the max amount of fullrows or the min value on the y axis.
 */ 
void cpu_strategy(struct game_data *g);

/*
 * Movement control:
 * - keypress movement association
 * - automatic tetramino fall down
 */ 
void run(struct game_data *g);

/*
 * Shows the title screen.
 */ 
void print_title();

/*
 * Game mode selection.
 */ 
void title(struct game_data *g);

int main(void);

#endif /* X_TETRIS_MAIN_H */
