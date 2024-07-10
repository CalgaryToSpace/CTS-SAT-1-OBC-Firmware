#include "telecommands/telecommand_args_helpers.h"

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


/// @brief Extracts the longest substring of integer characters, starting from the beginning of the
///     string, to a maximum length or until the first non-integer character is found.
/// @param str Input string, starting with an integer
/// @param str_len Max length of the input string
/// @param result Pointer to the result
/// @return 0 if successful, 1 if the string is empty, 2 if the string does not start with an integer
uint8_t TCMD_ascii_to_uint64(const char *str, uint32_t str_len, uint64_t *result) {
    // FIXME: return error if the string is too long/number is too large
    // FIXME: return error if the number doesn't occupy the whole string (e.g., "123abc" with str_len=6 should error)
    // TODO: write unit tests for this function
    // TODO: consider removing the str_len parameter and using strlen(str) instead (requires refactor in caller)
    if (str_len == 0) {
        return 1;
    }

    uint64_t temp_result = 0;
    uint32_t i = 0;
    for (; i < str_len; i++) {
        if (str[i] < '0' || str[i] > '9') {
            break;
        }

        temp_result = temp_result * 10 + (str[i] - '0');
    }

    if (i == 0) {
        return 2;
    }

    *result = temp_result;
    return 0;
}

/// @brief Extracts the nth comma-separated argument from the input string, assuming it's a uint64
/// @param str Input string
/// @param str_len Length of the input string
/// @param arg_index Index of the argument to extract (0-based)
/// @param result Pointer to the result
/// @return 0 if successful, 1 if the string is empty, 2 if the string does not contain enough arguments
///        3 if the argument is not an integer, 4 for other error
uint8_t TCMD_extract_uint64_arg(const char *str, uint32_t str_len, uint8_t arg_index, uint64_t *result) {
    if (str_len == 0) {
        return 1;
    }

    uint32_t arg_count = 0;
    uint32_t i = 0;
    uint32_t start_index = 0;
    for (; i < str_len; i++) {
        if (str[i] == ',') {
            if (arg_count == arg_index) {
                break;
            }

            arg_count++;
            start_index = i + 1;
        }
    }

    if (arg_count < arg_index) {
        return 2;
    }

    uint8_t parse_result = TCMD_ascii_to_uint64(&str[start_index], i - start_index, result);
    if (parse_result == 2) {
        // The argument is not an integer
        return 3;
    }
    else if (parse_result > 0) {
        // Other error
        return 4;
    }
    return 0;
}

/// @brief Extracts the nth comma-separated argument from the input string, assuming it's a string
/// @param str Input string (null-terminated)
/// @param arg_index Index of the argument to extract (0-based)
/// @param result Pointer to the result, to be filled with the extracted string (null-terminated)
/// @param result_max_len Maximum length of the result
/// @return 0 if successful, 1 if the string is empty, 2 if the string does not contain enough arguments
///         3 for other error
uint8_t TCMD_extract_string_arg(const char *str, uint8_t arg_index, char *result, uint16_t result_max_len) {
    const uint16_t str_len = strlen(str);
    if (str_len == 0) {
        return 1;
    }

    uint32_t arg_count = 0;
    uint32_t i = 0;
    uint32_t start_index = 0;
    for (; i < str_len; i++) {
        if (str[i] == ',') {
            if (arg_count == arg_index) {
                break;
            }

            arg_count++;
            start_index = i + 1;
        }
    }

    if (arg_count < arg_index) {
        return 2;
    }

    uint32_t end_index = i;
    if (end_index - start_index >= result_max_len) {
        return 3;
    }

    // Trim leading whitespace
    while (start_index < end_index && (str[start_index] == ' ' || str[start_index] == '\t')) {
        start_index++;
    }

    // Trim trailing whitespace
    while (end_index > start_index && (str[end_index - 1] == ' ' || str[end_index - 1] == '\t')) {
        end_index--;
    }

    uint32_t token_len = end_index - start_index;

    if (token_len >= result_max_len) {
        // The argument is too long for the result buffer.
        return 3;
    }

    strncpy(result, &str[start_index], token_len);
    result[token_len] = '\0';

    return 0; // Successful extraction
}
/// @brief Grabs a hexidecimal string of 32 bit and parses it into a byte array
/// @param str Input string in hexdecimal format and hex header (null-terminated)
/// @param str_len The length of the input string
/// @param result Pointer to the result; a byte array containing the values of the hex string (No header for byte array length)
/// @return 0 if successful, 1 if the string contains an uneven byte, 2 if the string contains hex header
///         3 for other error,
uint8_t TCMD_extract_hex_array_arg(const char *args_str, uint8_t arg_index, uint8_t result_array[], uint16_t result_array_size){

    if (strlen(args_str) == 0){ //string is empty
        return 3;
    }

    uint32_t arg_count = 0;
    uint32_t start_index = 0;
    uint32_t end_index = 0;
    for (uint32_t i =0; i < strlen(args_str); i++) {
        if (args_str[i] == ',') {
            if (arg_count == arg_index) {
                break;
            }
            arg_count++;
            start_index = i + 1;
        }
    }
    // char header[2]={}

    if (args_str[start_index]=='0' && args_str[start_index+1]=='x'){ // string contain hex header therefore return error
        return 2;
    }
    
    char byte_string[3];
    byte_string[2]='\0';
    uint32_t even_count = 0; // counter to ensure if byte is even
    uint32_t result_index = 0; // index for the result array incremented based off of delimiter or after a byte
    char *endptr;
    // uint32_t delim_flag=0; //check after the delim
    
    uint32_t char_count = 0; //checks for uneven without delimiter
    for (uint32_t i = start_index; i<strlen(args_str); i++){
        if (args_str[i]==',' || args_str[i]=='\0' ){
            break;
        }

        if (args_str[i]==' ' || args_str[i]=='_'){
            if (char_count==1){ //check if previous was uneven
                return 1;
            }
            char_count=0; //reset char count to begin counting again
            continue;
        } 
        else
        {
            char_count++;
        }

        uint32_t byte_index=(char_count-1)%2;
        byte_string[byte_index]=(uint8_t)args_str[i];
        
        if (char_count%2==0) // char count == 2 therefore a byte can be successfully converted and put into result_array
        {
            char_count = 0;
            long int byte= strtol(byte_string,&endptr,16);
            if (*endptr != '\0') {
                return 3;
            } 
            result_array[result_index] = byte;
            result_index++;
        }    
        
    }

    if (char_count == 1){
        return 1;
    }


    return 0;
}
