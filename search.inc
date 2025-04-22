<?php // -*-php-*-



function print_search_form($langpair, $corpus='', $version='', $fromDoc='', $toDoc='', $bitextID=0,
                           $searchquery='', $allowSourceSearch=1, $allowTargetSearch=1){
    echo(' <form action="'.$_SERVER['PHP_SELF'].'" method="get" style="display: inline;">');
    echo('<input type="hidden" id="langpair" name="langpair" value="'.$langpair.'">');
    echo('<input type="hidden" id="offset" name="offset" value="0">');
    echo('<input type="hidden" id="limit" name="limit" value="10">');
    if ($corpus && $version){
        echo('<input type="hidden" id="corpus" name="corpus" value="'.$corpus.'">');
        echo('<input type="hidden" id="version" name="version" value="'.$version.'">');
    }
    if ($fromDoc && $toDoc){
        echo('<input type="hidden" id="fromDoc" name="fromDoc" value="'.$fromDoc.'">');
        echo('<input type="hidden" id="toDoc" name="fromDoc" value="'.$toDoc.'">');
    }
    if ($bitextID){
        echo('<input type="hidden" id="bitextID" name="bitextID" value="'.$bitextID.'">');
    }
    echo('<input size="12" type="text" name="search" id="search" value="'.$searchquery.'" required />');
    // if ($allowSourceSearch) echo('<input type="submit" name="action" value="search source" />');
    // if ($allowTargetSearch) echo('<input type="submit" name="action" value="search target" />');
    if ($allowSourceSearch) echo('<button type="submit" name="action" value="search source">search</button>');
    if ($allowTargetSearch) echo('<button type="submit" name="action" value="search target">search</button>');

    echo('</form>');
}



function search_navigation($searchquery, $offset, $showMaxAlignments){

    //echo("lastentry = $offset + $showMaxAlignments");
    $lastentry = $offset + $showMaxAlignments;

    if ($offset){
        $query = make_query(['search' => $searchquery, 'offset' => 0]);
        echo(' <a id="prev" href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">start</a>');
        $previous = $offset - $showMaxAlignments;
        if ($previous < 0){ $previous = 0; }
        $query = make_query(['search' => $searchquery,'offset' => $previous]);
        echo(' / <a id="prev" href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">prev</a>');
    }
    else{
        echo(' <span style="color: white;">start</span> / <a id="prev">prev</a>');
        echo '<script>var x = document.getElementById("prev");x.style.visibility = "hidden";</script>';
    }
    // echo(" / [$offset:$lastentry]");
    $query = make_query(['search' => $searchquery, 'offset' => $lastentry]);
    echo(' / <a id="next" href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'"> next</a>');

}

function search_display_options($searchquery){
    global $showScores, $showLengthRatio, $showRatings, $showMyRatings, $tableStyle;

    if ($showScores){
        $query = make_query(['showScores' => 0]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">scores</a>');
    }
    else{
        $query = make_query(['showScores' => 1]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'"><strike>scores</strike></a>');
    }
    if ($showLengthRatio){
        $query = make_query(['search' => $searchquery, 'showLengthRatio' => 0]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">length ratio</a>');
    }
    else{
        $query = make_query(['search' => $searchquery, 'showLengthRatio' => 1]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'"><strike>length ratio</strike></a>');
    }
    if ($showMyRatings){
        $query = make_query(['search' => $searchquery, 'showMyRatings' => 0]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">my rating</a>');
    }
    else{
        $query = make_query(['search' => $searchquery, 'showMyRatings' => 1]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'"><strike>my rating</strike></a>');
    }
    if ($showRatings){
        $query = make_query(['search' => $searchquery, 'showRatings' => 0]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">avg rating</a>');
    }
    else{
        $query = make_query(['search' => $searchquery, 'showRatings' => 1]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'"><strike>avg rating</strike></a>');
    }
}





function search($searchquery, $searchside, $bitext, $limit=10, $offset=0, $orderByLinkID=0){
    global $showScores, $showLengthRatio, $showRatings, $showMyRatings;
    global $allowSortLinks;    

    $langpair = $bitext->langpair;
    $corpus = $bitext->corpus;
    $version = $bitext->version;
    $fromDoc = $bitext->fromDoc;
    $toDoc = $bitext->toDoc;
    $bitextID = $bitext->id();
    
    $alignments = $bitext->search($searchquery, $searchside, $limit, $offset, $orderByLinkID);

    search_navigation($searchquery, $offset, $limit);
    echo('<div class="rightalign">');
    search_display_options($searchquery);
    $shown=0;
    echo('</div></br><hr>');
    echo("<table class='hbitext'>\n<tr>\n");

    // show either bitext ID (if we search)
    // or link ID (if we browse, can be sorted)
    if ($searchquery){
        echo("<th class='hbitext'>bitext&nbsp;ID</th>");
    }
    else{
        echo("<th class='hbitext'>link&nbsp;ID");
        if ($allowSortLinks){
            $query = make_query(['sortLinkIDs' => (! $orderByLinkID)]);
            $sortlabel = $orderByLinkID ? '&#x29BB;' : '⇓';
            echo(' <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">'.$sortlabel.'</a>');
        }
        echo("</th>");
    }

    $srcquery = $searchside == 'search source' ? $searchquery : '';
    $trgquery = $searchside == 'search target' ? $searchquery : '';

    echo("<th class='hbitext'>source language");
    print_search_form($langpair, $corpus, $version, $fromDoc, $toDoc, $bitextID, $srcquery, 1, 0);
    echo("</th>");
    if ($showScores) echo('<th class="hbitext">score</th>');
    if ($showLengthRatio) echo('<th class="hbitext">ratio</th>');
    echo("<th class='hbitext'>target language");
    print_search_form($langpair, $corpus, $version, $fromDoc, $toDoc, $bitextID, $trgquery, 0, 1);
    echo("</th>");
    if ($showMyRatings) echo('<th class="hbitext">my rating</th>');
    if ($showRatings) echo('<th class="hbitext">rating</th>');
    echo("</tr>");

    
    while ($data = $alignments->next()){
        $linkID=$data['linkID'];
        $bitextID=$data['bitextID'];
        $srcText = implode(' ',$data['srcSents']);
        $trgText = implode(' ',$data['trgSents']);
            
        echo("<tr>\n");
        echo("<td class='hbitext-srcid'>");
        if ($searchquery){
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
            echo('<a title="'.$hreftitle.'" href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">');
            echo($bitextID);
            echo('</a>');
        }
        else echo($linkID);
        echo("</td>\n<td class='hbitext-src'>");
        echo($srcText);
        echo("</td>\n");

        if ($showScores){
            $score = array_key_exists('alignerScore',$data) ? $data['alignerScore'] : 0;
            $color = score_color($score);
            echo("</td><td bgcolor='$color' class='hbitext-score'>$score");
        }
        if ($showLengthRatio){
            $srcLen = strlen(str_replace('<b>','',str_replace('</b>','',$srcText)));
            $trgLen = strlen(str_replace('<b>','',str_replace('</b>','',$trgText)));
            $ratio = 0;
            if ($srcLen or $trgLen){
                $ratio = $srcLen > $trgLen ? $trgLen / $srcLen : $srcLen / $trgLen;
            }
            $color = score_color($ratio);
            $pretty_ratio = sprintf('%5.3f',$ratio);
            echo("</td><td bgcolor='$color' class='hbitext-ratio'>$pretty_ratio");
        }
        
        echo("<td class='hbitext-trg'>");
        echo($trgText);
            
        $textOK = ( strpos($srcText, 'SENTENCE NOT FOUND') === false &&
                    strpos($trgText, 'SENTENCE NOT FOUND') === false );

        if ($showMyRatings && $textOK){
            echo('</td><td class="hbitext-rating">');
            printRatingStars($bitext,$linkID);
        }
        if ($showRatings && $textOK){
            echo('</td><td class="hbitext-rating">');
            printAverageRatings($bitext,$linkID);
        }

        echo("</td>\n</tr>\n");
        $shown++;
    }
    echo('</table>');
    if ($shown < $limit){
        echo '<script>var x = document.getElementById("next");x.style.visibility = "hidden";</script>';
    }
}

    


?>
