#include <stdio.h>
#include <stdlib.h>
#include <time.h>   // For srand
#include <limits.h> // For INT_MAX

#include "defines.h"
#include "connect4.h"
#include "mcts.h"

// Helper to get integer input safely
int get_int_input(const char* prompt) {
    int value;
    long long input_l;
    char buffer[100];
    char *endptr;

    while (1) {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            // Handle EOF or input error
            printf("Input error. Exiting.\n");
            exit(EXIT_FAILURE);
        }

        // Remove trailing newline if present
        buffer[strcspn(buffer, "\n")] = 0;

        input_l = strtoll(buffer, &endptr, 10);

        // Check for conversion errors, overflow/underflow, and trailing characters
        if (endptr == buffer || *endptr != '\0' || input_l < INT_MIN || input_l > INT_MAX) {
            printf("Invalid input. Please enter an integer.\n");
        } else {
            value = (int)input_l;
            return value;
        }
    }
}


int main() {
    int board[ROWS][COLS];
    bool game_over = false;
    int turn = PLAYER1; // Player 1 starts

    // Seed random number generator ONCE
    srand(time(NULL));

    init_board(board);
    print_board(board);

    while (!game_over) {
        int col = -1;
        int row = -1;

        if (turn == PLAYER1) { // Human's turn
            bool valid_move = false;
            while(!valid_move) {
                col = get_int_input("Player 1 (X), enter column (0-6): ");

                if (is_valid_location(board, col)) {
                     row = get_next_open_row(board, col);
                     if (row != -1) { // Should be redundant if is_valid_location is true
                         valid_move = true;
                     } else {
                          printf("Error finding open row in valid column %d.\n", col);
                     }

                } else {
                    printf("Invalid column choice. Please try again.\n");
                }
            }
        } else { // AI's turn (PLAYER2)
            printf("AI Player 2 (O) is thinking...\n");
            col = mcts_get_best_move(board, PLAYER2);

            if (col == -1 || !is_valid_location(board, col)) {
                 printf("MCTS Error: AI failed to provide a valid move. Exiting.\n");
                 // As a fallback, try *any* valid move if MCTS failed completely
                 int num_moves = 0;
                 int* moves = get_valid_moves(board, &num_moves);
                 if(num_moves > 0 && moves) {
                    col = moves[0]; // Just pick the first valid one
                    printf("Fallback: AI choosing column %d\n", col);
                 } else {
                     game_over = true; // No moves possible at all? Should be draw state.
                     printf("Error: No valid moves possible for AI.\n");
                 }
                 if(moves) free(moves);

                 if(game_over) continue; // Skip rest of loop if error caused game over
            }
             row = get_next_open_row(board, col);
             printf("AI Player 2 (O) chose column %d\n", col);

        }

        // Drop the piece if row is valid (should always be if logic is correct)
        if (row != -1) {
             drop_piece(board, row, col, turn);
        } else {
             printf("Internal error: Could not place piece in column %d.\n", col);
             game_over = true; // Critical error
             continue;
        }


        print_board(board);

        // Check for win/draw
        int winner = check_game_over(board);
        if (winner != -1) {
            game_over = true;
            if (winner == PLAYER1) {
                printf("**********************\n");
                printf("* Player 1 (X) wins! *\n");
                printf("**********************\n");
            } else if (winner == PLAYER2) {
                 printf("********************\n");
                 printf("* Player 2 (O) wins! *\n");
                 printf("********************\n");
            } else { // winner == 0
                 printf("********************\n");
                 printf("* DRAW!       *\n");
                 printf("********************\n");
            }
        }

        // Switch turns
        turn = (turn == PLAYER1) ? PLAYER2 : PLAYER1;
    }

    return 0;
}
