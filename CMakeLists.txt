cmake_minimum_required(VERSION 3.12)
project(signals C)

set(CMAKE_VERBOSE_MAKEFILE 1)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_COMPILER "/usr/local/bin/avr-gcc")

set(MMCU atmega328pb)
set(ARCH_FLAGS "\
        -mmcu=${MMCU} \
")

set(CMAKE_C_FLAGS "${ARCH_FLAGS} \
        -DF_CPU=1000000UL \
        -Wall \
        -Werror \
        -std=gnu99 \
        -fdata-sections \
        -ffunction-sections \
        -fpack-struct \
        -fshort-enums \
        -funsigned-bitfields \
        -funsigned-char \
")

set(CMAKE_EXE_LINKER_FLAGS "${ARCH_FLAGS} \
        -Wl,--gc-sections \
")

set(CMAKE_C_LINK_EXECUTABLE "${CMAKE_C_COMPILER} <LINK_FLAGS> <OBJECTS> -o <TARGET>")

add_executable(${CMAKE_PROJECT_NAME} main.c)


add_custom_command(
        OUTPUT ${CMAKE_PROJECT_NAME}.hex
        COMMAND avr-objcopy -j .text -j .data -O ihex ${CMAKE_PROJECT_NAME} ${CMAKE_PROJECT_NAME}.hex
        DEPENDS ${CMAKE_PROJECT_NAME}
)

add_custom_target(
        hex
        DEPENDS ${CMAKE_PROJECT_NAME}.hex
)

set(MODEM "$(find /dev -name 'tty.usbmodem*' 2> /dev/null | head -n1)")

add_custom_target(
        flash
        DEPENDS ${CMAKE_PROJECT_NAME}.hex
        COMMAND sh -c "avrdude -c avrisp -p ${MMCU} -P ${MODEM} -b 19200 -U flash:w:${CMAKE_PROJECT_NAME}.hex"
        VERBATIM
)

add_custom_target(
        erase
        COMMAND sh -c "avrdude -c avrisp -p ${MMCU} -P ${MODEM} -b 19200 -e"
        VERBATIM
)
