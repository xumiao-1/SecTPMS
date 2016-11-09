/*
 * TPMSData.cpp
 *
 *  Created on: Jun 27, 2013
 *      Author: xum
 */

#include "TPMSData.h"

TPMSData::TPMSData() {
	_sid = 0;
	_pressure = 0;
	_temperature = 0;
}

TPMSData::TPMSData(uint8_t data[]) {
	setData(data);
}

TPMSData::~TPMSData() {
	// TODO Auto-generated destructor stub
}

void TPMSData::setData(uint8_t data[]) {
	for (uint8_t i = 0; i < TPMS_PKT_LEN; i++) {
		_data[i] = data[i];
	}

	// fill other data field
	// _data[0...27]: sensor id
	// _data[34...49]: pressure
	// _data[50...57]: temperature
	_sid = (((uint32_t) _data[0] << 24) | ((uint32_t) _data[1] << 16)
			| ((uint32_t) _data[2] << 8) | ((uint32_t) _data[3])) >> 4;
	_pressure = (0.0098
			* ((((uint32_t) (_data[4] & 0x3F)) << 16
					| ((uint32_t) _data[5]) << 8 | (uint32_t) (_data[6] & 0xC0))
					>> 6) - 102.6226);
	_temperature =
			((((uint32_t) (_data[6] & 0x3F)) << 8 | ((uint32_t) _data[7])) >> 6)
					- 40;
}

uint8_t* TPMSData::getData() {
	return _data;
}

uint32_t TPMSData::getSensorID() {
	return _sid;
}

double TPMSData::getPressure() {
	return _pressure;
}

uint32_t TPMSData::getTemperature() {
	return _temperature;
}
