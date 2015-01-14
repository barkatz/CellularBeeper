#include <tui/tui.h>
#include <string.h>
#include <tui/menu_screen.h>

void main_screen_handle_enter();

void main_screen_init() {
    tui_state.back_tui_handler = 0;
    tui_state.down_tui_handler = 0;
    tui_state.up_tui_handler = 0;
    tui_state.enter_tui_handler = main_screen_handle_enter;

    tui_state.top_line_index = 0;
    tui_state.cursor_index = -1;

    strcpy(tui_state.lines[0], "====================");
    strcpy(tui_state.lines[1], "        MAIN        ");
    strcpy(tui_state.lines[2], "====================");

    tui_update_screen();
}

void main_screen_handle_enter() {
    menu_screen_init();
}