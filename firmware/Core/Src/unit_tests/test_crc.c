#include <stdint.h>

#include "unit_tests/unit_test_helpers.h"
#include "unit_tests/test_crc.h"
#include "crc/crc.h"
#include "main.h"

/*
Online CRC calculator for reference: https://www.sunshine2k.de/coding/javascript/crc/crc_js.html

Select CRC32, custom parameters:
Input reflected: false
Outout reflected: false
Initial value: 0xFFFFFFFF
Final Xor value: 0
*/
uint8_t TEST_EXEC__test_crc_empty()
{
    uint32_t expected_crc = 0x4E08BFB4;  // Checksum result for null input

    uint8_t data[] = {0};

    uint32_t crc = GEN_crc32_checksum(data, 1);

    TEST_ASSERT(crc == expected_crc);

    return 0;
}

uint8_t TEST_EXEC__test_crc_max()
{
    /*
        String from: https://www.lipsum.com/feed/html 
        1024 bytes (Try minus 1 for end of string character)

    */
    
    char* data = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Proin malesuada libero nec   imperdiet vestibulum. Cras mattis ut est blandit pulvinar. Etiam quis quam sapien. Quisque ornare quam aliquet pharetra interdum. Etiam convallis facilisis nunc, eu dapibus mi mattis quis. Quisque quis aliquam sem. Morbi vitae ante non justo lacinia consequat. Vestibulum blandit mattis cursus.Curabitur lorem purus, tristique venenatis vestibulum nec, lobortis non purus. Aliquam erat volutpat. Duis pellentesque tincidunt sapien eu placerat. Suspendisse et accumsan tortor. In molestie ornare dolor. Nullam sagittis in tellus vel sodales. In iaculis laoreet lacinia. Maecenas blandit leo nec odio consectetur, eget congue erat ultricies. Integer lobortis ut justo eget vulputate. Nam at ullamcorper ipsum, vel mattis odio. Nulla et aliquet nisl. Pellentesque mauris ipsum, mattis at lacus at, sagittis mattis tortor. Duis varius nibh vitae erat tempus, in interdum quam facilisis. Pellentesque dapibus massa nulla, nec sodales augue qui";  // 1 KB of data
    uint32_t expected_crc = 0xB9C2127C;  // CRC from online calculator

    uint32_t crc = GEN_crc32_checksum((uint8_t*) data, 1024);

    TEST_ASSERT(crc == expected_crc);

    return 0;
}


uint8_t TEST_EXEC__test_crc_half()
{
    /*
    String from: https://www.lipsum.com/feed/html 
    512 bytes
    */
    
    char* data = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Suspendisse eget scelerisque sem. Nunc et tortor eleifend, pellentesque tellus id, facilisis mi. Pellentesque pretium posuere lacus fermentum ultricies. Nulla lorem dui, cursus sit amet dapibus vel, dapibus eget tortor. Donec egestas fermentum elementum. Sed ullamcorper nulla eros, id varius nunc varius posuere. Nunc vel erat nec nulla facilisis aliquam. Pellentesque lobortis varius metus, ut auctor tellus rhoncus ut. Phasellus porta fringilla tortor."; // 0.5 kb of data
    uint32_t expected_crc = 0x6C12E412;

    uint32_t crc = GEN_crc32_checksum((uint8_t*) data, 512);

    TEST_ASSERT(crc == expected_crc);

    return 0;
}

uint8_t TEST_EXEC__test_crc_random()
{
    /*
    String from: https://www.lipsum.com/feed/html 
    300 bytes (testing for size!%2)
    */
    char* data = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nunc venenatis tempus ex nec sagittis. In vel lorem auctor, tincidunt turpis in, aliquam orci. Cras ut velit quis augue finibus porttitor. Aliquam feugiat consectetur arcu id pellentesque. Duis commodo cursus erat, volutpat ornare lectus nisi."; // 300 bytes of data
    uint32_t expected_crc = 0xC1F6048C;

    uint32_t crc = GEN_crc32_checksum((uint8_t*) data, 300);

    TEST_ASSERT(crc == expected_crc);

    return 0;
}

uint8_t TEST_EXEC__test_crc_special()
{
    char* data ="#@?/`')(";
    uint32_t expected_crc = 0x72D7BC15;

    uint32_t crc = GEN_crc32_checksum((uint8_t*) data, 8);

    TEST_ASSERT(crc == expected_crc);

    return 0;
}
uint8_t TEST_EXEC__test_crc_odd()
{
    char* data ="12Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec massa orci, iaculis eget ultrices ut, pretium eu orci. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Curabitur nec laoreet odio. Phasellus odio.";
    uint32_t expected_crc = 0x585C8EA2;

    uint32_t crc = GEN_crc32_checksum((uint8_t*) data, 257);

    TEST_ASSERT(crc == expected_crc);

    return 0;
}