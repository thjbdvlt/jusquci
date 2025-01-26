__jusquci__ -- tokenizer for french (PostgreSQL/spaCy).

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
make install install_stop
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
- __is_sent_start__: a list of boolean values that's used to set `Token.is_sent_start` (based of the __token types__).

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
for token, ttype in zip(doc, doc._.jusqucy_ttypes):
    print(token, TokenType[ttype])
```

### normalizer

a normalizer can also be used as a spacy component. it replace the `norm_` attribute of token of some `ttypes`, in order to make the following components (e.g. morphologizer or parser) easier.

- `url`: `https://`
- `number`: `2`
- `ordinal`: `2ème`
- `emoticon`: `:)`
- `emoji`: `:)`

## as a command line tool

to use __jusquci__ as a simple command line tokenizer (that reads from `stdin`), just compile it with the makefile in the `cli` directory.
the program read a text from standard input and output tokens separated by spaces. it also add newlines after strong punctuation signs (`.`, `?`, `!`).

## sources

- [tsexample](https://github.com/postgrespro/tsexample)

the stopwords list is the concatenation of postgresql default stopwords for french (`french.stop`) and a list establish by Jacques Savoy[^Savoy]. I've also added a few words: *elised* words with apostrophe (e.g. `c'`), to be consistent with the `jusquci` parser (postgresql doesn't include the apostrophe), and non-binary pronouns (e.g. `iel`, `celleux`).

[Savoy]: *A stemming procedure and stopword list for general french corpora*, Jacques Savoy, Institut interfacultaire d'informatique, *Journal of the American Society for Information Science*, 50(10), 1999, 944-952. I removed a word from this list: `passé`.

## os

only tested on linux (debian) and postgresql 16
