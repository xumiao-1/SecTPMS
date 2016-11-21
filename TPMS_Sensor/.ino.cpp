//This is a automatic generated file
//Please do not modify this file
//If you touch this file your change will be overwritten during the next build
//This file has been generated on 2016-11-10 15:38:03

#include "Arduino.h"
#include "TPMS_Sensor.h"
#include <SPI.h>
#include <RF22.h>
#include <mydef.h>
#include <my_util.h>
#include <crypto.h>
#include <cbc_mac.h>
#include <TPMSData.h>
#include <manchester.h>
void setup() ;
void loop() ;
void sessionKeyUpdate() ;
void waitHails() ;
void reply() ;
void rcvConfirm() ;
void updateKey() ;
void loadSensorID() ;
void loadECUID() ;
void loadMasterKey() ;
void loadAuthenKey() ;
void sendMsg(uint8_t *msg, uint8_t len) ;
uint8_t recvMsg(uint8_t *buff, uint8_t lenn) ;

#include "TPMS_Sensor.ino"

