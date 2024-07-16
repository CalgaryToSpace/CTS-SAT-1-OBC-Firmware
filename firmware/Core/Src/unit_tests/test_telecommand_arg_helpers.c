#include "telecommands/telecommand_args_helpers.h"
#include "unit_tests/unit_test_helpers.h"


uint8_t TEST_EXEC__TCMD_hex_string_to_byte_array(){

    uint8_t result[4];

    char normal[9] = "FFFFFFFF\0";
    TEST_ASSERT(TCMD_extract_hex_array_arg(normal,0,result,sizeof(result))==0); // tests if function has no error

    TEST_ASSERT(result[0]==0xFF);
    TEST_ASSERT(result[1]==0xFF);
    TEST_ASSERT(result[2]==0xFF);
    TEST_ASSERT(result[3]==0xFF);

    uint8_t seg_result[3];
    TEST_ASSERT(TCMD_extract_hex_array_arg(normal,0,seg_result,sizeof(seg_result))==4); // tests if result does not fit hex string

    char normal_delim[13] = "FF_e2AA 01\0";
    TEST_ASSERT(TCMD_extract_hex_array_arg(normal_delim,0,result,sizeof(result))==0); // tests if function has no error
    TEST_ASSERT(result[0]==0xFF);
    TEST_ASSERT(result[1]==0xE2);
    TEST_ASSERT(result[2]==0xAA);
    TEST_ASSERT(result[3]==0x01);

    char normal_args_and_uneven[22] = "FF,012200FF,FFF\0";
    TEST_ASSERT(TCMD_extract_hex_array_arg(normal_args_and_uneven,1,result,sizeof(result))==0); // standard test for args
    TEST_ASSERT(result[0]==0x01);
    TEST_ASSERT(result[1]==0x22);
    TEST_ASSERT(result[2]==0x00);
    TEST_ASSERT(result[3]==0xFF);

    TEST_ASSERT(TCMD_extract_hex_array_arg(normal_args_and_uneven,2,result,sizeof(result))==1); //testing uneven in args

    char not_even[4]="FFF\0";
    TEST_ASSERT(TCMD_extract_hex_array_arg(not_even,0,result,sizeof(result))==1); // testing uneven

    char not_even_delimiter[5]="FF_F\0";
    TEST_ASSERT(TCMD_extract_hex_array_arg(not_even_delimiter,0,result,sizeof(result))==1); // testing uneven with delim
    
    char header_args[10] = "0xFF,0xFF\0";
    TEST_ASSERT(TCMD_extract_hex_array_arg(header_args,0,result,sizeof(result))==2); //testing header
    TEST_ASSERT(TCMD_extract_hex_array_arg(header_args,1,result,sizeof(result))==2); //testing header for args

    char no_string[2] = "\0";
    TEST_ASSERT(TCMD_extract_hex_array_arg(no_string,0,result,sizeof(result))==3); //testing empty string
    char no_string_arg[2]=",\0";
    TEST_ASSERT(TCMD_extract_hex_array_arg(no_string_arg,0,result,sizeof(result))==3); //testing empty string in arg=0
    TEST_ASSERT(TCMD_extract_hex_array_arg(no_string_arg,1,result,sizeof(result))==3); // testing empty string in arg=1

    return 0;
}