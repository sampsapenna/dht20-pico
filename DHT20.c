/*
Aosong AHT20/DHT20 sensor library for Raspberry Pi Pico.
*/

#include <string.h>

#include "DHT20.h"

uint8_t readStatus(DHT20 *sens)
{
    uint8_t ret;
    i2c_read_blocking(I2C_INST, DHT20_ADDRESS, &ret, 1, false);
    return ret;
}

static bool needsReset(DHT20 *sens)
{
    return ((readStatus(sens) & 0x18) != 0x18);
}

int DHT20_init(DHT20 *sens)
{
    sens->temperature = 0;
    sens->humidity = 0;
    sens->humOffset = 0;
    sens->tempOffset = 0;
    sens->status = 0;
    sens->lastRequest = 0;
    sens->lastRead = 0;
    sens->crc = 0;
    memset(sens->bytes, 0, 7);

    // Ensure 100ms wait after powerup as per sensor datasheet
    sleep_ms(100);

    for (int i = 0; i < 3; i++)
    {
        resetSensor(sens);
        sleep_ms(100);
        if (!needsReset(sens))
        {
            return DHT20_OK;
        }
    }

    return DHT20_ERROR_NORESET;
}

static void resetSensor(DHT20 *sens)
{
    if (needsReset(sens))
    {
        i2c_write_blocking(i2c0, DHT20_ADDRESS, rst_msg_1, 3, false);
        i2c_write_blocking(i2c0, DHT20_ADDRESS, rst_msg_2, 3, false);
        i2c_write_blocking(i2c0, DHT20_ADDRESS, rst_msg_3, 3, false);
    }
}

int startMeasurement(DHT20 *sens)
{
    if (i2c_write_blocking(I2C_INST, DHT20_ADDRESS, trigger_measurement, 3, false) == PICO_ERROR_GENERIC)
    {
        return DHT20_ERROR_CONNECT;
    }
    return DHT20_OK;
}

int readMeasurement(DHT20 *sens)
{
    if (i2c_read_blocking(I2C_INST, DHT20_ADDRESS, sens->bytes, 7, false) == PICO_ERROR_GENERIC)
    {
        return DHT20_ERROR_CONNECT;
    }

    // Return busy if the measurements weren't ready yet
    if ((sens->bytes[0] & 0x80) == 0x80)
    {
        return DHT20_ERROR_BUSY;
    }

    bool allZero = true;
    for (int i = 0; i < 7; i++)
    {
        if (sens->bytes[i] > 0)
        {
            allZero = false;
            break;
        }
    }
    if (allZero)
    {
        return DHT20_ERROR_BYTES_ALL_ZERO;
    }

    sens->lastRead = to_ms_since_boot(get_absolute_time());

    return DHT20_OK;
}

int convert(DHT20 *sens)
{
    //  CONVERT AND STORE
    sens->status = sens->bytes[0];
    uint32_t raw = sens->bytes[1];
    raw <<= 8;
    raw += sens->bytes[2];
    raw <<= 4;
    raw += (sens->bytes[3] >> 4);
    sens->humidity = raw * 9.5367431640625e-5; // ==> / 1048576.0 * 100%;

    raw = (sens->bytes[3] & 0x0F);
    raw <<= 8;
    raw += sens->bytes[4];
    raw <<= 8;
    raw += sens->bytes[5];
    sens->temperature = raw * 1.9073486328125e-4 - 50; //  ==> / 1048576.0 * 200 - 50;

    //  TEST CHECKSUM
    sens->crc = _crc8(sens->bytes, 6);
    if (sens->crc != sens->bytes[6])
    {
        return DHT20_ERROR_CHECKSUM;
    }

    return DHT20_OK;
}

int getMeasurement(DHT20 *sens)
{
    int status;

    if (to_ms_since_boot(get_absolute_time()) - sens->lastRead < 1000)
    {
        return DHT20_ERROR_LASTREAD;
    }

    status = startMeasurement(sens);
    if (status != DHT20_OK)
    {
        return status;
    }

    sleep_ms(50);

    for (int i = 5; i > 0; i--)
    {
        status = readMeasurement(sens);
        if (status == DHT20_ERROR_BUSY)
        {
            sleep_ms(10);
            continue;
        }
    }

    if (status != DHT20_OK)
    {
        return status;
    }

    return convert(sens);
}

float getHumidity(DHT20 *sens)
{
    return sens->humidity + sens->humOffset;
}

float getTemperature(DHT20 *sens)
{
    return sens->temperature + sens->tempOffset;
}

void setHumOffset(struct DHT20 *sens, float offset)
{
    sens->humOffset = offset;
};

void setTempOffset(struct DHT20 *sens, float offset)
{
    sens->tempOffset = offset;
};

float getHumOffset(DHT20 *sens)
{
    return sens->humOffset;
};

float getTempOffset(DHT20 *sens)
{
    return sens->tempOffset;
};

bool isCalibrated(DHT20 *sens)
{
    return (readStatus(sens) & 0x08) == 0x08;
}

bool isMeasuring(DHT20 *sens)
{
    return (readStatus(sens) & 0x80) == 0x80;
}

bool isIdle(DHT20 *sens)
{
    return (readStatus(sens) & 0x80) == 0x00;
}

int internalStatus(DHT20 *sens)
{
    return sens->status;
};

uint32_t lastRead(DHT20 *sens)
{
    return sens->lastRead;
};

uint32_t lastRequest(DHT20 *sens)
{
    return sens->lastRequest;
};

static uint8_t _crc8(uint8_t *ptr, uint8_t len)
{
    uint8_t crc = 0xFF;
    while (len--)
    {
        crc ^= *ptr++;
        for (uint8_t i = 0; i < 8; i++)
        {
            if (crc & 0x80)
            {
                crc <<= 1;
                crc ^= 0x31;
            }
            else
            {
                crc <<= 1;
            }
        }
    }
    return crc;
}
