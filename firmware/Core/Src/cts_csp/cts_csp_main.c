#include "cts_csp/cts_csp_main.h"

#include "csp/csp.h"

// Note: Can't use "csp/csp_init.h", because header is in the Src folder right now.
#include "csp_init.h"
#include "csp_conn.h"

#include "log/log.h"

#include "main.h"

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

int csp_i2c_driver_tx(void * driver_data, csp_i2c_frame_t * frame){ //TODO call HAL I2C TRANSMIT FUNCTION
    LOG_message(
        LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "transmit_i2c_message_function"
    );
    return 0; //TODO need better return value for later
}


csp_i2c_interface_data_t i2c_data = {
    .tx_func = csp_i2c_driver_tx
    };


struct csp_iface_s iface_instance = {
    .name = "eth0",                   //!< Name, max compare length is #CSP_IFLIST_NAME_MAX
    .interface_data =  &i2c_data,           //!< Interface data, only known/used by the interface layer, e.g. state information.
    .driver_data = NULL,              //!< Driver data, only known/used by the driver layer, e.g. device/channel references.
    .nexthop = csp_i2c_tx ,                  //!< Next hop (Tx) function
    .mtu = 255,                     //!< Maximum Transmission Unit of interface FOUND FROM PAGE 27 OF AX100_MANUAL
    .split_horizon_off = 0,           //!< Disable the route-loop prevention
    .tx = 0,                          //!< Successfully transmitted packets
    .rx = 0,                          //!< Successfully received packets
    .tx_error = 0,                    //!< Transmit errors (packets)
    .rx_error = 0,                    //!< Receive errors, e.g. too large message
    .drop = 0,                        //!< Dropped packets
    .autherr = 0,                     //!< Authentication errors (packets)
    .frame = 1,                       //!< Frame format errors (packets)
    .txbytes = 0,                     //!< Transmitted bytes
    .rxbytes = 0,                     //!< Received bytes
    .irq = 0,                         //!< Interrupts
    .next = NULL                      //!< Internal, interfaces are stored in a linked list
};

void CSP_init_for_cts1() {
    LOG_message(
        LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "Starting CSP_INIT_FOR_1()"
    );
    csp_init(&csp_conf_value_1);
    LOG_message(
        LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "COMPLETED CSP_INIT_FOR_1()"
    );
        LOG_message(
        LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "Starting i2c_add_interface()"
    );
    int return_add_interface = csp_i2c_add_interface(&iface_instance);
    
    LOG_message(
        LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "FINISHED i2c_add_interface(), with return of %d", return_add_interface
    );
}

uint8_t CSP_demo_1() {
    //TODO check csp_send function
    // ADD PRINT STATEMENTS THROUGHOUT CODE TO CHECK FOR I2C CALLS
    //TRY TO GO THROUGH EACH FUNCTION CALL IN THIS CODE
    // TO SEE WHICH ONES MIGHT CALL I2C
    // TODO: try csp_ping(...)
    
    // From AX100 User Manual:
    // TODO: try csp_sendto(CSP_PRIO_LOW, 10, 31, 0, CSP_O_NONE, beacon, 0);

    // LOG_message(
    //     LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
    //     "Starting CSP_demo_1()"
    // );

    // char demo_packet_1[] = "help";

    // csp_conn_t * conn = csp_conn_allocate(CONN_SERVER);

    // if (conn == NULL) {
    //     LOG_message(
    //         LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_ERROR, LOG_SINK_ALL, // FIXME: disable logging to UHF probably
    //         "Could not allocate connection for demo 1"
    //     );
    //     return 10;
    // }

    // csp_packet_t * packet = csp_buffer_get(256);


    // if (packet == NULL) {
    //     LOG_message(
    //         LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
    //         "Could not allocate packet for demo 1"
    //     );
    //     return 10;
    // }

    // // Copy into packet.
    // memcpy(packet->data, &demo_packet_1, sizeof(demo_packet_1));
    // packet->length = sizeof(demo_packet_1);

    // const int ret = csp_send(
    //     conn,
    //     packet,
    //     1000 // FIXME: timeout
    // );

    // if (ret != 1) {
    //     LOG_message(
    //         LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
    //         "Could not send packet for demo 1. Returned %d.",
    //         ret
    //     );
    //     csp_buffer_free(packet);
    //     return 20;
    // }

    // LOG_message(
    //     LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
    //     "Sent demo_1 packet. Returned %d.",
    //     ret // Is always 1 by now.
    // );

    // return 0;

    int8_t txBuffer[16];
    uint8_t rxBuffer[16];
    HAL_StatusTypeDef ret;

    /* Example Command: Set CSP Node Address to 5 */
    txBuffer[0] = 0x00;  // Address in AX100 parameter memory for 'csp_node'
    txBuffer[1] = 0x01;  // Value to set (CSP Node ID)


    const uint16_t AX100_I2C_ADDRESS = 0x05;
    
                 LOG_message(
             LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
             "First"
            );
    /* Transmit the command */
    ret = HAL_I2C_Master_Transmit(&hi2c2, AX100_I2C_ADDRESS << 1, txBuffer, 2, HAL_MAX_DELAY);
    if (ret != HAL_OK) {
 LOG_message(
             LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
             "Failed Transmit. Ret: %d",
             ret
            );
       return 1; 
    }
LOG_message(
             LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
             "Second"
            );
    /* Read back to verify */
    ret = HAL_I2C_Master_Receive(&hi2c2, AX100_I2C_ADDRESS << 1, rxBuffer, 2, HAL_MAX_DELAY);
LOG_message(
             LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
             "Third"
            );
    if (ret == HAL_OK) {
        // Successfully read data
        if (rxBuffer[1] == 0x05) {
            // Verification successful
             LOG_message(
             LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
             "Success, is 5. Returned %d. rxBuffer: %s",
             ret,
             rxBuffer
            );
        }
    } else {
         LOG_message(
             LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
             "Could not send packet for demo 1. Returned %d.",
             ret
         );
    }

    return 0;
}