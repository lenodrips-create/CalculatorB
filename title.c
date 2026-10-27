#include <graphx.h>
#include <ti/getcsc.h>

#include "title.h"
#include "ui.h"

#define BTN_W   100
#define BTN_H   32
#define BTN_GAP 20
#define BTN_Y   130
#define BTN_X0  ((GFX_LCD_WIDTH - 2 * BTN_W - BTN_GAP) / 2)
#define BTN_X1  (BTN_X0 + BTN_W + BTN_GAP)

static menu_choice_t selected = MENU_CAMERA;

static void draw_option(const char *label, int x, bool is_selected)
{
    draw_box(x, BTN_Y, BTN_W, BTN_H, is_selected);
    gfx_PrintStringXY(label, x + (BTN_W - gfx_GetStringWidth(label)) / 2, BTN_Y + (BTN_H - 8) / 2);
}

static void draw_title(void)
{
    gfx_FillScreen(COL_BG);

    gfx_SetTextScale(2, 2);
    print_centered("WELCOME", 30);
    print_centered("welcome ~.", 75);
    gfx_SetTextScale(1, 1);

    draw_option("CAMERA", BTN_X0, selected == MENU_CAMERA);
    draw_option("CLAUDE", BTN_X1, selected == MENU_CLAUDE);

    gfx_SwapDraw();
}

menu_choice_t title_screen(void)
{
    for (;;) {
        draw_title();

        switch (get_key()) {
            case sk_Left:
                selected = MENU_CAMERA;
                break;
            case sk_Right:
                selected = MENU_CLAUDE;
                break;
            case sk_Enter:
                return selected;
            case sk_Clear:
                return MENU_QUIT;
            default:
                break;
        }
    }
}
