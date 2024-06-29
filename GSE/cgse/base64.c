#include "base64.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>


char * CGSE_base64_encode_from_file(char *file_name, size_t *file_size)
{
    if (file_name == NULL || strlen(file_name) == 0 || file_size == NULL) {
        return NULL;
    }

    char *base64 = NULL;
    
    FILE *f = fopen(file_name, "r");
    if (f == NULL) {
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    *file_size = ftell(f);
    uint8_t *buffer = malloc(sizeof *buffer * *file_size + 1);
    if (buffer == NULL) {
        fclose(f);
        return NULL;
    }

    fseek(f, 0, SEEK_SET);
    unsigned long bytes_read = fread(buffer, 1, *file_size, f);
    if (bytes_read != *file_size) {
        fclose(f);
        free(buffer);
        return NULL;
    }
   
    fclose(f);

    char *base64_caller_frees = CGSE_base64_encode_bytes(buffer, *file_size);

    free(buffer);

    return base64_caller_frees;
}

char * CGSE_base64_encode_bytes(uint8_t *byte_array, int len)
{
    int base64_size = ((len + 2) / 3) * 4;

    char *base64 = malloc(sizeof *base64 * (base64_size + 1));
    if (base64 == NULL) {
        return NULL;
    }

    uint8_t bits = 0;
    int offset = 0;
    int bytes_encoded = 0;
    int i = 0;
    for (; i < base64_size; i++) {
        offset = i % 4;
        if (bytes_encoded >= len) {
            bits = 255; // trigger '='
        }
        else {
            switch (offset) {
                case 0:
                    bits = byte_array[bytes_encoded] >> 2;
                    break;
                case 1:
                    bits = ((byte_array[bytes_encoded] & 0b11) << 4) | (byte_array[bytes_encoded + 1] >> 4);
                    bytes_encoded++;
                    break;
                case 2:
                    bits = ((byte_array[bytes_encoded] & 0b1111) << 2) | (byte_array[bytes_encoded + 1] >> 6);
                    bytes_encoded++;
                    break;
                case 3:
                    bits = byte_array[bytes_encoded] & 0b111111;
                    bytes_encoded++;
                    break;
            }
        }
        base64[i] = CGSE_base64_encode_character(bits);
    }
    base64[i] = '\0';

    return base64;
}

char CGSE_base64_encode_character(uint8_t bits) {
    uint8_t result = 255;
    if (bits == 63) {
        result ='/';
    }
    else if (bits == 62) {
        result = '+';
    }
    else if (bits > 51 && bits < 62) {
        result = bits - 4;
    }
    else if (bits > 25 && bits < 52) {
        result = bits + 71;
    }
    else if  (bits < 26) {
        result = bits + 65;
    }
    else {
        result = (uint8_t)'='; 
    }

    return (char) result;

}

