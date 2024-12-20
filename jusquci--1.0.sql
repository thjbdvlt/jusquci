-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION pair" to load this file. \quit

CREATE OR REPLACE FUNCTION jusquci_parser_start (internal, int)
    RETURNS internal
    AS 'MODULE_PATHNAME'
LANGUAGE c
STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION jusquci_parser_gettoken (internal, internal, internal)
    RETURNS internal
    AS 'MODULE_PATHNAME'
LANGUAGE c
STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION jusquci_parser_end (internal)
    RETURNS void
    AS 'MODULE_PATHNAME'
LANGUAGE c
STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION jusquci_parser_lextype (internal)
    RETURNS internal
    AS 'MODULE_PATHNAME'
LANGUAGE c
STRICT IMMUTABLE;

DROP TEXT SEARCH PARSER IF EXISTS jusquci CASCADE;

CREATE TEXT SEARCH PARSER jusquci (
    START = jusquci_parser_start,
    GETTOKEN = jusquci_parser_gettoken,
    END = jusquci_parser_end,
    LEXTYPES = jusquci_parser_lextype
);

CREATE text search configuration jusquci (
    parser = 'jusquci'
);

DROP TEXT SEARCH DICTIONARY IF EXISTS jusquci CASCADE;

CREATE TEXT SEARCH DICTIONARY jusquci (
    TEMPLATE = simple,
    stopwords = french
);

ALTER TEXT SEARCH CONFIGURATION jusquci
    ALTER MAPPING FOR word WITH french_stem;
