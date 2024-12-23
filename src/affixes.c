#include "affixes.h"
#include "util.h"
#include <wchar.h>
#include <wctype.h>

/* "-rice-s", "-rice-x", "-rice-x-s" */
const suffix suff_plural_s = {
  U"s",
  1,
  0,
  NULL,
};
const suffix* const suff_plural[] = {
  &suff_plural_s,
};
const suffix suff_nonbinary_s = {
  U"x",
  1,
  1,
  suff_plural,
};
const suffix* const suff_nonbinary[] = {
  &suff_nonbinary_s,
};
const suffix* const suff_plural_nonbinary[] = {
  &suff_plural_s,
  &suff_nonbinary_s,
};

/* feminine suffixes */
const suffix suff_feminine[] = {
#define N_SUFF_FEMININE 15
#define xs 2, suff_plural_nonbinary
  { U"e", 1, xs },
  { U"te", 2, xs },
  { U"euse", 4, xs },
  { U"ese", 3, xs },
  { U"ère", 3, xs },
  { U"ice", 3, xs },
  { U"rice", 4, xs },
  { U"trice", 5, xs },
  { U"ale", 3, xs },
  { U"ne", 2, xs },
  { U"ive", 3, xs },
  { U"esse", 4, xs },
  { U"oresse", 6, xs },
  { U"se", 2, xs },
  { U"fe", 2, xs },
#undef xs
};

/* "peut-on", "arrivons-nous", "prends-les" */
const suffix inversion[] = {
#define N_WORD_INVERSION 28
#define no 0, NULL
  { U"je", 2, no },
  { U"là", 2, no },
  { U"ci", 2, no },
  { U"t", 1, no },
  { U"m", 1, no },
  { U"tu", 2, no },
  { U"on", 2, no },
  { U"nous", 4, no },
  { U"vous", 4, no },
  { U"elle", 4, no },
  { U"il", 2, no },
  { U"ils", 3, no },
  { U"elles", 5, no },
  { U"iel", 3, no },
  { U"iels", 4, no },
  { U"moi", 3, no },
  { U"toi", 3, no },
  { U"lui", 3, no },
  { U"leur", 4, no },
  { U"eux", 3, no },
  { U"elleux", 6, no },
  { U"en", 2, no },
  { U"ce", 2, no },
  { U"y", 1, no },
  { U"la", 2, no },
  { U"les", 3, no },
  { U"le", 2, no },
  { U"ici", 3, no },
#undef no
};

/* very minimalistic support for abbreviations. these ones are very
 * common ones (ex., tél., env.) or ones relative to literature
 * (because i know those). i don't include, e.g. "art.", "vol.":
 * because those are words!
 */
const prefixes abbrev[] = {
#define N_ABBREV 20
#define LEN_ABBREV_MAX 4
  { NULL, 0 },
  { U"ch", 2 },
  { U"ph", 2 },
  { U"al", 2 },
  { U"auj", 3 },
  { U"chap", 4 },
  { U"cit", 3 },
  { U"dir", 3 },
  { U"éd", 2 },
  { U"ed", 2 },
  { U"env", 3 },
  { U"ex", 2 },
  { U"fig", 3 },
  { U"hab", 3 },
  { U"maj", 3 },
  { U"pp", 2 },
  { U"tel", 3 },
  { U"tél", 3 },
  { U"dr", 2 },
  { U"mme", 2 },
  { U"mr", 2 },
  { NULL, 0 },
};

/* compare a string with a suffix */
jchar*
match_suff(jchar* str, const suffix* suffix, int max, jchar sep)
{

  int i;
  jchar* endptr;
  jchar c;
  jchar* x;

  /* if there is not enough space for the suffix, it does not match
   */
  if (max < suffix->len)
    return NULL;

  i = suffix->len;

  /* ensure all the characters from the suffixes are in the string
   */
  for (i = 0; i < suffix->len; i++) {
    if (towlower(str[i]) != suffix->str[i])
      return NULL;
  }

  /* pointer to the end of the string */
  endptr = &str[i];

  /* if the string reached the end, it matches */
  if ((max - i) == 0)
    return endptr;

  /* else, get the character class of the next characters. */
  c = str[i];

  /* if the next character is not a a word character, then it's a
   * match. and if it's not the separator, the match don't go
   * forward, so the function can end. */
  if (!iswordch(c) && c != sep)
    return endptr;

  /* if the next char is the suffix separator, then the match could
   * continue. if it doesn't, it still a match.
   */
  else if (c == sep)
    i++;

  /* if it's a word character, the match could continue, too. but if
   * it does not, it's not a match at all.
   */
  else
    endptr = NULL;

  /* try to match every optional suffix. */
  for (int iopt = 0; iopt < suffix->n_opts; iopt++) {

    /* recursive call, like in "auteur-rice-x-s". */
    x = match_suff(&str[i], suffix->opts[iopt], (max - i), sep);
    if (x)
      return x;
  }

  return endptr;
}

/* penses-tu */
int
is_inversion(TParser* pst)
{
  jchar* p = &pst->str[pst->pos];
  jchar* x = NULL;
  int remain = (pst->strlen - pst->pos) - 1;

  for (int i = 0; i < N_WORD_INVERSION; i++) {

    x = match_suff(p + 1, &inversion[i], remain, L'-');

    if (x) {
      return 1;
    }
  }

  return 0;
}

/* p. ex. */
int
is_abbrev(TParser* pst)
{
  size_t len = (size_t)(pst->pos - pst->tidx);
  jchar c = pst->str[pst->pos - 1];
  jchar* p = &pst->str[pst->tidx];

  if (len == 1) {
    switch (c) {
      case L'a':
      case L'à':
      case L'x':
      case L'y':
        return 0;
      default:
        return 1;
    }
  }

  if (len > LEN_ABBREV_MAX) {
    return 0;
  }

  for (int i = 1; i < N_ABBREV; i++) {
    if (abbrev[i].len == len && cmpi(p, abbrev[i].str, len)) {
      return 1;
    }
  }

  return 0;
}

/* enseignant.e */
int
is_incl_suff(TParser* pst, jchar sep)
{
  jchar* cur = &pst->str[pst->pos];
  jchar* x = NULL;
  int remain = (pst->strlen - pst->pos) - 1;

  for (int i = 0; i < N_SUFF_FEMININE; i++) {

    x = match_suff(cur + 1, &suff_feminine[i], remain, sep);

    if (x) {
      return (int)(x - cur);
    }
  }
  return 0;
}
