<?php // -*-php-*-


function bitext_options(){
    global $searchquery, $tableStyle;

    echo('/ <span class="option-selector">');
    echo('<a class="clickable"  id="score-option-selector" onclick="toggleOptions(\'score-option\');">scores</a>');
    echo('</span> ');
    
    echo('/ <span class="option-selector">');
    echo('<a class="clickable" id="rating-option-selector" onclick="toggleOptions(\'rating-option\');">ratings</a>');
    echo('</span> ');

    if ($searchquery) return;
    if ($tableStyle == 'edit') return;

    echo('/ <span class="option-selector">');
    echo('<a class="clickable" id="aligntype-option-selector" onclick="toggleOptions(\'aligntype-option\');">align&nbsp;types</a>');
    echo('</span> ');

    echo('/ <span class="option-selector">');
    echo('<a class="clickable" id="other-option-selector" onclick="toggleOptions(\'other-option\');">other</a>');
    echo('</span> ');

}

function score_options_menu(){
    global $showScores, $showLengthRatio;

    // change visibility of entire columns in a table does not work on Safari nor on iOS
    //
    // echo(' / <span id="hide-score-columns" onclick="toggleVisibility(\'bitext-score-column\')">scores</span>');
    // echo(' / <span id="hide-score-columns" onclick="toggleVisibility(\'bitext-ratio-column\')">scores</span>');

    echo('<span class="option-menu" id="score-option">');
    if ($showScores){
        $query = make_query(['showScores' => 0]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">align&nbsp;scores</a>');
    }
    else{
        $query = make_query(['showScores' => 1]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'"><strike>align&nbsp;scores</strike></a>');
    }
    if ($showLengthRatio){
        $query = make_query(['showLengthRatio' => 0]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">length&nbsp;ratio</a>');
    }
    else{
        $query = make_query(['showLengthRatio' => 1]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'"><strike>length&nbsp;ratio</strike></a>');
    }
    echo('</span>');
}


function rating_options_menu(){
    global $showRatings,$showMyRatings;

    echo('<span class="option-menu" id="rating-option">');
    if ($showRatings){
        echo(' / average&nbsp;rating');
    }
    else{
        $query = make_query(['showMyRatings' => 0, 'showRatings' => 1]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">average&nbsp;rating</a>');
    }

    if ($showMyRatings){
        echo(' / my&nbsp;rating');
    }
    else {
        $query = make_query(['showMyRatings' => 1, 'showRatings' => 0]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">my&nbsp;rating</a>');
    }

    if ($showMyRatings || $showRatings){
        $query = make_query(['showMyRatings' => 0, 'showRatings' => 0]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">hide&nbsp;rating</a>');
    }
    else{
        echo(' / hide&nbsp;rating');
    }
    echo('</span>');
}


function aligntype_options_menu(){
    global $alignType, $ALIGN_TYPES, $allowOtherAlignTypes, $allowRandomLinks;
    global $searchquery, $tableStyle, $showModified, $allowEdit, $showEmpty;

    if ($searchquery) return;
    if ($tableStyle == 'edit') return;

    echo('<span class="option-menu" id="aligntype-option">');
    if (! $alignType){
        echo(' / all');
    }
    else{
        $query = make_query(['aligntype' => '', 'offset' => 0]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">all</a>');
    }
    foreach ($ALIGN_TYPES as $type){
        if ($type == 'random' && ! $allowRandomLinks) continue;
        if ($alignType != $type){
            $query = make_query(['aligntype' => $type, 'offset' => 0]);
            echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">'.$type.'</a>');
        }
        else echo(' / '.$type);
    }
    if ($allowOtherAlignTypes){
        if ($alignType != 'other'){
            $query = make_query(['aligntype' => 'other', 'offset' => 0]);
            echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">other</a>');
        }
        else echo(' / other');
    }
    /*
    if ($showEmpty){
        $query = make_query(['showEmpty' => 0]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">hide empty</a>');
    }
    else{
        $query = make_query(['showEmpty' => 1]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">show empty</a>');
    }
    */
    echo('</span>');
}

function other_options_menu(){
    global $alignType, $allowRandomLinks, $showEmpty;
    global $searchquery, $tableStyle;

    if ($searchquery) return;
    if ($tableStyle == 'edit') return;

    $EXTRA_TYPES = array('random','rated');

    echo('<span class="option-menu" id="other-option">');
    if (! $alignType){
        echo(' / sorted');
    }
    else{
        $query = make_query(['aligntype' => '', 'offset' => 0]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">sorted</a>');
    }
    foreach ($EXTRA_TYPES as $type){
        if ($type == 'random' && ! $allowRandomLinks) continue;
        if ($alignType != $type){
            $query = make_query(['aligntype' => $type, 'offset' => 0]);
            echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">'.$type.'</a>');
        }
        else echo(' / '.$type);
    }
    if ($showEmpty){
        $query = make_query(['showEmpty' => 0]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">hide empty</a>');
    }
    else{
        $query = make_query(['showEmpty' => 1]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">show empty</a>');
    }
    echo('</span>');
}


function bitext_display_options(){
    global $tableStyle;

    if ($tableStyle == 'vertical'){
        $query = make_query(['style' => 'horizontal']);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">horizontal</a>');
    }
    else{
        $query = make_query(['style' => 'vertical']);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">vertical</a>');
    }
}

function bitext_search_options(){
    global $showSearch;
    echo(' / <a class="clickable" id="search-form" onclick="toggleSearchForm();">search</a>');
    /*
    if ($showSearch){
        $query = make_query(['showSearch' => 0]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">search</a>');
    }
    else{
        $query = make_query(['showSearch' => 1]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'"><strike>search</strike></a>');
    }
    */
}

function bitext_edit_links($bitext){
    global $tableStyle, $showModified, $allowEdit;

    if ($tableStyle == 'edit' && $allowEdit){
        $query = make_query(['style' => 'horizontal', 'showModified' => 1]);
        echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">stop editing</a>');
    }
    else{
        if ($bitext->userDataExists()){
            if ($showModified){
                $query = make_query(['showModified' => 0]);
                echo(' / modified / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">original</a>');
            }
            else{
                $query = make_query(['showModified' => 1]);
                echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">modified</a> / original');
            }
        }
        if ($allowEdit){
            $query = make_query(['style' => 'edit']);
            echo(' / <a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">edit</a>');
        }
    }
    if ($allowEdit){
        echo('</span>');
    }
}

    
?>
