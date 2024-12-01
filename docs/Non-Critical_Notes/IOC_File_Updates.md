# IOC File Updates

The IOC file is a file which defines the peripheral configuration. It is modified using the STM32CubeMX GUI tool.


## Updating the IOC File/Regenerating Boilerplate

Very few people will need to do this.

The IOC file must be updated using CubeMX instead of STM32CubeIDE now. Feel free to test IOC
file changes, but do not commit IOC file changes nor auto-generated code changes. Instead, once 
you've found the ideal configuration, request the changes to a Team Lead (who will make the change 
directly in the `main` branch).

Then, rebase off of the `main` branch, or merge `main` into your branch.

## Updating the IOC File in the `main` Branch

1. Open STM32CubeMX.
2. Edit the IOC file, as desired. Save.
3. Commit with just the "IOC file update" message, describing the changes (into a feature branch for just that IOC file update).
4. Click the "Generate Code" button.
6. **Quirky Step 1:** Revert linker script regeneration (discard changes using git), as we have changes in it.
8. Commit all this generated code, in a separate commit.
9. **Quirky Step 2:** Using the `meld` tool, you may have to copy changes from Makefile to `Makefile.GitHub.mk`.
10. Push the feature branch. PR to main. Squash and merge.
