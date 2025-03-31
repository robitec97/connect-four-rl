#include "mcts.h"
#include "connectfour.h" // Make sure connect4 functions are available
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // For memcpy
#include <math.h>
#include <float.h>  // For DBL_MAX
#include <time.h>   // For rand() seeding


MCTSNode* create_node(MCTSNode* parent, int move, int board[ROWS][COLS], int player) {
    MCTSNode* node = (MCTSNode*)malloc(sizeof(MCTSNode));
    if (!node) {
        perror("Failed to allocate MCTS node");
        return NULL;
    }

    copy_board(node->board, board);
    node->player = player;
    node->move = move;
    node->parent = parent;
    node->num_children = 0;
    node->wins = 0;
    node->visits = 0;

    // Initialize children pointers to NULL
    for (int i = 0; i < COLS; ++i) {
        node->children[i] = NULL;
    }

    // Check terminal state
    int winner = check_game_over(node->board);
    if (winner != -1) {
        node->is_terminal = true;
        node->terminal_winner = winner;
        node->num_untried_moves = 0;
    } else {
        node->is_terminal = false;
        node->terminal_winner = -1;
        // Populate untried moves
        int* valid_moves = get_valid_moves(node->board, &(node->num_untried_moves));
        if (valid_moves) {
            memcpy(node->untried_moves, valid_moves, node->num_untried_moves * sizeof(int));
            free(valid_moves);
        } else {
             node->num_untried_moves = 0; // Handle allocation failure
        }
    }

    return node;
}

// Recursively free node and descendants
void free_node(MCTSNode* node) {
    if (!node) return;
    for (int i = 0; i < node->num_children; i++) {
        if(node->children[i]) {
            free_node(node->children[i]);
            node->children[i] = NULL; // Good practice
        }
    }
    free(node);
}

// --- UCB1 Calculation ---
double ucb1(MCTSNode* node) {
    if (node->visits == 0) {
        return DBL_MAX; // Prioritize unvisited nodes (infinite score)
    }
    if (node->parent == NULL || node->parent->visits == 0) {
         // Should not happen for children of root after first visit, but safe guard
        return (double)node->wins / node->visits;
    }
    // UCB1 formula
    return ((double)node->wins / node->visits) +
           UCB_C * sqrt(log((double)node->parent->visits) / node->visits);
}

// --- Selection Phase ---
MCTSNode* select_node(MCTSNode* node) {
    while (!node->is_terminal) {
        if (node->num_untried_moves > 0) {
            return expand_node(node); // If node has untried moves, expand it
        }
        if (node->num_children == 0) {
             // Should ideally not happen if not terminal, unless board is full
             // but expansion failed or no valid moves (which means terminal)
             return node;
        }

        // Select best child using UCB1
        MCTSNode* best_child = NULL;
        double best_score = -1.0;

        for (int i = 0; i < node->num_children; i++) {
            if(node->children[i]) { // Ensure child pointer is valid
                double score = ucb1(node->children[i]);
                if (score > best_score) {
                    best_score = score;
                    best_child = node->children[i];
                }
            }
        }
        if (best_child == NULL) {
           // This might indicate an issue, or a state with no valid next moves
           // (which should have been caught as terminal). Return current node.
           fprintf(stderr, "Warning: No best child found in select_node for non-terminal node.\n");
           return node;
        }
        node = best_child; // Move down to the best child
    }
    return node; // Reached a terminal node
}


// --- Expansion Phase ---
MCTSNode* expand_node(MCTSNode* node) {
    if (node->is_terminal || node->num_untried_moves == 0) {
        return node; // Cannot expand terminal or fully expanded nodes
    }

    // Select an untried move randomly
    int move_index = rand() % node->num_untried_moves;
    int move_col = node->untried_moves[move_index];

    // Create the board state for the new child
    int next_board[ROWS][COLS];
    copy_board(next_board, node->board);
    int row = get_next_open_row(next_board, move_col);
    if (row != -1) { // Should always be valid if move was in untried_moves
        drop_piece(next_board, row, move_col, node->player);
    } else {
        fprintf(stderr, "Error: Invalid move selected during expansion.\n");
        // Remove the invalid move and try again or return node? For now, just return node.
        // A more robust approach might remove the move and recurse/retry.
        return node;
    }


    // Create the new child node
    int next_player = (node->player == PLAYER1) ? PLAYER2 : PLAYER1;
    MCTSNode* new_child = create_node(node, move_col, next_board, next_player);

    if(!new_child) {
        // Allocation failed
        return node; // Can't expand
    }

    // Add the new child to the parent's children list
    // Find the first NULL spot, should correspond to num_children index
    if(node->num_children < COLS) {
         node->children[node->num_children++] = new_child;
    } else {
        fprintf(stderr, "Error: Exceeded maximum children capacity.\n");
        free_node(new_child); // Clean up the created node
        // This case should theoretically not happen with COLS children max
        // Need to handle the removal of the tried move carefully now
    }


    // Remove the move from untried_moves
    // Swap with the last element and decrease count for efficiency
    node->untried_moves[move_index] = node->untried_moves[node->num_untried_moves - 1];
    node->num_untried_moves--;

    return new_child;
}

// --- Simulation Phase (Random Playout) ---
int simulate_random_playout(MCTSNode* node) {
    int temp_board[ROWS][COLS];
    copy_board(temp_board, node->board);
    int current_player = node->player;
    int winner = node->terminal_winner; // Check if starting node was already terminal

    // Simulate game until it ends
    while (winner == -1) {
        int num_valid_moves;
        int* valid_moves = get_valid_moves(temp_board, &num_valid_moves);

        if (num_valid_moves == 0 || !valid_moves) {
             // Should ideally mean a draw if no win yet, board is full
            winner = check_game_over(temp_board); // Double check state
            if (winner == -1) winner = 0; // If no winner and no moves, it's a draw
            if (valid_moves) free(valid_moves); // Free if allocated
            break;
        }

        // Choose a random valid move
        int random_move_col = valid_moves[rand() % num_valid_moves];
        free(valid_moves); // Free the list after use

        int row = get_next_open_row(temp_board, random_move_col);
        if (row != -1) {
             drop_piece(temp_board, row, random_move_col, current_player);
        } else {
            fprintf(stderr, "Error during simulation: get_next_open_row failed for valid move.\n");
             // This indicates a potential logic error elsewhere
             winner = 0; // Assume draw? Or maybe return an error code?
             break;
        }


        // Check if the game ended
        winner = check_game_over(temp_board);

        // Switch player for the next turn
        current_player = (current_player == PLAYER1) ? PLAYER2 : PLAYER1;
    }

    return winner; // Return winner (PLAYER1/PLAYER2) or 0 for draw
}


// --- Backpropagation Phase ---
void backpropagate(MCTSNode* node, int simulation_winner) {
    MCTSNode* current_node = node;
    while (current_node != NULL) {
        current_node->visits++;
        // Important: Increment wins if the simulation winner is the *opponent* of the player
        // whose turn it was at this node. MCTS aims to maximize the score for the player
        // *making the move* from the parent node.
        // Alternatively, track wins for PLAYER1 and PLAYER2 separately.
        // Let's stick to the common convention: win count is for the player who just moved *into* this state.
        // No, let's re-evaluate: The win should be from the perspective of the player *whose turn it is*
        // at the *parent* node. A simpler way is: if the simulation winner matches the *other* player
        // relative to the node's player, it's a win for the path leading here.

        // If the player *at this node* is NOT the winner of the simulation,
        // then the player who moved *to* this node potentially scored.
        // Except draws (winner = 0).
         if (simulation_winner != 0 && current_node->player != simulation_winner) {
             current_node->wins++;
         }
         // Alternative: Could give 0.5 wins for a draw, but standard MCTS often just increments visits.
         // if (simulation_winner == 0) { current_node->wins += 0.5; } // Requires changing wins to double

        current_node = current_node->parent;
    }
}


// --- Main MCTS Function ---
int mcts_get_best_move(int current_board[ROWS][COLS], int current_player) {
    // Seed random number generator if not already done globally
    // srand(time(NULL)); // Consider seeding once in main()

    MCTSNode* root = create_node(NULL, -1, current_board, current_player);
    if (!root) return -1; // Error creating root

    if(root->is_terminal) {
        fprintf(stderr, "Warning: MCTS called on a terminal state.\n");
        free_node(root);
        return -1; // No moves possible
    }
    if(root->num_untried_moves == 0 && root->num_children == 0) {
         fprintf(stderr, "Warning: MCTS called on a state with no valid moves, but not terminal?\n");
         free_node(root);
         return -1;
    }


    for (int i = 0; i < MCTS_ITERATIONS; i++) {
        // 1. Selection
        MCTSNode* leaf = select_node(root);

        // 2. Simulation (if selection didn't end on a terminal node already expanded)
        // Note: select_node already calls expand_node if appropriate.
        // 'leaf' might be the newly expanded node or a terminal node.
        int simulation_result = simulate_random_playout(leaf);

        // 3. Backpropagation
        backpropagate(leaf, simulation_result);

        // Optional: Print progress
        // if ((i + 1) % (MCTS_ITERATIONS / 10) == 0) {
        //     printf("MCTS Iteration %d/%d\n", i + 1, MCTS_ITERATIONS);
        // }
    }

    // Choose the best move based on the most visited child of the root
    MCTSNode* best_child = NULL;
    int most_visits = -1;

    //printf("\nMCTS Root Node Analysis:\n"); // Debug
    //printf("Root Visits: %d\n", root->visits);

    for (int i = 0; i < root->num_children; i++) {
        MCTSNode* child = root->children[i];
        if (child) {
             // Debug print:
             //printf("  Child Move: %d, Wins: %d, Visits: %d, Win Rate: %.2f%%\n",
             //       child->move, child->wins, child->visits,
             //       child->visits > 0 ? 100.0 * child->wins / child->visits : 0.0);

            if (child->visits > most_visits) {
                most_visits = child->visits;
                best_child = child;
            }
        }
    }

    int best_move = -1;
    if (best_child != NULL) {
        best_move = best_child->move;
         //printf("Chosen Move: %d (Visits: %d)\n", best_move, most_visits);
    } else if (root->num_untried_moves > 0) {
        // Fallback: If somehow no children were explored (e.g., low iterations),
        // pick a random untried move. Should be rare with sufficient iterations.
        fprintf(stderr, "Warning: No children explored, picking random untried move.\n");
        best_move = root->untried_moves[rand() % root->num_untried_moves];
    } else {
         fprintf(stderr, "Error: MCTS could not determine a best move.\n");
         // Maybe pick the first valid move?
         int num_valid = 0;
         int *valid = get_valid_moves(current_board, &num_valid);
         if(num_valid > 0) best_move = valid[0];
         if(valid) free(valid);
    }


    // Clean up the MCTS tree
    free_node(root);

    return best_move;
}
