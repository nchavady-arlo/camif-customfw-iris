# Bring headers into the project
include_directories(
    ../../common/firmware/STM/utils/Inc
    ../../rfal/include
    ../../rfal/source/${RFAL_VARIANT_FAMILY}
    ./Inc
    ../platform/Inc
    ../platform/${RFAL_VARIANT}
    ../../st25r_demos/Inc
    )

file(GLOB MAIN           "Src/*.c")
file(GLOB SOURCE_DEMO    "../../st25r_demos/Src/demo_polling.c")

# Card Emulation only available on ST25R3916 and ST25R3916B
if((RFAL_VARIANT STREQUAL "st25r3916") OR (RFAL_VARIANT STREQUAL "st25r3916b"))
file(GLOB SOURCE_DEMO_CE "../../st25r_demos/Src/demo_ce.c")
endif()

file(GLOB SOURCE_RFAL_CUSTOM_ANALOG_CONFIG "Src/rfal_analog_config_custom.c")

set(SOURCES ${MAIN} ${SOURCE_DEMO} ${SOURCE_DEMO_CE} ${SOURCE_RFAL_CUSTOM_ANALOG_CONFIG})

# Define to use RFAL Custom Analog Config
add_definitions(-DRFAL_ANALOG_CONFIG_CUSTOM)

# Directory in which the linker will look for the library (must be set before the add_executable() command!)
link_directories(../build/platform/${RFAL_VARIANT})

# link with pthread for semaphores
find_library(LIBPTHREAD_PATH pthread)

# Set the executable name
add_executable(nfc_demo_${RFAL_VARIANT}${EXE_VARIANT} ${SOURCES})

# Set the library to link with
target_link_libraries(nfc_demo_${RFAL_VARIANT}${EXE_VARIANT} librfal_${RFAL_VARIANT}.so ${LIBPTHREAD_PATH})
