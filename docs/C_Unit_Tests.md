# C Unit Tests

This repo uses an in-house unit test framework, which is about as minimal as unit tests can be.

## Why?

The intentions with these unit tests are:

1. Force structuring of functions in ways such that unit tests can be written for them.
2. Come up with tests cases from documentation/experimenting **before** implementing functions
(test-driven development).
3. Provide a way to ensure core functionality continues working in the wake of refactors and
configuration changes.

## Non-Objectives

Companies may care about the following, but we do not:

* Seeking high coverage of unit tests, or enforcing creating unit tests where there is minimal gain.
* Using mocks to emulate difficult-to-test systems.
    * Instead: write functions in a way that the testable part and the difficult-to-test system
    never appear in the same function. For example, implement reading from UART in a different
    function than the parser.

Unit tests, when used correctly, are extremely powerful, and enable far faster development.

## How to Write a Test

The following guide demonstrates an example of creating a testable function, and then writing
unit tests for that function.

To see the complete diff of implementing this test, look at [commit a1943af](https://github.com/CalgaryToSpace/CTS-SAT-1-OBC-Firmware/commit/a1943aff0f0741de2d8b6e10b146a1d3c005cdf3).

0. Install the Codeium extension for VS Code, and sign in. This AI code generation tool will help
generate a lot of the boilerplate code you're about to encounter.

1. Come up with a testable function (called the Function-Under-Test, or FUT). For examples:

    ```c
    // File: transforms/byte_transforms.c
    
    #include "transforms/byte_transforms.h"

    uint32_t GEN_convert_big_endian_four_byte_array_to_uint32(uint8_t four_byte_array[]) {
        // recall: big endian means the Most Significant Bit (MSB) is at index 0 (left side)
        return (four_byte_array[0] << 24) | (four_byte_array[1] << 16) |
                (four_byte_array[2] << 8)  | four_byte_array[3];
    }
    ```

2. Create a new file in `Core/Src/unit_tests` called `test_bytes_transforms.c`. Create the unit test
function within that file:

    ```c
    // File: unit_tests/test_bytes_transforms.c

    #include "unit_tests/unit_test_helpers.h" // for all unit tests
    #include "transforms/byte_transforms.h" // for the Function-Under-Test

    uint8_t TEST_EXEC__GEN_convert_big_endian_four_byte_array_to_uint32() {
        // TODO: implement unit test later
        TEST_ASSERT_TRUE(1 == 1);

        return 0; // THIS LINE MUST BE INCLUDED to indicate the test passed
    }
    ```

3. Add the function prototype to the header file `Core/Inc/unit_tests/test_bytes_transforms.h`:

    ```c
    // File: unit_tests/test_bytes_transforms.h
    // ...
    uint8_t TEST_EXEC__GEN_convert_big_endian_four_byte_array_to_uint32();
    // ...
    ```

3. Add the test to the "inventory table" of tests in `Core/Src/unit_tests/unit_test_inventory.c`:

    ```c
    // File: unit_tests/unit_test_inventory.c
    #include "unit_tests/test_byte_transforms.h"

    const TEST_Definition_t TEST_definitions[] = {
        // ...
        // Add the following section:
        {
            .test_func = TEST_EXEC__GEN_convert_big_endian_four_byte_array_to_uint32,
            .test_file = "transforms/byte_transforms",
            .test_func_name = "GEN_convert_big_endian_four_byte_array_to_uint32"
        },
        // ...
    };

    ```

4. Flash the STM32, and run the `CTS1+run_all_unit_tests` command via the Debug UART line. See
that the test passes.

5. Go back to the `TEST_EXEC__GEN_convert_big_endian_four_byte_array_to_uint32` function, and implement
the test:

    ```c
    // File: unit_tests/test_bytes_transforms.c

    uint8_t TEST_EXEC__GEN_convert_big_endian_four_byte_array_to_uint32() {
        uint8_t test_array_1[4] = {0x12, 0x34, 0x56, 0x78};
        uint32_t expected_result_1 = 0x12345678;
        TEST_ASSERT_TRUE(GEN_convert_big_endian_four_byte_array_to_uint32(test_array_1) == expected_result_1);

        uint8_t test_array_2[4] = {0xFE, 0xDC, 0xBA, 0x98};
        uint32_t expected_result_2 = 4275878552; // 0xFEDCBA98
        TEST_ASSERT_TRUE(GEN_convert_big_endian_four_byte_array_to_uint32(test_array_2) == expected_result_2);

        return 0; // THIS LINE MUST BE INCLUDED to indicate the test passed
    }
    ```

6. Run the test again.

