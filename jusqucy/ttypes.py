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


token_types = [None] + [
    i.name.lower() for i in sorted(TokenType, key=lambda i: i.value)
]


def get_ttype(token):
    return token.doc._.jusqucy_ttypes[token.i]


def token_isword(token):
    return token.doc._.jusqucy_ttypes[token.i] in (2, 3, 11)
