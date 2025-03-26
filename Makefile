

DB_STORAGE := https://object.pouta.csc.fi/synOPUS-index
DB_HOME    := /media/OPUS/synOpusIndex


AVAILABLE_LINKDB_FILES := $(patsubst %,${DB_HOME}/%,$(shell wget -qq -O - ${DB_STORAGE}/index.txt | grep 'linkdb/*\.db'))
INSTALLED_LINKDB_FILES := $(wildcard ${DB_HOME}/linkdb/*.db)
REQUIRED_FTSDB_FILES   := $(patsubst %,${DB_HOME}/%.fts5.db,\
				$(sort $(subst -, ,$(patsubst ${DB_HOME}/linkdb/%.db,%,${INSTALLED_LINKDB_FILES}))))

REDOWNLOAD_INSTALLED_LINKDB_FILES := $(patsubst %.db,%.redownload,${INSTALLED_LINKDB_FILES})


download-all: ${AVAILABLE_LINKDB_FILES}
	${MAKE} download-required-fts-dbs

redownload-all: ${REDOWNLOAD_INSTALLED_LINKDB_FILES}
	${MAKE} download-required-fts-dbs

download-linkdbs: ${AVAILABLE_LINKDB_FILES}
redownload-linkdbs: ${REDOWNLOAD_INSTALLED_LINKDB_FILES}
download-required-fts-dbs: ${REQUIRED_FTSDB_FILES}


${DB_HOME}/linkdb/%.db %.fts5.db:
	wget -q -O $@ ${DB_STORAGE}/$(patsubst ${DB_HOME}/%,%,$@)

${DB_HOME}/linkdb/%.redownload ${DB_HOME}/%.fts5.redownload:
	wget -q -O $(@:.redownload=.db) ${DB_STORAGE}/$(patsubst ${DB_HOME}/%.redownload,%.db,$@)


##--------------------------------------------------------------------------------
## database of all bitexts and aligned corpra
##--------------------------------------------------------------------------------

LANGPAIR_DBS        := $(wildcard ${DB_HOME}/linkdb/*-*.db)

CREATE_TABLE        := CREATE TABLE IF NOT EXISTS
CREATE_INDEX        := CREATE INDEX IF NOT EXISTS
CREATE_UNIQUE_INDEX := CREATE UNIQUE INDEX IF NOT EXISTS
INSERT_INTO         := INSERT OR IGNORE INTO

.PHONY: bitext-db
bitext-db: ${DB_HOME}/linkdb/bitexts.db

${DB_HOME}/linkdb/bitexts.db: ${LANGPAIR_DBS}
	echo "${CREATE_TABLE} bitexts (bitextID,corpus TEXT,version TEXT,fromDoc TEXT,toDoc TEXT)" | sqlite3 $@
	echo "${CREATE_UNIQUE_INDEX} idx_bitexts ON bitexts (corpus,version,fromDoc,toDoc)" | sqlite3 $@
	echo "${CREATE_UNIQUE_INDEX} idx_bitext_ids ON bitexts (bitextID)" | sqlite3 $@
	echo "${CREATE_INDEX} idx_corpus ON bitexts (corpus,version)" | sqlite3 $@
	echo "${CREATE_TABLE} corpora (corpusID,corpus TEXT,version TEXT,srclang TEXT,trglang TEXT,srclang3 TEXT,trglang3 TEXT, latest INTEGER)" | sqlite3 $@
	echo "${CREATE_UNIQUE_INDEX} idx_corpora ON corpora (corpus,version,srclang,trglang,srclang3,trglang3,latest)" | sqlite3 $@
	echo "${CREATE_UNIQUE_INDEX} idx_release ON corpora (corpus,version,srclang,trglang)" | sqlite3 $@
	for d in $?; do \
	  echo "processing $$d"; \
	  echo "ATTACH DATABASE '$$d' as l; \
		${INSERT_INTO} bitexts SELECT * FROM l.bitexts; \
		${INSERT_INTO} corpora SELECT * FROM l.corpora;" | sqlite3 $@; \
	done
