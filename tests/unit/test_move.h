/**
 * @file test_move.h
 * @brief Unit tests for chess move representation and manipulation
 *
 * Provides comprehensive test suites to verify the correctness of the 24-bit move
 * encoding system. Tests cover move construction, field manipulation, special move
 * handling, and string conversion for:
 * - Basic moves and captures
 * - Special moves (castling, en passant)
 * - Promotions and promotion captures
 * - Move priority management
 * - Move string parsing and generation
 */

#pragma once

#include "test_framework.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Test suite for move encoding and manipulation */
extern TestSuite MOVE_TESTS;

/**
 * @brief Execute all move-related unit tests
 *
 * Runs a comprehensive suite of tests covering move creation, manipulation,
 * conversion, complex scenarios and field interactions. Tests include:
 *
 * - Basic move construction and field access
 * - Capture move encoding and verification
 * - Promotion handling with all piece types
 * - Special move (castle/en passant) creation
 * - Move priority manipulation across types
 * - Edge cases for square validation
 * - Move string parsing and generation
 * - Combined move attributes (e.g. promotion captures)
 * - Move type inference and validation
 * - Board boundary move validation
 * - Invalid square handling
 * - All promotion piece combinations
 * - Special move interaction with captures
 * - Priority preservation across move types
 * - Move string edge cases and validation
 * - Combined attribute verification
 */
void run_move_tests(void);

#ifdef __cplusplus
}
#endif