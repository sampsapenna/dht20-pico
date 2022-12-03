# Aosong AHT20/DHT20 – Raspberry Pico SDK port
This is a library for driving the Aosong DHT20/AHT20 sensor, sold by multiple
3rd parties like Adafruit and Seeedstudio. The sensor is connected to using
i2c.

The only dependency for this library is `pico-sdk`, but since it will be
imported by whatever project uses this library as a dependency, it's not
necessary to add as a submodule separately here.

The library draws some inspiration from [Rob Tillaart's library for the Arduino IDE](https://github.com/RobTillaart/DHT20) 
but changes some things to better suit usage with a Raspberry Pi Pico.

An example pico program is provided in [DHT20 Pico example repository.](https://github.com/sampsapenna/dht20-pico-example)

## Usage
The port can be easily integrated into a Pico SDK project as a git submodule.
```bash
git submodule add https://github.com/sampsapenna/dht20-pico.git
git submodule update
```
After cloning the repository you need to add the library to `CMakeLists.txt`
used when building the project. Add the following lines before `target_link_libraries`:
```cmake
add_subdirectory("dht20-pico")

target_include_directories(dht20 PUBLIC "dht20-pico")
target_link_directories(rgb_lcd PUBLIC "dht20-pico")
```

Additionally you will need to add the required libraries to linking, by
adding `hardware_i2c dht20 pico_binary_info` to `target_link_libraries`.

Libary does not initialize i2c, but assumes that program initializes i2c
for it. Default initialization, sourced from Raspberry Pi Pico documentation,
can look e.g. like this:
```c
    i2c_init(i2c_default, 100 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));
```
Header file to include is `DHT20.h`, same as in the Arduino library.

## Bugs
Sensor has been tested for normal operation. AHT20 has not been tested, but
should work just as well as the DHT20 variant, since the only difference
between the two is packaging AFAIK.

If you encounter a bug, go ahead and open an issue.
