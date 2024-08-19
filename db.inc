<?php // -*-php-*-


function get_lang_ftsfile($lang){
    global $DB_DIR;
    if (file_exists($DB_DIR.$lang.'.fts5.db')) return $DB_DIR.$lang.'.fts5.db';
    return '';
}

function get_lang_dbfile($lang){
    global $DB_DIR;
    if (file_exists($DB_DIR.$lang.'.fts5.db')) return $DB_DIR.$lang.'.fts5.db';
    if (file_exists($DB_DIR.$lang.'.db')) return $DB_DIR.$lang.'.db';
    return '';
}

function get_langidx_dbfile($lang){
    global $DB_DIR;
    if (file_exists($DB_DIR.$lang.'.ids.db')) return $DB_DIR.$lang.'.ids.db';
    if (file_exists($DB_DIR.$lang.'.idx.db')) return $DB_DIR.$lang.'.idx.db';
    return '';
}

$DB_DIR.$srclang.'.ids.db';


function get_link_dbfile($langpair,$corpus,$version,$fromDoc,$toDoc){
    global $DB_DIR;
    if ($langpair){
        if ($corpus && $version){
            $linkdbfile = $DB_DIR.implode('/',['sqlite',$corpus,$version,$langpair.'.linked.db']);
            if (file_exists($linkdbfile)) return $linkdbfile;
            // if a specific bitext is selected than the general index would also work
            // TODO: if we change to only include the latest version in that corpus
            //       then we also need to check the version of the corpus here!
            if ($fromDoc && $toDoc){
                $linkdbfile = $DB_DIR.implode('/',['sqlite',$langpair.'.linked.db']);
                if (file_exists($linkdbfile)) return $linkdbfile;
            }
        }
        else{
            $linkdbfile = $DB_DIR.implode('/',['sqlite',$langpair.'.linked.db']);
            if (file_exists($linkdbfile)) return $linkdbfile;
        }
    }
    return '';
}

function get_alignment_dbfile($langpair){
    global $DB_DIR;
    $dbfile = $DB_DIR.$langpair.'.db';
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
    $dbfile = $DB_DIR.'sqlite/'.$langpair.'.bitexts.db';
    if (file_exists($dbfile)) return $dbfile;
    if (file_exists($DB_DIR.$langpair.'.db')) return $DB_DIR.$langpair.'.db';
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
    global $algDBH;
    
    $results = $algDBH->query("SELECT rowid FROM bitexts WHERE corpus='$corpus' AND version='$version' AND fromDoc='$fromDoc' AND toDoc='$toDoc'");
    if ($results){
        while ($row = $results->fetchArray(SQLITE3_NUM)) {
            return $row[0];
        }
    }
    return undef;
}

function get_link($linkID){
    global $algDBH;
    if ($algDBH){
        $result = $algDBH->query("SELECT * FROM links WHERE rowid=".$linkID);
        if ($result){
            while ($row = $result->fetchArray(SQLITE3_ASSOC)) {
                return $row;
            }
        }
    }
    return array();
}

function get_alignments($algDBH, $corpus, $version, $fromDoc, $toDoc,
                        $fromDocID=0, $toDocID=0, $bitextID=0, $type='all', $offset=0){
    
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
    // echo("SELECT srcIDs,trgIDs,alignerScore,rowid FROM $table $conditions ORDER BY rowid $limit");
    return $algDBH->query("SELECT srcIDs,trgIDs,alignerScore,rowid FROM $table $conditions ORDER BY rowid $limit");
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
        foreach ($srcIDs as $srcID){
            array_push($srcSentIDs,get_sentence_id($srcIdxDBH, $corpus, $version, $fromDoc, $srcID, $fromDocID));
        }
        foreach ($trgIDs as $trgID){
            array_push($trgSentIDs,get_sentence_id($trgIdxDBH, $corpus, $version, $toDoc, $trgID, $toDocID));
        }
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
    global $linkDbFile, $algDbFile, $algStarsDbFile, $algDBH;
    global $modifiedBitextExists, $modifiedBitext;
    global $showModified, $tableStyle;
    
    $USER_DB_DIR = $DB_DIR.$_SESSION['user'].'/';
    $userLinkDbFile = $USER_DB_DIR.$langpair.'.linked.db';
    $userAlgDbFile = $USER_DB_DIR.$langpair.'.db';
    $userAlgStarsDbFile = $USER_DB_DIR.$langpair.'.stars.db';

    if (is_dir($USER_DB_DIR)){
        if (file_exists($userAlgDbFile) ){
            $userAlgDBH = new SQLite3($userAlgDbFile);
            $results = $userAlgDBH->query("SELECT COUNT(*) FROM links WHERE bitextID=$bitextID");
            if ($results){
                $row = $results->fetchArray(SQLITE3_NUM);
                if ( $row[0] ){
                    // echo("... set to user DBs ($userAlgStarsDbFile)");
                    $modifiedBitextExists = true;
                    if ($tableStyle == 'edit' || $showModified){
                        $algDBH = $userAlgDBH;
                        $algStarsDbFile = $userAlgStarsDbFile;
                        $modifiedBitext = true;
                        if (file_exists($userLinkDbFile) ){
                            $linkDbFile = $userLinkDbFile;
                        }
                    }
                }
            }
        }
    }
}



// create a copy of the bitext in the link database

function initialize_user_linked_db($bitextID){
    global $DB_DIR, $langpair, $linkDbFile;
    global $modifiedBitextExists, $modifiedBitext;
    
    // return empty string if the link DB does not exist
    // TODO: should we create it from the alignment DB?
    // PROBLEM: that is quite expensive especially for large documents!
    if (!$linkDbFile) return '';
    
    $USER_DB_DIR = $DB_DIR.$_SESSION['user'].'/';
    if (! is_dir($USER_DB_DIR)) mkdir($USER_DB_DIR);

    $userLinkDbFile = $USER_DB_DIR.$langpair.'.linked.db';

    if (! file_exists($userLinkDbFile) ){
        $userLinkDBH = new SQLite3($userLinkDbFile);
        $userLinkDBH->exec('CREATE TABLE IF NOT EXISTS linkedsource ( sentID INTEGER, linkID INTEGER, bitextID INTEGER, PRIMARY KEY(linkID,sentID) )');
        $userLinkDBH->exec('CREATE TABLE IF NOT EXISTS linkedtarget ( sentID INTEGER, linkID INTEGER, bitextID INTEGER, PRIMARY KEY(linkID,sentID) )');
        $userLinkDBH->exec('CREATE INDEX IF NOT EXISTS idx_linkedsource_bitext ON linkedsource (bitextID,sentID)');
        $userLinkDBH->exec('CREATE INDEX IF NOT EXISTS idx_linkedtarget_bitext ON linkedtarget (bitextID,sentID)');
        $userLinkDBH->exec('CREATE INDEX IF NOT EXISTS idx_linkedsource_linkid ON linkedsource (linkID)');
        $userLinkDBH->exec('CREATE INDEX IF NOT EXISTS idx_linkedtarget_linkid ON linkedtarget (linkID)');
        $userLinkDBH->exec('CREATE INDEX IF NOT EXISTS idx_linkedsource_sentid ON linkedsource (sentID)');
        $userLinkDBH->exec('CREATE INDEX IF NOT EXISTS idx_linkedtarget_sentid ON linkedtarget (sentID)');
        $userLinkDBH->exec('PRAGMA journal_mode=WAL');
        
        $userLinkDBH->exec("ATTACH DATABASE '".$linkDbFile."' AS org");
        $userLinkDBH->exec("INSERT OR IGNORE INTO linkedsource 
                                   SELECT * FROM org.linkedsource WHERE bitextID=$bitextID ORDER BY rowid");
        $userLinkDBH->exec("INSERT OR IGNORE INTO linkedtarget 
                                   SELECT * FROM org.linkedtarget WHERE bitextID=$bitextID ORDER BY rowid");
        $userLinkDBH->close();
    }
    else{
        $userLinkDBH = new SQLite3($userLinkDbFile);
        $resultsSrc = $userLinkDBH->query("SELECT COUNT(*) FROM linkedsource WHERE bitextID=$bitextID");
        $resultsTrg = $userLinkDBH->query("SELECT COUNT(*) FROM linkedtarget WHERE bitextID=$bitextID");
        if ($resultsSrc && $resultsTrg){
            $rowSrc = $resultsSrc->fetchArray(SQLITE3_NUM);
            $rowTrg = $resultsTrg->fetchArray(SQLITE3_NUM);
            if (! $rowSrc[0]  && ! $rowTrg[0]){
                $userLinkDBH->exec("ATTACH DATABASE '".$linkDbFile."' AS org");
                $userLinkDBH->exec("INSERT OR IGNORE INTO linkedsource 
                                           SELECT * FROM org.linkedsource WHERE bitextID=$bitextID ORDER BY rowid");
                $userLinkDBH->exec("INSERT OR IGNORE INTO linkedtarget 
                                           SELECT * FROM org.linkedtarget WHERE bitextID=$bitextID ORDER BY rowid");
            }
        }
        $userLinkDBH->close();
    }

    $linkDbFile = $userLinkDbFile;

    $modifiedBitext = true;
    $modifiedBitextExists = true;

    return $linkDbFile;
}



// create a copy of the bitext in the alignment database
// create a user-specific DB for alignment ratings

function initialize_user_link_db($bitextID){
    global $DB_DIR, $langpair, $algDBH, $algStarsDbFile;
    global $modifiedBitextExists, $modifiedBitext;
    
    $USER_DB_DIR = $DB_DIR.$_SESSION['user'].'/';
    if (! is_dir($USER_DB_DIR)) mkdir($USER_DB_DIR);

    $orgAlgDbFile = $DB_DIR.$langpair.'.db';
    $userAlgDbFile = $USER_DB_DIR.$langpair.'.db';
    $userAlgStarsDbFile = $USER_DB_DIR.$langpair.'.stars.db';

    if (! file_exists($userAlgDbFile) ){
        $userAlgDBH = new SQLite3($userAlgDbFile);

        $userAlgDBH->exec('CREATE TABLE IF NOT EXISTS links ( bitextID, srcIDs TEXT, trgIDs TEXT, alignType TEXT,
                                                              alignerScore REAL, cleanerScore REAL, rowid INTEGER)');
        $userAlgDBH->exec('CREATE UNIQUE INDEX IF NOT EXISTS idx_links ON links ( bitextID, srcIDs, trgIDs )');
        $userAlgDBH->exec('CREATE UNIQUE INDEX IF NOT EXISTS idx_linkids ON links ( rowid )');
        $userAlgDBH->exec('CREATE INDEX IF NOT EXISTS idx_aligntype ON links ( bitextID, alignType )');
        $userAlgDBH->exec('CREATE INDEX IF NOT EXISTS idx_bitextid ON links ( bitextID )');
        $userAlgDBH->exec("ATTACH DATABASE '".$orgAlgDbFile."' AS org");
        $columns = 'bitextID, srcIDs, trgIDs, alignType, alignerScore, cleanerScore, rowid';
        $userAlgDBH->exec("INSERT OR IGNORE INTO links SELECT $columns FROM org.links WHERE bitextID=$bitextID");
        // $userAlgDBH->exec("INSERT OR IGNORE INTO links SELECT * FROM org.links WHERE bitextID=$bitextID");
        // echo("INSERT OR IGNORE INTO links SELECT * FROM org.links WHERE bitextID=$bitextID<br/>");
    }
    else{
        $userAlgDBH = new SQLite3($userAlgDbFile);
        $results = $userAlgDBH->query("SELECT COUNT(*) FROM links WHERE bitextID=$bitextID");
        if ($results){
            $row = $results->fetchArray(SQLITE3_NUM);
            if (! $row[0] ){
                $userAlgDBH->exec("ATTACH DATABASE '".$orgAlgDbFile."' AS org");
                $columns = 'bitextID, srcIDs, trgIDs, alignType, alignerScore, cleanerScore, rowid';
                $userAlgDBH->exec("INSERT OR IGNORE INTO links SELECT $columns FROM org.links WHERE bitextID=$bitextID");
                // $userAlgDBH->exec("INSERT OR IGNORE INTO links SELECT * FROM org.links WHERE bitextID=$bitextID");
                // echo("INSERT OR IGNORE INTO links SELECT * FROM org.links WHERE bitextID=$bitextID<br/>");
            }
        }
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
        $results = $userAlgDBH->query("SELECT COUNT(*) FROM ratings WHERE bitextID=$bitextID");
        if ($results){
            $row = $results->fetchArray(SQLITE3_NUM);
            if (! $row[0] ){
                $DBH->exec("ATTACH DATABASE '".$algStarsDbFile."' AS org");
                $DBH->exec("INSERT OR IGNORE INTO ratings SELECT * FROM org.ratings WHERE bitextID=$bitextID");
            }
        }
        $DBH->close();
    }

    
    $algDBH = $userAlgDBH;
    $algStarsDbFile = $userAlgStarsDbFile;
    $modifiedBitext = true;
    $modifiedBitextExists = true;

    return $userAlgDBH;
}



// delete source sentence ID from link

function delete_srcid_from_link($DBH, $linkID, $sentID){
    $results = $DBH->query("SELECT * FROM links WHERE rowid=$linkID");
    if ($results){
        if ($row = $results->fetchArray(SQLITE3_NUM)){
            $oldLinkSrc = trim($row[1]);
            $oldLinkTrg = trim($row[2]);
            $oldLinkIDs = explode(' ',$oldLinkSrc);
            $newLinkIDs = array_diff($oldLinkIDs, array($sentID));
            $newLink = implode(' ',$newLinkIDs);
            $trgLinkIDs = explode(' ',$oldLinkTrg);
            $algType = count($newLinkIDs).'-'.count($trgLinkIDs);
            if ($newLink || $oldLinkTrg)
                $DBH->exec("UPDATE links SET srcIDs='$newLink', alignType='$algType', alignerScore=0, cleanerScore=0 WHERE rowid=$linkID");
            else
                $DBH->exec("DELETE FROM links WHERE rowid=$linkID");
            return;
        }
    }
}


// delete target sentence ID from link

function delete_trgid_from_link($DBH, $linkID, $sentID){
    $results = $DBH->query("SELECT * FROM links WHERE rowid=$linkID");
    if ($results){
        if ($row = $results->fetchArray(SQLITE3_NUM)){
            $oldLinkSrc = trim($row[1]);
            $oldLinkTrg = trim($row[2]);
            $oldLinkIDs = explode(' ',$oldLinkTrg);
            $newLinkIDs = array_diff($oldLinkIDs, array($sentID));
            $newLink = implode(' ',$newLinkIDs);
            $srcLinkIDs = explode(' ',$oldLinkSrc);
            $algType = count($srcLinkIDs).'-'.count($newLinkIDs);
            if ($newLink || $oldLinkSrc)
                $DBH->exec("UPDATE links SET trgIDs='$newLink', alignType='$algType', 
                                             alignerScore=0, cleanerScore=0 WHERE rowid=$linkID");
            else
                $DBH->exec("DELETE FROM links WHERE rowid=$linkID");
            return;
        }
    }
}


function add_srcid_to_link($DBH, $bitextID, $linkID, $sentID, $front){
    $results = $DBH->query("SELECT * FROM links WHERE rowid=$linkID");
    if ($results){
        if ($row = $results->fetchArray(SQLITE3_NUM)){
            $newLinkSrc = trim($row[1]);
            $newLinkTrg = trim($row[2]);
            
            $newLinkIDs = explode(' ',$newLinkSrc);
            if ($front)
                array_unshift($newLinkIDs,$sentID);
            else
                array_push($newLinkIDs,$sentID);
            $newLink = implode(' ',$newLinkIDs);
            $trgLinkIDs = explode(' ',$newLinkTrg);
            $algType = count($newLinkIDs).'-'.count($trgLinkIDs);
            return $DBH->exec("UPDATE links SET srcIDs='$newLink', alignType='$algType', 
                                                alignerScore=0, cleanerScore=0 WHERE rowid=$linkID");
        }
    }
    return $DBH->exec("INSERT OR IGNORE INTO 
                       links (bitextID, srcIDs, trgIDs, alignType, alignerScore, cleanerScore, rowid) 
                       VALUES ($bitextID, '$sentID', '', '1-0', 0, 0, $linkID)");
}


function add_trgid_to_link($DBH, $bitextID, $linkID, $sentID, $front){
    $results = $DBH->query("SELECT * FROM links WHERE rowid=$linkID");
    if ($results){
        if ($row = $results->fetchArray(SQLITE3_NUM)){
            $newLinkSrc = trim($row[1]);
            $newLinkTrg = trim($row[2]);
            
            $newLinkIDs = explode(' ',$newLinkTrg);
            if ($front)
                array_unshift($newLinkIDs,$sentID);
            else
                array_push($newLinkIDs,$sentID);
            $newLink = implode(' ',$newLinkIDs);
            $srcLinkIDs = explode(' ',$newLinkSrc);
            $algType = count($srcLinkIDs).'-'.count($newLinkIDs);
            return $DBH->exec("UPDATE links SET trgIDs='$newLink', alignType='$algType', 
                                                alignerScore=0, cleanerScore=0 WHERE rowid=$linkID");
        }
    }
    return $DBH->exec("INSERT OR IGNORE INTO 
                       links (bitextID, srcIDs, trgIDs, alignType, alignerScore, cleanerScore, rowid) 
                       VALUES ($bitextID, '', '$sentID', '0-1', 0, 0, $linkID)");
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


function change_link($DBH, $linkDbFile, $bitextID, $oldLinkID){

    if (isset($_GET['del-src'])){
        delete_srcid_from_link($DBH, $oldLinkID, $_GET['del-src']);
        delete_alignment_rating($oldLinkID);
        change_source_link($linkDbFile,$_GET['del-srcid'],$oldLinkID,0);
    }
    elseif (isset($_GET['del-trg'])){
        delete_trgid_from_link($DBH, $oldLinkID, $_GET['del-trg']);
        delete_alignment_rating($oldLinkID);
        change_target_link($linkDbFile,$_GET['del-trgid'],$oldLinkID,0);
    }
    elseif (isset($_GET['move-src'])){
        $newLinkID = $_GET['to'];
        $front = ($newLinkID > $oldLinkID);
        delete_srcid_from_link($DBH, $oldLinkID, $_GET['move-src']);
        add_srcid_to_link($DBH, $bitextID, $newLinkID, $_GET['move-src'],$front);
        change_source_link($linkDbFile,$_GET['move-srcid'],$oldLinkID,$newLinkID);
        delete_alignment_rating($oldLinkID);
        delete_alignment_rating($newLinkID);
    }
    elseif (isset($_GET['move-trg'])){
        $newLinkID = $_GET['to'];
        $front = ($newLinkID > $oldLinkID);
        delete_trgid_from_link($DBH, $oldLinkID, $_GET['move-trg']);
        add_trgid_to_link($DBH, $bitextID, $newLinkID, $_GET['move-trg'],$front);
        change_target_link($linkDbFile,$_GET['move-trgid'],$oldLinkID,$newLinkID);
        delete_alignment_rating($oldLinkID);
        delete_alignment_rating($newLinkID);
    }    
}


?>
