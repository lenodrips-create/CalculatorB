// welcome ~. for the TI-84 Plus CE (eZ80, 320x240 color screen)
// Build with the CE C toolchain (CEdev): run `make` in this folder.
//
// Welcome screen:
//   [left] [right]  pick CAMERA or CLAUDE
//   [enter]         open it
//   [clear]         quit back to the home screen
//
// Files:
//   ui.c      colors, text helpers, keypad
//   link.c    USB serial link to the ESP32 Wi-Fi board
//   title.c   welcome screen with the two options
//   camera.c  8-bit camera viewfinder
//   chat.c    chat with Claude

#include "camera.h"
#include "chat.h"
#include "link.h"
#include "title.h"
#include "ui.h"

int main(void)
{
    ui_init();
    link_init();
    chat_init();

    for (;;) {
        menu_choice_t choice = title_screen();

        if (choice == MENU_QUIT) {
            break;
        }
        if (choice == MENU_CAMERA) {
            camera_screen();
        } else {
            chat_screen();
        }
    }

    link_end();
    ui_end();
    return 0;
}
