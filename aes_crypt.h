#ifndef _aes_crypt_h
#define _aes_crypt_h
#include <gcrypt.h>

char *
encrypt_aes256(const char *domain, const char *passwd, const char *key);

char *
decrypt_aes256(const char *domain, const char *passwd, const char *key);


#endif /*aes_crypt.h*/
