

# TODO's


* improve speed of browsing through data by better use of rowids (instead of limit/offset queries), see https://stackoverflow.com/questions/12266025/sqlite-query-optimization-using-limit-and-offset; maybe store beginning and end rowid for each bitext?
* show random sentence alignments: can be done with `ORDER BY RANDOM()`, but this becomes superslow on big corpora (because limit cannot just chop off the first results, ordering needs to be done first) - can we do some cascaded select trick? (see https://www.sqlitetutorial.net/sqlite-subquery/)
* sort by scores (same problem as above, it becomes slow to order big corpora)
* improve language selection screen (need to scale if many language pairs will be available); could be similar to opus-mt dashboard
* bugfix in link DB: (linkID,sentID) does not have to be unique (Quite a complex change with lots of implications. Can we leave it like this? The chance to have the same sentence twice in one translation unit is quite small.)
* enable editing without sentence index DBs (*.ids.db) - requires to create and update user-specific *linked.db files!
* show average rating per bitext (maybe also corpus?) - DONE (except for corpora)
* allow to search for document names in corpora (relevant for OpenSubtitles etc) - DONE


## bigger plans

* add word alignment information somehow (big change)
* export to xces align format, tmx format?
* allow bitext upload?
* add other views: my rated documents, alignments, ...
* cleanup code, define classes like opusindex, bitext, alignment
* show cleanerScores (but right now there are no scores in the DB)
* relevance ranking in search results?
* enable monolingual search
* enable search in both languages


## other ideas

* link OpenSubtitles to metadata from IMDb (https://datasets.imdbws.com/), remember to add leading '0' to the ID with something like `sprintf('%07d', 12345);`
* add the completely linked subtitles corpus (including alternative subtitles)


# SQLite

* on query optimization: https://sqlite.org/optoverview.html
* FTS search: https://www.sqlitetutorial.net/sqlite-full-text-search/


# Links

Language codes:

* https://github.com/matriphe/php-iso-639
* https://github.com/Daniel-KM/Simple-ISO-639-3
* https://www.php.net/manual/en/locale.getdisplayscript.php
* https://stackoverflow.com/questions/69002935/convert-iso-639-1-into-a-language-name-in-local-language-with-php

## Acknowledgements

This project has received funding from the European Union’s Horizon Europe research and innovation programme under grant agreement No 101070350 and from UK Research and Innovation (UKRI) under the UK government’s Horizon Europe funding guarantee [grant number 10052546]
