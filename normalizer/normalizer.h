#include <wchar.h>

/* reduce_repeated_letters -- remove repeated letters (3x or more).
 *
 * s:  the string (will be modified).
 * len:  the len of the string
 *
 * returns:  the new length.
 */
int
reduce_repeated_letters(wchar_t* s, int len);

int
normalize_inclusive_suffix(wchar_t* s, int suffstart, int suffend, int len);
