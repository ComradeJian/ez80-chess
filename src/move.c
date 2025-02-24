/**
 * @file move.c
 * @brief Implementation of chess move string conversion
 */

#include "move.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

// ==========================
//     Local Constants
// ==========================

/** File characters for algebraic notation (a-h) */
static const char FILE_CHARS[] = "abcdefgh";

/** Rank characters for algebraic notation (1-8) */
static const char RANK_CHARS[] = "12345678";

/** Promotion piece characters (N/B/R/Q) */
static const char PROMO_CHARS[] = "NBRQ";

// ==========================
//    Helper Functions
// ==========================

/**
 * @brief Convert file number to algebraic character
 * @param file File number (0-7)
 * @return char Algebraic file ('a'-'h') or '\0' if invalid
 */
static char file_to_char(uint8_t file) {
    return (file < 8) ? FILE_CHARS[file] : '\0';
}

/**
 * @brief Convert rank number to algebraic character
 * @param rank Rank number (0-7)
 * @return char Algebraic rank ('1'-'8') or '\0' if invalid
 */
static char rank_to_char(uint8_t rank) {
    return (rank < 8) ? RANK_CHARS[rank] : '\0';
}

/**
 * @brief Convert algebraic file character to number
 * @param c File character ('a'-'h')
 * @return int8_t File number (0-7) or -1 if invalid
 */
static int8_t char_to_file(char c) {
    if (c >= 'a' && c <= 'h') {
        return c - 'a';
    }
    return -1;
}

/**
 * @brief Convert algebraic rank character to number
 * @param c Rank character ('1'-'8')
 * @return int8_t Rank number (0-7) or -1 if invalid
 */
static int8_t char_to_rank(char c) {
    if (c >= '1' && c <= '8') {
        return c - '1';
    }
    return -1;
}

/**
 * @brief Get promotion piece character
 * @param type Piece type
 * @return char Piece character or '\0' if invalid
 */
static char promotion_to_char(piece_type_t type) {
    switch (type) {
        case PIECE_KNIGHT: return PROMO_CHARS[0];
        case PIECE_BISHOP: return PROMO_CHARS[1];
        case PIECE_ROOK: return PROMO_CHARS[2];
        case PIECE_QUEEN: return PROMO_CHARS[3];
        default: return '\0';
    }
}

/**
 * @brief Convert promotion character to piece type
 * @param c Promotion character (N/B/R/Q)
 * @return piece_type_t Piece type or PIECE_NONE if invalid
 */
static piece_type_t char_to_promotion(char c) {
    switch (toupper(c)) {
        case 'N': return PIECE_KNIGHT;
        case 'B': return PIECE_BISHOP;
        case 'R': return PIECE_ROOK;
        case 'Q': return PIECE_QUEEN;
        default: return PIECE_NONE;
    }
}

// ==========================
//     Public Functions
// ==========================

bool move_to_string(move_t move, char* str, size_t length) {
    /* Validate buffer size - need extra space if promotion */
    if (!move || !str || length < MOVE_STR_MIN_LEN ||
        (is_promotion(move) && length < MOVE_STR_MAX_LEN)) {
        return false;
    }

    /* Extract squares */
    square_t from = get_from_square(move);
    square_t to = get_to_square(move);
    if (!is_valid_square(from) || !is_valid_square(to)) {
        return false;
    }

    /* Convert from square */
    uint8_t from_file = square_to_file(from);
    uint8_t from_rank = square_to_rank(from);
    char ff = file_to_char(from_file);
    char fr = rank_to_char(from_rank);
    if (!ff || !fr) {
        return false;
    }

    /* Convert to square */
    uint8_t to_file = square_to_file(to);
    uint8_t to_rank = square_to_rank(to);
    char tf = file_to_char(to_file);
    char tr = rank_to_char(to_rank);
    if (!tf || !tr) {
        return false;
    }

    /* Build basic move string */
    str[0] = ff;
    str[1] = fr;
    str[2] = tf;
    str[3] = tr;

    /* Add promotion if present */
    piece_type_t promo = get_promotion_type(move);
    if (promo != PIECE_NONE) {
        char pc = promotion_to_char(promo);
        if (!pc) {
            return false;
        }
        str[4] = tolower(pc);  // Convention: use lowercase for promotion
        str[5] = '\0';
    } else {
        str[4] = '\0';
    }

    return true;
}

move_t string_to_move(const char* str, const board_t* board) {
    if (!str || !board) {
        return 0;
    }

    size_t len = strlen(str);
    if (len < MOVE_STR_MIN_LEN || len > MOVE_STR_MAX_LEN) {
        return 0;
    }

    int8_t from_file = char_to_file(str[0]);
    int8_t from_rank = char_to_rank(str[1]);
    int8_t to_file = char_to_file(str[2]);
    int8_t to_rank = char_to_rank(str[3]);
    if (from_file < 0 || from_rank < 0 || to_file < 0 || to_rank < 0) {
        return 0;
    }

    square_t from = square_from_file_rank(from_file, from_rank);
    square_t to = square_from_file_rank(to_file, to_rank);

    piece_t piece = board_get_piece(board, from);
    if (piece == PIECE_NONE) {
        return 0;
    }

    piece_type_t type = GET_PIECE_TYPE(piece);
    piece_color_t color = GET_PIECE_COLOR(piece);

    /* Start with basic move */
    move_t move = make_move(from, to);

    /* Check destination square for capture */
    piece_t capture = board_get_piece(board, to);
    piece_type_t capture_type = PIECE_NONE;
    if (capture != PIECE_NONE) {
        capture_type = GET_PIECE_TYPE(capture);
        move = make_capture(from, to, capture_type);
    }

    bool has_promotion = (len == MOVE_STR_MAX_LEN);
    if (has_promotion) {
        piece_type_t promo = char_to_promotion(str[4]);

        if (promo == PIECE_NONE) {
            return 0;
        }

        bool valid_white_promotion = (color == PIECE_WHITE && to_rank == 7);
        bool valid_black_promotion = (color == PIECE_BLACK && to_rank == 0);

        if (type != PIECE_PAWN || (!valid_white_promotion && !valid_black_promotion)) {
            return 0;
        }

        /* Create promotion move, preserving capture if present */
        if (capture_type != PIECE_NONE) {
            move = make_capture_promotion(from, to, capture_type, promo);
        } else {
            move = make_promotion(from, to, promo);
        }
    }

    if (type == PIECE_KING) {
        /* Check for castling */
        int8_t file_diff = to_file - from_file;
        if (abs(file_diff) == 2) {
            uint8_t special = (file_diff > 0) ? SPECIAL_CASTLE_KING : SPECIAL_CASTLE_QUEEN;
            move = make_special(from, to, special);
        }
    } else if (type == PIECE_PAWN) {
        /* Check for en passant */
        if (to == board->en_passant_square) {
            move = make_special(from, to, SPECIAL_EN_PASSANT);
        }
    }

    return move;
}