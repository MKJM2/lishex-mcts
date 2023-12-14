#ifndef BOARD_H_
#define BOARD_H_

#include <string>

#include "types.h"
#include "bitboard.h"


/************************/
/* Board representation */
/************************/

// The Board type
/**
 * @brief The board struct
 * Stores all necessary information about the current board state
*/
typedef struct board_t {
    // We store a separate bitboard for each piece (type, color)
    bb_t bitboards[PIECE_NO] = {};
    // In addition to bitboards, we store a regular 8x8 array
    // for quick piece lookups during move-making
    piece_t pieces[SQUARE_NO] = {};
    // Additionally, we store bitboards of all pieces for a given side
    bb_t sides_pieces[BOTH] = {};
    // Side to play (Black = 0, White = 1)
    int turn = 1;
    // Ply of the game in the current search
    int ply = 0;
    // How many halfmoves have been made until current position
    int history_ply = 0;
    // Current castle rights for both players
    int castle_rights = WK | WQ | BK | BQ;
    // Fifty move counter
    int fifty_move = 0;
    // En passant square (if any)
    square_t ep_square = NO_SQ;
    // Zobrist hash key for the current position
    uint64_t key = 0ULL;
    // History of previous positions
    undo_t history[MAX_MOVES];
    // Killer moves for move ordering (cause a beta cutoff but aren't captures)
    // move_t killer1[MAX_DEPTH] = {};
    // move_t killer2[MAX_DEPTH] = {};
    // History heuristic, table indexed by [stm][piece][to square]
    int32_t history_h[BOTH][PIECE_NO][SQUARE_NO] = {};
} board_t;

#ifdef DEBUG
extern void history_trace(const board_t *board, size_t n);
extern bool check(const board_t *board);
extern bool check_against_ref(const board_t* b);
extern void perft_test(board_t *board, const std::string& epd_filename);
#endif // DEBUG


extern void init_keys();

extern void reset(board_t *board);

extern void setup(board_t *board, const std::string& fen);

extern void print(const board_t *board, bool verbose = true);

std::string to_fen(const board_t *board);

extern void test(board_t *board);

extern uint64_t generate_pos_key(const board_t *board);

bool is_repetition(const board_t *board);

bool make_move(board_t *board, move_t move);

void undo_move(board_t *board, move_t move);
void undo_move(board_t *board); // Undo last move

/* Null move pruning */
void make_null(board_t *board);
void undo_null(board_t *board);

// Prints out the moves taken from the root of the search, helpful for debugging
inline void path_from_root(const board_t* board) {
    std::cout << "Moves since root (in reverse order): ";
    int count = 0;
    while (count <= board->history_ply) {
        std::cout << move_to_str(board->history[board->history_ply - count].move) << ' ';
        count++;
    }
    std::cout << std::endl;
}

inline bb_t all_pieces(const board_t *board) {
    return board->sides_pieces[BLACK] | board->sides_pieces[WHITE];
}

inline bb_t queens(const board_t *board) {
    return board->bitboards[q] | board->bitboards[Q];
}

inline bb_t bishops(const board_t *board) {
    return board->bitboards[b] | board->bitboards[B];
}

inline bb_t rooks(const board_t *board) {
    return board->bitboards[r] | board->bitboards[R];
}

inline bb_t knights(const board_t *board) {
    return board->bitboards[n] | board->bitboards[N];
}

inline bb_t pawns(const board_t *board) {
    return board->bitboards[p] | board->bitboards[P];
}

inline bb_t king_square_bb(const board_t* board, const int colour) {
    return colour ? board->bitboards[K] : board->bitboards[k];
}

inline square_t king_square(const board_t* board, const int colour) {
    if (king_square_bb(board, colour) == 0ULL) {
        print(board);
        path_from_root(board);
    }
    square_t sq = GETLSB(king_square_bb(board, colour));
    assert(square_ok(sq));
    return sq;
}

#endif // BOARD_H_
