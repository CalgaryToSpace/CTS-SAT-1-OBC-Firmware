/*
 * gps_types.h
 *
 *
 *  Created on: Jul 7, 2022
 *  Updated: 2023-11-21
 *      Author: mswel, Matjaz Cigler
 *  Updated on 2024-04-23
 *  Author: Matjaz Cigler
 */
#ifndef __INCLUDE_GUARD__GPS_TYPES_H__
#define __INCLUDE_GUARD__GPS_TYPES_H__

#include <stdint.h>
typedef enum
{
	GPS_TIME_STATUS_UNKNOWN = 20,			  // Time validity is unknown
	GPS_TIME_STATUS_APPROXIMATE = 60,		  // Time is set approximately
	GPS_TIME_STATUS_COARSEADJUSTING = 80,	  // Time is approaching coarse precision
	GPS_TIME_STATUS_COARSE = 100,			  // This time is valid to coarse precision
	GPS_TIME_STATUS_COARSESTEERING = 120,	  // Time is coarse set and is being steered
	GPS_TIME_STATUS_FREEWHEELING = 130,		  // Position is lost and the range bias cannot be calculated
	GPS_TIME_STATUS_FINEADJUSTING = 140,	  // Time is adjusting to fine precision
	GPS_TIME_STATUS_FINE = 160,				  // Time has fine precision
	GPS_TIME_STATUS_FINEBACKUPSTEERING = 170, // Time is fine set and is being steered by the backup system
	GPS_TIME_STATUS_FINESTEERING = 180,		  // Time is fine set and is being steered
	GPS_TIME_STATUS_SATTIME = 200,			  // Time from satellite. Only used in logs containing satellite data such as ephemeris and almanac

} GPS_TIME_STATUS_enum_t; // Table 13: GPS Reference Time Status

// header
typedef struct __attribute__((__packed__))
{
	uint8_t sync_1;						// 0xAA Uchar offset: 0
	uint8_t sync_2;						// 0x44 Uchar offset: 1
	uint8_t sync_3;						// 0x12 Uchar offset: 2
	uint8_t header_length;				// header length Uchar offset: 3
	uint16_t msg_id;					// message id Ushort offset: 4
	int8_t msg_type;					// message type char offset: 6
	uint8_t prt_addr;					// port address Uchar offset: 7, table 5
	uint16_t msg_length;				// message length Ushort offset: 8
	uint16_t sequence;					// seqeunce Ushort offset: 10
	uint8_t idle_time;					// idle time Uchar offset: 12
	GPS_TIME_STATUS_enum_t time_status; // time status Enum offset: 13, Table 13: GPS Reference Time Status
	uint16_t week;						// GPS reference week number, offset: 14
	int32_t ms;							// ms GPSEC, from beginning of reference week, long offset: 16
	uint32_t rcvr_status;				// receiver status, Ulong, offset: 20, Table 199
	uint16_t reserved;					// reserved for internal use, Ushort, offset: 24
	uint16_t rcvr_version;				// 0-65535, receiver software build number, Ushort, offset: 25

} header_status;

// Messgae ID: 241, 00x00F1: BESTXYZ
typedef struct
{
	uint32_t psol_status;	  // P-sol status, Enum, see table 92 offset: H
	uint32_t pos_type;		  // Position type, Enum, see table 93 offset: H+4
	int64_t p_x;			  // Position coord X (m), Double, offset: H+8
	int64_t p_y;			  // Position coord Y (m), Double, offset: H+16
	int64_t p_z;			  // Position coord Z (m), Double, offset: H+24
	int32_t p_x_omega;		  // Std dev P-X (m), Float, offset: H+32
	int32_t p_y_omega;		  // Std dev P-Y (m), Float, offset: H+36
	int32_t p_z_omega;		  // Std dev P-Z (m), Float, offset: H+40
	uint32_t vsol_status;	  // V-sol status, Enum, see table 92 offset: H+44
	uint32_t vel_type;		  // velocity type, Enum, see table 93 offset: H+48
	int64_t v_x;			  // velocity coord X (m), Double, offset: H+52
	int64_t v_y;			  // velocity coord Y (m), Double, offset: H+60
	int64_t v_z;			  // velocity coord Z (m), Double, offset: H+68
	int32_t v_x_omega;		  // Std dev V-X (m), Float, offset: H+76
	int32_t v_y_omega;		  // Std dev V-Y (m), Float, offset: H+80
	int32_t v_z_omega;		  // Std dev V-Z (m), Float, offset: H+84
	int8_t stn_id[4];		  // Base station identification, Char[4], offset: H+88
	int32_t v_latency;		  // velocity time tag in seconds (subtract from time for improved results, Float, offset: H+92
	int32_t diff_age;		  // differential age in seconds, Float, offset: H+96
	int32_t sol_age;		  // Solution age in seconds, Float, offset: H+100
	uint8_t num_svs;		  // Number of satellites tracked, Uchar offset: H+104
	uint8_t num_soln_svs;	  // Number of satellites used in solution, Uchar offset: H+105
	uint8_t num_ggL1;		  // Number of satellites with L1/E1/B1 signals used in solution, Uchar offset: H+106
	uint8_t solnMultiSVs;	  // Number of satellites with multi-frequency signals used in solution, Uchar offset: H+107
	int8_t reserved2;		  // Reserved, Char, offset: H+108
	uint8_t ext_sol_stat;	  // Extended solution status, Hex, offset: H+109
	uint8_t gal_beidou_mask;  // Galileo and BeiDou signals used mask, Hex, offset: H+110
	uint8_t gps_glonass_mask; // GPS and GLONASS signals used mask, Hex, offset: H+111
	uint8_t crc[4];			  // 32-bit CRC, Hex 4 , offset: H+112

} __attribute__((__packed__)) gps_bestxyz_status;

// Message ID: 101, 00x0065: TIME
// page 960 OEM7 Commands and Logs Reference Manual
typedef struct
{
	uint32_t clock_status; // Clock model status, Table 106 Enum, offset: H
	int64_t offset;		   // Receiver clock offset GPS system time = GPS reference time - offset, Double, offset: H+4
	int64_t offset_std;	   // Receiver clock offset standard deviation, Double, offset: H+12
	int64_t utc_offset;	   // UTC offset,UTC time = GPS reference time - offset + UTC offset, Double, offset: H+20
	uint32_t utc_year;	   // UTC year, Ulong, offset: H+28
	uint8_t utc_month;	   // UTC month (0-12), UTC time unknown then 0, Uchar, offset: H+32
	uint8_t utc_day;	   // UTC day (0-31),UTC time unknown then 0, Uchar, offset: H+33
	uint8_t utc_hour;	   // UTC hour (0-23), Uchar, offset: H+34
	uint8_t utc_min;	   // UTC minute (0-59), Uchar, offset: H+35
	uint32_t utc_ms;	   // UTC millisecond (0-60999), max 60999 when leapsecond applied, Ulong, offset: H+36
	uint32_t utc_status;   // UTC status, 0 = Invalid, 1 = Valid, 2 = warning, Enum, offset: H+40
	uint8_t crc[4];		   // 32-bit CRC, Ulong, offset: H+44
} __attribute__((__packed__)) gps_time_status;

// Message ID: 2065, 00x0811: ITDETECTSTATUS
// page 656 OEM7 Commands and Logs Reference Manual
typedef struct
{
	uint32_t num_entries; // Number of interferences to follow, Ulong, offset: H
} __attribute__((__packed__)) gps_itdetectstatus_entry_count;

// entry number starts at 0
typedef struct
{
	uint32_t rf_path;			// RF path for this entry, 2= L1, 3 = L2, 5 = L5, See Table 131, Enum, offset: H+4 + (entry# * 36)
	uint32_t interference_type; // Interference detection type,0 = SPECTRUMANALYSIS, 1 = STATISTICALANALYSIS, Enum, offset: H+8+ (entry# * 36)
	int32_t parameter1;			// The first parameter of the interference: for SPECTRUMANALYSIS type, this is the center frequency in MHz; for STATISTICALANALYSIS type, this is reserved, Float, offset: H+12
	int32_t parameter2;			// The second parameter of the interference: for SPECTRUMANALYSIS type, this is the bandwidth in MHz; for STATISTICALANALYSIS type, this is reserved, Float, offset: H+16
	int32_t parameter3;			// The third parameter of the interference: for SPECTRUMANALYSIS type, this is the estimated power in dBm of the interference; for STATISTICALANALYSIS type, this is reserved, Float, offset: H+20
	int32_t parameter4;			// The fourth parameter of the interference: for SPECTRUMANALYSIS type, this is the highest estimated power spectrum density in dBmHz of the interference; for STATISTICALANALYSIS type, this is reserved, Float, offset: H+24
	uint32_t reserved1;			// Reserved, Ulong, offset: H+28+ (entry# * 36)
	uint32_t reserved2;			// Reserved, Ulong, offset: H+32+ (entry# * 36)
	uint32_t reserved3;			// Reserved, Ulong, offset: H+36+ (entry# * 36)
} __attribute__((__packed__)) itdetectstatus_entry;

typedef struct
{
	uint32_t crc[4]; // Number of interferences to follow, Ulong, offset: H+ 4 + (entries# * 36)
} __attribute__((__packed__)) itdetectstatus_entry_crc;

// Message ID: 93, 00x005D: RXSTATUS
// page 873 OEM7 Commands and Logs Reference Manual
// Need to look into if the num_stats is always 5??? if it is need to check this!!!!!!!!!!!!!!!!!!!
typedef struct
{
	uint32_t error;			 // Receiver error (see Table 197), Ulong, offset: H
	uint32_t num_stats;		 // Number of status codes, Ulong, offset: H+4
	uint32_t rxstat;		 // Receiver status word (see Table 198), Ulong, offset: H+8
	uint32_t rxstat_pri;	 // Receiver status priority mask (can be set using STATUSCONFIG see page 403, applies for all masks), Ulong, offset: H+12
	uint32_t rxstat_set;	 // Receiver status event set mask, Ulong, offset: H+16
	uint32_t rxstat_clear;	 // Receiver status event clear mask, Ulong, offset: H+20
	uint32_t aux1stat;		 // Auxiliary 1 status word (see Table 200), Ulong, offset: H+24
	uint32_t aux1stat_pri;	 // Auxiliary 1 status priority mask, Ulong, offset: H+28
	uint32_t aux1stat_set;	 // Auxiliary 1 status event set mask, Ulong, offset: H+32
	uint32_t aux1stat_clear; // Auxiliary 1 status event clear mask, Ulong, offset: H+36
	uint32_t aux2stat;		 // Auxiliary 2 status word (see Table 202), Ulong, offset: H+40
	uint32_t aux2stat_pri;	 // Auxiliary 2 status priority mask, Ulong, offset: H+44
	uint32_t aux2stat_set;	 // Auxiliary 2 status event set mask, Ulong, offset: H+48
	uint32_t aux2stat_clear; // Auxiliary 2 status event clear mask, Ulong, offset: H+52
	uint32_t aux3stat;		 // Auxiliary 3 status word (see Table 203), Ulong, offset: H+56
	uint32_t aux3stat_pri;	 // Auxiliary 3 status priority mask, Ulong, offset: H+60
	uint32_t aux3stat_set;	 // Auxiliary 3 status event set mask, Ulong, offset: H+64
	uint32_t aux3stat_clear; // Auxiliary 3 status event clear mask, Ulong, offset: H+68
	uint32_t aux4stat;		 // Auxiliary 4 status word (see Table 205), Ulong, offset: H+72
	uint32_t aux4stat_pri;	 // Auxiliary 4 status priority mask, Ulong, offset: H+76
	uint32_t aux4stat_set;	 // Auxiliary 4 status event set mask, Ulong, offset: H+80
	uint32_t aux4stat_clear; // Auxiliary 4 status event clear mask, Ulong, offset: H+84
	uint8_t crc[4];			 // 32-bit CRC, Ulong, offset: H+88
	// Note: The offset for CRC will depend on the number of entries.
} __attribute__((__packed__)) gps_rxstatus;

// Look into RXCONFIG because this is very variable not sure if necesarry to parse on the GPS

#endif /* __INCLUDE_GUARD__GPS_TYPES_H__ */
