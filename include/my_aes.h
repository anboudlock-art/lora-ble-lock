#ifndef __MY_AES_H
#define __MY_AES_H

#include "string.h"

int aes_encrypt_ecb(const unsigned char* Key,const unsigned char KeyLen,unsigned char* PlainContent,unsigned char* CipherContent,const int BlockCount);
int aes_decrypt_ecb(const unsigned char* Key,const unsigned char KeyLen,unsigned char* CipherContent,unsigned char* PlainContent,const int BlockCount);

int my_aes_encrypt(const unsigned char* Key, unsigned char* CipherContent, unsigned char* PlainContent);
int my_aes_decrypt(const unsigned char* Key, unsigned char* CipherContent, unsigned char* PlainContent);

#endif /* __MY_AES_H */
