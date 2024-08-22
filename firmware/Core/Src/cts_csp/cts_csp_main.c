#include "cts_csp/cts_csp_main.h"

#include "csp/csp.h"
#include "csp_init.h" // FIXME: why doesn't it like "csp/csp_init.h"? Because it's in the Src folder right now. 
#include "csp_conn.h"

#include "log/log.h"

// TODO: confirm global or not
csp_conf_t csp_conf_value_1 = {
    .address = 1,                       // Unique CSP address of the cubesat
    .hostname = "CTS-SAT-1",            // Hostname for identification
    .model = "CTS-SAT-1-v1.0",          // Model name
    .revision = "v1.0",                 // Hardware/Software revision

    // TODO: these are all default-ish, update
    .conn_max = 10,                     // Max number of simultaneous connections
    .conn_queue_length = 5,             // Max length of the connection queue
    .fifo_length = 10,                  // Length of the incoming message queue
    .port_max_bind = 15,                // Max/highest port for CSP binding
    .rdp_max_window = 5,                // Max RDP window size
    .buffers = 20,                      // Number of CSP buffers
    .buffer_data_size = 256,            // Size of data in each buffer
    .conn_dfl_so = CSP_SO_NONE          // Default connection options, e.g., no special options
};

void CSP_init_for_cts1() {

    csp_init(&csp_conf_value_1);

}

uint8_t CSP_demo_1() {
    // TODO: try csp_ping(...)
    
    // From AX100 User Manual:
    // TODO: try csp_sendto(CSP_PRIO_LOW, 10, 31, 0, CSP_O_NONE, beacon, 0);

    LOG_message(
        LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "Starting CSP_demo_1()"
    );

    char demo_packet_1[] = "Hello World!";

    csp_conn_t * conn = csp_conn_allocate(CONN_CLIENT);

    if (conn == NULL) {
        LOG_message(
            LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_ERROR, LOG_SINK_ALL, // FIXME: disable logging to UHF probably
            "Could not allocate connection for demo 1"
        );
        return 10;
    }

    csp_packet_t * packet = csp_buffer_get(256);


    if (packet == NULL) {
        LOG_message(
            LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Could not allocate packet for demo 1"
        );
        return 10;
    }

    // Copy into packet.
    memcpy(packet->data, &demo_packet_1, sizeof(demo_packet_1));
    packet->length = sizeof(demo_packet_1);

    const int ret = csp_send(
        conn,
        packet,
        1000 // FIXME: timeout
    );

    if (ret != 1) {
        LOG_message(
            LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Could not send packet for demo 1. Returned %d.",
            ret
        );
        csp_buffer_free(packet);
        return 20;
    }

    LOG_message(
        LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
        "Sent demo_1 packet. Returned %d.",
        ret // Is always 1 by now.
    );

    return 0;
}
