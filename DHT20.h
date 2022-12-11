/*
Aosong AHT20/DHT20 sensor library for Raspberry Pi Pico.

Version: 0.0.1
Copyright 2022 Sampsa Penna, Kimi Malkamäki
*/

#ifndef PICO_DHT20_DEFINED
#define PICO_DHT20_DEFINED

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>

#define DHT20_OK 0
#define DHT20_ERROR_CHECKSUM -10
#define DHT20_ERROR_CONNECT -11
#define DHT20_MISSING_BYTES -12
#define DHT20_ERROR_BYTES_ALL_ZERO -13
#define DHT20_ERROR_READ_TIMEOUT -14
#define DHT20_ERROR_LASTREAD -15
#define DHT20_ERROR_NORESET -16
#define DHT20_ERROR_BUSY -17

#ifdef USE_I2C_1
#define I2C_INST i2c1
#else
#define I2C_INST i2c0
#endif

#define DHT20_ADDRESS 0x38

typedef struct DHT20
{
    float humidity;
    float temperature;
    float humOffset;
    float tempOffset;

    uint8_t status;
    uint32_t lastRequest;
    uint32_t lastRead;
    uint32_t updateInterval;
    uint8_t bytes[7];
    uint8_t crc;
} DHT20;

/*
Prep constant i2c messages in program memory
*/
// Resets
static const uint8_t __in_flash() rst_msg_1[3] = {0x1B, 0x00, 0x00};
static const uint8_t __in_flash() rst_msg_2[3] = {0x1C, 0x00, 0x00};
static const uint8_t __in_flash() rst_msg_3[3] = {0x1E, 0x00, 0x00};
// Trigger measurement
static const uint8_t __in_flash() trigger_measurement[3] = {0xAC, 0x33, 0x00};

/*
Initialize the DHT sensor
*/
int DHT20_init(DHT20 *sens);

/*
Calculate checksum value (private)
*/
static uint8_t _crc8(uint8_t *ptr, uint8_t len);

/*
Reset sensor (private)
*/
static void resetSensor(DHT20 *sens);

/*
Start taking a temperature measurement
*/
int startMeasurement(DHT20 *sens);
/*
Read the finished measurement
*/
int readMeasurement(DHT20 *sens);
/*
Convert the raw data to sensible values
*/
int convert(DHT20 *sens);

/*
Take a complete measurement from the sensor without external timing
*/
int getMeasurement(DHT20 *sens);

/*
updateMeasurements this function does not sleep and updates based on the updateInterval variable
*/
int updateMeasurement(DHT20 *sens);

/*
Access the converted temperature & humidity
*/
float getHumidity(DHT20 *sens);
float getTemperature(DHT20 *sens);

/*
Offset getters and setters
*/
void setHumOffset(DHT20 *sens, float offset);
void setTempOffset(DHT20 *sens, float offset);
float getHumOffset(DHT20 *sens);
float getTempOffset(DHT20 *sens);

/*
set updateInterval for the updateMeasurement function. this interval should be over 1000ms
*/
void setUpdateInterval(struct DHT20 *sens, uint8_t time);

/*
Check the sensor status word
*/
uint8_t readStatus(DHT20 *sens);

/*
Check sensor calibration status
*/
bool isCalibrated(DHT20 *sens);

/*
Check if sensor needs a reset
*/
static bool needsReset(DHT20 *sens);

/*
Check status from last completed read()
*/
int internalStatus(DHT20 *sens);

/*
Check time since last read() call
*/
uint32_t lastRead(DHT20 *sens);
/*
Check time since last measurement request
*/
uint32_t lastRequest(DHT20 *sens);

#endif
