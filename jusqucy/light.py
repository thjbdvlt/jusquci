import jusqucy
from .ttypes import token_types


class Token:
    def __init__(
        self,
        text: str,
        ttype: int,
        space: int,
        sent_start: int,
    ):
        self.sent_start = sent_start
        self.space = space
        self.text = text
        self.ttype = ttype

    def __str__(self):
        return self.text

    def __repr__(self):
        return self.text

    @property
    def type_(self):
        return token_types[self.ttype]


def to_token_list(text: str) -> list[Token]:
    """Tokenize a text."""
    return [Token(*i) for i in zip(*jusqucy.tokenize(text))]
