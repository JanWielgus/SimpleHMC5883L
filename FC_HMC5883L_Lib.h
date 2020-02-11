// FC_HMC5883L_Lib.h
/*
    Created:	06/06/2019
    Author:     Jan Wielgus
	
	Parts of that program are based on the YMFC-32 made by Joop Brokking
	Link: http://www.brokking.net/YMFC-32_auto_downloads.html
*/


#ifndef _FC_HMC5883L_LIB_h
#define _FC_HMC5883L_LIB_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <Wire.h>

// !!! Comment if you don't want to debug
#define SERIAL_CALIBRATION_DEBUG


class FC_HMC5883L_Lib
{
 public:
	struct vector3Int
	{
		int16_t x;
		int16_t y;
		int16_t z;
	};
	
	FC_HMC5883L_Lib();
	bool initialize(bool needToBeginWire_flag = true);
	void setFastClock();
	void readCompassData(float angleX=-100, float angleY=-100);
	float getHeading();
	void calibrateCompass(uint8_t seconds=10);
	void setCalibrationValues(vector3Int& minimums, vector3Int& maximums); // instead of making a full calibration, provide values from earlier calibration
	void getCalibrationValues(vector3Int* minimums, vector3Int* maximums); // values returned through the pointers
	void setCompassDeclination(float declin);
	void enableHMC_on_MPU(bool needToBeginWire_flag = true); // set proper bits in MPU-6050 to enable compass in GY-86 module
	
 private:
	static const uint8_t HMC5883L_Address = 0x1E;
	static const uint8_t MPU6050_Address = 0x68;

	vector3Int compass_raw; // raw data read from the device
	vector3Int compass; // compass data after calculations
	float heading;
	float declination;
	
	// calibration values to calculate scale and offset
	vector3Int calMins; // calibration minimum values of raw readings
	vector3Int calMaxs; // calibration maximum values of raw readings
	// offset
	vector3Int offset;
	// scale multipliers (don't know why without x)
	float scale_y;
	float scale_z;
	
	// To enable HMC5883L on GY-86
	uint8_t readRegister8(uint8_t reg);
	void writeRegister8(uint8_t reg, uint8_t value);
	void writeRegisterBit(uint8_t reg, uint8_t pos, bool state);
};


#endif

