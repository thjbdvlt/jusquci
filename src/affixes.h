#ifndef AFFIXES_H
#define AFFIXES_H

#include "parser.h"
#include <stdlib.h>

/* structs for suffixes */
typedef struct Suffixes
{
  const jchar* str; /* string */
  int len;          /* length */
  int n_opts;       /* number of optional suffixes */

  /* self reference (-rice-x-s) */
  const struct Suffixes* const* opts;

} suffix;

/* prefixes is simplier, because have no optional parts. */
typedef struct Prefixes
{
  jchar* str;
  size_t len;
} prefixes;

/* match a suffix */
jchar*
match_suff(jchar* str, const suffix* suffix, int max, jchar sep);

/* specific matching */
int
is_incl_suff(TParser* pst, jchar sep);
int
is_inversion(TParser* pst);
int
is_abbrev(TParser* pst);

#endif
