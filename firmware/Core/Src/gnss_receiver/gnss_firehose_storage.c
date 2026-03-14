// GNSS Firehose Storage
// This module is responsible for storing GNSS data received at any point to a file.
// It is in contrast with the nominal "command mode" operation, where the GNSS receiver only
// replies to commands sent by the OBC (e.g., `log bestxyza once`). In "firehose mode", the
// OBC stores all data received from the GNSS receiver to a file; it expects that the GNSS
// receiver is configured in a `log bestxyza ontime 100` mode or similar.

#include "gnss_receiver/gnss_firehose_storage.h"
#include "uart_handler/uart_handler.h"

