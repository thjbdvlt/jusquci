#include "punct.h"
#include "util.h"
#include <wctype.h>

/* :happy: */
int
is_emoji(TParser* pst)
{
  jchar c;
  jchar* p = &pst->str[pst->pos];
  int remain = pst->strlen - pst->pos;

  if (pst->strlen - pst->pos > 2) {

    for (int i = 1; i < remain; i++) {
      c = p[i];

      if (c == L':') {
        return i + 1;
      }

      if (!iswalnum(c) && c != L'_')
        break;
    }
  }

  return 0;
}

/* ^^ */
int
is_emoticon_super(TParser* pst)
{
  jchar* p = &pst->str[pst->pos];
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
  jchar* p = &pst->str[pst->pos];
  jchar c = p[0];
  int i = 0;
  while (i < remain && p[i] == c)
    i++;
  if (remain - i && p[i] == '>')
    return i;
  return i - 1;
}

/* (socio)anthropologique */
int
is_intrapar_start(TParser* pst, jchar opening)
{
  int startpos = pst->pos;
  int i = startpos;
  jchar c;
  jchar closing = (opening == '(') ? ')' : ']';

  while (++i < pst->strlen) {
    c = pst->str[i];

    /* it's an intraword parenthese. update current position */
    if (c == closing && i < pst->strlen &&
        iswordch(pst->str[++i])) {
      pst->pos = i;
      return 1;
    }

    /* no, it's not an intraword parenthese, abort */
    else if (!iswordch(c) && c != '-')
      break;
  }

  /* if it's not an intraword parenthese, do not change anything */
  return 0;
}

/* ô.ô */
int
is_face_emoticon(TParser* pst)
{
  jchar* str = &pst->str[pst->pos];
  int remain = pst->strlen - pst->pos;

  if (remain < 3)
    return 0;

  if (towlower(str[0]) != towlower(str[2]) ||
      (str[1] != '.' && str[1] != '_'))
    return 0;

  if (remain == 3 || !iswalpha(str[3]))
    return 3; /* o.o, x_x */

  return 0;
}

/* :-) */
int
is_side_emoticon(TParser* pst, int eyesfirst)
{
  jchar* p = &pst->str[pst->pos];
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
      !iswalpha(p[emolen])) {
    return emolen;
  }

  return 0;
}
