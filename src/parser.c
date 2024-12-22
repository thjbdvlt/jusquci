/* jusquci -- french tokenizer. */

#include "affixes.h"
#include "parser.h"
#include "punct.h"
#include "util.h"
#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>

/* these functions modify TParser values (especially `pos`) and
 * returns the token type (word, url, digit, ordinal, ...).
 */
int
parse_word(TParser* pst);
int
parse_url(TParser* pst, jchar c);
void
parse_citekey(TParser* pst);
int
parse_digit(TParser* pst);

/* initialize values for a parser. */
void
init_parser(TParser* pst, jchar* str, int len)
{
  /* string's informations */
  pst->str = str;
  pst->strlen = len;

  /* start at the beginning of the string. */
  pst->pos = 0;

  /* token informations */
  pst->tidx = 0;
  pst->tlen = 0;
  pst->ttype = TS_START;

  /* special cases when next token's type is already known. */
  pst->_next = TS_START;
  pst->_prev = TS_START;
}

int
parse_word(TParser* pst)
{
  jchar c = pst->str[pst->pos];
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
        if (!iswalpha(pst->str[pst->pos + 1])) {
          return TS_WORD;
        }
        /* penses-tu */
        else if (is_inversion(pst)) {
          pst->_next = TS_WORD;
          return TS_WORD;
        }
        break;

      /* dots */
      case '.':
        /* auteur.rice */
        if ((len = is_incl_suff(pst, c))) {
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
        if (pst->strlen - pst->pos &&
            iswalpha(pst->str[pst->pos + 1])) {
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
  jchar c;

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

/* enseignant.e */
/* www.on-tenk.com */
int
parse_url(TParser* pst, jchar c)
{
  jchar* x = &pst->str[pst->pos];

  if (pst->strlen - pst->pos < 4)
    return 0;

  if (cmpi((c == L'h') ? U"http" : U"www.", x, 4) == 0) {
    do {
      pst->pos++;
      c = pst->str[pst->pos];
    } while (
      pst->pos < pst->strlen && (!iswspace(c) && !iswcntrl(c)));
    return 1;
  }

  return 0;
}

#define N_SUFF_ORD 4
const jchar* const suff_ord[] = {
  U"ère",
  U"ème",
  U"er",
  U"e",
};

int
parse_digit(TParser* pst)
{
  jchar c;
  int remain = (pst->strlen - pst->pos) - 1;
  int tryord = 1; /* start at 1, cause number starts with digit */
  size_t lenord = 0;

  int i = pst->pos + 1;
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
      case 'k':
      case 'x':
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

  lenord = cmpiany(
    &pst->str[pst->pos], suff_ord, (size_t)remain, N_SUFF_ORD);
  if (lenord) {
    pst->pos += lenord;
    return TS_ORDINAL;
  }

  return TS_NUMBER;
}

int
get_token(TParser* pst)
{
  jchar c;
  int chtype; /* character type */
  int ttype;  /* token type */
  int tlen;

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
