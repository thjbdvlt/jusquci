/* jusquci -- french tokenizer. */

#ifdef JUSQUCI_POSTGRESQL
/* if it's used as a PostgreSQL extension, use the <postgres.h> main
 * header file, so i can use `palloc`, `repalloc` and `pfree`.
 */
#include <postgres.h>
#define MALLOC palloc
#define REALLOC repalloc
#define FREE pfree
#else
#define MALLOC malloc
#define REALLOC realloc
#define FREE free
#endif

#include "parser.h"
#include <stdlib.h>
#include <wctype.h>

/* structs for suffixes */
typedef struct Suffixes
{
  const wchar_t* str; /* string */
  int len;            /* length */
  int n_opts;         /* number of optional suffixes */
  const struct Suffixes* const*
    opts; /* self reference (-rice-x-s) */
} suffix;

/* prefixes is simplier, because it has no optional parts. */
typedef struct Prefixes
{
  wchar_t* str;
  size_t len;
} prefixes;

/* match a suffix */
wchar_t*
match_suff(wchar_t* str,
  const suffix* suffix,
  int max,
  wchar_t sep);

/* these functions modify TParser values (especially `pos`) and
 * returns the token type (word, url, digit, ordinal, ...).
 */
int
parse_word(TParser* pst);
int
parse_url(TParser* pst, wchar_t c);
void
parse_citekey(TParser* pst);
int
parse_digit(TParser* pst);

/* these function do not modify the TParser directly, but returns
 * the length of the substring (emoticon, parentheses, ...) if a
 * substring is found.
 */
int
is_incl_suff(TParser* pst, wchar_t sep);
int
is_inversion(TParser* pst);
int
is_intrapar_start(TParser* pst, wchar_t opening);
int
is_abbrev(TParser* pst);
int
is_face_emoticon(TParser* pst);
int
is_side_emoticon(TParser* pst, int eyesfirst);
int
is_emoticon_super(TParser* pst);
int
is_emoji(TParser* pst);
int
is_arrow(TParser* pst);

#ifdef JUSQUCI_TRACK
/* these functions are used to keep track of the position of dash
 * (in compound words) and inclusive language suffixes, using
 * dynamically allocated memory.
 */
int*
double_memory_intptr(int* ptr, int size);
int
track_suff(TParser* pst, int pos, int len);
int
track_comp(TParser* pst, int pos);
#define TRACK_SUFF_MAX 10
#define TRACK_COMP_MAX 10
#endif

/* re-initialize values to parse a new string with the same parser.
 */
void
reinit_parser(TParser* pst, wchar_t* str, int len)
{
  /* string informations */
  pst->str = str;
  pst->strlen = len;

  /* start at the beginning of the string. */
  pst->pos = 0;

  /* token informations */
  pst->tidx = 0;
  pst->tlen = 0;
  pst->ttype = TS_START;

  /* special cases know the type of the next token. */
  pst->_next = TS_START;
  pst->_prev = TS_START;

#ifdef JUSQUCI_TRACK
  pst->nsuff = 0;
  pst->ncomp = 0;
#endif
}

/* initialize values for a parser. */
void
init_parser(TParser* pst, wchar_t* str, int len)
{
  reinit_parser(pst, str, len);
#ifdef JUSQUCI_TRACK
  /* allocate memory for int arrays if option JUSQUCI_TRACK is set
   */

  /* inclusive suffixes */
  pst->nsuff = 0;
  pst->maxsuff = TRACK_SUFF_MAX;
  pst->suffidx = (int*)MALLOC(sizeof(int*) * TRACK_SUFF_MAX);
  pst->sufflen = (int*)MALLOC(sizeof(int*) * TRACK_SUFF_MAX);

  /* compound words */
  pst->ncomp = 0;
  pst->maxcomp = TRACK_COMP_MAX;
  pst->compsep = (int*)MALLOC(sizeof(int*) * TRACK_COMP_MAX);
#endif
}

/* free memory allocated to the parser values */
void
free_parser(TParser* pst)
{
#ifdef JUSQUCI_TRACK
  FREE(pst->suffidx);
  FREE(pst->sufflen);
  FREE(pst->compsep);
#endif
}

#ifdef JUSQUCI_TRACK
/* below are three functions only usefull for the option `track`,
 * that keep informations about compound words (number and position
 * of dashes) and inclusive writings (indexes and length of
 * suffixes).
 */

/* increase the memory allocated to a string array */
int*
double_memory_intptr(int* ptr, int size)
{
  int* tmp;
  tmp = (int*)REALLOC(ptr, sizeof(int*) * (size_t)(size));
  if (!tmp) {
    free(ptr);
    return NULL;
  }
  return tmp;
}

/* store informations about the position and length of a suffix */
int
track_suff(TParser* pst, int pos, int len)
{
  int* idx = pst->suffidx;
  int* lens = pst->sufflen;
  int n = pst->nsuff;
  int max = pst->maxsuff;

  /* reallocate memory if needed */
  if (n >= max) {
    max *= 2;
    if (!(pst->suffidx = double_memory_intptr(idx, max)) ||
        !(pst->sufflen = double_memory_intptr(lens, max)))
      return 0;
    pst->maxsuff = max;
  }

  /* append to both arrays */
  idx[n] = pos;
  lens[n] = pos + len;
  pst->nsuff++;

  return 1;
}

/* store informations about the position of a compounding dash */
int
track_comp(TParser* pst, int pos)
{
  int* idx = pst->compsep;
  int n = pst->ncomp;
  int max = pst->maxcomp;

  /* reallocate memory if needed */
  if (n >= max) {
    max *= 2;
    if (!(pst->compsep = double_memory_intptr(idx, max)))
      return 0;
    pst->maxcomp = max;
  }

  idx[n] = pos;
  pst->ncomp++;

  return 1;
}
#endif

/* "-rice-s", "-rice-x", "-rice-x-s" */
const suffix suff_plural_s = { L"s", 1, 0, NULL };
const suffix* const suff_plural[] = {
  &suff_plural_s,
};
const suffix suff_nonbinary_s = { L"x", 1, 1, suff_plural };
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
  { L"e", 1, xs },
  { L"te", 2, xs },
  { L"euse", 4, xs },
  { L"ese", 3, xs },
  { L"ère", 3, xs },
  { L"ice", 3, xs },
  { L"rice", 4, xs },
  { L"trice", 5, xs },
  { L"ale", 3, xs },
  { L"ne", 2, xs },
  { L"ive", 3, xs },
  { L"esse", 4, xs },
  { L"oresse", 6, xs },
  { L"se", 2, xs },
  { L"fe", 2, xs },
  // { U"le", 2, xs }, // would requires condition BEFORE
  // { L"Ère", 3, xs }, // was for char*
#undef xs
};

/* "peut-on", "arrivons-nous", "prends-les" */
const suffix inversion[] = {
#define N_WORD_INVERSION 28
#define no 0, NULL
  { L"je", 2, no },
  { L"là", 2, no },
  { L"ci", 2, no },
  { L"t", 1, no },
  { L"m", 1, no },
  { L"tu", 2, no },
  { L"on", 2, no },
  { L"nous", 4, no },
  { L"vous", 4, no },
  { L"elle", 4, no },
  { L"il", 2, no },
  { L"ils", 3, no },
  { L"elles", 5, no },
  { L"iel", 3, no },
  { L"iels", 4, no },
  { L"moi", 3, no },
  { L"toi", 3, no },
  { L"lui", 3, no },
  { L"leur", 4, no },
  { L"eux", 3, no },
  { L"elleux", 6, no },
  { L"en", 2, no },
  { L"ce", 2, no },
  { L"y", 1, no },
  { L"la", 2, no },
  { L"les", 3, no },
  { L"le", 2, no },
  { L"ici", 3, no },
#undef no
};

/* 122ème, XVIIIe, 1ères, 1er, 422e */
const suffix suff_ord[] = {
#define N_SUFF_ORD 4
  { L"ère", 3, 1, suff_plural },
  { L"ème", 3, 1, suff_plural },
  { L"er", 2, 1, suff_plural },
  { L"e", 1, 0, NULL },
};

/* very minimalistic support for abbreviations. these ones are very
 * common ones (ex., tél., env.) or ones relative to literature
 * (because i know those). i don't include, e.g. "art.", "vol.":
 * because those are words!
 */
const prefixes abbrev[] = {
#define N_ABBREV 17
#define LEN_ABBREV_MAX 4
  { NULL, 0 },
  { L"ch", 2 },
  { L"ph", 2 },
  { L"al", 2 },
  { L"auj", 3 },
  { L"chap", 4 },
  { L"cit", 3 },
  { L"dir", 3 },
  { L"éd", 2 },
  { L"ed", 2 },
  { L"env", 3 },
  { L"ex", 2 },
  { L"fig", 3 },
  { L"hab", 3 },
  { L"maj", 3 },
  { L"pp", 2 },
  { L"tel", 3 },
  { L"tél", 3 },
  { NULL, 0 },
};

/* character types */
enum CharType
{
  Ch_Eof = 0,
  Ch_Word = TS_WORD,
  Ch_Digit = TS_NUMBER,
  Ch_PunctStrong = TS_PUNCTSTRONG,
  Ch_PunctSoft = TS_PUNCT,
  Ch_Space = TS_SPACESIGN,
  Ch_CiteKeyChar = TS_CITEKEY,
  Ch_Ctrl = 10,
};

int
iswordch(wchar_t c)
{
  wint_t c2 = (wint_t)c;
  return (
    iswalpha(c2) || c == L'·');
}

/* classify a character */
int
getchtype(wchar_t c)
{

  wint_t c2 = (wint_t)c;
  if (iswspace(c2))
    return Ch_Space;

  if (iswalpha(c2) || c == L'·')
    return Ch_Word;

  if (iswdigit(c2))
    return Ch_Digit;

  if (c == L'@') /* @becker1982 */
    return Ch_CiteKeyChar;

  if (iswcntrl(c2))
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

/* :happy: */
int
is_emoji(TParser* pst)
{
  wchar_t c;
  wchar_t* p = &pst->str[pst->pos];
  int remain = pst->strlen - pst->pos;

  if (pst->strlen - pst->pos > 2) {

    for (int i = 1; i < remain; i++) {
      c = p[i];

      if (c == L':') {
        return i + 1;
      }

      if (!iswalnum((wint_t)c) && c != L'_')
        break;
    }
  }

  return 0;
}

/* compare a string with a suffix */
wchar_t*
match_suff(wchar_t* str, const suffix* suffix, int max, wchar_t sep)
{

  int i;
  wchar_t* endptr;
  wchar_t c;
  wchar_t* x;

  /* if there is not enough space for the suffix, it does not match
   */
  if (max < suffix->len)
    return NULL;

  i = suffix->len;

  /* ensure all the characters from the suffixes are in the string
   */
  for (i = 0; i < suffix->len; i++) {
    // TODO: there's probably a simplier way.
    // if (iswupper((wint_t)str[i]) ? (wchar_t)towlower((wint_t)str[i])
    //                              : str[i] != suffix->str[i])
    if (towlower((wint_t)str[i]) != (wint_t)suffix->str[i])
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
  wchar_t* p = &pst->str[pst->pos];
  wchar_t* x = NULL;
  int remain = (pst->strlen - pst->pos) - 1;

  for (int i = 0; i < N_WORD_INVERSION; i++) {

    x = match_suff(p + 1, &inversion[i], remain, L'-');

    if (x) {
      return 1;
    }
  }

  return 0;
}

/* enseignant.e */
int
is_incl_suff(TParser* pst, wchar_t sep)
{
  wchar_t* cur = &pst->str[pst->pos];
  wchar_t* x = NULL;
  int remain = (pst->strlen - pst->pos) - 1;

  for (int i = 0; i < N_SUFF_FEMININE; i++) {

    x = match_suff(cur + 1, &suff_feminine[i], remain, sep);

    if (x) {
      return (int)(x - cur);
    }
  }
  return 0;
}

/* www.on-tenk.com */
int
parse_url(TParser* pst, wchar_t c)
{
  wchar_t* x = &pst->str[pst->pos];
  wint_t c2;

  if (pst->strlen - pst->pos < 4)
    return 0;

  if (wcsncmp((c == L'h') ? L"http" : L"www.", x, 4) == 0) {
    do {
      pst->pos++;
      c = pst->str[pst->pos];
      c2 = (wint_t)c;
    } while (
      pst->pos < pst->strlen && (!iswspace(c2) && !iswcntrl(c2)));
    return 1;
  }

  return 0;
}

/* compare two strings. (second one must be lowercase.) */
int
cmpi(wchar_t* s_anycase, wchar_t* s_lowercase, size_t len)
{
  for (size_t i = 0; i < len; i++) {
    if (towlower((wint_t)(s_anycase[i])) != (wint_t)s_lowercase[i])
      return 0;
  }
  return 1;
}

/* p. ex. */
int
is_abbrev(TParser* pst)
{
  size_t len = (size_t)(pst->pos - pst->tidx);
  wchar_t c = pst->str[pst->pos - 1];
  wchar_t* p = &pst->str[pst->tidx];

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

/* ^^ */
int
is_emoticon_super(TParser* pst)
{
  wchar_t* p = &pst->str[pst->pos];
  if (pst->strlen - pst->pos > 1 && p[1] == '^') {
    return 2;
  }
  return 0;
}

/* ---> */
int
is_arrow(TParser* pst)
{
  int remain = (pst->strlen - pst->pos);
  wchar_t* p = &pst->str[pst->pos];
  wchar_t c = p[0];
  int i = 0;
  while (i < remain && p[i] == c)
    i++;
  if (remain - i && p[i] == '>')
    return i;
  return i - 1;
}

/* (socio)anthropologique */
int
is_intrapar_start(TParser* pst, wchar_t opening)
{
  int startpos = pst->pos;
  int i = startpos;
  wchar_t c;
  wchar_t closing = (opening == '(') ? ')' : ']';

  while (++i < pst->strlen) {
    c = pst->str[i];

    /* it's an intraword parenthese. update current position */
    if (c == closing && i < pst->strlen &&
        iswordch(pst->str[++i])) {
      pst->pos = i;
      return 1;
    }

    /* no, it's not an intraword parenthese, abort */
    else if (getchtype(c) != Ch_Word && c != '-')
      break;
  }

  /* if it's not an intraword parenthese, do not change anything */
  return 0;
}

/* ô.ô */
int
is_face_emoticon(TParser* pst)
{
  wchar_t* str = &pst->str[pst->pos];
  int remain = pst->strlen - pst->pos;

  if (remain < 3)
    return 0;

  if (towlower((wint_t)str[0]) != towlower((wint_t)str[2]) ||
      (str[1] != '.' && str[1] != '_'))
    return 0;

  if (remain == 3 || !iswalpha((wint_t)str[3]))
    return 3; /* o.o, x_x */

  return 0;
}

/* :-) */
int
is_side_emoticon(TParser* pst, int eyesfirst)
{
  wchar_t* p = &pst->str[pst->pos];
  int len = pst->strlen - pst->pos;
  int emolen;

  /* 2: eyes + mouth */
  if (len < 2)
    return 0;

  /* nose only with dash */
  emolen = (p[1] == '-') ? 2 : 1;

  /* if eyes + nose reach the end, it's not a smiley. */
  if (len < emolen + 1)
    return 0;

  /* eyes or mouth, depending of what was matched before */
  switch (p[emolen]) {
    case ':':
    case ';':
      if (eyesfirst)
        return 0;
      break;

    case 'd':
    case 'p':
    case 'D':
    case 'P':
    case 'O':
    case 'o':
    case 's':
    case '3':
    case 'S':
    case ')':
    case ']':
    case '(':
    case '[':
      if (!eyesfirst)
        return 0;
      break;

    default:
      return 0;
  }

  emolen++;

  if (pst->strlen - pst->pos == emolen ||
      !iswalpha((wint_t)p[emolen])) {
    return emolen;
  }

  return 0;
}

int
parse_word(TParser* pst)
{
  wchar_t c = pst->str[pst->pos];
  int par = 0;
  int len;

  while (pst->pos < pst->strlen) {

    c = pst->str[pst->pos];

    switch (c) {

      /* opening parentheses */
      case '(':
      case '[':
      case '{':
        par = 1;
        break;

      /* closing parentheses */
      case ')':
      case ']':
      case '}':
        if (!par)
          return TS_WORD;
        par = 0;
        break;

      case '-':
        /* depuis->là */
        if (!iswalpha((wint_t)pst->str[pst->pos + 1])) {
          return TS_WORD;
        }
        /* penses-tu */
        else if (is_inversion(pst)) {
          pst->_next = TS_WORD;
          return TS_WORD;
        }
        // TODO: no need to check for inclusive suffix
        // if not JUSQUCI_TRACK
        /* lecteur-rice-s */
        else if ((len = is_incl_suff(pst, c))) {
#ifdef JUSQUCI_TRACK
          track_suff(pst, pst->pos, len);
#endif
          pst->pos += len - 1; /* -1 because ++ after */
          break;
        }
#ifdef JUSQUCI_TRACK
        /* brise-glace */
        track_comp(pst, pst->pos);
#endif
        break;

      /* dots */
      case '.':
        /* auteur.rice */
        if ((len = is_incl_suff(pst, c))) {
#ifdef JUSQUCI_TRACK
          track_suff(pst, pst->pos, len);
#endif
          pst->pos += len - 1;
          break;
        }
        /* p. ex. */
        else if ((is_abbrev(pst))) {
          pst->pos++;
          return TS_ABBREV;
        }
        /* Adieu. */
        else {
          return TS_WORD;
        }
        break;

      /* very minimal support for "·ère": it's not check, just */
      /* assumed that it is inclusive language. */
      case L'·':
#ifdef JUSQUCI_TRACK
        /* auteur.rice */
        if ((len = is_incl_suff(pst, c))) {
          track_suff(pst, pst->pos, len);
          pst->pos += len - 1;
        } else {
          return TS_WORD;
        }
        break;
#endif
        if (pst->strlen - pst->pos &&
            iswalpha((wint_t)pst->str[pst->pos + 1])) {
          pst->pos++;
        } else {
          return TS_WORD;
        }
        break;

      /* jusqu' ici */
      case '\'':
      case L'’':
      case L'‘':
        pst->pos++;
        return TS_WORD;
        break;

      default:
        if (!iswordch(c))
          return TS_WORD;
        break;
    }

    pst->pos++;
  }

  return TS_WORD;
}

void
parse_citekey(TParser* pst)
{
  int chtype;
  wchar_t c;

  /* @becker1982, @_12xZle */
  while (pst->pos < pst->strlen) {
    c = pst->str[pst->pos];
    chtype = getchtype(c);
    if (chtype == Ch_Word || chtype == Ch_Digit || c == '_')
      pst->pos++;
    else
      break;
  }
}

int
parse_digit(TParser* pst)
{
  wchar_t* x = NULL;
  wchar_t c;
  int remain = (pst->strlen - pst->pos) - 1;
  int tryord = 1; /* start at 1, cause number starts with digit */

  int i=pst->pos+1;
  for (; i < pst->strlen; i++) {
    c = pst->str[i];
    switch (c) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        tryord = 1;
        break;
      case ')':
      case '.':
      case '-':
      case '/':
      case '^':
      case '+':
      case '=':
      case '*':
        tryord = 0;
        break;
      default:
        goto EndDigit;
        break;
    }
  }

EndDigit:
  pst->pos = i;

  /* 17ème, 1ère */
  if (!tryord)
    return TS_NUMBER;
  for (i = 0; i < N_SUFF_ORD; i++) {
    x = match_suff(&pst->str[pst->pos], &suff_ord[i], remain, ' ');
    if (x) {
      pst->pos += (int)(x - &pst->str[pst->pos]);
      return TS_ORDINAL;
    }
  }

  return TS_NUMBER;
}

int
get_token(TParser* pst)
{
  wchar_t c;
  int chtype; /* character type */
  int ttype;  /* token type */
  int tlen;

#ifdef JUSQUCI_TRACK
  pst->ncomp = 0;
  pst->nsuff = 0;
#endif

  /* reach the end */
  if (pst->pos >= pst->strlen) {
    pst->pos = pst->strlen;
    pst->tidx = pst->strlen;
    pst->tlen = 0;
    return TS_END;
  }

  c = pst->str[pst->pos]; /* current character */
  pst->tidx = pst->pos;   /* the token start index */

  switch (c) {

    /* end of string. it should not get to this point. */
    case L'\0':
      ttype = TS_END;
      chtype = Ch_Ctrl;
      goto EndToken;
      break;

    /* simple white space */
    case L' ':
      chtype = Ch_Space;
      if (pst->_prev == TS_SPACE) {
        ttype = TS_SPACESIGN;
      } else {
        ttype = TS_SPACE;
        pst->pos++;
        goto EndToken;
      }
      break;

    /* newline */
    case L'\n':
      ttype = TS_NEWLINE;
      chtype = Ch_Space;
      pst->pos++;
      goto EndToken;
      break;

    /* periodcentered is a punct sign unless it's inside a word */
    case L'·':
      ttype = TS_PUNCT;
      chtype = Ch_PunctSoft;
      pst->pos++;
      goto EndToken;
      break;

    case L':':
      chtype = Ch_PunctSoft;
      /* :happy: */
      if ((tlen = is_emoji(pst))) {
        ttype = TS_EMOJI;
        pst->pos += tlen;
        /* :-) */
      } else if ((tlen = is_side_emoticon(pst, 1))) {
        ttype = TS_EMOTICON;
        pst->pos += tlen;
        /* default usage */
      } else {
        ttype = TS_PUNCT;
        pst->pos++;
      }
      goto EndToken;
      break;

    case L'=':
      chtype = Ch_PunctSoft;
      ttype = TS_PUNCT;
      /* =) */
      if ((tlen = is_side_emoticon(pst, 1))) {
        ttype = TS_EMOTICON;
        pst->pos += tlen;
        /* ===> */
      } else if ((tlen = is_arrow(pst))) {
        pst->pos += tlen;
        goto EndToken;
        /* ici = là */
      } else {
        pst->pos++;
        goto EndToken;
      }
      break;

    case L'^':
      chtype = Ch_PunctSoft;
      /* ^^ */
      if ((tlen = is_emoticon_super(pst))) {
        ttype = TS_EMOTICON;
        pst->pos += tlen;
        /* as an simili-punctuation sign */
      } else {
        ttype = TS_PUNCT;
        pst->pos++;
      }
      goto EndToken;
      break;

    case L'x':
    case L'X':
      /* XD, x.x */
      if ((tlen = is_face_emoticon(pst)) ||
          ((tlen = is_side_emoticon(pst, 1)))) {
        ttype = TS_EMOTICON;
        chtype = Ch_PunctSoft;
        pst->pos += tlen;
        goto EndToken;
      } else {
        chtype = Ch_Word;
        ttype = TS_WORD;
      }
      break;

    case L'v':
    case L'o':
    case L'ô':
    case L'V':
    case L'O':
    case L'Ô':
      /* v.v ô.ô O_o */
      if ((tlen = is_face_emoticon(pst))) {
        ttype = TS_EMOTICON;
        chtype = Ch_PunctSoft;
        pst->pos += tlen;
        goto EndToken;
      } else {
        chtype = Ch_Word;
        ttype = TS_WORD;
      }
      break;

    case L'(':
    case L'[':
      /* (: */
      if ((tlen = is_side_emoticon(pst, 0))) {
        ttype = TS_EMOTICON;
        pst->pos += tlen;
        goto EndToken;
      }
      chtype = is_intrapar_start(pst, c) ? Ch_Word : Ch_PunctSoft;
      break;

    case L'h':
    case L'w':
      /* www.on-tenk.com */
      if (parse_url(pst, c)) {
        chtype = Ch_Word;
        ttype = TS_URL;
        goto EndToken;
      }
      ttype = TS_WORD;
      chtype = Ch_Word;
      break;

    case L'-':
      chtype = Ch_PunctSoft;
      ttype = Ch_PunctSoft;
      /* -je */
      if (pst->_next == TS_WORD) {
        pst->_next = TS_START;
        pst->pos++;
        chtype = Ch_Word;
        ttype = TS_WORD;
        /* ---> */
      } else if ((tlen = is_arrow(pst))) {
        pst->pos += tlen;
        goto EndToken;
      } else {
        pst->pos++;
        goto EndToken;
      }
      break;

    default:
      chtype = getchtype(c);
      break;
  }

  switch (chtype) {

    case Ch_Word:
      ttype = parse_word(pst);
      break;

    case Ch_Digit:
      ttype = parse_digit(pst);
      break;

    case Ch_Ctrl:
    case Ch_Space:
    case Ch_PunctStrong:
      ttype = chtype;
      while (pst->pos < pst->strlen &&
             getchtype(pst->str[pst->pos]) == chtype)
        pst->pos++;
      break;

    case Ch_CiteKeyChar:
      ttype = TS_CITEKEY;
      pst->pos++;
      parse_citekey(pst);
      break;

    case Ch_PunctSoft:
    default:
      pst->pos++;
      ttype = TS_PUNCT;
      break;
  }

EndToken:

  /* end of string */
  if (pst->pos > pst->strlen)
    pst->pos = pst->strlen;

  /* update the token informations */
  pst->tlen = pst->pos - pst->tidx;
  pst->ttype = ttype;
  pst->_prev = ttype;

  return ttype;
}
