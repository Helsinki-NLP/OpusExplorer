

# TODO's

* bugfix in link DB: (linkID,sentID) does not have to be unique
* enable monolingual search
* show cleanerScores (but right now there are no scores in the DB)
* enable editing without sentence index DBs (*.ids.db) - requires to create and update user-specific *linked.db files!
* show average rating per bitext (maybe also corpus?) - DONE (except for corpora)
* allow to search for document names in corpora (relevant for OpenSubtitles etc) - DONE


# SQLite

* on query optimization: https://sqlite.org/optoverview.html
* FTS search: https://www.sqlitetutorial.net/sqlite-full-text-search/


# Links

Language codes:

* https://github.com/matriphe/php-iso-639
* https://github.com/Daniel-KM/Simple-ISO-639-3
* https://www.php.net/manual/en/locale.getdisplayscript.php
* https://stackoverflow.com/questions/69002935/convert-iso-639-1-into-a-language-name-in-local-language-with-php
