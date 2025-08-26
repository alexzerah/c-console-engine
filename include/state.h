#ifndef CE_STATE_H
#define CE_STATE_H

#include <stdbool.h>
#include <stdint.h>

#define CE_STATE_MAGIC 0x43534554u /* 'CSET' */
#define CE_STATE_VERSION 1

typedef struct
{
  int px, py;
  int score;
  int lives;
  unsigned int seed;
} StatePayload;

bool state_save(const char *path, const StatePayload *p);
bool state_load(const char *path, StatePayload *out);

#endif // CE_STATE_H
