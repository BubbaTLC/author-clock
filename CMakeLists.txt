# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.22)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Read OpenWeatherMap API key from environment
file(STRINGS "${CMAKE_SOURCE_DIR}/.env" ENV_VARS)
foreach(ENV_VAR ${ENV_VARS})
    string(REGEX MATCH "^OPEN_WEATHER_MAP_KEY=(.*)$" MATCH_RESULT ${ENV_VAR})
    if(MATCH_RESULT)
        set(OWM_API_KEY ${CMAKE_MATCH_1})
        add_compile_definitions(OWM_API_KEY="${OWM_API_KEY}")
    endif()
endforeach()

project(author_clock)

# Build LittleFS image from main/data/ and flash it with the firmware
littlefs_create_partition_image(storage main/data FLASH_IN_PROJECT)
