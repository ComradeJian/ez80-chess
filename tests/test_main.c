#include "test_board.h"
#include "test_move.h"

#include <debug.h>

int main(void) {
    dbg_ClearConsole();
    dbg_printf("\n==========================================");
    dbg_printf("\n          Running test suite ...          ");
    dbg_printf("\n==========================================\n");

    run_board_tests();
    run_fen_tests();
    run_move_tests();

    dbg_printf("\n==========================================\n");

    return 0;
}