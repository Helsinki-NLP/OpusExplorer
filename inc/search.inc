<?php // -*-php-*-



function print_search_form($langpair, $corpus='', $version='', $fromDoc='', $toDoc='', $bitextID=0,
                           $searchquery='', $allowSourceSearch=1, $allowTargetSearch=1){
    
    echo(' <span class="search-form"><form action="'.$_SERVER['PHP_SELF'].'" method="get" style="display: inline;">');
    echo('<input type="hidden" id="langpair" name="langpair" value="'.$langpair.'">');
    echo('<input type="hidden" id="offset" name="offset" value="0">');
    echo('<input type="hidden" id="limit" name="limit" value="10">');
    if ($corpus && $version){
        echo('<input type="hidden" id="corpus" name="corpus" value="'.$corpus.'">');
        echo('<input type="hidden" id="version" name="version" value="'.$version.'">');
    }
    if ($fromDoc && $toDoc){
        echo('<input type="hidden" id="fromDoc" name="fromDoc" value="'.$fromDoc.'">');
        echo('<input type="hidden" id="toDoc" name="toDoc" value="'.$toDoc.'">');
    }
    if ($bitextID){
        echo('<input type="hidden" id="bitextID" name="bitextID" value="'.$bitextID.'">');
    }
    echo('<input size="12" type="text" name="search" id="search" value="'.$searchquery.'" required />&nbsp;&nbsp;');
    if ($allowSourceSearch && $allowTargetSearch){
        echo('<input type="submit" name="action" value="search source" />');
        echo('<input type="submit" name="action" value="search target" />');
    }
    elseif ($allowSourceSearch) echo('<button type="submit" name="action" value="search source">search</button>');
    elseif ($allowTargetSearch) echo('<button type="submit" name="action" value="search target">search</button>');

    echo('</form></span> ');
}


function search($searchquery, $searchside, $bitext, $limit=10, $offset=0, $orderByLinkID=0){
    $alignments = $bitext->search($searchquery, $searchside, $limit, $offset, $orderByLinkID);
    print_bitext_table($bitext, $alignments, $offset);
    return;
}

    


?>
