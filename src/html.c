#include "html.h"
#include "util.h"
#include <wctype.h>

int
is_html_entity(TParser* pst)
{
  jchar c;
  int pos = pst->pos;
  int tlen = 1;
  int chtype = Ch_Word;

  // &#142;
  if (pst->str[pos+1] == L'#') {
    tlen++;
    chtype = Ch_Digit;
  }

  // &amp;
  while (pos + tlen < pst->strlen) {
    c = pst->str[pos+tlen];
    if (c == L';')
      return tlen+1;
    else if (getchtype(c) == chtype)
      tlen++;
    else
      break;
  }
  return 0;
}
