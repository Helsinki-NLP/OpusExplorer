<?php
// session_reset();
session_start();
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
  <title>OPUS Explorer</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
table.bitext   {width: 100%;}
td.leftalign   {text-align: left;width: 40%;padding-left: 10px;}
td.rightalign  {text-align: right;width: 40%;padding-right: 10px;}
td.centeralign  {text-align: center;}
table, th {
  border: 1px solid;
  border-collapse: collapse;
}
td {
  border: 1px dotted;
}
  </style>
</head>
<body>

<?php



include('env.inc');

$USER_DATADIR    = $DB_DIR;
$USER_NAME_FILE  = $USER_DATADIR.'/users.php';
$USER_DB         = $USER_DATADIR.'/users.db';
$ALLOW_NEW_USERS = 1;

include('users.inc');

check_setup();

if (!logged_in()){
	exit;
}


list($srclang, $trglang, $langpair) = get_langpair();

$corpus = get_param('corpus');
$version = get_param('version');
$fromDoc = get_param('fromDoc');
$toDoc = get_param('toDoc');
$alignType = get_param('aligntype');
$offset = get_param('offset',0);

$bitextID  = get_param('bitextID');
$fromDocID = get_param('fromDocID');
$toDocID   = get_param('toDocID');

$showScores = get_param('showScores',1);
$showLengthRatio = get_param('showLengthRatio',0);
$showRatings = get_param('showRatings',0);
$showMyRatings = get_param('showMyRatings',1);

$showMaxAlignments = get_param('showMaxAlignments',$SHOW_ALIGNMENTS_LIMIT);
$showMaxDocuments = get_param('showMaxDocuments',$DOCUMENT_LIST_LIMIT);


## check whether we have a new rating to take care of

$linkID = get_param('linkID',0);
$rating = get_param('rating',0);

if ($rating){
    add_alignment_rating($bitextID,$linkID,$_SESSION['user'],$rating);
    delete_param('linkID');
    delete_param('rating');
}


/*
$langpair = 'fi-sv';
$srclang = 'fi';
$trglang = 'sv';
$corpus = 'OpenSubtitles';
$version = 'v2012';
$fromDoc = 'fi/1941/25528/3182613_1of1.xml';
$toDoc = 'sv/1941/25528/3298325_1of1.xml';
*/

/////////////////////////////////////////////////////////////////
// menu
/////////////////////////////////////////////////////////////////


$query = make_query(['srclang' => '', 'trglang' => '', 'langpair' => '',
                     'corpus' => '', 'fromDoc' => '', 'toDoc' => '', 'aligntype' => '', 'offset' => 0]);
echo('<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">OPUS</a> / ');

if ($srclang && $trglang){
    $query = make_query(['corpus' => '', 'fromDoc' => '', 'toDoc' => '', 'aligntype' => '', 'offset' => 0]);
    echo('<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">'.$langpair.'</a> / ');
    if ($corpus && $version){
        $query = make_query(['fromDoc' => '', 'toDoc' => '', 'aligntype' => '', 'offset' => 0]);
        echo('<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">'.$corpus.' / '.$version.'</a> / ');
        if ($fromDoc && $toDoc){
            $query = make_query(['aligntype' => '', 'offset' => 0]);
            echo('<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">'.$fromDoc.'</a> / ');
            foreach ($ALIGN_TYPES as $type){
                $query = make_query(['aligntype' => $type, 'offset' => 0]);
                echo('<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">'.$type.'</a> / ');
            }
            // TODO: other is very inefficient because of negative conditions (subqueries instead?)
            $query = make_query(['aligntype' => 'other', 'offset' => 0]);
            echo('<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">other</a>');
        }
    }
}

echo('</br><hr>');

/////////////////////////////////////////////////////////////////
// content
/////////////////////////////////////////////////////////////////

if ($srclang && $trglang){

    $srcDbFile    = $DB_DIR.$srclang.'.db';
    $trgDbFile    = $DB_DIR.$trglang.'.db';
    $srcIdxDbFile = $DB_DIR.$srclang.'.ids.db';
    $trgIdxDbFile = $DB_DIR.$trglang.'.ids.db';
    $algDbFile    = $DB_DIR.$langpair.'.db';

    $srcDBH    = new SQLite3($srcDbFile,SQLITE3_OPEN_READONLY);
    $srcIdxDBH = new SQLite3($srcIdxDbFile,SQLITE3_OPEN_READONLY);
    $trgDBH    = new SQLite3($trgDbFile,SQLITE3_OPEN_READONLY);
    $trgIdxDBH = new SQLite3($trgIdxDbFile,SQLITE3_OPEN_READONLY);
    $algDBH    = new SQLite3($algDbFile,SQLITE3_OPEN_READONLY);

    // echo("---$algDbFile--");
    
    if ($corpus && $version){
        if ($fromDoc && $toDoc){
            //print_links($corpus, $version, $fromDoc, $toDoc);
            print_bitext($corpus, $version, $fromDoc, $toDoc,
                         $fromDocID, $toDocID, $bitextID,
                         $alignType, $offset);
        }
        else{
            print_document_list($corpus, $version, $offset);
        }
    }
    else{
        print_corpus_list();
    }
}
else{
    print_langpair_list();
}


/////////////////////////////////////////////////////////////////
// functions
/////////////////////////////////////////////////////////////////

function get_bitextid($corpus, $version, $fromDoc, $toDoc){
    global $algDBH;
    
    $results = $algDBH->query("SELECT rowid FROM bitexts WHERE corpus='$corpus' AND version='$version' AND fromDoc='$fromDoc' AND toDoc='$toDoc'");
    if ($results){
        while ($row = $results->fetchArray(SQLITE3_NUM)) {
            return $row[0];
        }
    }
    return undef;
}


////////////////////
// print bitext display options
///////////////////

function bitext_display_options(){
    global $showScores, $showLengthRatio, $showRatings, $showMyRatings;
    
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
    if ($showRatings){
        $query = make_query(['showRatings' => 0]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'"> hide user ratings</a>');
    }
    else{
        $query = make_query(['showRatings' => 1]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'"> show user ratings</a>');
    }
    if ($showMyRatings){
        $query = make_query(['showMyRatings' => 0]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'"> hide my ratings</a>');
    }
    else{
        $query = make_query(['showMyRatings' => 1]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'"> show my ratings</a>');
    }
}

////////////////////
// print bitext navigation bar
///////////////////

function bitext_navigation($offset, $showMaxAlignments){

    $lastentry = $offset + $showMaxAlignments;

    if ($offset){
        $query = make_query(['offset' => 0]);
        echo('<a id="prev" href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">start</a>');
        $previous = $offset - $showMaxAlignments;
        if ($previous < 0){ $previous = 0; }
        $query = make_query(['offset' => $previous]);
        echo(' / <a id="prev" href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">prev</a>');
    }
    else{
        echo('start / <a id="prev">prev</a>');
        echo '<script>var x = document.getElementById("prev");x.style.visibility = "hidden";</script>';
    }
    // echo(" / [$offset:$lastentry]");
    $query = make_query(['offset' => $lastentry]);
    echo(' / <a id="next" href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'"> next</a>');

}

function get_alignments($corpus, $version, $fromDoc, $toDoc, $fromDocID=0, $toDocID=0, $bitextID=0, $type='all', $offset=0){
    global $algDBH, $showMaxAlignments, $ALIGN_TYPES;
    
    if ($bitextID){
        $conditions = "WHERE bitextID=$bitextID";
        $table = 'links';
    }
    else{
        $conditions = "WHERE corpus='$corpus' AND version='$version' AND fromDoc='$fromDoc' AND toDoc='$toDoc'";
        $table = 'alignments';
    }
    if ($type){
        if ($type == 'other'){
            foreach ($ALIGN_TYPES as $type){
                $conditions .= " AND NOT alignType='$type'";
            }
        }
        else{
            $conditions .= " AND alignType='$type'";
        }
    }
    $limit = "LIMIT $showMaxAlignments";
    if ($offset){
        $limit .= " OFFSET $offset";
    }
    return $algDBH->query("SELECT srcIDs,trgIDs,alignerScore,rowid FROM $table $conditions ORDER BY rowid $limit");
}

function print_bitext($corpus, $version, $fromDoc, $toDoc, $fromDocID=0, $toDocID=0, $bitextID=0, $type='all', $offset=0){
    global $srcDBH, $srcIdxDBH, $trgDBH, $trgIdxDBH;
    global $showMaxAlignments, $showScores, $showLengthRatio, $showRatings, $showMyRatings;

    if (! $bitextID){
        $bitextID = get_bitextid($corpus, $version, $fromDoc, $toDoc);
        if ($bitextID) set_param('bitextID',$bitextID);
    }

    $results = get_alignments($corpus, $version, $fromDoc, $toDoc, $fromDocID, $toDocID, $bitextID, $type, $offset);

    if ($results){

        bitext_navigation($offset, $showMaxAlignments);
        bitext_display_options();
        if ($showMyRatings){
            echo(" / rate the overall bitext quality: ");
            print_rating_stars($bitextID,0);
        }
        if ($showRatings){
            echo(" (");
            print_average_ratings($bitextID,0);
            echo(")");
        }
        
        echo('<table class="bitext">');
        echo("<tr><th>IDs</th><th>$fromDoc</th>");
        if ($showScores) echo('<th>score</th>');
        if ($showLengthRatio) echo('<th>ratio</th>');
        echo("<th>$toDoc</th><th>IDs</th>");
        if ($showMyRatings) echo('<th>my ratings</th>');
        if ($showRatings) echo('<th>ratings</th>');
        echo("</tr>");
        $i = 0;
        while ($row = $results->fetchArray(SQLITE3_NUM)) {
            $i++;
            $srcIDs = explode(' ',$row[0]);
            $trgIDs = explode(' ',$row[1]);
            $linkID = $row[3];
            
            $srcSents = array();
            foreach ($srcIDs as $srcID){
                array_push($srcSents,get_sentence($srcDBH, $srcIdxDBH, $corpus, $version, $fromDoc, $srcID, $fromDocID));
            }
            $trgSents = array();
            foreach ($trgIDs as $trgID){
                array_push($trgSents,get_sentence($trgDBH, $trgIdxDBH, $corpus, $version, $toDoc, $trgID, $toDocID));
            }
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
        // hide next button if we are at the end of the document
        if ($i < $showMaxAlignments){
            // echo '<script>var x = document.getElementById("next");x.style.display = "none";</script>';
            echo '<script>var x = document.getElementById("next");x.style.visibility = "hidden";</script>';
        }
    }
}

function print_rating_stars($bitextID,$linkID){
    $rating = get_alignment_rating($bitextID,$linkID,$_SESSION['user']);
    for ($x=1;$x<=$rating;$x++){
        $query = make_query(['rating' => $x, 'bitextID' => $bitextID, 'linkID' => $linkID]);
        echo '<a style="text-decoration: none; color: #ffbb00;" href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">★</a>';
    }
    if ($rating < 5){
        for ($y=$x;$y<=5;$y++){
            $query = make_query(['rating' => $y, 'bitextID' => $bitextID, 'linkID' => $linkID]);
            echo '<a style="text-decoration: none; color: black;" href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">☆</a>';
        }
    }
}

function print_average_ratings($bitextID,$linkID){
    $rating = get_alignment_rating($bitextID,$linkID);
    echo '<span style="color: #ffbb00;">';
    for ($x=1;$x<=$rating+0.25;$x++) echo '★';
    if ($rating >= $x-0.75 && $rating <= $x-0.25){ echo '☆';$x++; }
    echo '</span>';
    if ($rating < 5){
        for ($y=$x;$y<=5;$y++) echo '☆';
    }
}

function get_alignment_rating($bitextID,$linkID,$user=''){
    global $DB_DIR, $langpair;
    $algRatingDB = $DB_DIR.$langpair.'.stars.db';
    if (! file_exists($algRatingDB) ) return 0;

    $DBH = new SQLite3($algRatingDB,SQLITE3_OPEN_READONLY);
    if ($user){
        $results = $DBH->query("SELECT rating FROM ratings WHERE bitextID=$bitextID AND linkID=$linkID AND user='$user'");
        if ($results){
            while ($row = $results->fetchArray(SQLITE3_NUM)) {
                return $row[0];
            }
        }
    }
    else{
        $results = $DBH->query("SELECT AVG(rating) FROM ratings WHERE bitextID=$bitextID AND linkID=$linkID");
        if ($results){
            while ($row = $results->fetchArray(SQLITE3_NUM)) {
                return $row[0];
            }
        }
    }
    return 0;
}


function add_alignment_rating($bitextID,$linkID,$user,$rating){
    global $DB_DIR, $langpair;
    $algRatingDB = $DB_DIR.$langpair.'.stars.db';
    if (! file_exists($algRatingDB) ){
        $DBH = new SQLite3($algRatingDB);
        $DBH->exec('CREATE TABLE IF NOT EXISTS ratings (bitextID INTEGER, linkID INTEGER, user TEXT, rating INTEGER)');
        $DBH->exec('CREATE UNIQUE INDEX IF NOT EXISTS idx_ratings ON ratings (bitextID, linkID, user)');
    }
    else{
        $DBH = new SQLite3($algRatingDB);
    }
    $DBH->exec("UPDATE OR IGNORE ratings SET rating=$rating WHERE bitextID=$bitextID AND linkID=$linkID AND user='$user'");
    $DBH->exec("INSERT OR IGNORE INTO ratings (bitextID, linkID, user, rating) VALUES ($bitextID,$linkID,'$user',$rating)");
}


function get_sentence_id($SentDBH, $IdxDBH, $corpus, $version, $document, $sentID, $docID=0){
    if ($docID){
        $results = $IdxDBH->query("SELECT id FROM sentids WHERE docID=$docID AND sentID='$sentID'");
        if ($results){
            while ($row = $results->fetchArray(SQLITE3_NUM)) {
                return $row[0];
            }
        }
    }

    $condition = "WHERE corpus='$corpus' AND version='$version' AND document='$document' AND sentID='$sentID'";
    $results = $IdxDBH->query("SELECT id FROM sentindex $condition");
    if ($results){
        while ($row = $results->fetchArray(SQLITE3_NUM)) {
            return $row[0];
        }
    }
    return 0;
}

function get_sentence($SentDBH, $IdxDBH, $corpus, $version, $document, $sentID, $docID=0){
    if (!$sentID) return '';
    $id = get_sentence_id($SentDBH, $IdxDBH, $corpus, $version, $document, $sentID, $docID);
    if ($id){
        $results = $SentDBH->query("SELECT sentence FROM sentences WHERE rowid='$id'");
        if ($results){
            while ($row = $results->fetchArray(SQLITE3_NUM)) {
                return $row[0];
            }
        }
    }
    return 'SENTENCE NOT FOUND';
}


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


## could also use this query to see whether the documents table existsL
## SELECT name FROM sqlite_master WHERE name='documents'

function get_source_document_id($corpus, $version, $document){
    global $srcIdxDbFile, $srcIdxDBH;
    global $DB_DIR,$srclang;
    if ( $srcIdxDbFile == $DB_DIR.$srclang.'.ids.db' ){
        $condition = "WHERE corpus='$corpus' AND version='$version' AND document='$document'";
        $results = $srcIdxDBH->query("SELECT rowid FROM documents $condition");
        if ($results){
            while ($row = $results->fetchArray(SQLITE3_NUM)) {
                return $row[0];
            }
        }
    }
    return 0;
}

function get_target_document_id($corpus, $version, $document){
    global $trgIdxDbFile, $trgIdxDBH;
    global $DB_DIR,$trglang;
    if ( $trgIdxDbFile == $DB_DIR.$trglang.'.ids.db' ){
        $condition = "WHERE corpus='$corpus' AND version='$version' AND document='$document'";
        $results = $trgIdxDBH->query("SELECT rowid FROM documents $condition");
        if ($results){
            while ($row = $results->fetchArray(SQLITE3_NUM)) {
                return $row[0];
            }
        }
    }
    return 0;
}


function print_document_list($corpus, $version, $offset=0){
    global $algDBH;
    global $showMaxDocuments;
    
    $condition = "WHERE corpus='$corpus' AND version='$version'";

    /*
    $res = $algDBH->query("SELECT COUNT(*) FROM bitexts $condition");
    if ($res){
        if ($row = $res->fetchArray(SQLITE3_NUM)){
            if ($row[0] == 1){
                $res = $algDBH->query("SELECT fromDoc,toDoc,rowid FROM bitexts $condition");
                if ($res){
                    if ($row = $res->fetchArray(SQLITE3_NUM)){
                        $fromDoc = $row[0];
                        $toDoc = $row[1];
                        $fromDocID = get_source_document_id($corpus, $version, $row[0]);
                        $toDocID = get_target_document_id($corpus, $version, $row[1]);
                        $bitextID = $row[2];
                        set_param('fromDoc',$fromDoc);
                        set_param('toDoc',$toDoc);
                        set_param('bitextID',$bitextID);
                        set_param('fromDocID',$fromDocID);
                        set_param('toDocID',$toDocID);
                        print_bitext($corpus, $version, $fromDoc, $toDoc,
                                     $fromDocID, $toDocID, $bitextID,'all',0);
                        return undef;
                    }
                }
            }
        }
    }
    */
    
    $limit = "LIMIT $showMaxDocuments";
    if ($offset){
        $limit .= " OFFSET $offset";
    }
    $results = $algDBH->query("SELECT DISTINCT fromDoc,toDoc,rowid FROM bitexts $condition $limit");
    
    if ($results){
        if ($offset){
            $start = $offset - $showMaxDocuments;
            if ($start < 0) $start = 0;
            $query = make_query(['offset' => $start]);
            echo '<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">previous documents</a>'."\n";
        }
        else{
            echo 'select a source document:'."\n";
        }
        echo('<ul>');
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
            echo '<li><a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">'.$row[0].'</a></li>'."\n";
            // echo '<li><a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">'.$row[0].' - '.$row[1].'</a></li>'."\n";
            // echo($row[0]." - ".$row[1]."\n");
        }
        echo('</ul>');
        if ($count >= $showMaxDocuments){
            $query = make_query(['offset' => $offset + $count]);
            echo '<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">more documents</a>'."\n";
        }
    }
}

function print_corpus_list(){
    global $algDBH;
    echo('<ul>');
    $results = $algDBH->query("SELECT DISTINCT corpus,version FROM aligned_corpora ORDER BY corpus");
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
            $query = make_query(['corpus' => $corpus, 'version' => $version]);
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
    asort($langpairs);

    echo('<ul>');
    foreach ($langpairs as $langpair){
        list($srclang,$trglang) = explode('-',$langpair);
        $srclangname = Locale::getDisplayName($srclang, 'en');
        $trglangname = Locale::getDisplayName($trglang, 'en');
        $query = make_query(['langpair' => $langpair]);
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
            if (substr($entry,-3) == '.db') {
                $lang = basename($entry,'.db');
                $parts = explode('-',$lang);
                if (count($parts) == 2){
                    array_push($langpairs,$lang);
                }
            }
        }
        closedir($handle);
    }
    return $langpairs;
}

?>
