import spacy
from .ttypes import TokenType

end_ttypes = ("PUNCTSTRONG", "NEWLINE", "EMOTICON", "EMOJI")
end_ttypes = [TokenType[i].value for i in end_ttypes]
end_ttypes = set(end_ttypes)


@spacy.Language.component("jusqucy_sentencizer")
def sentencize(doc):
    """Sentencize a Doc using jusqucy token types.

    Args:
        doc (Doc)

    Returns (Doc)
    """

    if len(doc) == 0:
        return doc
    doc[0].is_sent_start = True
    for token, ttype in zip(doc[1:], doc._.jusqucy_ttypes):
        token.is_sent_start = True if ttype in end_ttypes else False
    return doc
