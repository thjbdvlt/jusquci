"""Token types. (See: ../parser.h)"""

# see ../parser.h
from enum import Enum


class TokenType(Enum):
    SPACE = 1
    WORD = 2
    COMPOUND = 3
    PUNCTSTRONG = 4
    PUNCT = 5
    NUMBER = 6
    URL = 7
    CITEKEY = 8
    EMOTICON = 9
    EMOJI = 10
    ABBREV = 11
    CTRL = 12
    ORDINAL = 13
    NEWLINE = 14
    SPACESIGN = 15
