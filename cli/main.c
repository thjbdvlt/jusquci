#include "../src/parser.h"
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>

#include <argp.h>
struct arguments
{
  int newline;
};
const char* argp_program_version = "v0.1.0";
static char args_doc[] = "<command> [...]";
static char doc[] = "Tokenizer for french.";

static struct argp_option options[] = {
  { "newline",
    'n',
    NULL,
    0,
    "Add a newline instead of space between tokens",
    0 },
};

error_t
parse_opt(int key, char* arg, struct argp_state* state)
{
  struct arguments* arguments = state->input;
  switch (key) {
    case 'n':
      arguments->newline = 1;
      break;
    default:
      return ARGP_ERR_UNKNOWN;
  }
  return 0;
};

static struct argp // argument parsing
  argp = { options, parse_opt, args_doc, doc, NULL, NULL, NULL };

#define BASE_SIZE 256

void
tokenize_print(TParser* pst, jchar* text, int len, int newline)
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
      for (int c = 0; c < pst->token.length; c++)
        putwchar((wchar_t)pst->str[pst->token.index + c]);

      if (newline)
        putwchar(L'\n');
      else
        putwchar(L' ');
    }

  } while (ttype != TS_END);

  // add a newline
  putwchar(L'\n');
}

int
main(int argc, char** argv)
{

  setlocale(LC_CTYPE, ""); // wide char support

  struct arguments a = {
    .newline = 0,
  };
  argp_parse(&argp, argc, argv, 0, 0, &a);

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
      tokenize_print(&pst, (jchar*)str, (int)index, a.newline);
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
