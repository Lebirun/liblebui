#include <stdio.h>
#include <string.h>
#include "lebui.h"
#include "internal.h"

void lebui_progress(const char *label, int percent,
                    int y, int x, int w) {
    int bar_w;
    int filled;
    int i;

    if (!label || w < 4) return;
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

void lebui_progress_init(lebui_prog_state_t *st, const char *title,
                         int rows, int cols) {
    int prog_h;
    int top_row;

    if (!st) return;
    st->drawn = 0;
    st->log_count = 0;
    if (!title) title = "";
    if (cols < 16) return;
    top_row = lebui_internal_top_content_row();
    if (rows < top_row + 15) return;
    st->bw = 56;
    if (st->bw > cols - 4) st->bw = cols - 4;
    prog_h = 8;
    st->log_h = 10;
    st->by = top_row + 2;
    st->log_y = st->by + prog_h;
    if (st->log_h > rows - st->log_y - 1)
        st->log_h = rows - st->log_y - 1;
    if (st->log_h < 4) st->log_h = 4;
    st->bx = (cols - st->bw) / 2 + 1;
    lebui_draw_screen(title, " Please wait...", rows, cols);
    lebui_draw_box_shadow(st->by, st->bx, prog_h, st->bw, title);
    lebui_draw_box_shadow(st->log_y, st->bx, st->log_h, st->bw, "Log");
    lebui_flush();
    st->drawn = 1;
}

void lebui_progress_update(lebui_prog_state_t *st, const char *msg, int pct) {
    int bar_w;
    int filled;
    int mw;
    int i;

    if (!st || !st->drawn) return;
    if (!msg) msg = "";
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

    if (!st || !st->drawn || !msg) return;
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
    if (!st) return;
    st->drawn = 0;
    st->log_count = 0;
}

