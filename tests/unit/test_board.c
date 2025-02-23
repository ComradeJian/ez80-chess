#include "test_board.h"

#include "board.h"
#include "fen.h"

#include <stdint.h>
#include <string.h>

INIT_TEST_SUITE(BOARD_TESTS);
INIT_TEST_SUITE(FEN_TESTS);

void run_board_tests(void) {
    TEST_SUITE(BOARD_TESTS);

    TEST_CASE(BOARD_TESTS, "Board initialization") {
        board_t board;
        board_init(&board);

        /* Test empty board */
        for (square_t sq = 0; sq < BOARD_SIZE(BOARD_LOGICAL); ++sq) {
            ASSERT(BOARD_TESTS, board_is_empty(&board, board.squares[sq]));
        }

        /* Test initial state */
        ASSERT(BOARD_TESTS, board.side_to_move == SIDE_WHITE);
        ASSERT(BOARD_TESTS, board_has_castling_rights(&board, CASTLE_NONE));
        ASSERT(BOARD_TESTS, board.en_passant_square == NO_SQUARE);
        ASSERT(BOARD_TESTS, board.halfmove_clock == 0);
        ASSERT(BOARD_TESTS, board.fullmove_number == 1);
    }
    END_TEST_CASE(BOARD_TESTS);

    TEST_CASE(BOARD_TESTS, "Square validity") {
        /* Test valid squares */
        ASSERT(BOARD_TESTS, is_valid_square(FILE_RANK_TO_SQUARE(0, 0)));  // a1
        ASSERT(BOARD_TESTS, is_valid_square(FILE_RANK_TO_SQUARE(7, 7)));  // h8

        /* Test invalid squares */
        ASSERT(BOARD_TESTS, !is_valid_square(FILE_RANK_TO_SQUARE(8, 0)));
        ASSERT(BOARD_TESTS, !is_valid_square(FILE_RANK_TO_SQUARE(0, 8)));
    }
    END_TEST_CASE(BOARD_TESTS);

    TEST_CASE(BOARD_TESTS, "Piece conversion") {
        /* Test white pieces */
        ASSERT(BOARD_TESTS, char_to_piece('P') == MAKE_PIECE(PIECE_WHITE, PIECE_PAWN));
        ASSERT(BOARD_TESTS, char_to_piece('N') == MAKE_PIECE(PIECE_WHITE, PIECE_KNIGHT));
        ASSERT(BOARD_TESTS, char_to_piece('B') == MAKE_PIECE(PIECE_WHITE, PIECE_BISHOP));
        ASSERT(BOARD_TESTS, char_to_piece('R') == MAKE_PIECE(PIECE_WHITE, PIECE_ROOK));
        ASSERT(BOARD_TESTS, char_to_piece('Q') == MAKE_PIECE(PIECE_WHITE, PIECE_QUEEN));
        ASSERT(BOARD_TESTS, char_to_piece('K') == MAKE_PIECE(PIECE_WHITE, PIECE_KING));

        /* Test black pieces */
        ASSERT(BOARD_TESTS, char_to_piece('p') == MAKE_PIECE(PIECE_BLACK, PIECE_PAWN));
        ASSERT(BOARD_TESTS, char_to_piece('n') == MAKE_PIECE(PIECE_BLACK, PIECE_KNIGHT));
        ASSERT(BOARD_TESTS, char_to_piece('b') == MAKE_PIECE(PIECE_BLACK, PIECE_BISHOP));
        ASSERT(BOARD_TESTS, char_to_piece('r') == MAKE_PIECE(PIECE_BLACK, PIECE_ROOK));
        ASSERT(BOARD_TESTS, char_to_piece('q') == MAKE_PIECE(PIECE_BLACK, PIECE_QUEEN));
        ASSERT(BOARD_TESTS, char_to_piece('k') == MAKE_PIECE(PIECE_BLACK, PIECE_KING));

        /* Test invalid characters */
        ASSERT(BOARD_TESTS, char_to_piece('.') == PIECE_NONE);
        ASSERT(BOARD_TESTS, char_to_piece('x') == PIECE_NONE);
    }
    END_TEST_CASE(BOARD_TESTS);

    TEST_CASE(BOARD_TESTS, "Piece manipulation") {
        board_t board;
        board_init(&board);

        square_t e4 = FILE_RANK_TO_SQUARE(4, 3);  // e4 square
        piece_t white_pawn = MAKE_PIECE(PIECE_WHITE, PIECE_PAWN);

        /* Test setting pieces */
        board_set_piece(&board, e4, white_pawn);
        ASSERT(BOARD_TESTS, board_get_piece(&board, e4) == white_pawn);

        /* Test removing pieces */
        board_set_piece(&board, e4, PIECE_NONE);
        ASSERT(BOARD_TESTS, board_is_empty(&board, e4));

        /* Test king tracking */
        square_t e1 = FILE_RANK_TO_SQUARE(4, 0);
        piece_t white_king = MAKE_PIECE(PIECE_WHITE, PIECE_KING);
        board_set_piece(&board, e1, white_king);
        ASSERT(BOARD_TESTS, board.king_square[SIDE_WHITE] == e1);
    }
    END_TEST_CASE(BOARD_TESTS);

    print_test_results(&BOARD_TESTS);
}

void run_fen_tests(void) {
    TEST_SUITE(FEN_TESTS);

    TEST_CASE(FEN_TESTS, "Initial position parsing") {
        board_t board;
        board_init(&board);

        bool success = board_set_fen(&board, INITIAL_FEN);
        ASSERT(FEN_TESTS, success);

        /* Verify initial position */
        ASSERT(FEN_TESTS, board_get_piece(&board, FILE_RANK_TO_SQUARE(0, 0)) ==
                              MAKE_PIECE(PIECE_WHITE, PIECE_ROOK));
        ASSERT(FEN_TESTS, board_get_piece(&board, FILE_RANK_TO_SQUARE(4, 0)) ==
                              MAKE_PIECE(PIECE_WHITE, PIECE_KING));
        ASSERT(FEN_TESTS, board.side_to_move == SIDE_WHITE);
        ASSERT(FEN_TESTS, board_has_castling_rights(&board, CASTLE_ALL));
        ASSERT(FEN_TESTS, board.en_passant_square == NO_SQUARE);
        ASSERT(FEN_TESTS, board.halfmove_clock == 0);
        ASSERT(FEN_TESTS, board.fullmove_number == 1);
    }
    END_TEST_CASE(FEN_TESTS);

    TEST_CASE(FEN_TESTS, "FEN generation") {
        board_t board;
        board_init(&board);
        board_set_fen(&board, INITIAL_FEN);

        char fen[100];
        board_get_fen(&board, fen, sizeof(fen));
        ASSERT(FEN_TESTS, strcmp(fen, INITIAL_FEN) == 0);
    }
    END_TEST_CASE(FEN_TESTS);

    TEST_CASE(FEN_TESTS, "Complex position parsing") {
        board_t board;
        board_init(&board);

        /* Test position after 1. e4 e5 2. Nf3 */
        const char* fen = "rnbqkbnr/pppp1ppp/8/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2";
        bool success = board_set_fen(&board, fen);
        ASSERT(FEN_TESTS, success);

        char output_fen[100];
        board_get_fen(&board, output_fen, sizeof(output_fen));
        ASSERT(FEN_TESTS, strcmp(fen, output_fen) == 0);
    }
    END_TEST_CASE(FEN_TESTS);

    TEST_CASE(FEN_TESTS, "Invalid FEN handling") {
        board_t board;
        board_init(&board);

        /* Test invalid piece placement */
        ASSERT(FEN_TESTS,
               !board_set_fen(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNRR w KQkq - 0 1"));

        /* Test missing fields */
        ASSERT(FEN_TESTS,
               !board_set_fen(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -"));

        /* Test invalid active color */
        ASSERT(FEN_TESTS,
               !board_set_fen(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR x KQkq - 0 1"));

        /* Test invalid castling rights */
        ASSERT(FEN_TESTS,
               !board_set_fen(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w XYZq - 0 1"));
    }
    END_TEST_CASE(FEN_TESTS);

    TEST_CASE(FEN_TESTS, "En passant square handling") {
        board_t board;
        board_init(&board);

        /* Position after 1. e4 e5 2. e5 (double pawn push) */
        const char* fen = "rnbqkbnr/pppp1ppp/8/4P3/8/8/PPPP1PPP/RNBQKBNR b KQkq e6 0 2";
        bool success = board_set_fen(&board, fen);
        ASSERT(FEN_TESTS, success);

        /* Verify en passant square */
        ASSERT(FEN_TESTS, board.en_passant_square == FILE_RANK_TO_SQUARE(4, 5));  // e6

        char output_fen[100];
        board_get_fen(&board, output_fen, sizeof(output_fen));
        ASSERT(FEN_TESTS, strcmp(fen, output_fen) == 0);
    }
    END_TEST_CASE(FEN_TESTS);

    print_test_results(&FEN_TESTS);
}