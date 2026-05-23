#ifndef PARSER_H
#define PARSER_H

// jchar == pg_wchar == Py_UCS4 == wchar_t
typedef unsigned int jchar;

// the current token informations
typedef struct
{
  int index;
  int length;
  int byte_index;
  int byte_length;
  int line_number;
  int kind;
} Token;

// the parser struct holds informations about string to be parsed,
// state (position) and current token.
typedef struct
{
  // the whole string to parse
  jchar* str; // the string to be parsed
  int strlen; // the length of the string
  int pos;    // the current position
  int line_number;

  // for char*.
  // (not used by 'get_token', only for the Postgres extension)
  char* _str;
  int _mb; // multibytes characters (difference)
  int _pos;
  int _len;

  // sometimes, the type of the next token is known in advance.
  // used by the parser for cases like 'penses-tu' (penses -tu).
  int next;
  int prev;

  // the current token
  Token token;

} TParser;

// main functions
int
get_token(TParser* pst);
void
init_parser(TParser* pst, jchar* str, int len);
TParser
new_parser();

// token types identifiers
#define TS_ANY -2
#define TS_START -1
#define TS_END 0
#define TS_SPACE 1
#define TS_WORD 2
#define TS_COMPOUND 3
#define TS_PUNCTSTRONG 4
#define TS_PUNCT 5
#define TS_NUMBER 6
#define TS_URL 7
#define TS_CITEKEY 8
#define TS_EMOTICON 9
#define TS_EMOJI 10
#define TS_ABBREV 11
#define TS_CTRL 12
#define TS_ORDINAL 13
#define TS_NEWLINE 14
#define TS_SPACESIGN 15
#define TS_LASTNUM 15

#endif
