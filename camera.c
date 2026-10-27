#include <graphx.h>
#include <ti/getcsc.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "camera.h"
#include "link.h"
#include "ui.h"

// Viewfinder frame
#define VF_X 20
#define VF_Y 26
#define VF_W 280
#define VF_H 176

#define CORNER       14
#define NOISE_DOTS   300
#define SCAN_TICKS   (CLOCKS_PER_SEC / 60)   // scan line moves 60px per second
#define FLASH_TICKS  (CLOCKS_PER_SEC / 8)

static void draw_corners(void)
{
    int x0 = VF_X, y0 = VF_Y;
    int x1 = VF_X + VF_W - 1, y1 = VF_Y + VF_H - 1;

    gfx_HorizLine(x0, y0, CORNER);
    gfx_VertLine(x0, y0, CORNER);
    gfx_HorizLine(x1 - CORNER + 1, y0, CORNER);
    gfx_VertLine(x1, y0, CORNER);
    gfx_HorizLine(x0, y1, CORNER);
    gfx_VertLine(x0, y1 - CORNER + 1, CORNER);
    gfx_HorizLine(x1 - CORNER + 1, y1, CORNER);
    gfx_VertLine(x1, y1 - CORNER + 1, CORNER);
}

static void draw_crosshair(void)
{
    int cx = VF_X + VF_W / 2, cy = VF_Y + VF_H / 2;

    gfx_HorizLine(cx - 10, cy, 7);
    gfx_HorizLine(cx + 4, cy, 7);
    gfx_VertLine(cx, cy - 10, 7);
    gfx_VertLine(cx, cy + 4, 7);
}

static void draw_frame(unsigned secs, clock_t now, unsigned photos)
{
    char buf[16];

    gfx_FillScreen(COL_BG);

    gfx_PrintStringXY("8-BIT CAM", 8, 8);
    if (secs % 2 == 0) {
        gfx_FillCircle(GFX_LCD_WIDTH - 44, 11, 3);
    }
    gfx_PrintStringXY("REC", GFX_LCD_WIDTH - 32, 8);

    draw_corners();
    for (int i = 0; i < NOISE_DOTS; i++) {
        gfx_SetPixel(VF_X + rand() % VF_W, VF_Y + rand() % VF_H);
    }
    gfx_HorizLine(VF_X + 2, VF_Y + (int)((now / SCAN_TICKS) % VF_H), VF_W - 4);
    draw_crosshair();

    sprintf(buf, "%02u:%02u:%02u", secs / 3600, secs / 60 % 60, secs % 60);
    gfx_PrintStringXY(buf, 8, 214);
    sprintf(buf, "PHOTOS %u", photos);
    gfx_PrintStringXY(buf, GFX_LCD_WIDTH - 8 - gfx_GetStringWidth(buf), 214);

    gfx_SwapDraw();
}

void camera_screen(void)
{
    clock_t start = clock();
    clock_t flash_until = 0;
    unsigned photos = 0;

    srand(start);

    for (;;) {
        clock_t now = clock();
        uint8_t key;

        link_update();
        key = os_GetCSC();
        if (key == sk_Clear) {
            return;
        }
        if (key == sk_Enter) {
            photos++;
            flash_until = now + FLASH_TICKS;
        }

        if (now < flash_until) {
            gfx_FillScreen(COL_FG);
            gfx_SwapDraw();
        } else {
            draw_frame((unsigned)((now - start) / CLOCKS_PER_SEC), now, photos);
        }
    }
}
