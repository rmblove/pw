#include <sqlite3.h> //build with :: -lsqlite3
#define _GUN_SOURCE
#include <stdio.h>
#include "aes_crypt.h"
#include "base64.h"
#include "sqlite.h"

sqlite3 *init_sqlite(sqlite3 *db, const char *path_to_db){
    int err = 0;
    err = sqlite3_open(path_to_db, &db);
    if (err){
        fprintf(stderr, "Can not open the database : %s\n", sqlite3_errmsg(db));
        fprintf(stderr, "ErrorCode : %d\n", err);
        sqlite3_close(db);
        exit(2);
    }
    return db;
}

int end_sqlite(sqlite3 *db){
    sqlite3_exec(db, "commit;", 0, 0 ,0);
    sqlite3_close(db);
    return 0;
}


int sqlite_nocallback(sqlite3 *db, const char *sql){
    int err = 0;
    char *errmsg;
    err = sqlite3_exec(db, sql, 0, 0, &errmsg);
    if(err){
        fprintf(stderr, "Can not insert into db : %s \n", errmsg);
        fprintf(stderr, "ErrorCode : %d\n", err);
        sqlite3_free(errmsg);
        return err;
    }
    return 0;
}

int create_table(sqlite3 *db){
    char *sql = "CREATE TABLE passwd (id INTEGER PRIMARY KEY, domain TEXT, username TEXT, password TEXT);";
    int err = sqlite_nocallback(db, sql);
    if(err) exit(2);
    return 0;
}

static int exec_sql_wcallback(sqlite3 *db, const char *sql, int (*callback)(void *, int, char**, char**), void* callback_arg){
    int err = 0;
    char *errmsg;
    err = sqlite3_exec(db, sql, callback, callback_arg, &errmsg); //if find nothing, find_callback would return 1;
    if(err){
        fprintf(stderr, "Failure in exec_sql_wcallback: %s\n", errmsg);
        sqlite3_free(errmsg);
        return err;
    }
    return 0;
}
static int find_callback(void *isfind, int argc, char **argv, char **azColName){
    int *changeisfind = (int *)isfind;
    *changeisfind = 0;
    int i;
    if (argv == NULL) return 1;
    for (i=0; i<argc; i++){
        printf("%s : %s \n", azColName[i], argv[i] ? argv[i]:NULL);
    }
    printf("\n");
    return 0;
}
// if domain is found, return 0 else return -1;
int find_domain(sqlite3 *db, const char *domain){
    int isfind = -1;
    char *sql;
    int err = 0;
    asprintf(&sql, "SELECT domain, username FROM passwd WHERE domain = '%s';", domain);
    err = exec_sql_wcallback(db, sql, find_callback, &isfind);
    free(sql);
    if(err) exit(2);
    return isfind;
}

static int find_table(void *isfind, int argc, char **argv, char **azColName){
    int *changeisfind = (int*)isfind;
    *changeisfind = -1;
    if(argv[0] && !strcmp(argv[0], "passwd")){
        *changeisfind = 0;
    }
    return 0;
}

// if table passwd is found, return 0 else return -1;
int find_table_passwd(sqlite3 *db){
    int isfind = -1;
    char *sql;
    int err = 0;
    asprintf(&sql, "SELECT name FROM sqlite_master WHERE type = 'table' ORDER BY name;");
    err = exec_sql_wcallback(db, sql, find_table, &isfind);
    free(sql);
    if(err) exit(2);
    return isfind;
}

int list_domain(sqlite3 *db){
    int notused = -1;
    char *sql;
    asprintf(&sql, "SELECT * FROM passwd;");
    int err = 0;
    err = exec_sql_wcallback(db, sql, find_callback, &notused);
    free(sql);
    if (err) exit(2);
    return 0;
}


static int pw_callback(void *key, int argc, char **argv, char **azColName){
    int i;
    char *domain = argv[0];
    char *username = argv[1];
    char *passwd = argv[2];
    char *passwd_decode_base64 = base64_decode(passwd, strlen(passwd) + 1);
    char *temp = passwd_decode_base64; //free(temp);
    char *passwd_decrypted = decrypt_aes256(domain, passwd_decode_base64, (char *)key);
    char *show[] = {domain, username, passwd_decrypted};
    for(i=0; i<argc; i++){
        printf("%s : %s \n", azColName[i], show[i]);
    }
    printf("\n");
    free(temp);
    free(passwd_decrypted);
    return 0;
}

int show_password(sqlite3 *db, const char *domain, char *key){
    int err = 0;
    char *errmsg;
    char *sql; //not finished
    asprintf(&sql, "SELECT domain, username, password FROM passwd WHERE domain = '%s';", domain);
    err = sqlite3_exec(db, sql, pw_callback, key, &errmsg);
    free(sql);
    if(err){
        fprintf(stderr, "Failure in show_passwd(): %s \n", errmsg);
        sqlite3_free(errmsg);
        exit(2);
    }
    return 0;
}
