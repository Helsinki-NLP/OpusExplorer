<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
  <title>OPUS Explorer</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
       div.rightalign  {text-align: right; float: right; display: inline;}
table.bitext   {width: 100%;}
td.leftalign   {text-align: left;width: 40%;padding-left: 10px;}
td.rightalign  {text-align: right;width: 40%;padding-right: 10px;}
td.centeralign  {text-align: center;}
table, th {
  border: 1px solid;
  border-collapse: collapse;
}
td {
  border: 1px dotted;
}
  </style>
</head>
<body>
<h1>OpusExplorer - Usage information</h1>

<?php

$pages = array(
    'get started' => 'OpusExplorer-login.svg',
    'browsing resource list' => 'OpusExplorer-resourcelist.svg',
    'bitext navigation 1' => 'OpusExplorer-bitexts1.svg',
    'bitext navigation 2' => 'OpusExplorer-bitexts2.svg',
    'vertical alignment' => 'OpusExplorer-vertical.svg',
    'user ratings' => 'OpusExplorer-rating.svg',
    'editing alignments' => 'OpusExplorer-editmode.svg'
);

$page = 'get started';
if (isset($_GET['page'])){
    $page = $_GET['page'];
}

echo("[<a href='index.php'>back to explorer</a>]");
foreach ($pages as $text => $image){
    if ($page == $text)
        echo("[$text]\n");
    else
        echo("[<a href='help.php?page=$text'>$text</a>]\n");
}

echo('<hr/>');
$image = $pages[$page];
echo("<img src='$image' height='80%'/>");

?>

</body>
</html>
