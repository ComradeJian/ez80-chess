#include "board.h"

#include "fen.h"

#include <ctype.h>
#include <debug.h>
#include <string.h>

void board_init(board_t* board) {
    board_clear(board);
}

void board_clear(board_t* board) {
    memset(board, 0, sizeof(board_t));

    /* Initialize required non-zero defaults */
    board->en_passant_square = NO_SQUARE;
    board->fullmove_number = 1;
}

void board_reset(board_t* board) {
    board_clear(board);
    board_set_fen(board, INITIAL_FEN);
}

bool board_has_castling_rights(const board_t* board, castling_rights_t rights) {
    return (board->castling_rights & rights) == rights;
}

piece_t char_to_piece(char c) {
    piece_color_t color = islower(c) ? PIECE_BLACK : PIECE_WHITE;
    c = tolower(c);

    piece_type_t type;
    switch (c) {
        case 'p': type = PIECE_PAWN; break;
        case 'n': type = PIECE_KNIGHT; break;
        case 'b': type = PIECE_BISHOP; break;
        case 'r': type = PIECE_ROOK; break;
        case 'q': type = PIECE_QUEEN; break;
        case 'k': type = PIECE_KING; break;
        default: return PIECE_NONE;
    }

    return MAKE_PIECE(color, type);
}

char piece_to_char(piece_t piece) {
    char c;
    switch (GET_PIECE_TYPE(piece)) {
        case PIECE_PAWN: c = 'p'; break;
        case PIECE_KNIGHT: c = 'n'; break;
        case PIECE_BISHOP: c = 'b'; break;
        case PIECE_ROOK: c = 'r'; break;
        case PIECE_QUEEN: c = 'q'; break;
        case PIECE_KING: c = 'k'; break;
        default: return '.';
    }

    return IS_PIECE_COLOR(piece, PIECE_WHITE) ? toupper(c) : c;
}

void board_set_piece(board_t* board, square_t square, piece_t piece) {
    if (!is_valid_square(square)) {
        return;
    }

    board->squares[square] = piece;

    /* Update king position tracking if needed */
    if (IS_PIECE_TYPE(piece, PIECE_KING)) {
        board->king_square[COLOR_TO_SIDE(GET_PIECE_COLOR(piece))] = square;
    }
}

piece_t board_get_piece(const board_t* board, square_t square) {
    return is_valid_square(square) ? board->squares[square] : PIECE_NONE;
}

bool board_is_empty(const board_t* board, square_t square) {
    return board_get_piece(board, square) == PIECE_NONE;
}

#ifndef NDEBUG
void board_display(const board_t* board) {
    dbg_printf("\n  +---+---+---+---+---+---+---+---+\n");

    for (int8_t rank = BOARD_HEIGHT(BOARD_PHYSICAL) - 1; rank >= 0; --rank) {
        dbg_printf("%d |", rank + 1);

        for (uint8_t file = 0; file < BOARD_WIDTH(BOARD_PHYSICAL); ++file) {
            square_t square = FILE_RANK_TO_SQUARE(file, rank);
            piece_t piece = board_get_piece(board, square);
            dbg_printf(" %c |", piece_to_char(piece));
        }

        dbg_printf("\n  +---+---+---+---+---+---+---+---+\n");
    }

    dbg_printf("    a   b   c   d   e   f   g   h\n");

    /* Print additional state information */
    dbg_printf("\nSide to move: %s\n", board->side_to_move == SIDE_WHITE ? "White" : "Black");
    dbg_printf("Castling rights: %s%s%s%s\n", (board->castling_rights & CASTLE_WK) ? "K" : "",
               (board->castling_rights & CASTLE_WQ) ? "Q" : "",
               (board->castling_rights & CASTLE_BK) ? "k" : "",
               (board->castling_rights & CASTLE_BQ) ? "q" : "-");

    if (board->en_passant_square != NO_SQUARE) {
        dbg_printf("En passant square: %c%d\n", 'a' + SQUARE_TO_FILE(board->en_passant_square),
                   1 + SQUARE_TO_RANK(board->en_passant_square));
    }

    dbg_printf("Halfmove clock: %d\n", board->halfmove_clock);
    dbg_printf("Fullmove number: %d\n", board->fullmove_number);
}
#else
void board_display(const board_t* board __attribute__((unused))) {}
#endif