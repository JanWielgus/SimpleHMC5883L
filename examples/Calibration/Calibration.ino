/**
 * @file Calibration.ino
 * @author Jan Wielgus
 * @brief Calibration of compass
 * @date 2021-08-17
 */

// #define THROUGH_MPU6050

#include <SimpleHMC5883L.h>

#ifdef THROUGH_MPU6050
    #include <SimpleMPU6050.h>
    SimpleMPU6050 mpu;
#endif

SimpleHMC5883L compass;

SimpleHMC5883L::vector3Int16 mins = {};
SimpleHMC5883L::vector3Int16 maxs = {};


void setup()
{
    Serial.begin(115200);
    Serial.println("Program has started");

    Wire.begin();

    #ifdef THROUGH_MPU6050
        mpu.initialize();
        mpu.enableCompassBypass();
    #endif

    if (!compass.initialize())
    {
        while (true)
        {
            Serial.println("Problems with compass initialization");
            delay(1000);
        }
    }

    Wire.setClock(400000L);

    Serial.println("Starting calibration");
    delay(1000);
    compass.setCompassOffset(0, 0, 0);
}


void loop()
{
    compass.readRaw();

    auto raw = compass.getRaw();

    if (raw.x < mins.x)
        mins.x = raw.x;
    else if (raw.x > maxs.x)
        maxs.x = raw.x;

    if (raw.y < mins.y)
        mins.y = raw.y;
    else if (raw.y > maxs.y)
        maxs.y = raw.y;

    if (raw.z < mins.z)
        mins.z = raw.z;
    else if (raw.z > maxs.z)
        maxs.z = raw.z;

    SimpleHMC5883L::vector3Float offset;
    offset.x = (mins.x + maxs.x) / 2.f + 0.5f;
    offset.y = (mins.y + maxs.y) / 2.f + 0.5f;
    offset.z = (mins.z + maxs.z) / 2.f + 0.5f;

    Serial.print(offset.x);
    Serial.print('\t');
    Serial.print(offset.y);
    Serial.print('\t');
    Serial.println(offset.z);

    delay(15);
}
