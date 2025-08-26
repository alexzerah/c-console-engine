#ifndef CE_GAME_H
#define CE_GAME_H

#include <stddef.h>
#include <stdbool.h>

typedef struct
{
  int width;
  int height;
  char *buf; // tampon 2D linéarisé (width * height)
  bool cursor_hidden;
} Game;

// Cycle de vie
bool game_init(Game *g, int width, int height);
void game_shutdown(Game *g);

// Dessin
void game_clear(Game *g, char fill);
void game_draw_border(Game *g);
void game_put(Game *g, int x, int y, char c);

// Rendu terminal (ANSI)
void game_begin_frame(void);
void game_end_frame(void);
void game_render(const Game *g);

#endif // CE_GAME_H
