import spacy
from .jusqucy import get_ttype_norm


@spacy.Language.component("jusqucy_normalizer")
def normalize(doc):
    """Normalize some Tokens in a Doc.

    Args:
        doc (Doc)

    Returns (Doc)
    """

    for token, norm in zip(
        doc, map(get_ttype_norm, doc._.jusqucy_ttypes)
    ):
        if norm != 0:
            token.norm_ = norm
    return doc
