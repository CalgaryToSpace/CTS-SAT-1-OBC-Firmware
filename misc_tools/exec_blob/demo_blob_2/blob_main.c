// blob_main.c

typedef int (*snprintf_fn_t)(char *buf, unsigned int size, const char *fmt, ...);

// arm-none-eabi-nm -n firmware/build/debug/CTS-SAT-1_FW.elf | grep snprintf
// Below: Address from nm command above, but you MUST add 1 to make it odd.
#define FIRMWARE_SNPRINTF ((snprintf_fn_t) 0x0802d825UL)

__attribute__((used, section(".text.entry")))
unsigned char blob_main(
    const char *args_str,
    char *response_buf, unsigned short response_buf_len
) {
    FIRMWARE_SNPRINTF(
        response_buf, response_buf_len,
        "Hello world from the blob: %s!",
        args_str
    );
    return 0;
}
