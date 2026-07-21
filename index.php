<?php
// session_reset();
session_start();

if (isset($_GET['logout'])){
    unset($_SESSION['user']);
}

/*
# add those lines to experiment with different css styles
#
#  <meta http-equiv="Cache-Control" content="no-cache, no-store, must-revalidate" />
#  <meta http-equiv="Pragma" content="no-cache" />
#  <meta http-equiv="Expires" content="0" />
*/

// keep scroll position on reload: see
// https://stackoverflow.com/questions/17642872/refresh-page-and-keep-scroll-position

?>
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
  <title>OPUS Explorer</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="index.css?v62" type="text/css">
  <script type="text/javascript">
	function setStyle(obj,style,value){
		obj.style[style] = value;
	}
    function toggleOptions(id) {
        const selected = localStorage.getItem('option-menu');
        if (selected){
            const x = document.getElementById(selected);
            if (x) x.style.display = 'none';
            const m = document.getElementById(selected + '-selector');
            if (m) m.style.color = 'blue';
        }
        if (selected == id){
            localStorage.removeItem('option-menu');
        } else {
            const x = document.getElementById(id);
            if (x){
                x.style.display = 'inline';
                localStorage.setItem('option-menu',id);
                const m = document.getElementById(id + '-selector');
                if (m) m.style.color = 'black';
            }
        }
    }

    function displaySearchForm(style){
        if (style){
            const x = document.getElementsByClassName('search-form');
            for (var i = 0; i < x.length; i++){
                x[i].style.display = style;
            }
        }
    }
    function toggleSearchForm(){
        const style = localStorage.getItem('displaySearchForm');
        var newstyle = style === 'none' ? 'inline' : 'none';
        localStorage.setItem('displaySearchForm',newstyle);
        displaySearchForm(newstyle);
    }

    function displayScore(scoreType,style){
        if (style){
            const x = document.getElementsByClassName('bitext-' + scoreType);
            for (var i = 0; i < x.length; i++){
                x[i].style.display = style;
            }
            const m = document.getElementById(scoreType + '-selector');
            if (m){
                m.style.color = style === 'inline' ? 'black' : 'blue';
            }
        }
    }
    function toggleScore(scoreType){
        const scoreTypeKey = 'displayScore-' + scoreType;
        const style = localStorage.getItem(scoreTypeKey);
        var newstyle = style === 'none' ? 'inline' : 'none';
        localStorage.setItem(scoreTypeKey,newstyle);
        displayScore(scoreType,newstyle);
    }

    function saveScrollPosition() {
      localStorage.setItem('scrollpos', window.scrollY);
      location.reload(); 
    }
    document.addEventListener("DOMContentLoaded", function(event) {
        var scrollpos = localStorage.getItem('scrollpos');
        if (scrollpos) window.scrollTo(0, scrollpos);
        localStorage.setItem('scrollpos', 0);
        displaySearchForm(localStorage.getItem('displaySearchForm'));
        displayScore('alignscore',localStorage.getItem('displayScore-alignscore'));
        displayScore('lengthratio',localStorage.getItem('displayScore-lengthratio'));
        const selected = localStorage.getItem('option-menu');
        if (selected){
            const x = document.getElementById(selected);
            if (x){
                x.style.display = 'inline';
                const m = document.getElementById(selected + '-selector');
                if (m) m.style.color = 'black';
            }
        }
    });
    window.onbeforeunload = function(e) {
        localStorage.setItem('scrollpos', window.scrollY);
    };
  </script>
</head>
<body>
<?php

include('inc/env.inc');
include('inc/users.inc');
include('inc/opus.inc');
include('inc/opusindex.inc');
include('inc/bitexts.inc');
include('inc/ratings.inc');
include('inc/search.inc');
include('inc/options.inc');




if (!logged_in()){
	exit;
}
echo('<div class="rightalign"><a href="help.php">[help]</a><a href="index.php?logout">[logout]</a></div>');
echo('<h1>OpusExplorer</h1>');

list($srclang, $trglang, $langpair) = get_langpair();
$opusLangpair = get_param('opusLangpair');

$user = $_SESSION['user'];
$corpus = get_param('corpus');
$version = get_param('version');
$fromDoc = get_param('fromDoc');
$toDoc = get_param('toDoc');

$bitextID  = get_param('bitextID');
$fromDocID = get_param('fromDocID');
$toDocID   = get_param('toDocID');

$offset = get_param('offset',0);
$alignType = get_param('aligntype');
$showEmpty = get_param('showEmpty',1);
if ($alignType == '0-1' || $alignType == '1-0') $showEmpty=1;

$showScores = get_param('showScores',1);
$showLengthRatio = get_param('showLengthRatio',1);
$showRatings = get_param('showRatings',1);
$showMyRatings = get_param('showMyRatings',0);
$showModified = get_param('showModified',1);
// $showLatest = get_param('showLatest',0);
$showLatest = 0;

$showMaxAlignments = get_param('showMaxAlignments',$SHOW_ALIGNMENTS_LIMIT);
$showMaxDocuments = get_param('showMaxDocuments',$DOCUMENT_LIST_LIMIT);

// $tableStyle = get_param('style','horizontal');
$tableStyle = get_param('style','vertical');
if ($tableStyle == 'edit') $showModified=1;

// $resourceView = get_param('resourceView','langList');
$resourceView = get_param('resourceView','langMatrix');


/*
## special permissions for document-level corpora (sentences in context):
## - allow to edit alignments
## - allow to sort by link ID
## - allow to search for other alignment types (expensive search for large doc's!)
*/

$allowEdit            = $user != 'guest' ? in_array($corpus, $ALLOW_EDIT) : 0;
$allowEdit = 1;
$showAlignTypes       = get_param('showAlignTypes',$SHOW_ALIGN_TYPES);
$allowOtherAlignTypes = in_array($corpus, $ALLOW_EDIT);
$allowRandomLinks     = in_array($corpus, $ALLOW_EDIT);
$allowSortLinks       = in_array($corpus, $ALLOW_EDIT);

$orderByLinkID = get_param('sortLinkIDs',0);
$searchquery   = filter_var(get_param('search',''),FILTER_SANITIZE_STRING);
$fromDocQuery  = filter_var(get_param('fromDocQuery',''),FILTER_SANITIZE_STRING);
$toDocQuery    = filter_var(get_param('toDocQuery',''),FILTER_SANITIZE_STRING);


/////////////////////////////////////////////////////////////////
// create the bitext object and handle ratings
/////////////////////////////////////////////////////////////////

$bitext = new bitext($DB_DIR, $user, $corpus, $version,
                     $langpair, $fromDoc, $toDoc, $opusLangpair,
                     $showModified, $showLatest);
// $version = $bitext->version;


if ($srclang && $trglang){

    $opusLangpair = $bitext->opusLangpair;

    ## check whether we have a new rating to take care of
    
    $linkID = get_param('linkID',0);
    $rating = get_param('rating',0);
    
    if ($rating){
        $bitext->addAlignmentRating($bitextID,$linkID,$rating);
        // echo("add rating $bitextID,$linkID,$rating");
        delete_param('linkID');
        delete_param('rating');
    }
}

/////////////////////////////////////////////////////////////////
// content
/////////////////////////////////////////////////////////////////

print_bitext_menu($corpus,$version,$srclang,$trglang,$langpair,$fromDoc,$toDoc, $searchquery, $alignType);

if ($searchquery){
    $searchside = get_param('action','search source');
    search($searchquery, $searchside, $bitext, $showMaxAlignments, $offset);
}
elseif ($srclang && $trglang){
    if ($corpus && $version){
        if ($fromDoc && $toDoc) print_bitext($bitext, $alignType, $offset);
        else{
            echo('<div class="rightalign">');
            bitext_search_options();
            echo('</div>');
            echo('</br><hr>');
            print_document_list($bitext, $offset, $fromDocQuery, $toDocQuery);
        }
    }
    else{
        echo('<div class="rightalign">');
        bitext_search_options();
        echo('</div>');
        echo('</br><hr>');
        print_corpus_list($bitext);
    }
}
elseif ($resourceView == 'corpusList'){
    echo('</br><hr>');
    print_corpus_list($bitext);
}
else{
    echo('</br><hr>');
    print_langpairs($bitext, $resourceView);
}

?>
