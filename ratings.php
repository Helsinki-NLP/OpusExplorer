<?php

// print alignments with ratings and stars
// set parameters with query arguments or the regular index-page


session_start();

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

header("Content-Type: text/plain");

list($srclang, $trglang, $langpair) = get_langpair();
$opusLangpair = get_param('opusLangpair');

$user = $_SESSION['user'];
$corpus = get_param('corpus');
$version = get_param('version');
$fromDoc = get_param('fromDoc');
$toDoc = get_param('toDoc');

$bitext = new bitext($DB_DIR, $user, $corpus, $version,
                     $langpair, $fromDoc, $toDoc, $opusLangpair);


// ratings

$alignments = $bitext->alignments('ratings',0,$offset);
print_bitext_tsv($alignments,true, true, false, true, true);

echo("\n");


// stars

$alignments = $bitext->alignments('stars',0,$offset);
print_bitext_tsv($alignments,true, true, false, true, true);

?>
