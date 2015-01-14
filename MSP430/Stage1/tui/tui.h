#ifndef _TUI_H
#define _TUI_H

#include <misc.h>

#define SCREEN_WIDTH  20
#define SCREEN_HEIGHT 4

#define TUI_WIDTH     (SCREEN_WIDTH)
#define TUI_HEIGHT    (SCREEN_HEIGHT-1)

#define TUI_FIRST_LINE 1

typedef void (*tui_handler_t)(void);

typedef struct tui_state_t {
    char lines[TUI_HEIGHT][TUI_WIDTH+1];
    word top_line_index;
    word cursor_index;

    tui_handler_t back_tui_handler;
    tui_handler_t down_tui_handler;
    tui_handler_t up_tui_handler;
    tui_handler_t enter_tui_handler;
} tui_state_t;

extern tui_state_t tui_state;

void tui_init();

void tui_handle_back();
void tui_handle_down();
void tui_handle_up();
void tui_handle_enter();

void tui_update_screen();

#endif // _TUI_H