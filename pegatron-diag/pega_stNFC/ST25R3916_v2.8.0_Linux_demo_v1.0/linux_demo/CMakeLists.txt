# CMakeLists text file of the project

cmake_minimum_required(VERSION 2.8.12)

project(rfal_demo)

# RFAL library variant to link the executable with
set(RFAL_VARIANT "st25r3916" CACHE STRING "Select the RFAL library")

# Convert user input to generate lowercase files
string(TOLOWER ${RFAL_VARIANT} RFAL_VARIANT)

set(RFAL_VARIANT_FAMILY ${RFAL_VARIANT})

option(CARD_EMULATION_ONLY "Card Emulation-only demo" OFF)

# Force Card Emulation-only demo, or use the command "cmake -DCARD_EMULATION_ONLY=ON .."
#set(CARD_EMULATION_ONLY ON)

option(BUILD_I2C "I2C communication" OFF)
if(BUILD_I2C)
    set(COMMUNICATION "using I2C communication")
    add_definitions(-DRFAL_USE_I2C)
endif()

if(CARD_EMULATION_ONLY)
    message("Building " ${RFAL_VARIANT} " Card Emulation-only demo " ${COMMUNICATION})
    set(EXE_VARIANT "_CE")

    add_definitions(-DDEMO_CARD_EMULATION_ONLY=true)
    add_definitions(-DDEMO_NO_DELAY_IN_DEMOCYCLE)
else()
    message("Building " ${RFAL_VARIANT} " Reader/Writer demo " ${COMMUNICATION})
endif()

# Compile the RFAL library variant to link the executable with
add_subdirectory(platform/${RFAL_VARIANT})
add_subdirectory(demo)
