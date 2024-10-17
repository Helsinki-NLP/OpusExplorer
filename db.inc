<?php // -*-php-*-


function get_lang_dbfile($lang){
    global $DB_DIR;
    if (file_exists($DB_DIR.$lang.'.fts5.db')) return $DB_DIR.$lang.'.fts5.db';
    return '';
}

function get_langidx_dbfile($lang){
    global $DB_DIR;
    if (file_exists($DB_DIR.$lang.'.ids.db')) return $DB_DIR.$lang.'.ids.db';
    if (file_exists($DB_DIR.$lang.'.idx.db')) return $DB_DIR.$lang.'.idx.db';
    return '';
}

function get_link_dbfile($langpair,$corpus,$version,$fromDoc,$toDoc){
    global $DB_DIR;
    if ($langpair){
        if ($corpus && $version){
            $linkdbfile = $DB_DIR.implode('/',['sqlite',$corpus,$version,$langpair.'.db']);
            if (file_exists($linkdbfile)) return $linkdbfile;
            // if a specific bitext is selected than the general index would also work
            // TODO: if we change to only include the latest version in that corpus
            //       then we also need to check the version of the corpus here!
            if ($fromDoc && $toDoc){
                $linkdbfile = $DB_DIR.implode('/',['sqlite',$langpair.'.db']);
                if (file_exists($linkdbfile)) return $linkdbfile;
            }
        }
        else{
            $linkdbfile = $DB_DIR.implode('/',['sqlite',$langpair.'.db']);
            if (file_exists($linkdbfile)) return $linkdbfile;
        }
    }
    return '';
}

function get_alignment_dbfile($langpair){
    global $DB_DIR;
    $dbfile = $DB_DIR.implode('/',['sqlite',$langpair.'.db']);
    if (file_exists($dbfile)) return $dbfile;
    return '';
}

function get_ratings_dbfile($langpair){
    global $DB_DIR;
    $dbfile = $DB_DIR.$langpair.'.stars.db';
    if (file_exists($dbfile)) return $dbfile;
    return '';
}

function get_bitext_dbfile($langpair){
    global $DB_DIR;
    $dbfile = $DB_DIR.implode('/',['sqlite',$langpair.'.db']);
    if (file_exists($dbfile)) return $dbfile;
    return '';
}



function get_bitext($bitextID){
    global $bitextDBH;

    if ($bitextDBH){
        $results = $bitextDBH->query("SELECT * FROM bitexts WHERE rowid=".$bitextID);
        if ($results)
            if ($row=$results->fetchArray(SQLITE3_ASSOC))
                return $row;
    }
    return array('corpus' => '', 'version' => '', 'fromDoc' => '', 'toDoc' => '');
}


function get_bitextid($corpus, $version, $fromDoc, $toDoc){
    global $linksDBH;
    
    $results = $linksDBH->query("SELECT rowid FROM bitexts WHERE corpus='$corpus' AND version='$version' AND fromDoc='$fromDoc' AND toDoc='$toDoc'");
    if ($results){
        while ($row = $results->fetchArray(SQLITE3_NUM)) {
            return $row[0];
        }
    }
    return undef;
}

function get_link($linkID){
    global $linksDBH;
    if ($linksDBH){
        $result = $linksDBH->query("SELECT * FROM links WHERE linkID=".$linkID);
        if ($result){
            while ($row = $result->fetchArray(SQLITE3_ASSOC)) {
                return $row;
            }
        }
    }
    return array();
}


function get_alignments_old($linksDBH, $corpus, $version, $fromDoc, $toDoc,
                            $bitextID=0, $type='all', $offset=0){
    
    global $showEmpty, $showMaxAlignments, $ALIGN_TYPES;
    
    if ($bitextID){
        $conditions = "WHERE bitextID=$bitextID";
        $table = 'links';
    }
    else{
        $conditions = "WHERE corpus='$corpus' AND version='$version' AND fromDoc='$fromDoc' AND toDoc='$toDoc'";
        $table = 'alignments';
    }
    if ($type){
        if ($type == 'other'){
            foreach ($ALIGN_TYPES as $type){
                $conditions .= " AND NOT alignType='$type'";
            }
        }
        else{
            $conditions .= " AND alignType='$type'";
        }
    }
    if ((! $showEmpty) && ($type != 'other')){
        $conditions .= " AND NOT alignType='0-1' AND NOT alignType='1-0'";
    }
    $limit = "LIMIT $showMaxAlignments";
    if ($offset){
        $limit .= " OFFSET $offset";
    }
    // echo("SELECT srcIDs,trgIDs,alignerScore,linkID FROM $table $conditions ORDER BY linkID $limit");
    return $linksDBH->query("SELECT srcIDs,trgIDs,alignerScore,linkID FROM $table $conditions ORDER BY linkID $limit");
}


function get_alignments($linksDBH, $bitext, $type='all', $offset=0){
    global $showEmpty, $showMaxAlignments, $ALIGN_TYPES;

    $bitextID = $bitext->id();
    $conditions = "WHERE bitextID=$bitextID";
    
    if ($type){
        if ($type == 'other'){
            foreach ($ALIGN_TYPES as $type){
                $conditions .= " AND NOT alignType='$type'";
            }
        }
        else{
            $conditions .= " AND alignType='$type'";
        }
    }
    if ((! $showEmpty) && ($type != 'other')){
        $conditions .= " AND NOT alignType='0-1' AND NOT alignType='1-0'";
    }
    $limit = "LIMIT $showMaxAlignments";
    if ($offset){
        $limit .= " OFFSET $offset";
    }
    return $linksDBH->query("SELECT * FROM links $conditions ORDER BY linkID $limit");
}



// fetch internal sentence IDs from link DB

function fetch_linked_sentence_ids($linksDBH,$linkID){
    
    // echo("SELECT sentID FROM linkedsource WHERE linkID=$linkID\n");
    $results = $linksDBH->query("SELECT sentID FROM linkedsource WHERE linkID=$linkID ORDER BY rowid");
    $srcIDs = array();
    if ($results){
        while ($row = $results->fetchArray(SQLITE3_NUM)) {
            array_push($srcIDs,$row[0]);
        }
    }
    // echo("SELECT sentID FROM linkedtarget WHERE linkID=$linkID");
    $results = $linksDBH->query("SELECT sentID FROM linkedtarget WHERE linkID=$linkID ORDER BY rowid");
    $trgIDs = array();
    if ($results){
        while ($row = $results->fetchArray(SQLITE3_NUM)) {
            array_push($trgIDs,$row[0]);
        }
    }
    return array($srcIDs,$trgIDs);
}

// fetch a specific sentence from sentence DB

function fetch_sentence($SentDBH, $id){
    if ($id){
        // echo("SELECT sentence FROM sentences WHERE rowid='$id'");
        $results = $SentDBH->query("SELECT sentence FROM sentences WHERE rowid='$id'");
        if ($results){
            while ($row = $results->fetchArray(SQLITE3_NUM)) {
                return $row[0];
            }
        }
    }
    return 'SENTENCE NOT FOUND';
}







// get sentences that are aligned to each other for a given link
// --> use sentence index files if they exist
// --> otherwise use link index (internal sentence IDs mapped to LinkIDs)

function get_linked_sentences($linkID, $srcIDs, $trgIDs,
                              $corpus='', $version='',
                              $fromDoc='', $toDoc='',
                              $fromDocID=0, $toDocID=0){
    
    global $srcDBH, $trgDBH;

    list($srcSentIDs,$trgSentIDs) = get_linked_sentences_ids($linkID, $srcIDs, $trgIDs,
                                                             $corpus, $version,
                                                             $fromDoc, $toDoc,
                                                             $fromDocID, $toDocID);
    $srcSents = array();
    $trgSents = array();
    
    foreach ($srcSentIDs as $id) array_push($srcSents, fetch_sentence($srcDBH, $id));
    foreach ($trgSentIDs as $id) array_push($trgSents, fetch_sentence($trgDBH, $id));

    return array($srcSents, $trgSents);
}


function get_linked_sentences_ids($linkID, $srcIDs, $trgIDs,
                                  $corpus='', $version='',
                                  $fromDoc='', $toDoc='',
                                  $fromDocID=0, $toDocID=0){
    global $linkDbFile;
    global $srcDBH, $trgDBH;
    global $srcIdxDBH, $trgIdxDBH;

    $srcSentIDs = array();
    $trgSentIDs = array();
    
    if ($srcIdxDBH && $trgIdxDBH){
        $srcSentIDs = get_sentence_ids($srcIdxDBH, $corpus, $version, $fromDoc, $srcIDs, $fromDocID);
        $trgSentIDs = get_sentence_ids($trgIdxDBH, $corpus, $version, $toDoc, $trgIDs, $toDocID);
        return array($srcSentIDs, $trgSentIDs);
    }
    elseif ($linkDbFile){
        $linksDBH = new SQLite3($linkDbFile,SQLITE3_OPEN_READONLY);
        return fetch_linked_sentence_ids($linksDBH, $linkID);        
    }
    return array($srcSentIDs, $trgSentIDs);
}






// get the internal sentence ID from the OPUS sentence ID
// - use sentids table if docID is given
// - use sentindex view otherwise (with corpus, version, document names from OPUS)

function get_sentence_id($IdxDBH, $corpus, $version, $document, $sentID, $docID=0){
    if ($docID){
        $results = $IdxDBH->query("SELECT id FROM sentids WHERE docID=$docID AND sentID='$sentID'");
        if ($results){
            while ($row = $results->fetchArray(SQLITE3_NUM)) {
                return $row[0];
            }
        }
    }

    $condition = "WHERE corpus='$corpus' AND version='$version' AND document='$document' AND sentID='$sentID'";
    $results = $IdxDBH->query("SELECT id FROM sentindex $condition");
    if ($results){
        while ($row = $results->fetchArray(SQLITE3_NUM)) {
            return $row[0];
        }
    }
    return 0;
}


function get_sentence_ids($IdxDBH, $corpus, $version, $document, $sentIDs, $docID=0){
    $ids = array();
    foreach ($sentIDs as $sentID){
        if ($sentID)
            array_push($ids,get_sentence_id($IdxDBH, $corpus, $version, $document, $sentID, $docID));
    }
    return $ids;
}
                       

// get a sentence from the index using the OPUS sentence ID
// - retrieve the internal sentence ID from the function above
// - fetch the sentence from the DB

function get_sentence($SentDBH, $IdxDBH, $corpus, $version, $document, $sentID, $docID=0){
    if (!$sentID) return '';
    $id = get_sentence_id($IdxDBH, $corpus, $version, $document, $sentID, $docID);
    return fetch_sentence($SentDBH, $id);
}


## could also use this query to see whether the documents table exists:
## SELECT name FROM sqlite_master WHERE name='documents'

function get_source_document_id($corpus, $version, $document){
    global $srcIdxDbFile, $srcIdxDBH;
    global $DB_DIR,$srclang;
    if ( $srcIdxDbFile == $DB_DIR.$srclang.'.ids.db' ){
        $condition = "WHERE corpus='$corpus' AND version='$version' AND document='$document'";
        $results = $srcIdxDBH->query("SELECT rowid FROM documents $condition");
        if ($results){
            while ($row = $results->fetchArray(SQLITE3_NUM)) {
                return $row[0];
            }
        }
    }
    return 0;
}

function get_target_document_id($corpus, $version, $document){
    global $trgIdxDbFile, $trgIdxDBH;
    global $DB_DIR,$trglang;
    if ( $trgIdxDbFile == $DB_DIR.$trglang.'.ids.db' ){
        $condition = "WHERE corpus='$corpus' AND version='$version' AND document='$document'";
        $results = $trgIdxDBH->query("SELECT rowid FROM documents $condition");
        if ($results){
            while ($row = $results->fetchArray(SQLITE3_NUM)) {
                return $row[0];
            }
        }
    }
    return 0;
}


// set handle for link DB and file name for ratings DB
// --> either user-specific or leave it as the global one

function set_link_db($bitextID){
    global $DB_DIR, $langpair;
    global $linkDbFile, $algDbFile, $algStarsDbFile, $linksDBH;
    global $modifiedBitextExists, $modifiedBitext;
    global $showModified, $tableStyle;
    
    $USER_DB_DIR = $DB_DIR.$_SESSION['user'].'/';
    $userLinkDbFile = $USER_DB_DIR.$langpair.'.db';
    $userAlgStarsDbFile = $USER_DB_DIR.$langpair.'.stars.db';

    if (is_dir($USER_DB_DIR)){
        if (file_exists($userLinkDbFile) ){
            $userLinksDBH = new SQLite3($userLinkDbFile);
            $results = $userLinksDBH->query("SELECT COUNT(*) FROM links WHERE bitextID=$bitextID");
            if ($results){
                $row = $results->fetchArray(SQLITE3_NUM);
                if ( $row[0] ){
                    // echo("... set to user DBs ($userAlgStarsDbFile)");
                    $modifiedBitextExists = true;
                    if ($tableStyle == 'edit' || $showModified){
                        $linksDBH = $userLinksDBH;
                        $linkDbFile = $userLinkDbFile;
                        $algStarsDbFile = $userAlgStarsDbFile;
                        $modifiedBitext = true;
                    }
                }
            }
        }
    }
}




// create a copy of the bitext in the alignment database
// create a user-specific DB for alignment ratings

function initialize_user_link_db($bitextID){
    global $DB_DIR, $langpair, $linkDbFile, $linksDBH, $algStarsDbFile;
    global $modifiedBitextExists, $modifiedBitext;
    
    $USER_DB_DIR = $DB_DIR.$_SESSION['user'].'/';
    if (! is_dir($USER_DB_DIR)) mkdir($USER_DB_DIR);

    $userAlgDbFile = $USER_DB_DIR.$langpair.'.db';
    $userAlgStarsDbFile = $USER_DB_DIR.$langpair.'.stars.db';

    if (! file_exists($userAlgDbFile) ){
        $DBH = new SQLite3($userAlgDbFile);

        ## links table

        $DBH->exec('CREATE TABLE IF NOT EXISTS links ( bitextID, srcIDs TEXT, trgIDs TEXT, 
                                                              srcSentIDs, trgSentIDs, alignType TEXT,
                                                              alignerScore REAL, cleanerScore REAL, linkID INTEGER)');
        $DBH->exec('CREATE UNIQUE INDEX IF NOT EXISTS idx_links ON links ( bitextID, srcIDs, trgIDs )');
        $DBH->exec('CREATE UNIQUE INDEX IF NOT EXISTS idx_linkids ON links ( linkID )');
        $DBH->exec('CREATE INDEX IF NOT EXISTS idx_aligntype ON links ( bitextID, alignType )');
        $DBH->exec('CREATE INDEX IF NOT EXISTS idx_bitextid ON links ( bitextID )');
        $DBH->exec("ATTACH DATABASE '".$linkDbFile."' AS org");
        $columns = 'bitextID, srcIDs, trgIDs, srcSentIDs, trgSentIDs, alignType, alignerScore, cleanerScore, linkID';
        $DBH->exec("INSERT OR IGNORE INTO links SELECT $columns FROM org.links WHERE bitextID=$bitextID");

        ## linked source and target tables

        $DBH->exec('CREATE TABLE IF NOT EXISTS linkedsource ( sentID INTEGER, linkID INTEGER, bitextID INTEGER, PRIMARY KEY(linkID,sentID) )');
        $DBH->exec('CREATE TABLE IF NOT EXISTS linkedtarget ( sentID INTEGER, linkID INTEGER, bitextID INTEGER, PRIMARY KEY(linkID,sentID) )');
        $DBH->exec('CREATE INDEX IF NOT EXISTS idx_linkedsource_bitext ON linkedsource (bitextID,sentID)');
        $DBH->exec('CREATE INDEX IF NOT EXISTS idx_linkedtarget_bitext ON linkedtarget (bitextID,sentID)');
        $DBH->exec('CREATE INDEX IF NOT EXISTS idx_linkedsource_linkid ON linkedsource (linkID)');
        $DBH->exec('CREATE INDEX IF NOT EXISTS idx_linkedtarget_linkid ON linkedtarget (linkID)');
        $DBH->exec('CREATE INDEX IF NOT EXISTS idx_linkedsource_sentid ON linkedsource (sentID)');
        $DBH->exec('CREATE INDEX IF NOT EXISTS idx_linkedtarget_sentid ON linkedtarget (sentID)');
        $DBH->exec('PRAGMA journal_mode=WAL');
        
        $DBH->exec("ATTACH DATABASE '".$linkDbFile."' AS org");
        $DBH->exec("INSERT OR IGNORE INTO linkedsource 
                                   SELECT * FROM org.linkedsource WHERE bitextID=$bitextID ORDER BY rowid");
        $DBH->exec("INSERT OR IGNORE INTO linkedtarget 
                                   SELECT * FROM org.linkedtarget WHERE bitextID=$bitextID ORDER BY rowid");
        $DBH->close();
    }
    else{
        $DBH = new SQLite3($userAlgDbFile);
        $results = $DBH->query("SELECT COUNT(*) FROM links WHERE bitextID=$bitextID");
        if ($results){
            $row = $results->fetchArray(SQLITE3_NUM);
            if (! $row[0] ){
                $DBH->exec("ATTACH DATABASE '".$linkDbFile."' AS org");
                $columns = 'bitextID, srcIDs, trgIDs, srcSentIDs, trgSentIDs, alignType, alignerScore, cleanerScore, linkID';
                $DBH->exec("INSERT OR IGNORE INTO links SELECT $columns FROM org.links WHERE bitextID=$bitextID");
            }
        }
        
        $resultsSrc = $DBH->query("SELECT COUNT(*) FROM linkedsource WHERE bitextID=$bitextID");
        $resultsTrg = $DBH->query("SELECT COUNT(*) FROM linkedtarget WHERE bitextID=$bitextID");
        if ($resultsSrc && $resultsTrg){
            $rowSrc = $resultsSrc->fetchArray(SQLITE3_NUM);
            $rowTrg = $resultsTrg->fetchArray(SQLITE3_NUM);
            if (! $rowSrc[0]  && ! $rowTrg[0]){
                $DBH->exec("ATTACH DATABASE '".$linkDbFile."' AS org");
                $DBH->exec("INSERT OR IGNORE INTO linkedsource 
                                           SELECT * FROM org.linkedsource WHERE bitextID=$bitextID ORDER BY rowid");
                $DBH->exec("INSERT OR IGNORE INTO linkedtarget 
                                           SELECT * FROM org.linkedtarget WHERE bitextID=$bitextID ORDER BY rowid");
            }
        }
        $DBH->close();
    }

    if (! file_exists($userAlgStarsDbFile) ){
        $DBH = new SQLite3($userAlgStarsDbFile);
        $DBH->exec('CREATE TABLE IF NOT EXISTS ratings (bitextID INTEGER, linkID INTEGER, user TEXT, rating INTEGER)');
        $DBH->exec('CREATE UNIQUE INDEX IF NOT EXISTS idx_ratings ON ratings (bitextID, linkID, user)');
        $DBH->exec("ATTACH DATABASE '".$algStarsDbFile."' AS org");
        $DBH->exec("INSERT OR IGNORE INTO ratings SELECT * FROM org.ratings WHERE bitextID=$bitextID");
        $DBH->close();
    }
    else{
        $DBH = new SQLite3($userAlgStarsDbFile);
        $results = $DBH->query("SELECT COUNT(*) FROM ratings WHERE bitextID=$bitextID");
        if ($results){
            $row = $results->fetchArray(SQLITE3_NUM);
            if (! $row[0] ){
                $DBH->exec("ATTACH DATABASE '".$algStarsDbFile."' AS org");
                $DBH->exec("INSERT OR IGNORE INTO ratings SELECT * FROM org.ratings WHERE bitextID=$bitextID");
            }
        }
        $DBH->close();
    }

    $modifiedBitext = true;
    $modifiedBitextExists = true;

    $linkDbFile = $userAlgDbFile;
    $algStarsDbFile = $userAlgStarsDbFile;
    $linksDBH = new SQLite3($userAlgDbFile,SQLITE3_OPEN_READONLY);
    
    return $linkDbFile;
}








// delete source sentence ID from link

function delete_from_link($linkDbFile, $linkID, $sentID, $side='source'){
    if (! $linkDbFile) return;
    $DBH = new SQLite3($linkDbFile);

    // set the keys for selecting the correct table fields
    // depending on whether we change the source or the target side of the link
    
    if ($side == 'target'){
        $toIdKey = 'srcIDs';
        $fromIdKey = 'trgIDs';
        $fromSentIdKey = 'trgSentIDs';
    }
    else{
        $toIdKey = 'trgIDs';
        $fromIdKey = 'srcIDs';
        $fromSentIdKey = 'srcSentIDs';
    }

    $deletedSentId = 0;
    
    $results = $DBH->query("SELECT srcIDs,trgIDs,srcSentIDs,trgSentIDs FROM links WHERE linkID=$linkID");
    if ($results){
        if ($row = $results->fetchArray(SQLITE3_ASSOC)){
            $oldLinkFromIDs     = explode(' ',trim($row[$fromIdKey]));
            $oldLinkToIDs       = explode(' ',trim($row[$toIdKey]));
            $oldLinkFromSentIDs = explode(' ',trim($row[$fromSentIdKey]));

            if (! count($oldLinkFromIDs)){
                $DBH->close();
                return 0;
            }
            
            // TODO: fix the problem if sentence IDs are missing
            if (count($oldLinkFromIDs) != count($oldLinkFromSentIDs)){
                echo("Problem: number of OPUS and internal $side IDs do not match for link $linkID");
                // echo(var_dump($oldLinkFromIDs));
                // echo(var_dump($oldLinkFromSentIDs));
                // echo(var_dump($row));
                $DBH->close();
                return 0;
            }
            
            $oldFromSentIncluded = array();
            foreach ($oldLinkFromSentIDs as $id) $oldFromSentIncluded[$id] = true;

            // look for the OPUS sent ID to be removed
            // make sure to remove the corresponding internal sentence ID

            $newLinkFromIDs = array();
            $newLinkFromSentIDs = array();
            $newFromSentIncluded = array();
            for ( $x=0; $x<count($oldLinkFromIDs); $x++){
                if ($oldLinkFromIDs[$x] != $sentID){
                    array_push($newLinkFromIDs,$oldLinkFromIDs[$x]);
                    array_push($newLinkFromSentIDs,$oldLinkFromSentIDs[$x]);
                    $newSentIncluded[$oldLinkFromSentIDs[$x]] = true;
                }
                else{
                    $deletedSentId = $oldLinkFromSentIDs[$x];
                }
            }

            // update the record in the links table
            // or delete the entry in case it becomes empty
            
            $newLinkFrom = implode(' ',$newLinkFromIDs);
            $newLinkFromSent = implode(' ',$newLinkFromSentIDs);
            $algType = $side == 'source' ?
                     count($newLinkFromIDs).'-'.count($oldLinkToIDs) :
                     count($oldLinkToIDs).'-'.count($newLinkFromIDs) ;
            
            if ($newLinkFrom || count($oldLinkToIDs)){
                $DBH->exec("UPDATE links SET $fromIdKey='$newLinkFrom', $fromSentIdKey='$newLinkFromSent', 
                                             alignType='$algType', alignerScore=0, cleanerScore=0 
                            WHERE linkID=$linkID");
            }
            else{
                $DBH->exec("DELETE FROM links WHERE linkID=$linkID");
            }

            foreach ($oldLinkFromSentIDs as $sentID){
                if (!array_key_exists($sentID,$newSentIncluded)){
                    // echo("DELETE FROM linked$side WHERE linkID=$linkID AND sentID=$sentID<br/>");
                    $DBH->exec("DELETE FROM linked$side WHERE linkID=$linkID AND sentID=$sentID");
                }
            }
        }
    }
    $DBH->close();
    return $deletedSentId;
}



function delete_srcid_from_link($linkDbFile, $linkID, $sentID){
    return delete_from_link($linkDbFile, $linkID, $sentID, 'source');
}

function delete_trgid_from_link($linkDbFile, $linkID, $sentID){
    return delete_from_link($linkDbFile, $linkID, $sentID, 'target');
}





function add_to_link($linkDbFile, $bitextID, $linkID, $sentID, $id, $side='source', $push=true){
    if (! $linkDbFile) return;
    $DBH = new SQLite3($linkDbFile);

    // set the keys for selecting the correct table fields
    // depending on whether we change the source or the target side of the link

    if ($side == 'target'){
        $toIdKey = 'srcIDs';
        $fromIdKey = 'trgIDs';
        $toSentIdKey = 'trgSentIDs';
        $fromSentIdKey = 'trgSentIDs';
    }
    else{
        $toIdKey = 'trgIDs';
        $fromIdKey = 'srcIDs';
        $toSentIdKey = 'srcSentIDs';
        $fromSentIdKey = 'srcSentIDs';
    }

    $results = $DBH->query("SELECT srcIDs,trgIDs,srcSentIDs,trgSentIDs FROM links WHERE linkID=$linkID");
    if ($results){
        if ($row = $results->fetchArray(SQLITE3_ASSOC)){
            $newLinkToIDs       = explode(' ',trim($row[$toIdKey]));
            $newLinkFromIDs     = explode(' ',trim($row[$fromIdKey]));
            $newLinkFromSentIDs = explode(' ',trim($row[$fromSentIdKey]));
            
            if ($push){
                array_push($newLinkFromIDs,$sentID);
                array_push($newLinkFromSentIDs,$id);
            }
            else{
                array_unshift($newLinkFromIDs,$sentID);
                array_unshift($newLinkFromSentIDs,$id);
            }

            $newLinkFrom = implode(' ',$newLinkFromIDs);
            $newLinkFromSent = implode(' ',$newLinkFromSentIDs);
            $algType = $side == 'source' ?
                     count($newLinkFromIDs).'-'.count($newLinkToIDs) :
                     count($newLinkToIDs).'-'.count($newLinkFromIDs) ;

            $newLink = implode(' ',$newLinkIDs);
            $trgLinkIDs = explode(' ',$newLinkTo);
            $algType = count($newLinkIDs).'-'.count($trgLinkIDs);

            // echo("UPDATE links SET $fromIdKey='$newLinkFrom', $fromSentIdKey='$newLinkFromSent', alignType='$algType', alignerScore=0, cleanerScore=0 WHERE linkID=$linkID");
            $DBH->exec("UPDATE links SET $fromIdKey='$newLinkFrom', $fromSentIdKey='$newLinkFromSent', 
                                         alignType='$algType', alignerScore=0, cleanerScore=0 
                        WHERE linkID=$linkID");

            // echo("INSERT OR IGNORE INTO linked$side (linkID, bitextID, sentID) VALUES ($linkID, $bitextID, $id)<br/>");
            $DBH->exec("INSERT OR IGNORE INTO linked$side (linkID, bitextID, sentID) 
                        VALUES ($linkID, $bitextID, $id)");
            return $DBH->close();
        }
    }
    
    $algType = $side == 'source' ? '1-0' : '0-1';
    $DBH->exec("INSERT OR IGNORE INTO 
                       links (bitextID, $fromIdKey, $toIdKey, $fromSentIdKey, $toSentIdKey, 
                              alignType, alignerScore, cleanerScore, linkID) 
                VALUES ($bitextID, '$sentID', '', '$id', '', $algType, 0, 0, $linkID)");

    // echo("INSERT OR IGNORE INTO linked$side (linkID, bitextID, sentID) VALUES ($linkID, $bitextID, $id)<br/>");
    $DBH->exec("INSERT OR IGNORE INTO linked$side (linkID, bitextID, sentID) 
                        VALUES ($linkID, $bitextID, $id)");
    return $DBH->close();
}




function add_srcid_to_link($linkDbFile, $bitextID, $linkID, $sentID, $id, $push){
    return add_to_link($linkDbFile, $bitextID, $linkID, $sentID, $id, 'source', $push);
}

function add_trgid_to_link($linkDbFile, $bitextID, $linkID, $sentID, $id, $push){
    return add_to_link($linkDbFile, $bitextID, $linkID, $sentID, $id, 'target', $push);
}





// change links in link DB
// NOTE: this needs to correspond to internal sentence IDs (not OPUS sentence IDs)

function change_source_link($linkDbFile,$sentID,$oldLinkID,$newLinkID){
    if ($linkDbFile){
        $DBH = new SQLite3($linkDbFile);
        // echo("UPDATE OR IGNORE linkedsource SET linkID=$newLinkID WHERE linkID=$oldLinkID AND sentID=$sentID");
        $DBH->exec("UPDATE OR IGNORE linkedsource SET linkID=$newLinkID WHERE linkID=$oldLinkID AND sentID=$sentID");
        $DBH->close();
    }
}

function change_target_link($linkDbFile,$sentID,$oldLinkID,$newLinkID){
    if ($linkDbFile){
        $DBH = new SQLite3($linkDbFile);
        // echo("UPDATE OR IGNORE linkedtarget SET linkID=$newLinkID WHERE linkID=$oldLinkID AND sentID=$sentID");
        $DBH->exec("UPDATE OR IGNORE linkedtarget SET linkID=$newLinkID WHERE linkID=$oldLinkID AND sentID=$sentID");
        $DBH->close();
    }
}


function change_link($linkDbFile, $bitextID, $oldLinkID){

    if (isset($_GET['del-src'])){
        delete_srcid_from_link($linkDbFile, $oldLinkID, $_GET['del-src']);
        delete_alignment_rating($oldLinkID);
        // change_source_link($linkDbFile,$_GET['del-srcid'],$oldLinkID,0);
    }
    elseif (isset($_GET['del-trg'])){
        delete_trgid_from_link($linkDbFile, $oldLinkID, $_GET['del-trg']);
        delete_alignment_rating($oldLinkID);
        // change_target_link($linkDbFile,$_GET['del-trgid'],$oldLinkID,0);
    }
    elseif (isset($_GET['move-src'])){
        $newLinkID = $_GET['to'];
        $push = ($newLinkID < $oldLinkID);
        $sentID = delete_srcid_from_link($linkDbFile, $oldLinkID, $_GET['move-src']);
        add_srcid_to_link($linkDbFile, $bitextID, $newLinkID, $_GET['move-src'],$sentID, $push);
        // change_source_link($linkDbFile,$_GET['move-srcid'],$oldLinkID,$newLinkID);
        delete_alignment_rating($oldLinkID);
        delete_alignment_rating($newLinkID);
    }
    elseif (isset($_GET['move-trg'])){
        $newLinkID = $_GET['to'];
        $push = ($newLinkID < $oldLinkID);
        $sentID = delete_trgid_from_link($linkDbFile, $oldLinkID, $_GET['move-trg']);
        add_trgid_to_link($linkDbFile, $bitextID, $newLinkID, $_GET['move-trg'],$sentID, $push);
        // change_target_link($linkDbFile,$_GET['move-trgid'],$oldLinkID,$newLinkID);
        delete_alignment_rating($oldLinkID);
        delete_alignment_rating($newLinkID);
    }    
}


?>
