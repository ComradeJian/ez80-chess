/**
 * @file move.h
 * @brief Chess move representation and manipulation
 *
 * Implements a 24-bit move structure optimized for the eZ80's word size.
 * The structure efficiently encodes all necessary move information including
 * source/destination squares, piece types, and special move flags.
 *
 * Move bit layout (24 bits total):
 * [23-22] Special move type  (2 bits)
 * [21-19] Captured piece     (3 bits)
 * [18-16] Promotion piece    (3 bits)
 * [15-14] Move priority      (2 bits)
 * [13-07] To square         (7 bits)
 * [06-00] From square       (7 bits)
 */

#pragma once

#include "board.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ==========================
//     Type Definitions
// ==========================

/**
 * @brief 24-bit move representation
 *
 * Combines all move information in a single word optimized for eZ80.
 * @see move bit layout description above
 */
typedef uint24_t move_t;

/**
 * @brief Move priority levels for ordering
 */
typedef enum {
    PRIORITY_NORMAL,  /**< Basic moves */
    PRIORITY_KILLER,  /**< Historical good moves */
    PRIORITY_CAPTURE, /**< Capturing moves */
    PRIORITY_HASH     /**< Transposition table moves */
} move_priority_t;

/**
 * @brief Special move type enumeration
 */
typedef enum {
    SPECIAL_NONE,         /**< Not a special move */
    SPECIAL_EN_PASSANT,   /**< En passant capture */
    SPECIAL_CASTLE_KING,  /**< Kingside castle */
    SPECIAL_CASTLE_QUEEN, /**< Queenside castle */
    SPECIAL_COUNT         /**< Number of special move types */
} special_move_t;

// ==========================
//        Constants
// ==========================

/**
 * @brief Field position and size constants
 * @{
 */
#define MOVE_FROM_SHIFT     0
#define MOVE_FROM_BITS      7
#define MOVE_TO_SHIFT       7
#define MOVE_TO_BITS        7
#define MOVE_PRIORITY_SHIFT 14
#define MOVE_PRIORITY_BITS  2
#define MOVE_PROMOTE_SHIFT  16
#define MOVE_PROMOTE_BITS   3
#define MOVE_CAPTURE_SHIFT  19
#define MOVE_CAPTURE_BITS   3
#define MOVE_SPECIAL_SHIFT  22
#define MOVE_SPECIAL_BITS   2
/** @} */

/**
 * @brief Field masks for move manipulation
 * @{
 */
#define MOVE_FROM_MASK     ((1UL << MOVE_FROM_BITS) - 1)
#define MOVE_TO_MASK       ((1UL << MOVE_TO_BITS) - 1)
#define MOVE_PRIORITY_MASK ((1UL << MOVE_PRIORITY_BITS) - 1)
#define MOVE_PROMOTE_MASK  ((1UL << MOVE_PROMOTE_BITS) - 1)
#define MOVE_CAPTURE_MASK  ((1UL << MOVE_CAPTURE_BITS) - 1)
#define MOVE_SPECIAL_MASK  ((1UL << MOVE_SPECIAL_BITS) - 1)
/** @} */

/**
 * @brief Move string length and buffer size constants
 * @{
 *
 * Moves are encoded as coordinate strings:
 * - Basic moves: source + destination squares ("e2e4")
 * - Promotion moves: include promotion piece ("e7e8q")
 * Special moves (castling, en passant) use only coordinates.
 */

/** String length for basic moves ("e2e4" = 4 chars, excluding null) */
#define MOVE_STR_MIN_LEN 4

/** String length for promotion moves ("e7e8q" = 5 chars, excluding null) */
#define MOVE_STR_MAX_LEN 5

/** Buffer size for basic moves (includes null terminator) */
#define MOVE_STR_MIN_BUFFER (MOVE_STR_MIN_LEN + 1)

/** Buffer size for any move type (includes null terminator) */
#define MOVE_STR_MAX_BUFFER (MOVE_STR_MAX_LEN + 1)

/** Helper macro for determining required buffer size */
#define MOVE_STR_BUFFER_SIZE(allows_promotion)                                                     \
    ((allows_promotion) ? MOVE_STR_MAX_BUFFER : MOVE_STR_MIN_BUFFER)
/** @} */

// ==========================
//    Move Construction
// ==========================

/**
 * @brief Create a basic move from source to destination squares
 *
 * @param from Source square in 0x88 format
 * @param to   Destination square in 0x88 format
 * @return move_t Encoded move structure
 */
static inline move_t make_move(square_t from, square_t to) {
    return ((move_t)from << MOVE_FROM_SHIFT) | ((move_t)to << MOVE_TO_SHIFT);
}

/**
 * @brief Create a capturing move
 *
 * @param from     Source square
 * @param to       Destination square
 * @param captured Type of captured piece
 * @return move_t  Encoded move with capture info
 */
static inline move_t make_capture(square_t from, square_t to, piece_type_t captured) {
    return make_move(from, to) | ((move_t)captured << MOVE_CAPTURE_SHIFT) |
           ((move_t)PRIORITY_CAPTURE << MOVE_PRIORITY_SHIFT);
}

/**
 * @brief Create a pawn promotion move
 *
 * @param from      Source square
 * @param to        Destination square
 * @param promote   Piece type to promote to
 * @return move_t   Encoded move with promotion info
 */
static inline move_t make_promotion(square_t from, square_t to, piece_type_t promote) {
    return make_move(from, to) | ((move_t)promote << MOVE_PROMOTE_SHIFT);
}

/**
 * @brief Create a pawn promotion move
 *
 * @param from      Source square
 * @param to        Destination square
 * @param captured Type of captured piece
 * @param promote   Piece type to promote to
 * @return move_t   Encoded move with promotion info
 */
static inline move_t make_capture_promotion(square_t from, square_t to, piece_type_t captured,
                                            piece_type_t promote) {
    return make_move(from, to) | ((move_t)captured << MOVE_CAPTURE_SHIFT) |
           ((move_t)PRIORITY_CAPTURE << MOVE_PRIORITY_SHIFT) |
           ((move_t)promote << MOVE_PROMOTE_SHIFT);
}

/**
 * @brief Create a special move (castling or en passant)
 *
 * @param from     Source square
 * @param to       Destination square
 * @param special  Special move type
 * @return move_t  Encoded special move
 */
static inline move_t make_special(square_t from, square_t to, special_move_t special) {
    move_t move = make_move(from, to) | ((move_t)special << MOVE_SPECIAL_SHIFT);

    // En passant is always a pawn capture
    if (special == SPECIAL_EN_PASSANT) {
        move |= ((move_t)PIECE_PAWN << MOVE_CAPTURE_SHIFT) |
                ((move_t)PRIORITY_CAPTURE << MOVE_PRIORITY_SHIFT);
    }

    return move;
}

// ==========================
//     Move Information
// ==========================

/**
 * @brief Get source square from move
 * @param move  Encoded move
 * @return square_t Source square in 0x88 format
 */
static inline square_t get_from_square(move_t move) {
    return (square_t)((move >> MOVE_FROM_SHIFT) & MOVE_FROM_MASK);
}

/**
 * @brief Get destination square from move
 * @param move  Encoded move
 * @return square_t Destination square in 0x88 format
 */
static inline square_t get_to_square(move_t move) {
    return (square_t)((move >> MOVE_TO_SHIFT) & MOVE_TO_MASK);
}

/**
 * @brief Get captured piece type if any
 * @param move  Encoded move
 * @return piece_type_t Type of captured piece or PIECE_NONE
 */
static inline piece_type_t get_capture_type(move_t move) {
    return (piece_type_t)((move >> MOVE_CAPTURE_SHIFT) & MOVE_CAPTURE_MASK);
}

/**
 * @brief Get promotion piece type if any
 * @param move  Encoded move
 * @return piece_type_t Type of promotion piece or PIECE_NONE
 */
static inline piece_type_t get_promotion_type(move_t move) {
    return (piece_type_t)((move >> MOVE_PROMOTE_SHIFT) & MOVE_PROMOTE_MASK);
}

/**
 * @brief Get special move type if any
 * @param move  Encoded move
 * @return uint8_t Special move type or SPECIAL_NONE
 */
static inline uint8_t get_special_type(move_t move) {
    return (special_move_t)((move >> MOVE_SPECIAL_SHIFT) & MOVE_SPECIAL_MASK);
}

/**
 * @brief Get move priority level
 * @param move  Encoded move
 * @return move_priority_t Priority level for move ordering
 */
static inline move_priority_t get_priority(move_t move) {
    return (move_priority_t)((move >> MOVE_PRIORITY_SHIFT) & MOVE_PRIORITY_MASK);
}

/**
 * @brief Set move priority level
 * @param move      Encoded move
 * @param priority  New priority level
 * @return move_t   Updated move with new priority
 */
static inline move_t set_priority(move_t move, move_priority_t priority) {
    move &= ~((move_t)MOVE_PRIORITY_MASK << MOVE_PRIORITY_SHIFT);
    return move | ((move_t)priority << MOVE_PRIORITY_SHIFT);
}

// ==========================
//    Move Type Checking
// ==========================

/**
 * @brief Check if move is a capture
 * @param move  Encoded move
 * @return true if move captures a piece
 */
static inline bool is_capture(move_t move) {
    return get_capture_type(move) != PIECE_NONE;
}

/**
 * @brief Check if move is a promotion
 * @param move  Encoded move
 * @return true if move includes promotion
 */
static inline bool is_promotion(move_t move) {
    return get_promotion_type(move) != PIECE_NONE;
}

/**
 * @brief Check if move is a special move
 * @param move  Encoded move
 * @return true if move is castling or en passant
 */
static inline bool is_special(move_t move) {
    return get_special_type(move) != SPECIAL_NONE;
}

// ==========================
//    Move String Format
// ==========================

/**
 * @brief Convert move to algebraic notation string
 *
 * @param move   Move to convert
 * @param str    Buffer to receive string
 * @param length Buffer length
 * @return true if conversion succeeded
 *
 * Formats move as source-destination notation:
 * - Basic moves: "e2e4"
 * - Promotions: "e7e8q"
 * Buffer must be at least MOVE_STR_MAX_LEN bytes.
 */
bool move_to_string(move_t move, char* str, size_t length);

/**
 * @brief Parse move from algebraic notation
 *
 * @param str    Move string to parse
 * @param board  Current board position
 * @return move_t Parsed move or 0 if invalid
 *
 * Accepts source-destination notation:
 * - Basic moves: "e2e4"
 * - Promotions: "e7e8q"
 */
move_t string_to_move(const char* str, const board_t* board);

#ifdef __cplusplus
}
#endif