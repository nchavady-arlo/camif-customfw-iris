add_definitions(-DST25R3916)





# Bring headers into the project
include_directories(
    ../../../common/firmware/STM/utils/Inc
    ../../../rfal/include
    ../../../rfal/source/${RFAL_VARIANT}
    ../../../rfal/source
    ../Inc
    ../${RFAL_VARIANT}
    )

# Source files to build the project
file(GLOB SOURCE1   "../../../rfal/source/*.c")
file(GLOB SOURCE2   "../../../rfal/source/${RFAL_VARIANT}/*.c")

file(GLOB PLATFORM1 "../../platform/Src/pltf_gpio.c")
file(GLOB PLATFORM2 "../../platform/Src/pltf_timer.c")
if(BUILD_I2C)
file(GLOB PLATFORM3 "../../platform/Src/pltf_i2c.c")
else()
file(GLOB PLATFORM3 "../../platform/Src/pltf_spi.c")
endif()

set(SOURCE ${SOURCE1} ${SOURCE2} ${PLATFORM1} ${PLATFORM2} ${PLATFORM3})

# Link with rt for POSIX timers
find_library(LIBRT_PATH rt)
find_library(LIBMATH_PATH m)

# Create the shared library
add_library(rfal_${RFAL_VARIANT} SHARED ${SOURCE})

target_link_libraries(rfal_${RFAL_VARIANT} "${LIBRT_PATH}" "${LIBMATH_PATH}")
