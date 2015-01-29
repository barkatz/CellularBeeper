#ifndef _TUI_H
#define _TUI_H

#include <misc.h>

#define SCREEN_WIDTH  20                // columns
#define SCREEN_HEIGHT 4                 // lines

#define TUI_WIDTH     (SCREEN_WIDTH)    // columns
#define TUI_HEIGHT    (SCREEN_HEIGHT-1) // lines

// the screen line in which the TUI starts drawing
#define TUI_FIRST_LINE 1

// this is the callback definition for TUI operations (back/down/up/enter)
typedef void (*tui_handler_t)(void);

// this represents the current TUI state
typedef struct tui_state_t {
        // the currently displayed lines
    char lines[TUI_HEIGHT][TUI_WIDTH+1];

        // the currently assigned TUI operation handlers
    tui_handler_t back_tui_handler;
    tui_handler_t down_tui_handler;
    tui_handler_t up_tui_handler;
    tui_handler_t enter_tui_handler;
} tui_state_t;

// this represents the current state of a specific TUI screen (which may not be
// currently displayed).
// we have to remember the state of non-current screens in order to come back
// to them in a correct state.
typedef struct tui_screen_state_t {
        // the index of the first displayed line
    word top_line_index;
        // the index of the currently chosen line
    word cursor_index
} tui_screen_state_t;

// the global current TUI state
extern tui_state_t tui_state;

// initializes the global current TUI state and draws the main screen
void tui_init();

// calls the currently assigned TUI operation handlers
void tui_handle_back();
void tui_handle_down();
void tui_handle_up();
void tui_handle_enter();

// redraws the screen with the currently displayed lines
void tui_update_screen();

#endif // _TUI_H