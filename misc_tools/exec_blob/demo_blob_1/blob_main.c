// blob_main.c

__attribute__((used, section(".text.entry")))
unsigned char blob_main(
    const char *args_str,
    char *response_buf, unsigned short response_buf_len
) {
    // Write out "Hello world from blob\0", one byte at a time.
    response_buf[0] = 'H';
    response_buf[1] = 'e';
    response_buf[2] = 'l';
    response_buf[3] = 'l';
    response_buf[4] = 'o';
    response_buf[5] = ' ';
    response_buf[6] = 'w';
    response_buf[7] = 'o';
    response_buf[8] = 'r';
    response_buf[9] = 'l';
    response_buf[10] = 'd';
    response_buf[11] = ' ';
    response_buf[12] = 'f';
    response_buf[13] = 'r';
    response_buf[14] = 'o';
    response_buf[15] = 'm';
    response_buf[16] = ' ';
    response_buf[17] = 'b';
    response_buf[18] = 'l';
    response_buf[19] = 'o';
    response_buf[20] = 'b';
    response_buf[21] = '\0';

    return 0;
}
