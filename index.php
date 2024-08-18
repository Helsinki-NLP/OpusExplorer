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
include('bitexts.inc');
include('ratings.inc');
include('index.inc');
include('search.inc');


echo('<h1>OpusExplorer</h1>');

if (!logged_in()){
	exit;
}
echo('<div class="rightalign"><a href="help.php">[help]</a><a href="index.php?logout">[logout]</a></div>');

list($srclang, $trglang, $langpair) = get_langpair();

$corpus = get_param('corpus');
$version = get_param('version');
$fromDoc = get_param('fromDoc');
$toDoc = get_param('toDoc');
$alignType = get_param('aligntype');
$offset = get_param('offset',0);

$bitextID  = get_param('bitextID');
$fromDocID = get_param('fromDocID');
$toDocID   = get_param('toDocID');

$showEmpty = get_param('showEmpty',0);
$showScores = get_param('showScores',1);
$showLengthRatio = get_param('showLengthRatio',0);
$showRatings = get_param('showRatings',0);
$showMyRatings = get_param('showMyRatings',1);
$showModified = get_param('showModified',1);

$showMaxAlignments = get_param('showMaxAlignments',$SHOW_ALIGNMENTS_LIMIT);
$showMaxDocuments = get_param('showMaxDocuments',$DOCUMENT_LIST_LIMIT);

$tableStyle = get_param('style','horizontal');
$allowEdit = in_array($corpus, $ALLOW_EDIT);

$modifiedBitextExists = false;
$modifiedBitext = false;


$orderByLinkID = get_param('sortLinkIDs',0);
$searchquery = filter_var(get_param('search',''),FILTER_SANITIZE_STRING);
$fromDocQuery = filter_var(get_param('fromDocQuery',''),FILTER_SANITIZE_STRING);

## check whether we have a new rating to take care of

$linkID = get_param('linkID',0);
$rating = get_param('rating',0);

if ($srclang && $trglang){
    $srcDbFile      = get_lang_dbfile($srclang);
    $trgDbFile      = get_lang_dbfile($trglang);
    $srcIdxDbFile   = get_langidx_dbfile($srclang);
    $trgIdxDbFile   = get_langidx_dbfile($trglang);
    $bitextDbFile   = get_bitext_dbfile($langpair);
    $algDbFile      = get_alignment_dbfile($langpair);
    $algStarsDbFile = get_ratings_dbfile($langpair);

    $srcFtsFile     = get_lang_ftsfile($srclang);
    $trgFtsFile     = get_lang_ftsfile($trglang);
    $linkDbFile     = get_link_dbfile($langpair,$corpus,$version,$fromDoc,$toDoc);

    if ($srcDbFile)    $srcDBH    = new SQLite3($srcDbFile,SQLITE3_OPEN_READONLY);
    if ($trgDbFile)    $trgDBH    = new SQLite3($trgDbFile,SQLITE3_OPEN_READONLY);
    if ($srcIdxDbFile) $srcIdxDBH = new SQLite3($srcIdxDbFile,SQLITE3_OPEN_READONLY);
    if ($trgIdxDbFile) $trgIdxDBH = new SQLite3($trgIdxDbFile,SQLITE3_OPEN_READONLY);
    if ($algDbFile)    $algDBH    = new SQLite3($algDbFile,SQLITE3_OPEN_READONLY);
    if ($bitextDbFile) $bitextDBH = new SQLite3($bitextDbFile,SQLITE3_OPEN_READONLY);

    $browsable = ( $srcDbFile && $trgDbFile && $algDbFile && ( ($srcIdxDbFile && $trgIdxDbFile) || $linkDbFile ) );
    $searchable = ( $srcFtsFile && $trgFtsFile && $linkDbFile );
}

if ($rating){
    set_link_db($bitextID);
    add_alignment_rating($bitextID,$linkID,$_SESSION['user'],$rating);
    delete_param('linkID');
    delete_param('rating');
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
        $query = make_query(['fromDoc' => '', 'toDoc' => '', 'aligntype' => '',
                             'search' => '', 'offset' => 0, 'sortLinkIDs' => 0, 'fromDocQuery' => '']);
        echo('<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">'.$corpus.' / '.$version.'</a> / ');
        
        if ($fromDoc && $toDoc){
            if (!$bitextID) $bitextID = get_bitextid($corpus, $version, $fromDoc, $toDoc);
            set_param('bitextID',$bitextID);
            set_link_db($bitextID);
            $query = make_query(['aligntype' => '', 'offset' => 0, 'search' => '', 'sortLinkIDs' => 0, 'fromDocQuery' => '']);
            echo('<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">'.$fromDoc.'</a> / ');
            if ($browsable && ! $searchquery) bitext_browsing_links();
        }
    }
    if (! $fromDoc || ! $toDoc){
        $bitextID = 0;
        set_param('bitextID',$bitextID);
    }
    // if ( ($searchable && ! $bitextID) || ($searchable && $searchquery) || ($searchable && ! $browsable) ){
    if ($searchable && ! $bitextID){
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
    search($searchquery, $searchside,
           $langpair, $corpus, $version, $bitextID,
           $searchlimit, $searchoffset);
}
elseif ($srclang && $trglang){    
    if ($corpus && $version){
        if ($fromDoc && $toDoc){
            if ($browsable){
                print_bitext($corpus, $version, $fromDoc, $toDoc,
                             $fromDocID, $toDocID, $bitextID,
                             $alignType, $offset);
            }
            elseif ($searchable){
                search('', '',$langpair, $corpus, $version, $bitextID,
                       $showMaxAlignments, $offset, $orderByLinkID);
            }
            else{
                echo("<b>Something is missing - cannot show this bitext ($fromDoc - $toDoc)</b><br/><br/>");
                print_document_list($corpus, $version, $offset);
            }
        }
        else print_document_list($corpus, $version, $offset, $fromDocQuery);
    }
    else print_corpus_list();
}
else print_langpair_list();

?>
