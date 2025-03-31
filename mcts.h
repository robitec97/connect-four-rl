#ifndef MCTS_H
#define MCTS_H

#include "defines.h"
#include "connectfour.h" // Include connect4 for game logic functions

// --- MCTS Function Declarations ---

MCTSNode* create_node(MCTSNode* parent, int move, int board[ROWS][COLS], int player);
void free_node(MCTSNode* node); // Recursively frees node and its children
MCTSNode* select_node(MCTSNode* node);
MCTSNode* expand_node(MCTSNode* node);
int simulate_random_playout(MCTSNode* node); // Returns winner (PLAYER1/PLAYER2) or 0 for draw
void backpropagate(MCTSNode* node, int simulation_winner);
int mcts_get_best_move(int current_board[ROWS][COLS], int current_player);

#endif // MCTS_H
