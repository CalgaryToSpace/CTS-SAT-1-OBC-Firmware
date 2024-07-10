#include "unit_tests/unit_test_helpers.h"
#include "telecommands/telecommand_args_helpers.h"
#include "string.h"

uint8_t TEST_EXEC__TCMD_hex_string_to_byte_array(){
    char normal[9] = "FFFFFFFF\0";
    char normal_delim[13] = "FF_FFAA 01\0";
    char normal_args_and_uneven[22] = "FF,012200FF,FFF\0";
    char not_even[3]="FFF";
    char not_even_delimiter[5]="FF_F\0";
    char header_args[10] = "0xFF,0xFF\0";
    // uint8_t expected_answer[2][4] = {
    //     {255,255,255,255},
    //     {1,22,0,255}
    // };
    // uint8_t answer[4] = {255,255,255,255};
    uint8_t result[4];
    // uint8_t result[3];
    uint8_t val = 0;

    TEST_ASSERT(TCMD_extract_hex_array_arg(normal,0,result,sizeof(result))==0); // tests if function has no error

    for(int i=0;i<4;i++){
        if (result[i]!=255){
            val = 1;
        }
    }

    TEST_ASSERT(TCMD_extract_hex_array_arg(normal_delim,0,result,sizeof(result))==0); // tests if function has no error

    if (result[0]!=255){val=1;} 
    if (result[1]!=255){val=1;} 
    if (result[2]!=170){val=1;} 
    if (result[3]!=1){val=1;} 

    TEST_ASSERT(TCMD_extract_hex_array_arg(normal_args_and_uneven,1,result,sizeof(result))==0);
    if (result[0]!=1){val=1;}
    if (result[1]!=34){val=1;}
    if (result[2]!=0){val=1;}
    if (result[3]!=255){val=1;}

    TEST_ASSERT(TCMD_extract_hex_array_arg(not_even,0,result,sizeof(result))==1); // testing uneven
    uint8_t result_odd[3];
    TEST_ASSERT(TCMD_extract_hex_array_arg(not_even_delimiter,0,result,sizeof(result))==1); // testing uneven deli
    TEST_ASSERT(TCMD_extract_hex_array_arg(normal_args_and_uneven,2,result,sizeof(result))==1);
    TEST_ASSERT(TCMD_extract_hex_array_arg(header_args,0,result,sizeof(result))==2); //testing header
    TEST_ASSERT(TCMD_extract_hex_array_arg(header_args,1,result,sizeof(result))==2); //testing header for arguement
    
    
    

    TEST_ASSERT(val == 0);



    return 0;
}