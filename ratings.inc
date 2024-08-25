<?php // -*-php-*-


function bitext_rating_options(){
    global $showRatings, $showMyRatings;

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



function printRatingStars($bitext,$linkID){
    $rating = $bitext->getMyAlignmentRating($linkID);
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

function printAverageRatings($bitext,$linkID){
    $rating = $bitext->getAverageAlignmentRating($linkID);
    printStars($rating);
    return $rating;
}

function printStars($rating){
    echo '<span style="color: #ffbb00;">';
    for ($x=1;$x<=$rating+0.25;$x++) echo '★';
    if ($rating >= $x-0.75 && $rating <= $x-0.25){ echo '☆';$x++; }
    echo '</span>';
    if ($rating < 5){
        for ($y=$x;$y<=5;$y++) echo '☆';
    }
}


?>
