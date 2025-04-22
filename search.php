<?php


function clean_input($data) {
  $data = trim($data);
  $data = stripslashes($data);
  $data = strip_tags($data);
  $data = htmlspecialchars($data);
  return $data;
}

$langpair = (isset($_GET['langpair'])) ? $_GET['langpair'] : 'de-sv';
$corpus = (isset($_GET['corpus'])) ? $_GET['corpus'] : 'Europarl';
$version = (isset($_GET['version'])) ? $_GET['version'] : 'v8';
$query = (isset($_GET['query'])) ? $_GET['query'] : 'trainieren';
$limit = (isset($_GET['max'])) ? $_GET['max'] : 10;
$offset = (isset($_GET['offset'])) ? $_GET['offset'] : 0;

list($srclang,$trglang) = explode('-',$langpair);


echo('<form action="search.php">');
echo('<label for="query">query (source language): </label>');
echo('<input type="text" name="query" id="query" value="'.$query.'" required />');
echo('<input type="submit" value="search" />');
echo('</form>');


// phpinfo();


$srcDBH = new SQLite3('/media/OpusIndex/'.$srclang.'.fts5.db',SQLITE3_OPEN_READONLY);
$trgDBH = new SQLite3('/media/OpusIndex/'.$trglang.'.fts5.db',SQLITE3_OPEN_READONLY);

$linksDBH = new SQLite3('/media/OpusIndex/'.$langpair.'.linked.db',SQLITE3_OPEN_READONLY);
$algDBH = new SQLite3('/media/OpusIndex/'.$langpair.'.db',SQLITE3_OPEN_READONLY);

$srcIdxDBH = new SQLite3('/media/OpusIndex/'.$srclang.'.ids.db',SQLITE3_OPEN_READONLY);
$trgIdxDBH = new SQLite3('/media/OpusIndex/'.$trglang.'.ids.db',SQLITE3_OPEN_READONLY);


$linksDBH = new SQLite3('/media/OpusIndex/'.$langpair.'.linked.db',SQLITE3_OPEN_READONLY);
$linksDBH->exec("ATTACH DATABASE '/media/OpusIndex/".$srclang.".fts5.db' AS srcdb");
$linksDBH->exec("ATTACH DATABASE '/media/OpusIndex/".$langpair.".db' AS algdb");
// $linksDBH->exec("ATTACH DATABASE '/media/OpusIndex/".$trglang.".fts5.db' AS trgdb");


$bitexts = array();
$corpusID = 0;
$limitstr = '';
$offsetstr = '';

if ($query){
    $condition = "src.sentence MATCH '".$query."'";
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
    $results = $linksDBH->query(
        "SELECT bitextID, sentID, srcIDs, trgIDs, highlight(sentences,0, '<b>', '</b>') sentence 
            FROM srcdb.sentences as src
            INNER JOIN linkedsource ON src.rowid=linkedsource.sentID
            INNER JOIN algdb.links AS links ON links.rowid=linkedsource.linkID
            WHERE ".$condition.$limitstr.$offsetstr);
    $queryTime = microtime(true) - $start;
}

$fetchDocIdTime = 0;
$fetchSentenceIdTime = 0;
$fetchSentenceTime = 0;

if ($results){
    echo("<table class='hbitext'>\n");
    while ($row = $results->fetchArray(SQLITE3_NUM)) {
        $bitextID=$row[0];
        $srcID=$row[1];
        $srcIDs=explode(' ',trim($row[2]));
        $trgIDs=explode(' ',trim($row[3]));
        $sent=$row[4];
        $start = microtime(true);
        $docIDs = get_bitext_docids($bitextID);
        $fetchDocIdTime += microtime(true) - $start;

        $srcSents = array();
        foreach ($srcIDs as $sentID){
            $start = microtime(true);
            $id = get_sentence_id($srcIdxDBH, $docIDs[0], $sentID);
            $fetchSentenceIdTime += microtime(true) - $start;
            if ($id == $srcID)
                array_push($srcSents, $sent);
            else{
                $start = microtime(true);
                array_push($srcSents, get_sentence($srcDBH, $id));
                $fetchSentenceTime += microtime(true) - $start;
            }
        }
        $trgSents = array();
        foreach ($trgIDs as $sentID){
            $start = microtime(true);
            $id = get_sentence_id($trgIdxDBH, $docIDs[1], $sentID);
            $fetchSentenceIdTime += microtime(true) - $start;
            $start = microtime(true);
            array_push($trgSents, get_sentence($trgDBH, $id));
            $fetchSentenceTime += microtime(true) - $start;
        }

        echo("<tr>\n<td class='hbitext-srcid'>$bitextID</td>\n<td class='hbitext-src'>");
        echo(implode(' ',$srcSents));
        echo("</td>\n<td class='hbitext-trg'>");
        echo(implode(' ',$trgSents));
        echo("\n</tr>\n");
    }
    echo('</table>');

    $totalTime = $queryTime + $fetchDocIdTime + $fetchSentenceIdTime + $fetchSentenceTime;
    echo('<ul>');
    echo("<li>".$queryTime." (query time)</li>\n");
    echo("<li>".$fetchDocIdTime." (time for fetching IDs)</li>\n");
    echo("<li>".$fetchSentenceIdTime." (time for fetching sentence IDs)</li>\n");
    echo("<li>".$fetchSentenceTime." (time for fetching sentences)</li>\n");
    echo("<li>".$totalTime." (total time for fetching data)</li>\n");
    echo('</ul>');
}


function get_bitext_docids($bitextID){
    global $algDBH, $srcIdxDBH, $trgIdxDBH;
    if (isset($bitexts[$bitextID])){
        return $bitexts[$bitextID];
    }
    $results = $algDBH->query("SELECT * FROM bitexts WHERE rowid=$bitextID");
    if ($results){
        while ($row = $results->fetchArray(SQLITE3_NUM)) {
            $corpus = $row[0];
            $version = $row[1];
            $fromDoc = $row[2];
            $toDoc = $row[3];
            $docres = $srcIdxDBH->query("SELECT rowid FROM documents WHERE corpus='$corpus' AND version='$version' AND document='$fromDoc'");
            while ($docrow = $docres->fetchArray(SQLITE3_NUM)) {
                $fromDocID = $docrow[0];
            }
            $docres = $trgIdxDBH->query("SELECT rowid FROM documents WHERE corpus='$corpus' AND version='$version' AND document='$toDoc'");
            while ($docrow = $docres->fetchArray(SQLITE3_NUM)) {
                $toDocID = $docrow[0];
            }
            $bitexts[$bitextID] = array($fromDocID,$toDocID);
            return $bitexts[$bitextID];
        }
    }
    return array(0,0);
}


function get_sentence_id($IdxDBH, $docID, $sentID){
    $results = $IdxDBH->query("SELECT id FROM sentids WHERE docID=$docID AND sentID='$sentID'");
    if ($results){
        while ($row = $results->fetchArray(SQLITE3_NUM)) {
            return $row[0];
        }
    }
}


function get_sentence($SentDBH, $id){
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




?>
