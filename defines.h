#ifndef DEFINES_H
#define DEFINES_H

#include <stdbool.h>
#include <stdlib.h> // Required for size_t

// --- Game Constants ---
#define ROWS 6
#define COLS 7
#define CONNECT_LEN 4 // How many pieces in a row to win

#define EMPTY 0
#define PLAYER1 1 // Human
#define PLAYER2 2 // AI

// --- MCTS Constants ---
#define MCTS_ITERATIONS 10000 // Key parameter for AI strength. Increase for stronger AI (but longer thinking time).
#define UCB_C 1.414         // Exploration constant (sqrt(2) is common)

// --- MCTS Node Structure ---
typedef struct MCTSNode {
    int board[ROWS][COLS];
    int player; // Player whose turn it is *at this node*
    int move;   // The move (column) that led to this state (-1 for root)

    struct MCTSNode *parent;
    struct MCTSNode *children[COLS]; // Max COLS possible moves
    int num_children;

    int wins;   // Number of wins from simulations passing through this node
    int visits; // Number of times this node was visited

    int untried_moves[COLS]; // Columns not yet explored from this node
    int num_untried_moves;

    bool is_terminal;
    int terminal_winner; // 0 = draw, PLAYER1, PLAYER2, -1 = not terminal

} MCTSNode;

#endif // DEFINES_H
