# Deployment Final Checks

* The code should be audited for the following aspects, as a final check before the final flash.

## Automated Check Ideas (GitHub Actions)

1. ✅ Telecommand Execution (`TCMDEXEC_`) function names match their registrations in the table.
    * Implemented in `check_tcmd_function_names_match_registration_names()`.
2. No malloc usage.
3. ✅ Are all `TEST_EXEC_` functions registered in the test table?
    * Implemented in `validate_test_exec_registration()`.
4. ✅ Are all `TCMDEXEC_` functions registered in the telecommand table?
    * Implemented in `validate_tcmdexec_registration()`.

✅ = Automated test now exists.

## Manual Execution Checks

The following checks involve executing code and/or telecommands.

* Units tests pass.
* Python-based system tests/checks pass.


## Logging Checks

1. Are all logs to `LOG_message()`? Limited use of UART logging.
2. Are `LOG_message()` calls using the correct exclusions and levels?
3. Are the log levels appropriate? Logs right before non-zero returns should generally be a warning/error level.
4. Are the log format strings consistent with the actual arguments? TODO: Find a build option to enable this.

## Other Checks

1. Check the changelog of libraries (LittleFS, CSP maybe) for bug fixes.
2. Are there any large VLA allocations? Can/should any be replaced with static allocations?
    * Maybe enable `-Wvla` or `-Wstack-usage`? Not sure what they are yet.
3. Maybe enable compiler optimizations?
4. Ensure that the Golden Copy is mapped in STM32L4R5ZITx_FLASH.ld file as such:
    ```
    GOLDEN_COPY (xrw) : ORIGIN = 0x8100000, LENGTH = 1024K
    ```
    This is the linker script. The length can change.
5. Ensure all tasks are registered in the `FREERTOS_task_info_struct_t FREERTOS_task_handles_array []` array. Consider a unit test for this check, maybe.


## Management Checks

1. Are all the requirements met?
2. Are all GitHub Issues implemented and/or closed as not implemented.
3. Are all PRs merged?
4. Are all branches merged?
5. Ctrl+Shift+F for `TODO` and `FIXME` in the codebase.
