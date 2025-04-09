<?php
// session_reset();
session_start();

if (isset($_GET['logout'])){
    unset($_SESSION['user']);
}


# add those lines to experiment with different css styles
#
#  <meta http-equiv="Cache-Control" content="no-cache, no-store, must-revalidate" />
#  <meta http-equiv="Pragma" content="no-cache" />
#  <meta http-equiv="Expires" content="0" />


?>
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
  <title>OPUS Explorer</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="index.css?v17" type="text/css">
  <script type="text/javascript">
	function setStyle(obj,style,value){
		obj.style[style] = value;
	}
  </script>
</head>
<body>
<?php

include('env.inc');
include('users.inc');
include('opus.inc');
include('opusindex.inc');
include('bitexts.inc');
include('ratings.inc');
include('search.inc');




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
$showRatings = get_param('showRatings',0);
$showMyRatings = get_param('showMyRatings',1);
$showModified = get_param('showModified',1);
// $showLatest = get_param('showLatest',0);

$showMaxAlignments = get_param('showMaxAlignments',$SHOW_ALIGNMENTS_LIMIT);
$showMaxDocuments = get_param('showMaxDocuments',$DOCUMENT_LIST_LIMIT);

// $tableStyle = get_param('style','horizontal');
$tableStyle = get_param('style','vertical');
if ($tableStyle == 'edit') $showModified=1;

// $resourceView = get_param('resourceView','langList');
$resourceView = get_param('resourceView','langMatrix');


## special permissions for document-level corpora (sentences in context):
## - allow to edit alignments
## - allow to sort by link ID
## - allow to search for other alignment types (expensive search for large doc's!)

// $allowEdit = in_array($corpus, $ALLOW_EDIT);
$allowEdit = 0;
$allowOtherAlignTypes = in_array($corpus, $ALLOW_EDIT);
$allowSortLinks = in_array($corpus, $ALLOW_EDIT);

$orderByLinkID = get_param('sortLinkIDs',0);
$searchquery = filter_var(get_param('search',''),FILTER_SANITIZE_STRING);
$fromDocQuery = filter_var(get_param('fromDocQuery',''),FILTER_SANITIZE_STRING);
$toDocQuery = filter_var(get_param('toDocQuery',''),FILTER_SANITIZE_STRING);


/////////////////////////////////////////////////////////////////
// create the bitext object and handle ratings
/////////////////////////////////////////////////////////////////

$bitext = new bitext($DB_DIR, $user, $corpus, $version, $langpair, $fromDoc, $toDoc, $opusLangpair,
                     $showModified, $showLatest);
// $version = $bitext->version;


if ($srclang && $trglang){

    $opusLangpair = $bitext->opusLangpair;

    ## check whether we have a new rating to take care of
    
    $linkID = get_param('linkID',0);
    $rating = get_param('rating',0);
    
    if ($rating){
        $bitext->addAlignmentRating($bitextID,$linkID,$rating);
        delete_param('linkID');
        delete_param('rating');
    }
}

/////////////////////////////////////////////////////////////////
// content
/////////////////////////////////////////////////////////////////

print_bitext_menu($corpus,$version,$srclang,$trglang,$langpair,$fromDoc,$toDoc, $searchquery, $alignType);
// echo('</br><hr>');

if ($searchquery){
    $searchlimit = get_param('limit',10);
    $searchoffset = get_param('offset',0);
    $searchside = get_param('action','search source');
    search($searchquery, $searchside, $bitext, $searchlimit, $searchoffset);
}
elseif ($srclang && $trglang){
    if ($corpus && $version){
        if ($fromDoc && $toDoc) print_bitext($bitext, $alignType, $offset);
        else{
            echo('</br><hr>');
            print_document_list($bitext, $offset, $fromDocQuery, $toDocQuery);
        }
    }
    else{
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
