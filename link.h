#ifndef LINK_H
#define LINK_H

#include <stdbool.h>

// USB serial link to the ESP32 Wi-Fi board, one text line per message:
//   calc -> board:  MESSAGE TEXT\n
//   board -> calc:  R:reply text\n   or   E:error text\n

void link_init(void);
void link_end(void);

// Call often: handles USB plug/unplug events
void link_update(void);

bool link_connected(void);
bool link_send_line(const char *s);

// Returns a full line from the board (without the newline), or NULL
const char *link_read_line(void);

#endif
