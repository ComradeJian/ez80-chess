#include "fen.h"

#include "board.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief FEN parser state tracking
 *
 * Maintains position and success state during parsing. Once an error occurs,
 * success is set to false and all subsequent operations are skipped.
 */
typedef struct {
    const char* ptr; /**< Current position in input string */
    bool success;    /**< Parsing success state */
} parser_t;

/**
 * @brief FEN writer state tracking
 *
 * Manages buffer position and remaining space during FEN generation.
 * Tracks success to handle buffer overflow gracefully.
 */
typedef struct {
    char* ptr;        /**< Current position in output buffer */
    bool success;     /**< Writing success state */
    size_t remaining; /**< Remaining space in buffer */
} writer_t;

/* Parser utility functions */
/**
 * @brief Create a new FEN parser state
 * @param fen Input FEN string to parse
 * @return Initialized parser state
 */
static parser_t parser_create(const char* fen);

/**
 * @brief Expect and consume a specific character
 * @param parser Parser state
 * @param expected Character to expect
 * @return true if character matched and consumed
 */
static bool parser_expect(parser_t* parser, char expected);

/**
 * @brief Expect and consume a space character
 * @param parser Parser state
 * @return true if space was found and consumed
 */
static bool parser_expect_space(parser_t* parser);

/* Writer utility functions */
/**
 * @brief Create a new FEN writer state
 * @param buffer Output buffer
 * @param len Buffer length
 * @return Initialized writer state
 */
static writer_t writer_create(char* buffer, size_t len);

/**
 * @brief Write a single character to the output buffer
 * @param writer Writer state
 * @param c Character to write
 */
static void writer_write_char(writer_t* writer, char c);

/**
 * @brief Write a space character to the output buffer
 * @param writer Writer state
 */
static void writer_write_space(writer_t* writer);

/**
 * @brief Parse piece placement section of FEN
 * @param board Target board
 * @param parser Parser state
 */
static void parse_piece_placement(board_t* board, parser_t* parser);

/**
 * @brief Parse active color section of FEN
 * @param board Target board
 * @param parser Parser state
 */
static void parse_side_to_move(board_t* board, parser_t* parser);

/**
 * @brief Parse castling availability section of FEN
 * @param board Target board
 * @param parser Parser state
 */
static void parse_castling_rights(board_t* board, parser_t* parser);

/**
 * @brief Parse en passant target square section of FEN
 * @param board Target board
 * @param parser Parser state
 */
static void parse_en_passant(board_t* board, parser_t* parser);

/**
 * @brief Parse halfmove clock section of FEN
 * @param board Target board
 * @param parser Parser state
 */
static void parse_halfmove_clock(board_t* board, parser_t* parser);

/**
 * @brief Parse fullmove number section of FEN
 * @param board Target board
 * @param parser Parser state
 */
static void parse_fullmove_number(board_t* board, parser_t* parser);

/**
 * @brief Write piece placement section of FEN
 * @param board Source board
 * @param writer Writer state
 */
static void write_piece_placement(const board_t* board, writer_t* writer);

/**
 * @brief Write active color section of FEN
 * @param board Source board
 * @param writer Writer state
 */
static void write_side_to_move(const board_t* board, writer_t* writer);

/**
 * @brief Write castling availability section of FEN
 * @param board Source board
 * @param writer Writer state
 */
static void write_castling_rights(const board_t* board, writer_t* writer);

/**
 * @brief Write en passant target square section of FEN
 * @param board Source board
 * @param writer Writer state
 */
static void write_en_passant(const board_t* board, writer_t* writer);

/**
 * @brief Write move counters section of FEN
 * @param board Source board
 * @param writer Writer state
 */
static void write_move_counters(const board_t* board, writer_t* writer);

bool board_set_fen(board_t* board, const char* fen) {
    if (!fen) {
        return false;
    }
    board_clear(board);

    parser_t parser = parser_create(fen);

    /* Example FEN String:
     * - rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQk e6 0 2
     *
     * Breakdown:
     * - piece_placement    | rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR
     * - side_to_move       | w
     * - castling_rights    | KQk
     * - en_passant         | e6
     * - halfmove_clock     | 0
     * - fullmove_number    | 2
     *
     * Valid arguments must be separated by a space ==> parser_expect_space()
     */

    parse_piece_placement(board, &parser);
    parser_expect_space(&parser);

    parse_side_to_move(board, &parser);
    parser_expect_space(&parser);

    parse_castling_rights(board, &parser);
    parser_expect_space(&parser);

    parse_en_passant(board, &parser);
    parser_expect_space(&parser);

    parse_halfmove_clock(board, &parser);
    parser_expect_space(&parser);

    parse_fullmove_number(board, &parser);

    return parser.success;
}

void board_get_fen(const board_t* board, char* fen, size_t len) {
    if (!fen || len < FEN_MIN_LEN) {
        return;
    }

    writer_t writer = writer_create(fen, len);

    write_piece_placement(board, &writer);
    writer_write_space(&writer);

    write_side_to_move(board, &writer);
    writer_write_space(&writer);

    write_castling_rights(board, &writer);
    writer_write_space(&writer);

    write_en_passant(board, &writer);
    writer_write_space(&writer);

    write_move_counters(board, &writer);

    if (writer.success && writer.remaining > 0) {
        *writer.ptr = '\0';  // Null terminate the string
    } else {
        fen[0] = '\0';  // Error occurred, return empty string
    }
}

static parser_t parser_create(const char* fen) {
    return (parser_t){.ptr = fen, .success = true};
}

static bool parser_expect(parser_t* parser, char expected) {
    if (!parser->success || !*parser->ptr || *parser->ptr != expected) {
        parser->success = false;
        return false;
    }
    parser->ptr++;
    return true;
}

static bool parser_expect_space(parser_t* parser) {
    return parser_expect(parser, ' ');
}

static writer_t writer_create(char* buffer, size_t len) {
    return (writer_t){.ptr = buffer, .success = true, .remaining = len};
}

static void writer_write_char(writer_t* writer, char c) {
    if (!writer->success || writer->remaining <= 1) {  // Need space for char + null terminator
        writer->success = false;
        return;
    }
    *writer->ptr++ = c;
    writer->remaining--;
}

static void writer_write_space(writer_t* writer) {
    writer_write_char(writer, ' ');
}

static void parse_piece_placement(board_t* board, parser_t* parser) {
    if (!parser->success) {
        return;
    }

    int8_t rank = BOARD_HEIGHT(BOARD_PHYSICAL) - 1;
    uint8_t file = 0;

    while (parser->success && *parser->ptr && *parser->ptr != ' ') {
        char c = *parser->ptr;

        if (c == '/') {
            if (file != BOARD_WIDTH(BOARD_PHYSICAL)) {
                parser->success = false;
                return;
            }
            rank--;
            file = 0;
            parser->ptr++;
            continue;
        }

        if (c >= '1' && c <= '0' + BOARD_WIDTH(BOARD_PHYSICAL)) {
            file += c - '0';
            parser->ptr++;
            continue;
        }

        if (file >= BOARD_WIDTH(BOARD_PHYSICAL) || rank < 0) {
            parser->success = false;
            return;
        }

        uint8_t piece = char_to_piece(c);
        if (piece == PIECE_NONE && c != ' ') {
            parser->success = false;
            return;
        }

        board_set_piece(board, FILE_RANK_TO_SQUARE(file, rank), piece);
        file++;
        parser->ptr++;
    }

    parser->success = (rank == 0 && file == BOARD_WIDTH(BOARD_PHYSICAL));
}

static void parse_side_to_move(board_t* board, parser_t* parser) {
    if (!parser->success) {
        return;
    }

    char c = *parser->ptr;
    if (c == 'w') {
        board->side_to_move = SIDE_WHITE;
    } else if (c == 'b') {
        board->side_to_move = SIDE_BLACK;
    } else {
        parser->success = false;
        return;
    }
    parser->ptr++;
}

static void parse_castling_rights(board_t* board, parser_t* parser) {
    if (!parser->success) {
        return;
    }

    if (*parser->ptr == '-') {
        parser->ptr++;
        return;
    }

    while (parser->success && *parser->ptr && *parser->ptr != ' ') {
        switch (*parser->ptr) {
            case 'K': board->castling_rights |= CASTLE_WK; break;
            case 'Q': board->castling_rights |= CASTLE_WQ; break;
            case 'k': board->castling_rights |= CASTLE_BK; break;
            case 'q': board->castling_rights |= CASTLE_BQ; break;
            default: parser->success = false; return;
        }
        parser->ptr++;
    }
}

static void parse_en_passant(board_t* board, parser_t* parser) {
    if (!parser->success) {
        return;
    }

    if (*parser->ptr == '-') {
        board->en_passant_square = NO_SQUARE;
        parser->ptr++;
        return;
    }

    /* Need two characters for file and rank */
    if (!parser->ptr[0] || !parser->ptr[1]) {
        parser->success = false;
        return;
    }

    uint8_t file = parser->ptr[0] - 'a';
    uint8_t rank = parser->ptr[1] - '1';

    if (file >= BOARD_WIDTH(BOARD_PHYSICAL) || rank >= BOARD_HEIGHT(BOARD_PHYSICAL)) {
        parser->success = false;
        return;
    }

    board->en_passant_square = FILE_RANK_TO_SQUARE(file, rank);
    parser->ptr += SQUARE_STR_LEN;
}

static void parse_halfmove_clock(board_t* board, parser_t* parser) {
    if (!parser->success) {
        return;
    }

    char* end;
    board->halfmove_clock = strtol(parser->ptr, &end, 10);

    if (end == parser->ptr) {
        parser->success = false;
        return;
    }

    parser->ptr = end;
}

static void parse_fullmove_number(board_t* board, parser_t* parser) {
    if (!parser->success) {
        return;
    }

    char* end;
    board->fullmove_number = strtol(parser->ptr, &end, 10);

    if (end == parser->ptr) {
        parser->success = false;
        return;
    }

    parser->ptr = end;
}

static void write_piece_placement(const board_t* board, writer_t* writer) {
    if (!writer->success) {
        return;
    }
    for (int8_t rank = BOARD_HEIGHT(BOARD_PHYSICAL) - 1; rank >= 0; --rank) {
        uint8_t empty = 0;
        for (uint8_t file = 0; file < BOARD_WIDTH(BOARD_PHYSICAL); ++file) {
            piece_t piece = board_get_piece(board, FILE_RANK_TO_SQUARE(file, rank));

            if (piece == PIECE_NONE) {
                empty++;
                continue;
            }

            if (empty) {
                writer_write_char(writer, '0' + empty);
                empty = 0;
            }
            writer_write_char(writer, piece_to_char(piece));
        }

        if (empty) {
            writer_write_char(writer, '0' + empty);
        }
        if (rank > 0) {
            writer_write_char(writer, '/');
        }
    }
}

static void write_side_to_move(const board_t* board, writer_t* writer) {
    if (!writer->success) {
        return;
    }
    writer_write_char(writer, board->side_to_move == SIDE_WHITE ? 'w' : 'b');
}

static void write_castling_rights(const board_t* board, writer_t* writer) {
    if (!writer->success) {
        return;
    }
    uint8_t rights = board->castling_rights;

    if (rights == 0) {
        writer_write_char(writer, '-');
        return;
    }

    if (rights & CASTLE_WK) {
        writer_write_char(writer, 'K');
    }
    if (rights & CASTLE_WQ) {
        writer_write_char(writer, 'Q');
    }
    if (rights & CASTLE_BK) {
        writer_write_char(writer, 'k');
    }
    if (rights & CASTLE_BQ) {
        writer_write_char(writer, 'q');
    }
}

static void write_en_passant(const board_t* board, writer_t* writer) {
    if (!writer->success) {
        return;
    }
    square_t en_passant = board->en_passant_square;

    if (en_passant == NO_SQUARE) {
        writer_write_char(writer, '-');
        return;
    }

    writer_write_char(writer, 'a' + SQUARE_TO_FILE(en_passant));
    writer_write_char(writer, '1' + SQUARE_TO_RANK(en_passant));
}

static void write_move_counters(const board_t* board, writer_t* writer) {
    if (!writer->success || writer->remaining <= 16) {  // Conservative space check
        writer->success = false;
        return;
    }

    int written = snprintf(writer->ptr, writer->remaining, "%d %d", board->halfmove_clock,
                           board->fullmove_number);

    if (written < 0 || (size_t)written >= writer->remaining) {
        writer->success = false;
        return;
    }

    writer->ptr += written;
    writer->remaining -= written;
}