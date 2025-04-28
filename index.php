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
  <link rel="stylesheet" href="index.css?v54" type="text/css">
  <script type="text/javascript">
	function setStyle(obj,style,value){
		obj.style[style] = value;
	}
    function toggleElement(id) {
        var x = document.getElementById(id);
        if (x.style.display === "inline") {
            x.style.display = "none";
        } else {
            x.style.display = "inline";
        }
    }
    function toggleElementClass(name) {
        var x = document.getElementsByClassName(name);
        for (var i = 0; i < x.length; i++){
            const display = x[i].style.display;
            if (display === "none") {
                x[i].style.display = "inline";
                localStorage.setItem(name, 'class-inline');
            } else {                
                x[i].style.display = "none";
                localStorage.setItem(name, 'class-none');
            }
        }
    }
    function toggleClassVisibility(name) {
        var x = document.getElementsByClassName(name);
        for (var i = 0; i < x.length; i++){
            if (x[i].style.visibility === "collapse") {
                x[i].style.visibility = "visible";
                localStorage.setItem(name, 'class-visible');
            } else {
                x[i].style.visibility = "collapse";
                localStorage.setItem(name, 'class-collapse');
            }
        }
    }
    function toggleVisibility(id) {
        var x = document.getElementById(id);
        if (x.style.visibility === "collapse") {
            x.style.visibility = "visible";
            localStorage.setItem(id, 'visible');
        } else {
            x.style.visibility = "collapse";
            localStorage.setItem(id, 'collapse');
        }        
    }

    function hideSearchForm(){
        localStorage.setItem('showSearch', 0);
        var x = document.getElementsByClassName('search-form');
        for (var i = 0; i < x.length; i++){
            x[i].style.display = "none";
        }
    }
    function showSearchForm(){
        localStorage.setItem('showSearch', 1);
        var x = document.getElementsByClassName('search-form');
        for (var i = 0; i < x.length; i++){
            x[i].style.display = "inline";
        }
    }

    function toggleSearchForm(){
        const show = localStorage.getItem('showSearchForm');
        const x = document.getElementsByClassName('search-form');
        if (show){
            localStorage.setItem('showSearchForm',0);
            displaySearchForm(0);
        } else {
            localStorage.setItem('showSearchForm',1);
            displaySearchForm(1);
        }                                 
    }

    function displaySearchForm(show){
        const x = document.getElementsByClassName('search-form');
        const style = show ? 'inline' : 'none';
        for (var i = 0; i < x.length; i++){
            x[i].style.display = style;
        }
    }

    document.addEventListener("DOMContentLoaded", function(event) {
        var scrollpos = localStorage.getItem('scrollpos');
        if (scrollpos) window.scrollTo(0, scrollpos);
        localStorage.setItem('scrollpos', 0);
        // localStorage.clear();
        // displaySearchForm(localStorage.getItem('showSearchForm'));
        for (var i = 0; i < localStorage.length; i++){
            var key = localStorage.key(i);
            if (localStorage.getItem(key) === "hidden"){
                var x = document.getElementById(key);
                x.style.visibility = "hidden";
            }
            else if (localStorage.getItem(key) === "collapse"){
                var x = document.getElementById(key);
                x.style.visibility = "collapse";
            }
            else if (localStorage.getItem(key) === "visible"){
                var x = document.getElementById(key);
                x.style.visibility = "visible";
            }
            /*
            else if (localStorage.getItem(key) === "inline"){
                var x = document.getElementById(key);
                x.style.display = "visible";
            }
            else if (localStorage.getItem(key) === "none"){
                var x = document.getElementById(key);
                x.style.display = "none";
            }
            else if (localStorage.getItem(key) === "class-none"){
                var x = document.getElementsByClassName(key);
                for (var i = 0; i < x.length; i++){
                    x[i].style.display = "none";
                }
            }
            else if (localStorage.getItem(key) === "class-inline"){
                var x = document.getElementsByClassName(key);
                for (var i = 0; i < x.length; i++){
                    x[i].style.display = "inline";                    
                }                
            }
            else if (localStorage.getItem(key) === "class-visible"){
                var x = document.getElementsByClassName(key);
                for (var i = 0; i < x.length; i++){
                    x[i].style.visibility = "visible";
                }
            }
            else if (localStorage.getItem(key) === "class-hidden"){
                var x = document.getElementsByClassName(key);
                for (var i = 0; i < x.length; i++){
                    x[i].style.visibility = "hidden";
                }
            }
            */
        }
    });
    window.onbeforeunload = function(e) {
        localStorage.setItem('scrollpos', window.scrollY);
    };
    function saveScrollPosition() {
      localStorage.setItem('scrollpos', window.scrollY);
      location.reload(); 
    }
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

$showSearch = get_param('showSearch',0);
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

// $allowEdit = 1;
$allowEdit            = $user != 'guest' ? in_array($corpus, $ALLOW_EDIT) : 0;
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
            // echo(' / <a class="clickable" onclick="toggleElementClass(\'search-form\');">search</a>');
            echo('</div>');
            echo('</br><hr>');
            print_document_list($bitext, $offset, $fromDocQuery, $toDocQuery);
        }
    }
    else{
        echo('<div class="rightalign">');
        bitext_search_options();
        // echo(' / <a class="clickable" onclick="toggleElementClass(\'search-form\');">search</a>');
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
