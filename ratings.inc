<?php // -*-php-*-

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
    return $rating;
}

function get_alignment_rating($bitextID,$linkID,$user=''){
    /*
    global $DB_DIR, $langpair;
    $algRatingDB = $DB_DIR.$langpair.'.stars.db';
    if (! file_exists($algRatingDB) ) return 0;

    $DBH = new SQLite3($algRatingDB,SQLITE3_OPEN_READONLY);
    */
    
    global $algStarsDbFile;
    if (! file_exists($algStarsDbFile) ) return 0;

    $DBH = new SQLite3($algStarsDbFile,SQLITE3_OPEN_READONLY);
    
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
    global $algStarsDbFile;
    // echo("add rating to $algStarsDbFile");
    if (! file_exists($algStarsDbFile) ){
        $DBH = new SQLite3($algStarsDbFile);
            
        $DBH->exec('CREATE TABLE IF NOT EXISTS ratings (bitextID INTEGER, linkID INTEGER, user TEXT, rating INTEGER)');
        $DBH->exec('CREATE UNIQUE INDEX IF NOT EXISTS idx_ratings ON ratings (bitextID, linkID, user)');
    }
    else{
        $DBH = new SQLite3($algStarsDbFile);
    }
    $DBH->exec("UPDATE OR IGNORE ratings SET rating=$rating WHERE bitextID=$bitextID AND linkID=$linkID AND user='$user'");
    $DBH->exec("INSERT OR IGNORE INTO ratings (bitextID, linkID, user, rating) VALUES ($bitextID,$linkID,'$user',$rating)");
    $DBH->close();
}


function delete_alignment_rating($linkID,$user=''){
    global $algStarsDbFile;
    // echo("delete rating in $algStarsDbFile for $linkID (user=$user)");
    if (file_exists($algStarsDbFile) ){
        $DBH = new SQLite3($algStarsDbFile);
        $condition = "linkID=$linkID";
        if ($user) $condition .= "AND user='$user'";
        // echo("DELETE FROM ratings WHERE $condition");
        $DBH->exec("DELETE FROM ratings WHERE $condition");
        $DBH->close();
    }
}



?>
