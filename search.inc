<?php // -*-php-*-


function clean_input($data) {
  $data = trim($data);
  $data = stripslashes($data);
  $data = strip_tags($data);
  $data = htmlspecialchars($data);
  return $data;
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




function search($searchquery, $searchside, $langpair, $corpus='', $version='', $limit=10, $offset=0){
    // global $showMaxAlignments, $showScores, $showLengthRatio, $showRatings, $showMyRatings, $showEmpty;
    global $showLengthRatio, $showRatings, $showMyRatings;

    list($srclang,$trglang) = explode('-',$langpair);
    
    $srcDBH = new SQLite3('/media/OpusIndex/'.$srclang.'.fts5.db',SQLITE3_OPEN_READONLY);
    $trgDBH = new SQLite3('/media/OpusIndex/'.$trglang.'.fts5.db',SQLITE3_OPEN_READONLY);
    $linksDBH = new SQLite3('/media/OpusIndex/'.$langpair.'.linked.db',SQLITE3_OPEN_READONLY);

    $bitexts = array();
    $corpusID = 0;
    $limitstr = '';
    $offsetstr = '';

    // if ($corpus && $version)
        // echo("search $corpus / $version");
    // echo("search $corpus / $version / $searchside");
    
    if ($searchquery){
        $condition = "sentence MATCH '".$searchquery."'";
        if ($corpus && $version){
            $results = $linksDBH->query("SELECT rowid FROM corpora WHERE corpus='$corpus' AND version='$version'");
            while ($row = $results->fetchArray(SQLITE3_NUM)) {
                $corpusID = $row[0];
                $condition .= ' AND corpusID='.$corpusID;
            }
        }
        if ($limit) $limitstr = " LIMIT ".$limit;
        if ($offset) $offsetstr = " OFFSET ".$offset;
        $start = microtime(true);

        if ($searchside == 'search target'){
            $linksDBH->exec("ATTACH DATABASE '/media/OpusIndex/".$trglang.".fts5.db' AS trgdb");
            $results = $linksDBH->query(
                "SELECT sentID, linkID, highlight(sentences,0, '<b>', '</b>') sentence 
                        FROM trgdb.sentences as trg
                        INNER JOIN linkedtarget ON trg.rowid=linkedtarget.sentID
                        WHERE ".$condition.$limitstr.$offsetstr);

        }
        else{
            $linksDBH->exec("ATTACH DATABASE '/media/OpusIndex/".$srclang.".fts5.db' AS srcdb");
            $results = $linksDBH->query(
                "SELECT sentID, linkID, highlight(sentences,0, '<b>', '</b>') sentence 
                        FROM srcdb.sentences as src
                        INNER JOIN linkedsource ON src.rowid=linkedsource.sentID
                        WHERE ".$condition.$limitstr.$offsetstr);
        }
        $queryTime = microtime(true) - $start;
    }
    
    $fetchSentenceIdTime = 0;
    $fetchSentenceTime = 0;

    if ($results){
        // echo("last = $offset + $limit");
        search_navigation($searchquery, $offset, $limit);
        search_display_options($searchquery);
        $shown=0;
        echo("<table class='bitext'>\n");
        echo("<tr><th>link ID</th><th>source language</th>");
        if ($showScores) echo('<th>score</th>');
        if ($showLengthRatio) echo('<th>ratio</th>');
        echo("<th>target language</th>");
        if ($showMyRatings) echo('<th>my ratings</th>');
        if ($showRatings) echo('<th>ratings</th>');
        echo("</tr>");

        while ($row = $results->fetchArray(SQLITE3_NUM)) {
            $sentID=$row[0];
            $linkID=$row[1];
            $sent=$row[2];

            // echo("get linked sentences\n");
            $start = microtime(true);
            list($srcIDs,$trgIDs) = get_linked_sentences($linksDBH, $linkID);
            $fetchSentenceIdTime += microtime(true) - $start;

            $srcSents = array();
            foreach ($srcIDs as $id){
                $start = microtime(true);
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
            echo($linkID);
            echo("</td>\n<td class='rightalign'>");
            echo($srcText);
            echo("</td>\n");

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
            echo("\n</tr>\n");
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


function get_linked_sentences($linksDBH, $linkID){
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
