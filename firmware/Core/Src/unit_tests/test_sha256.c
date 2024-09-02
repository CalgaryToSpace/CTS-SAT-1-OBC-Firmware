#include "unit_tests/unit_test_helpers.h"
#include <stdint.h>
#include "unit_tests/sha256_unit_test_cases.h"
#include "transforms/sha256.h"
#include <string.h>
#include "debug_tools/debug_uart.h"
uint8_t TEST_EXEC__compute_sha256_hash() {
    uint8_t message[] = "abc" ;
    uint8_t expected[] = {0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea, 
                                0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23, 
                                0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c, 
                                0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad};
    uint8_t digest[32];
    compute_sha256_hash(message, 3, digest);
    TEST_ASSERT_TRUE(memcmp(digest, expected, 32) == 0);


    

    //random tests 
    compute_sha256_hash(random_message_0, random_message_0_size, digest);    
    TEST_ASSERT_TRUE(memcmp(digest, random_expected_0, CMOX_SHA256_SIZE) == 0);
    compute_sha256_hash(random_message_1, random_message_1_size, digest);    
    TEST_ASSERT_TRUE(memcmp(digest, random_expected_1, CMOX_SHA256_SIZE) == 0);
    compute_sha256_hash(random_message_2, random_message_2_size, digest);    
    TEST_ASSERT_TRUE(memcmp(digest, random_expected_2, CMOX_SHA256_SIZE) == 0);
    compute_sha256_hash(random_message_3, random_message_3_size, digest);    
    TEST_ASSERT_TRUE(memcmp(digest, random_expected_3, CMOX_SHA256_SIZE) == 0);
    compute_sha256_hash(random_message_4, random_message_4_size, digest);    
    TEST_ASSERT_TRUE(memcmp(digest, random_expected_4, CMOX_SHA256_SIZE) == 0);
    compute_sha256_hash(random_message_5, random_message_5_size, digest);    
    TEST_ASSERT_TRUE(memcmp(digest, random_expected_5, CMOX_SHA256_SIZE) == 0);
    compute_sha256_hash(random_message_6, random_message_6_size, digest);    
    TEST_ASSERT_TRUE(memcmp(digest, random_expected_6, CMOX_SHA256_SIZE) == 0);
    compute_sha256_hash(random_message_7, random_message_7_size, digest);    
    TEST_ASSERT_TRUE(memcmp(digest, random_expected_7, CMOX_SHA256_SIZE) == 0);
    compute_sha256_hash(random_message_8, random_message_8_size, digest);    
    TEST_ASSERT_TRUE(memcmp(digest, random_expected_8, CMOX_SHA256_SIZE) == 0);
    compute_sha256_hash(random_message_9, random_message_9_size, digest);    
    TEST_ASSERT_TRUE(memcmp(digest, random_expected_9, CMOX_SHA256_SIZE) == 0);
    compute_sha256_hash(random_message_10, random_message_10_size, digest);    
    TEST_ASSERT_TRUE(memcmp(digest, random_expected_10, CMOX_SHA256_SIZE) == 0);
    compute_sha256_hash(random_message_11, random_message_11_size, digest);    
    TEST_ASSERT_TRUE(memcmp(digest, random_expected_11, CMOX_SHA256_SIZE) == 0);
    compute_sha256_hash(random_message_12, random_message_12_size, digest);    
    TEST_ASSERT_TRUE(memcmp(digest, random_expected_12, CMOX_SHA256_SIZE) == 0);
    compute_sha256_hash(random_message_13, random_message_13_size, digest);    
    TEST_ASSERT_TRUE(memcmp(digest, random_expected_13, CMOX_SHA256_SIZE) == 0);
    compute_sha256_hash(random_message_14, random_message_14_size, digest);    
    TEST_ASSERT_TRUE(memcmp(digest, random_expected_14, CMOX_SHA256_SIZE) == 0);
    compute_sha256_hash(random_message_15, random_message_15_size, digest);    
    TEST_ASSERT_TRUE(memcmp(digest, random_expected_15, CMOX_SHA256_SIZE) == 0);
    compute_sha256_hash(random_message_16, random_message_16_size, digest);    
    TEST_ASSERT_TRUE(memcmp(digest, random_expected_16, CMOX_SHA256_SIZE) == 0);
    compute_sha256_hash(random_message_17, random_message_17_size, digest);    
    TEST_ASSERT_TRUE(memcmp(digest, random_expected_17, CMOX_SHA256_SIZE) == 0);
    compute_sha256_hash(random_message_18, random_message_18_size, digest);    
    TEST_ASSERT_TRUE(memcmp(digest, random_expected_18, CMOX_SHA256_SIZE) == 0);
    compute_sha256_hash(random_message_19, random_message_19_size, digest);    
    TEST_ASSERT_TRUE(memcmp(digest, random_expected_19, CMOX_SHA256_SIZE) == 0);






    return 0;


}