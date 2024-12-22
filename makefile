MODULE_big = jusquci
EXTENSION = jusquci
HEADERS = parser.h
OBJS = jusquci.o src/parser.o src/affixes.o src/punct.o src/util.o
DATA = jusquci--1.0.sql

PG_CFLAGS = -DJUSQUCI_POSTGRESQL

PG_CONFIG ?= pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
