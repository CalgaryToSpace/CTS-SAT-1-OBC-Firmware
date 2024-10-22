#include "debug_tools/debug_uart.h"
#include <stdint.h>
#include "transforms/sha256.h"
#include "log/log.h"

uint8_t compute_sha256_hash(uint8_t* message, size_t message_length, uint8_t* digest) {
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