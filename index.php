<?php
// session_reset();
session_start();

if (isset($_GET['logout'])){
    unset($_SESSION['user']);
}

?>
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
  <title>OPUS Explorer</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta name="viewport" content="width=device-width, initial-scale=1"> 
  <link rel="stylesheet" href="index.css" type="text/css">
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
// include('db.inc');
include('bitexts.inc');
include('ratings.inc');
include('search.inc');


echo('<h1>OpusExplorer</h1>');

if (!logged_in()){
	exit;
}
echo('<div class="rightalign"><a href="help.php">[help]</a><a href="index.php?logout">[logout]</a></div>');

list($srclang, $trglang, $langpair) = get_langpair();

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
$showEmpty = get_param('showEmpty',0);
if ($alignType == '0-1' || $alignType == '1-0') $showEmpty=1;

$showScores = get_param('showScores',1);
$showLengthRatio = get_param('showLengthRatio',0);
$showRatings = get_param('showRatings',0);
$showMyRatings = get_param('showMyRatings',1);
$showModified = get_param('showModified',1);

$showMaxAlignments = get_param('showMaxAlignments',$SHOW_ALIGNMENTS_LIMIT);
$showMaxDocuments = get_param('showMaxDocuments',$DOCUMENT_LIST_LIMIT);

$tableStyle = get_param('style','horizontal');
if ($tableStyle == 'edit') $showModified=1;


## special permissions for document-level corpora (sentences in context):
## - allow to edit alignments
## - allow to sort by link ID
## - allow to search for other alignment types (expensive search for large doc's!)

$allowEdit = in_array($corpus, $ALLOW_EDIT);
$allowOtherAlignTypes = in_array($corpus, $ALLOW_EDIT);
$allowSortLinks = in_array($corpus, $ALLOW_EDIT);


$modifiedBitextExists = false;
$modifiedBitext = false;


$orderByLinkID = get_param('sortLinkIDs',0);
$searchquery = filter_var(get_param('search',''),FILTER_SANITIZE_STRING);
$fromDocQuery = filter_var(get_param('fromDocQuery',''),FILTER_SANITIZE_STRING);

## check whether we have a new rating to take care of

$linkID = get_param('linkID',0);
$rating = get_param('rating',0);

/*
if ($srclang && $trglang){
    $srcDbFile      = get_lang_dbfile($srclang);
    $trgDbFile      = get_lang_dbfile($trglang);
    $srcIdxDbFile   = get_langidx_dbfile($srclang);
    $trgIdxDbFile   = get_langidx_dbfile($trglang);
    
    $bitextDbFile   = get_bitext_dbfile($langpair);
    $linkDbFile     = get_link_dbfile($langpair,$corpus,$version,$fromDoc,$toDoc);
    $algStarsDbFile = get_ratings_dbfile($langpair);

    if ($srcDbFile)    $srcDBH    = new SQLite3($srcDbFile,SQLITE3_OPEN_READONLY);
    if ($trgDbFile)    $trgDBH    = new SQLite3($trgDbFile,SQLITE3_OPEN_READONLY);
    if ($srcIdxDbFile) $srcIdxDBH = new SQLite3($srcIdxDbFile,SQLITE3_OPEN_READONLY);
    if ($trgIdxDbFile) $trgIdxDBH = new SQLite3($trgIdxDbFile,SQLITE3_OPEN_READONLY);

    if ($bitextDbFile) $bitextDBH = new SQLite3($bitextDbFile,SQLITE3_OPEN_READONLY);
    if ($linkDbFile)   $linksDBH  = new SQLite3($linkDbFile,SQLITE3_OPEN_READONLY);

    $browsable = 1;
    $searchable = 1;

}
*/


$browsable = 1;
$searchable = 1;


if ($srclang && $trglang){
    $bitext = new bitext($DB_DIR, $user, $corpus, $version, $langpair, $fromDoc, $toDoc, $showModified);    
    if ($rating){
        $bitext->addAlignmentRating($bitextID,$linkID,$rating);
        delete_param('linkID');
        delete_param('rating');
    }
}


/////////////////////////////////////////////////////////////////
// menu
/////////////////////////////////////////////////////////////////


$query = make_query(['srclang' => '', 'trglang' => '', 'langpair' => '',
                     'corpus' => '', 'fromDoc' => '', 'toDoc' => '',
                     'aligntype' => '', 'offset' => 0, 'sortLinkIDs' => 0,
                     'search' => '', 'fromDocQuery' => '']);
echo('<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">OPUS</a> / ');

if ($srclang && $trglang){
    $query = make_query(['corpus' => '', 'fromDoc' => '', 'toDoc' => '', 
                         'aligntype' => '', 'offset' => 0, 'sortLinkIDs' => 0,
                         'search' => '', 'fromDocQuery' => '']);
    echo('<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">'.$langpair.'</a> / ');
    
    if ($corpus && $version){
        $query = make_query(['fromDoc' => '',
                             'toDoc' => '',
                             'search' => '',
                             'fromDocQuery' => '']);

        echo('<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">'.$corpus.' / '.$version.'</a> / ');
        
        if ($fromDoc && $toDoc){
            $query = make_query(['aligntype' => '',
                                 'offset' => 0,
                                 'search' => '',
                                 'sortLinkIDs' => 0,
                                 'fromDocQuery' => '']);
            echo('<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">'.$fromDoc.'</a> / ');
            if ($browsable && ! $searchquery) bitext_browsing_links($bitext, $alignType);
        }
    }
    if (! $fromDoc || ! $toDoc){
        $bitextID = 0;
        set_param('bitextID',$bitextID);
        print_search_form($langpair, $corpus, $version, $fromDoc, $toDoc, $bitextID, $searchquery);
    }
}

echo('</br><hr>');

/////////////////////////////////////////////////////////////////
// content
/////////////////////////////////////////////////////////////////


if ($searchquery && $searchable){
    $searchlimit = get_param('limit',10);
    $searchoffset = get_param('offset',0);
    $searchside = get_param('action','search source');
    search($searchquery, $searchside, $bitext, $searchlimit, $searchoffset);
}
elseif ($srclang && $trglang){    
    if ($corpus && $version){
        if ($fromDoc && $toDoc){
            print_bitext($bitext, $alignType, $offset);
        }
        else print_document_list($bitext, $offset, $fromDocQuery);
    }
    else print_corpus_list($bitext);
}
else print_langpair_list();

?>
