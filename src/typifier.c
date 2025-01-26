// TODO: write this
// TODO: write tests
// TODO: test

#include "parser.h"
#include "wctype.h"

int ttypify(jchar* token, int len)
{
  TParser pst;
  int ttype;
  init_parser(&pst, token, len);

  // there's a simple exception before i call the simple `get_token(pst)`:
  // -tu, -on, ...; because these cases are parsed using `pst._prev`, and
  // in the current function, there is no `pst._prev`, so `-on` would be
  // tokenized and typified as a `TS_PUNCT` instead of `TS_WORD`.
  if (!len)
    ttype = TS_START;
  else if (len > 1 && token[0] == L'-' && iswalpha(token[1]))
    ttype = TS_WORD;
  else
    ttype = get_token(&pst);

  return ttype;
}
