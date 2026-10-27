#ifndef TITLE_H
#define TITLE_H

typedef enum {
    MENU_CAMERA,
    MENU_CLAUDE,
    MENU_QUIT
} menu_choice_t;

// Welcome screen with the CAMERA and CLAUDE options side by side
menu_choice_t title_screen(void);

#endif
