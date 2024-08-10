<?php


// phpinfo();


$style = (isset($_GET['style'])) ? $_GET['style'] : 'tsv';
$limit = (isset($_GET['max'])) ? $_GET['max'] : 100;

$srcDBH = new SQLite3('/media/OpusIndex/de.fts5.db',SQLITE3_OPEN_READONLY);
$trgDBH = new SQLite3('/media/OpusIndex/sv.fts5.db',SQLITE3_OPEN_READONLY);

$srcIdxDBH = new SQLite3('/media/OpusIndex/de.ids.db',SQLITE3_OPEN_READONLY);
$trgIdxDBH = new SQLite3('/media/OpusIndex/sv.ids.db',SQLITE3_OPEN_READONLY);

$algDBH = new SQLite3('/media/OpusIndex/de-sv.db',SQLITE3_OPEN_READONLY);

$starsDBH = new SQLite3('/media/OpusIndex/de-sv.stars.db',SQLITE3_OPEN_READONLY);
$starsDBH->exec("ATTACH DATABASE '/media/OpusIndex/de-sv.db' AS algdb");


$results = $starsDBH->query("SELECT links.bitextID,srcIDs,trgIDs,alignType,alignerScore,user,rating 
                             FROM ratings 
                             INNER JOIN algdb.links AS links ON links.rowid=ratings.linkID 
                             LIMIT $limit");

$bitexts=array();
if ($results){

    if ($style == 'table')
        echo("<table border='1'>\n");
    else
        echo("<pre>\n");
    while ($row = $results->fetchArray(SQLITE3_NUM)) {
        $bitextID=$row[0];
        $srcIDs=explode(' ',trim($row[1]));
        $trgIDs=explode(' ',trim($row[2]));
        $alignType=$row[3];
        $alignerScore=$row[4];
        $user=$row[5];
        $rating=$row[6];
        
        $docIDs = get_bitext_docids($bitextID);
        $corpus = $docIDs[2];
        $version = $docIDs[3];

        $srcSents = array();
        foreach ($srcIDs as $sentID){
            if ($id = get_sentence_id($srcIdxDBH, $docIDs[0], $sentID)){
                array_push($srcSents, get_sentence($srcDBH, $id));
            }
        }
        $trgSents = array();
        foreach ($trgIDs as $sentID){
            if ($id = get_sentence_id($trgIdxDBH, $docIDs[1], $sentID)){
                array_push($trgSents, get_sentence($trgDBH, $id));
            }
        }

        if ($style == 'table'){
            echo("<tr>\n<td>$bitextID</td>");
            echo("<td>$corpus</td><td>$version</td>\n");
            echo("<td>$alignType</td><td>$alignerScore</td>\n");
            echo("<td>$user</td><td>$rating</td>\n");
            echo("<td style='text-align: right'>");        
            echo(implode(' ',$srcSents));
            echo("</td>\n<td>");
            echo(implode(' ',$trgSents));
            echo("\n</tr>\n");
        }
        else{
            echo("$bitextID\t$corpus\t$version\t$alignType\t$alignerScore\t");
            echo("$user\t$rating\t");
            echo(implode(' ',$srcSents));
            echo("\t");
            echo(implode(' ',$trgSents));
            echo("\n");
        }
    }
    if ($style == 'table')
        echo("</table>\n");
    else
        echo("</pre>\n");
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
            $bitexts[$bitextID] = array($fromDocID,$toDocID,$corpus,$version);
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
