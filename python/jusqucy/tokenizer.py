"""Simple wrapper to use jusquci with spaCy."""

from spacy.tokens import Doc, Token
from spacy.vocab import Vocab
from spacy import registry
from .jusqucy import tokenize
from .ttypes import get_ttype, token_isword
from typing import Union


def token_byte_index(token):
    offset = token.doc._.jusqucy_offsets[token.i]
    length = token.doc._.jusqucy_lengths[token.i]
    return (offset, offset + length)


def token_line_number(token):
    return token.doc._.jusqucy_line_numbers[token.i]


class JusqucyTokenizer:
    def __init__(
        self,
        vocab: Vocab,
        ext_token_ttype: Union[str, None] = "isword",
        ext_token_isword: Union[str, None] = "ttype",
        ext_token_byte_index: Union[str, None] = "byte_index",
        ext_token_line_number: Union[str, None] = "line_number",
    ):
        self.vocab = vocab

        for i in ["ttypes", "offsets", "lengths", "line_numbers"]:
            Doc.set_extension(f"jusqucy_{i}", default=None, force=True)

        for ext_name, getter in [
            (ext_token_byte_index, token_byte_index),
            (ext_token_isword, token_isword),
            (ext_token_ttype, get_ttype),
            (ext_token_line_number, token_line_number),
        ]:
            if ext_name:
                Token.set_extension(ext_name, getter=getter, force=True)

    def __call__(self, text: str, *args, **kwargs) -> Doc:
        """Tokenize a text.

        Args:
            text (str): the text to tokenize.

        Returns (Doc): the spacy.tokens.Doc.
        """

        (
            words,
            ttypes,
            spaces,
            sent_starts,
            offsets,
            lengths,
            line_numbers,
        ) = tokenize(text)

        doc = Doc(
            words=words,
            spaces=spaces,
            vocab=self.vocab,
            sent_starts=sent_starts,
            **kwargs,
        )
        doc._.jusqucy_ttypes = ttypes
        doc._.jusqucy_offsets = offsets
        doc._.jusqucy_lengths = lengths
        doc._.jusqucy_line_numbers = line_numbers

        return doc

    def pipe(self, texts, batch_size=1000):
        for i in texts:
            yield self(i)

    def to_disk(self, path, *, exclude=tuple(), **kwargs):
        pass

    def from_disk(self, path, *, exclude=tuple(), **kwargs):
        return self


@registry.tokenizers("jusqucy_tokenizer")
def create_tokenizer(
    ext_token_ttype: Union[str, None] = "ttype",
    ext_token_isword: Union[str, None] = "isword",
    ext_token_byte_index: Union[str, None] = "byte_index",
):
    def make_tokenizer(nlp):
        return JusqucyTokenizer(
            nlp.vocab,
            ext_token_ttype,
            ext_token_isword,
            ext_token_byte_index,
        )

    return make_tokenizer
