<?php // -*-php-*-


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
                $this->opusLangpair = $this->opusLangpairs[0];
                $this->index->setLinkDb($indexDir,$srclang,$trglang,$corpus,$version,
                                        $fromDoc,$toDoc,$opusLangpair);
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
        
    public function alignments($type, $limit=10, $offset=0){
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

    public function addAlignmentRating($bitextID,$linkID,$rating){
        if (!$bitextID) $bitextID=$this->id();
        $user = $this->user;
        if ($user){
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
        $this->searchResults = $this->index->getAlignments($this->bitext->id(), $type, $limit, $offset);
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


function bitext_browsing_links($bitext, $selectedAlignType=''){
    global $tableStyle, $showModified, $allowEdit, $showEmpty;
    global $ALIGN_TYPES, $allowOtherAlignTypes;

    if ($tableStyle == 'edit' && $allowEdit){
        $query = make_query(['style' => 'horizontal']);
        echo('<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">stop editing</a> / ');
    }
    else{
        if ($bitext->userDataExists()){
            if ($showModified){
                $query = make_query(['showModified' => 0]);
                echo('modified / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">original</a> / ');
            }
            else{
                $query = make_query(['showModified' => 1]);
                echo('<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">modified</a> / original / ');
            }
        }
        if ($allowEdit){
            $query = make_query(['style' => 'edit']);
            echo('<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">edit mode</a> / ');
        }
        
        foreach ($ALIGN_TYPES as $type){
            if ($selectedAlignType != $type){
                $query = make_query(['aligntype' => $type, 'offset' => 0]);
                echo('<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">'.$type.'</a> / ');
            }
            else echo($type.' / ');
        }
        if ($allowOtherAlignTypes){
            if ($selectedAlignType != 'other'){
                $query = make_query(['aligntype' => 'other', 'offset' => 0]);
                echo('<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">other</a> / ');
            }
            else echo('other / ');
        }
        if ($showEmpty){
            $query = make_query(['showEmpty' => 0]);
            echo('<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">hide empty</a>');
        }
        else{
            $query = make_query(['showEmpty' => 1]);
            echo('<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">show empty</a>');
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
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'"> horizontal</a>');
    }
    else{
        $query = make_query(['style' => 'vertical']);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'"> vertical</a>');
    }

    bitext_score_options();
    bitext_rating_options();
}

function bitext_score_options(){
    global $showScores, $showLengthRatio;
    if ($showScores){
        $query = make_query(['showScores' => 0]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'"> hide scores</a>');
    }
    else{
        $query = make_query(['showScores' => 1]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'"> show scores</a>');
    }
    if ($showLengthRatio){
        $query = make_query(['showLengthRatio' => 0]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'"> hide length ratio</a>');
    }
    else{
        $query = make_query(['showLengthRatio' => 1]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'"> show length ratio</a>');
    }
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
    global $srcDBH, $srcIdxDBH, $trgDBH, $trgIdxDBH, $linksDBH;
    global $showMaxAlignments, $showScores, $showLengthRatio, $showRatings, $showMyRatings, $showEmpty;
    global $tableStyle, $allowEdit;

    if ($tableStyle == 'edit' && $allowEdit){
        $showEmpty = 1;
        $type = '';
        if (isset($_GET['changelink'])){
            $bitext->changeLink($_GET['changelink'],$_GET);
        }
    }

    $alignments = $bitext->alignments($type,$showMaxAlignments,$offset);

    if ($alignments){

        bitext_navigation($offset, $showMaxAlignments);
        bitext_display_options();
        bitext_rating($bitext);

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


function print_horizontal_bitext_table($alignments,
                                       $showScores=1, $showLengthRatio=0,
                                       $showMyRatings=1, $showRatings=0){

    // TODO: should pass on bitext instance to functions instead of those variables
    $bitext = $alignments->bitext;
    $corpus = $bitext->corpus;
    $version = $bitext->version;
    $fromDoc = $bitext->fromDoc;
    $toDoc = $bitext->toDoc;
    $bitextID = $bitext->id();

    global $langpair, $showModified;
    
    echo('<table class="bitext">');
    echo("<tr><th>IDs</th>");
    echo("<th style='text-align: right;'>".$fromDoc);
    print_search_form($langpair, $corpus, $version, $fromDoc, $toDoc, $bitextID, '', 1, 0);
    echo("</th>");
    if ($showScores) echo('<th>score</th>');
    if ($showLengthRatio) echo('<th>ratio</th>');
    echo("<th style='text-align: right;'>".$toDoc);
    print_search_form($langpair, $corpus, $version, $fromDoc, $toDoc, $bitextID, '', 0, 1);
    echo("</th><th>IDs</th>");
    if ($showMyRatings) echo('<th>my ratings</th>');
    if ($showRatings) echo('<th>ratings</th>');
    echo("</tr>");
    $i = 0;
    while ($row = $alignments->next()) {
        $i++;

        $srcText = implode(' ',$row['srcSents']);
        $trgText = implode(' ',$row['trgSents']);

        echo('<tr><td class="centeralign">');
        echo(implode('&nbsp;',$row['srcIDs']).'</td><td class="rightalign">');
        echo($srcText);
            
        if ($showScores){
            $score = $row['alignerScore'];
            $color = score_color($score);
            echo("</td><td bgcolor='$color' class='centeralign'>$score");
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
            echo("</td><td bgcolor='$color' class='centeralign'>$pretty_ratio");
        }
            
        echo('</td><td class="leftalign">');
        echo($trgText);
        echo('</td><td class="centeralign">'.implode('&nbsp;',$row['trgIDs']));

        $textOK = ( strpos($srcText, 'SENTENCE NOT FOUND') === false &&
                    strpos($trgText, 'SENTENCE NOT FOUND') === false );

        if ($showMyRatings && $textOK){
            echo('</td><td class="centeralign">');
            // print_rating_stars($bitextID,$row['linkID']);
            printRatingStars($bitext,$row['linkID']);
        }
        if ($showRatings && $textOK){
            echo('</td><td class="centeralign">');
            // print_average_ratings($bitextID,$row['linkID']);
            printAverageRatings($bitext,$row['linkID']);
        }
            
        echo('</td></tr>'."\n");
    }
    echo('</table>');
    return $i;
}


function print_vertical_bitext_table($alignments,
                                     $showScores=1, $showLengthRatio=0,
                                     $showMyRatings=1, $showRatings=0){

    $bitext = $alignments->bitext;
    $fromDoc = $bitext->fromDoc;
    $toDoc = $bitext->toDoc;
    $bitextID = $bitext->id();

    
    echo('<table class="bitext">');
    
    echo("<tr><th>source<br/>target</th><th>$fromDoc<br/>$toDoc</th>");
    /*
    echo("<tr><th>source<br/>target</th><th>$fromDoc");
    print_search_form($langpair, $corpus, $version, $fromDoc, $toDoc, $bitextID, '', 1, 0);
    echo("<br/>$toDoc");
    print_search_form($langpair, $corpus, $version, $fromDoc, $toDoc, $bitextID, '', 0, 1);
    echo('</th>');
    */
    if ($showScores){
        if ($showLengthRatio) echo('<th>score<br/>ratio</th>');
        else echo('<th>score<br/>&nbsp;</th>');
    }
    elseif ($showLengthRatio) echo('<th>&nbsp;<br/>ratio</th>');
    
    if ($showMyRatings){
        if ($showRatings) echo('<th>my&nbsp;ratings<br/>avg&nbsp;ratings</th>');
        else echo('<th>my&nbsp;ratings<br/>&nbsp;</th>');
    }
    elseif ($showRatings) echo('<th>&nbsp;<br/>avg&nbsp;ratings</th>');
    echo("</tr>");
    
    $i = 0;
    
    while ($row = $alignments->next()) {
        $i++;
        
        $srcText = implode(' ',$row['srcSents']);
        $trgText = implode(' ',$row['trgSents']);

        $textOK = ( strpos($srcText, 'SENTENCE NOT FOUND') === false &&
                    strpos($trgText, 'SENTENCE NOT FOUND') === false );

        if ($showScores){
            $score = $row['alignerScore'];
            $scoreColor = score_color($score);
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
        
        // source language row
        
        echo('<tr class="bitextsrc"><td class="centeralign">');
        echo(implode('&nbsp;',$row['srcIDs']).'</td><td>');
        echo($srcText);

        if ($showScores)
            echo("</td><td bgcolor='$scoreColor' class='centeralign'>$score");
        elseif ($showLengthRatio)
            echo("</td><td bgcolor='$ratioColor'>&nbsp;");

        if ($textOK){
            if ($showMyRatings){
                echo('</td><td class="centeralign">');
                // print_rating_stars($bitextID,$row['linkID']);
                printRatingStars($bitext,$row['linkID']);
            }
            elseif ($showRatings){
                echo("</td><td>");
            }
        }
        echo("</td></tr>");

        
        // target language row
        
        echo('<tr class="bitexttrg"><td class="centeralign">');
        echo(implode('&nbsp;',$row['trgIDs']).'</td><td class="bitexttrg">');
        echo($trgText);

        if ($showLengthRatio)
            echo("</td><td bgcolor='$ratioColor' class='centeralign'>$pretty_ratio");
        elseif ($showScores)
            echo("</td><td bgcolor='$scoreColor' class='centeralign'>&nbsp;");


        if ($textOK){
            if ($showRatings){
                echo('</td><td class="centeralign">');
                // print_average_ratings($bitextID,$row['linkID']);
                printAverageRatings($bitext,$row['linkID']);
            }
            elseif ($showMyRatings){
                echo("</td><td>");
            }
        }
        echo("</td></tr>");

    }
    echo('</table>');
    return $i;
}



function print_editable_bitext_table($alignments,
                                     $showScores=1, $showLengthRatio=0,
                                     $showMyRatings=1, $showRatings=0){

    global $segmentFirstBgColor, $segmentFirstBorderColor;
    global $segmentOtherBgColor, $segmentOtherBorderColor;

    $bitext = $alignments->bitext;
    $fromDoc = $bitext->fromDoc;
    $toDoc = $bitext->toDoc;
    $bitextID = $bitext->id();

    $PHP_SELF=$_SERVER['PHP_SELF'];

    echo('<table class="bitext">');
    echo("<tr><th>$fromDoc</th>");
    if ($showScores) echo('<th>score</th>');
    if ($showLengthRatio) echo('<th>ratio</th>');
    echo("<th>$toDoc</th>");
    if ($showMyRatings) echo('<th>my ratings</th>');
    if ($showRatings) echo('<th>ratings</th>');
    echo("</tr>");

    $srcAnchors = array();
    $trgAnchors = array();
        
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
        echo('<td class="bitext-src"><table class="segment">');
        for ($x=0; $x<count($row['srcIDs']);$x++){
            array_push($srcAnchors,$row['srcIDs'][$x]);
            echo '<a name="src-'.$row['srcIDs'][$x].'" />';
            $anchor = (count($srcAnchors) > 4) ? '#src-'.$srcAnchors[count($srcAnchors)-4] : '';
            echo("<tr>");
            echo("<td class='segment-move'>");
            if ($row['srcIDs'][$x]){
                echo($row['srcIDs'][$x]);
                $query = make_query(['changelink' => $row['linkID'], 'del-src' => $row['srcIDs'][$x]]);
                echo("</td><td class='segment-move' onMouseOver=\"setStyle(this,'backgroundColor','$segmentOtherBgColor')\" 
                                  onMouseOut=\"setStyle(this,'backgroundColor','white')\"
                                  onClick=\"window.location='$PHP_SELF?$query$anchor'\">&#x1f5d1;");
            }
            echo("</td>");
            
            if ($x == 0 && $prevLinkID && $row['srcIDs'][$x]){
                $query = make_query(['changelink' => $row['linkID'], 'move-src' => $row['srcIDs'][$x], 'to' => $prevLinkID]);
                echo("<td class='segment-move' 
                          onMouseOver=\"setStyle(this,'backgroundColor','$segmentFirstBgColor')\" 
                          onMouseOut=\"setStyle(this,'backgroundColor','white')\"
                          onClick=\"window.location='$PHP_SELF?$query$anchor'\">⇑</td>");
            }
            elseif ($x == count($row['srcIDs'])-1 && $row['srcIDs'][$x]){
                $query = make_query(['changelink' => $row['linkID'], 'move-src' => $row['srcIDs'][$x], 'to' => $nextLinkID]);
                echo("<td class='segment-move' 
                          onMouseOver=\"setStyle(this,'backgroundColor','$segmentOtherBgColor')\" 
                          onMouseOut=\"setStyle(this,'backgroundColor','white')\"
                          onClick=\"window.location='$PHP_SELF?$query$anchor'\">⇓</td>");
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
                          onClick=\"window.location='$PHP_SELF?$query$anchor'\">⇓</td>");
            }
            elseif ($x == 0 && $prevLinkID && $row['srcIDs'][$x]){
                $query = make_query(['changelink' => $row['linkID'], 'move-src' => $row['srcIDs'][$x], 'to' => $prevLinkID]);
                echo("<td class='segment-move' 
                          onMouseOver=\"setStyle(this,'backgroundColor','$segmentFirstBgColor')\" 
                          onMouseOut=\"setStyle(this,'backgroundColor','white')\"
                          onClick=\"window.location='$PHP_SELF?$query$anchor'\">⇑</td>");
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
            echo("<td bgcolor='$color' class='centeralign'>$score</td>");
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
            echo("<td bgcolor='$color' class='centeralign'>$pretty_ratio</td>");
        }
            

        echo('<td class="bitext-trg"><table class="segment">');
        for ($x=0; $x<count($row['trgIDs']);$x++){
            array_push($trgAnchors,$row['trgIDs'][$x]);
            echo '<a name="trg-'.$row['trgIDs'][$x].'" />';
            $anchor = (count($trgAnchors) > 4) ? '#trg-'.$trgAnchors[count($trgAnchors)-4] : '';
            echo("<tr>");
            
            if ($x == count($row['trgIDs'])-1 && $row['trgIDs'][$x]){
                $query = make_query(['changelink' => $row['linkID'], 'move-trg' => $row['trgIDs'][$x], 'to' => $nextLinkID]);
                echo("<td class='segment-move' 
                          onMouseOver=\"setStyle(this,'backgroundColor','$segmentOtherBgColor')\" 
                          onMouseOut=\"setStyle(this,'backgroundColor','white')\"
                          onClick=\"window.location='$PHP_SELF?$query$anchor'\">⇓</td>");
            }
            elseif ($x == 0 && $prevLinkID && $row['trgIDs'][$x]){
                $query = make_query(['changelink' => $row['linkID'], 'move-trg' => $row['trgIDs'][$x], 'to' => $prevLinkID]);
                echo("<td class='segment-move' 
                          onMouseOver=\"setStyle(this,'backgroundColor','$segmentFirstBgColor')\" 
                          onMouseOut=\"setStyle(this,'backgroundColor','white')\"
                          onClick=\"window.location='$PHP_SELF?$query$anchor'\">⇑</td>");
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
                          onClick=\"window.location='$PHP_SELF?$query$anchor'\">⇑</td>");
            }
            elseif ($x == count($row['trgIDs'])-1 && $row['trgIDs'][$x]){
                $query = make_query(['changelink' => $row['linkID'], 'move-trg' => $row['trgIDs'][$x], 'to' => $nextLinkID]);
                echo("<td class='segment-move' 
                          onMouseOver=\"setStyle(this,'backgroundColor','$segmentOtherBgColor')\" 
                          onMouseOut=\"setStyle(this,'backgroundColor','white')\"
                          onClick=\"window.location='$PHP_SELF?$query$anchor'\">⇓</td>");
            }
            else{
                echo("<td class='segment-move'></td>");
            }
            
            // echo("<td class='segment-id'>");
            if ($row['trgIDs'][$x]){
                $query = make_query(['changelink' => $row['linkID'], 'del-trg' => $row['trgIDs'][$x]]);
                echo("<td class='segment-move' onMouseOver=\"setStyle(this,'backgroundColor','$segmentOtherBgColor')\" 
                            onMouseOut=\"setStyle(this,'backgroundColor','white')\"
                            onClick=\"window.location='$PHP_SELF?$query$anchor'\">&#x1f5d1;");
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
            echo('<td class="centeralign">');
            // print_rating_stars($bitextID,$row['linkID']);
            printRatingStars($bitext,$row['linkID']);
            echo('</td>');
        }
        if ($showRatings && $textOK){
            echo('<td class="centeralign">');
            // print_average_ratings($bitextID,$row['linkID']);
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
