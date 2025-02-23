/**
 * @file board_tests.h
 * @brief Unit tests for chess board representation and FEN parsing
 *
 * Provides comprehensive test suites to verify the correctness of the chess board
 * implementation and Forsyth-Edwards Notation (FEN) parsing/generation. Tests cover:
 * - Board initialization and state management
 * - Piece placement and manipulation
 * - Square coordinate conversion
 * - FEN string parsing and validation
 * - Edge cases and error handling
 */

#ifndef BOARD_TESTS_H
#define BOARD_TESTS_H

#include "test_framework.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Test suite for board functionality */
extern TestSuite BOARD_TESTS;

/** Test suite for FEN parsing and generation */
extern TestSuite FEN_TESTS;

/**
 * @brief Execute all board-related unit tests
 *
 * Runs a comprehensive suite of tests covering board initialization,
 * piece manipulation, and square coordinate handling. Tests include:
 *
 * - Board initialization to empty state
 * - Square validity checking for 0x88 board
 * - Piece conversion between ASCII and internal representation
 * - Setting and getting pieces on the board
 * - King position tracking
 */
void run_board_tests(void);

/**
 * @brief Execute all FEN-related unit tests
 *
 * Runs tests to verify correct parsing and generation of FEN strings.
 * Test cases include:
 *
 * - Parsing standard starting position
 * - Generating FEN strings from board state
 * - Handling complex positions with en passant
 * - Validating incorrect/malformed FEN strings
 * - Preserving special state (castling rights, move counts)
 */
void run_fen_tests(void);

#ifdef __cplusplus
}
#endif

#endif  // BOARD_TESTS_H