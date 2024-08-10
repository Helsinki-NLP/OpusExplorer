<?php


function clean_input($data) {
  $data = trim($data);
  $data = stripslashes($data);
  $data = strip_tags($data);
  $data = htmlspecialchars($data);
  return $data;
}

$query = (isset($_GET['query'])) ? $_GET['query'] : 'ikkje lesbar';
$limit = (isset($_GET['max'])) ? $_GET['max'] : 100;


echo('<form action="search.php">');
echo('<label for="query">query (source language): </label>');
echo('<input type="text" name="query" id="query" value="'.$query.'" required />');
echo('<input type="submit" value="search" />');
echo('</form>');


// phpinfo();


$srcDBH = new SQLite3('/media/OpusIndex/nn.fts5.db',SQLITE3_OPEN_READONLY);
$trgDBH = new SQLite3('/media/OpusIndex/se.fts5.db',SQLITE3_OPEN_READONLY);

$linksDBH = new SQLite3('/media/OpusIndex/nn-se.linked.db',SQLITE3_OPEN_READONLY);
$algDBH = new SQLite3('/media/OpusIndex/nn-se.db',SQLITE3_OPEN_READONLY);

$srcIdxDBH = new SQLite3('/media/OpusIndex/nn.ids.db',SQLITE3_OPEN_READONLY);
$trgIdxDBH = new SQLite3('/media/OpusIndex/se.ids.db',SQLITE3_OPEN_READONLY);


$linksDBH = new SQLite3('/media/OpusIndex/nn-se.linked.db',SQLITE3_OPEN_READONLY);
$linksDBH->exec("ATTACH DATABASE '/media/OpusIndex/nn.fts5.db' AS srcdb");
$linksDBH->exec("ATTACH DATABASE '/media/OpusIndex/nn-se.db' AS algdb");
// $linksDBH->exec("ATTACH DATABASE '/media/OpusIndex/se.fts5.db' AS trgdb");


$bitexts=array();

if ($query){
    $results = $linksDBH->query(
        "SELECT bitextID, sentID, srcIDs, trgIDs, highlight(sentences,0, '<b>', '</b>') sentence 
            FROM srcdb.sentences as src
            INNER JOIN linkedsource ON src.rowid=linkedsource.sentID
            INNER JOIN algdb.links AS links ON links.rowid=linkedsource.linkID
            WHERE src.sentence MATCH '".$query."' LIMIT ".$limit);
}

if ($results){
    echo("<table border='1'>\n");
    while ($row = $results->fetchArray(SQLITE3_NUM)) {
        $bitextID=$row[0];
        $srcID=$row[1];
        $srcIDs=explode(' ',trim($row[2]));
        $trgIDs=explode(' ',trim($row[3]));
        $sent=$row[4];
        $docIDs = get_bitext_docids($bitextID);

        $srcSents = array();
        foreach ($srcIDs as $sentID){
            $id = get_sentence_id($srcIdxDBH, $docIDs[0], $sentID);
            if ($id == $srcID){
                array_push($srcSents, $sent);
            }
            else{
                array_push($srcSents, get_sentence($srcDBH, $id));
            }
        }
        $trgSents = array();
        foreach ($trgIDs as $sentID){
            $id = get_sentence_id($trgIdxDBH, $docIDs[1], $sentID);
            array_push($trgSents, get_sentence($trgDBH, $id));
        }

        echo("<tr>\n<td>$bitextID</td>\n<td style='text-align: right'>");
        echo(implode(' ',$srcSents));
        echo("</td>\n<td>");
        echo(implode(' ',$trgSents));
        echo("\n</tr>\n");
    }
    echo('</table>');
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
