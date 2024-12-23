#ifndef UTIL_H
#define UTIL_H

#include "parser.h"
#include <stdlib.h>

/* character types */
enum CharType
{
  Ch_Eof = 0,
  Ch_Word = TS_WORD,
  Ch_Digit = TS_NUMBER,
  Ch_PunctEndSent = TS_PUNCTSTRONG,
  Ch_Punct = TS_PUNCT,
  Ch_Space = TS_SPACESIGN,
  Ch_CiteKeyChar = TS_CITEKEY,
  Ch_Ctrl = 10,
};

// function to get the character type (word character, punct, ...)
int
iswordch(jchar c);
int
getchtype(jchar c);

// compare two string. (the first string is lowercased.)
size_t
cmpi(jchar* s_anycase, const jchar* s_lowercase, size_t len);

size_t
cmpiany(jchar* s,
  const jchar* const* array,
  size_t len,
  size_t arraylen);

#endif
