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
    
    protected $bitextDbFile;
    protected $linkDbFile;
    protected $linkStarsDbFile;
    protected $userDataExists;

    protected $srcDBH;
    protected $trgDBH;
    protected $srcIdxDBH;
    protected $trgIdxDBH;
    protected $bitextDBH;
    protected $linksDBH;

    function __construct($dbDir,$user,$srclang,$trglang,$corpus='',$version='',$fromDoc='',$toDoc=''){

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
        
        $this->linkDbFile      = $this->__find_link_dbfile($dbDir,$langpair,$corpus,$version,$fromDoc,$toDoc);
        $this->linkStarsDbFile = $this->__find_ratings_dbfile($dbDir,$langpair);
        $this->bitextDbFile    = $this->__find_bitext_dbfile($dbDir,$langpair);
        
        if ($this->srcDbFile)    $this->srcDBH    = new SQLite3($this->srcDbFile,SQLITE3_OPEN_READONLY);
        if ($this->trgDbFile)    $this->trgDBH    = new SQLite3($this->trgDbFile,SQLITE3_OPEN_READONLY);
        if ($this->srcIdxDbFile) $this->srcIdxDBH = new SQLite3($this->srcIdxDbFile,SQLITE3_OPEN_READONLY);
        if ($this->trgIdxDbFile) $this->trgIdxDBH = new SQLite3($this->trgIdxDbFile,SQLITE3_OPEN_READONLY);
        
        if ($this->bitextDbFile) $this->bitextDBH = new SQLite3($this->bitextDbFile,SQLITE3_OPEN_READONLY);
        if ($this->linkDbFile)   $this->linksDBH  = new SQLite3($this->linkDbFile,SQLITE3_OPEN_READONLY);
    }


    public function userDataExists($bitextID){
        if (isset($this->$userDataExists)) return $this->$userDataExists;
        
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
        }
        $dbfile = $this->dbDir.implode('/',[$this->user,$this->langpair.'.stars.db']);
        if (file_exists($dbfile)){
            $this->linkStarsDbFile = $dbfile;
        }
    }
    
    public function getBitextID($corpus,$version,$fromDoc,$toDoc){
        if (! isset($this->bitextDBH) ) return 0;
        $results = $this->bitextDBH->query("SELECT rowid FROM bitexts WHERE corpus='".$corpus.
                                           "' AND version='".$version.
                                           "' AND fromDoc='".$fromDoc.
                                           "' AND toDoc='".$toDoc."'");
        if ($results){
            while ($row = $results->fetchArray(SQLITE3_NUM)) {
                return $row[0];
            }
        }
        return 0;
    }

    public function getAlignments($bitextID, $type='all', $offset=0){
        
        // TODO: avoid those globals
        global $showEmpty, $showMaxAlignments, $ALIGN_TYPES;

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
        return $this->linksDBH->query("SELECT * FROM links $conditions ORDER BY linkID $limit");
    }


    public function getAlignmentRating($bitextID,$linkID,$user=''){
        if (! file_exists($this->linkStarsDbFile) ) return 0;

        $DBH = new SQLite3($this->linkStarsDbFile,SQLITE3_OPEN_READONLY);

        if ($user){
            if ($user == 'guest') $user .= clean_input($_SERVER['REMOTE_ADDR']);
            $results = $DBH->query("SELECT rating FROM ratings WHERE bitextID=$bitextID AND linkID=$linkID AND user='$user'");
            if ($results){
                while ($row = $results->fetchArray(SQLITE3_NUM)) {
                    return $row[0];
                }
            }
        }
        else{
            $results = $DBH->query("SELECT AVG(rating) FROM ratings WHERE bitextID=$bitextID AND linkID=$linkID");
            if ($results){
                while ($row = $results->fetchArray(SQLITE3_NUM)) {
                    return $row[0];
                }
            }
        }
        return 0;
    }

    public function getBitextRating($bitextID,$user=''){
        if (! file_exists($this->linkStarsDbFile) ) return 0;
        $DBH = new SQLite3($this->linkStarsDbFile,SQLITE3_OPEN_READONLY);

        $condition = 'bitextID='.$bitextID;
        if ($user){
            if ($user == 'guest') $user .= clean_input($_SERVER['REMOTE_ADDR']);
            $condition .= " AND user='".$user."'";
        }
        $results = $DBH->query("SELECT AVG(rating) FROM ratings WHERE ".$condition);
        if ($results){
            while ($row = $results->fetchArray(SQLITE3_NUM)) {
                return $row[0];
            }
        }
        return 0;
    }


    public function addAlignmentRating($bitextID,$linkID,$user,$rating){

        // for guest users: add the referring IP address
        if ($user == 'guest') $user .= clean_input($_SERVER['REMOTE_ADDR']);
    
        // echo("add rating to $algStarsDbFile");
        if (! file_exists($this->linkStarsDbFile) ){
            $DBH = new SQLite3($this->linkStarsDbFile);
            
            $DBH->exec('CREATE TABLE IF NOT EXISTS ratings (bitextID INTEGER, linkID INTEGER, user TEXT, rating INTEGER)');
            $DBH->exec('CREATE UNIQUE INDEX IF NOT EXISTS idx_ratings ON ratings (bitextID, linkID, user)');
        }
        else{
            $DBH = new SQLite3($this->linkStarsDbFile);
        }
        $DBH->exec("UPDATE OR IGNORE ratings SET rating=$rating WHERE bitextID=$bitextID AND linkID=$linkID AND user='$user'");
        $DBH->exec("INSERT OR IGNORE INTO ratings (bitextID, linkID, user, rating) VALUES ($bitextID,$linkID,'$user',$rating)");
        $DBH->close();
    }


    public function deleteAlignmentRating($linkID,$user=''){

        if (file_exists($this->linkStarsDbFile) ){
            $DBH = new SQLite3($this->linkStarsDbFile);
            $condition = "linkID=$linkID";
            if ($user) $condition .= "AND user='$user'";
            // echo("DELETE FROM ratings WHERE $condition");
            $DBH->exec("DELETE FROM ratings WHERE $condition");
            $DBH->close();
        }
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

    protected function __find_link_dbfile($dir,$langpair,$corpus,$version,$fromDoc,$toDoc){
        if ($langpair){
            if ($corpus && $version){
                $linkdbfile = $dir.implode('/',['sqlite',$corpus,$version,$langpair.'.db']);
                if (file_exists($linkdbfile)) return $linkdbfile;
                if ($fromDoc && $toDoc){
                    $linkdbfile = $dir.implode('/',['sqlite',$langpair.'.db']);
                    if (file_exists($linkdbfile)) return $linkdbfile;
                }
            }
            else{
                $linkdbfile = $dir.implode('/',['sqlite',$langpair.'.db']);
                if (file_exists($linkdbfile)) return $linkdbfile;
            }
        }
        return '';
    }

    protected function __find_user_link_dbfile($dir,$user,$langpair){
        $linkdbfile = $dir.implode('/',[$user,$langpair.'.db']);
        if (file_exists($linkdbfile)) return $dbfile;
        return '';
    }

    protected function __find_alignment_dbfile($dir,$langpair){
        $dbfile = $dir.implode('/',['sqlite',$langpair.'.db']);
        if (file_exists($dbfile)) return $dbfile;
        return '';
    }

    protected function __find_ratings_dbfile($dir,$langpair){
        $dbfile = $dir.$langpair.'.stars.db';
        if (file_exists($dbfile)) return $dbfile;
        return '';
    }

    protected function __find_user_ratings_dbfile($dir,$user,$langpair){
        $linkdbfile = $dir.implode('/',[$user,$langpair.'.stars.db']);
        if (file_exists($linkdbfile)) return $dbfile;
        return '';
    }

    protected function __find_bitext_dbfile($dir,$langpair){
        $dbfile = $dir.implode('/',['sqlite',$langpair.'.db']);
        if (file_exists($dbfile)) return $dbfile;
        return '';
    }
    
}

?>
