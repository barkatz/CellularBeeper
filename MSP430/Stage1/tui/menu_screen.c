#include <tui/tui.h>
#include <string.h>
#include <tui/main_screen.h>

void menu_screen_handle_back();
void menu_screen_handle_down();
void menu_screen_handle_up();

void menu_screen_init() {
    tui_state.back_tui_handler = menu_screen_handle_back;
    tui_state.down_tui_handler = menu_screen_handle_down;
    tui_state.up_tui_handler = menu_screen_handle_up;
    tui_state.enter_tui_handler = 0;

    tui_state.top_line_index = 0;
    tui_state.cursor_index = 0;

    strcpy(tui_state.lines[0], "foo");
    strcpy(tui_state.lines[1], "bar");
    strcpy(tui_state.lines[2], "baz");

    tui_update_screen();
}

void menu_screen_handle_back() {
    main_screen_init();
}

void menu_screen_handle_up() {
    if (tui_state.cursor_index == 0)
        tui_state.cursor_index = 2;
    else
        tui_state.cursor_index--;

    tui_update_screen();
}

void menu_screen_handle_down() {
    if (tui_state.cursor_index == 2)
        tui_state.cursor_index = 0;
    else
        tui_state.cursor_index++;

    tui_update_screen();
}