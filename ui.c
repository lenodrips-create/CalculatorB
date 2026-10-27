#include <graphx.h>
#include <ti/getcsc.h>

#include "link.h"
#include "ui.h"

// os_GetCSC key code -> letter (same layout as the green ALPHA letters)
static const char key_chars[] =
    "\0\0\0\0\0\0\0\0\0\0\0WRMH\0\0?\0VQLG\0\0\0ZUPKFC\0 YTOJEB\0\0XSNIDA";

void ui_init(void)
{
    gfx_Begin();

    gfx_palette[COL_BG] = gfx_RGBTo1555(0, 0, 0);
    gfx_palette[COL_FG] = gfx_RGBTo1555(150, 150, 150);

    gfx_SetColor(COL_FG);
    gfx_SetTextFGColor(COL_FG);
    gfx_SetTextBGColor(COL_TRANSP);
    gfx_SetTextTransparentColor(COL_TRANSP);
    gfx_SetMonospaceFont(8);
    gfx_SetDrawBuffer();
}

void ui_end(void)
{
    gfx_End();
}

void print_centered(const char *s, int y)
{
    gfx_PrintStringXY(s, (GFX_LCD_WIDTH - gfx_GetStringWidth(s)) / 2, y);
}

void draw_box(int x, int y, int w, int h, bool selected)
{
    gfx_Rectangle(x, y, w, h);
    if (selected) {
        gfx_Rectangle(x + 2, y + 2, w - 4, h - 4);
    }
}

uint8_t get_key(void)
{
    uint8_t key;
    while (!(key = os_GetCSC())) {
        link_update();
    }
    return key;
}

char key_to_char(uint8_t key)
{
    return key < sizeof key_chars ? key_chars[key] : '\0';
}
