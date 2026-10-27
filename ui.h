#ifndef UI_H
#define UI_H

#include <stdbool.h>
#include <stdint.h>

// One color: gray on black
enum {
    COL_BG = 1,
    COL_FG,
    COL_TRANSP
};

void ui_init(void);
void ui_end(void);

void print_centered(const char *s, int y);

// Gray outline box; a selected box gets a second, inner outline
void draw_box(int x, int y, int w, int h, bool selected);

// Wait for a key press, keeping the USB link alive meanwhile
uint8_t get_key(void);

// Letter for a key code (the green ALPHA letters), or 0 if none
char key_to_char(uint8_t key);

#endif
