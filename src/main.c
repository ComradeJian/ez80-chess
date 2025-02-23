#include "board.h"

#include <debug.h>

int main(void) {
    dbg_ClearConsole();

    dbg_printf("\n╔════════════════════════════════════════╗");
    dbg_printf("\n║          Chess Engine v0.1.0           ║");
    dbg_printf("\n╚════════════════════════════════════════╝\n");

    board_t board;
    board_init(&board);
    board_reset(&board);
    board_display(&board);

    return 0;
}