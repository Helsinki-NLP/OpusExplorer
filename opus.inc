<?php // -*-php-*-


function print_bitext_menu($corpus,$version,$srclang,$trglang,$langpair,$fromDoc,$toDoc,$searchquery,$alignType){
    global $bitext, $bitextID, $opusLangpair;

    $query = make_query(['srclang' => '', 'trglang' => '', 'langpair' => '', 'opusLangpair' => '',
                         'corpus' => '', 'version' => '', 'fromDoc' => '', 'toDoc' => '',
                         'aligntype' => '', 'offset' => 0, 'sortLinkIDs' => 0,
                         'search' => '', 'fromDocQuery' => '', 'toDocQuery' => '']);
    echo('<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">OPUS</a> / ');

    if ($srclang && $trglang){

        $opusLangpairs = $bitext->opusLangpairs;
        $query = make_query(['corpus' => '', 'version' => '', 'fromDoc' => '', 'toDoc' => '',
                             'opusLangpair' => $opusLangpair,
                             'aligntype' => '', 'offset' => 0, 'sortLinkIDs' => 0,
                             'search' => '', 'fromDocQuery' => '', 'toDocQuery' => '']);
        echo('<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">'.$langpair.'</a> / ');

        if (sizeof($opusLangpairs) > 1){
            foreach ($opusLangpairs as $l){
                if ($l == $opusLangpair)
                    echo($l.' / ');
                else{
                    $query = make_query(['corpus' => '', 'version' => '', 'fromDoc' => '', 'toDoc' => '',
                                         'opusLangpair' => $l,
                                         'aligntype' => '', 'offset' => 0, 'sortLinkIDs' => 0,
                                         'search' => '', 'fromDocQuery' => '', 'toDocQuery' => '']);
                    echo('<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">'.$l.'</a> / ');
                }
            }
        }

        if ($corpus){
            $query = make_query(['version' => '', 'fromDoc' => '', 'toDoc' => '', 'offset' => 0,
                                 'search' => '', 'fromDocQuery' => '', 'toDocQuery' => '']);
            echo('<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">'.$corpus.'</a> / ');

            if ($version){
                $query = make_query(['fromDoc' => '', 'toDoc' => '', 'offset' => 0,
                                     'search' => '', 'fromDocQuery' => '', 'toDocQuery' => '']);
                echo('<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">'.$version.'</a> / ');
        
                if ($fromDoc && $toDoc){
                    $query = make_query(['aligntype' => '', 'offset' => 0, 'sortLinkIDs' => 0,
                                         'search' => '', 'fromDocQuery' => '', 'toDocQuery' => '']);
                    echo('<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">'.$fromDoc.'</a> / ');
                    if (! $searchquery) bitext_browsing_links($bitext, $alignType);
                }
            }
        }
        else{
            $corpusID = 0;
            set_param('corpusID',0);
        }            
        if (! $fromDoc || ! $toDoc){
            $bitextID = 0;
            set_param('bitextID',$bitextID);
            print_search_form($langpair, $corpus, $version, $fromDoc, $toDoc, $bitextID, $searchquery);
        }
    }
    elseif ($corpus && $version){
        $query = make_query(['version' => '', 'fromDoc' => '', 'toDoc' => '', 'offset' => 0,
                             'search' => '', 'fromDocQuery' => '', 'toDocQuery' => '']);
        echo('<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">'.$corpus.'</a> / ');
        $query = make_query(['fromDoc' => '', 'toDoc' => '', 'offset' => 0,
                             'search' => '', 'fromDocQuery' => '', 'toDocQuery' => '']);
        echo('<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">'.$version.'</a> / ');
    }
    else{
        $latest = 1;
        $corpora = $bitext->corpora($latest);
        echo '<form action="index.php" method="get" style="padding: 0px;margin: 0px;display: inline;">';
        echo '<input type="hidden" id="showLatest" name="showLatest" value="1">';
        echo '<select name="corpus" id="corpus" onchange="this.form.submit()">';
        echo('<option value=""> -- select corpus -- </option>');

        while ($row = $corpora->next()) {
            if (substr( $row['corpus'], 0, 4 ) != "ELRA")
                if (substr( $row['corpus'], 0, 4 ) != "ELRC")
                    if ($corpus && $corpus == $row['corpus'])
                        echo('<option selected value="'.$row['corpus'].'">'.$row['corpus'].'</option>');
                    else
                        echo('<option value="'.$row['corpus'].'">'.$row['corpus'].'</option>');
        }
        echo '</select>';
        echo '</form>';
    }
}


function print_document_list($bitext, $offset=0, $fromDocQuery='', $toDocQuery=''){
    global $showMaxDocuments, $showRatings, $showMyRatings;

    // $fromDocQuery = 'de/1988/%';
    $documents = $bitext->documents($offset, $showMaxDocuments, $fromDocQuery, $toDocQuery);
    
    echo 'select a source document: ';
    bitext_navigation($offset, $showMaxDocuments, false);
    //bitext_rating_options();
    //bitext_average_rating_option();
    echo('<br/><br/><table class="doclist">');
    echo '<tr><td class="doclist"></td><td class="docliststars">';
    bitext_user_rating_option();
    echo '</td>';
    echo '<td class="docliststars">';
    bitext_average_rating_option();
    echo '</td></tr>';
    
    $count = 0;
    $differentNames = 0;
    while ($row = $documents->next()) {
        $count++;
        $bitextID = $row['bitextID'];
        $fromDocID = $bitext->getSourceDocumentID($row['fromDoc']);
        $toDocID = $bitext->getTargetDocumentID($row['toDoc']);
        $query = make_query(['fromDoc' => $row['fromDoc'],
                             'toDoc' => $row['toDoc'],
                             'bitextID' => $row['bitextID'],
                             'fromDocID' => $fromDocID,
                             'toDocID' => $toDocID,
                             'offset' => 0]);

        $link = '<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">';
        echo '<tr><td class="doclist">'.$link.$row['fromDoc'].'</a></td>';
        if ($count == 1){
            $srcparts = explode('/',$row['fromDoc']);
            $trgparts = explode('/',$row['toDoc']);
            array_shift($srcparts);
            array_shift($trgparts);
            if (implode('/',$srcparts) != implode('/',$trgparts)){
                $differentNames = 1;
            }
        }
        if ($differentNames){
            echo '<td class="doclist">'.$link.$row['toDoc'].'</a></td>';
        }
        // echo '<tr><td class="doclist"><a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">'.$docpair.'</a></td>';
        echo('<td class="docliststars">');
        if ($showMyRatings){
            $rating = $bitext->getMyBitextRating($bitextID);
            if ($rating){
                printStars($rating);
            }
        }
        echo('</td>');

        echo('<td class="docliststars">');
        if ($showRatings){
            $rating = $bitext->getAverageBitextRating($bitextID);
            if ($rating){
                printStars($rating);
            }
        }
        echo("</td></tr>\n");
    }
    if ($count >= $showMaxDocuments || $fromDocQuery || $toDocQuery){
        // $queryValue = $fromDocQuery ? $fromDocQuery : $bitext->srclang.'/%';
        $queryValue = $fromDocQuery ? $fromDocQuery : $bitext->opusSrclang.'/%';
        echo('<tr><td class="doclist"><form action="'.$_SERVER['PHP_SELF'].'" method="get" style="display: inline;">');
        echo('<input type="hidden" id="langpair" name="langpair" value="'.$bitext->langpair.'">');
        echo('<input type="hidden" id="offset" name="offset" value="0">');
        echo('<input type="hidden" id="limit" name="limit" value="10">');
        echo('<input type="hidden" id="corpus" name="corpus" value="'.$bitext->corpus.'">');
        echo('<input type="hidden" id="version" name="version" value="'.$bitext->version.'">');
        echo('<input type="hidden" id="toDocQuery" name="toDocQuery" value="">');
        echo('<input type="text" name="fromDocQuery" id="fromDocQuery" size="15" value="'.$queryValue.'" required />');
        echo('<input type="submit" name="docsearch" value="search" />');
        echo('</form></td>');
        // echo('</td><td class="docliststars"><input type="submit" name="docsearch" value="search document" />');
        // echo('</form></td>');
        if ($differentNames){
            $queryValue = $toDocQuery ? $toDocQuery : $bitext->opusTrglang.'/%';
            echo('<td class="doclist"><form action="'.$_SERVER['PHP_SELF'].'" method="get" style="display: inline;">');
            echo('<input type="hidden" id="langpair" name="langpair" value="'.$bitext->langpair.'">');
            echo('<input type="hidden" id="offset" name="offset" value="0">');
            echo('<input type="hidden" id="limit" name="limit" value="10">');
            echo('<input type="hidden" id="corpus" name="corpus" value="'.$bitext->corpus.'">');
            echo('<input type="hidden" id="version" name="version" value="'.$bitext->version.'">');
            echo('<input type="hidden" id="fromDocQuery" name="fromDocQuery" value="">');
            echo('<input type="text" name="toDocQuery" id="toDocQuery" size="15" value="'.$queryValue.'" required />');
            echo('<input type="submit" name="docsearch" value="search" />');
            echo('</form></td>');
        }
        echo('</tr>');
    }
    else{
        echo '<script>var x = document.getElementById("next");x.style.visibility = "hidden";</script>';
    }
    echo('</table>');
}


function print_corpus_list($bitext){

    echo 'corpus list / ';
    $query = make_query(['srclang' => '', 'trglang' => '',
                         'langpair' => '', 'opusLangpair' => '',
                         'resourceView' => 'langList']);
    echo '<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">language list</a> / ';
    $query = make_query(['srclang' => '', 'trglang' => '',
                         'langpair' => '', 'opusLangpair' => '',
                         'resourceView' => 'langMatrix']);
    echo '<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">language matrix</a><br/>';
    
    $corpora = $bitext->releases();
    $corpus = '';
    $versions = array();
    $link = '';
    $versionLinks = '';
    echo('<ul>');
    while ($row = $corpora->next()) {
        if ($row['corpus'] != $corpus){
            // if ($corpus) echo "<li>$link$corpus</a>: $versionLinks</li>\n";
            // if ($corpus) echo "<li>$corpus: [${link}latest</a>] $versionLinks</li>\n";
            if ($corpus) echo "<li>$corpus: $versionLinks</li>\n";
            $corpus = $row['corpus'];
            $versionLinks = '';
        }
        array_push($versions,$row['version']);
        $version = $row['version'];
        $query = make_query(['corpus' => $corpus, 'version' => $version, 'search' => '', 'resourceView' => 'langMatrix']);
        $link = '<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">';
        $versionLinks .= ' ['.$link.$version.'</a>]';
    }
    // if ($corpus) echo "<li>$link$corpus</a>: $versionLinks</li>\n";
    // if ($corpus) echo "<li>$corpus: [${link}latest</a>] $versionLinks</li>\n";
    if ($corpus) echo "<li>$corpus: $versionLinks</li>\n";
    echo('</ul>');
}


function print_langpairs($bitext, $resourceView='langList'){

    $query = make_query(['resourceView' => 'corpusList', 'version' => '']);
    echo '<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">corpus list</a> / ';
    
    $langpairs = find_bitext_dbs($bitext);    
    if ( ($resourceView == 'langMatrix') && count($langpairs)>4 ){
        return print_langpair_matrix($langpairs);
    }
    return print_langpair_list($langpairs);
}

function print_langpair_list($langpairs){
    
    // $langpairs = array('fi-uk');
    ksort($langpairs);

    if (count($langpairs)>4){
        echo('language list / ');
        $query = make_query(['resourceView' => 'langMatrix']);
        echo '<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">language matrix</a><br/>';
    }

    echo('<ul>');
    foreach ($langpairs as $langpair => $val){
        list($srclang,$trglang) = explode('-',$langpair);
        $srclangname = Locale::getDisplayName($srclang, 'en');
        $trglangname = Locale::getDisplayName($trglang, 'en');
        $query = make_query(['langpair' => $langpair, 'search' => '']);
        echo '<li><a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">';
        echo $langpair.' ('.$srclangname.' - '.$trglangname;
        echo ')</a></li>'."\n";
    }
    echo('</ul>');
    /*
    if (!class_exists('Locale')) {
        echo ('No php_intl extension installed.');
    }
    */
}


function print_langpair_matrix($langpairs){

    $langmatrix = array();
    $srclangs = array();
    $trglangs = array();

    foreach ($langpairs as $langpair => $val){
        list($srclang,$trglang) = explode('-',$langpair);
        $srclangs[$srclang]++;
        $trglangs[$trglang]++;
    }
    
    ksort($srclangs);
    ksort($trglangs);


    if (count($langpairs)>4){
        $query = make_query(['resourceView' => 'langList']);
        echo '<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">language list</a>';
        echo(' / language matrix<br/><br/>');
    }
    
    echo('<table><tr><th></th>');
    foreach ($trglangs as $trglang => $count){
        $trglangname = Locale::getDisplayName($trglang, 'en');
        $trglangs[$trglang] = $trglangname;
        // echo('<th>'.$trglangname.'</th>');
        echo('<th>'.$trglang.'</th>');
    }
    echo('</tr>');
    foreach ($srclangs as $srclang => $srccount){
        $srclangname = Locale::getDisplayName($srclang, 'en');
        // echo('<tr><th>'.$srclangname.'</th>');
        echo('<tr><th>'.$srclang.'</th>');
        foreach ($trglangs as $trglang => $trglangname){
            $langpair = "$srclang-$trglang";
            if (array_key_exists($langpair,$langpairs)){
                $query = make_query(['langpair' => $langpair, 'search' => '']);
                echo '<td><a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">';
                // echo $langpair;
                echo 'index</a></td>'."\n";
            }
            else{
                echo('<td></td>');
            }
        }
        echo('</tr>');
    }
    echo('</table>');
}


function find_bitext_dbs($bitext){
    global $DB_DIR;

    $indexLangpairs = $bitext->langpairs();
    $langpairs = array();
    foreach ($indexLangpairs as $lp){
        if (file_exists($DB_DIR.'/linkdb/'.$lp.'.db')){
            list($srclang,$trglang) = explode('-',$lp);
            if (file_exists($DB_DIR.'/'.$srclang.'.fts5.db'))
                if (file_exists($DB_DIR.'/'.$trglang.'.fts5.db'))
                    $langpairs[$lp] = true;
        }
    }
    return $langpairs;
    
    
    $langpairs = array();
    if ($handle = opendir($DB_DIR.'/linkdb')) {
        while (false !== ($entry = readdir($handle))) {
            $parts = explode('.',$entry);
            if (array_pop($parts) == 'db'){
                if (count($parts) == 1){
                    $langs = explode('-',$parts[0]);
                    if (count($langs) == 2){
                        if (file_exists($DB_DIR.'/'.$langs[0].'.fts5.db'))
                            if (file_exists($DB_DIR.'/'.$langs[1].'.fts5.db'))
                                $langpairs[$parts[0]] = true;
                    }
                }
            }
        }
        closedir($handle);
    }
    /*
    if ($handle = opendir($DB_DIR)) {
        while (false !== ($entry = readdir($handle))) {
            $parts = explode('.',$entry);
            if (array_pop($parts) == 'db'){
                if (count($parts) == 1){
                    if (count(explode('-',$parts[0])) == 2){
                        $langpairs[$parts[0]] = true;
                    }
                }
            }
        }
        closedir($handle);
    }
    if ($handle = opendir($DB_DIR.'/sqlite')) {
        while (false !== ($entry = readdir($handle))) {
            if (substr($entry,-10) == '.linked.db') {
                $lang = basename($entry,'.linked.db');
                $parts = explode('-',$lang);
                if (count($parts) == 2){
                    $langpairs[$lang] = true;
                }
            }
        }
    }
    */
    return $langpairs;
}


?>
