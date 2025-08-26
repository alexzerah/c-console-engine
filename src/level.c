#include "level.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <limits.h>
#include <stddef.h>

bool level_load(const char *path,
                Level *lvl,
                int *start_px, int *start_py,
                int *ex, int *ey,
                Coin **coins, int *coin_count)
{
  if (!path || !lvl)
    return false;

  // Lecture brute en mémoire
  FILE *f = fopen(path, "rb");
  if (!f)
    return false;
  fseek(f, 0, SEEK_END);
  long len = ftell(f);
  fseek(f, 0, SEEK_SET);
  if (len <= 0)
  {
    fclose(f);
    return false;
  }

  char *buf = (char *)malloc((size_t)len + 1);
  if (!buf)
  {
    fclose(f);
    return false;
  }
  fread(buf, 1, (size_t)len, f);
  fclose(f);
  buf[len] = '\0';

  // Découpage en lignes
  int width = 0, height = 0;
  for (char *p = buf; *p;)
  {
    char *nl = strchr(p, '\n');
    int w = (int)(nl ? (nl - p) : strlen(p));
    if (w > width)
      width = w;
    height++;
    if (!nl)
      break;
    p = nl + 1;
  }

  lvl->width = width;
  lvl->height = height;
  lvl->tiles = (char *)malloc((size_t)width * (size_t)height);
  if (!lvl->tiles)
  {
    free(buf);
    return false;
  }

  int px = 1, py = 1;     // défaut si pas de '@'
  int enx = -1, eny = -1; // pas d'ennemi par défaut

  // Première passe : remplir tiles + compter coins
  int count = 0;
  {
    int y = 0;
    char *p = buf;
    while (y < height && *p)
    {
      char *nl = strchr(p, '\n');
      int w = (int)(nl ? (nl - p) : (int)strlen(p));
      for (int x = 0; x < width; ++x)
      {
        char c = (x < w ? p[x] : ' ');
        if (c == '@')
        {
          px = x;
          py = y;
          c = ' ';
        }
        else if (c == 'E')
        {
          enx = x;
          eny = y;
          c = ' ';
        }
        else if (c == '*')
        {
          count++;
        }
        lvl->tiles[(size_t)y * (size_t)width + (size_t)x] = c;
      }
      if (!nl)
        break;
      p = nl + 1;
      y++;
    }
    // Si le fichier finit sans newline, y est correct; sinon les lignes restantes sont déjà vides
  }

  // Deuxième passe : extraire positions des coins
  Coin *arr = NULL;
  if (count > 0)
  {
    arr = (Coin *)malloc((size_t)count * sizeof(Coin));
    if (!arr)
    {
      free(buf);
      free(lvl->tiles);
      return false;
    }
    int idx = 0;
    for (int y = 0; y < height; ++y)
    {
      for (int x = 0; x < width; ++x)
      {
        char c = lvl->tiles[(size_t)y * (size_t)width + (size_t)x];
        // ATTENTION: on a retiré les '*' en 1ère passe ? Non → on les a laissés.
        if (c == '*')
        {
          arr[idx++] = (Coin){.x = x, .y = y};
          // On laisse '*' dans tiles pour le rendu passif ou on l’efface ?
          // Ici: on l’efface pour n’afficher que via la logique gameplay.
          lvl->tiles[(size_t)y * (size_t)width + (size_t)x] = ' ';
        }
      }
    }
  }

  free(buf);

  if (start_px)
    *start_px = px;
  if (start_py)
    *start_py = py;
  if (ex)
    *ex = enx;
  if (ey)
    *ey = eny;
  if (coins)
    *coins = arr;
  if (coin_count)
    *coin_count = count;

  return true;
}

void level_free(Level *lvl)
{
  if (!lvl)
    return;
  free(lvl->tiles);
  lvl->tiles = NULL;
  lvl->width = lvl->height = 0;
}
