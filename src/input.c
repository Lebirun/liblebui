#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <lebirun.h>
#include "lebui.h"
#include "internal.h"

static int lebui_decode_sequence(const char *seq, int len) {
    int code;
    int i;

    if (len == 2 && seq[0] == 'O') {
        switch (seq[1]) {
        case 'P': return LEBUI_KEY_F1;
        case 'Q': return LEBUI_KEY_F2;
        case 'R': return LEBUI_KEY_F3;
        case 'S': return LEBUI_KEY_F4;
        case 'H': return LEBUI_KEY_HOME;
        case 'F': return LEBUI_KEY_END;
        }
    }
    if (len < 2 || seq[0] != '[') return LEBUI_KEY_ESC;
    switch (seq[len - 1]) {
    case 'A': return LEBUI_KEY_UP;
    case 'B': return LEBUI_KEY_DOWN;
    case 'C': return LEBUI_KEY_RIGHT;
    case 'D': return LEBUI_KEY_LEFT;
    case 'H': return LEBUI_KEY_HOME;
    case 'F': return LEBUI_KEY_END;
    }
    if (seq[len - 1] != '~') return LEBUI_KEY_ESC;
    code = 0;
    for (i = 1; i < len - 1; i++) {
        if (seq[i] < '0' || seq[i] > '9') return LEBUI_KEY_ESC;
        code = code * 10 + seq[i] - '0';
    }
    switch (code) {
    case 1: return LEBUI_KEY_HOME;
    case 2: return LEBUI_KEY_INSERT;
    case 3: return LEBUI_KEY_DELETE;
    case 4: return LEBUI_KEY_END;
    case 5: return LEBUI_KEY_PGUP;
    case 6: return LEBUI_KEY_PGDN;
    case 7: return LEBUI_KEY_HOME;
    case 8: return LEBUI_KEY_END;
    case 11: return LEBUI_KEY_F1;
    case 12: return LEBUI_KEY_F2;
    case 13: return LEBUI_KEY_F3;
    case 14: return LEBUI_KEY_F4;
    case 15: return LEBUI_KEY_F5;
    case 17: return LEBUI_KEY_F6;
    case 18: return LEBUI_KEY_F7;
    case 19: return LEBUI_KEY_F8;
    case 20: return LEBUI_KEY_F9;
    case 21: return LEBUI_KEY_F10;
    case 23: return LEBUI_KEY_F11;
    case 24: return LEBUI_KEY_F12;
    }
    return LEBUI_KEY_ESC;
}

int lebui_read_key(void) {
    char c;
    char seq[8];
    int len;

    if (read(STDIN_FILENO, &c, 1) <= 0) return -1;

    if (c == '\033') {
        len = 0;
        if (read_nb(STDIN_FILENO, &seq[len], 1) != 1) return LEBUI_KEY_ESC;
        len++;
        if (seq[0] != '[' && seq[0] != 'O') return LEBUI_KEY_ESC;
        while (len < (int)sizeof(seq) && read(STDIN_FILENO, &seq[len], 1) == 1) {
            len++;
            if (seq[len - 1] == '~') break;
            if (len >= 2 && ((seq[len - 1] >= 'A' && seq[len - 1] <= 'Z') ||
                             (seq[len - 1] >= 'a' && seq[len - 1] <= 'z'))) break;
        }
        return lebui_decode_sequence(seq, len);
    }

    if (c == '\r' || c == '\n') return LEBUI_KEY_ENTER;
    if (c == '\t') return LEBUI_KEY_TAB;
    if (c == 127 || c == '\b') return LEBUI_KEY_BKSP;
    return (int)(unsigned char)c;
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

    if (!buf || bufsz <= 0 || w < 8) return LEBUI_RESULT_CANCEL;
    len = (int)strlen(buf);
    if (len >= bufsz) {
        len = bufsz - 1;
        buf[len] = '\0';
    }
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
            return LEBUI_RESULT_OK;
        }
        if (key == LEBUI_KEY_ESC) {
            lebui_hide_cursor();
            return LEBUI_RESULT_CANCEL;
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
    int cur;
    int start;
    int visible;

    if (!title) title = "";
    if (!prompt) prompt = "";
    if (!buf || maxlen <= 0 || rows < 10 || cols < 16)
        return LEBUI_RESULT_CANCEL;
    len = 0;
    buf[0] = '\0';
    cur = len;
    plen = (int)strlen(prompt);
    tlen = (int)strlen(title);
    bw = 50;
    if (plen + 6 > bw) bw = plen + 6;
    if (tlen + 6 > bw) bw = tlen + 6;
    if (bw > cols - 4) bw = cols - 4;
    bh = 8;
    bx = (cols - bw) / 2 + 1;
    by = (rows - bh) / 2;
    if (by < lebui_internal_top_content_row() + 2)
        by = lebui_internal_top_content_row() + 2;
    if (by + bh >= rows) return LEBUI_RESULT_CANCEL;
    fw = bw - 6;
    if (fw < 1) return LEBUI_RESULT_CANCEL;

    lebui_draw_screen(NULL, " <Enter> Confirm  <Esc> Cancel", rows, cols);
    lebui_draw_box_shadow(by, bx, bh, bw, title);

    lebui_goto(by + 2, bx + 2);
    printf("%s%.*s", LEBUI_CLR_MENU, bw - 4, prompt);

    lebui_goto(by + 6, bx + bw / 2 - 3);
    printf("%s< OK >%s", LEBUI_CLR_BTN, LEBUI_CLR_NORMAL);

    for (;;) {
        start = 0;
        if (cur >= fw) start = cur - fw + 1;
        visible = len - start;
        if (visible > fw) visible = fw;
        lebui_goto(by + 4, bx + 3);
        printf("%s", LEBUI_CLR_INPUT);
        if (hidden) {
            for (i = 0; i < visible; i++) putchar('*');
        } else {
            for (i = 0; i < visible; i++) putchar(buf[start + i]);
        }
        for (i = visible; i < fw; i++) putchar(' ');

        printf("%s", LEBUI_CLR_NORMAL);
        lebui_goto(by + 4, bx + 3 + cur - start);
        lebui_show_cursor();
        lebui_flush();

        key = lebui_read_key();
        lebui_hide_cursor();

        if (key == LEBUI_KEY_ENTER) {
            buf[len] = '\0';
            return len;
        } else if (key == LEBUI_KEY_ESC) {
            return LEBUI_RESULT_CANCEL;
        } else if (key == LEBUI_KEY_LEFT && cur > 0) {
            cur--;
        } else if (key == LEBUI_KEY_RIGHT && cur < len) {
            cur++;
        } else if (key == LEBUI_KEY_HOME) {
            cur = 0;
        } else if (key == LEBUI_KEY_END) {
            cur = len;
        } else if (key == LEBUI_KEY_BKSP && cur > 0) {
            memmove(&buf[cur - 1], &buf[cur], len - cur + 1);
            len--;
            cur--;
        } else if (key == LEBUI_KEY_DELETE && cur < len) {
            memmove(&buf[cur], &buf[cur + 1], len - cur);
            len--;
        } else if (key >= 32 && key < 127 && len < maxlen - 1) {
            memmove(&buf[cur + 1], &buf[cur], len - cur + 1);
            buf[cur] = (char)key;
            len++;
            cur++;
        }
        buf[len] = '\0';
    }
}
