from jusqucy.jusqucy import tokenize, ttypify
from jusqucy.ttypes import TokenType

try:
    import jusqucy.tokenizer
    from .tokenizer import JusqucyTokenizer

except ModuleNotFoundError:
    pass
