MODULE_big = jusquci
EXTENSION = jusquci
HEADERS = src/parser.h
OBJS = jusquci.o src/parser.o src/affixes.o src/punct.o src/util.o
DATA = jusquci--1.0.sql

PG_CFLAGS = -DJUSQUCI_POSTGRESQL

PG_CONFIG ?= pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

TSEARCH_DIR = $(shell $(PG_CONFIG) --sharedir)/tsearch_data
DICT_DATA = french_jusquci.stop

install_stop: install
	cp french_jusquci.stop $(TSEARCH_DIR)/french_jusquci.stop
