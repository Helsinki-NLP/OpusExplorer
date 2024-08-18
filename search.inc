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
    echo('<input type="text" name="search" id="search" value="'.$searchquery.'" required />');
    if ($allowSourceSearch) echo('<input type="submit" name="action" value="search source" />');
    if ($allowTargetSearch) echo('<input type="submit" name="action" value="search target" />');
    echo('</form>');
}



function search_navigation($searchquery, $offset, $showMaxAlignments){

    //echo("lastentry = $offset + $showMaxAlignments");
    $lastentry = $offset + $showMaxAlignments;

    if ($offset){
        $query = make_query(['search' => $searchquery, 'offset' => 0]);
        echo('<a id="prev" href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">start</a>');
        $previous = $offset - $showMaxAlignments;
        if ($previous < 0){ $previous = 0; }
        $query = make_query(['search' => $searchquery,'offset' => $previous]);
        echo(' / <a id="prev" href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">prev</a>');
    }
    else{
        echo('start / <a id="prev">prev</a>');
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
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'"> hide scores</a>');
    }
    else{
        $query = make_query(['showScores' => 1]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'"> show scores</a>');
    }
    if ($showLengthRatio){
        $query = make_query(['search' => $searchquery, 'showLengthRatio' => 0]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'"> hide length ratio</a>');
    }
    else{
        $query = make_query(['search' => $searchquery, 'showLengthRatio' => 1]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'"> show length ratio</a>');
    }
    if ($showRatings){
        $query = make_query(['search' => $searchquery, 'showRatings' => 0]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'"> hide user ratings</a>');
    }
    else{
        $query = make_query(['search' => $searchquery, 'showRatings' => 1]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'"> show user ratings</a>');
    }
    if ($showMyRatings){
        $query = make_query(['search' => $searchquery, 'showMyRatings' => 0]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'"> hide my ratings</a>');
    }
    else{
        $query = make_query(['search' => $searchquery, 'showMyRatings' => 1]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'"> show my ratings</a>');
    }
}




function search($searchquery, $searchside, $langpair, $corpus='', $version='', $bitextID='',
                $limit=10, $offset=0, $orderByLinkID=0){
    // global $showMaxAlignments, $showScores, $showLengthRatio, $showRatings, $showMyRatings, $showEmpty;
    global $showScores, $showLengthRatio, $showRatings, $showMyRatings;
    global $linkDbFile, $srcFtsFile, $trgFtsFile;
    global $fromDoc, $toDoc;

    list($srclang,$trglang) = explode('-',$langpair);

    $srcDBH = new SQLite3($srcFtsFile,SQLITE3_OPEN_READONLY);
    $trgDBH = new SQLite3($trgFtsFile,SQLITE3_OPEN_READONLY);
    $linksDBH = new SQLite3($linkDbFile,SQLITE3_OPEN_READONLY);

    $bitexts = array();
    $corpusID = 0;
    
    $srcquery = $searchside == 'search source' ? $searchquery : '';
    $trgquery = $searchside == 'search target' ? $searchquery : '';

    $conditions = array();
    if ($searchquery) array_push($conditions, "sentence MATCH '".$searchquery."'");
    if ($bitextID)    array_push($conditions, 'bitextID='.$bitextID);
    $condition = implode(' AND ',$conditions);

    $ordering = $orderByLinkID ? ' ORDER BY linkID ' : '';    
    $limitstr = $limit ?  " LIMIT ".$limit : '';
    $offsetstr = $offset ? " OFFSET ".$offset : '';

    $start = microtime(true);
    if ($searchquery){
        if ($searchside == 'search target'){
            $linksDBH->exec("ATTACH DATABASE '/media/OpusIndex/".$trglang.".fts5.db' AS trgdb");
            $results = $linksDBH->query(
                "SELECT linkID, bitextID, sentID, highlight(sentences,0, '<b>', '</b>') sentence 
                        FROM trgdb.sentences as trg
                        INNER JOIN linkedtarget ON trg.rowid=linkedtarget.sentID
                        WHERE ".$condition.$ordering.$limitstr.$offsetstr);
        }
        else{
            $linksDBH->exec("ATTACH DATABASE '/media/OpusIndex/".$srclang.".fts5.db' AS srcdb");
            $results = $linksDBH->query(
                "SELECT linkID, bitextID, sentID, highlight(sentences,0, '<b>', '</b>') sentence 
                        FROM srcdb.sentences as src
                        INNER JOIN linkedsource ON src.rowid=linkedsource.sentID
                        WHERE ".$condition.$ordering.$limitstr.$offsetstr);
        }
    }
    else{
        $results = $linksDBH->query("SELECT DISTINCT linkID, bitextID FROM linkedsource WHERE ".$condition.$ordering.$limitstr.$offsetstr);
    }
    $queryTime = microtime(true) - $start;
    
    $fetchSentenceIdTime = 0;
    $fetchSentenceTime = 0;

    if ($results){
        // echo("last = $offset + $limit");
        search_navigation($searchquery, $offset, $limit);
        search_display_options($searchquery);
        $shown=0;
        echo("<table class='bitext'>\n<tr>\n");

        // show either bitext ID (if we search)
        // or link ID (if we browse, can be sorted)
        if ($searchquery){
            echo("<th>bitext ID</th>");
        }
        else{
            echo("<th>link ID");
            $query = make_query(['sortLinkIDs' => (! $orderByLinkID)]);
            $sortlabel = $orderByLinkID ? '&#x29BB;' : '⇓';
            echo(' <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">'.$sortlabel.'</a></th>');
        }

        echo("<th style='text-align: right;'>source language");
        print_search_form($langpair, $corpus, $version, $fromDoc, $toDoc, $bitextID, $srcquery, 1, 0);
        echo("</th>");
        if ($showScores) echo('<th>score</th>');
        if ($showLengthRatio) echo('<th>ratio</th>');
        echo("<th style='text-align: right;'>target language");
        print_search_form($langpair, $corpus, $version, $fromDoc, $toDoc, $bitextID, $trgquery, 0, 1);
        echo("</th>");
        if ($showMyRatings) echo('<th>my ratings</th>');
        if ($showRatings) echo('<th>ratings</th>');
        echo("</tr>");

        while ($row = $results->fetchArray(SQLITE3_NUM)) {
            $linkID=$row[0];
            $bitextID=$row[1];
            if (count($row) > 2) $sentID=$row[2];
            if (count($row) > 3) $sent=$row[3];

            // echo("get linked sentences\n");
            $start = microtime(true);
            list($srcIDs,$trgIDs) = get_linked_sentence_ids($linksDBH, $linkID);
            $fetchSentenceIdTime += microtime(true) - $start;

            $srcSents = array();
            foreach ($srcIDs as $id){
                $start = microtime(true);
                if ($searchside == 'search source' && $id == $sentID)
                    array_push($srcSents, $sent);
                else
                    array_push($srcSents, fetch_sentence($srcDBH, $id));
                $fetchSentenceTime += microtime(true) - $start;
            }
            $trgSents = array();
            foreach ($trgIDs as $id){
                $start = microtime(true);
                if ($searchside == 'search target' && $id == $sentID)
                    array_push($trgSents, $sent);
                else
                    array_push($trgSents, fetch_sentence($trgDBH, $id));
                $fetchSentenceTime += microtime(true) - $start;
            }

            $srcText = implode(' ',$srcSents);
            $trgText = implode(' ',$trgSents);
            
            echo("<tr>\n");
            echo("<td class='centeralign'>");
            if ($searchquery){
                if (! ($corpus && $version && $fromDoc && $toDoc) ){
                    $bitext = get_bitext($bitextID);
                    $query = make_query(['corpus' => $bitext['corpus'],
                                         'version' => $bitext['version'],
                                         'fromDoc' => $bitext['fromDoc'],
                                         'toDoc' => $bitext['toDoc'],
                                         'bitextID' => $bitextID,
                                         'search' => '', 'offset' => 0]);
                    $hreftitle = implode('/',[$bitext['corpus'],$bitext['version'],$bitext['fromDoc']]);
                }
                else{
                    $query = make_query(['bitextID' => $bitextID, 'search' => '', 'offset' => 0]);
                    $hreftitle = implode('/',[$corpus,$version,$fromDoc]);
                }
                echo('<a title="'.$hreftitle.'" href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">');
                echo($bitextID);
                echo('</a>');
            }
            else echo($linkID);
            echo("</td>\n<td class='rightalign'>");
            echo($srcText);
            echo("</td>\n");

            if ($showScores){
                $link = get_link($linkID);
                $score = array_key_exists('alignerScore',$link) ? $link['alignerScore'] : 0;
                $color = score_color($score);
                echo("</td><td bgcolor='$color' class='centeralign'>$score");
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
                echo("</td><td bgcolor='$color' class='centeralign'>$pretty_ratio");
            }

            echo("<td class='leftalign'>");
            echo($trgText);
            
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

            echo("</td>\n</tr>\n");
            $shown++;
        }
        echo('</table>');
        if ($shown < $limit){
            echo '<script>var x = document.getElementById("next");x.style.visibility = "hidden";</script>';
        }

        $totalTime = $queryTime + $fetchSentenceIdTime + $fetchSentenceTime;
        echo('<ul>');
        echo("<li>".$queryTime." (query time)</li>\n");
        echo("<li>".$fetchSentenceIdTime." (time for fetching sentence IDs)</li>\n");
        echo("<li>".$fetchSentenceTime." (time for fetching sentences)</li>\n");
        echo("<li>".$totalTime." (total time for fetching data)</li>\n");
        echo('</ul>');
    }
}


function get_linked_sentence_ids($linksDBH, $linkID){
    // echo("SELECT sentID FROM linkedsource WHERE linkID=$linkID\n");
    $results = $linksDBH->query("SELECT sentID FROM linkedsource WHERE linkID=$linkID");
    $srcIDs = array();
    if ($results){
        while ($row = $results->fetchArray(SQLITE3_NUM)) {
            array_push($srcIDs,$row[0]);
        }
    }
    // echo("SELECT sentID FROM linkedtarget WHERE linkID=$linkID");
    $results = $linksDBH->query("SELECT sentID FROM linkedtarget WHERE linkID=$linkID");
    $trgIDs = array();
    if ($results){
        while ($row = $results->fetchArray(SQLITE3_NUM)) {
            array_push($trgIDs,$row[0]);
        }
    }
    return array($srcIDs,$trgIDs);
}


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




?>
