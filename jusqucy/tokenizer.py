"""Simple wrapper to use jusquci with spaCy."""

from spacy.tokens import Doc, Token
from spacy.vocab import Vocab
from spacy import registry
from spacy import Language
from .jusqucy import tokenize, ttypify


if not Token.has_extension("jusqucy_ttype"):
    Token.set_extension(
        "jusqucy_ttype",
        getter=lambda token: token.doc._.jusqucy_ttypes[token.i],
    )

if not Doc.has_extension("jusqucy_ttypes"):
    Doc.set_extension("jusqucy_ttypes", default=None)


class JusqucyTokenizer:
    def __init__(self, vocab: Vocab):
        self.vocab = vocab

    def __call__(self, text: str, *args, **kwargs) -> Doc:
        """Tokenize a text.

        Args:
            text (str): the text to tokenize.

        Returns (Doc): the spacy.tokens.Doc.
        """

        # tokenize
        words, ttypes, spaces, sent_starts = tokenize(text)

        # build the doc
        doc = Doc(
            words=words,
            spaces=spaces,
            vocab=self.vocab,
            sent_starts=sent_starts,
            **kwargs,
        )
        doc._.jusqucy_ttypes = ttypes

        # returns the Doc
        return doc

    def pipe(self, texts, batch_size=1000):
        for i in texts:
            yield self(i)

    def to_disk(self, path, *, exclude=tuple(), **kwargs):
        pass

    def from_disk(self, path, *, exclude=tuple(), **kwargs):
        super().__init__()


@registry.tokenizers("jusqucy_tokenizer")
def create_tokenizer():
    def make_tokenizer(nlp):
        return JusqucyTokenizer(nlp.vocab)

    return make_tokenizer


class Typifier:
    def __init__(self, nlp):
        """Initiate a Typifier.

        This overwrite the `Doc._.jusqucy_ttypes` extension. It's mainly usefull for training purpose, when you have an already tokenized corpus and just want to get the Token Types as if it was tokenized by a `JusqucyTokenizer`.
        """

        Doc.set_extension(
            "jusqucy_ttypes",
            getter=lambda doc: [ttypify(i.orth_) for i in doc],
            force=True,
        )

    def __call__(self, doc):
        """Typify tokens in a Doc."""

        return doc

    def pipe(self, texts, batch_size=1000):
        for i in texts:
            yield self(i)

    def to_disk(self, path, *, exclude=tuple(), **kwargs):
        pass

    def from_disk(self, path, *, exclude=tuple(), **kwargs):
        super().__init__()


@Language.factory("jusqucy_typifier")
def create_typifier(nlp, name):
    return Typifier(nlp)
