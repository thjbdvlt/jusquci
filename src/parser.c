/* jusquci -- french tokenizer. */

#include "affixes.h"
#include "html.h"
#include "parser.h"
#include "punct.h"
#include "util.h"
#include <locale.h>
#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>

#include "stdio.h"

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

/* update the byte index and length before returning the token */
void
update_byte_index(TParser* pst);

TParser
new_parser()
{
  setlocale(LC_CTYPE, "");
  TParser pst;
  return pst;
}

/* initialize values for a parser. */
void
init_parser(TParser* pst, jchar* str, int len)
{
  /* string's informations */
  pst->str = str;
  pst->strlen = len;

  /* start at the beginning of the string. */
  pst->pos = 0;
  pst->line_number = 0;

  /* special cases when next token's type is already known. */
  pst->next = TS_START;
  pst->prev = TS_START;

  /* initiate byte indexes and length */
  pst->_mb = 0;

  /* initiate an empty first token */
  Token token = {
    .index = 0,
    .length = 0,
    .byte_index = 0,
    .byte_length = 0,
    .line_number = 0,
    .kind = TS_START,
  };
  pst->token = token;
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
          pst->next = TS_WORD;
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

      /* very minimal support for "·ère·s": it's not check, just assumed that
       * it is inclusive language if followed by a letter. */
      case L'·':
        if (pst->strlen - pst->pos && iswalpha(pst->str[pst->pos + 1])) {
          pst->pos++;
        } else {
          return TS_WORD;
        }
        break;

      /* n° 47 */
      case L'°':
        pst->pos++;
        return TS_ABBREV;

      /* jusqu' ici */
      case '\'':
      case L'’':
      case L'‘':
        pst->pos++;
        return TS_WORD;
        break;

      /* stop parsing word when encounter a non-word character. */
      default:
        if (!iswalpha(c))
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

/* basic URL parsing, www.on-tenk.com or http://mubi.com */
int
parse_url(TParser* pst, jchar c)
{
  jchar* x = &pst->str[pst->pos];

  if (pst->strlen - pst->pos < 4)
    return 0;

  if (cmpi((c == L'h') ? U"http" : U"www.", x, 4)) {
    do {
      pst->pos++;
      c = pst->str[pst->pos];
    } while (pst->pos < pst->strlen && (!iswspace(c) && !iswcntrl(c)));
    return 1;
  }

  return 0;
}

#define N_SUFF_ORD 5
const jchar* const suff_ord[] = {
  U"ère",
  U"ème",
  U"er",
  U"e",
  U"ᵉ",
};

int
parse_digit(TParser* pst)
{
  jchar c;
  int tryord = 1; /* start at 1, cause number starts with digit */
  size_t lenord = 0;
  int i = pst->pos + 1;
  for (; i < pst->strlen; i++) {
    c = pst->str[i];
    switch (c) {
      /* 714e, 1ère */
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
      /* include these signs in the number, like 10k or 2), but not in ordinal.
       * these don't exists: 123)ème or 412=e */
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

  lenord = cmpiany(
    &pst->str[pst->pos], suff_ord, (size_t)(pst->strlen - i), N_SUFF_ORD);
  if (lenord) {
    pst->pos += (int)lenord;
    if (pst->str[pst->pos] == L's')
      pst->pos++;
    return TS_ORDINAL;
  }

  return TS_NUMBER;
}

int
get_token(TParser* pst)
{
  jchar c;
  int chtype; /* character type */
  int kind;  /* token type */
  int length;

  /* reach the end */
  if (pst->pos >= pst->strlen) {
    pst->pos = pst->strlen;
    pst->token.index = pst->strlen;
    pst->token.length = 0;
    return TS_END;
  }

  c = pst->str[pst->pos];      /* current character */
  pst->token.index = pst->pos; /* the token start index */

  switch (c) {

    /* end of string. it should not get to this point. */
    case L'\0':
      kind = TS_END;
      chtype = Ch_Ctrl;
      goto EndToken;
      break;

    /* simple white space */
    case L' ':
      chtype = Ch_Space;
      if (pst->prev == TS_SPACE) {
        kind = TS_SPACESIGN;
      } else {
        kind = TS_SPACE;
        pst->pos++;
        goto EndToken;
      }
      break;

    /* newline */
    case L'\n':
      kind = TS_NEWLINE;
      chtype = Ch_Space;
      pst->pos++;
      pst->line_number++;
      goto EndToken;
      break;

    /* periodcentered is a punct sign unless it's inside a word */
    case L'·':
      kind = TS_PUNCT;
      chtype = Ch_Punct;
      pst->pos++;
      goto EndToken;
      break;

    case L':':
      chtype = Ch_PunctEndSent;
      /* :happy: */
      if ((length = is_emoji(pst))) {
        kind = TS_EMOJI;
        pst->pos += length;
        /* :-) */
      } else if ((length = is_side_emoticon(pst, 1))) {
        kind = TS_EMOTICON;
        pst->pos += length;
        /* default usage */
      } else {
        kind = TS_PUNCTSTRONG;
        pst->pos++;
      }
      goto EndToken;
      break;

    case L'&':
      if ((length = is_html_entity(pst))) {
        chtype = Ch_Punct;
        kind = TS_PUNCT;
        pst->pos += length;
      } else {
        chtype = Ch_Word;
        kind = TS_WORD;
        pst->pos++;
      }
      goto EndToken;
      break;

    case L';':
      /* :-) */
      chtype = Ch_PunctEndSent;
      if ((length = is_side_emoticon(pst, 1))) {
        kind = TS_EMOTICON;
        pst->pos += length;
        /* default usage */
      } else {
        kind = TS_PUNCTSTRONG;
        pst->pos++;
      }
      goto EndToken;
      break;

    case L'=':
      chtype = Ch_Punct;
      kind = TS_PUNCT;
      /* =) */
      if ((length = is_side_emoticon(pst, 1))) {
        kind = TS_EMOTICON;
        pst->pos += length;
        /* ===> */
      } else if ((length = is_arrow(pst))) {
        pst->pos += length;
        goto EndToken;
        /* ici = là */
      } else {
        pst->pos++;
        goto EndToken;
      }
      break;

    case L'^':
      chtype = Ch_Punct;
      /* ^^ */
      if ((length = is_emoticon_super(pst))) {
        kind = TS_EMOTICON;
        pst->pos += length;
        /* as an simili-punctuation sign */
      } else {
        kind = TS_PUNCT;
        pst->pos++;
      }
      goto EndToken;
      break;

    case L'x':
    case L'X':
      /* XD, x.x */
      if ((length = is_face_emoticon(pst)) ||
          ((length = is_side_emoticon(pst, 1)))) {
        kind = TS_EMOTICON;
        chtype = Ch_Punct;
        pst->pos += length;
        goto EndToken;
      } else {
        chtype = Ch_Word;
        kind = TS_WORD;
      }
      break;

    case L'v':
    case L'o':
    case L'ô':
    case L'V':
    case L'O':
    case L'Ô':
      /* v.v ô.ô O_o */
      if ((length = is_face_emoticon(pst))) {
        kind = TS_EMOTICON;
        chtype = Ch_Punct;
        pst->pos += length;
        goto EndToken;
      } else {
        chtype = Ch_Word;
        kind = TS_WORD;
      }
      break;

    case L'(':
    case L'[':
      /* (: */
      if ((length = is_side_emoticon(pst, 0))) {
        kind = TS_EMOTICON;
        pst->pos += length;
        goto EndToken;
      }
      chtype = is_intrapar_start(pst, c) ? Ch_Word : Ch_Punct;
      break;

    case L'h':
    case L'w':
      /* www.on-tenk.com */
      if (parse_url(pst, c)) {
        chtype = Ch_Word;
        kind = TS_URL;
        goto EndToken;
      }
      kind = TS_WORD;
      chtype = Ch_Word;
      break;

    case L'-':
      chtype = Ch_Punct;
      kind = Ch_Punct;
      /* -je */
      if (pst->next == TS_WORD) {
        pst->next = TS_START;
        pst->pos++;
        chtype = Ch_Word;
        kind = TS_WORD;
        /* ---> */
      } else if ((length = is_arrow(pst))) {
        pst->pos += length;
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
      kind = parse_word(pst);
      break;

    case Ch_Digit:
      kind = parse_digit(pst);
      break;

    case Ch_Ctrl:
    case Ch_Space:
    case Ch_PunctEndSent:
      kind = chtype;
      while (pst->pos < pst->strlen && getchtype(pst->str[pst->pos]) == chtype)
        pst->pos++;
      break;

    case Ch_CiteKeyChar:
      kind = TS_CITEKEY;
      pst->pos++;
      parse_citekey(pst);
      break;

    case Ch_Punct:
    default:
      pst->pos++;
      kind = TS_PUNCT;
      break;
  }

EndToken:

  /* end of string */
  if (pst->pos > pst->strlen)
    pst->pos = pst->strlen;

  /* update the token informations */
  pst->token.length = pst->pos - pst->token.index;
  pst->token.kind = kind;
  pst->token.line_number = pst->line_number;
  pst->prev = kind;

  // TODO: only optionally maintain bytes indexes
  // TODO: optimize this if possible
  update_byte_index(pst);

  return kind;
}

void
update_byte_index(TParser* pst)
{
  char buf[32];

  pst->token.byte_index += pst->token.byte_length;
  pst->token.byte_length = 0;

  jchar* ptr = &pst->str[pst->token.index];

  for (int i = 0; i < pst->token.length; i++) {
    wchar_t wc = ptr[i];
    // if wchar_t is ascii, then mb = 1, else compute it.
    // source of iswascii: https://github.com/lattera/freebsd (iswctype.c)
    // (wctomb is very slow, that's why i want to avoid it when possible.)
    pst->token.byte_length += ((wc & ~0x7F) == 0) ? 1 : wctomb(buf, wc);
  }
}
