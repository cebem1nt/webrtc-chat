#ifndef _CRYPT_H
#define _CRYPT_H

#include <stdlib.h>

#define SHA1_BLOCK_SIZE 20


unsigned char* base64_encode(const unsigned char *src, size_t len);

/*
 * - src points to any kind of data to be hashed.
 * - bytelength the number of bytes to hash from the src pointer.
 * - hash should point to a buffer of at least 20 bytes of size for storing
 * the sha1 result in.
 */
void SHA1(void* src, size_t bytelength, unsigned char * hash);

/*
 * Returns verified webrtc handshake key
 */
char* sign_key(const char* key);

#endif