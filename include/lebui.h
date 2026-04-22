#ifndef LEBUI_H
#define LEBUI_H

#include <stdint.h>

#define LEBUI_KEY_UP      1000
#define LEBUI_KEY_DOWN    1001
#define LEBUI_KEY_LEFT    1002
#define LEBUI_KEY_RIGHT   1003
#define LEBUI_KEY_HOME    1004
#define LEBUI_KEY_END     1005
#define LEBUI_KEY_PGUP    1006
#define LEBUI_KEY_PGDN    1007
#define LEBUI_KEY_INSERT  1008
#define LEBUI_KEY_DELETE  1009
#define LEBUI_KEY_ENTER   1010
#define LEBUI_KEY_ESC     1011
#define LEBUI_KEY_TAB     1012
#define LEBUI_KEY_BKSP    1013
#define LEBUI_KEY_F1      1020
#define LEBUI_KEY_F2      1021
#define LEBUI_KEY_F3      1022
#define LEBUI_KEY_F4      1023
#define LEBUI_KEY_F5      1024
#define LEBUI_KEY_F6      1025
#define LEBUI_KEY_F7      1026
#define LEBUI_KEY_F8      1027
#define LEBUI_KEY_F9      1028
#define LEBUI_KEY_F10     1029
#define LEBUI_KEY_F11     1030
#define LEBUI_KEY_F12     1031

#define LEBUI_CLR_NORMAL   "\033[0m"
#define LEBUI_CLR_BOLD     "\033[1m"
#define LEBUI_CLR_REVERSE  "\033[7m"

#define LEBUI_CLR_TITLE    "\033[1;33;40m"
#define LEBUI_CLR_MENU     "\033[0;37;44m"
#define LEBUI_CLR_SELECT   "\033[0;34;47m"
#define LEBUI_CLR_BORDER   "\033[1;37;44m"
#define LEBUI_CLR_INPUT    "\033[0;30;47m"
#define LEBUI_CLR_BTN      "\033[1;37;44m"
#define LEBUI_CLR_BTN_SEL  "\033[0;30;47m"
#define LEBUI_CLR_BAR      "\033[0;30;46m"
#define LEBUI_CLR_PROG     "\033[1;37;42m"
#define LEBUI_CLR_PROG_BG  "\033[0;37;40m"
#define LEBUI_CLR_ERR      "\033[1;31;47m"
#define LEBUI_CLR_OK       "\033[1;32;44m"
#define LEBUI_CLR_DIM_CLR  "\033[0;36;44m"
#define LEBUI_CLR_SHADOW   "\033[0;30;40m"

#define LEBUI_PROG_LOG_MAX 8

typedef struct {
    int rows;
    int cols;
} lebui_size_t;

typedef struct {
    int drawn;
    int bx;
    int by;
    int bw;
    int log_y;
    int log_h;
    int log_count;
    char log_lines[LEBUI_PROG_LOG_MAX][64];
} lebui_prog_state_t;

void lebui_raw_enable(void);
void lebui_raw_disable(void);

int lebui_read_key(void);

void lebui_get_size(lebui_size_t *s);
void lebui_goto(int row, int col);
void lebui_clear(void);
void lebui_clear_line(void);
void lebui_hide_cursor(void);
void lebui_show_cursor(void);
void lebui_write(const char *s, int len);
void lebui_puts(const char *s);
void lebui_flush(void);

void lebui_fill_bg(const char *color, int rows, int cols);
void lebui_draw_box(int y, int x, int h, int w, const char *title);
void lebui_draw_box_shadow(int y, int x, int h, int w, const char *title);
void lebui_draw_hline(int row, int col, int len, char ch);
void lebui_center_text(int row, int bx, int bw, const char *color, const char *text);

void lebui_draw_titlebar(const char *title, int cols);
void lebui_draw_tabbar(const char **tabs, int count, int active, int cols);
void lebui_tabbar_attach(const char **tabs, int count, int active, int cols);
void lebui_tabbar_detach(void);
void lebui_draw_helpbar(const char *text, int row, int cols);
void lebui_draw_screen(const char *titlebar, const char *helpbar, int rows, int cols);

int lebui_menu(const char *title, const char **items, int count,
               int start_sel, int y, int x, int h, int w);
int lebui_menu_auto(const char *title, const char **items, int count,
                    const char *helpbar, int rows, int cols);
int lebui_input(const char *title, char *buf, int bufsz,
                int y, int x, int w);
int lebui_input_ex(const char *title, const char *prompt, char *buf,
                   int maxlen, int hidden, int rows, int cols);
int lebui_confirm(const char *title, const char *message,
                  int y, int x, int w);
int lebui_confirm_auto(const char *title, const char *message,
                       int rows, int cols);
void lebui_msgbox(const char *title, const char *message,
                  int y, int x, int w);
void lebui_msgbox_auto(const char *title, const char *message,
                       int rows, int cols);
void lebui_progress(const char *label, int percent,
                    int y, int x, int w);
void lebui_progress_init(lebui_prog_state_t *st, const char *title,
                         int rows, int cols);
void lebui_progress_update(lebui_prog_state_t *st, const char *msg, int pct);
void lebui_progress_log(lebui_prog_state_t *st, const char *msg);
void lebui_progress_reset(lebui_prog_state_t *st);
int lebui_checklist(const char *title, const char **items, int *checked,
                    int count, int y, int x, int h, int w);
int lebui_checklist_auto(const char *title, const char **items, int *checked,
                         int count, const char *helpbar, int rows, int cols);

#endif
