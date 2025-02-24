/**
 * @file board.h
 * @brief Chess board representation and core manipulation functions
 *
 * Implements a chess board using the 0x88 board representation for efficient
 * move validation and piece tracking. The representation uses a 16x8 board
 * where the high bit (0x88) of invalid squares is set, allowing move validation
 * through simple bitwise operations.
 *
 * Key features:
 * - Efficient move validation using 0x88 representation
 * - Piece encoding that combines type and color in a single byte
 * - Complete game state tracking (castling rights, en passant, move counts)
 * - FEN string support for position importing/exporting
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ==========================
//     Type Definitions
// ==========================

/**
 * @brief Board type enumeration
 *
 * Used by dimension macros to handle different board representations.
 */
typedef enum {
    BOARD_PHYSICAL, /**< Standard 8x8 chess board */
    BOARD_LOGICAL   /**< 16x8 board for 0x88 representation */
} board_type_t;

/**
 * @brief Piece color enumeration (stored in high bit of piece)
 */
typedef enum {
    PIECE_WHITE = 0x00, /**< White pieces (bit 7 = 0) */
    PIECE_BLACK = 0x80  /**< Black pieces (bit 7 = 1) */
} piece_color_t;

/**
 * @brief Piece type enumeration (stored in low 3 bits of piece)
 *
 * Ordered by conventional piece value for move generation optimization.
 */
typedef enum {
    PIECE_NONE,   /**< Empty or invalid square (0) */
    PIECE_PAWN,   /**< Pawn (1) */
    PIECE_KNIGHT, /**< Knight (3) */
    PIECE_BISHOP, /**< Bishop (3) */
    PIECE_ROOK,   /**< Rook (5) */
    PIECE_QUEEN,  /**< Queen (9) */
    PIECE_KING,   /**< King (∞) */
    PIECE_COUNT   /**< Number of piece types (including NONE) */
} piece_type_t;

/**
 * @brief Side to move identifier
 *
 * Used for tracking active player and king positions.
 */
typedef enum {
    SIDE_WHITE, /**< White to move */
    SIDE_BLACK, /**< Black to move */
    SIDE_COUNT  /**< Number of sides */
} side_t;

/**
 * @brief Castling rights representation
 *
 * Stores castle availability as individual bit flags.
 * @see CASTLE_WK, CASTLE_WQ, CASTLE_BK, CASTLE_BQ for flag values
 */
typedef uint8_t castling_rights_t;

/**
 * @brief Move counter type
 *
 * Used for halfmove clock (50-move rule) and fullmove counter.
 */
typedef uint16_t move_count_t;

/**
 * @brief Square representation in 0x88 format
 *
 * Binary layout: 0bxrrr_xfff
 * - rrr: Rank (0-7) ==> bits 4-6
 * - fff: File (0-7) ==> bits 0-2
 * - x: Validity bit ==> bits 3,7 (forms 0x88 when invalid)
 */
typedef uint8_t square_t;

/**
 * @brief Piece representation combining color and type
 *
 * Binary layout: 0bc000_0ttt
 * - c: Color (0/1) ==> bit 7
 * - ttt: Type (0-6) ==> bits 0-2
 */
typedef uint8_t piece_t;

// ==========================
//        Constants
// ==========================

/**
 * @brief Board dimension definitions
 * @{
 */
#define BOARD_WIDTH(board)  ((board) == BOARD_PHYSICAL ? 8 : 16)
#define BOARD_HEIGHT(board) ((board) == BOARD_PHYSICAL ? 8 : 8)
#define BOARD_SIZE(board)   (BOARD_WIDTH(board) * BOARD_HEIGHT(board))
/** @} */

/**
 * @brief Square coordinate manipulation constants
 * @{
 */
#define BOARD_RANK_SHIFT 4    /**< 16 squares per rank (1 << 4) */
#define BOARD_FILE_MASK  0x07 /**< Mask for extracting file (0-7) */
#define BOARD_COLOR_BIT  0x80 /**< Bit for piece color (0x00 or 0x80) */
/** @} */

/**
 * @brief Piece manipulation masks
 * @{
 */
#define COLOR_MASK BOARD_COLOR_BIT /**< Mask for extracting piece color */
#define PIECE_MASK 0x07            /**< Mask for extracting piece type */
/** @} */

/**
 * @brief Castling rights bit flags
 * @{
 */
#define CASTLE_NONE 0x00 /**< No castling rights */
#define CASTLE_WK   0x01 /**< White kingside castle */
#define CASTLE_WQ   0x02 /**< White queenside castle */
#define CASTLE_BK   0x04 /**< Black kingside castle */
#define CASTLE_BQ   0x08 /**< Black queenside castle */
#define CASTLE_ALL  0x0F /**< All castling rights */
/** @} */

/**
 * @brief Square and length constants
 * @{
 */
#define NO_SQUARE      ((square_t)(-1)) /**< Invalid square marker */
#define SQUARE_STR_LEN 2                /**< Length of algebraic square (e.g. e2) */
/** @} */

// ==========================
//    Internal Macros
// ==========================

/**
 * @brief Internal macros for efficient board operations
 * @{
 */
#define SQUARE_TO_FILE(square)          ((square) & BOARD_FILE_MASK)
#define SQUARE_TO_RANK(square)          ((square) >> BOARD_RANK_SHIFT)
#define FILE_RANK_TO_SQUARE(file, rank) (((rank) << BOARD_RANK_SHIFT) | (file))

#define MAKE_PIECE(color, type)      ((color) | (type))
#define GET_PIECE_TYPE(piece)        ((piece) & PIECE_MASK)
#define IS_PIECE_TYPE(piece, type)   (GET_PIECE_TYPE(piece) == (type))
#define GET_PIECE_COLOR(piece)       ((piece) & COLOR_MASK)
#define IS_PIECE_COLOR(piece, color) (GET_PIECE_COLOR(piece) == (color))

#define COLOR_TO_SIDE(color) ((color) == PIECE_WHITE ? SIDE_WHITE : SIDE_BLACK)
#define SIDE_TO_COLOR(side)  ((side) == SIDE_WHITE ? PIECE_WHITE : PIECE_BLACK)

#define INVALID_SQUARE(sq) ((sq) & 0x88)
/** @} */

// ==========================
//         Data Types
// ==========================

/**
 * @brief Complete chess board state representation
 *
 * Tracks all information needed for chess rule implementation
 * and position evaluation using the 0x88 board representation.
 */
typedef struct {
    piece_t squares[BOARD_SIZE(BOARD_LOGICAL)]; /**< 0x88 board array */
    side_t side_to_move;                        /**< Active player */
    square_t king_square[SIDE_COUNT];           /**< King position tracking */
    castling_rights_t castling_rights;          /**< Available castling moves */
    square_t en_passant_square;                 /**< Valid en passant target or NO_SQUARE */
    move_count_t halfmove_clock;                /**< Moves since pawn move or capture */
    move_count_t fullmove_number;               /**< Complete game moves */
} board_t;

// ==========================
//    Board Initialization
// ==========================

/**
 * @brief Initialize an empty chess board
 * @param board Board to initialize
 *
 * Creates an empty board with default values for all fields.
 * All squares are set to PIECE_NONE and move counters are reset.
 */
void board_init(board_t* board);

/**
 * @brief Clear all pieces and reset game state
 * @param board Board to clear
 *
 * Removes all pieces and resets game state to defaults:
 * - All squares set to PIECE_NONE
 * - No castling rights
 * - No en passant square
 * - Move counters reset
 */
void board_clear(board_t* board);

/**
 * @brief Set up the standard chess starting position
 * @param board Board to set up
 *
 * Clears the board and sets up the standard chess initial position.
 * Also initializes all game state (castling rights, move counters, etc.).
 * @see INITIAL_FEN in fen.h
 */
void board_reset(board_t* board);

// ========================
//     Board Utilities
// ========================

/**
 * @brief Check if a square is valid on the 0x88 board
 * @param square Square to validate
 * @return true if square is valid, false if not
 */
static inline bool is_valid_square(square_t square) {
    return !INVALID_SQUARE(square);
}

/**
 * @brief Check if specific castling rights are available
 * @param board Board to query
 * @param rights Castling rights to check (can combine with bitwise OR)
 * @return true if all specified rights are available, false otherwise
 *
 * Example usage:
 * @code
 * // Check if white can castle kingside
 * if (board_has_castling_rights(board, CASTLE_WK)) { ... }
 *
 * // Check if black can castle either side
 * if (board_has_castling_rights(board, CASTLE_BK | CASTLE_BQ)) { ... }
 * @endcode
 */
bool board_has_castling_rights(const board_t* board, castling_rights_t rights);

/**
 * @brief Convert an ASCII piece character to internal piece representation
 * @param c ASCII character ('K', 'n', etc.)
 * @return Piece value with color and type, or PIECE_NONE if invalid
 *
 * @note Case-sensitive: 'P' is white pawn, 'p' is black pawn.
 */
piece_t char_to_piece(char c);

/**
 * @brief Convert an internal piece to ASCII character representation
 * @param piece Piece value with color and type
 * @return ASCII character ('K', 'n', etc.), or '.' if empty/invalid
 *
 * @note Returns lowercase for black pieces, uppercase for white pieces.
 */
char piece_to_char(piece_t piece);

// ==========================
//    Square Manipulation
// ==========================

/**
 * @brief Convert file and rank to square index
 * @param file File (0-7)
 * @param rank Rank (0-7)
 * @return square_t Square in 0x88 format
 */
static inline square_t square_from_file_rank(uint8_t file, uint8_t rank) {
    return FILE_RANK_TO_SQUARE(file, rank);
}

/**
 * @brief Get file number from square
 * @param square Square in 0x88 format
 * @return uint8_t File (0-7)
 */
static inline uint8_t square_to_file(square_t square) {
    return SQUARE_TO_FILE(square);
}

/**
 * @brief Get rank number from square
 * @param square Square in 0x88 format
 * @return uint8_t Rank (0-7)
 */
static inline uint8_t square_to_rank(square_t square) {
    return SQUARE_TO_RANK(square);
}

// ========================
//    Piece Manipulation
// ========================

/**
 * @brief Place or remove a piece on the board
 * @param board Board to modify
 * @param square Target square (invalid squares are silently ignored)
 * @param piece Piece to place, or PIECE_NONE to clear the square
 *
 * Places a piece on the board and updates king tracking if needed.
 * Setting PIECE_NONE effectively removes any piece at that square.
 */
void board_set_piece(board_t* board, square_t square, piece_t piece);

/**
 * @brief Get the piece at a specific square
 * @param board Board to query
 * @param square Target square
 * @return Piece at the square, or PIECE_NONE if empty or invalid
 */
piece_t board_get_piece(const board_t* board, square_t square);

/**
 * @brief Check if a square is empty
 * @param board Board to query
 * @param square Square to check
 * @return true if square is empty or invalid, false if occupied
 */
bool board_is_empty(const board_t* board, square_t square);

/**
 * @brief Create a piece with specified color and type
 * @param color Piece color
 * @param type Piece type
 * @return piece_t Combined piece representation
 */
static inline piece_t create_piece(piece_color_t color, piece_type_t type) {
    return MAKE_PIECE(color, type);
}

/**
 * @brief Get the type of a piece
 * @param piece Piece to examine
 * @return piece_type_t Type of the piece
 */
static inline piece_type_t get_piece_type(piece_t piece) {
    return GET_PIECE_TYPE(piece);
}

/**
 * @brief Get the color of a piece
 * @param piece Piece to examine
 * @return piece_color_t Color of the piece
 */
static inline piece_color_t get_piece_color(piece_t piece) {
    return GET_PIECE_COLOR(piece);
}

/**
 * @brief Convert piece color to side
 * @param color Piece color to convert
 * @return side_t Corresponding side
 */
static inline side_t color_to_side(piece_color_t color) {
    return COLOR_TO_SIDE(color);
}

/**
 * @brief Convert side to piece color
 * @param side Side to convert
 * @return piece_color_t Corresponding piece color
 */
static inline piece_color_t side_to_color(side_t side) {
    return SIDE_TO_COLOR(side);
}

// ========================
//      Board Display
// ========================

/**
 * @brief Display the current board state
 * @param board Board to display
 *
 * Prints an ASCII representation of the board with current game state
 * to the debug console. Only available in debug builds.
 */
void board_display(const board_t* board);

#ifdef __cplusplus
}
#endif