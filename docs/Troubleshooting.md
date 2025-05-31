# Troubleshooting

## Hard Faults

Common causes of hard faults include:

* **Null or invalid pointer deference:** Accessing memory in an illegal location (e.g., array overrun).
* **Stack Overflow:** Using more stack memory than you're allowed too.
    * Potential fix: Increase the FreeRTOS allocated stack size in `main.c`
