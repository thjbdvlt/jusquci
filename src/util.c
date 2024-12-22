#include "util.h"
#include "wctype.h"

int
iswordch(jchar c)
{
  return (iswalpha(c) || c == L'·');
}

/* classify a character */
int
getchtype(jchar c)
{

  if (iswspace(c))
    return Ch_Space;

  if (iswalpha(c) || c == L'·')
    return Ch_Word;

  if (iswdigit(c))
    return Ch_Digit;

  if (c == L'@') /* @becker1982 */
    return Ch_CiteKeyChar;

  if (iswcntrl(c))
    return Ch_Ctrl;

  switch (c) {
    /* strong punctuation signs end sentences */
    case L'.':
    case L'!':
    case L'?':
      return Ch_PunctStrong;
      break;

    /* soft punctuation signse are everything else */
    default:
      return Ch_PunctSoft;
      break;
  }
}

/* compare two strings. (second one must be lowercase.) */
size_t
cmpi(jchar* s_anycase, const jchar* s_lowercase, size_t len)
{
  size_t i;
  for (i = 0; i < len; i++) {
    if (towlower(s_anycase[i]) != s_lowercase[i])
      return 0;
  }
  return i;
}

size_t
cmpiany(jchar* s,
  const jchar* const* array,
  size_t len,
  size_t arraylen)
{
  for (size_t i = 0; i < arraylen; i++) {
    size_t res = cmpi(s, array[i], len);
    if (res)
      return res;
  }
  return 0;
}
