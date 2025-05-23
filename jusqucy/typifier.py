from .jusqucy import ttypify
from spacy import Language
from spacy.tokens import Doc


def get_ttype(token):
    return token.doc._.jusqucy_ttypes[token.i]


def token_isword(token):
    return token.doc._.jusqucy_ttypes[token.i] in (2, 3, 11)


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
        return self


@Language.factory("jusqucy_typifier")
def create_typifier(nlp, name):
    return Typifier(nlp)
