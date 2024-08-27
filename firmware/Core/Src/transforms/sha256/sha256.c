#include "cmox_crypto.h"
#include "debug_tools/debug_uart.h"
#include <stdint.h>

uint8_t compute_sha256_hash(uint8_t* message, size_t message_length, uint8_t* digest) {
    if (cmox_initialize(NULL) != CMOX_INIT_SUCCESS)
    {
        //handle error
        DEBUG_uart_print_str("cmox crypto library initialization failed \n");
        return 1;
    }
    cmox_hash_retval_t ret;
    size_t digest_size;
    ret = cmox_hash_compute(CMOX_SHA256_ALGO, message, message_length , digest, CMOX_SHA256_SIZE, &digest_size);         
    if (ret != CMOX_HASH_SUCCESS)
    {
        DEBUG_uart_print_str("hash failed \n");
        return 1;
    }
    cmox_finalize(NULL);
    return 0;
}