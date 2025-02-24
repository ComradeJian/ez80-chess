#include "test_move.h"

#include "board.h"
#include "move.h"

#include <ctype.h>
#include <string.h>

INIT_TEST_SUITE(MOVE_TESTS);

void run_move_tests(void) {
    TEST_SUITE(MOVE_TESTS);

    TEST_CASE(MOVE_TESTS, "Basic move construction") {
        /* Create a simple e2-e4 move */
        square_t from = square_from_file_rank(4, 1);  // e2
        square_t to = square_from_file_rank(4, 3);    // e4
        move_t move = make_move(from, to);

        /* Verify fields */
        ASSERT(MOVE_TESTS, get_from_square(move) == from);
        ASSERT(MOVE_TESTS, get_to_square(move) == to);
        ASSERT(MOVE_TESTS, get_capture_type(move) == PIECE_NONE);
        ASSERT(MOVE_TESTS, get_promotion_type(move) == PIECE_NONE);
        ASSERT(MOVE_TESTS, get_special_type(move) == SPECIAL_NONE);
        ASSERT(MOVE_TESTS, get_priority(move) == PRIORITY_NORMAL);
    }
    END_TEST_CASE(MOVE_TESTS);

    TEST_CASE(MOVE_TESTS, "Capture move construction") {
        /* Create a capturing move (e4xd5) */
        square_t from = square_from_file_rank(4, 3);  // e4
        square_t to = square_from_file_rank(3, 4);    // d5
        move_t move = make_capture(from, to, PIECE_PAWN);

        /* Verify capture information */
        ASSERT(MOVE_TESTS, get_from_square(move) == from);
        ASSERT(MOVE_TESTS, get_to_square(move) == to);
        ASSERT(MOVE_TESTS, get_capture_type(move) == PIECE_PAWN);
        ASSERT(MOVE_TESTS, is_capture(move));
        ASSERT(MOVE_TESTS, get_priority(move) == PRIORITY_CAPTURE);
    }
    END_TEST_CASE(MOVE_TESTS);

    TEST_CASE(MOVE_TESTS, "Promotion move construction") {
        /* Create a promotion move (e7-e8=Q) */
        square_t from = square_from_file_rank(4, 6);  // e7
        square_t to = square_from_file_rank(4, 7);    // e8
        move_t move = make_promotion(from, to, PIECE_QUEEN);

        /* Verify promotion information */
        ASSERT(MOVE_TESTS, get_from_square(move) == from);
        ASSERT(MOVE_TESTS, get_to_square(move) == to);
        ASSERT(MOVE_TESTS, get_promotion_type(move) == PIECE_QUEEN);
        ASSERT(MOVE_TESTS, is_promotion(move));
    }
    END_TEST_CASE(MOVE_TESTS);

    TEST_CASE(MOVE_TESTS, "Special move construction") {
        /* Test castling move (e1-g1 kingside castle) */
        square_t from = square_from_file_rank(4, 0);  // e1
        square_t to = square_from_file_rank(6, 0);    // g1
        move_t castle = make_special(from, to, SPECIAL_CASTLE_KING);

        /* Verify castling information */
        ASSERT(MOVE_TESTS, get_from_square(castle) == from);
        ASSERT(MOVE_TESTS, get_to_square(castle) == to);
        ASSERT(MOVE_TESTS, get_special_type(castle) == SPECIAL_CASTLE_KING);
        ASSERT(MOVE_TESTS, is_special(castle));

        /* Test en passant move */
        square_t ep_from = square_from_file_rank(4, 4);  // e5
        square_t ep_to = square_from_file_rank(3, 5);    // d6
        move_t ep = make_special(ep_from, ep_to, SPECIAL_EN_PASSANT);

        /* Verify en passant information */
        ASSERT(MOVE_TESTS, get_from_square(ep) == ep_from);
        ASSERT(MOVE_TESTS, get_to_square(ep) == ep_to);
        ASSERT(MOVE_TESTS, get_special_type(ep) == SPECIAL_EN_PASSANT);
        ASSERT(MOVE_TESTS, is_special(ep));
    }
    END_TEST_CASE(MOVE_TESTS);

    TEST_CASE(MOVE_TESTS, "Move priority manipulation") {
        /* Create a move and test priority setting */
        square_t from = square_from_file_rank(4, 1);  // e2
        square_t to = square_from_file_rank(4, 3);    // e4
        move_t move = make_move(from, to);

        /* Test priority setting and getting */
        move = set_priority(move, PRIORITY_KILLER);
        ASSERT(MOVE_TESTS, get_priority(move) == PRIORITY_KILLER);

        move = set_priority(move, PRIORITY_HASH);
        ASSERT(MOVE_TESTS, get_priority(move) == PRIORITY_HASH);

        /* Verify other fields remain unchanged */
        ASSERT(MOVE_TESTS, get_from_square(move) == from);
        ASSERT(MOVE_TESTS, get_to_square(move) == to);
        ASSERT(MOVE_TESTS, get_capture_type(move) == PIECE_NONE);
        ASSERT(MOVE_TESTS, get_promotion_type(move) == PIECE_NONE);
        ASSERT(MOVE_TESTS, get_special_type(move) == SPECIAL_NONE);
    }
    END_TEST_CASE(MOVE_TESTS);

    TEST_CASE(MOVE_TESTS, "Combined move attributes") {
        /* Create a capturing promotion (e7xf8=Q) */
        square_t from = square_from_file_rank(4, 6);  // e7
        square_t to = square_from_file_rank(5, 7);    // f8
        move_t move = make_promotion(from, to, PIECE_QUEEN);

        /* Add capture information */
        move |= ((move_t)PIECE_BISHOP << MOVE_CAPTURE_SHIFT);
        move |= ((move_t)PRIORITY_CAPTURE << MOVE_PRIORITY_SHIFT);

        /* Verify all fields */
        ASSERT(MOVE_TESTS, get_from_square(move) == from);
        ASSERT(MOVE_TESTS, get_to_square(move) == to);
        ASSERT(MOVE_TESTS, get_promotion_type(move) == PIECE_QUEEN);
        ASSERT(MOVE_TESTS, get_capture_type(move) == PIECE_BISHOP);
        ASSERT(MOVE_TESTS, get_priority(move) == PRIORITY_CAPTURE);
        ASSERT(MOVE_TESTS, is_capture(move));
        ASSERT(MOVE_TESTS, is_promotion(move));
        ASSERT(MOVE_TESTS, !is_special(move));
    }
    END_TEST_CASE(MOVE_TESTS);

    TEST_CASE(MOVE_TESTS, "Move to string conversion") {
        /* Test basic move */
        move_t basic = make_move(square_from_file_rank(4, 1),  // e2
                                 square_from_file_rank(4, 3)   // e4
        );
        char str[MOVE_STR_MAX_BUFFER];
        ASSERT(MOVE_TESTS, move_to_string(basic, str, sizeof(str)));
        ASSERT(MOVE_TESTS, strcmp(str, "e2e4") == 0);

        /* Test promotion move */
        move_t promotion = make_promotion(square_from_file_rank(4, 6),  // e7
                                          square_from_file_rank(4, 7),  // e8
                                          PIECE_QUEEN);
        ASSERT(MOVE_TESTS, move_to_string(promotion, str, sizeof(str)));
        ASSERT(MOVE_TESTS, strcmp(str, "e7e8q") == 0);

        /* Test error cases */
        ASSERT(MOVE_TESTS, !move_to_string(basic, str, 2));        // Buffer too small
        ASSERT(MOVE_TESTS, !move_to_string(0, str, sizeof(str)));  // Invalid move
    }
    END_TEST_CASE(MOVE_TESTS);

    TEST_CASE(MOVE_TESTS, "String to move parsing") {
        board_t board;
        board_init(&board);

        /* Test 1: Basic move */
        board_reset(&board);
        move_t move = string_to_move("e2e4", &board);
        ASSERT(MOVE_TESTS, move != 0);
        ASSERT(MOVE_TESTS, get_from_square(move) == square_from_file_rank(4, 1));  // e2
        ASSERT(MOVE_TESTS, get_to_square(move) == square_from_file_rank(4, 3));    // e4

        /* Test 2: Promotion */
        board_clear(&board);
        board_set_piece(&board, square_from_file_rank(4, 6), MAKE_PIECE(PIECE_WHITE, PIECE_PAWN));
        move = string_to_move("e7e8q", &board);
        ASSERT(MOVE_TESTS, move != 0);
        ASSERT(MOVE_TESTS, get_from_square(move) == square_from_file_rank(4, 6));  // e7
        ASSERT(MOVE_TESTS, get_to_square(move) == square_from_file_rank(4, 7));    // e8
        ASSERT(MOVE_TESTS, get_promotion_type(move) == PIECE_QUEEN);

        /* Test invalid strings */
        ASSERT(MOVE_TESTS, string_to_move("", &board) == 0);        // Empty string
        ASSERT(MOVE_TESTS, string_to_move("e2e", &board) == 0);     // Too short
        ASSERT(MOVE_TESTS, string_to_move("e2e4q5", &board) == 0);  // Too long
        ASSERT(MOVE_TESTS, string_to_move("x2e4", &board) == 0);    // Invalid file
        ASSERT(MOVE_TESTS, string_to_move("e9e4", &board) == 0);    // Invalid rank
        ASSERT(MOVE_TESTS, string_to_move("e2e8x", &board) == 0);   // Invalid promotion
        ASSERT(MOVE_TESTS, string_to_move(NULL, &board) == 0);      // NULL string
        ASSERT(MOVE_TESTS, string_to_move("e2e4", NULL) == 0);      // NULL board
    }
    END_TEST_CASE(MOVE_TESTS);

    TEST_CASE(MOVE_TESTS, "Special move interactions") {
        /* Test en passant captures */
        square_t ep_from = square_from_file_rank(4, 4);  // e5
        square_t ep_to = square_from_file_rank(3, 5);    // d6
        move_t ep = make_special(ep_from, ep_to, SPECIAL_EN_PASSANT);

        /* Verify en passant is both special and capture */
        ASSERT(MOVE_TESTS, get_special_type(ep) == SPECIAL_EN_PASSANT);
        ASSERT(MOVE_TESTS, is_special(ep));
        ASSERT(MOVE_TESTS, get_capture_type(ep) == PIECE_PAWN);
        ASSERT(MOVE_TESTS, is_capture(ep));

        /* Test castling moves cannot be captures */
        square_t castle_from = square_from_file_rank(4, 0);  // e1
        square_t castle_to = square_from_file_rank(6, 0);    // g1
        move_t castle = make_special(castle_from, castle_to, SPECIAL_CASTLE_KING);

        ASSERT(MOVE_TESTS, get_special_type(castle) == SPECIAL_CASTLE_KING);
        ASSERT(MOVE_TESTS, is_special(castle));
        ASSERT(MOVE_TESTS, !is_capture(castle));
        ASSERT(MOVE_TESTS, get_capture_type(castle) == PIECE_NONE);
    }
    END_TEST_CASE(MOVE_TESTS);

    TEST_CASE(MOVE_TESTS, "Promotion with capture") {
        /* Test promoting captures in all four corners */
        square_t promotions[4][2] = {
            {square_from_file_rank(0, 6), square_from_file_rank(0, 7)},  // a7-a8
            {square_from_file_rank(7, 6), square_from_file_rank(7, 7)},  // h7-h8
            {square_from_file_rank(0, 1), square_from_file_rank(0, 0)},  // a2-a1
            {square_from_file_rank(7, 1), square_from_file_rank(7, 0)}   // h2-h1
        };

        piece_type_t promote_pieces[] = {PIECE_KNIGHT, PIECE_BISHOP, PIECE_ROOK, PIECE_QUEEN};

        for (size_t i = 0; i < 4; ++i) {
            for (piece_type_t j = 0; j < 4; ++j) {
                move_t move = make_capture_promotion(promotions[i][0], promotions[i][1],
                                                     PIECE_QUEEN, promote_pieces[j]);

                ASSERT(MOVE_TESTS, get_from_square(move) == promotions[i][0]);
                ASSERT(MOVE_TESTS, get_to_square(move) == promotions[i][1]);
                ASSERT(MOVE_TESTS, get_promotion_type(move) == promote_pieces[j]);
                ASSERT(MOVE_TESTS, get_capture_type(move) == PIECE_QUEEN);
                ASSERT(MOVE_TESTS, is_capture(move));
                ASSERT(MOVE_TESTS, is_promotion(move));
            }
        }
    }
    END_TEST_CASE(MOVE_TESTS);

    TEST_CASE(MOVE_TESTS, "Move string edge cases") {
        /* Test all promotion piece characters */
        char promo_chars[] = {'n', 'b', 'r', 'q', 'N', 'B', 'R', 'Q'};
        board_t board;
        board_clear(&board);
        board_set_piece(&board, square_from_file_rank(4, 6), MAKE_PIECE(PIECE_WHITE, PIECE_PAWN));

        for (piece_type_t i = 0; i < 8; ++i) {
            char move_str[6];
            snprintf(move_str, sizeof(move_str), "e7e8%c", promo_chars[i]);

            move_t move = string_to_move(move_str, &board);
            ASSERT(MOVE_TESTS, move != 0);
            ASSERT(MOVE_TESTS, is_promotion(move));

            /* Test case-insensitive parsing */
            piece_type_t expected_type;
            switch (tolower(promo_chars[i])) {
                case 'n': expected_type = PIECE_KNIGHT; break;
                case 'b': expected_type = PIECE_BISHOP; break;
                case 'r': expected_type = PIECE_ROOK; break;
                case 'q': expected_type = PIECE_QUEEN; break;
            }
            if (!expected_type) {
                ASSERT(MOVE_TESTS, false);  // Should never happen
            }
            ASSERT(MOVE_TESTS, get_promotion_type(move) == expected_type);
        }

        /* Test invalid promotion pieces */
        ASSERT(MOVE_TESTS, string_to_move("e7e8k", &board) == 0);  // King promotion
        ASSERT(MOVE_TESTS, string_to_move("e7e8p", &board) == 0);  // Pawn promotion
        ASSERT(MOVE_TESTS, string_to_move("e7e8x", &board) == 0);  // Invalid piece
    }
    END_TEST_CASE(MOVE_TESTS);

    TEST_CASE(MOVE_TESTS, "Move priority preservation") {
        /* Test that priority is preserved through all move creation methods */
        square_t from = square_from_file_rank(4, 1);  // e2
        square_t to = square_from_file_rank(4, 3);    // e4

        /* Test each priority level with different move types */
        for (move_priority_t priority = PRIORITY_NORMAL; priority <= PRIORITY_HASH; priority++) {
            /* Basic move */
            move_t basic = make_move(from, to);
            basic = set_priority(basic, priority);
            ASSERT(MOVE_TESTS, get_priority(basic) == priority);

            /* Capture */
            move_t capture = make_capture(from, to, PIECE_PAWN);
            capture = set_priority(capture, priority);
            ASSERT(MOVE_TESTS, get_priority(capture) == priority);
            ASSERT(MOVE_TESTS, get_capture_type(capture) == PIECE_PAWN);

            /* Promotion */
            move_t promotion = make_promotion(from, to, PIECE_QUEEN);
            promotion = set_priority(promotion, priority);
            ASSERT(MOVE_TESTS, get_priority(promotion) == priority);
            ASSERT(MOVE_TESTS, get_promotion_type(promotion) == PIECE_QUEEN);

            /* Special move */
            move_t special = make_special(from, to, SPECIAL_CASTLE_KING);
            special = set_priority(special, priority);
            ASSERT(MOVE_TESTS, get_priority(special) == priority);
            ASSERT(MOVE_TESTS, get_special_type(special) == SPECIAL_CASTLE_KING);
        }
    }
    END_TEST_CASE(MOVE_TESTS);

    print_test_results(&MOVE_TESTS);
}