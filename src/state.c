#include "state.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

typedef struct
{
  uint32_t magic;
  uint32_t version;
  uint32_t size;     // taille du payload en octets
  uint32_t checksum; // fletcher32 du payload
} StateHeader;

/* Fletcher-32 sur bytes */
static uint32_t fletcher32(const uint8_t *data, size_t len)
{
  uint32_t sum1 = 0xffff, sum2 = 0xffff;
  while (len)
  {
    size_t tlen = len > 360 ? 360 : len;
    len -= tlen;
    for (size_t i = 0; i < tlen; ++i)
    {
      sum1 += data[i];
      sum2 += sum1;
    }
    sum1 = (sum1 & 0xffff) + (sum1 >> 16);
    sum2 = (sum2 & 0xffff) + (sum2 >> 16);
    data += tlen;
  }
  sum1 = (sum1 & 0xffff) + (sum1 >> 16);
  sum2 = (sum2 & 0xffff) + (sum2 >> 16);
  return (sum2 << 16) | sum1;
}

static void ensure_dir(const char *dir)
{
  struct stat st;
  if (stat(dir, &st) == -1)
  {
    if (mkdir(dir, 0755) == -1 && errno != EEXIST)
    {
      /* best effort, silencieux */
    }
  }
}

bool state_save(const char *path, const StatePayload *p)
{
  if (!p)
    return false;

  ensure_dir("saves");

  StateHeader h;
  h.magic = CE_STATE_MAGIC;
  h.version = CE_STATE_VERSION;
  h.size = (uint32_t)sizeof(StatePayload);
  h.checksum = fletcher32((const uint8_t *)p, sizeof(StatePayload));

  FILE *f = fopen(path, "wb");
  if (!f)
    return false;

  bool ok = fwrite(&h, sizeof h, 1, f) == 1 && fwrite(p, sizeof *p, 1, f) == 1;

  fclose(f);
  return ok;
}

bool state_load(const char *path, StatePayload *out)
{
  if (!out)
    return false;

  FILE *f = fopen(path, "rb");
  if (!f)
    return false;

  StateHeader h;
  StatePayload p;
  bool ok = fread(&h, sizeof h, 1, f) == 1 && fread(&p, sizeof p, 1, f) == 1;
  fclose(f);
  if (!ok)
    return false;

  if (h.magic != CE_STATE_MAGIC)
    return false;
  if (h.version != CE_STATE_VERSION)
    return false;
  if (h.size != sizeof(StatePayload))
    return false;

  uint32_t chk = fletcher32((const uint8_t *)&p, sizeof p);
  if (chk != h.checksum)
    return false;

  *out = p;
  return true;
}
