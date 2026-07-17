#include <stdio.h>
#include <string.h>
#include "lebui.h"

void lebui_fill_bg(const char *color, int rows, int cols) {
    int r;

    (void)cols;
    if (!color || rows <= 0) return;
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

    if (h < 2 || w < 2) return;
    lebui_goto(y, x);
    printf("%s+", LEBUI_CLR_BORDER);
    for (i = 0; i < w - 2; i++) putchar('-');
    putchar('+');

    if (title && w > 4) {
        tw = (int)strlen(title);
        if (tw > w - 4) tw = w - 4;
        lebui_goto(y, x + (w - tw - 2) / 2);
        printf("%s %.*s ", LEBUI_CLR_TITLE, tw, title);
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

    if (len <= 0) return;
    lebui_goto(row, col);
    for (i = 0; i < len; i++) putchar(ch);
}

void lebui_center_text(int row, int bx, int bw, const char *color, const char *text) {
    int len;
    int col;

    if (!color || !text || bw <= 2) return;
    len = (int)strlen(text);
    if (len > bw - 2) len = bw - 2;
    col = bx + (bw - len) / 2;
    if (col < bx + 1) col = bx + 1;
    lebui_goto(row, col);
    printf("%s%.*s", color, len, text);
}


