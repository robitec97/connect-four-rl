#ifndef CONNECTFOUR_H
#define CONNECTFOUR_H

#include "defines.h"
#include <stdbool.h>

// --- Function Declarations ---

// Board management
void init_board(int board[ROWS][COLS]);
void print_board(int board[ROWS][COLS]);
bool is_valid_location(int board[ROWS][COLS], int col);
int get_next_open_row(int board[ROWS][COLS], int col);
void drop_piece(int board[ROWS][COLS], int row, int col, int player);
void copy_board(int dest[ROWS][COLS], int src[ROWS][COLS]);

// Game state checking
bool check_win(int board[ROWS][COLS], int player);
bool is_board_full(int board[ROWS][COLS]);
int check_game_over(int board[ROWS][COLS]); // Returns winner (PLAYER1/PLAYER2), 0 for draw, -1 for ongoing

// Move generation
int* get_valid_moves(int board[ROWS][COLS], int* num_moves); // Returns dynamically allocated array

#endif // CONNECTFOUR_H
