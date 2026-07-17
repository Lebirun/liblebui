#include <stdio.h>
#include <string.h>
#include "lebui.h"
#include "internal.h"

int lebui_confirm(const char *title, const char *message,
                  int y, int x, int w) {
    int sel;
    int key;

    if (w < 22) return 0;
    if (!message) message = "";
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

    if (w < 8) return;
    if (!message) message = "";
    lebui_draw_box(y, x, 7, w, title);
    lebui_center_text(y + 2, x, w, LEBUI_CLR_MENU, message);
    lebui_center_text(y + 4, x, w, LEBUI_CLR_BTN_SEL, "[ OK ]");
    lebui_flush();

    for (;;) {
        key = lebui_read_key();
        if (key == LEBUI_KEY_ENTER || key == LEBUI_KEY_ESC || key == 'q') return;
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

    if (!title) title = "";
    if (!message) message = "";
    if (rows < 10 || cols < 24) return 0;
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
    if (by < lebui_internal_top_content_row() + 2)
        by = lebui_internal_top_content_row() + 2;
    if (by + bh >= rows) return 0;

    lebui_draw_screen(NULL, " <Tab> Switch  <Enter> Select", rows, cols);
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

    if (!title) title = "";
    if (!message) message = "";
    if (rows < 9 || cols < 12) return;
    mlen = (int)strlen(message);
    tlen = (int)strlen(title);
    bw = mlen + 6;
    if (tlen + 6 > bw) bw = tlen + 6;
    if (bw < 24) bw = 24;
    if (bw > cols - 4) bw = cols - 4;
    bh = 7;
    bx = (cols - bw) / 2 + 1;
    by = (rows - bh) / 2;
    if (by < lebui_internal_top_content_row() + 2)
        by = lebui_internal_top_content_row() + 2;
    if (by + bh >= rows) return;

    lebui_draw_screen(NULL, " <Enter> OK", rows, cols);
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
