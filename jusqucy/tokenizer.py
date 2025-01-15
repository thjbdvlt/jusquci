"""Simple wrapper to use jusquci with spaCy."""

from spacy.tokens import Doc, Token
from spacy.vocab import Vocab
from spacy import registry
from .jusqucy import tokenize
from .ttypes import get_token_type


class JusqucyTokenizer:
    def __init__(
        self,
        vocab: Vocab,
        ext_name_token: str = "jusqucy_ttype",
    ):
        self.vocab = vocab

        Doc.set_extension("jusqucy_ttypes", default=None, force=True)

        Token.set_extension(
            ext_name_token, getter=get_token_type, force=True
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
def create_tokenizer(ext_name_token: str = 'jusqucy_ttype'):

    def make_tokenizer(nlp):
        return JusqucyTokenizer(nlp.vocab, ext_name_token)

    return make_tokenizer
