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
  <style>
       div.rightalign  {text-align: right; float: right; display: inline;}

table.bitext   {width: 100%;}
td.bitext-src  {width: 40%;}
td.bitext-trg  {width: 40%;}

table.segment  {width: 100%; border: none;}
td.segment-id  {width: 10%; border: none;}
td.segment-move {width: 10px; text-align: center; border: none; background-color: white;}
td.segment-src {text-align: right; border: none;}
td.segment-trg {text-align: left; border: none;}

td.leftalign   {text-align: left;width: 40%;padding-left: 10px;}
td.rightalign  {text-align: right;width: 40%;padding-right: 10px;}
td.centeralign  {text-align: center;}
table, th {
  border: 2px solid;
  border-collapse: collapse;
}
td {
  border: 1px dotted;
  padding-left: 5px;
  padding-right: 5px;
}
tr.bitextsrc   {
    border-top: 2px solid;
    background-color: #eeeeee;
}
tr.bitexttrg   { border-bottom: 1px solid solid;}
  </style>

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
include('bitexts.inc');
include('ratings.inc');
include('index.inc');
include('search.inc');


echo('<h1>OpusExplorer</h1>');

if (!logged_in()){
	exit;
}

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

$searchquery = get_param('search','');


## check whether we have a new rating to take care of

$linkID = get_param('linkID',0);
$rating = get_param('rating',0);

if ($srclang && $trglang){
    $srcDbFile      = get_lang_dbfile($srclang);
    $trgDbFile      = get_lang_dbfile($trglang);
    $srcIdxDbFile   = get_langidx_dbfile($srclang);
    $trgIdxDbFile   = get_langidx_dbfile($trglang);
    $bitextDbFile   = get_bitext_dbfile($langpair);
    $linkDbFile     = get_link_dbfile($langpair,$corpus,$version,$fromDoc,$toDoc);
    $algDbFile      = get_alignment_dbfile($langpair);
    $algStarsDbFile = get_ratings_dbfile($langpair);

    if ($srcDbFile)    $srcDBH    = new SQLite3($srcDbFile,SQLITE3_OPEN_READONLY);
    if ($trgDbFile)    $trgDBH    = new SQLite3($trgDbFile,SQLITE3_OPEN_READONLY);
    if ($srcIdxDbFile) $srcIdxDBH = new SQLite3($srcIdxDbFile,SQLITE3_OPEN_READONLY);
    if ($trgIdxDbFile) $trgIdxDBH = new SQLite3($trgIdxDbFile,SQLITE3_OPEN_READONLY);
    if ($algDbFile)    $algDBH    = new SQLite3($algDbFile,SQLITE3_OPEN_READONLY);
    if ($bitextDbFile) $bitextDBH = new SQLite3($bitextDbFile,SQLITE3_OPEN_READONLY);

    $browsable = ($srcIdxDbFile && $trgIdxDbFile && $algDbFile);
    $searchable = ($linkDbFile != '');
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


$query = make_query(['srclang' => '', 'trglang' => '', 'langpair' => '', 'search' => '',
                     'corpus' => '', 'fromDoc' => '', 'toDoc' => '', 'aligntype' => '', 'offset' => 0]);
echo('<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">OPUS</a> / ');

if ($srclang && $trglang){
    $query = make_query(['corpus' => '', 'fromDoc' => '', 'toDoc' => '', 'search' => '',
                         'aligntype' => '', 'offset' => 0]);
    echo('<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">'.$langpair.'</a> / ');
    
    if ($corpus && $version){
        $query = make_query(['fromDoc' => '', 'toDoc' => '', 'aligntype' => '', 'search' => '', 'offset' => 0]);
        echo('<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">'.$corpus.' / '.$version.'</a> / ');
        
        if ($fromDoc && $toDoc){
            if (!$bitextID) $bitextID = get_bitextid($corpus, $version, $fromDoc, $toDoc);
            set_param('bitextID',$bitextID);
            set_link_db($bitextID);
            $query = make_query(['aligntype' => '', 'offset' => 0, 'search' => '']);
            echo('<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">'.$fromDoc.'</a> / ');
            if ($browsable && ! $searchquery) bitext_browsing_links();
        }
    }
    if (! $fromDoc || ! $toDoc){
        $bitextID = 0;
        set_param('bitextID',$bitextID);
    }
    if ( ($searchable && ! $bitextID) || ($searchable && $searchquery) || ($searchable && ! $browsable) ){
        print_search_form($langpair, $corpus, $version, $fromDoc, $toDoc, $bitextID, $searchquery);
    }
}

// echo('<div class="rightalign"><a href="https://docs.google.com/presentation/d/1J6rPo08FOW9l0UbDjrHgTBFUggdT9LVaGENYH0t2a18/edit?usp=sharing">[help]</a><a href="index.php?logout">[logout]</a></div>');
echo('<div class="rightalign"><a href="help.php">[help]</a><a href="index.php?logout">[logout]</a></div>');

echo('</br><hr>');

/////////////////////////////////////////////////////////////////
// content
/////////////////////////////////////////////////////////////////


if ($searchable){
    if ($searchquery){
        $searchlimit = get_param('limit',10);
        $searchoffset = get_param('offset',0);
        $searchside = get_param('action','search source');
        // $searchside = $_GET['action'];
        search($searchquery, $searchside,
               $langpair, $corpus, $version, $bitextID,
               $searchlimit, $searchoffset);
        exit();
    }
}
if ($srclang && $trglang){    
    if ($corpus && $version){
        if ($fromDoc && $toDoc && $browsable){
            //print_links($corpus, $version, $fromDoc, $toDoc);
            print_bitext($corpus, $version, $fromDoc, $toDoc,
                         $fromDocID, $toDocID, $bitextID,
                         $alignType, $offset);
        }
        else{
            print_document_list($corpus, $version, $offset);
        }
    }
    else{
        print_corpus_list();
    }
}
else{
    print_langpair_list();
}


/////////////////////////////////////////////////////////////////
// functions
/////////////////////////////////////////////////////////////////





function print_document_list($corpus, $version, $offset=0){
    global $bitextDBH;
    global $showMaxDocuments;
    
    $condition = "WHERE corpus='$corpus' AND version='$version'";
    
    $limit = "LIMIT $showMaxDocuments";
    if ($offset){
        $limit .= " OFFSET $offset";
    }
    $results = $bitextDBH->query("SELECT DISTINCT fromDoc,toDoc,rowid FROM bitexts $condition $limit");
    
    if ($results){
        if ($offset){
            $start = $offset - $showMaxDocuments;
            if ($start < 0) $start = 0;
            $query = make_query(['offset' => $start]);
            echo '<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">previous documents</a>'."\n";
        }
        else{
            echo 'select a source document:'."\n";
        }
        echo('<ul>');
        $count = 0;
        while ($row = $results->fetchArray(SQLITE3_NUM)) {
            $count++;
            $fromDocID = get_source_document_id($corpus, $version, $row[0]);
            $toDocID = get_target_document_id($corpus, $version, $row[1]);
            $query = make_query(['fromDoc' => $row[0],
                                 'toDoc' => $row[1],
                                 'bitextID' => $row[2],
                                 'fromDocID' => $fromDocID,
                                 'toDocID' => $toDocID,
                                 'offset' => 0]);
            echo '<li><a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">'.$row[0].'</a></li>'."\n";
            // echo '<li><a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">'.$row[0].' - '.$row[1].'</a></li>'."\n";
            // echo($row[0]." - ".$row[1]."\n");
        }
        echo('</ul>');
        if ($count >= $showMaxDocuments){
            $query = make_query(['offset' => $offset + $count]);
            echo '<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">more documents</a>'."\n";
        }
    }
}

function print_corpus_list(){
    global $bitextDBH;
    echo('<ul>');
    $results = $bitextDBH->query("SELECT DISTINCT corpus,version FROM aligned_corpora ORDER BY corpus");
    // $results = $bitextDBH->query("SELECT DISTINCT corpus,version FROM bitexts ORDER BY corpus");
    if ($results){
        $corpus = '';
        $versions = array();
        $link = '';
        $versionLinks = '';
        while ($row = $results->fetchArray(SQLITE3_NUM)) {
            if ($row[0] != $corpus){
                // if ($corpus) echo "<li>$link$corpus</a>: $versionLinks</li>\n";
                // if ($corpus) echo "<li>$corpus: [${link}latest</a>] $versionLinks</li>\n";
                if ($corpus) echo "<li>$corpus: $versionLinks</li>\n";
                $corpus = $row[0];
                $versionLinks = '';
            }
            array_push($versions,$row[1]);
            $version = $row[1];
            $query = make_query(['corpus' => $corpus, 'version' => $version, 'search' => '']);
            $link = '<a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">';
            $versionLinks .= ' ['.$link.$version.'</a>]';
        }
        // if ($corpus) echo "<li>$link$corpus</a>: $versionLinks</li>\n";
        // if ($corpus) echo "<li>$corpus: [${link}latest</a>] $versionLinks</li>\n";
        if ($corpus) echo "<li>$corpus: $versionLinks</li>\n";
    }
    echo('</ul>');
}

function print_langpair_list(){
    global $DB_DIR;
    
    // $langpairs = array('fi-uk');
    $langpairs = find_bitext_dbs($DB_DIR);
    ksort($langpairs);

    echo('<ul>');
    foreach ($langpairs as $langpair => $val){
        list($srclang,$trglang) = explode('-',$langpair);
        $srclangname = Locale::getDisplayName($srclang, 'en');
        $trglangname = Locale::getDisplayName($trglang, 'en');
        $query = make_query(['langpair' => $langpair, 'search' => '']);
        echo '<li><a href="'.$_SERVER['PHP_SELF'].'?'.SID.'&'.$query.'">';
        echo $langpair.' ('.$srclangname.' - '.$trglangname;
        echo ')</a></li>'."\n";
    }
    echo('</ul>');
    /*
    if (!class_exists('Locale')) {
        echo ('No php_intl extension installed.');
    }
    */
}


function find_bitext_dbs($path){
    
    $langpairs = array();    
    if ($handle = opendir($path)) {
        while (false !== ($entry = readdir($handle))) {
            if (substr($entry,-3) == '.db') {
                if (substr($entry,-10) == '.linked.db') {
                    $lang = basename($entry,'.linked.db');
                    $parts = explode('-',$lang);
                    if (count($parts) == 2){
                        $langpairs[$lang] = true;
                    }
                }
                elseif (substr($entry,-9) != '.stars.db') {
                    $lang = basename($entry,'.db');
                    $parts = explode('-',$lang);
                    if (count($parts) == 2){
                        $langpairs[$lang] = true;
                    }
                }
            }
        }
        closedir($handle);
    }
    return $langpairs;
}

?>
