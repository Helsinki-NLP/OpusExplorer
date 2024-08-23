<?php // -*-php-*-


function print_document_list($corpus, $version, $offset=0, $fromDocQuery='', $toDocQuery=''){
    global $bitextDBH, $srclang, $trglang, $langpair;
    global $showMaxDocuments, $showRatings, $showMyRatings;

    // $fromDocQuery = 'de/1988/%';
    
    $condition = "WHERE corpus='$corpus' AND version='$version'";
    if ($fromDocQuery) $condition .= " AND fromDoc LIKE '".$fromDocQuery."'";
    if ($toDocQuery) $condition .= " AND toDoc LIKE '".$toDocQuery."'";    
    
    $limit = "LIMIT $showMaxDocuments";
    if ($offset){
        $limit .= " OFFSET $offset";
    }
    // echo "SELECT DISTINCT fromDoc,toDoc,rowid FROM bitexts $condition $limit";
    $results = $bitextDBH->query("SELECT DISTINCT fromDoc,toDoc,rowid FROM bitexts $condition $limit");
    
    if ($results){
        echo 'select a source document: ';
        bitext_navigation($offset, $showMaxDocuments, false);
        bitext_rating_options();
        echo('<br/><br/><table class="doclist">');
        
        $count = 0;
        while ($row = $results->fetchArray(SQLITE3_NUM)) {
            $count++;
            $fromDocID = get_source_document_id($corpus, $version, $row[0]);
            $toDocID = get_target_document_id($corpus, $version, $row[1]);
            $query = make_query(['fromDoc' => $row[0],
                                 'toDoc' => $row[1],
                                 'bitextID' => $row[2],
                                 'fromDocID' => $fromDocID,
                                 'toDocID' => $toDocID,
                                 'offset' => 0]);
    
            echo '<tr><td class="doclist"><a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">'.$row[0].'</a></td>';
            if ($showMyRatings){
                $rating = get_bitext_rating($row[2],$_SESSION['user']);
                echo('<td class="docliststars">');
                print_stars($rating);
                echo('</td>');
            }
            if ($showRatings){
                $rating = get_bitext_rating($row[2]);
                echo('<td class="docliststars">');
                print_stars($rating);
                echo('</td>');
            }
            echo("</tr>\n");
        }
        if ($count >= $showMaxDocuments || $fromDocQuery){
            $queryValue = $fromDocQuery ? $fromDocQuery : $srclang.'/%';
            echo('<tr><td class="doclist"><form action="'.$_SERVER['PHP_SELF'].'" method="get" style="display: inline;">');
            echo('<input type="hidden" id="langpair" name="langpair" value="'.$langpair.'">');
            echo('<input type="hidden" id="offset" name="offset" value="0">');
            echo('<input type="hidden" id="limit" name="limit" value="10">');
            echo('<input type="hidden" id="corpus" name="corpus" value="'.$corpus.'">');
            echo('<input type="hidden" id="version" name="version" value="'.$version.'">');
            echo('<input type="text" name="fromDocQuery" id="fromDocQuery" value="'.$queryValue.'" required /></td>');
            echo('<td class="docliststars"><input type="submit" name="docsearch" value="search document" />');
            echo('</form></td>');
        }
        else{
            echo '<script>var x = document.getElementById("next");x.style.visibility = "hidden";</script>';
        }
        echo('</table>');
    }
}


function print_corpus_list(){
    global $bitextDBH;
    echo('<ul>');
    $results = $bitextDBH->query("SELECT DISTINCT corpus,version FROM aligned_corpora ORDER BY corpus");
    // $results = $bitextDBH->query("SELECT DISTINCT corpus,version FROM bitexts ORDER BY corpus");
    if ($results){
        $corpus = '';
        $versions = array();
        $link = '';
        $versionLinks = '';
        while ($row = $results->fetchArray(SQLITE3_NUM)) {
            if ($row[0] != $corpus){
                // if ($corpus) echo "<li>$link$corpus</a>: $versionLinks</li>\n";
                // if ($corpus) echo "<li>$corpus: [${link}latest</a>] $versionLinks</li>\n";
                if ($corpus) echo "<li>$corpus: $versionLinks</li>\n";
                $corpus = $row[0];
                $versionLinks = '';
            }
            array_push($versions,$row[1]);
            $version = $row[1];
            $query = make_query(['corpus' => $corpus, 'version' => $version, 'search' => '']);
            $link = '<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">';
            $versionLinks .= ' ['.$link.$version.'</a>]';
        }
        // if ($corpus) echo "<li>$link$corpus</a>: $versionLinks</li>\n";
        // if ($corpus) echo "<li>$corpus: [${link}latest</a>] $versionLinks</li>\n";
        if ($corpus) echo "<li>$corpus: $versionLinks</li>\n";
    }
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
