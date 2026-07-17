#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include "lebui.h"

static struct termios lebui_orig_termios;
static int lebui_raw_on = 0;

void lebui_raw_enable(void) {
    struct termios raw;

    if (lebui_raw_on) return;
    if (tcgetattr(STDIN_FILENO, &lebui_orig_termios) != 0) return;
    raw = lebui_orig_termios;
    raw.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == 0) lebui_raw_on = 1;
}

void lebui_raw_disable(void) {
    if (lebui_raw_on) {
        if (tcsetattr(STDIN_FILENO, TCSANOW, &lebui_orig_termios) == 0)
            lebui_raw_on = 0;
    }
}

int lebui_raw_is_enabled(void) {
    return lebui_raw_on;
}

int lebui_init(void) {
    lebui_raw_enable();
    if (!lebui_raw_on) return LEBUI_RESULT_CANCEL;
    lebui_hide_cursor();
    return LEBUI_RESULT_OK;
}

void lebui_shutdown(void) {
    lebui_show_cursor();
    lebui_puts(LEBUI_CLR_NORMAL);
    lebui_flush();
    lebui_raw_disable();
}

void lebui_get_size(lebui_size_t *s) {
    struct winsize ws;

    if (!s) return;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 &&
        ws.ws_row > 0 && ws.ws_col > 0) {
        s->rows = ws.ws_row;
        s->cols = ws.ws_col;
    } else if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) == 0 &&
               ws.ws_row > 0 && ws.ws_col > 0) {
        s->rows = ws.ws_row;
        s->cols = ws.ws_col;
    } else {
        s->rows = LEBUI_DEFAULT_ROWS;
        s->cols = LEBUI_DEFAULT_COLS;
    }
}

void lebui_goto(int row, int col) {
    printf("\033[%d;%dH", row, col);
}

void lebui_clear(void) {
    printf("\033[2J\033[H");
}

void lebui_clear_line(void) {
    printf("\033[2K");
}

void lebui_hide_cursor(void) {
    printf("\033[?25l");
}

void lebui_show_cursor(void) {
    printf("\033[?25h");
}

void lebui_write(const char *s, int len) {
    int written;
    int result;

    if (!s || len <= 0) return;
    written = 0;
    while (written < len) {
        result = (int)write(STDOUT_FILENO, s + written, len - written);
        if (result <= 0) return;
        written += result;
    }
}

void lebui_puts(const char *s) {
    if (!s) return;
    lebui_write(s, (int)strlen(s));
}

void lebui_flush(void) {
    fflush(stdout);
}

