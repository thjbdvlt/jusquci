import spacy
from .jusqucy import get_ttype_norm


@spacy.Language.component("jusqucy_normalizer")
def normalize(doc):
    """Normalize some Tokens in a Doc.

    Args:
        doc (Doc)

    Returns (Doc)
    """

    for token, ttype in zip(doc, doc._.jusqucy_ttypes):
        norm = get_ttype_norm(ttype)
        if norm != 0:
            token.norm_ = norm
    return doc
