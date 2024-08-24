#!/usr/bin/env python3
#
# arguments: xx-yy.db xx.ids.db yy.ids.db xx-yy.old.db xx-yy.converted.db
#

import sys
import sqlite3
import os, traceback

if len(sys.argv) != 6:
    print("USAGE: convert_linkdb.py xx-yy.db xx.ids.db yy.ids.db xx-yy.old.db xx-yy.converted.db")
    exit()

bitextDB = sys.argv[1]
srcDB = sys.argv[2]
trgDB = sys.argv[3]
algDB = sys.argv[4]
linkDB = sys.argv[5]

buffersize = 100000

##----------------------------------------------------------------
## connect to source and target language sentence index DBs
##----------------------------------------------------------------

srcDBcon = sqlite3.connect(f"file:{srcDB}?immutable=1",uri=True)
srcDBcur = srcDBcon.cursor()

trgDBcon = sqlite3.connect(f"file:{trgDB}?immutable=1",uri=True)
trgDBcur = trgDBcon.cursor()

##----------------------------------------------------------------
# create DB that shows what sentences are included in what kind of alignment units
##----------------------------------------------------------------

linksDBcon = sqlite3.connect(linkDB, timeout=7200)
linksDBcur = linksDBcon.cursor()

## tables that map sentences to links

linksDBcur.execute("CREATE TABLE IF NOT EXISTS linkedsource ( sentID INTEGER, linkID INTEGER, bitextID INTEGER, PRIMARY KEY(linkID,sentID) )")
linksDBcur.execute("CREATE TABLE IF NOT EXISTS linkedtarget ( sentID INTEGER, linkID INTEGER, bitextID INTEGER, PRIMARY KEY(linkID,sentID) )")

linksDBcur.execute("CREATE INDEX IF NOT EXISTS idx_linkedsource_bitext ON linkedsource (bitextID,sentID)")
linksDBcur.execute("CREATE INDEX IF NOT EXISTS idx_linkedtarget_bitext ON linkedtarget (bitextID,sentID)")
linksDBcur.execute("CREATE INDEX IF NOT EXISTS idx_linkedsource_linkid ON linkedsource (linkID)")
linksDBcur.execute("CREATE INDEX IF NOT EXISTS idx_linkedtarget_linkid ON linkedtarget (linkID)")
linksDBcur.execute("CREATE INDEX IF NOT EXISTS idx_linkedsource_sentid ON linkedsource (sentID)")
linksDBcur.execute("CREATE INDEX IF NOT EXISTS idx_linkedtarget_sentid ON linkedtarget (sentID)")

## the original alignment table, now also with internal sentence IDs

linksDBcur.execute("""CREATE TABLE IF NOT EXISTS links ( linkID INTEGER NOT NULL PRIMARY KEY, bitextID, 
                                                         srcIDs TEXT, trgIDs TEXT, srcSentIDs TEXT, trgSentIDs TEXT,
                                                         alignType TEXT, alignerScore REAL, cleanerScore REAL)""")
linksDBcur.execute("CREATE UNIQUE INDEX IF NOT EXISTS idx_links ON links ( bitextID, srcIDs, trgIDs )")
linksDBcur.execute("CREATE INDEX IF NOT EXISTS idx_aligntype ON links ( bitextID, alignType )")
linksDBcur.execute("CREATE INDEX IF NOT EXISTS idx_bitextid ON links ( bitextID )")



linksDBcur.close()

##----------------------------------------------------------------
## connect to original sentence alignment DB
##----------------------------------------------------------------


algDBcon = sqlite3.connect(f"file:{algDB}?immutable=1",uri=True)
algDBcur = algDBcon.cursor()

bitextDBcon = sqlite3.connect(f"file:{bitextDB}?immutable=1",uri=True)
bitextDBcur = bitextDBcon.cursor()


# insert links from buffer

srcbuffer = []
trgbuffer = []
linkbuffer = []

def insert_links():
    global linkDB, srcbuffer, trgbuffer, linkbuffer
    if len(srcbuffer) > 0 or len(trgbuffer) > 0:
        linksDBcon = sqlite3.connect(linkDB, timeout=7200)
        linksDBcur = linksDBcon.cursor()
        if len(srcbuffer) > 0:
            linksDBcur.executemany("""INSERT OR IGNORE INTO linkedsource VALUES(?,?,?)""", srcbuffer)
        if len(trgbuffer) > 0:
            linksDBcur.executemany("""INSERT OR IGNORE INTO linkedtarget VALUES(?,?,?)""", trgbuffer)
        if len(linkbuffer) > 0:
            linksDBcur.executemany("""INSERT OR IGNORE INTO links VALUES(?,?,?,?,?,?,?,?,?)""", linkbuffer)

        linksDBcon.commit()
        linksDBcur.close()
        srcbuffer = []
        trgbuffer = []
        linkbuffer = []        


#----------------------------------------------------------------
# run through all bitexts in this corpus (aligned document pairs)
# and store links that map internal sentence IDs to internal linkIDs
#----------------------------------------------------------------

bitextID = 0
fromDocID = 0
toDocID = 0
count = 0


for row in algDBcur.execute(f"SELECT rowid,srcIDs,trgIDs,alignType,alignerScore,cleanerScore,bitextID FROM links"):

    linkID = row[0]
    srcIDs = row[1].split(' ')
    trgIDs = row[2].split(' ')

    if row[6] != bitextID:
        bitextID = row[6]    
        for bitext in bitextDBcur.execute(f"SELECT corpus,version,fromDoc,toDoc FROM bitexts WHERE rowid={bitextID}"):
            corpus = bitext[0]
            version = bitext[1]
            fromDoc = bitext[2]
            toDoc = bitext[3]

        # find document IDs (fromDocID and toDocID)
    
        for doc in srcDBcur.execute(f"SELECT rowid FROM documents WHERE corpus='{corpus}' AND version='{version}' AND document='{fromDoc}'"):
            fromDocID = doc[0]
        for doc in trgDBcur.execute(f"SELECT rowid FROM documents WHERE corpus='{corpus}' AND version='{version}' AND document='{toDoc}'"):
            toDocID = doc[0]
    

    count+=1
    if not count % 5000:
        sys.stderr.write('.')
        if not count % 100000:
            sys.stderr.write(f" {count}\n")
        sys.stderr.flush()


    # get source and target sentence IDs from the sentence indeces
    # (search for the OPUS IDs in the sentence index DBs)

    srcSentIDs = []
    trgSentIDs = []

    cleanSrcIDs = []
    cleanTrgIDs = []

    for s in srcIDs:
        if (s):
            cleanSrcIDs.append(s)
            for sent in srcDBcur.execute(f"SELECT id FROM sentids WHERE docID={fromDocID} AND sentID='{s}'"):
                sentID = sent[0]
                srcbuffer.append(tuple([sentID,linkID,bitextID]))
                srcSentIDs.append(str(sentID))

    for t in trgIDs:
        if (t):
            cleanTrgIDs.append(t)
            for sent in trgDBcur.execute(f"SELECT id FROM sentids WHERE docID={toDocID} AND sentID='{t}'"):
                sentID = sent[0]
                trgbuffer.append(tuple([sentID,linkID,bitextID]))
                trgSentIDs.append(str(sentID))

    srcID = ' '.join(cleanSrcIDs)
    trgID = ' '.join(cleanTrgIDs)
    srcSentID = ' '.join(srcSentIDs)
    trgSentID = ' '.join(trgSentIDs)

    linkbuffer.append([linkID,bitextID,srcID,trgID,srcSentID,trgSentID,row[3],row[4],row[5]])

    if len(srcbuffer) >= buffersize or len(trgbuffer) >= buffersize:
        insert_links()


# final insert if necessary
insert_links()


