/*
 * gpsCommands.h
 *
 *  Created on: Jul 25, 2022
 *      Author: mswel
 *  Updated on 2024-04-23
 *  Author: Matjaz Cigler
 */

#ifndef INC_GPSCOMMANDS_H_
#define INC_GPSCOMMANDS_H_

#include "main.h"
#include "send.h"

enum bufferSize{
	Buffer = 32,
};


void gps_bestxyz(char*);

//void gps_time();
//
//void gps_itdetectstatus();
//
//void gps_rxstatus();
//
//void gps_rxconfig();
//
//
//// CURENTLY NOT NEEDED COMMANDS
//
//void gps_antennaPower(int on);
//
//void gps_approxPosTimeout(int timeout);
//
//void gps_bdseCutoff(float angle);
//
//void gps_bestVelType(char* mode);
//
//void gps_dataDecodeSignal(char* signalType, int on);
//
//void gps_dllTimeConst(char* signalType, float timeConst);
//
//void gps_dynamics(char* settings);
//
//void gps_eCutoff(float angle);
//
//void gps_elevationCutoff(char* constellation, float angle);
//
//void gps_fix(char* type, float param1, float param2, float param3);
//
//void gps_freSet(char* target);
//
//void gps_geodeticDatum(char* theSwitch, char* name, float epsgCode, char* anchor, float semimajorAxis, float flattening);
//
//void gps_itDetectConfig(char* rfPath);
//
//void gps_log(char* port, char* message, char* trigger, float period, float offset, char* hold);
//
//void gps_posTimeOut(float seconds);
//
//void gps_ppsControl(char* theSwitch, char* polarity, float period, float pulseWidth);
//
//void gps_reset(float delay);
//
//void gps_saveConfig();
//
//void gps_serialProtocol(char* port, char* protocol);
//
//void gps_setApproxPos(float lat, float lon, float height);
//
//void gps_setApproxTime(float week, float second);
//
//void gps_setionoType(char* model);
//
//void gps_statusConfig(char* type, char* word, float mask);
//
//void gps_tiltCompensationControl(char* theSwitch);



#endif /* INC_GPSCOMMANDS_H_ */
