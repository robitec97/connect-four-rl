#include "connectfour.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // For memcpy

void init_board(int board[ROWS][COLS]) {
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            board[r][c] = EMPTY;
        }
    }
}

void print_board(int board[ROWS][COLS]) {
    printf("\n");
    // Print column numbers
    for (int c = 0; c < COLS; c++) {
        printf(" %d", c);
    }
    printf("\n");
    printf("---------------"); // Adjust width based on COLS
     printf("\n");


    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            char piece = '.';
            if (board[r][c] == PLAYER1) piece = 'X';
            if (board[r][c] == PLAYER2) piece = 'O';
            printf("|%c", piece);
        }
        printf("|\n");
    }
    printf("---------------\n"); // Adjust width based on COLS
}

bool is_valid_location(int board[ROWS][COLS], int col) {
    return col >= 0 && col < COLS && board[0][col] == EMPTY; // Check top row
}

int get_next_open_row(int board[ROWS][COLS], int col) {
    for (int r = ROWS - 1; r >= 0; r--) {
        if (board[r][col] == EMPTY) {
            return r;
        }
    }
    return -1; // Should not happen if is_valid_location was checked
}

void drop_piece(int board[ROWS][COLS], int row, int col, int player) {
    if (row >= 0 && row < ROWS && col >= 0 && col < COLS) {
         board[row][col] = player;
    }
}

void copy_board(int dest[ROWS][COLS], int src[ROWS][COLS]) {
    memcpy(dest, src, ROWS * COLS * sizeof(int));
}


bool check_win(int board[ROWS][COLS], int player) {
    // Horizontal check
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c <= COLS - CONNECT_LEN; c++) {
            bool win = true;
            for (int i = 0; i < CONNECT_LEN; i++) {
                if (board[r][c + i] != player) {
                    win = false;
                    break;
                }
            }
            if (win) return true;
        }
    }

    // Vertical check
    for (int c = 0; c < COLS; c++) {
        for (int r = 0; r <= ROWS - CONNECT_LEN; r++) {
             bool win = true;
            for (int i = 0; i < CONNECT_LEN; i++) {
                if (board[r + i][c] != player) {
                    win = false;
                    break;
                }
            }
            if (win) return true;
        }
    }

    // Positive diagonal check (/)
     for (int r = 0; r <= ROWS - CONNECT_LEN; r++) {
        for (int c = 0; c <= COLS - CONNECT_LEN; c++) {
            bool win = true;
            for (int i = 0; i < CONNECT_LEN; i++) {
                if (board[r + i][c + i] != player) {
                    win = false;
                    break;
                }
            }
            if (win) return true;
        }
    }

    // Negative diagonal check (\)
     for (int r = CONNECT_LEN - 1; r < ROWS; r++) {
        for (int c = 0; c <= COLS - CONNECT_LEN; c++) {
             bool win = true;
            for (int i = 0; i < CONNECT_LEN; i++) {
                if (board[r - i][c + i] != player) {
                    win = false;
                    break;
                }
            }
             if (win) return true;
        }
    }

    return false;
}

bool is_board_full(int board[ROWS][COLS]) {
    for (int c = 0; c < COLS; c++) {
        if (board[0][c] == EMPTY) {
            return false; // Found an empty spot in the top row
        }
    }
    return true; // No empty spots in the top row means full
}

int check_game_over(int board[ROWS][COLS]) {
    if (check_win(board, PLAYER1)) return PLAYER1;
    if (check_win(board, PLAYER2)) return PLAYER2;
    if (is_board_full(board)) return 0; // Draw
    return -1; // Game not over
}


int* get_valid_moves(int board[ROWS][COLS], int* num_moves) {
    int* valid_moves = (int*)malloc(COLS * sizeof(int));
    if (!valid_moves) {
        perror("Failed to allocate memory for valid moves");
        *num_moves = 0;
        return NULL;
    }
    *num_moves = 0;
    for (int c = 0; c < COLS; c++) {
        if (is_valid_location(board, c)) {
            valid_moves[*num_moves] = c;
            (*num_moves)++;
        }
    }
    // Optional: Reallocate to save memory, but usually not critical here
    // int* resized_moves = realloc(valid_moves, (*num_moves) * sizeof(int));
    // if (resized_moves) valid_moves = resized_moves;

    return valid_moves;
}
