<?php // -*-php-*-


class bitext {

    protected $index;
    protected $user;
    protected $bitextID;
    protected $fromDocID;
    protected $toDocID;
    
    public $corpus;
    public $version;
    public $langpair;
    public $fromDoc;
    public $toDoc;

    function __construct($indexDir, $user, $corpus, $version, $langpair, $fromDoc, $toDoc, $showUserData=1) {
        $this->corpus = $corpus;
        $this->version = $version;
        $this->langpair = $langpair;
        $this->fromDoc = $fromDoc;
        $this->toDoc = $toDoc;
        $this->user = $user;

        list($srclang,$trglang) = explode('-',$langpair);
        $this->index = new opusindex($indexDir, $user, $srclang, $trglang, $corpus, $version, $fromDoc ,$toDoc);
        $bitextID=$this->id();
        if ($showUserData) $this->$userDataExists = $this->index->openUserDB($bitextID);
    }
            
    public function id(){
        if ($this->bitextID) return $this->bitextID;
        if ($id = get_param('bitextID')){
            $this->bitextID = $id;
        }
        else{
            $this->bitextID = $this->index->getBitextID($this->corpus,
                                                        $this->version,
                                                        $this->fromDoc,
                                                        $this->toDoc);
            set_param('bitextID',$id);
        }
        return $this->bitextID;
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

    public function alignments($type, $limit, $offset){
        $alignments = new alignments($this, $this->index);
        $alignments->fetch($type, $limit, $offset);
        return $alignments;
    }

    public function userDataExists(){
        return $this->index->userDataExists($this->id());
    }

    public function getMyAlignmentRating($linkID){
        return $this->index->getAlignmentRating($this->id(),$linkID,$this->user);
    }

    public function getAverageAlignmentRating($linkID){
        return $this->index->getAlignmentRating($this->id(),$linkID);
    }

}


class alignments{
    
    public $bitext;
    protected $index;
    protected $searchResult;

    function __construct($bitext,$index){
        $this->bitext = $bitext;
        $this->index = $index;        
    }

    public function fetch($type, $limit, $offset){
        global $linksDBH;
        $this->searchResults = $this->index->getAlignments($this->bitext->id(), $type, $offset);
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
                         $link['trgSentIDs']) = get_linked_sentences_ids($link['linkID'],
                                                                         $link['srcIDs'], $link['trgIDs'],
                                                                         $bitext->corpus, $bitext->version,
                                                                         $bitext->fromDoc, $bitext->toDoc,
                                                                         $bitext->fromDocID(), $bitext->toDocID());
                }

                $link['srcIDs'] = $link['srcIDs'] ? explode(' ',$link['srcIDs']) : array();
                $link['trgIDs'] = $link['trgIDs'] ? explode(' ',$link['trgIDs']) : array();
                $link['srcSents'] = array();
                $link['trgSents'] = array();

                global $srcDBH,$trgDBH;
                foreach ($link['srcSentIDs'] as $id)
                    if ($id) array_push($link['srcSents'], fetch_sentence($srcDBH, $id));
                foreach ($link['trgSentIDs'] as $id)
                    if ($id) array_push($link['trgSents'], fetch_sentence($trgDBH, $id));
                
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
            $userLinkDbFile = initialize_user_link_db($bitextID);
            change_link($userLinkDbFile, $bitextID, $_GET['changelink']);
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

    global $langpair, $searchable, $showModified;
    
    echo('<table class="bitext">');
    echo("<tr><th>IDs</th>");
    if ($searchable){
        echo("<th style='text-align: right;'>".$fromDoc);
        print_search_form($langpair, $corpus, $version, $fromDoc, $toDoc, $bitextID, '', 1, 0);
    }
    else echo("<th>".$fromDoc);
    echo("</th>");
    if ($showScores) echo('<th>score</th>');
    if ($showLengthRatio) echo('<th>ratio</th>');
    if ($searchable){
        echo("<th style='text-align: right;'>".$toDoc);
        print_search_form($langpair, $corpus, $version, $fromDoc, $toDoc, $bitextID, '', 0, 1);
    }
    else echo("<th>".$toDoc);
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
            $srcLen = strlen($srcText);
            $trgLen = strlen($trgText);
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
            $srcLen = strlen($srcText);
            $trgLen = strlen($trgText);
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
            $srcLen = strlen($srcText);
            $trgLen = strlen($trgText);
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





// TEST FOR PRINTING LINKS

function print_links($corpus, $version, $fromDoc, $toDoc){
    global $linksDBH;
    echo('<pre>');
    $results = $linksDBH->query("SELECT DISTINCT srcIDs,trgIDs FROM alignments WHERE corpus='$corpus' AND version='$version' AND fromDoc='$fromDoc' AND toDoc='$toDoc'");
    if ($results){
        while ($row = $results->fetchArray(SQLITE3_NUM)) {
            echo($row[0]." - ".$row[1]."\n");
        }
    }
    echo('</pre>');
}




?>
