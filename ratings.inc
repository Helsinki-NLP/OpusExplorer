<?php // -*-php-*-


function bitext_rating_options(){
    bitext_average_rating_option();
    bitext_user_rating_option();
}


function bitext_user_rating_option(){
    global $showMyRatings;

    if ($showMyRatings){
        $query = make_query(['showMyRatings' => 0]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">my rating</a>');
    }
    else{
        $query = make_query(['showMyRatings' => 1]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'"><strike>my rating</strike></a>');
    }
}

function bitext_average_rating_option(){
    global $showRatings;

    if ($showRatings){
        $query = make_query(['showRatings' => 0]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">avg rating</a>');
    }
    else{
        $query = make_query(['showRatings' => 1]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'"><strike>avg rating</strike></a>');
    }
}



function printRatingStars($bitext,$linkID){
    $rating = $bitext->getMyAlignmentRating($linkID);
    $bitextID = $bitext->id();
    $stars = $rating / 20;
    // for ($x=1;$x<=$stars;$x++){
    for ($x=1;$x<=$stars+0.25;$x++){
        $query = make_query(['rating' => $x*20, 'bitextID' => $bitextID, 'linkID' => $linkID]);
        echo '<a style="text-decoration: none; color: #ffbb00;" href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">★</a>';
    }
    if ($stars >= $x-0.75 && $stars <= $x-0.25){
        $query = make_query(['rating' => $x*20, 'bitextID' => $bitextID, 'linkID' => $linkID]);
        echo '<a style="text-decoration: none; color: #ffbb00;" href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">☆</a>';
        $x++;
    }
    if ($stars < 5){
        for ($y=$x;$y<=5;$y++){
            $query = make_query(['rating' => $y*20, 'bitextID' => $bitextID, 'linkID' => $linkID]);
            echo '<a style="text-decoration: none; color: black;" href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">☆</a>';
        }
    }
}

function printAverageRatings($bitext,$linkID){
    $rating = $bitext->getAverageAlignmentRating($linkID);
    printStars($rating);
    return $rating;
}

function printStars($rating){
    echo '<span style="color: #ffbb00;">';
    $stars = $rating / 20;
    for ($x=1;$x<=$stars+0.25;$x++) echo '★';
    if ($stars >= $x-0.75 && $stars <= $x-0.25){ echo '☆';$x++; }
    echo '</span>';
    if ($stars < 5){
        for ($y=$x;$y<=5;$y++) echo '☆';
    }
}


?>
