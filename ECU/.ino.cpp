//This is a automatic generated file
//Please do not modify this file
//If you touch this file your change will be overwritten during the next build
//This file has been generated on 2016-11-04 00:15:57

#include "Arduino.h"
#include "ECU.h"
#include <SPI.h>
#include <RF22.h>
#include <my_util.h>
#include <mydef.h>
#include <cbc_mac.h>
#include <manchester.h>
#include <TPMSData.h>
void setup() ;
void loop() ;
void sessionKeyUpdate() ;
void hailSensors(uint8_t expectedRs[][RAND_LEN]) ;
int checkReplies(uint8_t expectedRs[][RAND_LEN], uint8_t Ss[][RAND_LEN]) ;
void sendConfirmation(uint8_t idx) ;
void updateKey(int index) ;
void loadECUID() ;
void loadSensorIDs() ;
void loadKeys() ;
void loadAuthKeys() ;
void sendMsg(uint8_t *msg, uint8_t len) ;
uint8_t recvMsg(uint8_t *buff, uint8_t lenn) ;

#include "ECU.ino"

