#include "normalizer.h"

#define REP_DIGIT '2'
#define REP_ORD L"2ème"

int
reduce_repeated_letters(wchar_t* s, int len)
{
  wchar_t c;
  int y;
  int x;
  int i;

  for (i = len; i > 2; i--) {
    c = s[i];

    if (c == s[i - 1] && c == s[i - 2]) {
      x = i;

      while (i > 0) {
        if (s[i - 1] != c)
          break;
        i--;
      }

      for (y = i; x <= len;) {
        s[y] = s[x];
        x++;
        y++;
      }

      len = y-1; // why -1?
    }
  }
  return len;
}

int
normalize_inclusive_suffix(wchar_t* s, int suffstart, int suffend, int len)
{
  int i;
  int y;
  wchar_t c;
  // TODO: fix compound words

  if (!suffstart)
    return len;

  s[suffstart] = L'·';

  // remove optionals separators ("·rice·x·s" -> "·ricexs")
  for (i=suffstart+1, y=i; i <= suffend; i++) {
    c = s[i];
    switch (c) {
      case L'.':
      case L'-':
      case L'·':
        len--;
        break;
      default:
        s[y++] = c;
        break;
    }
  }

  // copy the end of the string
  // maybe this should be done only at the end, because if it does
  // not, the other pointers and indexes could point to nowhere!
  // it's the same for `reduce_repeated_letters`.
  // or: just keep an eye on the operation sequence.
  for (i=suffend; i<=len; i++, y++) {
    s[y] = s[i];
  }

  // TODO: compound words.
  // mmmh. maybe compound words could be done separately,
  // so that this function would not be an issue.

  return len;
}

int
normalize(TParser* pst, wchar_t* normstr, int ttype)
{
  wchar_t* p = &normstr[pst->tidx];
  wchar_t c;
  int len = pst->tlen;

  switch (ttype) {

    /* 14242ème or 13e -> 2ème */
    case TS_ORDINAL:
      wcsncpy(p, L"2ème", 4);
      return 4;

    /* 97123880012 -> 2 */
    case TS_NUMBER:
      wcsncpy(p, L"2", 1);
      return 1;

    case TS_PUNCT:
    case TS_PUNCTSTRONG:
      return reduce_repeated_letters(p, len);

    case TS_WORD:
    case TS_COMPOUND:
      break;

    default:
      return len;
  }

  /* auteur-rice-x-s -> auteur·ricexs */
  for (int s = 0; s < pst->nsuff; s++) {
    len = normalize_inclusive_suffix(
        // TEST
        // FIXME
      normstr, pst->suffidx[s], pst->sufflen[s], len);
  }

  /* quuuuuoooooiiiiiii -> quoi */
  len = reduce_repeated_letters(p, len);

  /* (pré)disai[en]t -> prédisaient */
  for (int i=0; i<len; i++) {
    c = p[i];
    switch (c) {
      case '(':
      case ')':
      case ']':
      case '[':
        break;
      case L'‘':
      case L'’':
        p[i] = L'\'';
        break;
      // noooooooo ligatures... add something, so i'm all wrong
      // in the end....
      // but i could do this later, after all!
    }
  }

  return len;
}
