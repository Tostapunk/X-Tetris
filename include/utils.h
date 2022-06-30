#ifndef X_TETRIS_UTILS_H
#define X_TETRIS_UTILS_H

#include <termios.h>
#include "main.h"

#define ANSI_CURSOR_POS(x, y) printf("\033[%d;%dH", y, x)
#define ANSI_CURSOR_POS_RESET printf("\033[0;0H")
#define ANSI_CURSOR_SHOW "\033[?25h"
#define ANSI_CURSOR_HIDE "\033[?25l"
#define ANSI_CLEAR "\033[2J\033[1;1H"

/* ANSI colors */
#define ANSI_BGH_WHITE "\033[0;107m"
#define ANSI_COLOR_GRAY "\x1b[90m"
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GRN "\x1b[32m"
#define ANSI_COLOR_YEL "\x1b[33m"
#define ANSI_COLOR_BLU "\x1b[34m"
#define ANSI_COLOR_MAG "\x1b[35m"
#define ANSI_COLOR_CYN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define MINO "█"
extern const char GHOST_MINO;

extern const char EMPTY_CHAR;
extern const char NULL_CHAR;

/* Terminal modes. */
typedef enum { T_GAME, T_RESTORE } term_t;
/* Cursor modes. */
typedef enum { SHOW, HIDE } cursor_t;
/* Arrow keys. */
typedef enum { DOWN_ARROW = 66, RIGHT_ARROW = 67, LEFT_ARROW = 68} arrow_key_t;

static struct termios old_settings, new_settings;

/*
 * Custom error display function (use the GNU identifier __FUNCTION__).
 */ 
void error(const char* func, const char* text);

/* 
 * Uses ANSI_CLEAR to clear the screen content.
 */
void clear_screen();

/*
 * Sets the cursor mode.
 */
void cursor_mode(cursor_t action);

/*
 * Sets the terminal mode:
 * - T_GAME:
 *   - disable ICANON
 *   - disable ECHO
 *   - hide cursor
 *
 * - T_RESTORE
 */
void term_setup(term_t mode);

struct winsize term_size();

/*
 * POSIX implementation of _kbhit().
 */ 
bool_t kbhit();

/*
 * Prints colored text.
 */
void printc(const char *color, const char *text);

/*
 * Prints the game graphics (loaded from graphics/).
 */ 
void printg(const char *file_name, const char *color);

/*
 * Creates an array of array (not flattened due to my laziness).
 */ 
char **create_array_of_array(int x, int y, bool_t empty_init);

/*
 * Array deep copy.
 */ 
void copy_array_of_array(char **src, char **dest, int x, int y);

/*
 * Frees an array correctly.
 */ 
void free_array_of_array(char **array, int size);

/*
 * CPU time (milliseconds).
 */
void time_float(float *time);

/*
 * Checks if the window size has changed and then updates the value.
 */
void window_resize(int *screen_width);

/*
 * Player 2 keypress check.
 */
void move_sanitize(int *input, turn_t turn);

#endif /* X_TETRIS_UTILS_H */
