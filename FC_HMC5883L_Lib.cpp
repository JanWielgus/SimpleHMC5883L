/*
    Created:	06/06/2019
    Author:     Jan Wielgus
*/
// 
// 
// 

#include <FC_HMC5883L_Lib.h>


FC_HMC5883L_Lib::FC_HMC5883L_Lib()
{
	compass_raw.x = 0;
	compass_raw.y = 0;
	compass_raw.z = 0;
	
	offset.x = 0;
	offset.y = 0;
	offset.z = 0;
	
	heading = 0;
	
	
	// default calibration values (!!!!!!!!!!!!!!) <--- !!!
	calMins.x = 0;
	calMins.y = 0;
	calMins.z = 0;
	calMaxs.x = 0;
	calMaxs.y = 0;
	calMaxs.z = 0;
	
	declination = 0;
}


bool FC_HMC5883L_Lib::initialize(bool needToBeginWire_flag)
{
	if (needToBeginWire_flag)
		Wire.begin();
	
	// Check if the compass is responding
	Wire.beginTransmission(HMC5883L_Address);
	if (Wire.endTransmission() != 0)
	{
		// Cannot connect with the HMC5883L
		
		return false;
	}
	
	
	// Device setup
	Wire.beginTransmission(HMC5883L_Address);
	Wire.write(0x00);                            // We want to write to the Configuration Register A (00 hex).
	Wire.write(0x78);                            // Set the Configuration Register A bits as 01111000 to set sample rate (average of 8 at 75Hz).
	Wire.write(0x20);                            // Set the Configuration Register B bits as 00100000 to set the gain at +/-1.3Ga.
	Wire.write(0x00);                            // Set the Mode Register bits as 00000000 to set Continues-Measurement Mode.
	Wire.endTransmission();
	
	
	// calculate the calibration offset and scale values
	scale_y = float(calMaxs.x - calMins.x) / float(calMaxs.y - calMins.y);
	scale_z = float(calMaxs.x - calMins.x) / float(calMaxs.z - calMins.z);
	
	offset.x = float(calMaxs.x - calMins.x) / 2 - calMaxs.x;
	offset.y = (float(calMaxs.y - calMins.y) / 2 - calMaxs.y) * scale_y;
	offset.z = (float(calMaxs.z - calMins.z) / 2 - calMaxs.z) * scale_z;
	
	
	return true;
}


void FC_HMC5883L_Lib::setFastClock()
{
	Wire.setClock(400000L);
}


void FC_HMC5883L_Lib::setCalibrationValues(vector3Int& minimums, vector3Int& maximums)
{
	// calculate the calibration offset and scale values
	scale_y = float(maximums.x - minimums.x) / float(maximums.y - minimums.y);
	scale_z = float(maximums.x - minimums.x) / float(maximums.z - minimums.z);
	
	offset.x = float(maximums.x - minimums.x) / 2 - maximums.x;
	offset.y = (float(maximums.y - minimums.y) / 2 - maximums.y) * scale_y;
	offset.z = (float(maximums.z - minimums.z) / 2 - maximums.z) * scale_z;
}


void FC_HMC5883L_Lib::getCalibrationValues(vector3Int* minimums, vector3Int* maximums)
{
	minimums->x = calMins.x;
	minimums->y = calMins.y;
	minimums->z = calMins.z;
	
	maximums->x = calMaxs.x;
	maximums->y = calMaxs.y;
	maximums->z = calMaxs.z;
}


void FC_HMC5883L_Lib::calibrateCompass(uint8_t seconds)
{
	#ifdef SERIAL_CALIBRATION_DEBUG
		Serial.println("Compass calibration has started");
	#endif
	
	// Initial values for calibration. If not provided, there is zero (Z axiz is always below zero so max is never reached))
	readCompassData();
	calMins.x = calMaxs.x = compass_raw.x;
	calMins.y = calMaxs.y = compass_raw.y;
	calMins.z = calMaxs.z = compass_raw.z;
	
	uint32_t startTime = millis();
	while (millis()-startTime < seconds*1000)
	{
		delayMicroseconds(3700); // simulate 250Hz program
		
		readCompassData();
		
		// Detect minimum and maximum values
		if (compass_raw.x < calMins.x) calMins.x = compass_raw.x;
		if (compass_raw.x > calMaxs.x) calMaxs.x = compass_raw.x;
		
		if (compass_raw.y < calMins.y) calMins.y = compass_raw.y;
		if (compass_raw.y > calMaxs.y) calMaxs.y = compass_raw.y;
		
		if (compass_raw.z < calMins.z) calMins.z = compass_raw.z;
		if (compass_raw.z > calMaxs.z) calMaxs.z = compass_raw.z;
		
		#ifdef SERIAL_CALIBRATION_DEBUG
			Serial.print("min|max: X: ");
			Serial.print(calMins.x);
			Serial.print(" | ");
			Serial.print(calMaxs.x);
			Serial.print("\tY: ");
			Serial.print(calMins.y);
			Serial.print(" | ");
			Serial.print(calMaxs.y);
			Serial.print("\tZ: ");
			Serial.print(calMins.z);
			Serial.print(" | ");
			Serial.print(calMaxs.z);
			Serial.println();
		#endif
	}
	
	
	// update scale and offset after calibration
	setCalibrationValues(calMins, calMaxs);
	
	#ifdef SERIAL_CALIBRATION_DEBUG
		Serial.println("Finished");
	#endif
}


void FC_HMC5883L_Lib::readCompassData(float angleX, float angleY)
{
	// Read raw data
	Wire.beginTransmission(HMC5883L_Address);
	Wire.write(0x03);
	Wire.endTransmission();
	
	Wire.requestFrom(HMC5883L_Address, 6);
	compass_raw.y = Wire.read() << 8 | Wire.read();
	compass_raw.y *= -1;
	compass_raw.z = Wire.read() << 8 | Wire.read();
	compass_raw.x = Wire.read() << 8 | Wire.read();
	compass_raw.x *= -1;
	
	/*
	Serial.print("raw X: ");
	Serial.print(compass_raw.x);
	Serial.print("\tY:");
	Serial.print(compass_raw.y);
	Serial.print("\tZ:");
	Serial.print(compass_raw.z);
	Serial.println();
	*/
	
	// Process data
	compass.y = compass_raw.y + offset.y;
	compass.y *= scale_y;
	compass.z = compass_raw.z + offset.z;
	compass.z *= scale_z;
	compass.x = compass_raw.x + offset.x;
	
	// If angle data is provided
	if (angleX != -100) // others are not necessary
	{
		compass.x = (float)compass.x * cos(-radians(angleY)) + (float)compass.y * sin(radians(angleX)) * sin(-radians(angleY)) - (float)compass.z * cos(radians(angleX)) * sin(-radians(angleY));
		compass.y = (float)compass.y * cos(radians(angleX)) + (float)compass.z * sin(radians(angleX));
	}
	
	// Heading is calculated in degrees
	if (compass.y < 0)
		heading = 360 + degrees(atan2(compass.x, compass.y));
	else
		heading = degrees(atan2(compass.x, compass.y));
	
	// heading is back to front
	heading -= 180;
	
	// To get the geographic north
	heading += declination;
	
	// Keep heading in the range of 0-359.99 degrees
	if (heading < 0)
		heading += 360;
	else if (heading >= 360)
		heading -= 360;
}


float FC_HMC5883L_Lib::getHeading()
{
	return heading;
}


void FC_HMC5883L_Lib::setCompassDeclination(float declin)
{
	declination = declin;
}



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// To enable HMC5883L on GY-86

uint8_t FC_HMC5883L_Lib::readRegister8(uint8_t reg)
{
	uint8_t value;

	Wire.beginTransmission(MPU6050_Address);
	Wire.write(reg);
	Wire.endTransmission();

	Wire.beginTransmission(MPU6050_Address);
	Wire.requestFrom(MPU6050_Address, 1);
	while (!Wire.available())
	{};
	value = Wire.read();
	Wire.endTransmission();

	return value;
}


void FC_HMC5883L_Lib::writeRegister8(uint8_t reg, uint8_t value)
{
	Wire.beginTransmission(MPU6050_Address);
	Wire.write(reg);
	Wire.write(value);
	Wire.endTransmission();
}


void FC_HMC5883L_Lib::writeRegisterBit(uint8_t reg, uint8_t pos, bool state)
{
	uint8_t value;
	value = readRegister8(reg);

	if (state)
		value |= (1 << pos);
	else
		value &= ~(1 << pos);

	writeRegister8(reg, value);
}


void FC_HMC5883L_Lib::enableHMC_on_MPU(bool needToBeginWire_flag)
{
	if (needToBeginWire_flag)
		Wire.begin();
	
	// setting I2C Master Mode disabled
	writeRegisterBit(0x6A, 5, false);
	
	// setting I2C Bypass enabled
	writeRegisterBit(0x37, 1, true);
	
	// setting Sleep disabled
	writeRegisterBit(0x6B, 6, false);
}

