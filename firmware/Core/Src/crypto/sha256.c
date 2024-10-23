#include <stdint.h>
#include "crypto/sha256.h"
#include "log/log.h"
/*
 * @brief compute sha256 hash for an array of bytes of length *message_length*, store the result in digest
 * @param message array of bytes to hash
 * @param message_length number of bytes in message
 * @param digest a pointer to a 32 byte array to store the result
 * @return 0 if successful, >0 for error
 */
uint8_t CRYPT_compute_sha256_hash(uint8_t* message, size_t message_length, uint8_t* digest) {
    if (cmox_initialize(NULL) != CMOX_INIT_SUCCESS)
    {
        //handle error
        LOG_message(
            LOG_SYSTEM_OBC,
            LOG_SEVERITY_ERROR,
            LOG_SINK_ALL,
            "cmox crypto library initialization failed \n"
        );

        return 1;
    }

    cmox_hash_retval_t ret;
    ret = cmox_hash_compute(CMOX_SHA256_ALGO, message, message_length , digest, CMOX_SHA256_SIZE, NULL);         
    
    if (ret != CMOX_HASH_SUCCESS)
    {
        LOG_message(
            LOG_SYSTEM_OBC,
            LOG_SEVERITY_ERROR,
            LOG_SINK_ALL,
            "sha256 hash failed \n"
        );
        return 1;
    }
    cmox_finalize(NULL);
    return 0;
}