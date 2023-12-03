#ifndef SEARCH_H_
#define SEARCH_H_

#include "types.h"
#include "board.h"
#include "bitboard.h"
#include "movegen.h"
#include "time.h"

// Choice of algorithm
#define MCTS
// #define AB



/**
 @brief Quiescence search
 @param alpha the lowerbound
 @param beta the upperbound
 @param board the board position to search
 @param info search info: time, depth to search, etc.
*/
int quiescence(int alpha, int beta, board_t *board, searchinfo_t *info, stack_t *stack);

/**
 @brief Searches the current board state for the best move
 @param board the board state to search
 @param info search info: time, depth to search, etc.
*/
void search(board_t *board, searchinfo_t *info);

/**
 @brief Initializes the values used for various reductions,
 like the Late Move Reduction
*/
void init_reductions();

// Constants / parameters
// [LMR]
constexpr int lmr_fully_searched_req = 4;
constexpr int lmr_depth_req = 3;
constexpr int iir_depth_req = 5;

#endif // SEARCH_H_
