#include <tui/tui.h>
#include <misc.h>
#include <lcdi2c.h>
#include <i2c.h>
#include <tui/main_screen.h>

tui_state_t tui_state;

void tui_update_screen() {
    byte row, col, i;

    // set the position to the top-left corner of the TUI area
    lcdi2c_set_pos(0, TUI_FIRST_LINE);

    for (row = 0; row < TUI_HEIGHT ; row++) {
        col = 0;

        // prefix the line with a space or cursor if required
        if (tui_state.cursor_index != -1) {
            if (tui_state.cursor_index == tui_state.top_line_index + row)
                lcdi2c_putc('>');
            else
                lcdi2c_putc(' ');
            col++;
        }

        // write the content of the line itself
        for (i = 0; tui_state.lines[row][i] && col < TUI_WIDTH; i++, col++)
            lcdi2c_putc(tui_state.lines[row][i]);

        // pad with spaces
        for (; col < TUI_WIDTH; col++)
            lcdi2c_putc(' ');
    }
}

void tui_init()
{
    // initialize the LCD screen
        // initialize the I2C library
    i2c_init(LCD_I2C_ADDR);

        // initialize the LCD module
    lcdi2c_init(SCREEN_WIDTH, SCREEN_HEIGHT);

    // initialize the main screen
    main_screen_init();
}

void tui_handle_back()
{
    if (tui_state.back_tui_handler)
        tui_state.back_tui_handler();
}

void tui_handle_down()
{
    if (tui_state.down_tui_handler)
        tui_state.down_tui_handler();
}

void tui_handle_up()
{
    if (tui_state.up_tui_handler)
        tui_state.up_tui_handler();
}

void tui_handle_enter()
{
    if (tui_state.enter_tui_handler)
        tui_state.enter_tui_handler();
}
