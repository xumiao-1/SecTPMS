#include "ECU.h"
#include <SPI.h>
//#include <EEPROM.h>
#include <RF22.h>
#include <my_util.h>
#include <mydef.h>
#include <cbc_mac.h>
#include <manchester.h>
#include <TPMSData.h>

// function prototype
void sessionKeyUpdate();
void hailSensors(uint8_t expectedRs[][RAND_LEN]);
int checkReplies(uint8_t expectedRs[][RAND_LEN], uint8_t Ss[][RAND_LEN]);
//void sendConfirmations(int replies, uint8_t expectedRs[][RAND_LEN],
//		uint8_t Ss[][RAND_LEN]);
void sendConfirmation(uint8_t idx);
void updateKey(int index);
//boolean checkMessage(int i, uint8_t expectedRs[][RAND_LEN]);
void loadECUID();
void loadSensorIDs();
void loadKeys();
void loadAuthKeys();
/*
 Handy function to find the free ram on an arduino. Taken from the internet
 */
//int freeRam();
//void clearBuffers();
// add by Miao
// send & recv message
void sendMsg(uint8_t *msg, uint8_t len);
uint8_t recvMsg(uint8_t *buff, uint8_t lenn);

// Singleton instance of the radio
RF22 rf22;

// modified by Miao
// macro
//#define NUM_SENSORS (1)
//#define ID_ECU_LEN (2) // assume 16 bits long
//#define ID_S_LEN (4) // 28 bits, but use 32 for simplicity

//uint8_t id_ecu[ID_ECU_LEN]; // assume 16 bits
//uint8_t id_s[NUM_SENSORS][ID_S_LEN];

//unsigned long plain[2];
//unsigned long cipher[2];
//unsigned long message[8];
//uint8_t msg[MAX_MSG_LEN];
//unsigned char masterKeys[4][10];
//unsigned char sessionKeys[4][10];
//unsigned long IDs[4];
//unsigned long IDe;
//uint8_t buf[32];
//uint8_t len;
//const int BLOCK_SIZE = 2;
//const int MSG_SIZE = 8;
//const int NUM_SENSORS = 4;
const int HAIL_WAIT_TIME = 1000;
//long r, y;
//uint8_t r[RAND_LEN];

//Session key update messages are currently 256 bits long. Sensor data measurements 64 bits.

void setup() {
	Serial.begin(115200);

	if (!rf22.init()) {
		Serial.println("RF22 init failed");
		while (true)
			;
	}

	Serial.println("I am an ECU");
	//Defaults after init are 434.0MHz, 0.05MHz AFC pull-in, modulation FSK_Rb2_4Fd36
	//Radio will only send 50 bytes at a time

//	len = sizeof(buf);
//    IDe = 129576345UL; //homemade GUID
//	cipher[0] = 0;
//	cipher[1] = 0;

	//randomSeed(0);
	srandom(0);
//	r = 0;
//	y = 30;

	// load prior knowledge
	loadECUID();
	loadSensorIDs();
	loadKeys();
	loadAuthKeys();

	// session key update
	sessionKeyUpdate(); //to simulate on startup behavior
}

void loop() {
	while (true) {
		while (1) {
			// Wait for a message addressed to us
//			rf22.waitAvailable();

			// Should be a message for us now
			uint8_t recv_data[ENCODED_MSG_LEN];
//			;
//			uint8_t len, from, to;
//			if (rf22.recvfrom(recv_data, &len, &from, &to)) {
			if (recvMsg(recv_data, ENCODED_MSG_LEN) == ENCODED_MSG_LEN) {
				Serial.println("Got message:");
				my_print_hex(recv_data, ENCODED_MSG_LEN);
				Serial.println();
				Serial.println();

				// diff. machester decoding
				uint8_t data[TPMS_PKT_LEN], plaintext[TPMS_PKT_LEN] = { 0 };
				shift_r(recv_data + 2, 1, ENCODED_MSG_LEN - 2);
				set_bit(recv_data, 17, (recv_data[1] == 0xF3) ? 1 : 0);
				decode_dmanchester(recv_data + 2, data, ENCODED_MSG_LEN - 2, 1);
				Serial.println("Decoded (diff. Manchester):");
				my_print_hex(data, TPMS_PKT_LEN);
				Serial.println();
				Serial.println();

				// decrypt using TEA
				unsigned long start_time = 0, end_time = 0;
#ifdef CRYPTO
				start_time = micros();
#if defined(KLEIN80)
				decrypt_klein80(data, TPMS_PKT_LEN - 1, k_s[0], plaintext);
#elif defined(KATAN32)
				initKey(k_s[0], 254);
				decrypt_katan32(data, TPMS_PKT_LEN - 1, 254, plaintext);
#else
#error "No block cipher defined!"
#endif
				plaintext[TPMS_PKT_LEN - 1] = data[TPMS_PKT_LEN - 1];
				end_time = micros();
#endif // CRYPTO
				Serial.println("Decrypted:");
				my_print_hex(plaintext, TPMS_PKT_LEN);
				Serial.print("\t(took ");
				Serial.print(end_time - start_time);
				Serial.println("us)");

				// parse the data
				TPMSData tpms_data(plaintext);
				//				Serial.println("TPMS data");
				//				my_print_hex(tpms_data.getData(), TPMS_PKT_LEN);
				Serial.print("    Sensor ID: ");
				Serial.println(tpms_data.getSensorID(), HEX);
				Serial.print("    Pressure:  ");
				Serial.print(tpms_data.getPressure());
				Serial.println("kPa");
				Serial.print("    Temper.:   ");
				Serial.print(tpms_data.getTemperature());
				Serial.println("C");

				Serial.println("\r\n\r\n\r\n");
			}
//			}
		}
	}
}

void sessionKeyUpdate() {
	Serial.println("Initiating session key update...");

	uint8_t Ss[NUM_SENSORS][RAND_LEN];
	uint8_t expectedRs[NUM_SENSORS][RAND_LEN];

	// hail sensors
	hailSensors(expectedRs);

	// wait for sensors reply
	checkReplies(expectedRs, Ss);

//	// reply to sensors
//	sendConfirmations(replies, expectedRs, Ss);
}

//void hailSensors(unsigned long expectedRs[]) {
void hailSensors(uint8_t expectedRs[][RAND_LEN]) {

	for (int i = 0; i < NUM_SENSORS; i++) {
		Serial.print("Hailing sensor ");
		Serial.print(i, DEC);
		Serial.print(" with ID of ");
		Serial.println(id_sensor[i], HEX); //Serial.println(IDs[i], DEC);

		//expectedRs[i] = r;
		getNextR(expectedRs[i]); // planning on having a different r per sensor. Might change

		// construct MSG1
		my_memset(MSG1.P, 0, RAND_LEN);
		my_memcpy(MSG1.R, expectedRs[i], RAND_LEN);
		MSG1.id_ecu = id_ecu;
		MSG1.id_sensor = id_sensor[i];

		// mac
		cbc_mac((uint8_t*) &MSG1, sizeof(MSG1) - BLOCK_SIZE, k_a[i], MSG1.mac);
		sendMsg((uint8_t*) &MSG1, sizeof(MSG1));
	}

	Serial.println();
}

int checkReplies(uint8_t expectedRs[][RAND_LEN], uint8_t Ss[][RAND_LEN]) {
	int replies = 0;

	while (true) { // should have a timer here to break
		uint8_t idx_sensor = 0;

		uint8_t buff[MAX_MSG_LEN] = { 0 };
		if (sizeof(MSG2) == recvMsg(buff, MAX_MSG_LEN)) {
			// construct MSG2
			my_memcpy((uint8_t*) &MSG2, buff, sizeof(MSG2));

			if (id_ecu == MSG2.id_ecu) { // msg for me
				for (; idx_sensor < NUM_SENSORS; idx_sensor++) { // look for right sensor
					if (id_sensor[idx_sensor] == MSG2.id_sensor) {
						replies++;
						break; // find the sensor
					}
				}

				// a registered sensor, so check mac
				if (idx_sensor < NUM_SENSORS
						&& checkMAC((uint8_t*) &MSG2, sizeof(MSG2) - BLOCK_SIZE,
								k_a[idx_sensor], MSG2.mac)) { // check pass
						// so send confirmation to the sensor
					sendConfirmation(idx_sensor);

					if (replies >= NUM_SENSORS)
						break;
				}
			}
		}
	}

	return replies;
}

// must be called inside checkReplies
// to ensure valid MSG2
void sendConfirmation(uint8_t idx) {
	// construct MSG3
	my_memcpy(MSG3.R, MSG2.R, RAND_LEN);
	my_memcpy(MSG3.S, MSG2.S, RAND_LEN);
	MSG3.id_sensor = MSG2.id_sensor;
	MSG3.id_ecu = id_ecu;

	// send message
	cbc_mac((uint8_t*) &MSG3, sizeof(MSG3) - BLOCK_SIZE, k_a[idx], MSG3.mac);
	sendMsg((uint8_t*) &MSG3, sizeof(MSG3));

	Serial.print("Confirmation message sent to sensor ");
	Serial.println(MSG3.id_sensor, HEX);

	Serial.print("Update confirmed.\r\nKs: ");
	updateKey(idx); //goes at bottom because key is determined from final message order
}

void updateKey(int index) {
	uint8_t output[16] = { 0 }; //128bit
#if defined(KLEIN80)
	encrypt_klein80((uint8_t*) &MSG3, sizeof(output), k_ll[index], output);
#elif defined(KATAN32)
	initKey(k_ll[index], 254);
	encrypt_katan32((uint8_t*) &MSG3, sizeof(output), 254, output);
#else
#error "No block cipher defined!"
#endif
	my_memcpy(k_s[index], output, KEY_LEN);

	my_print_hex(k_s[index], KEY_LEN);
	Serial.println();
}

void loadECUID() {
	Serial.println("loading saved ECU...");
	Serial.println(id_ecu, HEX);
}

void loadSensorIDs() {
	Serial.println("loading registered sensor(s)...");
	for (uint8_t j = 0; j < NUM_SENSORS; j++) {
		Serial.println(id_sensor[j], HEX);
	}
}

void loadKeys() {
//    int readIndex;
	Serial.println("loading respective master key(s)...");
	for (int i = 0; i < NUM_SENSORS; i++) {
		Serial.print("K");
		Serial.print(i, DEC);
		Serial.print(": ");
		my_print_hex(k_ll[i], KEY_LEN);
		Serial.println();
	}
}

void loadAuthKeys() {
	Serial.println("loading respective authentication key(s)...");
	for (int i = 0; i < NUM_SENSORS; i++) {
		uint8_t input[16] = { 0 }, output[16] = { 0 };

#if defined(KLEIN80)
		encrypt_klein80(input, 16, k_ll[i], output);
#elif defined(KATAN32)
		initKey(k_ll[i], 254);
		encrypt_katan32(input, 16, 254, output);
#else
#error "No block cipher defined!"
#endif
		my_memcpy(k_a[i], output, KEY_LEN);

		Serial.print("K");
		Serial.print(i, DEC);
		Serial.print(": ");
		my_print_hex(k_a[i], KEY_LEN);
		Serial.println();
	}
}

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

///*
// Handy function to find the free ram on an arduino. Taken from the internet
// */
//int freeRam() {
//	extern int __heap_start, *__brkval;
//	int v;
//	return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
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
//void sendConfirmations(int replies, uint8_t expectedRs[][RAND_LEN],
//		uint8_t Ss[][RAND_LEN]) {
//	for (int i = 0; i < replies; i++) {
//		// construct MSG3
//		my_memcpy(MSG3.R, expectedRs[i], RAND_LEN);
//		my_memcpy(MSG3.S, Ss[i], RAND_LEN);
//		MSG3.id_sensor = id_sensor[i];
//		MSG3.id_ecu = id_ecu;
//
////		encryptBlock();
//
//		// send message
//		cbc_mac((uint8_t*) &MSG3, sizeof(MSG3) - BLOCK_SIZE, k_a[i], MSG3.mac);
//		sendMsg((uint8_t*) &MSG3, sizeof(MSG3));
//
//		Serial.print("Confirmation message sent to sensor ");
//		Serial.println(i, DEC);
//
//		Serial.println("Update confirmed. Ks: ");
//		updateKey(i); //goes at bottom because key is determined from final message order
//	}
//}
//int checkReplies(unsigned long expectedRs[], unsigned long Ss[]) {
//	int replies = 0;
//
//	for (int i = 0; i < NUM_SENSORS; i++) {
//		if (!rf22.waitAvailableTimeout(HAIL_WAIT_TIME * 20)) { //roughly the whole sending time
//			//should work bc staggered whole
//			Serial.print("No transmission from sensor "); //sending time replies?
//			Serial.println(i, DEC);
//			continue;
//		}
//		uint8_t buff[MAX_MSG_LEN] = { 0 };
//		if (sizeof(MSG2) == recvMsg(buff, MAX_MSG_LEN)) {
//			// construct MSG2
//			my_memcpy((uint8_t*) &MSG2, buff, sizeof(MSG2));
//		}
//		/*Serial.println(r, DEC);
//		 Serial.println(message[0], DEC);
//		 Serial.println(message[1], DEC);
//		 Serial.println(message[2], DEC);
//		 Serial.println(message[3], DEC);*/
//
//		if (!checkMessage(i, expectedRs))
//			continue;
//		else {
//			Serial.print("Proper reply received from sensor ");
//			replies++;
//			Serial.println(i, DEC);
//		}
//
////		Ss[i] = message[0]; //Note: sequence S?
//		my_memcpy(Ss[i], MSG2.S, RAND_LEN);
//	}
//
//	Serial.println();
//	return replies;
//	}
