// Header-only, repaired EPS PIU housekeeping (0xA2/0xA4) reader for blobs.
//
// Why: In the core firmware (rc3), `EPS_struct_piu_housekeeping_data_eng_t` puts fields in a
// different order than the reply bytes (all channels together, then all conditioning chains, then
// the extended bitfields), but `EPS_pack_eps_result_piu_housekeeping_data_eng()` `memcpy`s the
// reply into it as-is. Everything from reply byte 92 onward lands in the wrong field: e.g., JSON
// channels 9-12 contain CC1-CC3 data, CH13-CH16 are never shown, and every conditioning chain
// entry contains bytes from CH24-CH31.
//
// This header has a struct in the ICD's wire order (ISIS.EPS2.ICD.SW.IVID.7, 0xA2), so a plain
// `memcpy` is correct, plus `_FIXED` copies of the firmware functions that use it.
//
// Usage:
//     #include "../blob_libs/eps_piu_fixed.h"
//     EPS_struct_piu_housekeeping_data_eng_FIXED_t piu;
//     if (EPS_CMD_get_piu_housekeeping_data_eng_FIXED(&piu) == 0) {
//         const EPS_vpid_eng_t *boom = EPS_piu_get_channel_vip_FIXED(&piu, EPS_CHANNEL_12V_BOOM);
//         const EPS_conditioning_channel_short_datatype_eng_t *cc1 = EPS_piu_get_conditioning_channel_FIXED(&piu, 0);
//     }
//
// Notes:
// 1. Everything is `static`, so each blob gets its own copy, and unused functions are dropped.
// 2. Names match the firmware's, with a `_FIXED` suffix, so they don't clash with the firmware's.
// 3. EPS_send_cmd_get_response(), EPS_vpid_eng_TO_json(), etc. resolve to the firmware's at link
//     time (--just-symbols).
// 4. The reply must be the 274-byte (with daughterboard) variant, same as the firmware.

#ifndef BLOB_LIBS_EPS_PIU_FIXED_H
#define BLOB_LIBS_EPS_PIU_FIXED_H

#include <stdint.h>
#include <stddef.h>

#include "eps_drivers/eps_types.h"
#include "eps_drivers/eps_types_to_json.h"
#include "eps_drivers/eps_internal_drivers.h"

extern void *memcpy(void *dest, const void *src, size_t len);
extern int snprintf(char *buf, unsigned int size, const char *fmt, ...);

#pragma pack(push, 1)

// Command Response: 0xA2: Get PIU Housekeeping Data (Eng)
// Command Response: 0xA4: Get PIU Housekeeping Data (Running Average, Eng)
// Fields are in the order they are received in, starting from reply byte 6 (after the reserved byte).
// Use EPS_piu_get_channel_vip_FIXED() and EPS_piu_get_conditioning_channel_FIXED() to index the
// channels/chains, as the wire format splits them into several groups.
typedef struct {
    int16_t voltage_internal_board_supply_mV;
    int16_t temperature_mcu_cC; // centiCelsius = 1e-2 degrees C

    EPS_vpid_eng_t vip_dist_input;
    EPS_vpid_eng_t vip_batt_input;
    uint16_t stat_ch_on_bitfield;
    uint16_t stat_ch_overcurrent_fault_bitfield;
    uint16_t battery_status_bitfield; // Table 3-18: Battery Pack Status
    int16_t battery_temp2_cC;
    int16_t battery_temp3_cC;

    int16_t vd0_voltage_mV;
    int16_t vd1_voltage_mV;
    int16_t vd2_voltage_mV;

    EPS_vpid_eng_t vip_ch00_to_ch08[9];
    EPS_conditioning_channel_short_datatype_eng_t conditioning_channel_cc1_to_cc3[3];

    // Daughterboard items below this line.
    EPS_vpid_eng_t vip_ch09_to_ch15[7];
    EPS_conditioning_channel_short_datatype_eng_t conditioning_channel_cc4_to_cc5[2];

    uint16_t stat_ch_ext_on_bitfield; // channels not present on our model
    uint16_t stat_ch_ext_overcurrent_fault_bitfield; // channels not present on our model

    EPS_vpid_eng_t vip_ch16_to_ch31[16];
} EPS_struct_piu_housekeeping_data_eng_FIXED_t;

#pragma pack(pop)

// Check each field against the ICD's reply offsets (which count from reply byte 0; the struct
// starts at reply byte 6).
#define EPS_PIU_FIXED_CHECK_OFFSET(field, icd_offset) \
    _Static_assert( \
        offsetof(EPS_struct_piu_housekeeping_data_eng_FIXED_t, field) == ((icd_offset) - 6), \
        "PIU field " #field " is not at ICD offset " #icd_offset \
    )
EPS_PIU_FIXED_CHECK_OFFSET(voltage_internal_board_supply_mV, 6);
EPS_PIU_FIXED_CHECK_OFFSET(temperature_mcu_cC, 8);
EPS_PIU_FIXED_CHECK_OFFSET(vip_dist_input, 10);
EPS_PIU_FIXED_CHECK_OFFSET(vip_batt_input, 16);
EPS_PIU_FIXED_CHECK_OFFSET(stat_ch_on_bitfield, 22);
EPS_PIU_FIXED_CHECK_OFFSET(stat_ch_overcurrent_fault_bitfield, 24);
EPS_PIU_FIXED_CHECK_OFFSET(battery_status_bitfield, 26);
EPS_PIU_FIXED_CHECK_OFFSET(battery_temp2_cC, 28);
EPS_PIU_FIXED_CHECK_OFFSET(battery_temp3_cC, 30);
EPS_PIU_FIXED_CHECK_OFFSET(vd0_voltage_mV, 32);
EPS_PIU_FIXED_CHECK_OFFSET(vd1_voltage_mV, 34);
EPS_PIU_FIXED_CHECK_OFFSET(vd2_voltage_mV, 36);
EPS_PIU_FIXED_CHECK_OFFSET(vip_ch00_to_ch08, 38);
EPS_PIU_FIXED_CHECK_OFFSET(vip_ch00_to_ch08[8], 86);
EPS_PIU_FIXED_CHECK_OFFSET(conditioning_channel_cc1_to_cc3, 92);
EPS_PIU_FIXED_CHECK_OFFSET(conditioning_channel_cc1_to_cc3[2], 108);
EPS_PIU_FIXED_CHECK_OFFSET(vip_ch09_to_ch15, 116);
EPS_PIU_FIXED_CHECK_OFFSET(vip_ch09_to_ch15[6], 152);
EPS_PIU_FIXED_CHECK_OFFSET(conditioning_channel_cc4_to_cc5, 158);
EPS_PIU_FIXED_CHECK_OFFSET(conditioning_channel_cc4_to_cc5[1], 166);
EPS_PIU_FIXED_CHECK_OFFSET(stat_ch_ext_on_bitfield, 174);
EPS_PIU_FIXED_CHECK_OFFSET(stat_ch_ext_overcurrent_fault_bitfield, 176);
EPS_PIU_FIXED_CHECK_OFFSET(vip_ch16_to_ch31, 178);
EPS_PIU_FIXED_CHECK_OFFSET(vip_ch16_to_ch31[15], 268);
_Static_assert(
    sizeof(EPS_struct_piu_housekeeping_data_eng_FIXED_t) == (274 - 6),
    "PIU struct must be the 274-byte reply, minus the 6-byte header"
);
#undef EPS_PIU_FIXED_CHECK_OFFSET


/// @brief Get the V/I/P of an output bus channel.
/// @param channel 0 to 31 (e.g., an EPS_CHANNEL_enum_t value).
/// @return Pointer into `data`, or NULL if `channel` is out of range.
static inline const EPS_vpid_eng_t *EPS_piu_get_channel_vip_FIXED(
    const EPS_struct_piu_housekeeping_data_eng_FIXED_t *data, uint8_t channel
) {
    if (channel <= 8) {
        return &data->vip_ch00_to_ch08[channel];
    }
    if (channel <= 15) {
        return &data->vip_ch09_to_ch15[channel - 9];
    }
    if (channel <= 31) {
        return &data->vip_ch16_to_ch31[channel - 16];
    }
    return NULL;
}

/// @brief Get a conditioning chain (MPPT) entry.
/// @param cc_idx 0 to 4, for CC1 to CC5 (same 0-based indexing as the PCU's
///     `conditioning_channel_info_each_channel`).
/// @return Pointer into `data`, or NULL if `cc_idx` is out of range.
static inline const EPS_conditioning_channel_short_datatype_eng_t *EPS_piu_get_conditioning_channel_FIXED(
    const EPS_struct_piu_housekeeping_data_eng_FIXED_t *data, uint8_t cc_idx
) {
    if (cc_idx <= 2) {
        return &data->conditioning_channel_cc1_to_cc3[cc_idx];
    }
    if (cc_idx <= 4) {
        return &data->conditioning_channel_cc4_to_cc5[cc_idx - 3];
    }
    return NULL;
}


/// @brief Sum the total solar panel input power (MPPT input V x I) of all 5 conditioning chains.
/// @return Total input of all conditioning chains in cW.
/// @note Unlike the firmware's EPS_calculate_total_pcu_power_input_cW(), which takes the PCU
///     housekeeping (only CC1-CC4), this includes CC5. Integer math (no float/double).
static inline int32_t EPS_calculate_total_pcu_power_input_cW_FIXED(
    const EPS_struct_piu_housekeeping_data_eng_FIXED_t *piu_data
) {
    int32_t total_cW = 0;

    for (uint8_t cc_idx = 0; cc_idx < 5; cc_idx++) {
        const EPS_conditioning_channel_short_datatype_eng_t *cc = EPS_piu_get_conditioning_channel_FIXED(piu_data, cc_idx);

        // Convert to cW. Fits in int32 (max 32767 x 32767).
        // Example: 5000 mV x 1000 mA = 5,000,000 ==> 5,000,000 / 10^4 = 500 cW = 5 W
        total_cW += ((int32_t)cc->volt_in_mppt_mV * (int32_t)cc->curr_in_mppt_mA) / 10000;
    }

    return total_cW;
}

/// @brief Sum the total solar panel output power (MPPT output V x I) of all 5 conditioning chains.
/// @return Total output of all conditioning chains in cW.
/// @note Unlike the firmware's EPS_calculate_total_pcu_power_output_cW(), which takes the PCU
///     housekeeping (only CC1-CC4), this includes CC5. Integer math (no float/double).
static inline int32_t EPS_calculate_total_pcu_power_output_cW_FIXED(
    const EPS_struct_piu_housekeeping_data_eng_FIXED_t *piu_data
) {
    int32_t total_cW = 0;

    for (uint8_t cc_idx = 0; cc_idx < 5; cc_idx++) {
        const EPS_conditioning_channel_short_datatype_eng_t *cc = EPS_piu_get_conditioning_channel_FIXED(piu_data, cc_idx);

        // Convert to cW. Fits in int32 (max 32767 x 32767).
        total_cW += ((int32_t)cc->volt_ou_mppt_mV * (int32_t)cc->curr_ou_mppt_mA) / 10000;
    }

    return total_cW;
}


static inline void EPS_pack_eps_result_piu_housekeeping_data_eng_FIXED(
    const uint8_t rx_buf[], EPS_struct_piu_housekeeping_data_eng_FIXED_t *result_dest
) {
    // Note: Has a reserved value at rx_buf[5].
    memcpy(result_dest, &rx_buf[6], sizeof(EPS_struct_piu_housekeeping_data_eng_FIXED_t));
}


/// @brief Send a PIU housekeeping command (0xA2 or 0xA4), and unpack the reply.
/// @return 0 on success, >0 on EPS comms error (from EPS_send_cmd_get_response()).
static inline uint8_t EPS_piu_housekeeping_cmd_FIXED(
    uint8_t command_code, EPS_struct_piu_housekeeping_data_eng_FIXED_t *result_dest
) {
    const uint8_t cmd_len = 4;
    const uint16_t rx_len = 274;

    uint8_t cmd_buf[4];
    uint8_t rx_buf[274];

    cmd_buf[0] = EPS_COMMAND_STID;
    cmd_buf[1] = EPS_COMMAND_IVID;
    cmd_buf[2] = command_code;
    cmd_buf[3] = EPS_COMMAND_BID;

    const uint8_t comms_err = EPS_send_cmd_get_response(cmd_buf, cmd_len, rx_buf, rx_len);
    if (comms_err != 0) {
        return comms_err;
    }

    EPS_pack_eps_result_piu_housekeeping_data_eng_FIXED(rx_buf, result_dest);
    return 0;
}

static inline uint8_t EPS_CMD_get_piu_housekeeping_data_eng_FIXED(
    EPS_struct_piu_housekeeping_data_eng_FIXED_t *result_dest
) {
    return EPS_piu_housekeeping_cmd_FIXED(0xA2, result_dest);
}

static inline uint8_t EPS_CMD_get_piu_housekeeping_data_run_avg_FIXED(
    EPS_struct_piu_housekeeping_data_eng_FIXED_t *result_dest
) {
    return EPS_piu_housekeeping_cmd_FIXED(0xA4, result_dest);
}


/// @brief Convert the PIU housekeeping data to JSON, with the same keys as the firmware's
///     EPS_struct_piu_housekeeping_data_eng_TO_json() (channels 0-16, and CC1-CC5).
/// @return 0 on success, >0 on error.
/// @note Writes straight into `json_output_str`.
static inline uint8_t EPS_struct_piu_housekeeping_data_eng_TO_json_FIXED(
    const EPS_struct_piu_housekeeping_data_eng_FIXED_t *data,
    char json_output_str[],
    uint16_t json_output_str_size
) {
    if (data == NULL || json_output_str == NULL || json_output_str_size < 10) {
        return 1; // Error: Invalid input
    }

    // Appends at `pos`, and returns the error code from the enclosing function on overflow.
    uint16_t pos = 0;
    #define EPS_PIU_FIXED_APPEND(err_code, ...) do { \
        const int ret = snprintf(&json_output_str[pos], json_output_str_size - pos, __VA_ARGS__); \
        if (ret < 0) return 6; \
        if (ret >= (json_output_str_size - pos)) return (err_code); \
        pos += (uint16_t)ret; \
    } while (0)
    #define EPS_PIU_FIXED_APPEND_VIP(err_code, vip) \
        EPS_PIU_FIXED_APPEND(err_code, "{\"mV\":%d,\"mA\":%d,\"cW\":%d}", \
            (vip)->voltage_mV, (vip)->current_mA, (vip)->power_cW)

    EPS_PIU_FIXED_APPEND(7,
        "{"
        "\"voltage_internal_board_supply_mV\":%d,"
        "\"temperature_mcu_cC\":%d,"
        "\"vip_dist_input\":",
        data->voltage_internal_board_supply_mV,
        data->temperature_mcu_cC
    );
    EPS_PIU_FIXED_APPEND_VIP(2, &data->vip_dist_input);
    EPS_PIU_FIXED_APPEND(7, ",\"vip_batt_input\":");
    EPS_PIU_FIXED_APPEND_VIP(3, &data->vip_batt_input);
    EPS_PIU_FIXED_APPEND(7,
        ","
        "\"stat_ch_on_bitfield\":%d,"
        "\"stat_ch_overcurrent_fault_bitfield\":%d,"
        "\"battery_status_bitfield\":%d,"
        "\"battery_temp2_cC\":%d,"
        "\"battery_temp3_cC\":%d,"
        "\"vd012_voltage_mV\":[%d,%d,%d],"
        "\"vip_each_channel\":[", // List-of-Dicts.
        data->stat_ch_on_bitfield,
        data->stat_ch_overcurrent_fault_bitfield,
        data->battery_status_bitfield,
        data->battery_temp2_cC,
        data->battery_temp3_cC,
        data->vd0_voltage_mV,
        data->vd1_voltage_mV,
        data->vd2_voltage_mV
    );

    // Note: Could do all 32 channels, but only using channels 0-16 on CTS-SAT-1.
    for (uint8_t ch = 0; ch <= EPS_MAX_ACTIVE_CHANNEL_NUMBER; ch++) {
        EPS_PIU_FIXED_APPEND_VIP(4, EPS_piu_get_channel_vip_FIXED(data, ch));
        if (ch < EPS_MAX_ACTIVE_CHANNEL_NUMBER) {
            EPS_PIU_FIXED_APPEND(4, ",");
        }
    }

    EPS_PIU_FIXED_APPEND(7, "],\"conditioning_channel_info_each_channel\":["); // List-of-Dicts.
    for (uint8_t i = 0; i < 5; i++) {
        const EPS_conditioning_channel_short_datatype_eng_t *cc = EPS_piu_get_conditioning_channel_FIXED(data, i);
        EPS_PIU_FIXED_APPEND(5,
            "{\"volt_in_mppt_mV\":%d,\"curr_in_mppt_mA\":%d,\"volt_ou_mppt_mV\":%d,\"curr_ou_mppt_mA\":%d}%s",
            cc->volt_in_mppt_mV,
            cc->curr_in_mppt_mA,
            cc->volt_ou_mppt_mV,
            cc->curr_ou_mppt_mA,
            (i < 4) ? "," : ""
        );
    }

    EPS_PIU_FIXED_APPEND(7,
        "],"
        "\"stat_ch_ext_on_bitfield\":%d,"
        "\"stat_ch_ext_overcurrent_fault_bitfield\":%d"
        "}",
        data->stat_ch_ext_on_bitfield,
        data->stat_ch_ext_overcurrent_fault_bitfield
    );

    #undef EPS_PIU_FIXED_APPEND_VIP
    #undef EPS_PIU_FIXED_APPEND
    return 0;
}

#endif // BLOB_LIBS_EPS_PIU_FIXED_H
