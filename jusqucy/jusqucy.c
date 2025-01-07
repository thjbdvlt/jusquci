#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "../src/parser.h"
#include "../src/typifier.h"

static PyObject*
tokenize(PyObject* self, PyObject* arg)
{
  TParser pst;           /* parser */
  Py_ssize_t len, _len;  /* len of input string */
  int i, y, ttype;       /* for iterations */
  PyObject *input, *ret; /* input value and output values */
  PyObject *list_words, *list_types, *list_spaces,
    *list_sents; /* lists */

  /* get the parameter value */
  if (!PyArg_Parse(arg, "U:tokenize", &input)) {
    PyErr_BadArgument();
    return NULL;
  }

  /* get its length */
  if ((len = PyUnicode_GetLength(input)) == -1) {
    PyErr_BadArgument();
    return NULL;
  }

  Py_UCS4* str;
  str = PyUnicode_AsUCS4Copy(input);
  if (!str)
    return PyErr_NoMemory();

  /* the position in the result */
  i = 0;
  y = 0;
  ttype = TS_START;

  /* initialize the parser with the string to parse */
  init_parser(&pst, str, (int)len);

  /* allocate memory for temporary array of integers */
  int* spaces = (int*)malloc(sizeof(int*) * (size_t)len);
  int* idx = (int*)malloc(sizeof(int*) * (size_t)len);
  int* lens = (int*)malloc(sizeof(int*) * (size_t)len);
  int* _types = (int*)malloc(sizeof(int*) * (size_t)len + 1);

  /* ensure that memory has been allocated */
  if (!spaces || !idx || !lens || !_types) {
    PyMem_FREE(str);
    return PyErr_NoMemory();
  }

  /* get the type of the first token */
  ttype = get_token(&pst);

  /* if the list is empty, returns empty list, skip following parts.
   */
  if (ttype == TS_END)
    goto MakeLists;

  /* if the first token is a space, change its type */
  else if (ttype == TS_SPACE)
    ttype = TS_SPACESIGN;

  /* add the first element. it's important to split the first item
   * assignment and the followings, because spaces is populated by
   * `spaces[i-1]`: i do not want to access what's outside the array
   * (before 0)
   */
  spaces[0] = 0;
  idx[0] = pst.tidx;
  lens[0] = pst.tlen;

  /* types is used for two things: `ttypes` and `is_sent_start`. */
  _types[0] = TS_NEWLINE;
  int* types = &_types[1];
  types[0] = ttype;

  /* start at one */
  i = 1;

  /* iterates over the tokens, until it reach the end of the
   * tokenized text. standard spaces are not added to the list, but
   * rather modified the other list `spaces`, which indicate if a
   * token is FOLLOWED by a space (hence `spaces[i-1]`).
   */
  while ((ttype = get_token(&pst)) != TS_END) {
    if (ttype == TS_SPACE) {
      spaces[i - 1] = 1;
    } else {
      spaces[i] = 0;
      idx[i] = pst.tidx;
      lens[i] = pst.tlen;
      types[i] = ttype;
      i++;
    }
  }

MakeLists:

  /* make the python objects: three lists.*/
  list_words = PyList_New(i);
  list_types = PyList_New(i);
  list_spaces = PyList_New(i);
  list_sents = PyList_New(i);

  int* sents = (int*)malloc(sizeof(int*) * (size_t)i + 1);
  sents[i + 1] = 0;

  if (!list_words || !list_types || !list_words || !list_sents) {
    ret = PyErr_NoMemory();
    Py_XDECREF(list_types);
    Py_XDECREF(list_words);
    Py_XDECREF(list_spaces);
    Py_XDECREF(list_sents);
    goto FreeEnd;
  }

  /* populate the lists */
  int isword = 0;
  for (y = 0; y < i; y++) {
    PyObject* word = PyUnicode_FromKindAndData(
      PyUnicode_4BYTE_KIND, &str[idx[y]], lens[y]);
    PyObject* space = PyLong_FromLong(spaces[y]);
    PyObject* ttype = PyLong_FromLong(types[y]);

    PyList_SET_ITEM(list_words, y, word);
    PyList_SET_ITEM(list_spaces, y, space);
    PyList_SET_ITEM(list_types, y, ttype);

    Py_DECREF(space);
    Py_DECREF(ttype);

    switch (types[y - 1]) {
      case TS_EMOTICON:
      case TS_EMOJI:
      case TS_URL:
      case TS_NEWLINE:
      case TS_PUNCTSTRONG:
        sents[y] = 1;
        break;
      default:
        sents[y] = 0;
        break;
    }
  }

  PyObject* is_sent_start = PyLong_FromLong(1);
  PyObject* isnt_sent_start = PyLong_FromLong(-1);

  for (y = 0; y < i; y++) {
    if (sents[y] && !sents[y + 1])
      PyList_SET_ITEM(list_sents, y, is_sent_start);
    else
      PyList_SET_ITEM(list_sents, y, isnt_sent_start);
  }

  Py_DECREF(is_sent_start);
  Py_DECREF(isnt_sent_start);

  /* build the final tuple */
  ret = PyTuple_Pack(
    4, list_words, list_types, list_spaces, list_sents);

  /* decrement reference count of each list. */
  Py_DECREF(list_types);
  Py_DECREF(list_words);
  Py_DECREF(list_spaces);
  Py_DECREF(list_sents);

FreeEnd:

  PyMem_FREE(str);

  /* free memory for the parser and for the jchar* string */
  free(spaces);
  free(idx);
  free(lens);
  free(_types);
  free(sents);

  return ret;
}

static PyObject*
get_ttype_norm(PyObject* self, PyObject* arg)
{

  int ttype;           /* input value */
  PyObject* res;       /* output values */
  Py_UCS4* norm = U""; /* default norm is no norm */
  Py_ssize_t len = 0;  /* default norm len is 0 */

  /* get the input value */
  if (!PyArg_Parse(arg, "i:ttype", &ttype)) {
    PyErr_BadArgument();
    return NULL;
  }

  switch (ttype) {

    /* normalize emoticon, emoji and url as "@" */
    case TS_EMOTICON:
    case TS_EMOJI:
    case TS_URL:
      norm = U"@";
      len = 1;
      break;

    /* normalize any number as "2" */
    case TS_NUMBER:
      norm = U"2";
      len = 1;
      break;

    /* normalize ordinal like "412ème" as "2ème" */
    case TS_ORDINAL:
      norm = U"2ème";
      len = 4;
      break;

    default:
      break;
  }

  if (!len)
    /* return 0 if not emoticon/emoji/url/number/ordinal */
    res = PyLong_FromLong(0);
  else

    /* return the replacement string */
    res =
      PyUnicode_FromKindAndData(PyUnicode_4BYTE_KIND, norm, len);

  return res;
}

static PyObject*
ttypify_token(PyObject* self, PyObject* arg)
{

  PyObject *input, *ret; /* input value and output values */
  Py_ssize_t len, _len;  /* len of input string */
  Py_UCS4* str;
  int ttype;

  /* get the parameter value */
  if (!PyArg_Parse(arg, "U:tokenize", &input)) {
    PyErr_BadArgument();
    return NULL;
  }

  /* get its length */
  if ((len = PyUnicode_GetLength(input)) == -1) {
    PyErr_BadArgument();
    return NULL;
  }

  /* get the string as C string */
  str = PyUnicode_AsUCS4Copy(input);
  if (!str)
    return PyErr_NoMemory();

  /* get the token type as an int */
  ttype = ttypify(str, (int)len);

  /* make a python int */
  ret = PyLong_FromLong(ttype);

  /* free string */
  PyMem_FREE(str);

  /* return python int (token type ID) */
  return ret;

}

/* informations about the module, so it can be called from within
 * python. */
static PyMethodDef jusqucy_methods[] = {
  { "tokenize", tokenize, METH_O, "Tokenize a text." },
  { "get_ttype_norm", get_ttype_norm, METH_O, "Normalize a special token." },
  { "ttypify", ttypify_token, METH_O, "Typify a token." },
  { NULL, NULL, 0, NULL }
};

static struct PyModuleDef jusqucy_module = {
  PyModuleDef_HEAD_INIT,
  "jusqucy",
  "",
  -1,
  jusqucy_methods,
  NULL,
  NULL,
  NULL,
  NULL,
};

PyMODINIT_FUNC
PyInit_jusqucy(void)
{
  return PyModule_Create(&jusqucy_module);
}
