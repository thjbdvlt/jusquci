#include "../parser.h"
#include <wctype.h>
#include <wchar.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

#define BASE_SIZE 256

void
tokenize_print(TParser* pst, jchar* text, int len)
{
  // init or re-init parser
  init_parser(pst, text, len);

  int ttype = TS_START;

  do {
    // get next token
    ttype = get_token(pst);

    // only print words
    if (ttype != TS_SPACE && ttype != TS_END) {

      // iterate over the chars of the token
      for (int c = 0; c < pst->tlen; c++)
        putwchar((wchar_t)pst->str[pst->tidx + c]);

      putwchar(L' ');

      // add a newline after strong punctuation
      if (ttype == TS_PUNCTSTRONG)
        putwchar(L'\n');
    }

  } while (ttype != TS_END);

  // add a newline
  putwchar(L'\n');
}

int
main(int argc, char** argv)
{

  setlocale(LC_CTYPE, ""); // wide char support

  TParser pst;
  init_parser(&pst, NULL, 0);
  wint_t c;
  size_t index = 0;

  // allocate memory to read from stdin
  size_t size = BASE_SIZE;
  wchar_t* str = malloc(size * sizeof(wchar_t*));
  if (!str)
    return 1;

  while ((c = getwchar()) != WEOF) {

    // reallocate memory if needed
    while (index >= size) {
      size *= 2;
      wchar_t* temp = realloc(str, size * sizeof(wchar_t*));
      if (!temp) {
        fputs("(memory error.)", stderr);
        free(str);
        return 1;
      }
      str = temp;
    }

    // parse newline per newline
    if (c == '\n') {
      tokenize_print(&pst, (jchar*)str, (int)index);
      index = 0;
      continue;

      // add the character to the string
    } else {
      str[index] = (wchar_t)c;
      index++;
    }
  }

  free(str);
  // free_parser(&pst);

  return 0;
}
