#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

#include "main.h"
#include "utils.h"

const char GHOST_MINO = '-';
const char EMPTY_CHAR = ' ';
const char NULL_CHAR = '\0';

/* 
 * Use __FUNCTION__ GNU identifier as func argument
 * because __func__ was added to the standard in C99 
 */  
void error(const char* func, const char* text){
    fprintf(stderr, "%s: %s\n",func, text);
    abort();
}

void clear_screen() { printf(ANSI_CLEAR); }

void cursor_mode(cursor_t action) {
    if (action == SHOW) {
        printf(ANSI_CURSOR_SHOW);
    } else {
        printf(ANSI_CURSOR_HIDE);
    }
}

/*
 * https://man7.org/linux/man-pages/man3/termios.3.html
 */
void term_setup(term_t mode) {
    if (mode == T_GAME) {
        tcgetattr(STDIN_FILENO, &old_settings);
        new_settings = old_settings;
        new_settings.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);
        cursor_mode(HIDE);
    } else if (mode == T_RESTORE) {
        tcsetattr(STDIN_FILENO, TCSANOW, &old_settings);
        cursor_mode(SHOW);
    }
}

struct winsize term_size() {
    struct winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    return ws;
}

bool_t kbhit() {
    int init = FALSE;
    int hit;

    if (!init) {
        setbuf(stdin, NULL);
        init = TRUE;
    }

    ioctl(STDIN_FILENO, FIONREAD, &hit);
    return hit;
}

void printc(const char *color, const char *text) {
    if (color == NULL) {
        switch (text[0]) {
        case '#':
            color = tetramino[I_TETRAMINO].color;
            break;
        case '@':
            color = tetramino[J_TETRAMINO].color;
            break;
        case 'B':
            color = tetramino[L_TETRAMINO].color;
            break;
        case 'X':
            color = tetramino[O_TETRAMINO].color;
            break;
        case '%':
            color = tetramino[S_TETRAMINO].color;
            break;
        case 'Z':
            color = tetramino[Z_TETRAMINO].color;
            break;
        case 'O':
            color = tetramino[T_TETRAMINO].color;
            break;
        case GHOST_MINO:
            color = ANSI_BGH_WHITE;
            break;
        }
        printf("%s%s%s", color, MINO, ANSI_COLOR_RESET);
    } else {
        printf("%s%s%s", color, text, ANSI_COLOR_RESET);
    }
}

void printg(const char *file_name, const char *color){
    char *dir = "graphics/";
    char *ext = ".txt";
    char *file_with_ext = malloc(sizeof(file_with_ext) * (strlen(dir)+strlen(file_name)+strlen(ext)));
    char *line = NULL;
    size_t len = 0;
    FILE *fp;

    strcpy(file_with_ext, dir);
    strcat(file_with_ext, file_name);
    strcat(file_with_ext, ext);

    fp = fopen(file_with_ext, "r");
    if (fp == NULL){
        error(__FUNCTION__, "Cannot open file");
    }

    while(getline(&line, &len, fp) != -1){
        if(color == NULL){
            printf("%s", line);
        } else {
            printc(color, line);
        }
    }
    fclose(fp);
    free(line);
    free(file_with_ext);
}

char **create_array_of_array(int x, int y, bool_t empty_init) {
    int i, j;
    char **array;
    array = malloc(sizeof(array) * y);
    for (i = 0; i < y; ++i) {
        array[i] = malloc(sizeof(array[i]) * x);
        if (empty_init == TRUE) {
            for (j = 0; j < x; ++j) {
                array[i][j] = EMPTY_CHAR;
            }
        }
    }
    if (array == NULL) {
        error(__FUNCTION__, "Memory allocation failed");
    }
    return array;
}

void copy_array_of_array(char **src, char **dest, int x, int y) {
    int i, j;
    for (i = 0; i < y; ++i) {
        for (j = 0; j < x; ++j) {
            dest[i][j] = src[i][j];
        }
    }
}

void free_array_of_array(char **array, int size) {
    int i;
    if (array != NULL) {
        for (i = 0; i < size; ++i) {
            free(array[i]);
        }
        free(array);
    }
}

void time_float(float *time) {
    *time = (clock() / (CLOCKS_PER_SEC / 1000.0)) * 0.001f;
}

void window_resize(int *saved_width) {
    int current_width;
    if ((current_width = term_size().ws_col) != *saved_width) {
        *saved_width = current_width;
        clear_screen();
    }
}

void move_sanitize(int *input, turn_t turn){
    int ch = *input;
    int special_key_init = 27;
    if(ch == special_key_init) {
		getchar();
		ch = getchar();
		if(turn == PLAYER_2){
			switch(ch) {
				case DOWN_ARROW:
					*input = DOWN;
					break;
				case RIGHT_ARROW:
					*input = RIGHT;
					break;
				case LEFT_ARROW:
					*input = LEFT;
					break;
			}
		}
		else {
			*input = NULL_CHAR;
		}
    } else if (turn == PLAYER_2) {
        if(ch == LEFT || ch == RIGHT || ch == DOWN || ch == ROTATE){
            *input = NULL_CHAR;
        } else if (ch == '-'){
            *input = ROTATE;
        }
    }
}
