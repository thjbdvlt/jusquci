#ifndef AFFIXES_H
#define AFFIXES_H

#include "parser.h"
#include <stdlib.h>

/* structs for suffixes */
typedef struct RecAffix
{
  const jchar* str; /* string */
  int len;          /* length */
  int n_opts;       /* number of optional suffixes */

  /* self reference (-rice-x-s) */
  const struct RecAffix* const* opts;

} recaffix;

/* prefixes is simplier, because have no optional parts. */
typedef struct Affix
{
  jchar* str;
  size_t len;
} affix;

/* match a recursive affix */
jchar*
match_recaff(jchar* str, const recaffix* affix, int max, jchar sep);

/* match a simple affix */
jchar*
match_aff(jchar* str, const recaffix* affix, int max);

/* specific matching */
int
is_incl_suff(TParser* pst, jchar sep);
int
is_inversion(TParser* pst);
int
is_abbrev(TParser* pst);

#endif
