/**
 * @file fen.h
 * @brief Forsyth-Edwards Notation (FEN) parsing and generation
 *
 * Provides functionality to import and export chess positions using FEN
 * strings. FEN is a standard notation that encodes a complete chess position
 * including:
 * - Piece positions
 * - Active color
 * - Castling rights
 * - En passant targets
 * - Move counters
 *
 * Example FEN: "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
 */

#ifndef FEN_H
#define FEN_H

#include "board.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Minimum length needed for a valid FEN string buffer */
#define FEN_MIN_LEN 24

/** Standard chess starting position in FEN notation */
#define INITIAL_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

/**
 * @brief Import a chess position from FEN string
 *
 * @param board Target board to receive the position
 * @param fen   FEN string to parse
 * @return true if parsing succeeded, false if FEN was invalid
 *
 * Parses a FEN string and sets up the corresponding position on the board. The
 * board is cleared before parsing begins. If parsing fails, the board remains
 * in a cleared state.
 */
bool board_set_fen(board_t* board, const char* fen);

/**
 * @brief Export current board position to FEN string
 * @param board Target board to generate FEN from
 * @param fen   Buffer to receive the FEN string
 * @param len   Length of the provided buffer
 *
 * Generates a FEN string representing the current board position. If the buffer
 * is too small (less than FEN_MIN_LEN), an empty string is written.
 */
void board_get_fen(const board_t* board, char* fen, size_t len);

#ifdef __cplusplus
}
#endif

#endif  // FEN_H