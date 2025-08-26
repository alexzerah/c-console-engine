#include "input.h"
#include <curses.h>

static bool g_initialized = false;

/* Capture des codes curses, puis on renvoie nos enums */
enum
{
  CU_KEY_UP = KEY_UP,
  CU_KEY_DOWN = KEY_DOWN,
  CU_KEY_LEFT = KEY_LEFT,
  CU_KEY_RIGHT = KEY_RIGHT
};

static Key map_char_ncurses(int ch)
{
  switch (ch)
  {
  case CU_KEY_UP:
    return KEY_UP;
  case CU_KEY_DOWN:
    return KEY_DOWN;
  case CU_KEY_LEFT:
    return KEY_LEFT;
  case CU_KEY_RIGHT:
    return KEY_RIGHT;
  case 27:
    return KEY_ESC;
  case ' ':
    return KEY_SPACE;
  case '\n':
    return KEY_ENTER;
  case 'z':
  case 'Z':
  case 'w':
  case 'W':
    return KEY_UP;
  case 's':
    return KEY_DOWN;
  case 'q':
  case 'Q':
  case 'a':
  case 'A':
    return KEY_LEFT;
  case 'd':
  case 'D':
    return KEY_RIGHT;
  case 'S':
    return KEY_SAVE;
  case 'L':
    return KEY_LOAD;
  case 'k':
    return KEY_UP;
  case 'j':
    return KEY_DOWN;
  case 'h':
    return KEY_LEFT;
  case 'l':
    return KEY_RIGHT;
  default:
    return KEY_NONE;
  }
}

bool input_init(void)
{
  if (g_initialized)
    return true;
  initscr();
  keypad(stdscr, TRUE);
  cbreak();
  noecho();
  nodelay(stdscr, TRUE); // non-bloquant
  curs_set(0);
  set_escdelay(25); // aide à agréger ESC-[A/B/C/D
  g_initialized = true;
  return true;
}

void input_shutdown(void)
{
  if (!g_initialized)
    return;
  nodelay(stdscr, FALSE);
  nocbreak();
  echo();
  curs_set(1);
  endwin();
  g_initialized = false;
}

Key input_poll(void)
{
  Key last = KEY_NONE;
  for (;;)
  {
    int ch = getch();
    if (ch == ERR)
      break;
    if (ch == 27)
    { // fallback ESC-seq
      timeout(25);
      int a = getch();
      if (a == ERR)
      {
        timeout(0);
        last = KEY_ESC;
        continue;
      }
      if (a == '[')
      {
        int b = getch();
        timeout(0);
        switch (b)
        {
        case 'A':
          last = KEY_UP;
          break;
        case 'B':
          last = KEY_DOWN;
          break;
        case 'C':
          last = KEY_RIGHT;
          break;
        case 'D':
          last = KEY_LEFT;
          break;
        default:
          last = KEY_ESC;
          break;
        }
        continue;
      }
      timeout(0);
      last = KEY_ESC;
      continue;
    }
    Key k = map_char_ncurses(ch);
    if (k != KEY_NONE)
      last = k;
  }
  return last;
}
