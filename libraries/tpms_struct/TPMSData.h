/*
 * TPMSData.h
 *
 *  Created on: Jun 27, 2013
 *      Author: xum
 */

#ifndef TPMSDATA_H_
#define TPMSDATA_H_

#include <stdint.h>

#define TPMS_PKT_LEN (9)
#define ENCODED_MSG_LEN (20)


class TPMSData {
public:
	TPMSData();
	TPMSData(uint8_t data[]);
	virtual ~TPMSData();

	void setData(uint8_t data[]);
	uint8_t* getData();
	uint32_t getSensorID();
	double getPressure();
	uint32_t getTemperature();

private:
	uint8_t _data[TPMS_PKT_LEN];

	uint32_t _sid; // 28 bit
	double _pressure; // unit: Celsius
	uint32_t _temperature; // unit: kPa
};

#endif /* TPMSDATA_H_ */
