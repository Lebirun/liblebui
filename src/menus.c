#include <stdio.h>
#include <string.h>
#include "lebui.h"
#include "internal.h"

int lebui_menu(const char *title, const char **items, int count,
               int start_sel, int y, int x, int h, int w) {
    int sel;
    int top;
    int visible;
    int i;
    int key;
    int inner_h;
    int idx;
    const char *item;

    if (!items || count <= 0 || h < 3 || w < 4) return LEBUI_RESULT_CANCEL;
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
            idx = top + i;
            lebui_goto(y + 1 + i, x + 1);
            if (idx == sel) {
                printf("%s", LEBUI_CLR_SELECT);
            } else {
                printf("%s", LEBUI_CLR_MENU);
            }
            item = items[idx] ? items[idx] : "";
            printf(" %-*.*s", w - 3, w - 3, item);
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
        else if (key == LEBUI_KEY_ESC || key == 'q' || key == 'Q') return LEBUI_RESULT_CANCEL;
    }
}

int lebui_checklist(const char *title, const char **items, int *checked,
                    int count, int y, int x, int h, int w) {
    int sel;
    int top;
    int visible;
    int inner_h;
    int i;
    int key;
    int idx;
    const char *item;

    if (!items || !checked || count <= 0 || h < 3 || w < 7)
        return LEBUI_RESULT_CANCEL;
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
            idx = top + i;
            lebui_goto(y + 1 + i, x + 1);
            if (idx == sel) {
                printf("%s", LEBUI_CLR_SELECT);
            } else {
                printf("%s", LEBUI_CLR_MENU);
            }
            item = items[idx] ? items[idx] : "";
            printf(" [%c] %-*.*s", checked[idx] ? 'X' : ' ', w - 7,
                   w - 7, item);
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
        else if (key == ' ') checked[sel] = !checked[sel];
        else if (key == LEBUI_KEY_ENTER) return LEBUI_RESULT_OK;
        else if (key == LEBUI_KEY_ESC || key == 'q' || key == 'Q')
            return LEBUI_RESULT_CANCEL;
    }
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
    int idx;
    const char *item;

    if (!items || count <= 0 || rows < 10 || cols < 16)
        return LEBUI_RESULT_CANCEL;
    if (!title) title = "";
    maxw = (int)strlen(title) + 4;
    for (i = 0; i < count; i++) {
        item = items[i] ? items[i] : "";
        len = (int)strlen(item);
        if (len + 8 > maxw) maxw = len + 8;
    }
    bw = maxw + 4;
    if (bw > cols - 4) bw = cols - 4;
    bh = count + 4;
    if (bh > rows - 4) bh = rows - 4;
    bx = (cols - bw) / 2 + 1;
    by = (rows - bh) / 2;
    if (by < lebui_internal_top_content_row() + 2)
        by = lebui_internal_top_content_row() + 2;
    if (by + bh >= rows) return LEBUI_RESULT_CANCEL;
    view_h = bh - 4;

    sel = 0;
    scroll = 0;

    lebui_draw_screen(title, helpbar, rows, cols);
    lebui_draw_box_shadow(by, bx, bh, bw, title);

    for (;;) {
        if (sel < scroll) scroll = sel;
        if (sel >= scroll + view_h) scroll = sel - view_h + 1;

        for (i = 0; i < view_h; i++) {
            idx = scroll + i;
            lebui_goto(by + 2 + i, bx + 2);
            if (idx < count) {
                if (idx == sel)
                    printf("%s", LEBUI_CLR_SELECT);
                else
                    printf("%s", LEBUI_CLR_MENU);
                item = items[idx] ? items[idx] : "";
                printf(" %-*.*s ", bw - 8, bw - 8, item);
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
        else if (key == LEBUI_KEY_PGUP) {
            sel -= view_h;
            if (sel < 0) sel = 0;
        } else if (key == LEBUI_KEY_PGDN) {
            sel += view_h;
            if (sel >= count) sel = count - 1;
        } else if (key == LEBUI_KEY_HOME) sel = 0;
        else if (key == LEBUI_KEY_END) sel = count - 1;
        else if (key == LEBUI_KEY_ENTER) return sel;
        else if (key == LEBUI_KEY_TAB) return LEBUI_KEY_TAB;
        else if (key == LEBUI_KEY_ESC || key == 'q' || key == 'Q')
            return LEBUI_RESULT_CANCEL;
    }
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
    int idx;
    const char *item;

    if (!items || !checked || count <= 0 || rows < 10 || cols < 18)
        return LEBUI_RESULT_CANCEL;
    if (!title) title = "";
    sel = 0;
    scroll = 0;
    maxw = (int)strlen(title) + 4;
    for (i = 0; i < count; i++) {
        item = items[i] ? items[i] : "";
        len = (int)strlen(item) + 4;
        if (len + 8 > maxw) maxw = len + 8;
    }
    bw = maxw + 4;
    if (bw > cols - 4) bw = cols - 4;
    bh = count + 4;
    if (bh > rows - 4) bh = rows - 4;
    bx = (cols - bw) / 2 + 1;
    by = (rows - bh) / 2;
    if (by < lebui_internal_top_content_row() + 2)
        by = lebui_internal_top_content_row() + 2;
    if (by + bh >= rows) return LEBUI_RESULT_CANCEL;
    view_h = bh - 4;

    lebui_draw_screen(title, helpbar, rows, cols);
    lebui_draw_box_shadow(by, bx, bh, bw, title);

    for (;;) {
        if (sel < scroll) scroll = sel;
        if (sel >= scroll + view_h) scroll = sel - view_h + 1;

        for (i = 0; i < view_h; i++) {
            idx = scroll + i;
            lebui_goto(by + 2 + i, bx + 2);
            if (idx < count) {
                if (idx == sel)
                    printf("%s", LEBUI_CLR_SELECT);
                else
                    printf("%s", LEBUI_CLR_MENU);
                item = items[idx] ? items[idx] : "";
                if (checked[idx] == -1)
                    printf("   *  %-*.*s ", bw - 13, bw - 13, item);
                else
                    printf("  [%c] %-*.*s ", checked[idx] ? 'x' : ' ',
                           bw - 14, bw - 14, item);
            } else {
                printf("%s%-*s", LEBUI_CLR_MENU, bw - 6, "");
            }
        }

        printf("%s", LEBUI_CLR_NORMAL);
        lebui_flush();

        key = lebui_read_key();
        if (key == LEBUI_KEY_UP && sel > 0) sel--;
        else if (key == LEBUI_KEY_DOWN && sel < count - 1) sel++;
        else if (key == LEBUI_KEY_PGUP) {
            sel -= view_h;
            if (sel < 0) sel = 0;
        } else if (key == LEBUI_KEY_PGDN) {
            sel += view_h;
            if (sel >= count) sel = count - 1;
        } else if (key == LEBUI_KEY_HOME) sel = 0;
        else if (key == LEBUI_KEY_END) sel = count - 1;
        else if (key == ' ') {
            if (checked[sel] != -1)
                checked[sel] = !checked[sel];
        } else if (key == LEBUI_KEY_ENTER) return LEBUI_RESULT_OK;
        else if (key == LEBUI_KEY_ESC || key == 'q' || key == 'Q')
            return LEBUI_RESULT_CANCEL;
    }
}
