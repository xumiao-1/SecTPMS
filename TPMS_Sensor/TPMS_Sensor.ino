#include "TPMS_Sensor.h"
#include <SPI.h>
//#include <EEPROM.h>
#include <RF22.h>
#include <mydef.h>
#include <my_util.h>
#include <crypto.h>
#include <cbc_mac.h>
#include <TPMSData.h>
#include <manchester.h>

// function prototype
//void transmit();
void sessionKeyUpdate();
void reply();
void rcvConfirm();
void updateKey();
void loadSensorID();
void loadECUID();
void loadMasterKey();
void loadAuthenKey();
/*
 Handy function to find the free ram on an arduino. Taken from the internet
 */
//int freeRam();
//void clearBuffers();
// add by Miao
void waitHails();
//bool checkMAC(uint8_t *msg, uint8_t len, uint8_t *key, uint8_t *mac);

// macro
#define IDX_SENSOR (0)

// add by Miao
// measure the delay
unsigned long start_time = 0, end_time = 0;

// send & recv message
void sendMsg(uint8_t *msg, uint8_t len);
uint8_t recvMsg(uint8_t *buff, uint8_t lenn);

// Singleton instance of the radio
RF22 rf22;

// TPMS messages
const uint8_t TPMS_MSG[][TPMS_PKT_LEN] = {
		{ 0x4A, 0x9E, 0x3A, 0x13, 0x8A, 0x75, 0x9E, 0x67, 0x40 },
		{ 0x4A, 0x9E, 0x3A, 0x14, 0x0A, 0xF5, 0x10, 0x61, 0x00 },
		{ 0x4A, 0x9E, 0x3A, 0x15, 0x8A, 0x75, 0x9D, 0x40, 0x40 },
		{ 0x4A, 0x9E, 0x3A, 0x16, 0x0A, 0xF5, 0x10, 0x6F, 0x00 },
		{ 0x4A, 0x9E, 0x3A, 0x17, 0x8A, 0x75, 0x9F, 0xE1, 0xC0 },
};

//unsigned long plain[2];
//unsigned long cipher[2];
//unsigned long message[8];
//unsigned char masterKey[10];
//unsigned char sessionKey[10];
//unsigned long IDs;
//unsigned long IDe;
//int sensorNumber;
//uint8_t buf[32];
//uint8_t len;
//const int NUM_SENSORS = 4;
//const int BLOCK_SIZE = 2;
//const int MSG_SIZE = 8;
//const int HAIL_WAIT_TIME = 1000;
//long /*r, */y, s;

//I plan to have a 256 bit long message in session key update case
//S||R||IDs||IDe||hash

void setup() {
	Serial.begin(115200);

	// init RF
	if (!rf22.init()) {
		Serial.println("RF22 init failed");
		while (true)
			;
	}

	Serial.println("I am a sensor");

	srandom(0);

	// load prior knowledge
	loadSensorID();
	loadECUID();
	loadMasterKey();
	loadAuthenKey();

	// session key update
	sessionKeyUpdate();
}

void loop() {
	uint8_t i = 0;

	while (1) {
		// TPMS message
		uint8_t data[TPMS_PKT_LEN], cipher[TPMS_PKT_LEN] = { 0 };
		for (uint8_t k = 0; k < TPMS_PKT_LEN; k++)
			data[k] = TPMS_MSG[i][k];

		Serial.println("TPMS message:");
		my_print_hex(data, TPMS_PKT_LEN);
		Serial.println();
		Serial.println();

		// encrypt using key
		unsigned long start_time = 0, end_time = 0;
#ifdef CRYPTO
		start_time = micros();
#if defined(KLEIN80)
		encrypt_klein80(data, TPMS_PKT_LEN - 1, k_s[IDX_SENSOR], cipher); // s: 212us, r: 360us
#elif defined(KATAN32)
		encrypt_katan32(data, TPMS_PKT_LEN - 1, k_s[IDX_SENSOR], 254, cipher);
#endif
		cipher[TPMS_PKT_LEN - 1] = data[TPMS_PKT_LEN - 1];
		end_time = micros();
#endif // CRYPTO
		Serial.println("Encrypted:");
		my_print_hex(cipher, TPMS_PKT_LEN);
		Serial.print("\t(took ");
		Serial.print(end_time - start_time);
		Serial.println("us)\r\n");

		// diff. manchester encoding (20 bytes)
		uint8_t encoded[ENCODED_MSG_LEN];
		// head
		encoded[0] = 0x03;
		encoded[1] = (cipher[0] >> 7) ? 0xF3 : 0xF2;
		// body
		encode_dmanchester(cipher, encoded + 2, TPMS_PKT_LEN, 1);
		shift_l(encoded + 2, 1, ENCODED_MSG_LEN - 2);
		Serial.println("Encoded (diff. Manchester):");
		my_print_hex(encoded, sizeof(encoded));
		Serial.println("\r\n\r\n\r\n");

		// send message
		sendMsg(encoded, sizeof(encoded));
//		rf22.send(encoded, sizeof(encoded));
//		rf22.waitPacketSent();

// next TPMS message
		i = (i + 1) % 5;
		delay(3000);
	}
}

//void transmit() {
//	delay(3000);
//
//	encrypt();
//
//	toBytes();
//
//	rf22.send(buf, len);
//	rf22.waitPacketSent();
//
//	cipher[0] = 0;
//	cipher[1] = 0;
//
//	Serial.println("Sent readings\n");
//}

void sessionKeyUpdate() {
	Serial.println("Waiting for ECU...");

	// wait for hails from ECU
	waitHails();

	// reply to ECU
	reply();

	// wait for ECU reply
	rcvConfirm();

	// update key
	updateKey();
	end_time = millis();

	Serial.print("Update confirmed.\r\nKs: ");
	my_print_hex(k_s[IDX_SENSOR], KEY_LEN);
	Serial.print("\t(took ");
	Serial.print(end_time - start_time);
	Serial.print("ms)\r\n");
}

void waitHails() {
	while (true) {
		//	receive hail messages
		uint8_t buff[MAX_MSG_LEN] = { 0 };
		if (sizeof(MSG1) == recvMsg(buff, MAX_MSG_LEN)) {
			start_time = millis(); // start session key update

			// construct MSG1
			my_memcpy((uint8_t*) &MSG1, buff, sizeof(MSG1));

			if (id_sensor[IDX_SENSOR] == MSG1.id_sensor && id_ecu == MSG1.id_ecu
					&& checkMAC((uint8_t*) &MSG1, sizeof(MSG1) - BLOCK_SIZE,
							k_a[IDX_SENSOR], MSG1.mac)) { // MSG1 for me
#ifdef MY_DEBUG
							Serial.print("Sensor ");
							Serial.print(id_sensor[IDX_SENSOR], HEX);
							Serial.print(" pairing to ECU ");
							Serial.println(id_ecu, HEX);
#endif
				break;
			}
		}
	}
}

void reply() {
//wait our turn to transmit - hail wait time includes a margin for ecu to get ready
//	delay(2 * HAIL_WAIT_TIME); //constant time per sensor, but sensors start
//							   //in staggered wait time increments - bc waiting
//							   //in rcvhail

	// construct MSG2
	getNextR(MSG2.S);
	my_memcpy(MSG2.R, MSG1.R, RAND_LEN);
	MSG2.id_ecu = id_ecu;
	MSG2.id_sensor = id_sensor[IDX_SENSOR];

	cbc_mac((uint8_t*) &MSG2, sizeof(MSG2) - BLOCK_SIZE, k_a[IDX_SENSOR],
			MSG2.mac);

	sendMsg((uint8_t*) &MSG2, sizeof(MSG2));
//	Serial.print("Reply sent: ");
//	my_print_hex((uint8_t*) &MSG2, sizeof(MSG2));
//	Serial.println();
}

void rcvConfirm() {
	while (true) {
		// receive MSG3
		uint8_t buff[MAX_MSG_LEN] = { 0 };
		if (sizeof(MSG3) == recvMsg(buff, MAX_MSG_LEN)) {
			// construct MSG3
			my_memcpy((uint8_t*) &MSG3, buff, sizeof(MSG3));

			if (id_sensor[IDX_SENSOR] == MSG3.id_sensor && id_ecu == MSG3.id_ecu
					&& checkMAC((uint8_t*) &MSG3, sizeof(MSG3) - BLOCK_SIZE,
							k_a[IDX_SENSOR], MSG3.mac)) { // MSG3 for me
				break;
			}
		}
	}

//	for (int i = 0; i < NUM_SENSORS; i++) {
//		rf22.waitAvailable();
//		/*if(!rf22.waitAvailableTimeout(HAIL_WAIT_TIME*8))
//		 {
//		 Serial.println("Timed out while waiting for final confirmation");
//		 return false;
//		 }*/
//		rcvMessage();
//		if (message[2] == id_sensor[IDX_SENSOR] && checkMessage(true))
//			return true;
//	}
//
//	return false;
}

//bool checkMAC(uint8_t *msg, uint8_t len, uint8_t *key, uint8_t *mac) {
//	uint8_t my_mac[BLOCK_SIZE];
//	cbc_mac(msg, len, key, my_mac);
//	return my_memcmp(mac, my_mac, BLOCK_SIZE);
//}
//boolean checkMessage(boolean final) {
//	if (final == false) {
//		//heard too many transmissions but none for this sensor. Strictly needed?
//		if (id_sensor[IDX_SENSOR] != message[3]) {
//			Serial.println("No transmission for this sensor received.");
//			sensorNumber = 1; //reset for if we go again later
//			return false;
//		}
//
//		if (r != message[1]) {
//			Serial.println(
//					"Improper R for hail message. Returning to avoid forgery or drain.");
//			return false;
//		}
//
//		if (id_ecu != message[2]) {
//			Serial.println(
//					"Message from unknown ECU. Returning to avoid forgery or drain.");
//			return false;
//		}
//	} else {
//		//heard too many transmissions but none for this sensor. Strictly needed?
//		if (id_sensor[IDX_SENSOR] != message[2]) {
//			Serial.println("No transmission for this sensor received.");
//			sensorNumber = 1; //reset for if we go again later
//			return false;
//		}
//
//		if (s != message[1]) {
//			Serial.println(
//					"Improper S for response. Returning to avoid forgery or drain.");
//			return false;
//		}
//
//		if (r != message[0]) {
//			Serial.println(
//					"Improper R for response. Returning to avoid forgery or drain.");
//			return false;
//		}
//
//		if (id_ecu != message[3]) {
//			Serial.println(
//					"Message from unknown ECU. Returning to avoid forgery or drain.");
//			return false;
//		}
//	}
//
//	decryptBlock();
//
//	if (message[0] != message[4] || message[1] != message[5]
//			|| message[2] != message[6] || message[3] != message[7]) {
//		Serial.println(
//				"Message inconsistent with signature. Returning to avoid forgery or drain.");
//		return false;
//	} else {
//		return true;
//	}
//}

void updateKey() {
	uint8_t output[16] = { 0 }; //128bit
#if defined(KLEIN80)
	encrypt_klein80((uint8_t*) &MSG3, sizeof(output), k_ll[IDX_SENSOR], output);
#elif defined(KATAN32)
	encrypt_katan32((uint8_t*) &MSG3, sizeof(output), k_ll[IDX_SENSOR], 254, output);
#endif
	my_memcpy(k_s[IDX_SENSOR], output, KEY_LEN);
}

void loadSensorID() {
	Serial.println("loading sensor id...");
	Serial.print(id_sensor[IDX_SENSOR], HEX);
	Serial.println();
}

void loadECUID() {
	Serial.println("loading saved ECU...");
	Serial.println(id_ecu, HEX);
}

void loadMasterKey() {
	Serial.println("loading long-lived key...");
	my_print_hex(k_ll[IDX_SENSOR], KEY_LEN);
	Serial.println();
}

// authentication key
void loadAuthenKey() {
	Serial.println("loading authentication key...");
	uint8_t input[16] = { 0 }, output[16] = { 0 };
#if defined(KLEIN80)
	encrypt_klein80(input, 16, k_ll[IDX_SENSOR], output);
#elif defined(KATAN32)
	encrypt_katan32(input, 16, k_ll[IDX_SENSOR], 254, output);
#endif
	my_memcpy(k_a[IDX_SENSOR], output, KEY_LEN);
	my_print_hex(k_a[IDX_SENSOR], KEY_LEN);
	Serial.println();
}

///*
// Handy function to find the free ram on an arduino. Taken from the internet
// */
//int freeRam() {
//	extern int __heap_start, *__brkval;
//	int v;
//	return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
//}

//void toLong() {
//	for (int i = 0; i < MSG_SIZE && i * 4 < len; i++) {
//		message[i] = (message[i] << 8) + buf[i * 4];
//		message[i] = (message[i] << 8) + buf[1 + i * 4];
//		message[i] = (message[i] << 8) + buf[2 + i * 4];
//		message[i] = (message[i] << 8) + buf[3 + i * 4];
//		//Serial.println(message[i], DEC);
//	}
//}
//
//void toBytes() {
//	int i = 0;
//
//	while (i * 4 < len && i < MSG_SIZE) {
//		buf[i * 4] = message[i] >> 24; //high
//		//Serial.println(buf[i*4], DEC);
//
//		buf[1 + i * 4] = message[i] >> 16;
//		//Serial.println(buf[1 + i*4], DEC);
//
//		buf[2 + i * 4] = message[i] >> 8;
//		//Serial.println(buf[2 + i*4], DEC);
//
//		buf[3 + i * 4] = message[i]; //low
//		//Serial.println(buf[3 + i*4], DEC);
//
//		i++;
//	}
//}

//void getNextR(uint8_t *r) {
//	long rd = 0;
//	uint8_t round = RAND_LEN / sizeof(long);
//	uint8_t left = RAND_LEN % sizeof(long);
//
//	for (uint8_t i = 0; i < round; i++) {
//		rd = random();
//		my_memcpy(r + i * sizeof(long), (uint8_t*) &rd, sizeof(long));
//	}
//	rd = random();
//	my_memcpy(r + RAND_LEN - left, (uint8_t*) &rd, left);
//}

//void clearBuffers() {
////	for (int i = 0; i < len; i++)
////		buf[i] = 0;
//
////	for (int i = 0; i < MAX_MSG_LEN; i++)
////		msg[i] = 0;
//	my_memset((uint8_t*) &MSG1, 0, sizeof(MSG1));
//	my_memset((uint8_t*) &MSG2, 0, sizeof(MSG2));
//	my_memset((uint8_t*) &MSG3, 0, sizeof(MSG3));
//}

// add by Miao
void sendMsg(uint8_t *msg, uint8_t len) {
	rf22.send(msg, len);
	rf22.waitPacketSent();
#ifdef MY_DEBUG
	Serial.println("*****sendMsg()*****");
	my_print_hex(msg, len);
	Serial.println("\r\n***************");
#endif
}

// return the size of the MSG I got
uint8_t recvMsg(uint8_t *buff, uint8_t lenn) {
	//	receive messages
	rf22.waitAvailable();
	rf22.recv(buff, &lenn); // receive MSG
#ifdef MY_DEBUG
			Serial.println("*****recvMsg()*****");
			my_print_hex((uint8_t*) &MSG1, sizeof(MSG1));
			Serial.println("\r\n***************");
#endif
	return lenn;
}
