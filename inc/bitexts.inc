<?php // -*-php-*-


$ArrowUp      = '&#x2B06;';
$ArrowDown    = '&#x2B07;';
$EditSymbol   = '&#x270E;';
// $DeleteSymbol = '&#x232B;';
// $DeleteSymbol = '&#x2B0B;'; // South West Black Arrow
// $DeleteSymbol = '&#x10102;';
// $DeleteSymbol = '&#x24E7;'; // circle cross
// $DeleteSymbol = '&#x29BB;';
$DeleteSymbol = '&#x2A02;';

// other symbols: ⇑⇓
// see also https://www.alt-codes.net/arrow_alt_codes.php

class bitext {

    protected $index;
    protected $user;
    protected $corpusID;
    protected $bitextID;
    protected $fromDocID;
    protected $toDocID;
    
    public $corpus;
    public $version;
    public $langpair;
    public $srclang;
    public $trglang;
    public $fromDoc;
    public $toDoc;

    public $opusLangpairs;
    public $opusLangpair;
    public $opusSrclang;
    public $opusTrglang;


    function __construct($indexDir, $user, $corpus, $version, $langpair, $fromDoc, $toDoc, $opusLangpair='',
                         $showUserData=1, $showLatest=0) {
        $this->corpus = $corpus;
        $this->version = $version;
        $this->langpair = $langpair;
        $this->opusLangpair = $opusLangpair ? $opusLangpair : $langpair;
        $this->fromDoc = $fromDoc;
        $this->toDoc = $toDoc;
        $this->user = $user;

        if ($langpair){
            list($srclang,$trglang) = explode('-',$langpair);
            $this->srclang = $srclang;
            $this->trglang = $trglang;
        }

        $this->index = new opusindex($indexDir, $user, $srclang, $trglang,
                                     $corpus, $version,
                                     $fromDoc ,$toDoc, $opusLangpair);
        if ($fromDoc && $toDoc){
            $bitextID=$this->id();
            if ($showUserData && $bitextID) $this->index->openUserDB($bitextID);
        }
        
        // set version to latest version if not specified and showLatest is activated
        if ($corpus && ! $version && $showLatest){
            $releases = $this->releases(1);
            if ($release = $releases->next()){
                $version = $release['version'];
                $this->version = $version;
            }
        }

        // find all OPUS language pairs that exist for the selected srclang-trglang pair
        // and check that the given opusLangpair is one of them
        // if not: take the first valid language pair from the list
        $this->opusLangpairs = $this->index->getOpusLangpairs($corpus, $version, $srclang, $trglang);
        asort($this->opusLangpairs);
        if (sizeof($this->opusLangpairs)){
            if (! $opusLangpair){
                // echo("$this->opusLangpair = ".$this->opusLangpairs[0]);
                $this->opusLangpair = $this->opusLangpairs[0];
                $this->index->setLinkDb($indexDir,$srclang,$trglang,$corpus,$version,
                                        $fromDoc,$toDoc,$opusLangpair);
                if ($fromDoc && $toDoc){
                    $bitextID=$this->id();
                    if ($showUserData && $bitextID) $this->index->openUserDB($bitextID);
                }
            }
            elseif (! in_array($opusLangpair,$this->opusLangpairs)){
                $this->opusLangpair = $this->opusLangpairs[0];
                $this->index->setLinkDb($indexDir,$srclang,$trglang,$corpus,$version,
                                        $fromDoc,$toDoc,$opusLangpair);
            }
            else{
                $this->opusLangpair = $opusLangpair;
            }
        }

        // split the OPUS langpair string into source and target language
        if ($this->opusLangpair){
            list($this->opusSrclang,$this->opusTrglang) = explode('-',$this->opusLangpair);
        }
    }
            
    public function id(){
        if ($this->bitextID) return $this->bitextID;
        if ($id = get_param('bitextID',0)){
            $this->bitextID = $id;
        }
        else{
            $this->bitextID = $this->index->getBitextID($this->corpus,
                                                        $this->version,
                                                        $this->fromDoc,
                                                        $this->toDoc);
            set_param('bitextID',$this->bitextID);
        }
        return $this->bitextID;
    }

    public function corpusID(){
        if ($this->corpusID) return $this->corpusID;
        if ($corpusID = get_param('corpusID',0)){
            $this->corpusID = $corpusID;
        }
        else{
            $this->corpusID = $this->index->getCorpusID($this->corpus,
                                                        $this->version,
                                                        $this->opusSrclang,
                                                        $this->opusTrglang);
            set_param('corpusID',$this->corpusID);
        }
        return $this->corpusID;
    }

    
    public function fromDocID(){
        if ($this->fromDocID) return $this->fromDocID;
        if ($id = get_param('fromDocID')){
            $this->fromDocID = $id;
            set_param('fromDocID',$id);
        }
        return $this->fromDocID;
    }
    
    public function toDocID(){
        if ($this->toDocID) return $this->toDocID;
        if ($id = get_param('toDocID')){
            $this->toDocID = $id;
            set_param('toDocID',$id);
        }
        return $this->toDocID;
    }

    public function bitext($bitextID){
        return $this->index->getBitext($bitextID);
    }


    public function corpora($latest=0){
        $corpora = new corpora($this, $this->index);
        $corpora->fetch($latest);
        return $corpora;        
    }

    public function releases($latest=0){
        $corpora = new corpusReleases($this, $this->index);
        $corpora->fetch($latest);
        return $corpora;        
    }

    public function langpairs(){
        # return $this->index->getLangpairs($this->corpus, $this->version);
        return $this->index->getLangpairs($this->corpus, $this->version,
                                          $this->opusSrclang, $this->opusTrglang);
    }

    public function opusLangpairs(){
        return $this->index->getOpusLangpairs($this->corpus, $this->version,
                                              $this->srclang, $this->trglang);
    }
            
    public function documents($offset=0, $limit=10, $fromDocQuery='', $toDocQuery=''){
        $docpairs = new documentPairs($this, $this->index);
        $docpairs->fetch($fromDocQuery, $toDocQuery, $limit, $offset);
        return $docpairs;
    }

    public function getSourceDocumentID($document){
        return $this->index->getSourceDocumentID($this->corpus, $this->version, $document);
    }
    
    public function getTargetDocumentID($document){
        return $this->index->getTargetDocumentID($this->corpus, $this->version, $document);
    }
        
    public function alignments($type='all', $limit=10, $offset=0){
        $alignments = new alignments($this, $this->index);
        $alignments->fetch($type, $limit, $offset);
        return $alignments;
    }

    public function search($query, $side, $limit=10, $offset=0, $orderByLinkID=0){
        $alignments = new alignmentSearchResult($this, $this->index);
        $alignments->fetch($query,$side,$limit,$offset,$orderByLinkID);
        return $alignments;
    }

    public function userDataExists(){
        return $this->index->userDataExists($this->id());
    }

    public function getMyAlignmentRating($linkID){
        $user = $this->user;
        if ($user == 'guest') $user .= clean_input($_SERVER['REMOTE_ADDR']);
        return $this->index->getAlignmentRating($this->id(),$linkID,$user);
    }

    public function getAverageAlignmentRating($linkID){
        return $this->index->getAlignmentRating($this->id(),$linkID);
    }

    public function getMyBitextRating($bitextID){
        $user = $this->user;
        if ($user == 'guest') $user .= clean_input($_SERVER['REMOTE_ADDR']);
        if (!$bitextID) $bitextID=$this->id();
        return $this->index->getBitextRating($bitextID,$user);
    }

    public function getAverageBitextRating($bitextID){
        if (!$bitextID) $bitextID=$this->id();
        return $this->index->getBitextRating($bitextID);
    }

    // rating < 0 means deleting the existing rating
    public function addAlignmentRating($bitextID,$linkID,$rating){
        // if ($rating < 0) $rating = 0;
        if (!$bitextID) $bitextID=$this->id();
        if (!$bitextID){
            $link = $this->index->getLink($linkID);
            $bitextID = $link['bitextID'];
            // echo("....$bitextID");
        }
        $user = $this->user;
        if ($user){
            if ($rating < 0){
                return $this->index->deleteAlignmentRating($linkID,$user);
            }
            if ($user == 'guest') $user .= clean_input($_SERVER['REMOTE_ADDR']);
            return $this->index->addAlignmentRating($bitextID,$linkID,$user,$rating);
        }
        return false;
    }

    public function deleteAlignmentRating($linkID, $user=''){
        if ($user == 'guest') $user .= clean_input($_SERVER['REMOTE_ADDR']);
        return $this->index->deleteAlignmentRating($linkID,$user);
    }
    

    public function changeLink($oldLinkID, $actions){
        
        $bitextID = $this->id();
        if (! $bitextID) return false;
        if (! $userDbFile = $this->index->initializeUserDB($bitextID)) return false;

        if (isset($actions['del-src'])){
            $this->index->deleteSrcIdFromLink($oldLinkID, $actions['del-src']);
            $this->deleteAlignmentRating($oldLinkID);
        }
        elseif (isset($actions['del-trg'])){
            $this->index->deleteTrgIdFromLink($oldLinkID, $actions['del-trg']);
            $this->deleteAlignmentRating($oldLinkID);
        }
        elseif (isset($actions['move-src'])){
            $newLinkID = $actions['to'];
            $push = ($newLinkID < $oldLinkID);
            $sentID = $this->index->deleteSrcIdFromLink($oldLinkID, $actions['move-src']);
            $this->index->addSrcIdToLink($bitextID, $newLinkID, $actions['move-src'],$sentID, $push);
            $this->deleteAlignmentRating($oldLinkID);
            $this->deleteAlignmentRating($newLinkID);
        }
        elseif (isset($actions['move-trg'])){
            $newLinkID = $actions['to'];
            $push = ($newLinkID < $oldLinkID);
            $sentID = $this->index->deleteTrgIdFromLink($oldLinkID, $actions['move-trg']);
            $this->index->addTrgIdToLink($bitextID, $newLinkID, $actions['move-trg'],$sentID, $push);
            $this->deleteAlignmentRating($oldLinkID);
            $this->deleteAlignmentRating($newLinkID);
        }    
    }


}


class corpora {
    
    public $bitext;
    protected $index;
    protected $searchResult;

    function __construct($bitext,$index){
        $this->bitext = $bitext;
        $this->index = $index;
    }
    public function fetch($latest=0){
        $this->searchResults = $this->index->getCorpora($this->bitext->corpus, $this->bitext->version,
                                                        $this->bitext->srclang, $this->bitext->trglang,
                                                        $this->bitext->opusSrclang, $this->bitext->opusTrglang,
                                                        $latest);
    }
    public function next(){
        if ($this->searchResults)
            return $this->searchResults->fetchArray(SQLITE3_ASSOC);
        return false;
    }

}


class corpusReleases extends corpora{
    
    public $bitext;
    protected $index;
    protected $searchResult;

    function __construct($bitext,$index){
        $this->bitext = $bitext;
        $this->index = $index;
    }
    public function fetch($latest=0){
        $this->searchResults = $this->index->getCorpusReleases($this->bitext->corpus, $this->bitext->version,
                                                               $this->bitext->srclang, $this->bitext->trglang,
                                                               $this->bitext->opusSrclang, $this->bitext->opusTrglang,
                                                               $latest);
    }
    public function next(){
        if ($this->searchResults)
            return $this->searchResults->fetchArray(SQLITE3_ASSOC);
        return false;
    }

}

        
class documentPairs extends corpora{
    
    public function fetch($fromDocQuery='', $toDocQuery='', $limit=10, $offset=0){
        $corpus = $this->bitext->corpus;
        $version = $this->bitext->version;
        $this->searchResults = $this->index->getDocuments($this->bitext->corpus, $this->bitext->version,
                                                          $this->bitext->srclang, $this->bitext->trglang,
                                                          $this->bitext->opusSrclang, $this->bitext->opusTrglang,
                                                          $fromDocQuery, $toDocQuery,
                                                          $limit, $offset);
        // $this->searchResults = $this->index->getDocuments($corpus, $version,
        //                                                   $fromDocQuery, $toDocQuery,
        //                                                  $limit, $offset);
    }

    public function next(){
        if ($this->searchResults){
            if ($docpair = $this->searchResults->fetchArray(SQLITE3_ASSOC)){
                return $docpair;
            }
        }
        return false;
    }
}



class alignments extends documentPairs {
    
    public function fetch($type, $limit, $offset){
        if ($type == 'rated')
            $this->searchResults = $this->index->getRatedAlignments($this->bitext->id(),
                                                                    0, 0, $type, $limit, $offset);
        elseif ($type == 'ratings')
            $this->searchResults = $this->index->getAlignmentsWithRatings($this->bitext->id(),
                                                                    0, 0, $type, $limit, $offset);
        elseif ($type == 'stars')
            $this->searchResults = $this->index->getAlignmentsWithStars($this->bitext->id(),
                                                                    0, 0, $type, $limit, $offset);
        else
            $this->searchResults = $this->index->getAlignments($this->bitext->id(),
                                                               $type, $limit, $offset);
    }

    public function nextLink(){
        if ($this->searchResults){
            return $this->searchResults->fetchArray(SQLITE3_ASSOC);
        }
        return false;
    }

    public function next(){
        if ($this->searchResults){
            if ($link = $this->searchResults->fetchArray(SQLITE3_ASSOC)){

                if ($link['srcSentIDs'] || $link['trgSentIDs']){
                    $link['srcSentIDs'] = $link['srcSentIDs'] ? explode(' ',$link['srcSentIDs']) : array();
                    $link['trgSentIDs'] = $link['trgSentIDs'] ? explode(' ',$link['trgSentIDs']) : array();
                }
                else{
                    list($link['srcSentIDs'],
                         $link['trgSentIDs']) = $this->index->fetchLinkedSentenceIDs($link['linkID']);
                }

                $link['srcIDs'] = $link['srcIDs'] ? explode(' ',$link['srcIDs']) : array();
                $link['trgIDs'] = $link['trgIDs'] ? explode(' ',$link['trgIDs']) : array();
                $link['srcSents'] = array();
                $link['trgSents'] = array();

                foreach ($link['srcSentIDs'] as $id)
                    if ($id) array_push($link['srcSents'], $this->index->fetchSourceSentence($id));
                foreach ($link['trgSentIDs'] as $id)
                    if ($id) array_push($link['trgSents'], $this->index->fetchTargetSentence($id));
                
                return $link;
            }
        }
        return false;
    }
}


class ratedAlignments extends alignments {
    
    public function fetch($type, $limit, $offset){
        $this->searchResults = $this->index->getRatedAlignments($this->bitext->id(), $type, $limit, $offset);
    }
}





class alignmentSearchResult extends alignments {

    private $searchside;
    
    public function fetch($query, $side, $limit=10, $offset=0, $orderByLinkID=0){
        $this->searchResults = $this->index->searchAlignments($query, $side,
                                                              $this->bitext->corpusID(),
                                                              $this->bitext->id(),
                                                              $this->bitext->opusSrclang,
                                                              $this->bitext->opusTrglang,
                                                              1, $limit, $offset, $orderByLinkID);
        $this->searchside = $side;
    }

    public function next(){
        if ($this->searchResults){
            if ($data = $this->searchResults->fetchArray(SQLITE3_ASSOC)){
                if (! isset($data['linkID'])) return false;
                $link = $this->index->getLink($data['linkID']);
                
                if ($link['srcSentIDs'] || $link['trgSentIDs']){
                    $link['srcSentIDs'] = $link['srcSentIDs'] ? explode(' ',$link['srcSentIDs']) : array();
                    $link['trgSentIDs'] = $link['trgSentIDs'] ? explode(' ',$link['trgSentIDs']) : array();
                }
                else{
                    list($link['srcSentIDs'],
                         $link['trgSentIDs']) = $this->index->fetchLinkedSentenceIDs($link['linkID']);
                }

                $link['srcIDs'] = $link['srcIDs'] ? explode(' ',$link['srcIDs']) : array();
                $link['trgIDs'] = $link['trgIDs'] ? explode(' ',$link['trgIDs']) : array();
                $link['srcSents'] = array();
                $link['trgSents'] = array();


                if (isset($data['sentID'])) $sentID=$data['sentID'];
                if (isset($data['sentence'])) $sent=$data['sentence'];

                $link['srcSents'] = array();
                foreach ($link['srcSentIDs'] as $id){
                    if ($this->searchside == 'search source' && $id == $sentID)
                        array_push($link['srcSents'], $sent);
                    else
                        array_push($link['srcSents'], $this->index->fetchSourceSentence($id));
                }

                $link['trgSents'] = array();
                foreach ($link['trgSentIDs'] as $id){
                    if ($this->searchside == 'search target' && $id == $sentID)
                        array_push($link['trgSents'], $sent);
                    else
                        array_push($link['trgSents'], $this->index->fetchTargetSentence($id));
                }
                
                return $link;
            }
        }
        return false;
    }
}


function bitext_edit_links($bitext){
    global $tableStyle, $showModified, $allowEdit;

    if ($tableStyle == 'edit' && $allowEdit){
        $query = make_query(['style' => 'horizontal', 'showModified' => 1]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">stop editing</a>');
    }
    else{
        if ($bitext->userDataExists()){
            if ($showModified){
                $query = make_query(['showModified' => 0]);
                echo(' / modified / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">original</a>');
            }
            else{
                $query = make_query(['showModified' => 1]);
                echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">modified</a> / original');
            }
        }
        if ($allowEdit){
            $query = make_query(['style' => 'edit']);
            echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">edit mode</a>');
        }

    }
}

function bitext_aligntype_links($bitext, $selectedAlignType=''){
    global $tableStyle, $showModified, $allowEdit, $showEmpty;
    global $ALIGN_TYPES, $allowOtherAlignTypes, $allowRandomLinks;

    if ($tableStyle == 'edit') return;

    echo('<span id="aligntype-options">');
    if (! $selectedAlignType){
        echo(' / standard');
    }
    else{
        $query = make_query(['aligntype' => '', 'offset' => 0]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">standard</a>');
    }
    foreach ($ALIGN_TYPES as $type){
        if ($type == 'random' && ! $allowRandomLinks) continue;
        if ($selectedAlignType != $type){
            $query = make_query(['aligntype' => $type, 'offset' => 0]);
            echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">'.$type.'</a>');
        }
        else echo(' / '.$type);
    }
    if ($allowOtherAlignTypes){
        if ($selectedAlignType != 'other'){
            $query = make_query(['aligntype' => 'other', 'offset' => 0]);
            echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">other</a>');
        }
        else echo(' / other');
    }
    if ($showEmpty){
        $query = make_query(['showEmpty' => 0]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">hide empty</a>');
    }
    else{
        $query = make_query(['showEmpty' => 1]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">show empty</a>');
    }
    echo(' / </span>');
}



function bitext_browsing_links($bitext, $selectedAlignType=''){
    global $tableStyle, $showModified, $allowEdit, $showEmpty;
    global $SHOW_ALIGN_TYPES, $ALIGN_TYPES, $allowOtherAlignTypes;

    if ($tableStyle == 'edit' && $allowEdit){
        $query = make_query(['style' => 'horizontal', 'showModified' => 1]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">stop editing</a>');
    }
    else{
        if ($bitext->userDataExists()){
            if ($showModified){
                $query = make_query(['showModified' => 0]);
                echo(' / modified / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">original</a>');
            }
            else{
                $query = make_query(['showModified' => 1]);
                echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">modified</a> / original');
            }
        }
        if ($allowEdit){
            $query = make_query(['style' => 'edit']);
            echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">edit mode</a>');
        }

        if (! $SHOW_ALIGN_TYPES) return;
        
        foreach ($ALIGN_TYPES as $type){
            if ($selectedAlignType != $type){
                $query = make_query(['aligntype' => $type, 'offset' => 0]);
                echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">'.$type.'</a>');
            }
            else echo(' / '.$type);
        }
        if ($allowOtherAlignTypes){
            if ($selectedAlignType != 'other'){
                $query = make_query(['aligntype' => 'other', 'offset' => 0]);
                echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">other</a>');
            }
            else echo(' / other');
        }
        if ($showEmpty){
            $query = make_query(['showEmpty' => 0]);
            echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">hide empty</a>');
        }
        else{
            $query = make_query(['showEmpty' => 1]);
            echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">show empty</a>');
        }
    }
}



////////////////////
// print bitext display options
///////////////////

function bitext_display_options(){
    global $tableStyle;

    if ($tableStyle == 'vertical'){
        $query = make_query(['style' => 'horizontal']);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">horizontal</a>');
    }
    else{
        $query = make_query(['style' => 'vertical']);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">vertical</a>');
    }

    // bitext_score_options();
    // bitext_rating_options();
}


function bitext_score_options(){
    global $showScores, $showLengthRatio;

    // change visibility of entire columns in a table does not work on Safari nor on iOS
    //
    // echo(' / <span id="hide-score-columns" onclick="toggleVisibility(\'bitext-score-column\')">scores</span>');
    // echo(' / <span id="hide-score-columns" onclick="toggleVisibility(\'bitext-ratio-column\')">scores</span>');
    
    if ($showScores){
        $query = make_query(['showScores' => 0]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">scores</a>');
    }
    else{
        $query = make_query(['showScores' => 1]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'"><strike>scores</strike></a>');
    }
    if ($showLengthRatio){
        $query = make_query(['showLengthRatio' => 0]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">length ratio</a>');
    }
    else{
        $query = make_query(['showLengthRatio' => 1]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'"><strike>length ratio</strike></a>');
    }
}

function bitext_aligntype_menu(){
    global $showAlignTypes;

    echo(' <span id="aligntype-menu" onclick="toggleVisibility(\'aligntype-options\');">.&nbsp;.&nbsp;.</span>');

    /*
    if ($showAlignTypes){
        $query = make_query(['showAlignTypes' => 0]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">align&nbsp;types</a>');
    }
    else{
        $query = make_query(['showAlignTypes' => 1]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'"><strike>align&nbsp;types</strike></a>');
    }
    */
}


////////////////////
// print bitext navigation bar
///////////////////

function bitext_navigation($offset, $showMax, $showStart=1){

    $lastentry = $offset + $showMax;

    if ($offset){
        if ($showStart){
            $query = make_query(['offset' => 0]);
            echo('<a id="prev" href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">start</a> / ');
        }
        $previous = $offset - $showMax;
        if ($previous < 0){ $previous = 0; }
        $query = make_query(['offset' => $previous]);
        echo('<a id="prev" href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">prev</a> / ');
    }
    else{
        if ($showStart) echo('start / ');
        echo('<a id="prev">prev</a> / ');
        echo '<script>var x = document.getElementById("prev");x.style.visibility = "hidden";</script>';
    }
    $query = make_query(['offset' => $lastentry]);
    echo('<a id="next" href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'"> next</a>');

}

function bitext_rating($bitext){
    global $showMyRatings, $showRatings;
    if ($showMyRatings){
        echo(" / rate the overall bitext quality: ");
        // print_rating_stars($bitextID,0);
        printRatingStars($bitext,0);
    }
    if ($showRatings){
        echo(" (");
        // print_average_ratings($bitextID,0);
        printAverageRatings($bitext,0);
        echo(")");
    }
}


function print_bitext($bitext, $type='', $offset=0){
    global $showMaxAlignments, $showScores, $showLengthRatio, $showRatings, $showMyRatings, $showEmpty, $showAlignTypes;
    global $tableStyle, $allowEdit;

    if ($tableStyle == 'edit' && $allowEdit){
        $showEmpty = 1;
        $type = '';
        if (isset($_GET['changelink'])){
            $bitext->changeLink($_GET['changelink'],$_GET);
        }
    }

    $alignments = $bitext->alignments($type,$showMaxAlignments,$offset);
    print_bitext_table($bitext, $alignments, $offset);
}


function print_bitext_table($bitext, $alignments, $offset=0){
    global $showMaxAlignments, $showScores, $showLengthRatio,
        $showRatings, $showMyRatings,
        $showEmpty, $showAlignTypes;
    global $tableStyle, $allowEdit;


    if ($alignments){

        global $searchquery, $alignType;
        $showStart = $searchquery ? true : false;
        bitext_navigation($offset, $showMaxAlignments, $showStart);
        echo('<div class="rightalign">');
        bitext_display_options();
        echo('</div></br><hr>');
        // echo('<div class="rightalign">');
        bitext_score_options();
        bitext_rating_options();
        if (! $searchquery){
            echo(' / <span id="clickable" onclick="toggleVisibility(\'bitext-options\');">.&nbsp;.&nbsp;.</span>');
            echo('<span id="bitext-options">');
            bitext_edit_links($bitext, $alignType);
            if ($tableStyle != 'edit') bitext_aligntype_links($bitext, $alignType);
            echo('</span>');
        }
        // echo('</div><br/>');
        echo('<br/>');

        if ($tableStyle == 'edit' && $allowEdit){
            $shown = print_editable_bitext_table($alignments,
                                                 $showScores, $showLengthRatio,
                                                 $showMyRatings, $showRatings);
        }
        
        elseif ($tableStyle == 'vertical'){
            $shown = print_vertical_bitext_table($alignments,
                                                 $showScores, $showLengthRatio,
                                                 $showMyRatings, $showRatings);
        }

        elseif ($tableStyle == 'tsv'){
            echo("<pre>\n");
            $shown = print_bitext_tsv($alignments,
                                      $showScores, $showLengthRatio,
                                      $showMyRatings, $showRatings);
            echo('</pre>');
        }
        
        else{
            $shown = print_horizontal_bitext_table($alignments,
                                                   $showScores, $showLengthRatio,
                                                   $showMyRatings, $showRatings);
        }
        
        // hide next button if we are at the end of the document
        if ($shown < $showMaxAlignments){
            echo '<script>var x = document.getElementById("next");x.style.visibility = "hidden";</script>';
        }
    }
}




function bitext_link($bitext,$bitextID, $corpus, $fromDoc, $toDoc){
    if (! ($corpus && $version && $fromDoc && $toDoc) ){
        $info = $bitext->bitext($bitextID);
        $query = make_query(['corpus' => $info['corpus'],
                             'version' => $info['version'],
                             'fromDoc' => $info['fromDoc'],
                             'toDoc' => $info['toDoc'],
                             'bitextID' => $bitextID,
                             'search' => '','opusLangpair' => '',
                             'offset' => 0]);
        $hreftitle = implode('/',[$info['corpus'],$info['version'],$info['fromDoc']]);    
    }
    else{
        $query = make_query(['corpus' => $corpus,
                             'version' => $version,
                             'fromDoc' => $fromDoc,
                             'toDoc' => $toDoc,
                             'bitextID' => $bitextID,
                             'search' => '',
                             'offset' => 0]);
        $hreftitle = implode('/',[$corpus,$version,$fromDoc]);
    }
    return '<a title="'.$hreftitle.'" href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">'.$bitextID.'</a>';
}




function print_bitext_tsv($alignments,
                          $showScores=1, $showLengthRatio=1,
                          $showMyRatings=0, $showRatings=1, $showUser=0){

    // TODO: should pass on bitext instance to functions instead of those variables
    $bitext = $alignments->bitext;
    $corpus = $bitext->corpus;
    $version = $bitext->version;
    $fromDoc = $bitext->fromDoc;
    $toDoc = $bitext->toDoc;
    $bitextID = $bitext->id();

    echo("corpus\tversion\tfromDoc\ttoDoc\tsrcIDs\ttrgIDs\talignType");
    if ($showScores) echo("\tscore");
    if ($showLengthRatio) echo("\tlength ratio");
    if ($showRatings) echo("\trating");
    if ($showMyRatings) echo("\tmy rating");
    if ($showUser) echo("\tuser");
    echo("\tsrcSents\ttrgSents\n");
    
    $lastBitextID = '';

    $i=0;
    while ($row = $alignments->next()) {
        $i++;

        if ($bitextID != $row['bitextID']){
            if ($lastBitextID != $row['bitextID']){
                $info = $bitext->bitext($row['bitextID']);
                $corpus = $info['corpus'];
                $version = $info['version'];
                $fromDoc = $info['fromDoc'];
                $toDoc = $info['toDoc'];
                $lastBitextID = $row['bitextID'];
            }
        }

        $srcIDs = implode(' ',$row['srcIDs']);
        $trgIDs = implode(' ',$row['trgIDs']);
        
        echo("$corpus\t$version\t$fromDoc\t$toDoc");
        echo("\t".$srcIDs);
        echo("\t".$trgIDs);
        echo("\t".$row['alignType']);

        if ($showScores) echo("\t".$row['alignerScore']);
        if ($showLengthRatio){
            $srcLen = mb_strlen($srcText);
            $trgLen = mb_strlen($trgText);
            $ratio = 0;
            if ($srcLen or $trgLen){
                $ratio = $srcLen > $trgLen ? $trgLen / $srcLen : $srcLen / $trgLen;
            }
            echo("\t".$ratio);
        }
        if ($showRatings){
            if (array_key_exists('rating',$row)){
                echo("\t".$row['rating']);
            }
            else{
                $rating = $bitext->getAverageAlignmentRating($row['linkID']);
                echo("\t".$rating);
            }
        }
        if ($showMyRatings){
            $rating = $bitext->getMyAlignmentRating($row['linkID']);
            echo("\t".$rating);
        }
        if ($showUser){
            if (array_key_exists('user',$row)) echo("\t".$row['user']);
            else echo("\t");
        }
        $srcText = implode(' ',$row['srcSents']);
        $trgText = implode(' ',$row['trgSents']);

        echo("\t".$srcText);
        echo("\t".$trgText);
        echo("\n");
    }
    return $i;
}


function print_horizontal_bitext_table($alignments,
                                       $showScores=1, $showLengthRatio=1,
                                       $showMyRatings=0, $showRatings=1){

    // TODO: should pass on bitext instance to functions instead of those variables
    $bitext = $alignments->bitext;
    $corpus = $bitext->corpus;
    $version = $bitext->version;
    $fromDoc = $bitext->fromDoc;
    $toDoc = $bitext->toDoc;
    $bitextID = $bitext->id();

    global $langpair, $showModified;
    
    echo('<p><table class="hbitext" id="bitext">');
    echo('<colgroup>');
    echo('<col id="bitext-srcid-col"/>');
    echo('<col id="bitext-srctxt-col"/>');
    echo('<col id="bitext-score-column"/>');
    echo('<col id="bitext-ratio-col"/>');
    echo('<col id="bitext-trgtxt-col"/>');
    echo('<col id="bitext-trgid-col"/>');
    echo('</colgroup>');
    echo("<tr><th class='hbitext'>IDs</th>");
    echo("<th class='hbitext'>");
    echo($fromDoc.'&nbsp;&nbsp;');
    if ($fromDoc) print_search_form($langpair, $corpus, $version, $fromDoc, $toDoc, $bitextID, '', 1, 0);
    // echo("&nbsp;&nbsp;$fromDoc");
    echo("</th>");
    if ($showScores) echo('<th class="hbitext">score</th>');
    if ($showLengthRatio) echo('<th class="hbitext">ratio</th>');
    echo("<th class='hbitext'>".$toDoc.'&nbsp;&nbsp;');
    if ($toDoc) print_search_form($langpair, $corpus, $version, $fromDoc, $toDoc, $bitextID, '', 0, 1);
    echo("</th><th class='hbitext'>IDs</th>");
    // if ($showMyRatings) echo('<th>my rating</th>');
    if ($showRatings) echo('<th class="hbitext">rating</th>');
    elseif ($showMyRatings) echo('<th class="hbitext">my&nbsp;rating</th>');
    echo("</tr>");

    $lastBitextID = '';
    $bitextLink = '';
    $i = 0;
    
    while ($row = $alignments->next()) {
        $i++;

        $srcText = implode(' ',$row['srcSents']);
        $trgText = implode(' ',$row['trgSents']);

        // for search results: add a link to the bitext
        if ($bitextID != $row['bitextID']){
            if ($lastBitextID != $row['bitextID']){
                $bitextLink = bitext_link($bitext,$row['bitextID'], $corpus, $fromDoc, $toDoc);
                $lastBitextID = $row['bitextID'];
            }
        }

        echo('<tr><td class="hbitext-srcid">');
        if ($bitextLink) echo($bitextLink.'-');
        echo(implode('&nbsp;',$row['srcIDs']).'</td><td class="hbitext-src">');
        echo($srcText);

        if ($showScores){
            $score = $row['alignerScore'];
            $color = score_color($score);
            echo("</td><td bgcolor='$color' class='hbitext-score'>$score");
        }
        if ($showLengthRatio){
            $srcLen = mb_strlen($srcText);
            $trgLen = mb_strlen($trgText);
            $ratio = 0;
            if ($srcLen or $trgLen){
                $ratio = $srcLen > $trgLen ? $trgLen / $srcLen : $srcLen / $trgLen;
            }
            $color = score_color($ratio);
            $pretty_ratio = sprintf('%5.3f',$ratio);
            echo("</td><td bgcolor='$color' class='hbitext-ratio'>$pretty_ratio");
        }
            
        echo('</td><td class="hbitext-trg">');
        echo($trgText);
        echo('</td><td class="hbitext-trgid">');
        if ($bitextLink) echo($bitextLink.'-');
        echo(implode('&nbsp;',$row['trgIDs']));

        $textOK = ( strpos($srcText, 'SENTENCE NOT FOUND') === false &&
                    strpos($trgText, 'SENTENCE NOT FOUND') === false );

        /*
        if ($showMyRatings && $textOK){
            echo('</td><td class="centeralign">');
            // print_rating_stars($bitextID,$row['linkID']);
            printRatingStars($bitext,$row['linkID']);
        }
        */
        if ($showRatings && $textOK){
            echo('</td><td class="hbitext-rating">');
            printAverageRatings($bitext,$row['linkID']);
        }
        elseif ($showMyRatings && $textOK){
            echo('</td><td class="hbitext-rating">');
            printRatingStars($bitext,$row['linkID']);
        }

            
        echo('</td></tr>'."\n");
    }
    echo('</table></p>');
    return $i;
}





function print_vertical_bitext_table($alignments,
                                     $showScores=1, $showLengthRatio=1,
                                     $showMyRatings=1, $showRatings=0){

    $bitext = $alignments->bitext;
    $corpus = $bitext->corpus;
    $version = $bitext->version;
    $fromDoc = $bitext->fromDoc;
    $toDoc = $bitext->toDoc;
    $bitextID = $bitext->id();

    // $showScores=1;
    // $showLengthRatio=1;

    global $langpair;
    global $DeleteSymbol;

    if ($showMyRatings){
        echo '<p><form action="index.php" method="get" id="ratingForm">';
        echo '<input type="hidden" id="rating" name="rating" value="0">';
        echo '<input type="hidden" id="linkID" name="linkID" value="0">';
        echo '</form></p>';
    }

    echo('<p><table class="vbitext">');
    if ($fromDoc){
        echo("<tr><th class='vbitext-search'>$fromDoc</th><th class='vbitext-search'>");
        print_search_form($langpair, $corpus, $version, $fromDoc, $toDoc, $bitextID, '', 1, 0);
        echo("</th></tr>");
    }
    if ($toDoc){
        echo("<tr><th class='vbitext-search'>$toDoc</th><th class='vbitext-search'>");
        print_search_form($langpair, $corpus, $version, $fromDoc, $toDoc, $bitextID, '', 0, 1);
        echo("</th></tr>");
    }
    echo("</table>");
    
    echo('<p><table class="vbitext">');
    
    // echo("<tr><th class='vbitext-docname'>source<br/>target</th>");
    echo("<tr><th class='vbitext'>IDs</th>");
    /*
    echo("<th><div class='leftalign'>&nbsp;&nbsp;");
    echo("$fromDoc&nbsp;&nbsp;");
    if ($fromDoc) print_search_form($langpair, $corpus, $version, $fromDoc, $toDoc, $bitextID, '', 1, 0);
    // echo("<br/>$toDoc&nbsp;");
    echo("</div> <div class='rightalign'>$toDoc&nbsp;&nbsp;");
    if ($toDoc) print_search_form($langpair, $corpus, $version, $fromDoc, $toDoc, $bitextID, '', 0, 1);
    echo("&nbsp;&nbsp;</div></th>");
    */

    /*
    if ($showScores){
        if ($showLengthRatio) echo('<th class="vbitext">score<br/>ratio</th>');
        else echo('<th class="vbitext">score<br/>&nbsp;</th>');
    }
    elseif ($showLengthRatio) echo('<th class="vbitext">&nbsp;<br/>ratio</th>');
    */

    echo("<th class='vbitext'></th>");
    if ($showLengthRatio || $showScores)
        echo('<th class="vbitext">scores</th>');
    echo("</tr>");
    

    $lastBitextID = '';
    $bitextLink = '';
    $i = 0;
    
    while ($row = $alignments->next()) {
        $i++;
        
        $srcText = implode(' ',$row['srcSents']);
        $trgText = implode(' ',$row['trgSents']);

        $textOK = ( strpos($srcText, 'SENTENCE NOT FOUND') === false &&
                    strpos($trgText, 'SENTENCE NOT FOUND') === false );

        if ($showScores){
            $score = $row['alignerScore'];
            if ($score > 0){
                $scoreColor = score_color($score);
            }
        }
        if ($showLengthRatio){
            $srcLen = mb_strlen($srcText);
            $trgLen = mb_strlen($trgText);
            $ratio = 0;
            if ($srcLen or $trgLen){
                $ratio = $srcLen > $trgLen ? $trgLen / $srcLen : $srcLen / $trgLen;
            }
            $ratioColor = score_color($ratio);
            $pretty_ratio = sprintf('%5.3f',$ratio);
        }

        // for search results: add a link to the bitext
        if ($bitextID != $row['bitextID']){
            if ($lastBitextID != $row['bitextID']){
                $bitextLink = bitext_link($bitext,$row['bitextID'], $corpus, $fromDoc, $toDoc);
                $lastBitextID = $row['bitextID'];
            }
        }
        
        // source language row
        
        echo('<tr class="vbitext-src"><td class="vbitext-srcid">');
        // echo($row['linkID'].'/');
        if ($bitextLink) echo($bitextLink.'-');
        echo(implode('&nbsp;',$row['srcIDs']).'</td><td class="vbitext-src">');
        echo($srcText);
        if ($showScores and $score)
            echo("</td><td bgcolor='$scoreColor' class='vbitext-score'>$score");
        elseif ($showLengthRatio or ($showScores and $score==0))
            echo("</td><td bgcolor='$ratioColor' class='vbitext-score'>&nbsp;");
        else
            echo("</td><td class='vbitext-score-empty'>&nbsp;");
        echo("</td></tr>");

        
        // target language row
        
        echo('<tr class="vbitext-trg"><td class="vbitext-trgid">');
        if ($bitextLink) echo($bitextLink.'-');
        echo(implode('&nbsp;',$row['trgIDs']).'</td><td class="vbitext-trg">');
        echo($trgText);
        if ($showLengthRatio)
            echo("</td><td bgcolor='$ratioColor' class='vbitext-ratio'>$pretty_ratio");
        elseif ($showScores)
            echo("</td><td bgcolor='$scoreColor' class='vbitext-ratio'>&nbsp;");
        else
            echo("</td><td class='vbitext-ratio-empty'>&nbsp;");
        echo('</td></tr>');

        
        // rating row
        
        echo('<tr class="vbitext-rating">');
        if ($textOK and $showMyRatings){
            $lid = $row['linkID'];
            $rating = $bitext->getMyAlignmentRating($lid);
            
            echo('<td class="vbitext-rating">');
            $query = make_query(['linkID' => $row['linkID'], 'rating' => -1]);
            if ($rating){
                echo("<a class='black' style=\"text-decoration: none;\" href=\"$PHP_SELF?$query\">$DeleteSymbol</a>&nbsp;");
            }
            // make sure that the table cell height stays the same: print a invisible white arrow
            else{
                echo("<span style=\"color: white;visibility: hidden;\">$DeleteSymbol</span>&nbsp;");
            }
            echo("<span id='rating$lid'>$rating</span>");
            echo('</td>');

            echo('<td class="vbitext-rating">');
            $value = $rating == 0 ? 50 : $rating;
            echo("<input type='range' min='1' max='100' value='$value' class='slider' id='slider$lid'>");
            echo('<script>');
            echo("var slider$lid = document.getElementById('slider$lid');");
            echo("slider$lid.oninput = function() {");
            echo("var output = document.getElementById('rating$lid');");
            echo("output.innerHTML = slider$lid.value;");
            echo("output.innerHTML = this.value;");
            echo('};');
            echo("slider$lid.onchange = function() {");
            echo("var form = document.getElementById('ratingForm');");
            echo("var rating = document.getElementById('rating');");
            echo("var lid = document.getElementById('linkID');");
            echo("rating.value = this.value;");
            echo("lid.value = '$lid';");
            echo("saveScrollPosition();");
            echo("form.submit();");
            echo('}</script>');

            echo('</td>');

            echo('<td class="vbitext-rating">');
            printRatingStars($bitext,$lid);
            /*
            if ($rating)
                printRatingStars($bitext,$lid);
            else
                printStars($bitext->getAverageAlignmentRating($lid));
            */
            echo('</td>');
            
        }
        elseif ($textOK and $showRatings){
            $lid = $row['linkID'];
            $rating = $bitext->getAverageAlignmentRating($lid);

            echo('<td class="vbitext-rating">');
            echo("<span style=\"color: white;visibility: hidden;\">$DeleteSymbol&nbsp;00</span>");
            echo('</td>');

            echo('<td class="vbitext-rating"></td>');
            
            echo('<td class="vbitext-rating">');
            if ($rating){
                printStars($rating);
            }
            echo('</td>');            
        }
        else{
            echo('<td class="vbitext-rating">');
            echo("<span style=\"color: white;visibility: hidden;\">$DeleteSymbol&nbsp;00</span>");
            echo('</td>');
        }
        echo('</tr>');

    }
    echo('</table></p>');
    return $i;
}



function print_editable_bitext_table($alignments,
                                     $showScores=1, $showLengthRatio=0,
                                     $showMyRatings=0, $showRatings=1){

    global $segmentFirstBgColor, $segmentFirstBorderColor;
    global $segmentOtherBgColor, $segmentOtherBorderColor;
    global $ArrowUp, $ArrowDown;

    $bitext = $alignments->bitext;
    $fromDoc = $bitext->fromDoc;
    $toDoc = $bitext->toDoc;
    $bitextID = $bitext->id();

    $PHP_SELF=$_SERVER['PHP_SELF'];

    echo('<table class="hbitext">');
    echo("<tr><th class='hbitext'>$fromDoc</th>");
    if ($showScores) echo('<th class="hbitext">score</th>');
    if ($showLengthRatio) echo('<th class="hbitext">ratio</th>');
    echo("<th class='hbitext'>$toDoc</th>");
    if ($showMyRatings) echo('<th class="hbitext">my rating</th>');
    if ($showRatings) echo('<th class="hbitext">rating</th>');
    echo("</tr>");

    $i = 0;
    $prevLinkID = 0;
                
    while ($row = $alignments->next()) {
        $i++;
        $nextLinkID = $row['linkID']+1;

        $srcText = implode(' ',$row['srcSents']);
        $trgText = implode(' ',$row['trgSents']);

        $segmentBgColor = 'white';
        if (isset($_GET['changelink'])){
            if ($row['linkID'] == $_GET['changelink']){
                $segmentBgColor = $segmentOtherBgColor;
            }
            elseif ($row['linkID'] == $_GET['to']){
                $segmentBgColor = $segmentFirstBgColor;
            }
        }

        echo('<tr bgcolor="'.$segmentBgColor.'">');
        echo('<td class="hbitext-edit-src"><table class="segment">');
        for ($x=0; $x<count($row['srcIDs']);$x++){
            echo("<tr>");
            echo("<td class='segment-move'>");
            if ($row['srcIDs'][$x]){
                echo($row['srcIDs'][$x]);
                $query = make_query(['changelink' => $row['linkID'], 'del-src' => $row['srcIDs'][$x]]);
                echo("</td><td class='segment-move' onMouseOver=\"setStyle(this,'backgroundColor','$segmentOtherBgColor')\" 
                                  onMouseOut=\"setStyle(this,'backgroundColor','white')\"
                                  onClick=\"window.location='$PHP_SELF?$query'\">&#x1f5d1;");
            }
            echo("</td>");
            
            if ($x == 0 && $prevLinkID && $row['srcIDs'][$x]){
                $query = make_query(['changelink' => $row['linkID'], 'move-src' => $row['srcIDs'][$x], 'to' => $prevLinkID]);
                echo("<td class='segment-move' 
                          onMouseOver=\"setStyle(this,'backgroundColor','$segmentFirstBgColor')\" 
                          onMouseOut=\"setStyle(this,'backgroundColor','white')\"
                          onClick=\"window.location='$PHP_SELF?$query'\">$ArrowUp</td>");
            }
            elseif ($x == count($row['srcIDs'])-1 && $row['srcIDs'][$x]){
                $query = make_query(['changelink' => $row['linkID'], 'move-src' => $row['srcIDs'][$x], 'to' => $nextLinkID]);
                echo("<td class='segment-move' 
                          onMouseOver=\"setStyle(this,'backgroundColor','$segmentOtherBgColor')\" 
                          onMouseOut=\"setStyle(this,'backgroundColor','white')\"
                          onClick=\"window.location='$PHP_SELF?$query'\">$ArrowDown</td>");
            }
            else{
                echo("<td class='segment-move'></td>");
            }
            
            echo("<td class='segment-src'>");
            echo($row['srcSents'][$x]);
            echo("</td>");

            if ($x == count($row['srcIDs'])-1 && $row['srcIDs'][$x]){
                $query = make_query(['changelink' => $row['linkID'], 'move-src' => $row['srcIDs'][$x], 'to' => $nextLinkID]);
                echo("<td class='segment-move' 
                          onMouseOver=\"setStyle(this,'backgroundColor','$segmentOtherBgColor')\" 
                          onMouseOut=\"setStyle(this,'backgroundColor','white')\"
                          onClick=\"window.location='$PHP_SELF?$query'\">$ArrowDown</td>");
            }
            elseif ($x == 0 && $prevLinkID && $row['srcIDs'][$x]){
                $query = make_query(['changelink' => $row['linkID'], 'move-src' => $row['srcIDs'][$x], 'to' => $prevLinkID]);
                echo("<td class='segment-move' 
                          onMouseOver=\"setStyle(this,'backgroundColor','$segmentFirstBgColor')\" 
                          onMouseOut=\"setStyle(this,'backgroundColor','white')\"
                          onClick=\"window.location='$PHP_SELF?$query'\">$ArrowUp</td>");
            }
            else{
                echo("<td class='segment-move'></td>");
            }
            echo('</tr>');
        }
        echo('</table></td>');
            
        if ($showScores){
            $score = $row['alignerScore'];
            $color = score_color($score);
            echo("<td bgcolor='$color' class='hbitext-score'>$score</td>");
        }
        if ($showLengthRatio){
            $srcLen = mb_strlen($srcText);
            $trgLen = mb_strlen($trgText);
            $ratio = 0;
            if ($srcLen or $trgLen){
                $ratio = $srcLen > $trgLen ? $trgLen / $srcLen : $srcLen / $trgLen;
            }
            $color = score_color($ratio);
            $pretty_ratio = sprintf('%5.3f',$ratio);
            echo("<td bgcolor='$color' class='hbitext-ratio'>$pretty_ratio</td>");
        }
            

        echo('<td class="hbitext-edit-trg"><table class="segment">');
        for ($x=0; $x<count($row['trgIDs']);$x++){
            echo("<tr>");
            
            if ($x == count($row['trgIDs'])-1 && $row['trgIDs'][$x]){
                $query = make_query(['changelink' => $row['linkID'], 'move-trg' => $row['trgIDs'][$x], 'to' => $nextLinkID]);
                echo("<td class='segment-move' 
                          onMouseOver=\"setStyle(this,'backgroundColor','$segmentOtherBgColor')\" 
                          onMouseOut=\"setStyle(this,'backgroundColor','white')\"
                          onClick=\"window.location='$PHP_SELF?$query'\">$ArrowDown</td>");
            }
            elseif ($x == 0 && $prevLinkID && $row['trgIDs'][$x]){
                $query = make_query(['changelink' => $row['linkID'], 'move-trg' => $row['trgIDs'][$x], 'to' => $prevLinkID]);
                echo("<td class='segment-move' 
                          onMouseOver=\"setStyle(this,'backgroundColor','$segmentFirstBgColor')\" 
                          onMouseOut=\"setStyle(this,'backgroundColor','white')\"
                          onClick=\"window.location='$PHP_SELF?$query'\">$ArrowUp</td>");
            }
            else{
                echo("<td class='segment-move'></td>");
            }

            echo("<td class='segment-trg'>");
            echo($row['trgSents'][$x]);
            echo("</td>");

            if ($x == 0 && $prevLinkID && $row['trgIDs'][$x]){
                $query = make_query(['changelink' => $row['linkID'], 'move-trg' => $row['trgIDs'][$x], 'to' => $prevLinkID]);
                echo("<td class='segment-move' 
                          onMouseOver=\"setStyle(this,'backgroundColor','$segmentFirstBgColor')\" 
                          onMouseOut=\"setStyle(this,'backgroundColor','white')\"
                          onClick=\"window.location='$PHP_SELF?$query'\">$ArrowUp</td>");
            }
            elseif ($x == count($row['trgIDs'])-1 && $row['trgIDs'][$x]){
                $query = make_query(['changelink' => $row['linkID'], 'move-trg' => $row['trgIDs'][$x], 'to' => $nextLinkID]);
                echo("<td class='segment-move' 
                          onMouseOver=\"setStyle(this,'backgroundColor','$segmentOtherBgColor')\" 
                          onMouseOut=\"setStyle(this,'backgroundColor','white')\"
                          onClick=\"window.location='$PHP_SELF?$query'\">$ArrowDown</td>");
            }
            else{
                echo("<td class='segment-move'></td>");
            }
            
            // echo("<td class='segment-id'>");
            if ($row['trgIDs'][$x]){
                $query = make_query(['changelink' => $row['linkID'], 'del-trg' => $row['trgIDs'][$x]]);
                echo("<td class='segment-move' onMouseOver=\"setStyle(this,'backgroundColor','$segmentOtherBgColor')\" 
                            onMouseOut=\"setStyle(this,'backgroundColor','white')\"
                            onClick=\"window.location='$PHP_SELF?$query'\">&#x1f5d1;");
                echo("</td>");
            }
            echo("<td class='segment-move'>");
            echo($row['trgIDs'][$x]);
            echo('</td></tr>');
        }
        echo('</table></td>');

        $textOK = ( strpos($srcText, 'SENTENCE NOT FOUND') === false &&
                    strpos($trgText, 'SENTENCE NOT FOUND') === false );

        if ($showMyRatings && $textOK){
            echo('<td class="hbitext-rating">');
            printRatingStars($bitext,$row['linkID']);
            echo('</td>');
        }
        if ($showRatings && $textOK){
            echo('<td class="hbitext-rating">');
            printAverageRatings($bitext,$row['linkID']);
            echo('</td>');
        }
            
        echo('</tr>'."\n");
        $prevLinkID = $row['linkID'];
    }
    echo('</table>');
    return $i;
}




?>
