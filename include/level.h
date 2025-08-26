#ifndef CE_LEVEL_H
#define CE_LEVEL_H

#include <stdbool.h>
#include <stddef.h> // ← nécessaire pour size_t

typedef struct
{
  int x, y;
} Coin;

typedef struct
{
  int width;
  int height;
  char *tiles; // width*height, accès via y*width + x
} Level;

/* Charge un niveau ASCII. Retourne:
   - lvl (tiles alloué)
   - start_px, start_py si '@' trouvé, sinon (1,1)
   - ex, ey si 'E' trouvé, sinon (-1,-1)
   - coins (malloc) + coin_count pour les '*'
*/
bool level_load(const char *path,
                Level *lvl,
                int *start_px, int *start_py,
                int *ex, int *ey,
                Coin **coins, int *coin_count);

void level_free(Level *lvl);

static inline bool level_is_wall(const Level *lvl, int x, int y)
{
  if (x < 0 || y < 0 || x >= lvl->width || y >= lvl->height)
    return true;
  return lvl->tiles[(size_t)y * (size_t)lvl->width + (size_t)x] == '#';
}

#endif // CE_LEVEL_H
