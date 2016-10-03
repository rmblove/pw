#ifndef _sqlite_h
#define _sqlite_h

sqlite3 *init_sqlite(sqlite3 *db, const char *path_to_db);

int end_sqlite(sqlite3 *db);

int create_table(sqlite3 *db);

int find_table_passwd(sqlite3 *db);

int sqlite_nocallback(sqlite3 *db, const char* sql);

int find_domain(sqlite3 *db, const char *domain);

int list_domain(sqlite3 *db);

int show_password(sqlite3 *db, const char *domain, char *key);

#endif /*sqlite.h*/
