#include <wchar.h>

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

// the parser struct holds informations about string to be parsed,
// state (position) and current token.
typedef struct
{
  // the whole string to parse
  wchar_t* str; // the string to be parsed
  int strlen;   // the length of the string
  int pos;      // the current position

  // current token
  int tlen;  // length
  int tidx;  // index (first character)
  int ttype; // token type (word, space, ...)

  // for char*.
  // (not used by 'get_token', only for the Postgres extension)
  char* _str;
  int _mb; // multibytes characters (difference)
  int _pos;
  int _len;

  // sometimes, the type of the next token is known in advance.
  // used by the parser for cases like 'penses-tu' (penses -tu).
  int _next;
  int _prev;

#ifdef JUSQUCI_TRACK
  // inclusive suffixes
  int* suffidx;
  int* sufflen;
  int nsuff;
  int maxsuff;

  // compound words
  int* compsep;
  int ncomp;
  int maxcomp;
#endif

} TParser;


int get_token(TParser* pst);
void init_parser(TParser* pst, wchar_t* str, int len);
void free_parser(TParser* pst);
void reinit_parser(TParser* pst, wchar_t* str, int len);


// function to get the character type (word character, punct, ...)
int iswordch(wchar_t c);
int getchtype(wchar_t c);


// compare two string. (the first string is lowercased.)
int cmpi(wchar_t* s_anycase, wchar_t* s_lowercase, size_t len);
