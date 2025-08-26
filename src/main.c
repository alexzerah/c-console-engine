#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "game.h"
#include "input.h"
#include "state.h"
#include "level.h"

#include <curses.h>

#define CE_VERSION "0.5.0"

/* invulnérabilité (frames) après un hit pour éviter la mort instantanée */
static int invul_frames = 30; // ~0,5 s

typedef struct
{
  int x, y;
  int alive;
} CoinLive;

static inline void draw_level(const Level *lvl, Game *g)
{
  for (int y = 0; y < lvl->height; ++y)
  {
    for (int x = 0; x < lvl->width; ++x)
    {
      char t = lvl->tiles[(size_t)y * (size_t)lvl->width + (size_t)x];
      if (t != ' ')
        game_put(g, x, y, t); // murs '#', bordure éventuelle
    }
  }
}

int main(void)
{
  const char *LEVEL_PATH = "levels/level1.txt";
  const char *SAVE_PATH = "saves/slot0.bin";

  /* Charge niveau */
  Level lvl;
  int start_px, start_py, ex, ey;
  Coin *coins_raw = NULL;
  int coins_n = 0;

  if (!level_load(LEVEL_PATH, &lvl, &start_px, &start_py, &ex, &ey, &coins_raw, &coins_n))
  {
    fputs("Failed to load level\n", stderr);
    return EXIT_FAILURE;
  }

  /* Init rendu + entrée */
  Game g;
  if (!game_init(&g, lvl.width, lvl.height))
  {
    fputs("Init failed\n", stderr);
    level_free(&lvl);
    free(coins_raw);
    return EXIT_FAILURE;
  }
  if (!input_init())
  {
    fputs("Input init failed (tty?)\n", stderr);
    game_shutdown(&g);
    level_free(&lvl);
    free(coins_raw);
    return EXIT_FAILURE;
  }

  /* État jeu */
  StatePayload st = {
      .px = start_px,
      .py = start_py,
      .score = 0,
      .lives = 3,
      .seed = (unsigned)time(NULL)};

  /* Ennemi : rebond horizontal optionnel */
  int has_enemy = (ex >= 0 && ey >= 0);
  int evx = +1;

  /* Coins vivants */
  CoinLive *coins = NULL;
  if (coins_n > 0)
  {
    coins = (CoinLive *)malloc((size_t)coins_n * sizeof(CoinLive));
    for (int i = 0; i < coins_n; ++i)
    {
      coins[i].x = coins_raw[i].x;
      coins[i].y = coins_raw[i].y;
      coins[i].alive = 1;
    }
  }
  free(coins_raw);

  /* Dash */
  int dash_cooldown = 0;
  const int DASH_COOLDOWN_FRAMES = 10;
  const int DASH_DISTANCE = 2;

  int running = 1;

  while (running)
  {
    /* -------- INPUT -------- */
    for (;;)
    {
      Key k = input_poll();
      if (k == KEY_NONE)
        break;

      int nx = st.px, ny = st.py;

      switch (k)
      {
      case KEY_UP:
        ny = st.py - 1;
        break;
      case KEY_DOWN:
        ny = st.py + 1;
        break;
      case KEY_LEFT:
        nx = st.px - 1;
        break;
      case KEY_RIGHT:
        nx = st.px + 1;
        break;

      case KEY_SPACE:
        if (dash_cooldown == 0)
        {
          /* dash horizontal : droite si libre, sinon gauche */
          int dir = +1;
          if (level_is_wall(&lvl, st.px + 1, st.py) && !level_is_wall(&lvl, st.px - 1, st.py))
          {
            dir = -1;
          }
          for (int i = 0; i < DASH_DISTANCE; ++i)
          {
            int tx = st.px + dir;
            if (level_is_wall(&lvl, tx, st.py))
              break;
            st.px = tx;
          }
          dash_cooldown = DASH_COOLDOWN_FRAMES;
        }
        break;

      case KEY_SAVE:
        (void)state_save(SAVE_PATH, &st);
        break;

      case KEY_LOAD:
      {
        StatePayload loaded;
        if (state_load(SAVE_PATH, &loaded))
        {
          st = loaded;
        }
      }
      break;

      case KEY_ESC:
        running = 0;
        break;

      default:
        break;
      }

      /* mouvement simple (si ce n’était pas un dash) */
      if (k == KEY_UP || k == KEY_DOWN || k == KEY_LEFT || k == KEY_RIGHT)
      {
        if (!level_is_wall(&lvl, nx, ny))
        {
          st.px = nx;
          st.py = ny;
        }
      }
    }

    if (dash_cooldown > 0)
      dash_cooldown--;

    /* -------- UPDATE ENNEMI -------- */
    if (has_enemy)
    {
      int nextx = ex + evx;
      if (level_is_wall(&lvl, nextx, ey))
      {
        evx = -evx;
        nextx = ex + evx;
        if (level_is_wall(&lvl, nextx, ey))
          nextx = ex; // coincé
      }
      ex = nextx;
    }

    /* Collision joueur <-> ennemi (une seule fois, avec i-frames) */
    if (has_enemy && st.px == ex && st.py == ey)
    {
      if (invul_frames == 0)
      {
        st.lives--;
        invul_frames = 30;
        if (st.lives <= 0)
          running = 0;
      }
    }
    if (invul_frames > 0)
      invul_frames--;

    /* -------- COLLECTE -------- */
    for (int i = 0; i < coins_n; ++i)
    {
      if (!coins[i].alive)
        continue;
      if (coins[i].x == st.px && coins[i].y == st.py)
      {
        coins[i].alive = 0;
        st.score += 10;
      }
    }

    /* -------- RENDER -------- */
    game_begin_frame();
    game_clear(&g, ' ');
    draw_level(&lvl, &g);
    for (int i = 0; i < coins_n; ++i)
    {
      if (coins[i].alive)
        game_put(&g, coins[i].x, coins[i].y, '*');
    }
    if (has_enemy)
      game_put(&g, ex, ey, 'E');
    game_put(&g, st.px, st.py, '@');

    game_render(&g); /* une seule fois */

    /* HUD centré sous la carte */
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    int offY = (rows - g.height) > 0 ? (rows - g.height) / 2 : 0;
    int hud_y = offY + g.height;
    if (hud_y >= rows)
      hud_y = rows - 1;
    char hud[256];
    snprintf(hud, sizeof hud,
             "console-engine %s | Score:%d  Vies:%d  [S]ave [L]oad  Espace:Dash  ESC:Quit",
             CE_VERSION, st.score, st.lives);
    int hud_len = (int)strlen(hud);
    int hud_x = (cols > hud_len) ? (cols - hud_len) / 2 : 0;
    mvprintw(hud_y, hud_x, "%s", hud);
    refresh();

    game_end_frame();
  }

  free(coins);
  input_shutdown();
  game_shutdown(&g);
  level_free(&lvl);

  printf("\nGame Over — Score final: %d\n", st.score);
  return EXIT_SUCCESS;
}
