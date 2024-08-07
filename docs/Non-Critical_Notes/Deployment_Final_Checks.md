# Deployment Final Checks

* The code should be audited for the following aspects, as a final check before the final flash.

## Automated Check Ideas (GitHub Actions)

1. Telecommand Execution (`TCMDEXEC_`) function names match their registrations in the table.
2. No malloc usage.
3. Are all `TEST_EXEC_` functions registered in the test table?
4. Are all `TCMDEXEC_` functions registered in the telecommand table?

## Manual Execution Checks

The following checks involve executing code and/or telecommands.

* Units tests pass.
* Python-based system tests/checks pass.


## Logging Checks

1. Are all logs to `LOG_message()`? Limited use of UART logging.
2. Are `LOG_message()` calls using the correct exclusions and levels?

## Other Checks

1. Check the changelog of libraries (LittleFS, CSP maybe) for bug fixes.
2. Are there any large VLA allocations? Can/should any be replaced with static allocations?
    * Maybe enable `-Wvla` or `-Wstack-usage`? Not sure what they are yet.
3. Maybe enable compiler optimizations?


## Management Checks

1. Are all the requirements met?
2. Are all GitHub Issues implemented and/or closed as not implemented.
3. Are all PRs merged?
4. Are all branches merged?
