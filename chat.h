#ifndef CHAT_H
#define CHAT_H

// Chat with Claude through the ESP32 Wi-Fi board.
//   letter keys (the green letters)  type
//   [0]      space
//   [(-)]    ?
//   [del]    delete the last letter
//   [enter]  send
//   [up] [down]  scroll back through the chat
//   [clear]  back to the welcome screen

void chat_init(void);
void chat_screen(void);

#endif
