/**
 * @file SimpleHMC5883L.h
 * @author Jan Wielgus
 * @brief Simple HMC5883L compass library.
 * @date 2020-11-26
 * 
 */

#ifndef SIMPLEHMC5883L_H
#define SIMPLEHMC5883L_H

#include <Wire.h>

#ifdef ARDUINO
    #include <Arduino.h>
#endif


class SimpleHMC5883L
{
public:
    struct vector3Int16
	{
		int16_t x;
		int16_t y;
		int16_t z;
	};

    struct vector3Float
	{
		float x;
		float y;
		float z;
	};

private:
	vector3Int16 rawData;
	vector3Float normData; // updated in getter
	vector3Int16 offset; // Calibration values
	const float compassNormalizeMultiplier = 0.92f; // Normalize multiplier
	static const uint8_t HMC5883L_Address = 0x1E;


public:
    SimpleHMC5883L();

    /**
     * @brief Connects with the device.
     * Call Wire.begin() before!!!
     * @return false if device was not initialized
     * (for example couldn't connect to HMC5883L). 
     */
	bool initialize();

	void readRaw();
	const vector3Int16& getRaw() const;
	const vector3Float& getNormalized() const;
	void calibrate(uint16_t duration = 30); // in seconds
	vector3Int16 getCompassOffset() const;
	void setCompassOffset(int16_t offX, int16_t offY, int16_t offZ);
};


#endif

