__jusquci__ -- a french tokenizer for postgresql text search and spacy.

| text                    | tokens                      |
| ----------------------- | --------------------------- |
| jusqu'ici=>             | `jusqu'` `ici` `=>`         |
| celle-ci-->ici          | `celle` `-ci` `-->` `ici`   |
| lecteur-rice-x-s        | `lecteur-rice-x-s`          |
| peut-être--là           | `peut-être` `--` `là`       |
| correcteur·rices        | `correcteur·rices`          |
| mais.maintenant         | `mais` `.` `maintenant`     |
| \[re\]lecteur.rice.s    | `[re]lecteur.rice.s`        |
| autre(s)                | `autre(s)`                  |
| (autres)                | `(` `autres` `)`            |
| (autre(s))              | `(` `autre(s)` `)`          |
| www<area/>.on-tenk.com  | `www.on-tenk.com`           |
| \[@becker_1982,p.12\]  | `[` `@becker_1982` `,` `p.` `12` `]` |
| oui..?                  | `oui` `..?`                 |
| dedans/dehors           | `dedans` `/` `dehors`       |
| :happy: :) pour:        | `:happy:` `:)` `pour` `:`   |
| ô.ô^^=):-)xd            | `ô.ô` `^^` `=)` `:-)` `xd`  |

## postgresql extension

the primary role of this tokenizer is to be used as a [text search parser](https://www.postgresql.org/docs/current/textsearch-parsers.html) in postgresql, hence it's proposed here as an postgresql extension.

```bash
make install
```

```sql
create extension jusquci;

select to_tsvector(
    'jusquci',
    'le quotidien,s''invente-t-il par mille.manière de braconner???'
);
```

## in python

the single provided function (`tokenize`) returns three lists:

- __tokens__: a list of strings.
- __tokens types__: a list of token types ID; the types are defined as an *Enum* (`jusqucy.ttypes.TokenType`).
- __spaces__: a list of boolean values that indicates if tokens are followed by a space or not (for spaCy, mostly).

the tokenizer can be used in a spacy pipeline. it tokenizes the text and add a attribute to the resulting `Doc` object, `Doc._.ttypes` in which are store token types (assigning to each token takes much more time).

```python
import spacy
import jusqucy

nlp = spacy.blank('fr')
nlp.tokenizer = jusqucy.JusqucyTokenizer(nlp.vocab)

# or:
nlp = spacy.load(your_model, config={
    "nlp": {"tokenizer": {"@tokenizers": "jusqucy_tokenizer"}}
})
```

to get the token types:

```python
from jusqucy.ttypes import TokenType
for token, ttype in zip(doc, doc._.ttypes):
    print(token, TokenType[ttype])
```

## as a command line tool

to use __jusquci__ as a simple command line tokenizer (that reads from `stdin`), just compile it with the makefile in the `cli` directory.
the program read a text from standard input and output tokens separated by spaces. it also add newlines after strong punctuation signs (`.`, `?`, `!`).

## sources

- [tsexample](https://github.com/postgrespro/tsexample)

## todo

- [ ] *presquci*, a dictionary for postgresql to be used with the parser.
- [x] jusqucy, a python module.

## os

only tested on linux (debian) and postgresql 16
