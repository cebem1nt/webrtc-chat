#ifndef _CRYPT_H
#define _CRYPT_H

#include <stdlib.h>

#define SHA1_BLOCK_SIZE 20

char* base64_encode(const unsigned char* data, size_t  input_length, size_t* output_length);

void SHA1(const unsigned char *data, size_t size, unsigned char hash[]);

#endif