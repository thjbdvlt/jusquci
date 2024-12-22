#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "../parser.h"

static PyObject*
tokenize(PyObject* self, PyObject* arg)
{
  TParser pst;          /* parser */
  Py_ssize_t len, _len; /* len of input string */
  wchar_t *str;     /* string converted to wide char */
  int i, y, ttype;      /* for iterations */

  PyObject *input, *ret; /* input value and output values */
  PyObject *list_words, *list_types, *list_spaces; /* lists */

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

  /* allocate memory for its content */
  if (!(str = malloc(sizeof(wchar_t*) * (size_t)len)))
    return PyErr_NoMemory();

  /* copy its values to the wchar_t pointer */
  if ((_len = PyUnicode_AsWideChar(input, str, len)) == -1) {
    PyErr_BadArgument();
    return NULL;
  }

  /* check that the lengths are equal -- or my code is wrong. */
  assert(len == _len);

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
  int* types = (int*)malloc(sizeof(int*) * (size_t)len);

  /* ensure that memory has been allocated */
  if (!spaces || !idx || !lens || !types) {
    free_parser(&pst);
    free(str);
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
  types[0] = ttype;
  i++;

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

  if (!list_words || !list_types || !list_words) {
    ret = PyErr_NoMemory();
    Py_XDECREF(list_types);
    Py_XDECREF(list_words);
    Py_XDECREF(list_spaces);
    goto FreeEnd;
  }

  /* populate the lists */
  for (y = 0; y < i; y++) {
    // TEST. (it's faster, so i should rebuild all that stuff)
    // PyObject* word = PyUnicode_New(lens[y], 1114111);
    // for (int i = 0; i < lens[y]; i++)
    //   PyUnicode_WriteChar(word, i, 'C');
    // PyList_SET_ITEM(list_words, y, word);

    // PyList_SET_ITEM(
    //   list_words, y, PyUnicode_FromWideChar(&pst.str[idx[y]], lens[y]));
    PyList_SET_ITEM(list_spaces, y, PyLong_FromLong(spaces[y]));
    PyList_SET_ITEM(list_types, y, PyLong_FromLong(types[y]));
  }

  /* build the final tuple */
  ret = PyTuple_Pack(3, list_words, list_types, list_spaces);

  /* decrement reference count of each list. */
  Py_DECREF(list_types);
  Py_DECREF(list_words);
  Py_DECREF(list_spaces);

FreeEnd:

  /* free memory for the parser and for the wchar_t* string */
  free_parser(&pst);
  free(str);
  free(spaces);
  free(idx);
  free(lens);
  free(types);

  /* return the nested tuples */
  return ret;
}

/* informations about the module, so it can be called from within
 * python. */
static PyMethodDef jusqucy_methods[] = {
  { "tokenize", tokenize, METH_O, "Tokenize a text." },
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
