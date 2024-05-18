
#ifndef __INCLUDE_GUARD__UNIT_TEST_INVENTORY_H__
#define __INCLUDE_GUARD__UNIT_TEST_INVENTORY_H__

#include "main.h"


typedef uint8_t (*TEST_Function_Ptr)();

typedef struct {
	TEST_Function_Ptr test_func;
	char* test_file;
    char* test_func_name;
} TEST_Definition_t;


extern const TEST_Definition_t TEST_definitions[];
extern const int16_t TEST_definitions_count;


#endif // __INCLUDE_GUARD__UNIT_TEST_INVENTORY_H__
