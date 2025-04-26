<?php // -*-php-*-

class opusindex{

    public $srclang;
    public $trglang;
    public $langpair;

    protected $dbDir;
    protected $user;
    protected $srcDbFile;
    protected $trgDbFile;
    protected $srcIdxDbFile;
    protected $trgIdxDbFile;

    protected $corpusDbFile;
    protected $bitextDbFile;
    protected $linkDbFile;
    protected $linkStarsDbFile;
    protected $linkRatingsDbFile;
    protected $userDataExists;

    protected $srcDBH;
    protected $trgDBH;
    protected $srcIdxDBH;
    protected $trgIdxDBH;
    protected $corpusDBH;
    protected $bitextDBH;
    protected $linksDBH;

    function __construct($dbDir,$user,$srclang,$trglang,$corpus='',$version='',$fromDoc='',$toDoc='',$opusLangpair=''){

        $langpair = implode('-',[$srclang,$trglang]);
        
        $this->dbDir          = $dbDir;
        $this->user           = $user;
        $this->srclang        = $srclang;
        $this->trglang        = $trglang;
        $this->langpair       = $langpair;

        $this->srcDbFile       = $this->__find_lang_dbfile($dbDir,$srclang);
        $this->trgDbFile       = $this->__find_lang_dbfile($dbDir,$trglang);
        $this->srcIdxDbFile    = $this->__find_langidx_dbfile($dbDir,$srclang);
        $this->trgIdxDbFile    = $this->__find_langidx_dbfile($dbDir,$trglang);
        
        $this->linkDbFile      = $this->__find_link_dbfile($dbDir,$langpair);
        $this->linkRatingsDbFile = $this->__find_ratings_dbfile($dbDir,$langpair);
        $this->linkStarsDbFile = $this->__find_stars_dbfile($dbDir,$langpair);
        $this->bitextDbFile    = $this->__find_bitext_dbfile($dbDir,$langpair);
        $this->corpusDbFile    = $this->__find_corpus_dbfile($dbDir);
        
        if ($this->srcDbFile)    $this->srcDBH    = new SQLite3($this->srcDbFile,SQLITE3_OPEN_READONLY);
        if ($this->trgDbFile)    $this->trgDBH    = new SQLite3($this->trgDbFile,SQLITE3_OPEN_READONLY);
        if ($this->srcIdxDbFile) $this->srcIdxDBH = new SQLite3($this->srcIdxDbFile,SQLITE3_OPEN_READONLY);
        if ($this->trgIdxDbFile) $this->trgIdxDBH = new SQLite3($this->trgIdxDbFile,SQLITE3_OPEN_READONLY);

        if ($this->corpusDbFile) $this->corpusDBH = new SQLite3($this->corpusDbFile,SQLITE3_OPEN_READONLY);
        if ($this->bitextDbFile) $this->bitextDBH = new SQLite3($this->bitextDbFile,SQLITE3_OPEN_READONLY);
        if ($this->linkDbFile)   $this->linksDBH  = new SQLite3($this->linkDbFile,SQLITE3_OPEN_READONLY);
    }

    public function setLinkDb($dbDir,$srclang,$trglang,$corpus='',$version='',$fromDoc='',$toDoc='',$opusLangpair=''){
        $langpair = implode('-',[$srclang,$trglang]);
        $this->linkDbFile = $this->__find_link_dbfile($dbDir,$langpair,$corpus,$version,$fromDoc,$toDoc,$opusLangpair);
        if ($this->linkDbFile) $this->linksDBH = new SQLite3($this->linkDbFile,SQLITE3_OPEN_READONLY);
    }

    public function userDataExists($bitextID){
        if (isset($this->userDataExists)) return $this->userDataExists;
        
        $linkdbfile = $this->dbDir.implode('/',[$this->user,$this->langpair.'.db']);
        if (file_exists($linkdbfile)){
            $DBH = new SQLite3($linkdbfile);
            $results = $DBH->query("SELECT rowid FROM links WHERE bitextID=$bitextID LIMIT 1");
            if ($results){
                if ($results->fetchArray(SQLITE3_NUM)){
                    $DBH->close();
                    $this->userDataExists = true;
                    return true;
                }
            }
        }
        $this->userDataExists = false;
        return false;
    }

    public function openUserDB($bitextID){
        if ($this->userDataExists($bitextID)){
            $linkdbfile = $this->dbDir.implode('/',[$this->user,$this->langpair.'.db']);
            $this->linkDbFile = $linkdbfile;
            $this->linksDBH->close();
            $this->linksDBH = new SQLite3($this->linkDbFile,SQLITE3_OPEN_READONLY);

            $dbfile = $this->dbDir.implode('/',[$this->user,$this->langpair.'.ratings.db']);
            $this->linkRatingsDbFile = $dbfile;

            $dbfile = $this->dbDir.implode('/',[$this->user,$this->langpair.'.stars.db']);
            $this->linkStarsDbFile = $dbfile;

        }
    }

    public function initializeUserDB($bitextID){

        $langpair = $this->langpair;
        $linkDbFile = $this->linkDbFile;
        $linkRatingsDbFile = $this->linkRatingsDbFile;
    
        $USER_DB_DIR = $this->dbDir.$this->user.'/';
        if (! is_dir($USER_DB_DIR)) mkdir($USER_DB_DIR);

        $userLinkDbFile = $USER_DB_DIR.$langpair.'.db';
        $userLinkRatingsDbFile = $USER_DB_DIR.$langpair.'.ratings.db';

        if (! file_exists($userLinkDbFile) ){
            $DBH = new SQLite3($userLinkDbFile);

            ## links table

            $DBH->exec('CREATE TABLE IF NOT EXISTS links ( bitextID, srcIDs TEXT, trgIDs TEXT, 
                                                           srcSentIDs, trgSentIDs, alignType TEXT,
                                                           alignerScore REAL, cleanerScore REAL, linkID INTEGER)');
            $DBH->exec('CREATE UNIQUE INDEX IF NOT EXISTS idx_links ON links ( bitextID, srcIDs, trgIDs )');
            $DBH->exec('CREATE UNIQUE INDEX IF NOT EXISTS idx_linkids ON links ( linkID )');
            $DBH->exec('CREATE INDEX IF NOT EXISTS idx_aligntype ON links ( bitextID, alignType )');
            $DBH->exec('CREATE INDEX IF NOT EXISTS idx_bitextid ON links ( bitextID )');
            $DBH->exec("ATTACH DATABASE '".$linkDbFile."' AS org");
            $columns = 'bitextID,srcIDs,trgIDs,srcSentIDs,trgSentIDs,alignType,alignerScore,cleanerScore,linkID';
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
            $DBH = new SQLite3($userLinkDbFile);
            $results = $DBH->query("SELECT COUNT(*) FROM links WHERE bitextID=$bitextID");
            if ($results){
                $row = $results->fetchArray(SQLITE3_NUM);
                if (! $row[0] ){
                    $DBH->exec("ATTACH DATABASE '".$linkDbFile."' AS org");
                    $columns = 'bitextID,srcIDs,trgIDs,srcSentIDs,trgSentIDs,alignType,alignerScore,cleanerScore,linkID';
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

        if (! file_exists($userLinkRatingsDbFile) ){
            $DBH = new SQLite3($userLinkRatingsDbFile);
            $DBH->exec('CREATE TABLE IF NOT EXISTS ratings (bitextID INTEGER, linkID INTEGER, user TEXT, rating INTEGER)');
            $DBH->exec('CREATE UNIQUE INDEX IF NOT EXISTS idx_ratings ON ratings (bitextID, linkID, user)');
            $DBH->exec("ATTACH DATABASE '".$linkRatingsDbFile."' AS org");
            $DBH->exec("INSERT OR IGNORE INTO ratings SELECT * FROM org.ratings WHERE bitextID=$bitextID");
            $DBH->exec('PRAGMA journal_mode=WAL');
            $DBH->close();
        }
        else{
            $DBH = new SQLite3($userLinkRatingsDbFile);
            $results = $DBH->query("SELECT COUNT(*) FROM ratings WHERE bitextID=$bitextID");
            if ($results){
                $row = $results->fetchArray(SQLITE3_NUM);
                if (! $row[0] ){
                    $DBH->exec("ATTACH DATABASE '".$linkRatingsDbFile."' AS org");
                    $DBH->exec("INSERT OR IGNORE INTO ratings SELECT * FROM org.ratings WHERE bitextID=$bitextID");
                }
            }
            $DBH->close();
        }

        $this->$userDataExists = true;
        $this->linkDbFile = $userLinkDbFile;
        $this->linkRatingsDbFile = $userLinkRatingsDbFile;
        $this->linksDBH = new SQLite3($userLinkDbFile,SQLITE3_OPEN_READONLY);
    
        return $linkDbFile;
    }

    function getBitext($bitextID){
        if (! isset($this->bitextDBH) )
            return array('corpus' => '', 'version' => '', 'fromDoc' => '', 'toDoc' => '');
        $results = $this->bitextDBH->query("SELECT * FROM bitexts WHERE rowid=".$bitextID);
        if ($results)
            if ($row=$results->fetchArray(SQLITE3_ASSOC))
                return $row;
        return array('corpus' => '', 'version' => '', 'fromDoc' => '', 'toDoc' => '');
    }
    
    public function getBitextID($corpus,$version,$fromDoc,$toDoc){
        if (! isset($this->bitextDBH) ) return 0;
        if ($corpus && $version && $fromDoc && $toDoc){
            $results = $this->bitextDBH->query("SELECT bitextID FROM bitexts WHERE corpus='".$corpus.
                                               "' AND version='".$version.
                                               "' AND fromDoc='".$fromDoc.
                                               "' AND toDoc='".$toDoc."'");
            if ($results){
                while ($row = $results->fetchArray(SQLITE3_NUM)) {
                    return $row[0];
                }
            }
        }
        return 0;
    }

    public function getCorpusID($corpus,$version,$srclang,$trglang){
        if (! isset($this->bitextDBH) ) return 0;
        if ($corpus && $version && $srclang && $trglang){
            $results = $this->bitextDBH->query("SELECT corpusID FROM corpora WHERE corpus='".$corpus.
                                               "' AND version='".$version.
                                               "' AND srclang='".$srclang.
                                               "' AND trglang='".$trglang."'");
            if ($results){
                while ($row = $results->fetchArray(SQLITE3_NUM)) {
                    return $row[0];
                }
            }
        }
        return 0;
    }

    // return all language pairs using the 3-letter language codes
    public function getLangpairs($corpus='', $version='', $srclang='', $trglang=''){
        if (! $this->corpusDBH) return array();
        $conditions = array();
        if ($corpus) array_push($conditions,"corpus='$corpus'");
        if ($version) array_push($conditions,"version='$version'");
        if ($srclang) array_push($conditions,"srclang='$srclang'");
        if ($trglang) array_push($conditions,"trglang='$trglang'");

        $condition = '';
        if (sizeof($conditions)){
            $condition = ' WHERE '.implode(' AND ',$conditions);
        }

        
        // echo("SELECT DISTINCT srclang3,trglang3 FROM corpora $condition");
        if ($this->bitextDBH)
            $results = $this->bitextDBH->query("SELECT DISTINCT srclang3,trglang3 FROM corpora $condition");
        else
            $results = $this->corpusDBH->query("SELECT DISTINCT srclang3,trglang3 FROM corpora $condition");

        $langpairs = array();
        if ($results){
            while ($row = $results->fetchArray(SQLITE3_NUM)) {
                array_push($langpairs,implode('-',$row));
            }
        }
        return $langpairs;
    }

    // return all language pairs using the internal OPUS language codes
    public function getOpusLangpairs($corpus='', $version='', $srclang='', $trglang=''){
        if (! $this->corpusDBH) return array();
        $conditions = array();
        if ($corpus) array_push($conditions,"corpus='$corpus'");
        if ($version) array_push($conditions,"version='$version'");
        if ($srclang) array_push($conditions,"srclang3='$srclang'");
        if ($trglang) array_push($conditions,"trglang3='$trglang'");

        $condition = '';
        if (sizeof($conditions)){
            $condition = ' WHERE '.implode(' AND ',$conditions);
        }

        
        // echo("SELECT DISTINCT srclang,trglang FROM corpora $condition");
        if ($this->bitextDBH)
            $results = $this->bitextDBH->query("SELECT DISTINCT srclang,trglang FROM corpora $condition");
        // $results = $this->corpusDBH->query("SELECT DISTINCT srclang,trglang FROM bitexts $condition");

        $langpairs = array();
        if ($results){
            while ($row = $results->fetchArray(SQLITE3_NUM)) {
                array_push($langpairs,implode('-',$row));
            }
        }
        return $langpairs;
    }

    public function getLink($linkID){
        $result = $this->linksDBH->query("SELECT * FROM links WHERE linkID=".$linkID);
        if ($result){
            while ($row = $result->fetchArray(SQLITE3_ASSOC)) {
                return $row;
            }
        }
        return array();
    }

    public function getSourceDocumentID($corpus, $version, $document){
        if (! $this->srcIdxDBH) return 0;
        $condition = "WHERE corpus='$corpus' AND version='$version' AND document='$document'";
        $results = $this->srcIdxDBH->query("SELECT rowid FROM documents $condition");
        if ($results){
            while ($row = $results->fetchArray(SQLITE3_NUM)) {
                return $row[0];
            }
        }
        return 0;
    }

    public function getTargetDocumentID($corpus, $version, $document){
        if (! $this->trgIdxDBH) return 0;
        $condition = "WHERE corpus='$corpus' AND version='$version' AND document='$document'";
        $results = $this->trgIdxDBH->query("SELECT rowid FROM documents $condition");
        if ($results){
            while ($row = $results->fetchArray(SQLITE3_NUM)) {
                return $row[0];
            }
        }
        return 0;
    }

    public function fetchSourceSentence($id){
        return $this->__fetchSentence($this->srcDBH,$id);
    }

    public function fetchTargetSentence($id){
        return $this->__fetchSentence($this->trgDBH,$id);
    }

    private function __fetchSentence($DBH,$id){
        if ($id){
            $results = $DBH->query("SELECT sentence FROM sentences WHERE rowid='$id'");
            if ($results){
                while ($row = $results->fetchArray(SQLITE3_NUM)) {
                    return $row[0];
                }
            }
        }
        // echo("SELECT sentence FROM sentences WHERE rowid='$id'");
        return 'SENTENCE NOT FOUND';
    }

    public function fetchLinkedSentenceIDs($linkID){
    
        $results = $this->linksDBH->query("SELECT sentID FROM linkedsource WHERE linkID=$linkID ORDER BY rowid");
        $srcIDs = array();
        if ($results){
            while ($row = $results->fetchArray(SQLITE3_NUM)) {
                array_push($srcIDs,$row[0]);
            }
        }
        $results = $this->linksDBH->query("SELECT sentID FROM linkedtarget WHERE linkID=$linkID ORDER BY rowid");
        $trgIDs = array();
        if ($results){
            while ($row = $results->fetchArray(SQLITE3_NUM)) {
                array_push($trgIDs,$row[0]);
            }
        }
        return array($srcIDs,$trgIDs);
    }

    public function getCorpora($corpus='', $version='', $srclang='', $trglang='', $opusSrclang='', $opusTrglang='', $latest=0){
        
        $conditions = array();
        // if ($corpus) array_push($conditions,"corpus='$corpus'");
        if ($version) array_push($conditions,"version='$version'");
        if ($srclang) array_push($conditions,"srclang3='$srclang'");
        if ($trglang) array_push($conditions,"trglang3='$trglang'");
        if ($opusSrclang) array_push($conditions,"srclang='$opusSrclang'");
        if ($opusTrglang) array_push($conditions,"trglang='$opusTrglang'");
        if ($latest) array_push($conditions,"latest=1");

        $condition = '';
        if (sizeof($conditions)) $condition = ' WHERE '.implode(' AND ',$conditions);
        
        if ($this->bitextDBH)
            return $this->bitextDBH->query("SELECT DISTINCT corpus FROM corpora $condition ORDER BY corpus");
        return $this->corpusDBH->query("SELECT DISTINCT corpus FROM corpora $condition ORDER BY corpus");
        
     }

    public function getCorpusReleases($corpus='', $version='', $srclang='', $trglang='', $opusSrclang='', $opusTrglang='', $latest=0){
        
        $conditions = array();
        if ($corpus) array_push($conditions,"corpus='$corpus'");
        if ($version) array_push($conditions,"version='$version'");
        if ($srclang) array_push($conditions,"srclang3='$srclang'");
        if ($trglang) array_push($conditions,"trglang3='$trglang'");
        if ($opusSrclang) array_push($conditions,"srclang='$opusSrclang'");
        if ($opusTrglang) array_push($conditions,"trglang='$opusTrglang'");
        if ($latest) array_push($conditions,"latest=1");

        $condition = '';
        if (sizeof($conditions)) $condition = ' WHERE '.implode(' AND ',$conditions);
        
        if ($this->bitextDBH)
            return $this->bitextDBH->query("SELECT DISTINCT corpus,version FROM corpora $condition ORDER BY corpus,version");
        return $this->corpusDBH->query("SELECT DISTINCT corpus,version FROM corpora $condition ORDER BY corpus,version");
        
     }

    
    public function getDocuments($corpus,$version,$srclang='',$trglang='',$opusSrclang='',$opusTrglang='',
                                 $fromDocQuery='',$toDocQuery='',$showMax=10,$offset=0){

        $conditions = array();
        if ($corpus) array_push($conditions,"corpus='$corpus'");
        if ($version) array_push($conditions,"version='$version'");
        // if ($srclang) array_push($conditions,"srclang3='$srclang'");
        // if ($trglang) array_push($conditions,"trglang3='$trglang'");
        if ($opusSrclang) array_push($conditions,"fromDoc LIKE '$opusSrclang/%'");
        if ($opusTrglang) array_push($conditions,"toDoc LIKE '$opusTrglang/%'");
        if ($fromDocQuery) array_push($conditions,"fromDoc LIKE '".$fromDocQuery."'");
        if ($toDocQuery) array_push($conditions,"toDoc LIKE '".$toDocQuery."'");

        
        $condition = '';
        if (sizeof($conditions)){
            $condition = ' WHERE '.implode(' AND ',$conditions);
        }
    
        $limit = " LIMIT ".$showMax;
        if ($offset) $limit .= " OFFSET ".$offset;
        // echo("SELECT DISTINCT fromDoc,toDoc,rowid FROM bitexts ".$condition.$limit);
        // return $this->corpusDBH->query("SELECT DISTINCT fromDoc,toDoc,rowid FROM bitexts ".$condition.$limit);
        return $this->bitextDBH->query("SELECT DISTINCT fromDoc,toDoc,bitextID FROM bitexts ".$condition.$limit);
    }
    
    public function getAlignments($bitextID, $type='all', $showMaxAlignments=10, $offset=0, $rangeStart=0, $rangeEnd=0){
        
        // TODO: avoid those globals
        global $showEmpty, $ALIGN_TYPES;

        $conditions = "bitextID=$bitextID";
        $orderby = ' ORDER BY linkID ';
        $limit = '';
        
        if ($type){
            // TODO: order by random becomes really slow on bigger queries!
            // --> try subqueries where we limit/offset search to some bigger value (maybe randomize that value)
            // --> problem: we probably need to know how many sentences we have in the bitext
            // --> and then select a smaller number of elements from that result ordered by random
            if ($type == 'random'){
                $orderby = ' ORDER BY RANDOM() ';
            }
            elseif ($type == 'other'){
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
        if ($showMaxAlignments || $offset){
            if ($showEmpty && ((! $type) || ($type == 'all'))){
                if (!$rangeStart) list($rangeStart,$rangeEnd) = $this->bitextRowIdRange($bitextID);
                if ($rangeStart){
                    $start = $rangeStart + $offset;
                    $end = $start + $showMaxAlignments;
                    $conditions = 'rowid>='.$start.' AND rowid<'.$end.' AND '.$conditions;
                }
                else{
                    $limit = "LIMIT $showMaxAlignments";
                    if ($offset) $limit .= " OFFSET $offset";
                }
            }
            else{
                $limit = "LIMIT $showMaxAlignments";
                if ($offset) $limit .= " OFFSET $offset";
            }
        }
        // echo("SELECT * FROM links WHERE $conditions $orderby $limit");
        return $this->linksDBH->query("SELECT * FROM links WHERE $conditions $orderby $limit");
    }

    public function bitextRowIdRange($bitextID){
        if ($res = $this->linksDBH->query("SELECT EXISTS (SELECT name FROM sqlite_master WHERE type='table' AND name='bitext_range')")){
            if ($row = $res->fetchArray(SQLITE3_NUM)){
                if ($row[0]){
                    if ($res = $this->linksDBH->query("SELECT start,end FROM bitext_range WHERE bitextID=".$bitextID)){
                        return $res->fetchArray(SQLITE3_NUM);
                    }
                }
            }
        }
        return array(0,0);
    }

    public function searchAlignments($searchquery, $searchside, $corpusID='', $bitextID='',
                                     $srclang='', $trglang='', $latest=1,
                                     $limit=10, $offset=0, $orderByLinkID=0){

        $srcquery = $searchside == 'search source' ? $searchquery : '';
        $trgquery = $searchside == 'search target' ? $searchquery : '';

        // build the query conditions
        $conditions = array();
        if ($searchquery) array_push($conditions, "sentence MATCH '".$searchquery."'");
        if ($bitextID)    array_push($conditions, 'bitextID='.$bitextID);
        if ($corpusID)    array_push($conditions, 'corpusID='.$corpusID);

        // if no specific corpus or bitext is selected:
        // --> search the entire collection
        // --> possibly restrict by language and latest releases only
        if ( (! $bitextID) && (! $corpusID)  ){
            $corpusConditions = array();
            if ($latest)  array_push($corpusConditions, 'latest=1');
            if ($srclang) array_push($corpusConditions, "srclang='".$srclang."'");
            if ($trglang) array_push($corpusConditions, "trglang='".$trglang."'");
            $corpusCondition = implode(' AND ',$corpusConditions);
            array_push($conditions, 'corpusID IN ( SELECT rowid FROM corpora WHERE '.$corpusCondition.' )');
        }

        $condition = implode(' AND ',$conditions);
        // echo(".... $condition ....");

        $ordering  = $orderByLinkID ? ' ORDER BY linkID ' : '';    
        $limitstr  = $limit         ? " LIMIT ".$limit    : '';
        $offsetstr = $offset        ? " OFFSET ".$offset  : '';

        if ($searchquery){
            if ($searchside == 'search target'){
                $this->linksDBH->exec("ATTACH DATABASE '".$this->trgDbFile."' AS trgdb");
                return $this->linksDBH->query(
                    "SELECT linkID, bitextID, sentID, highlight(sentences,0, '<b>', '</b>') sentence 
                            FROM trgdb.sentences as trg
                            INNER JOIN linkedtarget ON trg.rowid=linkedtarget.sentID
                            WHERE ".$condition.$ordering.$limitstr.$offsetstr);
            }
            else{
                $this->linksDBH->exec("ATTACH DATABASE '".$this->srcDbFile."' AS srcdb");
                return $this->linksDBH->query(
                    "SELECT linkID, bitextID, sentID, highlight(sentences,0, '<b>', '</b>') sentence 
                            FROM srcdb.sentences as src
                            INNER JOIN linkedsource ON src.rowid=linkedsource.sentID
                            WHERE ".$condition.$ordering.$limitstr.$offsetstr);
            }
        }
        else{
            return $this->linksDBH->query(
                "SELECT DISTINCT linkID, bitextID 
                        FROM linkedsource 
                        WHERE ".$condition.$ordering.$limitstr.$offsetstr);
        }
    }

    public function getRatedAlignments($bitextID=0, $minRating=0, $maxRating=0, $type='all', $limit=10, $offset=0){

        if (! file_exists($this->linkRatingsDbFile) ) return 0;
        $DBH = new SQLite3($this->linkRatingsDbFile,SQLITE3_OPEN_READONLY);
        $linkDB = $this->linkDbFile;
        $DBH->exec("ATTACH DATABASE '$linkDB' AS algdb");

        $conditions = array();
        array_push($conditions,'rating>'.$minRating);
        if ($maxRating) array_push($conditions,'rating<'.$maxRating);
        if ($bitextID)  array_push($conditions, 'links.bitextID='.$bitextID);
        
        if (count($conditions))
            $condition = ' WHERE '.implode(' AND ',$conditions);

        $limitstr  = $limit  ? " LIMIT ".$limit    : '';
        $offsetstr = $offset ? " OFFSET ".$offset  : '';

        return $DBH->query("SELECT links.bitextID as bitextID,links.linkID as linkID,srcIDs,trgIDs,alignType,alignerScore,user,rating 
                            FROM ratings 
                            INNER JOIN algdb.links AS links ON links.rowid=ratings.linkID 
                            $condition$limitstr$offsetstr");

    }

    public function getAlignmentRating($bitextID,$linkID,$user=''){
        
        // if (! file_exists($this->linkRatingsDbFile) ) return 0;
        if (! file_exists($this->linkRatingsDbFile) ){
            $stars = $this->getAlignmentStars($bitextID,$linkID,$user);
            if ($stars > 0 && $stars <=5) return 20*$stars-10;
            return $stars;
        }
        
        $DBH = new SQLite3($this->linkRatingsDbFile,SQLITE3_OPEN_READONLY);

        $conditions = array();
        if ($bitextID)  array_push($conditions, 'bitextID='.$bitextID);
        if ($linkID)    array_push($conditions, 'linkID='.$linkID);
        if ($user){
            if ($user == 'guest') $user .= clean_input($_SERVER['REMOTE_ADDR']);
            array_push($conditions, "user='$user'");
        }

        $condition = implode(' AND ',$conditions);
        // echo("SELECT rating FROM ratings WHERE ".$condition);

        if ($user)
            $results = $DBH->query("SELECT rating FROM ratings WHERE ".$condition);
        else
            $results = $DBH->query("SELECT AVG(rating) FROM ratings WHERE ".$condition);
            
        if ($results){
            while ($row = $results->fetchArray(SQLITE3_NUM)) {
                if ($row[0]){
                    $DBH->close();
                    return $row[0];
                }
            }
        }

        // try get stars instead of ranking
        
        $DBH->close();
        $stars = $this->getAlignmentStars($bitextID,$linkID,$user);
        if ($stars > 0 && $stars <= 5) return 20*$stars-10;
        return $stars;
    }

    
    // retrieve 5-value stars instead of 0-100 ratings
    // (compatibility with older version of the explorer interface)

    public function getAlignmentStars($bitextID,$linkID,$user=''){
        if (! file_exists($this->linkStarsDbFile) ) return 0;

        $DBH = new SQLite3($this->linkStarsDbFile,SQLITE3_OPEN_READONLY);

        $conditions = array();
        if ($bitextID)  array_push($conditions, 'bitextID='.$bitextID);
        if ($linkID)    array_push($conditions, 'linkID='.$linkID);
        if ($user){
            if ($user == 'guest') $user .= clean_input($_SERVER['REMOTE_ADDR']);
            array_push($conditions, "user='$user'");
        }

        $condition = implode(' AND ',$conditions);
        // echo("SELECT rating FROM ratings WHERE ".$condition);

        if ($user)
            $results = $DBH->query("SELECT rating FROM ratings WHERE ".$condition);
        else
            $results = $DBH->query("SELECT AVG(rating) FROM ratings WHERE ".$condition);
            
        if ($results){
            while ($row = $results->fetchArray(SQLITE3_NUM)) {
                if ($row[0]){
                    $DBH->close();
                    return $row[0];
                }
            }
        }
        $DBH->close();
        return 0;
    }


    public function getBitextRating($bitextID,$user=''){
        if (! $bitextID) return 0;
        // if (! file_exists($this->linkRatingsDbFile) ) return 0;
        if (! file_exists($this->linkRatingsDbFile) ){
            $stars = $this->getBitextStars($bitextID,$user);
            if ($stars > 0 && $stars <=5) return 20*$stars-10;
            return $stars;
        }
            
        $DBH = new SQLite3($this->linkRatingsDbFile,SQLITE3_OPEN_READONLY);

        $condition = 'bitextID='.$bitextID;
        if ($user){
            if ($user == 'guest') $user .= clean_input($_SERVER['REMOTE_ADDR']);
            $condition .= " AND user='".$user."'";
        }
        // echo $this->linkRatingsDbFile."SELECT AVG(rating) FROM ratings WHERE ".$condition.'<br/>';
        $results = $DBH->query("SELECT AVG(rating) FROM ratings WHERE ".$condition);
        if ($results){
            while ($row = $results->fetchArray(SQLITE3_NUM)) {
                if ($row[0]){
                    $DBH->close();
                    return $row[0];
                }
            }
        }
        $DBH->close();
        $stars = $this->getBitextStars($bitextID,$user);
        if ($stars > 0 && $stars <=5) return 20*$stars-10;
        return $stars;
    }


    public function getBitextStars($bitextID,$user=''){
        if (! $bitextID) return 0;
        if (! file_exists($this->linkStarsDbFile) ) return 0;
        $DBH = new SQLite3($this->linkStarsDbFile,SQLITE3_OPEN_READONLY);

        $condition = 'bitextID='.$bitextID;
        if ($user){
            if ($user == 'guest') $user .= clean_input($_SERVER['REMOTE_ADDR']);
            $condition .= " AND user='".$user."'";
        }
        // echo $this->linkRatingsDbFile."SELECT AVG(rating) FROM ratings WHERE ".$condition.'<br/>';
        $results = $DBH->query("SELECT AVG(rating) FROM ratings WHERE ".$condition);
        if ($results){
            while ($row = $results->fetchArray(SQLITE3_NUM)) {
                $DBH->close();
                return $row[0];
            }
        }
        $DBH->close();
        return 0;
    }


    public function addAlignmentRating($bitextID,$linkID,$user,$rating){

        // for guest users: add the referring IP address
        if ($user == 'guest') $user .= clean_input($_SERVER['REMOTE_ADDR']);
        
        // echo("add rating to $linkRatingsDbFile");
        // if (! file_exists($this->linkRatingsDbFile) ){
        if (! $this->linkRatingsDbFile ){
            $this->linkRatingsDbFile = $this->dbDir.$this->langpair.'.ratings.db';
            $DBH = new SQLite3($this->linkRatingsDbFile);
            
            $DBH->exec('CREATE TABLE IF NOT EXISTS ratings (bitextID INTEGER,linkID INTEGER,user TEXT,rating INTEGER)');
            $DBH->exec('CREATE UNIQUE INDEX IF NOT EXISTS idx_ratings ON ratings (bitextID, linkID, user)');
            $DBH->exec('PRAGMA journal_mode=WAL');
        }
        else{
            $DBH = new SQLite3($this->linkRatingsDbFile);
        }
        $DBH->exec("UPDATE OR IGNORE ratings SET rating=$rating 
                    WHERE bitextID=$bitextID AND linkID=$linkID AND user='$user'");
        $DBH->exec("INSERT OR IGNORE INTO ratings (bitextID, linkID, user, rating) 
                    VALUES ($bitextID,$linkID,'$user',$rating)");
        $DBH->close();
    }


    public function deleteAlignmentRating($linkID,$user=''){

        if ($user == 'guest') $user .= clean_input($_SERVER['REMOTE_ADDR']);
                
        if (file_exists($this->linkRatingsDbFile) ){
            $DBH = new SQLite3($this->linkRatingsDbFile);
            $condition = "linkID=$linkID";
            if ($user) $condition .= " AND user='$user'";
            // echo("DELETE FROM ratings WHERE $condition");
            $DBH->exec("DELETE FROM ratings WHERE $condition");
            $DBH->close();
        }
    }


    // delete source sentence ID from link

    public function deleteFromLink($linkID, $sentID, $side='source'){

        // make sure that we operate on the user link DB
        if (! $this->user) return false;
        $linkDbFile = $this->__find_user_link_dbfile($this->dbDir,$this->user,$this->langpair);
        if (! $linkDbFile) return false;
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
        // echo "SELECT srcIDs,trgIDs,srcSentIDs,trgSentIDs FROM links WHERE linkID=$linkID<br/>";
        $results = $DBH->query("SELECT srcIDs,trgIDs,srcSentIDs,trgSentIDs FROM links WHERE linkID=$linkID");
        if ($results){
            if ($row = $results->fetchArray(SQLITE3_ASSOC)){
                $oldLinkFromIDs     = $row[$fromIdKey]     ? explode(' ',trim($row[$fromIdKey])) : array();
                $oldLinkToIDs       = $row[$toIdKey]       ? explode(' ',trim($row[$toIdKey])) : array();
                $oldLinkFromSentIDs = $row[$fromSentIdKey] ? explode(' ',trim($row[$fromSentIdKey])) : array();

                if (! count($oldLinkFromIDs)){
                    $DBH->close();
                    return 0;
                }
            
                // TODO: fix the problem if sentence IDs are missing
                if (count($oldLinkFromIDs) != count($oldLinkFromSentIDs)){
                    echo("Problem: number of OPUS and internal $side IDs do not match for link $linkID");
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
                $algType = ($side == 'source') ?
                         count($newLinkFromIDs).'-'.count($oldLinkToIDs) :
                         count($oldLinkToIDs).'-'.count($newLinkFromIDs) ;
            
                if ($newLinkFrom || count($oldLinkToIDs)){
                    // echo "UPDATE links SET $fromIdKey='$newLinkFrom', $fromSentIdKey='$newLinkFromSent', alignType='$algType', alignerScore=0, cleanerScore=0 WHERE linkID=$linkID<br/>";
                    $DBH->exec("UPDATE links SET $fromIdKey='$newLinkFrom', $fromSentIdKey='$newLinkFromSent', 
                                                 alignType='$algType', alignerScore=0, cleanerScore=0 
                                WHERE linkID=$linkID");
                }
                else{
                    // echo("DELETE FROM links WHERE linkID=$linkID<br/>");
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

    public function deleteSrcIdFromLink($linkID, $sentID){
        return $this->deleteFromLink($linkID, $sentID, 'source');
    }

    public function deleteTrgIdFromLink($linkID, $sentID){
        return $this->deleteFromLink($linkID, $sentID, 'target');
    }


    // add a sentence to a link

    public function addToLink($bitextID, $linkID, $sentID, $id, $side='source', $push=true){

        // make sure that we operate on the user link DB
        if (! $this->user) return false;
        $linkDbFile = $this->__find_user_link_dbfile($this->dbDir,$this->user,$this->langpair);
        if (! $linkDbFile) return false;
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
                $newLinkToIDs       = $row[$toIdKey]       ? explode(' ',trim($row[$toIdKey])) : array();
                $newLinkFromIDs     = $row[$fromIdKey]     ? explode(' ',trim($row[$fromIdKey])) : array();
                $newLinkFromSentIDs = $row[$fromSentIdKey] ? explode(' ',trim($row[$fromSentIdKey])) : array();
            
                if ($push){
                    array_push($newLinkFromIDs,$sentID);
                    array_push($newLinkFromSentIDs,$id);
                }
                else{
                    array_unshift($newLinkFromIDs,$sentID);
                    array_unshift($newLinkFromSentIDs,$id);
                }

                $newLinkFrom = trim(implode(' ',$newLinkFromIDs));
                $newLinkFromSent = trim(implode(' ',$newLinkFromSentIDs));
                $algType = ($side == 'source') ?
                         count($newLinkFromIDs).'-'.count($newLinkToIDs) :
                         count($newLinkToIDs).'-'.count($newLinkFromIDs) ;

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


    public function addSrcIdToLink($bitextID, $linkID, $sentID, $id, $push){
        return $this->addToLink($bitextID, $linkID, $sentID, $id, 'source', $push);
    }

    public function addTrgIdToLink($bitextID, $linkID, $sentID, $id, $push){
        return $this->addToLink($bitextID, $linkID, $sentID, $id, 'target', $push);
    }


    


    protected function __find_lang_dbfile($dir,$lang){
        if (file_exists($dir.$lang.'.fts5.db')) return $dir.$lang.'.fts5.db';
        return '';
    }

    protected function __find_langidx_dbfile($dir,$lang){
        if (file_exists($dir.$lang.'.ids.db')) return $dir.$lang.'.ids.db';
        if (file_exists($dir.$lang.'.idx.db')) return $dir.$lang.'.idx.db';
        return '';
    }

    protected function __find_link_dbfile($dir,$langpair){
        if ($langpair){
            $linkdbfile = $dir.implode('/',['linkdb',$langpair.'.db']);
            if (file_exists($linkdbfile)) return $linkdbfile;
        }
        return '';
    }

    protected function __find_user_link_dbfile($dir,$user,$langpair){
        $dbfile = $dir.implode('/',[$user,$langpair.'.db']);
        if (file_exists($dbfile)) return $dbfile;
        return '';
    }

    protected function __find_alignment_dbfile($dir,$langpair){
        $dbfile = $dir.implode('/',['linkdb',$langpair.'.db']);
        if (file_exists($dbfile)) return $dbfile;
        return '';
    }

    protected function __find_ratings_dbfile($dir,$langpair){
        $dbfile = $dir.$langpair.'.ratings.db';
        if (file_exists($dbfile)) return $dbfile;
        return '';
    }

    protected function __find_user_ratings_dbfile($dir,$user,$langpair){
        $linkdbfile = $dir.implode('/',[$user,$langpair.'.ratings.db']);
        if (file_exists($linkdbfile)) return $dbfile;
        return '';
    }

    protected function __find_stars_dbfile($dir,$langpair){
        $dbfile = $dir.$langpair.'.stars.db';
        if (file_exists($dbfile)) return $dbfile;
        return '';
    }

    protected function __find_user_stars_dbfile($dir,$user,$langpair){
        $linkdbfile = $dir.implode('/',[$user,$langpair.'.stars.db']);
        if (file_exists($linkdbfile)) return $dbfile;
        return '';
    }
    
    protected function __find_bitext_dbfile($dir,$langpair){
        $dbfile = $dir.implode('/',['linkdb',$langpair.'.db']);
        if (file_exists($dbfile)) return $dbfile;
        return '';
    }

    protected function __find_corpus_dbfile($dir){
        $dbfile = $dir.implode('/',['linkdb','bitexts.db']);
        if (file_exists($dbfile)) return $dbfile;
        $dbfile = $dir.'bitexts.db';
        if (file_exists($dbfile)) return $dbfile;
        return '';
    }

}

?>
