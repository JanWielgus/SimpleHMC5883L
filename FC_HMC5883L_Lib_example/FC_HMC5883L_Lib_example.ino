/*
    Name:       FC_HMC5883L_Lib_example.ino
    Created:	06/06/2019 14:05:19
    Author:     Jan Wielgus
*/

#include "FC_HMC5883L_Lib.h"
#include "C:\Users\janwi\Documents\GitHub\STM_FlightController\libraries\FC_MPU6050Lib_example\FC_MPU6050Lib.h"


// !!!!!!!!!!!!!
// This is just temporary, because when MPU library is in GitHub localization, linker don't see the cpp file.
// When MPU library will be in the Arduino library folder it will not be necessary.
#include "C:\Users\janwi\Documents\GitHub\STM_FlightController\libraries\FC_MPU6050Lib_example\FC_MPU6050Lib.cpp"



FC_HMC5883L_Lib compass;
FC_MPU6050Lib mpu;

uint32_t loopStartTime;


void setup()
{
	Serial.begin(115200);
	Serial.println("Program has just started!");
	
	while (!mpu.initialize()) // While mpu is not initialized
	{
		// If program is there, some problems occured
		Serial.println("MPU6050 is cannot be initialized!");
		delay(500);
	}
	
	mpu.setCalculationsFrequency(75);
	mpu.calibrateAccelerometer(); // NEED TO BE IN THE LEVEL POSITION
	mpu.calibrateGyro(700);
	
	
	Serial.println("Started enabling");
	// !!!  IF MPU IS NOT USED, AS ARGUMENT THERE SHOULD BE true OR nothing  !!!
	compass.enableHMC_on_MPU(false); // ! IMPORTANT ! if use GY-86. If not then comment !!!
	Serial.println("ended");
	
	while (!compass.initialize(false))
	{
		// If program is there, some problems occured
		Serial.println("HMC5883L cannot be initialized!");
		delay(500);
	}
	
	
	// Compass calibration
	//compass.calibrateCompass();
	
	
	// Temporary and not accurate values
	FC_HMC5883L_Lib::vector3Int minCalVal = {-503, -505, -1440};
	FC_HMC5883L_Lib::vector3Int maxCalVal = {504, 463, -330};
	compass.setCalibrationValues(minCalVal, maxCalVal);
	
	
	/* How to get calibration values
	FC_HMC5883L_Lib::vector3Int readCalValuesMin, readCalValuesMax; // calibration values
	compass.getCalibrationValues(&readCalValuesMin, &readCalValuesMax);
	*/
	
	compass.setCompassDeclination(5.0);
	
	// set initial Z axis value
	mpu.read6AxisMotion();
	FC_MPU6050Lib::vector3Float angle;
	angle = mpu.getFusedXYAngles();
	compass.readCompassData(angle.x, angle.y);
	mpu.setInitialZAxisValue(compass.getHeading());
	
	
	loopStartTime = micros();
}


void loop()
{
	
	mpu.read6AxisMotion();
	FC_MPU6050Lib::vector3Float angle;
	angle = mpu.getFusedXYAngles();

	compass.readCompassData(angle.x, angle.y);
	
	// Fusion with the gyro
	float heading = mpu.getZAngle(compass.getHeading());
	
	Serial.print("Heading: ");
	Serial.print(heading); // fused heading
	Serial.println();
	
	
	// 75Hz
	while (micros()-loopStartTime <= 13333)
	{
		// wait
	}
	loopStartTime = micros();
}
