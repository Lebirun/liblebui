#include <stdio.h>
#include <string.h>
#include "lebui.h"
#include "internal.h"

static const char **g_tabbar_tabs = (const char **)0;
static int g_tabbar_count = 0;
static int g_tabbar_active = 0;

void lebui_tabbar_attach(const char **tabs, int count, int active, int cols) {
    (void)cols;
    if (!tabs || count <= 0) {
        lebui_tabbar_detach();
        return;
    }
    g_tabbar_tabs = tabs;
    g_tabbar_count = count;
    if (active < 0) active = 0;
    if (active >= count) active = count - 1;
    g_tabbar_active = active;
}

void lebui_tabbar_detach(void) {
    g_tabbar_tabs = (const char **)0;
    g_tabbar_count = 0;
}

int lebui_internal_top_content_row(void) {
    return (g_tabbar_tabs && g_tabbar_count > 0) ? 2 : 1;
}

void lebui_draw_titlebar(const char *title, int cols) {
    int i;
    int len;

    if (g_tabbar_tabs && g_tabbar_count > 0) {
        lebui_draw_tabbar(g_tabbar_tabs, g_tabbar_count, g_tabbar_active, cols);
        return;
    }

    if (cols <= 0) return;
    if (!title) return;
    lebui_goto(1, 1);
    printf("%s", LEBUI_CLR_BAR);
    len = (int)strlen(title);
    printf("%.*s", cols, title);
    if (len > cols) len = cols;
    for (i = len; i < cols; i++) putchar(' ');
}

void lebui_draw_tabbar(const char **tabs, int count, int active, int cols) {
    int i;
    int len;
    int used;
    const char *hint;
    int hlen;

    if (cols <= 0) return;
    if (!tabs || count <= 0) {
        lebui_goto(1, 1);
        printf("%s", LEBUI_CLR_TITLE);
        for (i = 0; i < cols; i++) putchar(' ');
        return;
    }
    if (active < 0) active = 0;
    if (active >= count) active = count - 1;
    lebui_goto(1, 1);
    printf("%s", LEBUI_CLR_TITLE);
    for (i = 0; i < cols; i++) putchar(' ');
    lebui_goto(1, 1);
    printf("%s", LEBUI_CLR_TITLE);
    used = 0;
    for (i = 0; i < count; i++) {
        if (!tabs[i]) continue;
        len = (int)strlen(tabs[i]);
        if (used + len + 5 > cols) break;
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
    int len;

    if (row <= 0 || cols <= 0) return;
    if (!text) text = "";
    lebui_goto(row, 1);
    printf("%s", LEBUI_CLR_BAR);
    lebui_clear_line();
    len = (int)strlen(text);
    if (len >= cols) len = cols - 1;
    if (len > 0) printf("%.*s", len, text);
}

void lebui_draw_box_shadow(int y, int x, int h, int w, const char *title) {
    int r;
    int i;

    if (h < 2 || w < 2) return;
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
    if (rows <= 0 || cols <= 0) return;
    lebui_fill_bg(LEBUI_CLR_MENU, rows, cols);
    lebui_draw_titlebar(titlebar, cols);
    lebui_draw_helpbar(helpbar, rows, cols);
}
