"""Simple wrapper to use jusquci with spaCy."""

from spacy.tokens import Doc, Token
from spacy.vocab import Vocab
from spacy import registry
from .jusqucy import tokenize
from .ttypes import get_ttype, token_isword
from typing import Union


class JusqucyTokenizer:
    def __init__(
        self,
        vocab: Vocab,
        ext_token_ttype: Union[str, None] = "isword",
        ext_token_isword: Union[str, None] = "ttype",
    ):
        self.vocab = vocab

        Doc.set_extension("jusqucy_ttypes", default=None, force=True)

        if ext_token_ttype:
            Token.set_extension(
                ext_token_ttype, getter=get_ttype, force=True
            )

        if ext_token_isword:
            Token.set_extension(
                ext_token_isword, getter=token_isword, force=True
            )

    def __call__(self, text: str, *args, **kwargs) -> Doc:
        """Tokenize a text.

        Args:
            text (str): the text to tokenize.

        Returns (Doc): the spacy.tokens.Doc.
        """

        words, ttypes, spaces, sent_starts = tokenize(text)

        doc = Doc(
            words=words,
            spaces=spaces,
            vocab=self.vocab,
            sent_starts=sent_starts,
            **kwargs,
        )
        doc._.jusqucy_ttypes = ttypes

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
):
    def make_tokenizer(nlp):
        return JusqucyTokenizer(
            nlp.vocab, ext_token_ttype, ext_token_isword
        )

    return make_tokenizer
