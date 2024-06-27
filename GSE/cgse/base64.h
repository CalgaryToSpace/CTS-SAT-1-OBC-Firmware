#ifndef __INCLUDE__GUARD__BASE64_H_
#define __INCLUDE__GUARD__BASE64_H_

#include <stdlib.h>

char * CGSE_base64_encode_from_file(char *file_name, size_t *file_size);
char * CGSE_base64_encode_bytes(uint8_t *byte_array, int len);
char CGSE_base64_encode_character(uint8_t bits);

#endif // __INCLUDE__GUARD__BASE64_H_

