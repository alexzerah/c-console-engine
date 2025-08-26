#include "game.h"
#include <curses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static inline size_t idx(const Game *g, int x, int y)
{
  return (size_t)y * (size_t)g->width + (size_t)x;
}

bool game_init(Game *g, int w, int h)
{
  if (w < 8 || h < 6)
    return false;
  g->width = w;
  g->height = h;
  g->buf = (char *)malloc((size_t)w * (size_t)h);
  if (!g->buf)
    return false;
  g->cursor_hidden = true;
  game_clear(g, ' ');
  return true;
}

void game_shutdown(Game *g)
{
  if (!g)
    return;
  free(g->buf);
  g->buf = NULL;
  g->cursor_hidden = false;
}

void game_clear(Game *g, char fill)
{
  memset(g->buf, (unsigned char)fill, (size_t)g->width * (size_t)g->height);
}

void game_draw_border(Game *g)
{
  game_put(g, 0, 0, '+');
  game_put(g, g->width - 1, 0, '+');
  game_put(g, 0, g->height - 1, '+');
  game_put(g, g->width - 1, g->height - 1, '+');
  for (int x = 1; x < g->width - 1; ++x)
  {
    game_put(g, x, 0, '-');
    game_put(g, x, g->height - 1, '-');
  }
  for (int y = 1; y < g->height - 1; ++y)
  {
    game_put(g, 0, y, '|');
    game_put(g, g->width - 1, y, '|');
  }
}

void game_put(Game *g, int x, int y, char c)
{
  if ((unsigned)x >= (unsigned)g->width || (unsigned)y >= (unsigned)g->height)
    return;
  g->buf[idx(g, x, y)] = c;
}

void game_begin_frame(void) { /* ncurses effacera à render */ }
void game_end_frame(void) { usleep(16 * 1000); }

void game_render(const Game *g)
{
  static bool init_colors = false;
  if (!init_colors && has_colors())
  {
    start_color();
    use_default_colors();
    init_pair(1, COLOR_CYAN, -1);   // murs '#'
    init_pair(2, COLOR_GREEN, -1);  // joueur '@'
    init_pair(3, COLOR_RED, -1);    // ennemi 'E'
    init_pair(4, COLOR_YELLOW, -1); // coin '*'
    init_pair(5, COLOR_WHITE, -1);  // bordure
    init_colors = true;
  }

  int rows, cols;
  getmaxyx(stdscr, rows, cols);
  int offY = (rows - g->height) > 0 ? (rows - g->height) / 2 : 0;
  int offX = (cols - g->width) > 0 ? (cols - g->width) / 2 : 0;

  werase(stdscr);
  for (int y = 0; y < g->height; ++y)
  {
    for (int x = 0; x < g->width; ++x)
    {
      char c = g->buf[(size_t)y * (size_t)g->width + (size_t)x];
      chtype attr = A_NORMAL;
      switch (c)
      {
      case '#':
        attr = COLOR_PAIR(1) | A_BOLD;
        break;
      case '@':
        attr = COLOR_PAIR(2) | A_BOLD;
        break;
      case 'E':
        attr = COLOR_PAIR(3) | A_BOLD;
        break;
      case '*':
        attr = COLOR_PAIR(4) | A_BOLD;
        break;
      case '+':
      case '-':
      case '|':
        attr = COLOR_PAIR(5);
        break;
      default:
        break;
      }
      if (attr != A_NORMAL)
        wattr_on(stdscr, attr, NULL);
      int yy = offY + y, xx = offX + x;
      if (yy >= 0 && yy < rows && xx >= 0 && xx < cols)
        mvwaddch(stdscr, yy, xx, c ? c : ' ');
      if (attr != A_NORMAL)
        wattr_off(stdscr, attr, NULL);
    }
  }
  wrefresh(stdscr);
}
