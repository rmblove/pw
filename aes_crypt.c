/* This app is for encrypt and decrypt password  with domain(URL) and KEY
 * domain is for iv in gcry_cipher_setiv
 * Version: 0.1
 * Author:  lex.xiao
 * Data:    2016.10.1
 */
#include "aes_crypt.h"
#include <gcrypt.h> //build with gcc -o foo foo.c `libgcrpyt-config --cflags --libs`
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stddef.h>

static void log_crypt(int err){
    assert(err!=0);
    fprintf(stderr, "Failure: %s %s\n", gcry_strsource(err), gcry_strerror(err));
    fprintf(stderr, "Error Code: %d\n", err);
    exit(2);
}
static void init_crypt()
{
    if(!gcry_check_version(GCRYPT_VERSION)){
        fprintf(stderr, "libgcrpyt verion mismatch \n");
        exit(2);
    }
    gcry_control(GCRYCTL_DISABLE_SECMEM, 0);
    gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);
}
enum crypt_type {
    ENCRYPT,
    DECRYPY,
};
static char* crypt_with_domain(const char * domain, const char * passwd, const char *global_key, enum crypt_type ct){
    init_crypt();
    gcry_cipher_hd_t handle;
    gcry_error_t err = 0;
    size_t key_size = gcry_cipher_get_algo_keylen(GCRY_CIPHER_AES256);
    size_t block_size = gcry_cipher_get_algo_blklen(GCRY_CIPHER_AES256);
    size_t global_key_len = strlen(global_key);
    size_t passwd_size = strlen(passwd) + 1;
    size_t domain_size = strlen(domain) + 1;
    err = gcry_cipher_open(&handle, GCRY_CIPHER_AES256, GCRY_CIPHER_MODE_CFB, 0); //GCRY_CIPHER_MODE_CFB with this mode, passwd does not need to fill up to block_size.
    if(err) log_crypt(err);
    //printf("key_size : %zu\n", key_size);
    //printf("block_size : %zu\n", block_size);
    char key[key_size];
    char *salt = "hahaa";
    err = gcry_kdf_derive(global_key, global_key_len, GCRY_KDF_PBKDF2, GCRY_MD_SHA256,
                        salt, sizeof(*salt), 5, key_size, key);
    if(err) log_crypt(err);
    //printf("key: %s\n", key);
    err = gcry_cipher_setkey(handle, key, key_size);
    if(err) log_crypt(err);
    char *iv = malloc(block_size);
    memset(iv, 0 , block_size);
    memcpy(iv, domain, ((block_size > domain_size)? domain_size : block_size));
    err = gcry_cipher_setiv(handle, iv, block_size);
    if(err) log_crypt(err);
    char *enc_passwd = malloc(passwd_size);
    if (ct == ENCRYPT){
        err = gcry_cipher_encrypt(handle, enc_passwd, passwd_size, passwd, passwd_size);
    }
    else if (ct == DECRYPY){
        err = gcry_cipher_decrypt(handle, enc_passwd, passwd_size, passwd, passwd_size);
    }
    if(err) log_crypt(err);
    gcry_cipher_close(handle);
    return enc_passwd;
}

char *
encrypt_aes256(const char *domain, const char *passwd , const char *key){
    return crypt_with_domain(domain, passwd, key, ENCRYPT);
}

char *
decrypt_aes256(const char *domain, const char *passwd, const char *key){
    return crypt_with_domain(domain, passwd, key, DECRYPY);
}


//int main(int argc, char * args[]){
//    assert(argc == 4);
//    char *passwd_encrypted;
//    passwd_encrypted = encrypt(args[1], args[2], args[3]);
//    printf("encrypted text : %s \n", passwd_encrypted);
//    char *passwd_decrypted;
//    passwd_decrypted = decrypt(args[1], passwd_encrypted, args[3]);
//    printf("decrypted test : %s \n", passwd_decrypted);
//
//
//}
