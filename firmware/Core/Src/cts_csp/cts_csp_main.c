#include "main.h"
#include "cts_csp/cts_csp_main.h"

#include "csp/csp.h"
#include "stm32l4xx_hal_i2c.h"
#include "stm32l4xx_hal_def.h"
// Note: Can't use "csp/csp_init.h", because header is in the Src folder right now.
#include "csp_init.h"
#include "csp_conn.h"

#include "log/log.h"

#include "csp/interfaces/csp_if_i2c.h"


//extern I2C_HandleTypeDef hi2c1; //TODO: Is I2C3 the right bus?
const uint8_t AX100_I2C_ADDR = 0x05 << 1; 
const uint32_t timeout_mss = 3000; //TODO: how much? in milliseconds for i2c transmit

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


csp_i2c_interface_data_t csp_cts_i2c_interface_data = {
    .tx_func = CSP_i2c_driver_tx,       // I2C transmit function
};

struct csp_iface_s csp_cts_i2c_interface = {
    .name = "cts_i2c_1",             //!< Name, max compare length is #CSP_IFLIST_NAME_MAX
    .interface_data = &csp_cts_i2c_interface_data,          //!< Interface data, only known/used by the interface layer, e.g. state information.
    .driver_data = NULL,             //!< Driver data, only known/used by the driver layer, e.g. device/channel references.
    .nexthop = csp_i2c_tx,           //!< Next hop (Tx) function
    .mtu = 255,                     //!< Maximum Transmission Unit of interface // Found in AX100 User Manual for I2C.
    .split_horizon_off = 0,          //!< Disable the route-loop prevention
    
    // Initialize all the following event counters to 0:
    .tx = 0,                       //!< Successfully transmitted packets
    .rx = 0,                       //!< Successfully received packets
    .tx_error = 0,                   //!< Transmit errors (packets)
    .rx_error = 0,                   //!< Receive errors, e.g. too large message
    .drop = 0,                       //!< Dropped packets
    .autherr = 0,                    //!< Authentication errors (packets)
    .frame = 0,                      //!< Frame format errors (packets)
    .txbytes = 0,                 //!< Transmitted bytes
    .rxbytes = 0,                 //!< Received bytes
    .irq = 0,                       //!< Interrupts // TODO: Check value.
    .next = NULL                     //!< Internal, interfaces are stored in a linked list
};

void CSP_init_for_cts1() {
    LOG_message(
        LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_DEBUG, LOG_all_sinks_except(LOG_SINK_UHF_RADIO),
        "Starting CSP_init_for_cts1()"
    );

    const int ret_init = csp_init(&csp_conf_value_1);

    // TODO: If ret_init is not 0, log an error message.

    LOG_message(
        LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_DEBUG, LOG_all_sinks_except(LOG_SINK_UHF_RADIO),
        "Done csp_init(...), with return value: %d. Starting csp_i2c_add_interface(...).",
        ret_init
    );

    const int ret_add_iface = csp_i2c_add_interface(
        &csp_cts_i2c_interface
    );

    LOG_message(
        LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_DEBUG, LOG_all_sinks_except(LOG_SINK_UHF_RADIO),
        "Done csp_i2c_add_interface(...), with return value: %d.",
        ret_add_iface
    );

    //setting up the route table
    //csp_route_table_add(10, 31, 0, &csp_cts_i2c_interface);

    // TODO: If ret_add_iface is not 0, log an error message.
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

    /*
    * 10 = destination address of ground station
    * 5 = net mask  
    * &csp_cts_i2c_interface = interface to route to
    * 5 = via, the next hop address
    */
    csp_rtable_set(
        10, // destination addr
        5,  // net mask. & this with the destinations address to get the final address which csp will use to route
        &csp_cts_i2c_interface,// interface to route to
        5 // next hop  address
    );

    
    //csp_conn_t * conn = csp_conn_allocate(CONN_CLIENT);
    csp_conn_t * conn = csp_connect(CSP_PRIO_LOW, 10, 35, 0, CSP_O_NONE);
    if (conn == NULL) {
        LOG_message(
            LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Could not open connection for demo 1"
        );
        return 10;
    }
    



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
    
    //int32_t ping_ret = csp_ping(5, timeout_mss, packet->length, CSP_O_NONE);
    // if(ping_ret < 0) {
    //     LOG_message(
    //         LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
    //         "Could not send ping for demo 1"
    //     );
    //     return 20;
    // }
    // else {
    //     LOG_message(
    //         LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
    //         "Success pinging!!!!!"
    //     );
    // }

    
    const int ret = csp_send(
        conn,
        packet,
        1000 // FIXME: timeout
    );

    // csp_packet_t * ret_packet = csp_read(conn, 1000);


    // if (ret_packet != NULL) {
    //     LOG_message(
    //         LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
    //         "Received packet from demo 1: %s", ret_packet->data + 4
    //     );
    //     csp_buffer_free(ret_packet);
    // }
    // else {
    //     LOG_message(
    //         LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
    //         "ERROR: Did not receive packet from demo 1"
    //     );
    // }

    if (ret != 1) {
        LOG_message(
            LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Could not send packet for demo 1. csp_send returned %d.",
            ret
        );
        csp_buffer_free(packet);
        return 20;
    }

    LOG_message(
        LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
        "Sent demo_1 packet. csp_send returned Success."
    );


 
    return 0;
}


/// @brief Driver function for CSP to send data via I2C.
/// @param driver_data Byte array to send. Is the main data of the frame. //TODO: I dont think this is right
/// @param frame Info about the frame, including the `drive_data`'s length.
/// @return CSP error code, to be interpreted by CSP.
/// @note This function gets called internally by CSP, and is a function passed to CSP during config.
int CSP_i2c_driver_tx(void * driver_data, csp_i2c_frame_t * frame) {
    // Type of Function [Pointer]: csp_i2c_driver_tx_t
    // typedef int (*csp_i2c_driver_tx_t)(void * driver_data, csp_i2c_frame_t * frame);


    LOG_message(
        LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_DEBUG, LOG_all_sinks_except(LOG_SINK_UHF_RADIO),
        "Starting CSP_i2c_driver_tx(...). Frame Info: "
        "Padding: [0x%02X, 0x%02X, 0x%02X], "
        "Retries: %u, "
        "Reserved: %lu, "
        "Dest: %u, "
        "Len Rx: %u, "
        "Len: %u",
        (uint16_t)frame->padding[0], (uint16_t)frame->padding[1], (uint16_t)frame->padding[2],
        frame->retries,
        frame->reserved,
        frame->dest,
        frame->len_rx,
        frame->len
    );
    //uint8_t * data = (uint8_t *) frame->data;
    for (int i = 0; i < frame->len; i++) {
        LOG_message(
            LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_DEBUG, LOG_all_sinks_except(LOG_SINK_UHF_RADIO),
            "frame->data[%d]: 0x%02X",
            i,  frame->data[i]
        );
    }
    // LOG_message(
    //     LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_DEBUG, LOG_all_sinks_except(LOG_SINK_UHF_RADIO),
    //     "frame contents: %s", data
    // );

    // TODO: Hex print the driver_data
    
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(
        &hi2c1, 
        AX100_I2C_ADDR, 
        frame->data, //TODO: try just sending frame->data
        frame->len ,
        timeout_mss);
    if (status != HAL_OK) {
        LOG_message(
            LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_ERROR, LOG_all_sinks_except(LOG_SINK_UHF_RADIO),
            "I2C transmit error: %d", status
        );
        return CSP_ERR_TX;
    }

    // const size_t CSP_FRAME_HEADER_SIZE = sizeof(csp_i2c_frame_t);
    // HAL_StatusTypeDef tx_status = HAL_I2C_Mem_Write(
        // &hi2c1, AX100_I2C_ADDR, 0x00, 1, (uint8_t*)(&frame), CSP_FRAME_HEADER_SIZE + frame->len, timeout_mss);
    // uint8_t buffer[30];
    // status = HAL_I2C_Slave_Receive(&hi2c1, buffer, CSP_FRAME_HEADER_SIZE + frame->len, 60000);
    // if (status != HAL_OK) {
    //     LOG_message(
    //         LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_ERROR, LOG_all_sinks_except(LOG_SINK_UHF_RADIO),
    //         "I2C receive error: %d", status
    //     );
    //     return CSP_ERR_TX;
    // }
    // LOG_message(
    //     LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_DEBUG, LOG_all_sinks_except(LOG_SINK_UHF_RADIO),
    //     "Done with receive(...)."
    // );

    // for (int i = 0; i < 20; i++) {
    //     LOG_message(
    //         LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_DEBUG, LOG_all_sinks_except(LOG_SINK_UHF_RADIO),
    //         "0x%02X", buffer[i]
    //     );
        
    // }


    return CSP_ERR_NONE;
}
