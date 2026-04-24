#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <lebirun.h>
#include "lebui.h"

static struct termios lebui_orig_termios;
static int lebui_raw_on = 0;

static const char **g_tabbar_tabs = (const char **)0;
static int g_tabbar_count = 0;
static int g_tabbar_active = 0;
static int g_tabbar_cols = 0;

void lebui_tabbar_attach(const char **tabs, int count, int active, int cols) {
    g_tabbar_tabs = tabs;
    g_tabbar_count = count;
    g_tabbar_active = active;
    g_tabbar_cols = cols;
}

void lebui_tabbar_detach(void) {
    g_tabbar_tabs = (const char **)0;
    g_tabbar_count = 0;
}

static int lebui_top_content_row(void) {
    return (g_tabbar_tabs && g_tabbar_count > 0) ? 2 : 1;
}

void lebui_raw_enable(void) {
    struct termios raw;

    tcgetattr(STDIN_FILENO, &lebui_orig_termios);
    raw = lebui_orig_termios;
    raw.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    lebui_raw_on = 1;
}

void lebui_raw_disable(void) {
    if (lebui_raw_on) {
        tcsetattr(STDIN_FILENO, TCSANOW, &lebui_orig_termios);
        lebui_raw_on = 0;
    }
}

int lebui_read_key(void) {
    char c;
    char seq[4];

    if (read(STDIN_FILENO, &c, 1) <= 0) return -1;

    if (c == '\033') {
        if (read_nb(STDIN_FILENO, &seq[0], 1) != 1) return LEBUI_KEY_ESC;
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return LEBUI_KEY_ESC;
        if (seq[0] == '[') {
            if (seq[1] >= '0' && seq[1] <= '9') {
                if (read(STDIN_FILENO, &seq[2], 1) != 1) return LEBUI_KEY_ESC;
                if (seq[2] == '~') {
                    switch (seq[1]) {
                    case '1': return LEBUI_KEY_HOME;
                    case '2': return LEBUI_KEY_INSERT;
                    case '3': return LEBUI_KEY_DELETE;
                    case '4': return LEBUI_KEY_END;
                    case '5': return LEBUI_KEY_PGUP;
                    case '6': return LEBUI_KEY_PGDN;
                    case '7': return LEBUI_KEY_HOME;
                    case '8': return LEBUI_KEY_END;
                    }
                }
                if (seq[1] == '1') {
                    if (seq[2] >= '5' && seq[2] <= '9') {
                        char tilde;
                        if (read(STDIN_FILENO, &tilde, 1) == 1 && tilde == '~') {
                            switch (seq[2]) {
                            case '5': return LEBUI_KEY_F5;
                            case '7': return LEBUI_KEY_F6;
                            case '8': return LEBUI_KEY_F7;
                            case '9': return LEBUI_KEY_F8;
                            }
                        }
                    }
                }
                if (seq[1] == '2') {
                    if (seq[2] >= '0' && seq[2] <= '4') {
                        char tilde;
                        if (read(STDIN_FILENO, &tilde, 1) == 1 && tilde == '~') {
                            switch (seq[2]) {
                            case '0': return LEBUI_KEY_F9;
                            case '1': return LEBUI_KEY_F10;
                            case '3': return LEBUI_KEY_F11;
                            case '4': return LEBUI_KEY_F12;
                            }
                        }
                    }
                }
            } else {
                switch (seq[1]) {
                case 'A': return LEBUI_KEY_UP;
                case 'B': return LEBUI_KEY_DOWN;
                case 'C': return LEBUI_KEY_RIGHT;
                case 'D': return LEBUI_KEY_LEFT;
                case 'H': return LEBUI_KEY_HOME;
                case 'F': return LEBUI_KEY_END;
                }
            }
            if (seq[0] == 'O') {
                switch (seq[1]) {
                case 'P': return LEBUI_KEY_F1;
                case 'Q': return LEBUI_KEY_F2;
                case 'R': return LEBUI_KEY_F3;
                case 'S': return LEBUI_KEY_F4;
                }
            }
        }
        return LEBUI_KEY_ESC;
    }

    if (c == '\r' || c == '\n') return LEBUI_KEY_ENTER;
    if (c == '\t') return LEBUI_KEY_TAB;
    if (c == 127 || c == '\b') return LEBUI_KEY_BKSP;
    return (int)(unsigned char)c;
}

void lebui_get_size(lebui_size_t *s) {
    struct winsize ws;

    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_row > 0) {
        s->rows = ws.ws_row;
        s->cols = ws.ws_col;
    } else {
        s->rows = 25;
        s->cols = 80;
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
    write(STDOUT_FILENO, s, len);
}

void lebui_puts(const char *s) {
    lebui_write(s, strlen(s));
}

void lebui_flush(void) {
    fflush(stdout);
}

void lebui_fill_bg(const char *color, int rows, int cols) {
    int r;

    (void)cols;
    printf("%s", color);
    for (r = 1; r <= rows; r++) {
        lebui_goto(r, 1);
        lebui_clear_line();
    }
}

void lebui_draw_box(int y, int x, int h, int w, const char *title) {
    int i;
    int r;
    int tw;

    lebui_goto(y, x);
    printf("%s+", LEBUI_CLR_BORDER);
    for (i = 0; i < w - 2; i++) putchar('-');
    putchar('+');

    if (title) {
        tw = (int)strlen(title);
        lebui_goto(y, x + (w - tw - 2) / 2);
        printf("%s %s ", LEBUI_CLR_TITLE, title);
    }

    for (r = 1; r < h - 1; r++) {
        lebui_goto(y + r, x);
        printf("%s|", LEBUI_CLR_BORDER);
        printf("%s", LEBUI_CLR_MENU);
        for (i = 0; i < w - 2; i++) putchar(' ');
        printf("%s|", LEBUI_CLR_BORDER);
    }

    lebui_goto(y + h - 1, x);
    printf("%s+", LEBUI_CLR_BORDER);
    for (i = 0; i < w - 2; i++) putchar('-');
    putchar('+');
}

void lebui_draw_hline(int row, int col, int len, char ch) {
    int i;

    lebui_goto(row, col);
    for (i = 0; i < len; i++) putchar(ch);
}

void lebui_center_text(int row, int bx, int bw, const char *color, const char *text) {
    int len;
    int col;

    len = (int)strlen(text);
    col = bx + (bw - len) / 2;
    if (col < bx + 1) col = bx + 1;
    lebui_goto(row, col);
    printf("%s%s", color, text);
}

void lebui_draw_titlebar(const char *title, int cols) {
    int i;
    int len;

    if (g_tabbar_tabs && g_tabbar_count > 0) {
        lebui_draw_tabbar(g_tabbar_tabs, g_tabbar_count, g_tabbar_active, cols);
        return;
    }

    lebui_goto(1, 1);
    printf("%s", LEBUI_CLR_BAR);
    len = (int)strlen(title);
    printf("%s", title);
    for (i = len; i < cols; i++) putchar(' ');
}

void lebui_draw_tabbar(const char **tabs, int count, int active, int cols) {
    int i;
    int len;
    int used;
    const char *hint;
    int hlen;
    const char *brand;

    brand = " Lebirun Installer ";
    lebui_goto(1, 1);
    printf("%s", LEBUI_CLR_TITLE);
    for (i = 0; i < cols; i++) putchar(' ');
    lebui_goto(1, 1);
    printf("%s", LEBUI_CLR_TITLE);
    printf("%s", brand);
    used = (int)strlen(brand);
    if (used < cols) {
        putchar(' ');
        used++;
    }
    for (i = 0; i < count; i++) {
        len = (int)strlen(tabs[i]);
        if (i == active) {
            printf("%s[ %s ]%s", LEBUI_CLR_BTN_SEL, tabs[i], LEBUI_CLR_TITLE);
            used += len + 4;
        } else {
            printf("%s %s %s", LEBUI_CLR_TITLE, tabs[i], LEBUI_CLR_TITLE);
            used += len + 2;
        }
        if (i < count - 1) {
            printf("%s | %s", LEBUI_CLR_TITLE, LEBUI_CLR_TITLE);
            used += 3;
        }
    }
    hint = " <TAB> Switch ";
    hlen = (int)strlen(hint);
    if (used + hlen < cols) {
        for (i = used; i < cols - hlen; i++) putchar(' ');
        printf("%s", LEBUI_CLR_TITLE);
        printf("%s", hint);
    }
    printf("%s", LEBUI_CLR_NORMAL);
}

void lebui_draw_helpbar(const char *text, int row, int cols) {
    int i;
    int len;

    lebui_goto(row, 1);
    printf("%s", LEBUI_CLR_BAR);
    len = (int)strlen(text);
    printf("%s", text);
    for (i = len; i < cols; i++) putchar(' ');
}

int lebui_menu(const char *title, const char **items, int count,
               int start_sel, int y, int x, int h, int w) {
    int sel;
    int top;
    int visible;
    int i;
    int key;
    int inner_h;

    sel = start_sel;
    if (sel < 0) sel = 0;
    if (sel >= count) sel = count - 1;
    top = 0;
    inner_h = h - 2;
    if (inner_h < 1) inner_h = 1;

    for (;;) {
        lebui_draw_box(y, x, h, w, title);

        if (sel < top) top = sel;
        if (sel >= top + inner_h) top = sel - inner_h + 1;

        visible = count - top;
        if (visible > inner_h) visible = inner_h;

        for (i = 0; i < visible; i++) {
            int idx;
            int pad;
            int tlen;

            idx = top + i;
            lebui_goto(y + 1 + i, x + 1);
            if (idx == sel) {
                printf("%s", LEBUI_CLR_SELECT);
            } else {
                printf("%s", LEBUI_CLR_MENU);
            }
            tlen = (int)strlen(items[idx]);
            pad = w - 2 - tlen;
            if (pad < 0) pad = 0;
            printf(" %s", items[idx]);
            for (; pad > 0; pad--) putchar(' ');
        }

        lebui_flush();
        key = lebui_read_key();

        if (key == LEBUI_KEY_UP && sel > 0) sel--;
        else if (key == LEBUI_KEY_DOWN && sel < count - 1) sel++;
        else if (key == LEBUI_KEY_PGUP) {
            sel -= inner_h;
            if (sel < 0) sel = 0;
        } else if (key == LEBUI_KEY_PGDN) {
            sel += inner_h;
            if (sel >= count) sel = count - 1;
        } else if (key == LEBUI_KEY_HOME) sel = 0;
        else if (key == LEBUI_KEY_END) sel = count - 1;
        else if (key == LEBUI_KEY_ENTER) return sel;
        else if (key == LEBUI_KEY_TAB) return LEBUI_KEY_TAB;
        else if (key == LEBUI_KEY_ESC || key == 'q') return -1;
    }
}

int lebui_input(const char *title, char *buf, int bufsz,
                int y, int x, int w) {
    int len;
    int cur;
    int key;
    int iw;
    int i;
    int start;
    int visible;

    len = (int)strlen(buf);
    cur = len;
    iw = w - 4;
    if (iw < 4) iw = 4;

    for (;;) {
        lebui_draw_box(y, x, 5, w, title);

        start = 0;
        if (cur > iw - 1) start = cur - iw + 1;
        visible = len - start;
        if (visible > iw) visible = iw;

        lebui_goto(y + 2, x + 2);
        printf("%s", LEBUI_CLR_INPUT);
        for (i = 0; i < iw; i++) {
            if (i < visible) {
                putchar(buf[start + i]);
            } else {
                putchar(' ');
            }
        }

        lebui_goto(y + 2, x + 2 + (cur - start));
        lebui_show_cursor();
        lebui_flush();

        key = lebui_read_key();

        if (key == LEBUI_KEY_ENTER) {
            lebui_hide_cursor();
            return 0;
        }
        if (key == LEBUI_KEY_ESC) {
            lebui_hide_cursor();
            return -1;
        }
        if (key == LEBUI_KEY_LEFT && cur > 0) cur--;
        else if (key == LEBUI_KEY_RIGHT && cur < len) cur++;
        else if (key == LEBUI_KEY_HOME) cur = 0;
        else if (key == LEBUI_KEY_END) cur = len;
        else if (key == LEBUI_KEY_BKSP && cur > 0) {
            memmove(&buf[cur - 1], &buf[cur], len - cur);
            len--;
            cur--;
            buf[len] = '\0';
        } else if (key == LEBUI_KEY_DELETE && cur < len) {
            memmove(&buf[cur], &buf[cur + 1], len - cur - 1);
            len--;
            buf[len] = '\0';
        } else if (key >= 32 && key < 127 && len < bufsz - 1) {
            memmove(&buf[cur + 1], &buf[cur], len - cur);
            buf[cur] = (char)key;
            len++;
            cur++;
            buf[len] = '\0';
        }
    }
}

int lebui_confirm(const char *title, const char *message,
                  int y, int x, int w) {
    int sel;
    int key;

    sel = 1;

    for (;;) {
        lebui_draw_box(y, x, 7, w, title);

        lebui_center_text(y + 2, x, w, LEBUI_CLR_MENU, message);

        lebui_goto(y + 4, x + w / 2 - 10);
        if (sel == 0) {
            printf("%s[ Yes ]%s  No  ", LEBUI_CLR_BTN_SEL, LEBUI_CLR_BTN);
        } else {
            printf("%s  Yes  %s[ No ]", LEBUI_CLR_BTN, LEBUI_CLR_BTN_SEL);
        }

        lebui_flush();
        key = lebui_read_key();

        if (key == LEBUI_KEY_LEFT || key == LEBUI_KEY_RIGHT || key == LEBUI_KEY_TAB) {
            sel = !sel;
        } else if (key == LEBUI_KEY_ENTER) {
            return (sel == 0) ? 1 : 0;
        } else if (key == 'y' || key == 'Y') {
            return 1;
        } else if (key == 'n' || key == 'N' || key == LEBUI_KEY_ESC) {
            return 0;
        }
    }
}

void lebui_msgbox(const char *title, const char *message,
                  int y, int x, int w) {
    int key;

    lebui_draw_box(y, x, 7, w, title);
    lebui_center_text(y + 2, x, w, LEBUI_CLR_MENU, message);
    lebui_center_text(y + 4, x, w, LEBUI_CLR_BTN_SEL, "[ OK ]");
    lebui_flush();

    for (;;) {
        key = lebui_read_key();
        if (key == LEBUI_KEY_ENTER || key == LEBUI_KEY_ESC || key == 'q') return;
    }
}

void lebui_progress(const char *label, int percent,
                    int y, int x, int w) {
    int bar_w;
    int filled;
    int i;

    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;

    bar_w = w - 4;
    if (bar_w < 4) bar_w = 4;
    filled = percent * bar_w / 100;

    lebui_goto(y, x + 2);
    printf("%s%s", LEBUI_CLR_MENU, label);

    lebui_goto(y + 1, x + 2);
    for (i = 0; i < bar_w; i++) {
        if (i < filled) {
            printf("%s ", LEBUI_CLR_PROG);
        } else {
            printf("%s ", LEBUI_CLR_PROG_BG);
        }
    }

    printf("%s %3d%%", LEBUI_CLR_MENU, percent);
}

int lebui_checklist(const char *title, const char **items, int *checked,
                    int count, int y, int x, int h, int w) {
    int sel;
    int top;
    int visible;
    int inner_h;
    int i;
    int key;

    sel = 0;
    top = 0;
    inner_h = h - 2;
    if (inner_h < 1) inner_h = 1;

    for (;;) {
        lebui_draw_box(y, x, h, w, title);

        if (sel < top) top = sel;
        if (sel >= top + inner_h) top = sel - inner_h + 1;

        visible = count - top;
        if (visible > inner_h) visible = inner_h;

        for (i = 0; i < visible; i++) {
            int idx;
            int pad;
            int tlen;

            idx = top + i;
            lebui_goto(y + 1 + i, x + 1);
            if (idx == sel) {
                printf("%s", LEBUI_CLR_SELECT);
            } else {
                printf("%s", LEBUI_CLR_MENU);
            }
            printf(" [%c] %s", checked[idx] ? 'X' : ' ', items[idx]);
            tlen = (int)strlen(items[idx]) + 5;
            pad = w - 2 - tlen;
            for (; pad > 0; pad--) putchar(' ');
        }

        lebui_flush();
        key = lebui_read_key();

        if (key == LEBUI_KEY_UP && sel > 0) sel--;
        else if (key == LEBUI_KEY_DOWN && sel < count - 1) sel++;
        else if (key == ' ') checked[sel] = !checked[sel];
        else if (key == LEBUI_KEY_ENTER) return 0;
        else if (key == LEBUI_KEY_ESC) return -1;
    }
}

void lebui_draw_box_shadow(int y, int x, int h, int w, const char *title) {
    int r;
    int i;

    lebui_draw_box(y, x, h, w, title);

    for (r = 1; r < h; r++) {
        lebui_goto(y + r, x + w);
        printf("%s ", LEBUI_CLR_SHADOW);
    }

    lebui_goto(y + h, x + 1);
    printf("%s", LEBUI_CLR_SHADOW);
    for (i = 0; i < w; i++) putchar(' ');
}

void lebui_draw_screen(const char *titlebar, const char *helpbar,
                       int rows, int cols) {
    lebui_fill_bg(LEBUI_CLR_MENU, rows, cols);
    lebui_draw_titlebar(titlebar, cols);
    lebui_draw_helpbar(helpbar, rows, cols);
    if (g_tabbar_tabs && g_tabbar_count > 0)
        lebui_draw_tabbar(g_tabbar_tabs, g_tabbar_count, g_tabbar_active, cols);
}

int lebui_menu_auto(const char *title, const char **items, int count,
                    const char *helpbar, int rows, int cols) {
    int maxw;
    int len;
    int bw;
    int bh;
    int bx;
    int by;
    int i;
    int sel;
    int scroll;
    int view_h;
    int key;
    int sb_pos;
    int sb_h;

    maxw = (int)strlen(title) + 4;
    for (i = 0; i < count; i++) {
        len = (int)strlen(items[i]);
        if (len + 8 > maxw) maxw = len + 8;
    }
    bw = maxw + 4;
    if (bw > cols - 4) bw = cols - 4;
    bh = count + 4;
    if (bh > rows - 4) bh = rows - 4;
    bx = (cols - bw) / 2 + 1;
    by = (rows - bh) / 2;
    if (by < lebui_top_content_row() + 2) by = lebui_top_content_row() + 2;
    view_h = bh - 4;

    sel = 0;
    scroll = 0;

    lebui_draw_screen(title, helpbar, rows, cols);
    lebui_draw_box_shadow(by, bx, bh, bw, title);

    for (;;) {
        if (sel < scroll) scroll = sel;
        if (sel >= scroll + view_h) scroll = sel - view_h + 1;

        for (i = 0; i < view_h; i++) {
            int idx;
            idx = scroll + i;
            lebui_goto(by + 2 + i, bx + 2);
            if (idx < count) {
                if (idx == sel)
                    printf("%s", LEBUI_CLR_SELECT);
                else
                    printf("%s", LEBUI_CLR_MENU);
                printf(" %-*.*s ", bw - 8, bw - 8, items[idx]);
            } else {
                printf("%s%-*s", LEBUI_CLR_MENU, bw - 6, "");
            }
        }

        if (count > view_h) {
            sb_h = view_h;
            sb_pos = (sel * (sb_h - 1)) / (count - 1);
            for (i = 0; i < sb_h; i++) {
                lebui_goto(by + 2 + i, bx + bw - 3);
                if (i == sb_pos)
                    printf("%s#%s", LEBUI_CLR_SELECT, LEBUI_CLR_BORDER);
                else
                    printf("%s:%s", LEBUI_CLR_DIM_CLR, LEBUI_CLR_BORDER);
            }
        }

        printf("%s", LEBUI_CLR_NORMAL);
        lebui_flush();

        key = lebui_read_key();
        if (key == LEBUI_KEY_UP && sel > 0) sel--;
        else if (key == LEBUI_KEY_DOWN && sel < count - 1) sel++;
        else if (key == LEBUI_KEY_ENTER) return sel;
        else if (key == LEBUI_KEY_TAB) return LEBUI_KEY_TAB;
        else if (key == LEBUI_KEY_ESC) return -1;
    }
}

int lebui_input_ex(const char *title, const char *prompt, char *buf,
                   int maxlen, int hidden, int rows, int cols) {
    int bw;
    int bh;
    int bx;
    int by;
    int key;
    int len;
    int plen;
    int tlen;
    int fw;
    int i;

    len = 0;
    buf[0] = '\0';
    plen = (int)strlen(prompt);
    tlen = (int)strlen(title);
    bw = 50;
    if (plen + 6 > bw) bw = plen + 6;
    if (tlen + 6 > bw) bw = tlen + 6;
    if (bw > cols - 4) bw = cols - 4;
    bh = 8;
    bx = (cols - bw) / 2 + 1;
    by = (rows - bh) / 2;
    if (by < lebui_top_content_row() + 2) by = lebui_top_content_row() + 2;
    fw = bw - 6;
    if (maxlen - 1 < fw) fw = maxlen - 1;

    lebui_draw_screen(title, " <Enter> Confirm  <Esc> Cancel", rows, cols);
    lebui_draw_box_shadow(by, bx, bh, bw, title);

    lebui_goto(by + 2, bx + 2);
    printf("%s%s", LEBUI_CLR_MENU, prompt);

    lebui_goto(by + 6, bx + bw / 2 - 3);
    printf("%s< OK >%s", LEBUI_CLR_BTN, LEBUI_CLR_NORMAL);

    for (;;) {
        lebui_goto(by + 4, bx + 3);
        printf("%s", LEBUI_CLR_INPUT);
        if (hidden) {
            for (i = 0; i < len && i < fw; i++) putchar('*');
        } else {
            for (i = 0; i < len && i < fw; i++) putchar(buf[i]);
        }
        for (i = len; i < fw; i++) putchar(' ');

        printf("%s", LEBUI_CLR_NORMAL);
        lebui_goto(by + 4, bx + 3 + len);
        lebui_show_cursor();
        lebui_flush();

        key = lebui_read_key();
        lebui_hide_cursor();

        if (key == LEBUI_KEY_ENTER) {
            buf[len] = '\0';
            return len;
        } else if (key == LEBUI_KEY_ESC) {
            buf[0] = '\0';
            return -1;
        } else if (key == LEBUI_KEY_BKSP) {
            if (len > 0) len--;
        } else if (key >= 32 && key < 127 && len < fw) {
            buf[len++] = (char)key;
        }
        buf[len] = '\0';
    }
}

int lebui_confirm_auto(const char *title, const char *message,
                       int rows, int cols) {
    int sel;
    int bw;
    int bh;
    int bx;
    int by;
    int key;
    int mlen;
    int tlen;

    sel = 0;
    mlen = (int)strlen(message);
    tlen = (int)strlen(title);
    bw = mlen + 6;
    if (tlen + 6 > bw) bw = tlen + 6;
    if (bw < 30) bw = 30;
    if (bw > cols - 4) bw = cols - 4;
    bh = 8;
    bx = (cols - bw) / 2 + 1;
    by = (rows - bh) / 2;
    if (by < lebui_top_content_row() + 2) by = lebui_top_content_row() + 2;

    lebui_draw_screen(title, " <Tab> Switch  <Enter> Select", rows, cols);
    lebui_draw_box_shadow(by, bx, bh, bw, title);
    lebui_center_text(by + 2, bx, bw, LEBUI_CLR_MENU, message);

    for (;;) {
        lebui_goto(by + 5, bx + bw / 2 - 9);
        if (sel == 0)
            printf("%s< Yes  >%s  %s[  No  ]%s", LEBUI_CLR_BTN_SEL, LEBUI_CLR_MENU, LEBUI_CLR_BTN, LEBUI_CLR_NORMAL);
        else
            printf("%s[ Yes  ]%s  %s<  No  >%s", LEBUI_CLR_BTN, LEBUI_CLR_MENU, LEBUI_CLR_BTN_SEL, LEBUI_CLR_NORMAL);

        lebui_flush();

        key = lebui_read_key();
        if (key == LEBUI_KEY_TAB || key == LEBUI_KEY_LEFT || key == LEBUI_KEY_RIGHT) {
            sel = !sel;
        } else if (key == LEBUI_KEY_ENTER) {
            return (sel == 0) ? 1 : 0;
        } else if (key == LEBUI_KEY_ESC) {
            return 0;
        } else if (key == 'y' || key == 'Y') {
            return 1;
        } else if (key == 'n' || key == 'N') {
            return 0;
        }
    }
}

void lebui_msgbox_auto(const char *title, const char *message,
                       int rows, int cols) {
    int bw;
    int bh;
    int bx;
    int by;
    int key;
    int mlen;
    int tlen;

    mlen = (int)strlen(message);
    tlen = (int)strlen(title);
    bw = mlen + 6;
    if (tlen + 6 > bw) bw = tlen + 6;
    if (bw < 24) bw = 24;
    if (bw > cols - 4) bw = cols - 4;
    bh = 7;
    bx = (cols - bw) / 2 + 1;
    by = (rows - bh) / 2;
    if (by < lebui_top_content_row() + 2) by = lebui_top_content_row() + 2;

    lebui_draw_screen(title, " <Enter> OK", rows, cols);
    lebui_draw_box_shadow(by, bx, bh, bw, title);
    lebui_center_text(by + 2, bx, bw, LEBUI_CLR_MENU, message);
    lebui_goto(by + 4, bx + bw / 2 - 3);
    printf("%s< OK >%s", LEBUI_CLR_BTN_SEL, LEBUI_CLR_NORMAL);
    lebui_flush();

    for (;;) {
        key = lebui_read_key();
        if (key == LEBUI_KEY_ENTER || key == LEBUI_KEY_ESC) return;
    }
}

void lebui_progress_init(lebui_prog_state_t *st, const char *title,
                         int rows, int cols) {
    int prog_h;
    int top_row;

    top_row = lebui_top_content_row();
    st->bw = 56;
    if (st->bw > cols - 4) st->bw = cols - 4;
    prog_h = 8;
    st->log_h = 10;
    if (st->log_h > rows - prog_h - 6) st->log_h = rows - prog_h - 6;
    if (st->log_h < 4) st->log_h = 4;
    st->bx = (cols - st->bw) / 2 + 1;
    st->by = top_row + 2;
    st->log_y = st->by + prog_h;
    st->log_count = 0;
    st->drawn = 0;

    lebui_draw_screen(title, " Please wait...", rows, cols);
    lebui_draw_box_shadow(st->by, st->bx, prog_h, st->bw, "Installing");
    lebui_draw_box_shadow(st->log_y, st->bx, st->log_h, st->bw, "Log");
    lebui_flush();
    st->drawn = 1;
}

void lebui_progress_update(lebui_prog_state_t *st, const char *msg, int pct) {
    int bar_w;
    int filled;
    int mw;
    int i;

    bar_w = st->bw - 8;
    if (bar_w < 1) bar_w = 1;
    mw = st->bw - 6;
    if (mw < 1) mw = 1;

    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;

    lebui_goto(st->by + 2, st->bx + 3);
    printf("%s%-*.*s", LEBUI_CLR_MENU, mw, mw, msg);

    filled = (pct * bar_w) / 100;
    if (filled > bar_w) filled = bar_w;

    lebui_goto(st->by + 4, st->bx + 4);
    printf("%s", LEBUI_CLR_PROG);
    for (i = 0; i < filled; i++) putchar(' ');
    printf("%s", LEBUI_CLR_PROG_BG);
    for (i = filled; i < bar_w; i++) putchar(' ');

    lebui_goto(st->by + 5, st->bx + st->bw / 2 - 2);
    printf("%s%3d%%", LEBUI_CLR_MENU, pct);

    printf("%s", LEBUI_CLR_NORMAL);
    lebui_flush();
}

void lebui_progress_log(lebui_prog_state_t *st, const char *msg) {
    int log_area;
    int i;
    int mw;

    log_area = st->log_h - 2;
    if (log_area > LEBUI_PROG_LOG_MAX) log_area = LEBUI_PROG_LOG_MAX;
    if (log_area < 1) return;
    mw = st->bw - 6;
    if (mw < 1) mw = 1;
    if (mw > 63) mw = 63;

    if (st->log_count < log_area) {
        strncpy(st->log_lines[st->log_count], msg, mw);
        st->log_lines[st->log_count][mw] = '\0';
        st->log_count++;
    } else {
        for (i = 0; i < log_area - 1; i++)
            strcpy(st->log_lines[i], st->log_lines[i + 1]);
        strncpy(st->log_lines[log_area - 1], msg, mw);
        st->log_lines[log_area - 1][mw] = '\0';
    }

    for (i = 0; i < st->log_count && i < log_area; i++) {
        lebui_goto(st->log_y + 1 + i, st->bx + 3);
        printf("%s%-*s", LEBUI_CLR_DIM_CLR, mw, st->log_lines[i]);
    }

    printf("%s", LEBUI_CLR_NORMAL);
    lebui_flush();
}

void lebui_progress_reset(lebui_prog_state_t *st) {
    st->drawn = 0;
    st->log_count = 0;
}

int lebui_checklist_auto(const char *title, const char **items, int *checked,
                         int count, const char *helpbar, int rows, int cols) {
    int maxw;
    int len;
    int bw;
    int bh;
    int bx;
    int by;
    int view_h;
    int sel;
    int scroll;
    int i;
    int key;
    char line[80];

    sel = 0;
    scroll = 0;
    maxw = (int)strlen(title) + 4;
    for (i = 0; i < count; i++) {
        len = (int)strlen(items[i]) + 4;
        if (len + 8 > maxw) maxw = len + 8;
    }
    bw = maxw + 4;
    if (bw > cols - 4) bw = cols - 4;
    bh = count + 4;
    if (bh > rows - 4) bh = rows - 4;
    bx = (cols - bw) / 2 + 1;
    by = (rows - bh) / 2;
    if (by < lebui_top_content_row() + 2) by = lebui_top_content_row() + 2;
    view_h = bh - 4;

    lebui_draw_screen(title, helpbar, rows, cols);
    lebui_draw_box_shadow(by, bx, bh, bw, title);

    for (;;) {
        if (sel < scroll) scroll = sel;
        if (sel >= scroll + view_h) scroll = sel - view_h + 1;

        for (i = 0; i < view_h; i++) {
            int idx;
            idx = scroll + i;
            lebui_goto(by + 2 + i, bx + 2);
            if (idx < count) {
                if (idx == sel)
                    printf("%s", LEBUI_CLR_SELECT);
                else
                    printf("%s", LEBUI_CLR_MENU);
                if (checked[idx] == -1)
                    snprintf(line, sizeof(line), "  *  %s", items[idx]);
                else
                    snprintf(line, sizeof(line), " [%c] %s",
                             checked[idx] ? 'x' : ' ', items[idx]);
                printf(" %-*.*s ", bw - 8, bw - 8, line);
            } else {
                printf("%s%-*s", LEBUI_CLR_MENU, bw - 6, "");
            }
        }

        printf("%s", LEBUI_CLR_NORMAL);
        lebui_flush();

        key = lebui_read_key();
        if (key == LEBUI_KEY_UP && sel > 0) sel--;
        else if (key == LEBUI_KEY_DOWN && sel < count - 1) sel++;
        else if (key == ' ') {
            if (checked[sel] != -1)
                checked[sel] = !checked[sel];
        } else if (key == LEBUI_KEY_ENTER) return 0;
        else if (key == LEBUI_KEY_ESC) return -1;
    }
}
