/*
 * gpsCommands.c
 *
 *  Created on: Jul 25, 2022
 *  Revided on : Decmeber 20, 2023 by MC
 *      Author: mswel, MC
 * Updated on 2024-04-23
 *  Author: Matjaz Cigler
 */

#include "../../Inc/GPS/gpsCommands.h"
#include <string.h>

// recommended command: log bestxyza ontime 1
// see page 557 in commands and logs for documentation
void gps_bestxyz(char *timeInterval)
{
	char command[50] = "";
	strcat(command, "log bestxyza ");
	strcat(command, timeInterval);
	strcat(command, "\n");
	sendGpsCommand(command);
	return;
}

// recommended command: log timea ontime 1
// see page 993 in commands and logs for documentation
void gps_time(char *timeInterval)
{
	char command[50] = "";
	strcat(command, "log timea ");
	strcat(command, timeInterval);
	strcat(command, "\n");
	sendGpsCommand(command);
	return;
}

// recommended command: log itdetectstatusa onchanged
// see page 684 in commands and logs for documentation
// documentation says should only by used with 'onchanged'trigger
void gps_itdetectstatus()
{
	char *command = "log itdetectstatusa onchanged";
	sendGpsCommand(command);
	strcat(command, "\n");
	return;
}

// recommended command: log rxstatusa onchanged
// see page 684 in commands and logs for documentation
// documentation doesnt say only 'onchanged'
// but it is in the recommended command
void gps_rxstatus()
{
	char *command = "log rxstatusa onchanged";
	strcat(command, "\n");
	sendGpsCommand(command);

	return;
}

// recommended command: log rxconfiga once
// see page 684 in commands and logs for documentation
void gps_rxconfig(char *timeInterval)
{
	char command[50] = "";
	strcat(command, "log rxconfiga ");
	strcat(command, timeInterval);
	strcat(command, "\n");
	sendGpsCommand(command);
	return;
}

//
//// CURENTLY NOT NEEDED COMMANDS
//
// void gps_antennaPower(int on){
//	char* command = "[COM1] ANTENNAPOWER ";
//	if(on == 1){
//		strcat("ON", command);
//	}
//	else if(on == 0){
//		strcat("OFF", command);
//	}
//	else
//		return;
//	sendGpsCommand(command);
//	return;
//}
//
// void gps_approxPosTimeout(int timeout){
//	char* command = "[COM1] APPROXPOSTIMEOUT ";
//	char temp[Buffer];
//	sprintf(temp, "%c", timeout);
//	strcat(temp, command);
//	sendGpsCommand(command);
//	return;
//}
//
// void gps_bdseCutoff(float angle){
//	char* command = "[COM1] BDSECUTOFF ";
//	char temp[Buffer];
//	sprintf(temp, sizeof(temp), "%f", angle);
//	strcat(temp, command);
//	sendGpsCommand(command);
//	return;
//}
//
// void gps_bestVelType(char* mode){
//	char* command = "[COM1] BESTVELTYPE ";
//	strcat(mode, command);
//	sendGpsCommand(command);
//	return;
//}
//
// void gps_dataDecodeSignal(char* signalType, int on){
//	char* command = "[COM1] DATADECODESIGNAL ";
//	strcat(signalType, command);
//	if(on == 1){
//		strcat(" enable", command);
//	}
//	else if(on == 0){
//		strcat(" disable", command);
//	}
//	else
//		return;
//	sendGpsCommand(command);
//	return;
//}
//
// void gps_dllTimeConst(char* signalType, float timeConst){
//	char* command = "DLLTIMECONST ";
//	strcar(signalType, command);
//	strcat(" ", command);
//	char temp[Buffer];
//	sprintf(temp, sizeof(temp), "%f", timeConst);
//	strcat(temp, command);
//	sendGpsCommand(command);
//	return;
//}
//
// void gps_dynamics(char* settings){
//	char* command = "DYNAMICS ";
//	strcar(settings, command);
//	sendGpsCommand(command);
//	return;
//}
//
// void gps_eCutoff(float angle){
//	char* command = "ECUTOFF ";
//	char temp[Buffer];
//	sprintf(temp, sizeof(temp), "%f", angle);
//	strcat(temp, command);
//	sendGpsCommand(command);
//	return;
//}
//
// void gps_elevationCutoff(char* constellation, float angle){
//	char* command = "ELEVATIONCUTOFF ";
//	strcar(constellation, command);
//	strcat(" ", command);
//	char temp[Buffer];
//	sprintf(temp, sizeof(temp), "%f", angle);
//	strcat(temp, command);
//	sendGpsCommand(command);
//	return;
//}
//
// void gps_fix(char* type, float param1, float param2, float param3){
//	char* command = "FIX ";
//	strcat(type, command);
//	if(strcomp(type, "HEIGHT")){
//		char temp[Buffer];
//		sprintf(temp, sizeof(temp), "%f", param1);
//		strcat(temp, command);
//	}
//	else if (strcom(type, "POSITION")){
//		char temp1[Buffer];
//		char temp2[Buffer];
//		char temp3[Buffer];
//		sprintf(temp1, sizeof(temp1), "%f", param1);
//		strcat(temp1, command);
//		strcat(" ", command);
//		sprintf(temp2, sizeof(temp2), "%f", param2);
//		strcat(temp2, command);
//		strcat(" ", command);
//		sprintf(temp3, sizeof(temp3), "%f", param2);
//		strcat(temp3, command);
//	}
//	sendGpsCommand(command);
//	return;
//}
//
// void gps_freSet(char* target){
//	char* command = "FRESET ";
//	strcat(target, command);
//	sendGpsCommand(command);
//	return;
//}
//
// void gps_geodeticDatum(char* theSwitch, char* name, float epsgCode, char* anchor, float semimajorAxis, float flattening){
//	char* command = " GEODETICDATUM ";
//	strcat(theSwitch, command);
//	strcat(" ", command);
//	strcat(name, command);
//	strcat(" ", command);
//	if(strcomp(theSwitch, "DELETE")){
//		sendGpsCommand(command);
//		return;
//	}
//	char temp1[Buffer];
//	char temp2[Buffer];
//	char temp3[Buffer];
//	sprintf(temp1, sizeof(temp1), "%f", epsgCode);
//	strcat(temp1, command);
//	strcat(" ", command);
//	strcat(anchor, command);
//	strcat(" ", command);
//	sprintf(temp2, sizeof(temp1), "%f", semimajorAxis);
//	strcat(temp2, command);
//	strcat(" ", command);
//	sprintf(temp3, sizeof(temp3), "%f", flattening);
//	strcat(temp3, command);
//	sendGpsCommand(command);
//	return;
//}
//
// void gps_itDetectConfig(char* rfPath){
//	char* command = "ITDETECTCONFIG ";
//	strcat(rfPath, command);
//	sendGpsCommand(command);
//	return;
//}
//
// void gps_log(char* port, char* message, char* trigger, float period, float offset, char* hold){
//	char* command = "LOG ";
//	char temp1[Buffer];
//	char temp2[Buffer];
//	strcat(port, command);
//	strcat(" ", command);
//	strcat(message, command);
//	strcat(" ", command);
//	strcat(trigger, command);
//	strcat(" ", command);
//	sprintf(temp1, sizeof(temp1), "%f", period);
//	sprintf(temp2, sizeof(temp1), "%f", offset);
//	strcat(temp1, command);
//	strcat(" ", command);
//	strcat(temp2, command);
//	strcat(" ", command);
//	strcat(hold, command);
//	sendGpsCommand(command);
//	return;
//}
//
// void gps_posTimeOut(float seconds){
//	char* command = "POSTIMEOUT ";
//	char temp[Buffer];
//	sprintf(temp, sizeof(temp), "%f", seconds);
//	strcat(temp, command);
//	sendGpsCommand(command);
//	return;
//}
//
// void gps_ppsControl(char* theSwitch, char* polarity, float period, float pulseWidth){
//	char* command = "PPSCONTROL ";
//	char temp1[Buffer];
//	char temp2[Buffer];
//	strcat(theSwitch, command);
//	strcat(" ", command);
//	strcat(polarity, command);
//	strcat(" ", command);
//	sprintf(temp1, sizeof(temp1), "%f", period);
//	sprintf(temp2, sizeof(temp2), "%f", pulseWidth);
//	strcat(temp1, command);
//	strcat(" ", command);
//	strcat(temp2, command);
//	sendGpsCommand(command);
//	return;
//}
//
// void gps_reset(float delay){
//	char* command = "RESET ";
//	char temp[Buffer];
//	sprintf(temp, sizeof(temp), "%f", delay);
//	strcat(temp, command);
//	sendGpsCommand(command);
//	return;
//}
//
// void gps_saveConfig(){
//	char* command = " SAVECONFIG";
//	sendGpsCommand(command);
//	return;
//}
//
// void gps_serialProtocol(char* port, char* protocol){
//	char* command = " SERIALPROTOCOL ";
//	strcat(port, command);
//	strcat(" ", command);
//	strcat(protocol, command);
//	sendGpsCommand(command);
//	return;
//}
//
// void gps_setApproxPos(float lat, float lon, float height){
//	char* command = "SETAPPROXPOS ";
//	char temp1[Buffer];
//	char temp2[Buffer];
//	char temp3[Buffer];
//	sprintf(temp1, sizeof(temp1), "%f", lat);
//	sprintf(temp2, sizeof(temp2), "%f", lon);
//	sprintf(temp3, sizeof(temp3), "%f", height);
//	strcat(temp1, command);
//	strcat(" ", command);
//	strcat(temp2, command);
//	strcat(" ", command);
//	strcat(temp3, command);
//	sendGpsCommand(command);
//	return;
//}
//
// void gps_setApproxTime(float week, float second){
//	char* command = "SETAPPROXTIME ";
//	char temp1[Buffer];
//	char temp2[Buffer];
//	sprintf(temp1, sizeof(temp1), "%f", week);
//	sprintf(temp2, sizeof(temp2), "%f", second);
//	strcat(temp1, command);
//	strcat(" ", command);
//	strcat(temp2, command);
//	sendGpsCommand(command);
//	return;
//}
//
// void gps_setionoType(char* model){
//	char* command = " SETIONOTYPE ";
//	strcat(model, command);
//	sendGpsCommand(command);
//	return;
//}
//
// void gps_statusConfig(char* type, char* word, float mask){
//	char* command = "STATUSCONFIG ";
//	char temp[Buffer];
//	strcat(type, command);
//	strcat(" ", command);
//	strcat(word, command);
//	strcat(" ", command);
//	sprintf(temp, sizeof(temp), "%f", mask);
//	strcat(mask, command);
//	sendGpsCommand(command);
//	return;
//}
//
// void gps_tiltCompensationControl(char* theSwitch){
//	char* command = "TILTCOMPENSATIONCONTROL ";
//	strcat(theSwitch, command);
//	sendGpsCommand(command);
//	return;
//}
