import spacy
from .jusqucy import get_ttype_norm


@spacy.Language.component("jusqucy_normalizer")
def normalize(doc):
    """Normalize some Tokens in a Doc.

    Args:
        doc (Doc)

    Returns (Doc)
    """

    doc[0].is_sent_start = True
    for token, ttype in filter(
        lambda i: i[0].lex.norm == 0, zip(doc, doc._.jusqucy_ttypes)
    ):
        norm = get_ttype_norm(ttype)
        if norm != 0:
            token.lex.norm_ = norm
    return doc
