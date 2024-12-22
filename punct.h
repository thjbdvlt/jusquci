#ifndef PUNCT_H
#define PUNCT_H
#include "parser.h"

/* these function do not modify the TParser directly, but returns
 * the length of the substring (emoticon, parentheses, ...) if a
 * substring is found.
 */
int
is_intrapar_start(TParser* pst, jchar opening);
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
#endif
