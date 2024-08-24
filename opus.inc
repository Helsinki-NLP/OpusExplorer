<?php // -*-php-*-


function print_document_list($bitext, $offset=0, $fromDocQuery='', $toDocQuery=''){
    global $showMaxDocuments, $showRatings, $showMyRatings;

    // $fromDocQuery = 'de/1988/%';
    $documents = $bitext->documents($offset, $showMaxDocuments, $fromDocQuery);
    
    echo 'select a source document: ';
    bitext_navigation($offset, $showMaxDocuments, false);
    bitext_rating_options();
    echo('<br/><br/><table class="doclist">');
        
    $count = 0;
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
    
        echo '<tr><td class="doclist"><a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">'.$row['fromDoc'].'</a></td>';
        if ($showMyRatings){
            $rating = $bitext->getMyBitextRating($bitextID);
            echo('<td class="docliststars">');
            printStars($rating);
            echo('</td>');
        }
        if ($showRatings){
            $rating = $bitext->getAverageBitextRating($bitextID);
            echo('<td class="docliststars">');
            printStars($rating);
            echo('</td>');
        }
        echo("</tr>\n");
    }
    if ($count >= $showMaxDocuments || $fromDocQuery){
        $queryValue = $fromDocQuery ? $fromDocQuery : $bitext->srclang.'/%';
        echo('<tr><td class="doclist"><form action="'.$_SERVER['PHP_SELF'].'" method="get" style="display: inline;">');
        echo('<input type="hidden" id="langpair" name="langpair" value="'.$bitext->langpair.'">');
        echo('<input type="hidden" id="offset" name="offset" value="0">');
        echo('<input type="hidden" id="limit" name="limit" value="10">');
        echo('<input type="hidden" id="corpus" name="corpus" value="'.$bitext->corpus.'">');
        echo('<input type="hidden" id="version" name="version" value="'.$bitext->version.'">');
        echo('<input type="text" name="fromDocQuery" id="fromDocQuery" value="'.$queryValue.'" required /></td>');
        echo('<td class="docliststars"><input type="submit" name="docsearch" value="search document" />');
        echo('</form></td>');
    }
    else{
        echo '<script>var x = document.getElementById("next");x.style.visibility = "hidden";</script>';
    }
    echo('</table>');
}


function print_corpus_list($bitext){
    $corpora = $bitext->corpora();
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
        $query = make_query(['corpus' => $corpus, 'version' => $version, 'search' => '']);
        $link = '<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">';
        $versionLinks .= ' ['.$link.$version.'</a>]';
    }
    // if ($corpus) echo "<li>$link$corpus</a>: $versionLinks</li>\n";
    // if ($corpus) echo "<li>$corpus: [${link}latest</a>] $versionLinks</li>\n";
    if ($corpus) echo "<li>$corpus: $versionLinks</li>\n";
    echo('</ul>');
}

function print_langpair_list(){
    global $DB_DIR;
    
    // $langpairs = array('fi-uk');
    $langpairs = find_bitext_dbs($DB_DIR);
    ksort($langpairs);

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


function find_bitext_dbs($path){    
    $langpairs = array();    
    if ($handle = opendir($path)) {
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
    if ($handle = opendir($path.'/sqlite')) {
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
    return $langpairs;
}


?>
