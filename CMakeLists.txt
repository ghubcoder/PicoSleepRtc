cmake_minimum_required(VERSION 3.12)
include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)
project(main_project)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()


add_library(ds3232rtc ds3232rtc.cpp)
target_link_libraries(ds3232rtc hardware_i2c pico_stdlib pico_runtime )

add_executable(
  sleep
  sleep.cpp
)

# Comment this out when running in prod
# else it hangs after 10 or so cycles
pico_enable_stdio_uart(sleep 1)

pico_add_extra_outputs(sleep)
target_link_libraries(sleep pico_stdlib hardware_sleep hardware_rtc ds3232rtc)
