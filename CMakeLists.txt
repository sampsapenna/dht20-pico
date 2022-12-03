add_library(dht20
    STATIC
    DHT20.c
)

target_link_libraries(dht20 pico_stdlib hardware_i2c)
