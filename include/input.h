#ifndef CE_INPUT_H
#define CE_INPUT_H

#include <stdbool.h>

typedef enum
{
  KEY_NONE = 0,
  KEY_UP,
  KEY_DOWN,
  KEY_LEFT,
  KEY_RIGHT,
  KEY_ENTER,
  KEY_SPACE,
  KEY_ESC,
  KEY_SAVE, // 'S'
  KEY_LOAD  // 'L'
} Key;

bool input_init(void);
void input_shutdown(void);
Key input_poll(void);

#endif // CE_INPUT_H
