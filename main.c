/* Command-line interface for pw (password book)
 * version:  0.1
 * author:   lex.xiao
 */
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#define _GUN_SOURCE //ask stdio.h include asprintf();
#include <stdio.h>
#include <unistd.h>
#include "base64.h"
#include "aes_crypt.h"
#include "sqlite.h"
#define VERSION 0.1
#define PATH_TO_DB "./password.db"
void help(){
    printf("Command-line interface for pw (password book) \n"
           "Encrypt Mode : AES256                         \n"
           "VERSION : 0.1                                 \n"
           "pw -[armfs] opinions                          \n"
           "     -a  add username and password                 \n"
           "         eg. pw -a domain:username:passwd          \n"
           "     -r  remove username and password              \n"
           "         eg. pw -r domain                          \n"
           "     -m  update username and password              \n"
           "         eg. pw -m domain:username:passwd          \n"
           "     -f  find a domain exits a passwd              \n"
           "         eg. pw -f domain                          \n"
           "     -s  show passwd in the domain                 \n"
           "     -l  show all list in the database             \n"
           "     -h  show help                                 \n");
}

static int countdelimiter(const char *str1, const char* delimiter){
    char *argv2 = strdup(str1);
    int count = 0;
    char *txt, *scratch;
    while((txt = strtok_r(!count? argv2:NULL, delimiter, &scratch))){
        count++;
        //printf("%d : %s\n", count, txt);
    }
    free(argv2);
    return count-1;
}

static int checkdomain(const char *domain){
    if(countdelimiter(domain, ":")!=0 && countdelimiter(domain, ".")!=1){
        fprintf(stderr, "Fail in checkdomain(): domain you type in should be like google.com");
        exit(2);
    }
    else return 0;
}

static int checkdomainusernamepasswd(const char * dupass){
    if(countdelimiter(dupass, ":")!=2) {
        fprintf(stderr, "Fail in checkdomainusernamepasswd(): you should type like google.com:username:passwd\n");
        exit(2);
    }
    else return 0;
}

static char *getkey(){
    char *key = malloc(sizeof(char)*20);
    key = getpass("Type a key :"); //getpass(): pass would not print back to terminal;
    if(strlen(key)>19) {
        fprintf(stderr, "Fail in add(): the length of key should not be greater than 20\n");
        exit(2);
    }
    return key;
}

int add(sqlite3 *db, const char *argv2){
    checkdomainusernamepasswd(argv2);
    char *argv = strdup(argv2); //free(argv);
    char *txt, *scratch, *delimiter = ":";
    char * sql_v[3];
    int count = 0;
    while((txt = strtok_r(!count? argv:NULL, delimiter, &scratch))){
        sql_v[count] = txt;
        count++;
    }
    checkdomain(sql_v[0]);
    char *key = getkey();
    //printf("\nKEY: %s\n", key);
    sql_v[2] = encrypt_aes256(sql_v[0], sql_v[2], key);
    char *temp = sql_v[2];
    sql_v[2] = base64_encode(sql_v[2], strlen(sql_v[2])+1);
    char *sql;
    asprintf(&sql, "INSERT INTO passwd (domain, username, password) VALUES ('%s', '%s', '%s');",
            sql_v[0], sql_v[1],sql_v[2]);
    //printf("SQL: %s\n", sql);
    free(temp);
    free(key);
    free(argv);
    int errcode = sqlite_nocallback(db, sql);
    if(errcode) {
        printf("sql errcode : %d\n", errcode);
        exit(2);
    }
    free(sql);
    return 0;
}

int find(sqlite3 *db, const char* argv){
    checkdomain(argv);
    int isfind = find_domain(db, argv);
    return isfind;
}

int rmdomain(sqlite3 *db, const char * argv){
    int isfind = find(db, argv);
    if(isfind == -1){
        fprintf(stderr, "Fail in rmdomain(): no such domain found in database\n");
        exit(2);
    }
    else if(isfind == 0){
        char *sql;
        asprintf(&sql, "DELETE FROM passwd WHERE domain = '%s';", argv);
        int errcode = sqlite_nocallback(db, sql);
        if(errcode){
            printf("sql errcode : %d\n", errcode);
            exit(2);
        }
        return 0;
    }
    else return -1;
}

int update(sqlite3 *db, const char *argv2){
    checkdomainusernamepasswd(argv2);
    char *argv = strdup(argv2); //free(argv2);
    char *domain, *scratch , *delimiter = ":";
    domain = strtok_r(argv, delimiter, &scratch);
    rmdomain(db, domain);
    add(db, argv2);
    free(argv);
    return 0;
}

int list(sqlite3 *db){
    list_domain(db);
    return 0;
}

int showpasswd(sqlite3 *db, const char *domain){
    int isfind = find(db, domain);
    if(isfind == -1){
        fprintf(stderr, "Fail in rmdomain(): no such domain found in database\n");
        exit(2);
    }
    else if(isfind == 0){
        char *key = getkey(); //free(key);
        show_password(db, domain, key);
        free(key);
        return 0;
    }
    else return 1;
}

int main(int argc, char *argv[]){
    int errcode = 0;
    sqlite3 *db;
    db = init_sqlite(db, PATH_TO_DB);
    if(argc == 2 && !strcmp(argv[1], "-l")) {
        list(db);
        goto freedb;
    }
    if(argc != 3) {
        help();
        goto freedb;
    }
    if (!strcmp(argv[1], "-a")){
        add(db, argv[2]);
    }
    else if(!strcmp(argv[1], "-f")){
        find(db, argv[2]);
    }
    else if(!strcmp(argv[1], "-r")){
        rmdomain(db, argv[2]);
    }
    else if(!strcmp(argv[1], "-m")){
        update(db, argv[2]);
    }
    else if(!strcmp(argv[1], "-s")){
        showpasswd(db, argv[2]);
    }
    else {
        help();
    }
freedb:
    if((errcode = end_sqlite(db))) return errcode;
}
