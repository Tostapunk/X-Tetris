#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>

#include "main.h"
#include "utils.h"

const int MATRIX_W = 10;
const int MATRIX_H = 15;
const int MATRIX_X_MIN = 0;
const int MATRIX_Y_MIN = 0;
const int MAX_TETRAMINOS = 20;
const int MAX_TETRAMINO_SIDE = 4;
const int MAX_TETRAMINOS_PREVIEW = 4;


struct tetraminos tetramino[7] = {{I_TETRAMINO, {"####"}, 4, 1, ANSI_COLOR_CYN},
                              {J_TETRAMINO, {"@  ",
                                			 "@@@"}, 3, 2, ANSI_COLOR_BLU},
                              {L_TETRAMINO, {"  B",
                                			 "BBB"}, 3, 2, ANSI_COLOR_GRAY},
                              {O_TETRAMINO, {"XX",
                                			 "XX"}, 2, 2, ANSI_COLOR_YEL},
                              {S_TETRAMINO, {" %%",
                                			 "%% "}, 3, 2, ANSI_COLOR_GRN},
                              {Z_TETRAMINO, {"ZZ ",
                                			 " ZZ"}, 3, 2, ANSI_COLOR_RED},
                              {T_TETRAMINO, {" O ",
                                			 "OOO"}, 3, 2, ANSI_COLOR_MAG}};

void game_over(struct game_data *g) {
    int winner = -1;
    char *player2_name = g->game_mode == MULTI ? "giocatore 2" : "CPU";
    clear_screen();
	printg("gameover", ANSI_COLOR_RED);
    printc(ANSI_COLOR_CYN, "I risultati della tua partita:\n");

    if (g->game_mode == SINGLE) {
        printf("Punti: %d\n", g->score1);
    } else {
        printf("Punti giocatore 1: %d\n"
               "Punti %s: %d\n", g->score1, player2_name, g->score2);
        if (g->remaining_tetraminos > 0) {
            winner = g->turn == PLAYER_1 ? PLAYER_2 : PLAYER_1;
        } else if (g->score1 > g->score2) {
            winner = PLAYER_1;
        } else if (g->score2 > g->score1) {
            winner = PLAYER_2;
        }
        if (winner != -1) {
            printf("Il giocatore %d ha vinto la partita!\n", winner + 1);
        } else {
            printf("Pareggio!\n");
        }
    }
    printf("Tetramini rimasti: %d\n\n", g->remaining_tetraminos);
    printf("Premi invio per tornare al menu principale\n");
}

void new_tetramino(struct game_data *g) {
	int i;
	struct tetraminos *curr_player_next_tetraminos = g->turn == PLAYER_2 && g->game_mode != SINGLE ? g->next_tetraminos2 : g->next_tetraminos1;

    if (g->remaining_tetraminos == 0) {
        g->gameover = 1;
        return;
    }
	
	/* Populate */
	if(g->remaining_tetraminos == g->total_tetraminos+1){
		for(i = 0; i < 4; i++){
			g->next_tetraminos1[i] = tetramino[0 + rand() / (RAND_MAX / (6 - 0 + 1) + 1)];
			g->current_tetramino = g->next_tetraminos1[0];
			if(g->game_mode != SINGLE){
				g->next_tetraminos2[i] = tetramino[0 + rand() / (RAND_MAX / (6 - 0 + 1) + 1)];
			}
		}
	} else {
		if(g->remaining_tetraminos < g->total_tetraminos){
			g->current_tetramino = curr_player_next_tetraminos[0];
		}
		for(i = 0; i < 3; i++){
			curr_player_next_tetraminos[i] = curr_player_next_tetraminos[i+1];
		}
		curr_player_next_tetraminos[3] = tetramino[0 + rand() / (RAND_MAX / (6 - 0 + 1) + 1)];
	}
    --g->remaining_tetraminos;
}

void init(struct game_data *g) {
	g->window_w = term_size().ws_col;
    g->gameover = 0;
    g->collision = 0;
    g->total_tetraminos =
        g->game_mode == SINGLE ? (MAX_TETRAMINOS * 7) : (MAX_TETRAMINOS * 7) * 2;
    g->remaining_tetraminos = g->total_tetraminos + 1;
    g->matrix1 = create_array_of_array(MATRIX_W, MATRIX_H, TRUE);
    g->score1 = 0;
	g->level1 = 1;
    g->turn = PLAYER_1;
	g->fullrows = 0;
    if (g->game_mode != SINGLE) {
        g->matrix2 = create_array_of_array(MATRIX_W, MATRIX_H, TRUE);
        g->score2 = 0;
		g->level2 = 1;
    }
    new_tetramino(g);
    g->x = (MATRIX_W - g->current_tetramino.w) / 2;
    g->y = MATRIX_Y_MIN-2;
}

void rows_invert(struct game_data *g) {
    int x, y;
    char **matrix = g->turn == PLAYER_1 ? g->matrix2 : g->matrix1;

    for (y = MATRIX_H - 1; y >= (MATRIX_H - g->fullrows); --y) {
        for (x = 0; x < MATRIX_W; ++x) {
			matrix[y][x] = matrix[y][x] == EMPTY_CHAR ? 'X' : EMPTY_CHAR;
        }
    }
}

void fullrow_check(struct game_data *g, char **matrix) {
    int y, x, count;
    int full_lines = 0;
    int *score = g->turn == PLAYER_1 ? &g->score1 : &g->score2;
	int *level = g->turn == PLAYER_1 ? &g->level1 : &g->level2;

	for (y=0; y <= MATRIX_H-1; ++y){
		count = 0;
		for(x=0; x <= MATRIX_W - 1; ++x){
			if(matrix[y][x] != EMPTY_CHAR){
				++count;
			}
		}
		if (count == MATRIX_W) {
			++full_lines;
		} else if (full_lines > 0) {
			break;
		}
	}

    if (full_lines > 0) {
        g->fullrows = full_lines;
        if (g->turn == PLAYER_CPU) {
            return;
        }

		if (full_lines >= 3 && g->game_mode != SINGLE) {
            rows_invert(g);
        }

		for(y -= 1; y >= 0; --y){
			for(x = 0; x < MATRIX_W; ++x){
				matrix[y][x] = y-full_lines >= 0 ? matrix[y - full_lines][x] : EMPTY_CHAR;
			}
		}
    }

    switch (full_lines) {
    case 1:
        *score += 1;
        break;
    case 2:
        *score += 3;
        break;
    case 3:
        *score += 6;
        break;
    case 4:
        *score += 12;
        break;
    }

	*level = *score/10 < 15 ? (*score/10)+1 : 15;
}

int collision_check(struct game_data *g) {
    int x, y;
    char **matrix = g->turn == PLAYER_1 ? g->matrix1 : g->matrix2;
	g->collision = 0;
    for (x = 0; x < g->current_tetramino.w; ++x) {
        for (y = 0; y < g->current_tetramino.h; ++y) {
            if (g->y >= MATRIX_Y_MIN && g->current_tetramino.mino[y][x] != EMPTY_CHAR && ((g->x + x >= MATRIX_W || g->y + y >= MATRIX_H) || (matrix[g->y + y][g->x + x] != EMPTY_CHAR && matrix[g->y + y][g->x + x] != GHOST_MINO))) {
                if (g->turn != PLAYER_CPU && g->y < g->current_tetramino.h/2) {
                    g->gameover = 1;
					g->collision = 1;
                } else if (g->turn == PLAYER_CPU && g->y < g->current_tetramino.h-1) {
					g->collision = 2;
				} else {
					g->collision = 1;
				}
            }
        }
    }

    return g->collision;
}

void help() {
    clear_screen();
    printc(ANSI_COLOR_GRN, "Comandi di gioco\n");
    printf("a - muovi il tetramino a sinistra\n");
    printf("d - muovi il tetramino a destra\n");
    printf("s - inserisci il tetramino e muovilo verso il basso\n");
    printf("r - ruota il tetramino in senso orario\n");
    printf("spazio - fai cadere il tetramino\n");
    printf("c - ricomincia la partita\n");
    printf("q - esci dalla partita\n");
	printf("\nDifferenze multiplayer:\n");
	printf("freccia sinistra - muovi il tetramino a sinistra\n");
	printf("fresccia destra - muovi il tetramino a destra\n");
	printf("freccia in basso - inserisci il tetramino e muovilo verso il basso\n");
	printf("trattino alto ('-') - ruota il tetramino in senso orario\n\n");
    printf("Premi invio per tornare al menu principale");
}

bool_t rotate_fix(struct game_data *g, char **matrix) { 
    int x, y, count ;
	bool_t is_fixable = TRUE;

    while (collision_check(g) && is_fixable) {
		count = 0;
        if (g->y + g->current_tetramino.h > MATRIX_H) {
            for (x = 0; x < g->current_tetramino.w; ++x) {
				if(g->y-1 >= MATRIX_Y_MIN && g->y-1 < MATRIX_H && g->x+x >= MATRIX_X_MIN && g->x+x < MATRIX_W){
					if (matrix[g->y - 1][g->x + x] == EMPTY_CHAR) {
						++count;
					}
				}
            }
            if (count == g->current_tetramino.w) {
                --g->y;
                g->collision = 0;
            } else {
				is_fixable = FALSE;
			}
        	count = 0;
        } 
        /* Checks for a collision on the right side */
        if (g->x > MATRIX_X_MIN) {
            for (y = 0; y < g->current_tetramino.h; ++y) {
				if(g->y+y >= MATRIX_Y_MIN && g->y+y < MATRIX_H && g->x-1 >= MATRIX_X_MIN && g->x-1 < MATRIX_W){
					if (matrix[g->y + y][g->x - 1] == EMPTY_CHAR) {
						++count;
					}
				}
            }
            if (count == g->current_tetramino.h) {
                --g->x;
                g->collision = 0;
            } else {
				is_fixable = FALSE;
			}
        } 
		if(count == 0){
			is_fixable = FALSE;
		}
    }
	g->gameover = 0;
	return is_fixable;
}

void rotate(struct game_data *g, bool_t check_collision) {
    int x, y;
	int orig_x = g->x;
	int orig_y = g->y;
    char **matrix = g->turn == PLAYER_1 ? g->matrix1 : g->matrix2;
    struct tetraminos old_tetramino = g->current_tetramino;

    g->current_tetramino.h = old_tetramino.w;
    g->current_tetramino.w = old_tetramino.h;
    
	for (y = 0; y < old_tetramino.h; ++y) {
        for (x = 0; x < old_tetramino.w; ++x) {
            g->current_tetramino.mino[x][y] = old_tetramino.mino[old_tetramino.h - y - 1][x];
        }
	}

	if (check_collision && !rotate_fix(g, matrix)) {
		g->current_tetramino = old_tetramino;
		g->x = orig_x;
		g->y = orig_y;
	}
}

void print_next_tetraminos(struct game_data *g, int screen_x, turn_t player){
	int i, x, y, offset;
	int start_y = 7;
	char mino;
	struct tetraminos tetramino_display;
	struct tetraminos *user_next_tetraminos = player == PLAYER_1 ? g->next_tetraminos1 : g->next_tetraminos2;
	
	screen_x = player == PLAYER_1 ? screen_x - 2 : screen_x + (MATRIX_W + 3 + MAX_TETRAMINO_SIDE); 

	for(i = 0; i < MAX_TETRAMINOS_PREVIEW; i++){
		tetramino_display = user_next_tetraminos[i];
		offset = i > 0 ? offset + (user_next_tetraminos[i-1].h + 1) : 1;
		for (y = 0; y <= MAX_TETRAMINO_SIDE && g->remaining_tetraminos > 0; ++y) {
			for (x = 0; x <= MAX_TETRAMINO_SIDE; ++x) {
				ANSI_CURSOR_POS(( screen_x - (MAX_TETRAMINO_SIDE / 2)) + x + 1, start_y + offset + y);
				mino = tetramino_display.mino[y][x];
				if (mino == NULL_CHAR || mino == EMPTY_CHAR) {
					printf("%c", EMPTY_CHAR);
				} else {
					printc(NULL, &mino);
				}
			}
			printf("\n");
		}
		printf("\n");
	}

}

void print_stats(struct game_data *g) {
    int start_y = 8;
    int mid_screen = g->window_w / 2;
    char *turn_lbl = "TURNO GIOCATORE ";
    int turn_lbl_size = strlen(turn_lbl);
    char *remaining_tetraminos_lbl = "TETRAMINI RIMANENTI: ";
    int remaining_tetraminos_lbl_size = strlen(remaining_tetraminos_lbl) + 1;

    if (g->game_mode != SINGLE) {
        ANSI_CURSOR_POS(mid_screen - (turn_lbl_size / 2), start_y++);
        printf("%s%d", turn_lbl, g->turn + 1);
    }

    ANSI_CURSOR_POS(mid_screen - (remaining_tetraminos_lbl_size / 2), start_y++);
    printf("%s%d", remaining_tetraminos_lbl, g->remaining_tetraminos);
}

void print_score(int x, int y, int level, int score){
	char *lvl_lbl = "LIVELLO:";
	int lvl_lbl_size = strlen(lvl_lbl) + 2;
	int lvl_size = 10%level == 0 ? 1 : 2;
	int i;

	ANSI_CURSOR_POS(x + 1, y);
	printf("╭");
	for(i=0; i<(lvl_lbl_size+lvl_size); ++i){
		printf("─");
	}
	printf("╮");
	ANSI_CURSOR_POS(x + 1, y + 1);
	printf("│ LIVELLO: %d ", level);
	ANSI_CURSOR_POS(x+(lvl_lbl_size+lvl_size+1) + 1, y + 1);
	printf("│");
	ANSI_CURSOR_POS(x + 1, y + 2);
	printf("│ PUNTI: %d", score);
	ANSI_CURSOR_POS(x+(lvl_lbl_size+lvl_size+1) + 1, y + 2);
	printf("│");
	ANSI_CURSOR_POS(x + 1, y + 3);
	printf("╰");
	for(i=0; i<(lvl_lbl_size+lvl_size); ++i){
		printf("─");
	}
	printf("╯");
}

void print_matrix(struct game_data *g, char **aux_matrix) {
    int x, y, player_n;
    char mino;
    int mid = g->window_w / 2;
    int start_x = mid / 3;
    int start_x_matrix2 = g->window_w - ((mid / 3) + MATRIX_W + 3);
    int start_y = 2;
    int score = g->score1;
	int level = g->level1;
    int players = g->game_mode == SINGLE ? 1 : 2;
    char **matrix = g->turn == PLAYER_1 ? aux_matrix : g->matrix1;
    char **matrix2;

    if (g->game_mode != SINGLE) {
        matrix2 = g->turn == PLAYER_1 ? g->matrix2 : aux_matrix;
    }

    print_stats(g);

    for (player_n = PLAYER_1; player_n < players; ++player_n) {
        if (player_n == PLAYER_2) {
            start_x = start_x_matrix2;
            start_y = 2;
            score = g->score2;
			level = g->level2;
            matrix = matrix2;
        }

		print_next_tetraminos(g, start_x, player_n);
		print_score(start_x, start_y, level, score);

        ANSI_CURSOR_POS(start_x += 2, start_y += MAX_TETRAMINO_SIDE);
		printf("╭");
        for (x = 0; x < MATRIX_W; ++x) {
            ANSI_CURSOR_POS(start_x + 1 + x, start_y);
			printf("─");
        }
		printf("╮");
        ++start_y;
        for (y = 0; y < MATRIX_H; ++y) {
            ANSI_CURSOR_POS(start_x, start_y + y);
			printf("│");
            for (x = 0; x < MATRIX_W; ++x) {
                mino = matrix[y][x];
                if (mino != EMPTY_CHAR) {
                    printc(NULL, &mino);
                } else {
                    printf("%c", EMPTY_CHAR);
                }
            }
			printf("│");
            printf("\n");
        }
        ANSI_CURSOR_POS(start_x, start_y + y);
		printf("╰");
        ++start_x;
        for (x = 0; x < MATRIX_W; ++x) {
            ANSI_CURSOR_POS(start_x + x, start_y + MATRIX_H);
			printf("─");
        }
		printf("╯");
		printf("\n");
    }
}

void tetramino_insert(struct game_data *g, char **aux_matrix, bool_t is_ghost){
	int x, y;
	for (x = 0; x < g->current_tetramino.w; ++x) {
		for (y = 0; y < g->current_tetramino.h; ++y) {
			if (g->x+x >= MATRIX_X_MIN && g->y+y >= MATRIX_Y_MIN && g->current_tetramino.mino[y][x] != EMPTY_CHAR) {
				aux_matrix[g->y + y][g->x + x] = is_ghost ? GHOST_MINO : g->current_tetramino.mino[y][x];
			}
		}
	}
}

void ghost(struct game_data *g, char **aux_matrix){
	int orig_x = g->x;
	int orig_y = g->y;
	int orig_collision = g->collision;
	struct tetraminos orig_tetramino = g->current_tetramino;

	hard_drop(g, FALSE);
	tetramino_insert(g, aux_matrix, TRUE);

	g->x = orig_x;
	g->y = orig_y;
	g->collision = orig_collision;
	g->current_tetramino = orig_tetramino;
}

void matrix(struct game_data *g) {
    char **matrix = g->turn == PLAYER_1 ? g->matrix1 : g->matrix2;
    char **aux_matrix = create_array_of_array(MATRIX_W, MATRIX_H, FALSE);

    copy_array_of_array(matrix, aux_matrix, MATRIX_W, MATRIX_H);
    if (g->y >= MATRIX_Y_MIN - g->current_tetramino.h/2) {
		ghost(g, aux_matrix);
		tetramino_insert(g, aux_matrix, FALSE);
    }
	if(g->turn != PLAYER_CPU){
		print_matrix(g, aux_matrix);
	}
	if(!g->gameover && g->collision && g->move == DOWN){
		fullrow_check(g, aux_matrix);
		if(g->turn != PLAYER_CPU){
    		fullrow_check(g, aux_matrix);
			copy_array_of_array(aux_matrix, matrix, MATRIX_W, MATRIX_H);
			if (g->game_mode != SINGLE) {
				g->turn = !g->turn;
				matrix = g->turn == PLAYER_1 ? g->matrix1 : g->matrix2;
			}
			new_tetramino(g);
			g->x = (MATRIX_W - g->current_tetramino.w) / 2;
			g->y = (MATRIX_Y_MIN- g->current_tetramino.h/2)-1;
			g->collision = 0;
			clear_screen();
			print_matrix(g, matrix);
		}
	}
    free_array_of_array(aux_matrix, MATRIX_H);
}

void drop(struct game_data *g) {
	if(g->remaining_tetraminos == g->total_tetraminos && g->y == MATRIX_X_MIN-1){
		new_tetramino(g);
	}
    ++g->y;
    if (collision_check(g)) {
        --g->y;
    }
}

void hard_drop(struct game_data *g, bool_t check_collision){
	g->collision = 0;
	while (!g->collision){
		drop(g);
	}
	if(!check_collision){
		g->collision = 0;
	}
}

float fall_speed(struct game_data *g){
	int level = g->turn == PLAYER_1 ? g->level1 : g->level2;
	return pow((0.8f - ((level - 1) * 0.007f)), level - 1);
}

void cpu_strategy(struct game_data *g) {
    int i, j;
    int empty_rows = 0;
    int min_x = -1;
    int min_y = -1;
    int rotation_count = 1;
    int best_rotation;
	bool_t moved = FALSE;

    g->fullrows = 0;
    g->turn = PLAYER_CPU;
    g->collision = 0;

    for (i = 0; i < 4; ++i) {
        if (i > 0) {
			g->x = (MATRIX_W - g->current_tetramino.w)/2;
			g->y = g->current_tetramino.h;
            g->move = ROTATE;
            rotate(g, FALSE);
            ++rotation_count;
        }
        for (j = 0; j < MATRIX_W; ++j){
			g->move = DOWN;
    		g->collision = 0;
			g->x = j;
			g->y = (MATRIX_Y_MIN - g->current_tetramino.h/2)-1;
			hard_drop(g, TRUE);
			matrix(g);
            if (g->fullrows > empty_rows) {
				if(g->collision != 2){
					empty_rows = g->fullrows;
					min_x = g->x;
					min_y = g->y;
					best_rotation = rotation_count;
					moved = TRUE;
				}
            } else if ((g->y > min_y) && empty_rows == 0) {
				if(g->collision != 2){
					min_x = g->x;
					min_y = g->y;
					best_rotation = rotation_count;
					moved = TRUE;
				}
            }
        }
    }
	if(!moved){
		g->gameover = 1;
		return;
	}
	/* Returns to the best rotation */
    for (i = 0; i < best_rotation; ++i) {
		g->x = (MATRIX_W - g->current_tetramino.w)/2;
		g->y = MATRIX_Y_MIN-1;
        rotate(g, FALSE);
    }

    g->x = min_x;
    g->y = min_y;
    g->turn = PLAYER_2;
    g->collision = 1;
	g->move = DOWN;
    matrix(g);
}

void run(struct game_data *g) {
    int ch;
    float time_beg, time_end;

    clear_screen();
    matrix(g);
	time_float(&time_beg);

    while ((ch = tolower(getchar())) != NULL_CHAR && !g->gameover) {
		matrix(g);
		move_sanitize(&ch, g->turn);

        switch (ch) {
        case LEFT:
            g->move = LEFT;
            if (g->y >= MATRIX_X_MIN-1 && g->x > MATRIX_X_MIN) {
                --g->x;
                if (collision_check(g)) {
                    ++g->x;
                }
            }
            break;
        case RIGHT:
            g->move = RIGHT;
            if (g->y >= MATRIX_X_MIN-1 && g->x < (MATRIX_W - g->current_tetramino.w)) {
                ++g->x;
                if (collision_check(g)) {
                    --g->x;
                }
            }
            break;
        case DOWN:
    		g->move = DOWN;
			drop(g);
            break;
        case ROTATE:
            g->move = ROTATE;
			if (g->y >= MATRIX_Y_MIN-1){
            	rotate(g, TRUE);
			}
            break;
        case HARD_DROP:
    		g->move = DOWN;
			hard_drop(g, TRUE);
            break;
        case CLEAR:
            free_array_of_array(g->matrix1, MATRIX_H);
            if (g->game_mode != SINGLE) {
            	free_array_of_array(g->matrix2, MATRIX_H);
            }
            init(g);
            break;
		case QUIT:
			g->gameover = 1;
			return;
        }

		matrix(g);

		while (!g->gameover && !kbhit()) {
			if (g->game_mode == CPU && g->turn == PLAYER_2) {
				cpu_strategy(g);
			}
			window_resize(&g->window_w);
			time_float(&time_end);
			if ((time_end - time_beg) >= fall_speed(g)) {
    			g->move = DOWN;
				drop(g);
				matrix(g);
				time_beg = time_end;
			}
		}
    }
    game_over(g);
    while (getchar() != '\n');
}

void print_title() {
    char options[] = "────────────────────────────────────────\n"
                     "Seleziona una modalità di gioco:\n"
                     "1) Single player\n"
                     "2) Multi player \n"
                     "3) Contro la CPU\n"
                     "4) Informazioni\n"
                     "5) Esci\n";
    clear_screen();
	printg("logo", ANSI_COLOR_GRN);
    printf("%s", options);
}

void title(struct game_data *g) {
    int game_mode;

    print_title();
    while ((game_mode = getchar()) != NULL_CHAR) {
        switch (game_mode) {
        case '1':
            g->game_mode = SINGLE;
            init(g);
            run(g);
            free_array_of_array(g->matrix1, MATRIX_H);
        	print_title();
            break;
        case '2':
            g->game_mode = MULTI;
            init(g);
            run(g);
            free_array_of_array(g->matrix1, MATRIX_H);
            free_array_of_array(g->matrix2, MATRIX_H);
        	print_title();
            break;
        case '3':
            g->game_mode = CPU;
            init(g);
            run(g);
            free_array_of_array(g->matrix1, MATRIX_H);
            free_array_of_array(g->matrix2, MATRIX_H);
        	print_title();
            break;
        case '4':
            help();
            while (getchar() != '\n');
        	print_title();
            break;
        case '5':
            term_setup(T_RESTORE);
            exit(EXIT_SUCCESS);
        default:
            break;
        }
    }
}

int main() {
    struct game_data g;
    srand(time(NULL));
    term_setup(T_GAME);
    title(&g);
    return 0;
}
