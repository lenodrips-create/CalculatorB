#include <graphx.h>
#include <ti/getcsc.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "chat.h"
#include "link.h"
#include "ui.h"

#define LINE_CHARS 26   // narrower than the screen so the two sides read apart
#define MAX_LINES  48
#define MAX_INPUT  34

#define LOG_TOP    30
#define LOG_BOTTOM 198
#define LINE_STEP  14   // 8px text + 6px between lines
#define MSG_GAP    8    // extra space between messages

#define REPLY_TIMEOUT (90 * CLOCKS_PER_SEC)

typedef struct {
    char text[LINE_CHARS + 1];
    bool right;   // your messages sit on the right, Claude's on the left
    bool first;   // first line of a message, gets MSG_GAP above it
} line_t;

static line_t lines[MAX_LINES];
static uint8_t num_lines;
static uint8_t scroll;        // lines scrolled up from the newest
static char input[MAX_INPUT + 1];
static uint8_t input_len;
static bool waiting;          // a message was sent and the reply has not arrived
static clock_t sent_at;

static void push_line(const char *s, size_t len, bool right, bool first)
{
    if (num_lines == MAX_LINES) {
        memmove(&lines[0], &lines[1], sizeof lines[0] * (MAX_LINES - 1));
        num_lines--;
    }
    memcpy(lines[num_lines].text, s, len);
    lines[num_lines].text[len] = '\0';
    lines[num_lines].right = right;
    lines[num_lines].first = first;
    num_lines++;
}

// Word-wrap msg into the chat log on one side
static void add_message(bool right, const char *msg)
{
    const char *p = msg;
    size_t n = strlen(p);
    bool first = true;

    while (n > LINE_CHARS) {
        size_t cut = LINE_CHARS;
        while (cut > 0 && p[cut] != ' ') {
            cut--;
        }
        if (cut == 0) {
            cut = LINE_CHARS;
        }
        push_line(p, cut, right, first);
        first = false;
        p += cut;
        n -= cut;
        while (*p == ' ') {
            p++;
            n--;
        }
    }
    push_line(p, n, right, first);
    scroll = 0;
}

// Returns true if a full reply arrived
static bool poll_reply(void)
{
    const char *line = link_read_line();

    if (line && (line[0] == 'R' || line[0] == 'E') && line[1] == ':') {
        add_message(false, line + 2);
        waiting = false;
        return true;
    }
    return false;
}

static void send_message(void)
{
    add_message(true, input);

    if (link_send_line(input)) {
        waiting = true;
        sent_at = clock();
    } else {
        add_message(false, "NO WI-FI BOARD FOUND. CHECK ITS USB CABLE.");
    }

    input_len = 0;
    input[0] = '\0';
}

static void draw_chat(void)
{
    gfx_FillScreen(COL_BG);

    print_centered(link_connected() ? "CLAUDE" : "CLAUDE - NOT CONNECTED", 8);
    gfx_HorizLine(4, 22, GFX_LCD_WIDTH - 8);

    // Newest line at the bottom; older lines scroll off the top
    int y = LOG_BOTTOM - 8;
    if (waiting && scroll == 0) {
        gfx_PrintStringXY("...", 8, y);
        y -= LINE_STEP + MSG_GAP;
    }
    for (int i = num_lines - 1 - scroll; i >= 0 && y >= LOG_TOP; i--) {
        int x = 8;
        if (lines[i].right) {
            x = GFX_LCD_WIDTH - 8 - (int)strlen(lines[i].text) * 8;
        }
        gfx_PrintStringXY(lines[i].text, x, y);
        y -= LINE_STEP;
        if (lines[i].first) {
            y -= MSG_GAP;
        }
    }

    gfx_Rectangle(4, 206, GFX_LCD_WIDTH - 8, 26);
    gfx_PrintStringXY("> ", 10, 215);
    gfx_PrintString(input);
    gfx_PrintChar('_');

    gfx_SwapDraw();
}

void chat_init(void)
{
    add_message(false, "TYPE A MESSAGE WITH THE GREEN LETTER KEYS. 0 IS A SPACE.");
}

void chat_screen(void)
{
    bool dirty = true;
    bool was_connected = link_connected();

    input_len = 0;
    input[0] = '\0';

    for (;;) {
        if (dirty) {
            draw_chat();
            dirty = false;
        }

        link_update();
        if (link_connected() != was_connected) {
            was_connected = link_connected();
            dirty = true;
        }
        if (poll_reply()) {
            dirty = true;
        }
        if (waiting && clock() - sent_at > REPLY_TIMEOUT) {
            waiting = false;
            add_message(false, "NO ANSWER FROM THE BOARD. TRY AGAIN.");
            dirty = true;
        }

        uint8_t key = os_GetCSC();
        if (!key) {
            continue;
        }
        dirty = true;

        if (key == sk_Clear) {
            return;
        }
        if (key == sk_Up) {
            if (scroll + 1 < num_lines) {
                scroll++;
            }
        } else if (key == sk_Down) {
            if (scroll > 0) {
                scroll--;
            }
        } else if (key == sk_Del) {
            if (input_len > 0) {
                input[--input_len] = '\0';
            }
        } else if (key == sk_Enter) {
            if (input_len > 0 && !waiting) {
                send_message();
            }
        } else if (key_to_char(key) && input_len < MAX_INPUT) {
            input[input_len++] = key_to_char(key);
            input[input_len] = '\0';
        }
    }
}
