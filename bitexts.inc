<?php // -*-php-*-



function bitext_browsing_links(){
    global $tableStyle, $modifiedBitextExists, $showModified, $allowEdit, $showEmpty;
    global $ALIGN_TYPES;

    if ($tableStyle == 'edit' && $allowEdit){
        $query = make_query(['style' => 'horizontal']);
        echo('<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">stop editing</a> / ');
    }
    else{
        if ($modifiedBitextExists){
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
            $query = make_query(['aligntype' => $type, 'offset' => 0]);
            echo('<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">'.$type.'</a> / ');
        }
        $query = make_query(['aligntype' => 'other', 'offset' => 0]);
        echo('<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">other</a> / ');
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

function bitext_rating($bitextID){
    global $showMyRatings, $showRatings;
    if ($showMyRatings){
        echo(" / rate the overall bitext quality: ");
        print_rating_stars($bitextID,0);
    }
    if ($showRatings){
        echo(" (");
        print_average_ratings($bitextID,0);
        echo(")");
    }
}


function print_bitext($corpus, $version, $fromDoc, $toDoc, $fromDocID=0, $toDocID=0, $bitextID=0, $type='', $offset=0){
    global $srcDBH, $srcIdxDBH, $trgDBH, $trgIdxDBH, $algDBH;
    global $showMaxAlignments, $showScores, $showLengthRatio, $showRatings, $showMyRatings, $showEmpty;
    global $tableStyle, $allowEdit;

    if (! $bitextID){
        $bitextID = get_bitextid($corpus, $version, $fromDoc, $toDoc);
        set_param('bitextID',$bitextID);
        set_link_db($bitextID);
    }

    if ($tableStyle == 'edit' && $allowEdit){
        $showEmpty = 1;
        $type = '';
        if (isset($_GET['changelink'])){
            $userAlgDBH = initialize_user_link_db($bitextID);
            // echo("change link ".$_GET['changelink']);
            change_link($userAlgDBH, $bitextID, $_GET['changelink']);
        }
    }

    $results = get_alignments($algDBH, $corpus, $version, $fromDoc, $toDoc,
                              $fromDocID, $toDocID, $bitextID, $type, $offset);

    if ($results){

        bitext_navigation($offset, $showMaxAlignments);
        bitext_display_options();
        bitext_rating($bitextID);

        if ($tableStyle == 'edit' && $allowEdit){
            $shown = print_editable_bitext_table($results,
                                                 $corpus, $version, $fromDoc, $toDoc,
                                                 $fromDocID, $toDocID, $bitextID,
                                                 $showScores, $showLengthRatio,
                                                 $showMyRatings, $showRatings);
        }
        elseif ($tableStyle == 'vertical'){
            $shown = print_vertical_bitext_table($results,
                                                 $corpus, $version, $fromDoc, $toDoc,
                                                 $fromDocID, $toDocID, $bitextID,
                                                 $showScores, $showLengthRatio,
                                                 $showMyRatings, $showRatings);
        }
        else{
            $shown = print_horizontal_bitext_table($results,
                                                   $corpus, $version, $fromDoc, $toDoc,
                                                   $fromDocID, $toDocID, $bitextID,
                                                   $showScores, $showLengthRatio,
                                                   $showMyRatings, $showRatings);
        }
        
        // hide next button if we are at the end of the document
        if ($shown < $showMaxAlignments){
            echo '<script>var x = document.getElementById("next");x.style.visibility = "hidden";</script>';
        }
    }
}


function print_horizontal_bitext_table($results,
                                       $corpus, $version, $fromDoc, $toDoc,
                                       $fromDocID, $toDocID, $bitextID,
                                       $showScores=1, $showLengthRatio=0,
                                       $showMyRatings=1, $showRatings=0){

    global $langpair, $searchable, $showModified;
    
    echo('<table class="bitext">');
    echo("<tr><th>IDs</th>");
    if ($searchable && ! $showModified){
        echo("<th style='text-align: right;'>".$fromDoc);
        print_search_form($langpair, $corpus, $version, $fromDoc, $toDoc, $bitextID, '', 1, 0);
    }
    else echo("<th>".$fromDoc);
    echo("</th>");
    if ($showScores) echo('<th>score</th>');
    if ($showLengthRatio) echo('<th>ratio</th>');
    if ($searchable && ! $showModified){
        echo("<th style='text-align: right;'>".$toDoc);
        print_search_form($langpair, $corpus, $version, $fromDoc, $toDoc, $bitextID, '', 0, 1);
    }
    else echo("<th>".$toDoc);
    echo("</th><th>IDs</th>");
    if ($showMyRatings) echo('<th>my ratings</th>');
    if ($showRatings) echo('<th>ratings</th>');
    echo("</tr>");
    $i = 0;
    while ($row = $results->fetchArray(SQLITE3_NUM)) {
        $i++;
        $srcIDs = explode(' ',$row[0]);
        $trgIDs = explode(' ',$row[1]);
        $linkID = $row[3];

        list($srcSents, $trgSents) = get_linked_sentences($linkID, $srcIDs, $trgIDs,
                                                          $corpus, $version, $fromDoc, $toDoc,
                                                          $fromDocID, $toDocID);        
        $srcText = implode(' ',$srcSents);
        $trgText = implode(' ',$trgSents);

        echo('<tr><td class="centeralign">');
        echo(implode('&nbsp;',$srcIDs).'</td><td class="rightalign">');
        echo($srcText);
            
        if ($showScores){
            $score = $row[2];
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
        echo('</td><td class="centeralign">'.implode('&nbsp;',$trgIDs));

        $textOK = ( strpos($srcText, 'SENTENCE NOT FOUND') === false &&
                    strpos($trgText, 'SENTENCE NOT FOUND') === false );

        if ($showMyRatings && $textOK){
            echo('</td><td class="centeralign">');
            print_rating_stars($bitextID,$linkID);
        }
        if ($showRatings && $textOK){
            echo('</td><td class="centeralign">');
            print_average_ratings($bitextID,$linkID);
        }
            
        echo('</td></tr>'."\n");
    }
    echo('</table>');
    return $i;
}


function print_vertical_bitext_table($results,
                                     $corpus, $version, $fromDoc, $toDoc,
                                     $fromDocID, $toDocID, $bitextID,
                                     $showScores=1, $showLengthRatio=0,
                                     $showMyRatings=1, $showRatings=0){
    
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
    while ($row = $results->fetchArray(SQLITE3_NUM)) {
        $i++;
        $srcIDs = explode(' ',$row[0]);
        $trgIDs = explode(' ',$row[1]);
        $linkID = $row[3];

        list($srcSents, $trgSents) = get_linked_sentences($linkID, $srcIDs, $trgIDs,
                                                          $corpus, $version, $fromDoc, $toDoc,
                                                          $fromDocID, $toDocID);        
        
        $srcText = implode(' ',$srcSents);
        $trgText = implode(' ',$trgSents);

        $textOK = ( strpos($srcText, 'SENTENCE NOT FOUND') === false &&
                    strpos($trgText, 'SENTENCE NOT FOUND') === false );

        if ($showScores){
            $score = $row[2];
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
        echo(implode('&nbsp;',$srcIDs).'</td><td>');
        echo($srcText);

        if ($showScores)
            echo("</td><td bgcolor='$scoreColor' class='centeralign'>$score");
        elseif ($showLengthRatio)
            echo("</td><td bgcolor='$ratioColor'>&nbsp;");

        if ($textOK){
            if ($showMyRatings){
                echo('</td><td class="centeralign">');
                print_rating_stars($bitextID,$linkID);
            }
            elseif ($showRatings){
                echo("</td><td>");
            }
        }
        echo("</td></tr>");

        
        // target language row
        
        echo('<tr class="bitexttrg"><td class="centeralign">');
        echo(implode('&nbsp;',$trgIDs).'</td><td class="bitexttrg">');
        echo($trgText);

        if ($showLengthRatio)
            echo("</td><td bgcolor='$ratioColor' class='centeralign'>$pretty_ratio");
        elseif ($showScores)
            echo("</td><td bgcolor='$scoreColor' class='centeralign'>&nbsp;");


        if ($textOK){
            if ($showRatings){
                echo('</td><td class="centeralign">');
                print_average_ratings($bitextID,$linkID);
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



function print_editable_bitext_table($results,
                                     $corpus, $version, $fromDoc, $toDoc,
                                     $fromDocID, $toDocID, $bitextID,
                                     $showScores=1, $showLengthRatio=0,
                                     $showMyRatings=1, $showRatings=0){

    global $segmentFirstBgColor, $segmentFirstBorderColor;
    global $segmentOtherBgColor, $segmentOtherBorderColor;

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
                
    while ($row = $results->fetchArray(SQLITE3_NUM)) {
        $i++;
        $srcIDs = explode(' ',trim($row[0]));
        $trgIDs = explode(' ',trim($row[1]));
        $linkID = $row[3];
        $nextLinkID = $linkID+1;

        list($srcSents, $trgSents) = get_linked_sentences($linkID, $srcIDs, $trgIDs,
                                                          $corpus, $version, $fromDoc, $toDoc,
                                                          $fromDocID, $toDocID);        
        
        $srcText = implode(' ',$srcSents);
        $trgText = implode(' ',$trgSents);

        $segmentBgColor = 'white';
        if (isset($_GET['changelink'])){
            if ($linkID == $_GET['changelink']){
                $segmentBgColor = $segmentOtherBgColor;
            }
            elseif ($linkID == $_GET['to']){
                $segmentBgColor = $segmentFirstBgColor;
            }
        }

        echo('<tr bgcolor="'.$segmentBgColor.'">');
        echo('<td class="bitext-src"><table class="segment">');
        for ($x=0; $x<count($srcIDs);$x++){
            array_push($srcAnchors,$srcIDs[$x]);
            echo '<a name="src-'.$srcIDs[$x].'" />';
            $anchor = (count($srcAnchors) > 4) ? '#src-'.$srcAnchors[count($srcAnchors)-4] : '';
            echo("<tr>");
            echo("<td class='segment-move'>");
            if ($srcIDs[$x]){
                echo($srcIDs[$x]);
                $query = make_query(['changelink' => $linkID, 'del-src' => $srcIDs[$x]]);
                echo("</td><td class='segment-move' onMouseOver=\"setStyle(this,'backgroundColor','$segmentOtherBgColor')\" 
                                  onMouseOut=\"setStyle(this,'backgroundColor','white')\"
                                  onClick=\"window.location='$PHP_SELF?$query$anchor'\">&#x1f5d1;");
#                echo("&nbsp;<span onMouseOver=\"setStyle(this,'backgroundColor','$segmentOtherBgColor')\" 
#                                  onMouseOut=\"setStyle(this,'backgroundColor','white')\"
#                                  onClick=\"window.location='$PHP_SELF?$query$anchor'\">&#x1f5d1;</span>");
            }
            echo("</td>");
            
            if ($x == 0 && $prevLinkID && $srcIDs[$x]){
                $query = make_query(['changelink' => $linkID, 'move-src' => $srcIDs[$x], 'to' => $prevLinkID]);
                echo("<td class='segment-move' 
                          onMouseOver=\"setStyle(this,'backgroundColor','$segmentFirstBgColor')\" 
                          onMouseOut=\"setStyle(this,'backgroundColor','white')\"
                          onClick=\"window.location='$PHP_SELF?$query$anchor'\">⇑</td>");
            }
            elseif ($x == count($srcIDs)-1 && $srcIDs[$x]){
                $query = make_query(['changelink' => $linkID, 'move-src' => $srcIDs[$x], 'to' => $nextLinkID]);
                echo("<td class='segment-move' 
                          onMouseOver=\"setStyle(this,'backgroundColor','$segmentOtherBgColor')\" 
                          onMouseOut=\"setStyle(this,'backgroundColor','white')\"
                          onClick=\"window.location='$PHP_SELF?$query$anchor'\">⇓</td>");
            }
            else{
                echo("<td class='segment-move'></td>");
            }
            
            echo("<td class='segment-src'>");
            echo($srcSents[$x]);
            echo("</td>");

            if ($x == count($srcIDs)-1 && $srcIDs[$x]){
                $query = make_query(['changelink' => $linkID, 'move-src' => $srcIDs[$x], 'to' => $nextLinkID]);
                echo("<td class='segment-move' 
                          onMouseOver=\"setStyle(this,'backgroundColor','$segmentOtherBgColor')\" 
                          onMouseOut=\"setStyle(this,'backgroundColor','white')\"
                          onClick=\"window.location='$PHP_SELF?$query$anchor'\">⇓</td>");
            }
            elseif ($x == 0 && $prevLinkID && $srcIDs[$x]){
                $query = make_query(['changelink' => $linkID, 'move-src' => $srcIDs[$x], 'to' => $prevLinkID]);
                echo("<td class='segment-move' 
                          onMouseOver=\"setStyle(this,'backgroundColor','$segmentFirstBgColor')\" 
                          onMouseOut=\"setStyle(this,'backgroundColor','white')\"
                          onClick=\"window.location='$PHP_SELF?$query$anchor'\">⇑</td>");
            }
            else{
                echo("<td class='segment-move'></td>");
            }
            echo('</tr>');
            // wastebasket: &#x1f5d1;
            // circle with cross for delete: &#x29BB;
            // white arrow up: ⇑ (thicker than the one down - why?)
            // white arrow down: ⇩
        }
        echo('</table></td>');
            
        if ($showScores){
            $score = $row[2];
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
        for ($x=0; $x<count($trgIDs);$x++){
            array_push($trgAnchors,$trgIDs[$x]);
            echo '<a name="trg-'.$trgIDs[$x].'" />';
            $anchor = (count($trgAnchors) > 4) ? '#trg-'.$trgAnchors[count($trgAnchors)-4] : '';
            echo("<tr>");
            
            if ($x == count($trgIDs)-1 && $trgIDs[$x]){
                $query = make_query(['changelink' => $linkID, 'move-trg' => $trgIDs[$x], 'to' => $nextLinkID]);
                echo("<td class='segment-move' 
                          onMouseOver=\"setStyle(this,'backgroundColor','$segmentOtherBgColor')\" 
                          onMouseOut=\"setStyle(this,'backgroundColor','white')\"
                          onClick=\"window.location='$PHP_SELF?$query$anchor'\">⇓</td>");
            }
            elseif ($x == 0 && $prevLinkID && $trgIDs[$x]){
                $query = make_query(['changelink' => $linkID, 'move-trg' => $trgIDs[$x], 'to' => $prevLinkID]);
                echo("<td class='segment-move' 
                          onMouseOver=\"setStyle(this,'backgroundColor','$segmentFirstBgColor')\" 
                          onMouseOut=\"setStyle(this,'backgroundColor','white')\"
                          onClick=\"window.location='$PHP_SELF?$query$anchor'\">⇑</td>");
            }
            else{
                echo("<td class='segment-move'></td>");
            }

            echo("<td class='segment-trg'>");
            echo($trgSents[$x]);
            echo("</td>");

            if ($x == 0 && $prevLinkID && $trgIDs[$x]){
                $query = make_query(['changelink' => $linkID, 'move-trg' => $trgIDs[$x], 'to' => $prevLinkID]);
                echo("<td class='segment-move' 
                          onMouseOver=\"setStyle(this,'backgroundColor','$segmentFirstBgColor')\" 
                          onMouseOut=\"setStyle(this,'backgroundColor','white')\"
                          onClick=\"window.location='$PHP_SELF?$query$anchor'\">⇑</td>");
            }
            elseif ($x == count($trgIDs)-1 && $trgIDs[$x]){
                $query = make_query(['changelink' => $linkID, 'move-trg' => $trgIDs[$x], 'to' => $nextLinkID]);
                echo("<td class='segment-move' 
                          onMouseOver=\"setStyle(this,'backgroundColor','$segmentOtherBgColor')\" 
                          onMouseOut=\"setStyle(this,'backgroundColor','white')\"
                          onClick=\"window.location='$PHP_SELF?$query$anchor'\">⇓</td>");
            }
            else{
                echo("<td class='segment-move'></td>");
            }
            
            // echo("<td class='segment-id'>");
            if ($trgIDs[$x]){
                $query = make_query(['changelink' => $linkID, 'del-trg' => $trgIDs[$x]]);
                echo("<td class='segment-move' onMouseOver=\"setStyle(this,'backgroundColor','$segmentOtherBgColor')\" 
                            onMouseOut=\"setStyle(this,'backgroundColor','white')\"
                            onClick=\"window.location='$PHP_SELF?$query$anchor'\">&#x1f5d1;");
#                echo("<span onMouseOver=\"setStyle(this,'backgroundColor','$segmentOtherBgColor')\" 
#                            onMouseOut=\"setStyle(this,'backgroundColor','white')\"
#                            onClick=\"window.location='$PHP_SELF?$query$anchor'\">&#x1f5d1;</span>&nbsp;");
                echo("</td>");
            }
            echo("<td class='segment-move'>");
            echo($trgIDs[$x]);
            echo('</td></tr>');
        }
        echo('</table></td>');

        $textOK = ( strpos($srcText, 'SENTENCE NOT FOUND') === false &&
                    strpos($trgText, 'SENTENCE NOT FOUND') === false );

        if ($showMyRatings && $textOK){
            echo('<td class="centeralign">');
            print_rating_stars($bitextID,$linkID);
            echo('</td>');
        }
        if ($showRatings && $textOK){
            echo('<td class="centeralign">');
            print_average_ratings($bitextID,$linkID);
            echo('</td>');
        }
            
        echo('</tr>'."\n");
        $prevLinkID = $linkID;
    }
    echo('</table>');
    return $i;
}





// TEST FOR PRINTING LINKS

function print_links($corpus, $version, $fromDoc, $toDoc){
    global $algDBH;
    echo('<pre>');
    $results = $algDBH->query("SELECT DISTINCT srcIDs,trgIDs FROM alignments WHERE corpus='$corpus' AND version='$version' AND fromDoc='$fromDoc' AND toDoc='$toDoc'");
    if ($results){
        while ($row = $results->fetchArray(SQLITE3_NUM)) {
            echo($row[0]." - ".$row[1]."\n");
        }
    }
    echo('</pre>');
}




?>
