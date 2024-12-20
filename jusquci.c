#include "postgres.h"

#include "parser.h"

#include "fmgr.h"
#include "tsearch/ts_public.h"
#include "mb/pg_wchar.h"

PG_FUNCTION_INFO_V1(jusquci_parser_start);
PG_FUNCTION_INFO_V1(jusquci_parser_end);
PG_FUNCTION_INFO_V1(jusquci_parser_gettoken);
PG_FUNCTION_INFO_V1(jusquci_parser_lextype);

PG_MODULE_MAGIC;

Datum
jusquci_parser_start(PG_FUNCTION_ARGS)
{
  TParser* pst;
  size_t len;
  char* _str;
  wchar_t* str;

  // allocate memory for parser
  pst = (TParser*)palloc0(sizeof(TParser));

  // get pointer to the text
  _str = (char*)PG_GETARG_POINTER(0);

  // set multbytes characters count to 0
  pst->_mb = 0;

  // get the length of the text to parse
  // (add +1 for the terminating string.)
  len = (size_t)PG_GETARG_INT32(1);

  // convert to wide char.
  str = (wchar_t*)palloc0(sizeof(wchar_t*) * len);

  // convert multbytes to wide char string, and get the length
  // here is the problem. for, e.g. "et?" it's correct (3), but for
  // "et»" it's wrong (4); but for "ôlî" its correct, too (3).
  // mbstowcs(pst->str, pst->_str, len);
  len = (size_t)pg_mb2wchar_with_len(_str, (pg_wchar*)str, (int)len);

  // pst->strlen = (int)len;
  init_parser(pst, str, (int)len);
  pst->_str = _str;

  PG_RETURN_POINTER(pst);
}

Datum
jusquci_parser_end(PG_FUNCTION_ARGS)
{
  // free memory allocated for parser: there is nothing else to do
  TParser* pst = (TParser*)PG_GETARG_POINTER(0);
  free_parser(pst); // TODO: pfree(), pinit_parser() using palloc
  pfree(pst->str);
  pfree(pst);
  PG_RETURN_VOID();
}

Datum
jusquci_parser_gettoken(PG_FUNCTION_ARGS)
{
  // the text parser
  TParser* pst = (TParser*)PG_GETARG_POINTER(0);

  // the pointers to store the token index and length
  char** t;
  int* tlen;

  // token len ,type and index
  int ttype;
  int idx;
  int len;

  // position in the string
  char* p;
  char* startpos;

  // get the next token type; its length and index are stored
  // within the parser.
  ttype = get_token(pst);

  // end of string, end of parsing
  if (ttype == TS_END)
    PG_RETURN_INT32(TS_END);

  // get the pointer where to write the token start position
  t = (char**)PG_GETARG_POINTER(1);

  // get the pointer where to write the token length
  tlen = (int*)PG_GETARG_POINTER(2);

  idx = pst->tidx + pst->_mb;
  len = pst->tlen;

  p = &pst->_str[idx];
  startpos = p;

  for (int i=0; i<pst->tlen; i++) {
    int mb = pg_mblen(p+i) - 1;
    if (mb > 0) {
      p+=mb;
      pst->_mb+=mb;
      len += mb;
    }
  }

  *t = startpos;
  *tlen = len;

  // return its type
  PG_RETURN_INT32(ttype);
}

// token types names
static const char* const tok_alias[] = {
  "",
  "space",
  "word",
  "compound",
  "punct1",
  "punct2",
  "number",
  "url",
  "citekey",
  "emoticon",
  "emoji",
  "abbrev",
  "ctrl",
  "ordinal",
  "newline",
  "space1",
  NULL,
};

// token type descriptions
static const char* const lex_descr[] = {
  "",
  "a single simple space.",
  "a word.",
  "brise-glace, peut-être",
  "strong punctuation (any sequence of '.', '?' or '!')",
  "any other punctuation sign.",
  "a number",
  "anything that starts with 'www' or 'http'.",
  ":-) ô.ô ^^",
  ":happy:",
  "@becker1982 or @_12xZle",
  "'A.', 'éd.', 'pp.'",
  "control key",
  "122ème 1ère",
  "\\n",
  "any other space (subsequent space, tab, ...)",
  NULL,
};


Datum
jusquci_parser_lextype(PG_FUNCTION_ARGS)
{
  // copy-pasted from postgresql source code (`wparser_def.c`).
  LexDescr* descr =
    (LexDescr*)palloc0(sizeof(LexDescr) * (TS_LASTNUM + 1));
  int i;
  for (i = 1; i <= TS_LASTNUM; i++) {
    descr[i - 1].lexid = i;
    descr[i - 1].alias = pstrdup(tok_alias[i]);
    descr[i - 1].descr = pstrdup(lex_descr[i]);
  }
  descr[TS_LASTNUM].lexid = 0;
  PG_RETURN_POINTER(descr);
}
