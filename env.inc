<?php // -*-php-*-


// $DB_DIR = '/media/OPUS-MT/OpusIndex/';
$DB_DIR = '/media/OpusIndex/';

$USER_DATADIR    = $DB_DIR;
$USER_NAME_FILE  = $USER_DATADIR.'/users.php';
$USER_DB         = $USER_DATADIR.'/users.db';
$ALLOW_NEW_USERS = 1;


$DOCUMENT_LIST_LIMIT = 25;
$SHOW_ALIGNMENTS_LIMIT = 25;

$ALIGN_TYPES = array('1-1','1-2','2-1','1-0','0-1');

$default_srclang = 'fi';
$default_trglang = 'sv';



// edit mode: list corpora that can be edited

$ALLOW_EDIT = array(
    'DGT',
    'ELITR-ECA',
    'EMEA',
    'Europarl',
    'GlobalVoices',
    'GNOME',
    'JRC-Acquis',
    'KDE4',
    'KDEdoc',
    'NeuLab-TedTalks',
    'OpenOffice',
    'OpenSubtitles',
    'QED',
    'Tanzil',
    'TED2020',
    'Ubuntu'
);

$segmentFirstBgColor = '#cfc';
$segmentFirstBorderColor = '#0a0';

$segmentOtherBgColor = '#fcc';
$segmentOtherBorderColor = 'red';



function get_param($key, $default=''){

    // check the query string first and overwrite session variable
    if (isset($_GET[$key])){
        $_SESSION['params'][$key] = test_input($_GET[$key]);
        return $_SESSION['params'][$key];
    }
    
    if (array_key_exists('params', $_SESSION)){
        if (isset($_SESSION['params'][$key])){
            return $_SESSION['params'][$key];
        }
    }
    
    return $default;
}

function set_param($key, $value){
    $_SESSION['params'][$key] = $value;
}

function delete_param($key){
    if (array_key_exists('params', $_SESSION)){
        if (array_key_exists($key, $_SESSION['params'])){
            unset($_SESSION['params'][$key]);
        }
    }
}

function get_langpair(){
    global $default_srclang, $default_trglang;
    if (isset($_GET['langpair'])){
        list($srclang,$trglang) = explode('-',$_GET['langpair']);
        $_SESSION['params']['src'] = $srclang;
        $_SESSION['params']['trg'] = $trglang;
    }
    else{
        $srclang   = get_param('src', $default_srclang);
        $trglang   = get_param('trg', $default_trglang);
    }
    $langpair  = implode('-',[$srclang,$trglang]);
    return [$srclang, $trglang, $langpair];
}


function set_langpair($langpair){
    list($srclang,$trglang) = explode('-',$langpair);
    $_SESSION['params']['src'] = $srclang;
    $_SESSION['params']['trg'] = $trglang;
    $_GET['src'] = $srclang;
    $_GET['trg'] = $trglang;
    $_GET['langpair'] = $langpair;
}

function make_query($data){
    if ( isset( $_COOKIE['PHPSESSID'] ) ) {
        return http_build_query($data);
    }
    if (array_key_exists('params', $_SESSION)){
        $params = $_SESSION['params'];
    }
    else{
        $params = array();
    }
    foreach ($data as $key => $value){
        $params[$key] = $value;
    }
    return http_build_query($params);
}

function make_share_link(){
    if (array_key_exists('params', $_SESSION)){
        $params = $_SESSION['params'];
    }
    else{
        $params = array();
    }
    return http_build_query($params);
}

function test_input($data) {
  $data = trim($data);
  $data = stripslashes($data);
  $data = htmlspecialchars($data);
  return $data;
}


function clear_session(){
    // cleanup_cache();
    $_SESSION = array();
    // session_reset();
}

function score_color($score){
    
    $avg = 50;
    $good = 100;

    $diff = 100*$score-$avg;

    $red=255;
    $green=255;
    $blue=255;

    if ($diff<0){
        $change1 = abs(pow((0-$diff/$avg),2)*64);
        $change2 = abs(($diff/$avg+1)*32);
        $green-=$change1;
        $blue-=$change1+$change2;
    }
    else{
        $change1 = abs(pow(($diff/$good),1)*96);
        $change2 = 0;
        if ($diff<$good){
            $change2 = abs((1-$diff/$good)*32);
        }
        if ($change1>64){
            $change1 = 64;
        }
        $red-=$change1;
        $blue-=$change1+$change2;
    }
    return sprintf("#%02x%02x%02x",$red,$green,$blue);
}


?>
