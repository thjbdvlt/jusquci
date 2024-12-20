from setuptools import setup

setup(
    name="jusqucy",
    entry_points={
        "spacy_tokenizers": [
            "jusqucy_tokenizer = jusqucy.tokenizer:create_jusqucy_tokenizer"
        ]
    },
)
