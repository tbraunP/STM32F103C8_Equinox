cmake_minimum_required(VERSION 2.8)
project(stm32f103Equinox)
SET(TARGET "stm32f103Equinox")

#custom command to use objcopy to create .bin files out of ELF files
function(make_firmware INPUT)
              add_custom_command(TARGET ${INPUT}
                      COMMAND arm-none-eabi-objcopy -O binary ${INPUT} ${TARGET}.bin
                      COMMENT "objcopying to make mbed compatible firmware")
              set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${TARGET}.bin)
endfunction(make_firmware)


INCLUDE(CMakeForceCompiler)
 
SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_VERSION 1)

# specify the cross compiler
CMAKE_FORCE_C_COMPILER(arm-none-eabi-gcc GNU)
CMAKE_FORCE_CXX_COMPILER(arm-none-eabi-g++ GNU)
 
find_program(LINKER arm-none-eabi-ld)
find_program(OBJCOPY arm-none-eabi-objcopy)
find_program(OBJDUMP arm-none-eabi-objdump)
SET(CMAKE_AR arm-none-eabi-ar)
SET(CMAKE_RANLIB arm-none-eabi-ranlib)

# Set compiler and linker options
SET(LIBS_FLAG "-DHSE_VALUE=8000000 -DSTM32F103C8 -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER -g") 
SET(COMMON_FLAGS "-O0 -mcpu=cortex-m3 -mthumb -ffunction-sections -fdata-sections -fno-common -fmessage-length=0 -fsingle-precision-constant -Wall ${LIBS_FLAG} -mfloat-abi=soft -Wunused-value -Wno-packed-bitfield-compat")
SET(CMAKE_CXX_FLAGS "${COMMON_FLAGS}  -fno-rtti  -fno-exceptions -std=gnu++0x")
SET(CMAKE_C_FLAGS "${COMMON_FLAGS} -std=gnu99")

SET(CMAKE_EXE_LINKER_FLAGS "-mcpu=cortex-m3  -mthumb -mfloat-abi=soft -lm -Wl,-Map=${TARGET}.map,--cref -Wl,--gc-sections \"-T${CMAKE_SOURCE_DIR}/stm32f103_flash.ld\" -static")
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${TARGET}.map)

#remove -rdynamic to link static
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS) 
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)

# Debug Options
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS}  -g")
SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -g")

# add libs in subdirectory using their CMakefile to build the libs
#add_subdirectory (lib/STM32F4_LIB_CMSIS)
#add_subdirectory (lib/STM32F4_LIB_PeriphLib)
#add_subdirectory (lib/FreeRTOS)

# add -I compile headers for LIBS
include_directories("lib/CMSIS")
include_directories("lib/STM32F10x_StdPeriph_Driver/inc")
include_directories("lib/syscalls")
include_directories("lib/STM32_USB-FS-Device_Driver/inc")

# CMSIS src
AUX_SOURCE_DIRECTORY("lib/CMSIS/" CMSISSRCS)

# PERIP
AUX_SOURCE_DIRECTORY("lib/STM32F10x_StdPeriph_Driver/src/" PERIPHSRCS)

# USB FS
#AUX_SOURCE_DIRECTORY("lib/STM32_USB-FS-Device_Driver/src/" USBFSSRCS)


# SYSCALLS
AUX_SOURCE_DIRECTORY("lib/syscalls/" SYSCALLSRCS)


SET(LIBSRCS ${CMSISSRCS} ${PERIPHSRCS} ${SYSCALLSRCS} ${USBFSSRCS})

# freertos include files
#include_directories("lib/FreeRTOS/FreeRTOS/Source/include")
#include_directories("lib/FreeRTOS/FreeRTOS/Source/portable/GCC/ARM_CM4F")

#SET(FREERTOS ${FREERTOS_SRCS} ${FREERTOS_PORT_SRCS})

# default files
include_directories("inc")
include_directories("src")

# add source files from folder
AUX_SOURCE_DIRECTORY("src/" SRCS)
AUX_SOURCE_DIRECTORY("src/hw" SRCSHW)
AUX_SOURCE_DIRECTORY("src/util" SRCSUTIL)
AUX_SOURCE_DIRECTORY("src/dcf77" SRCSDCF)
AUX_SOURCE_DIRECTORY("src/ledClock" SRCSLEDCLOCK)
AUX_SOURCE_DIRECTORY("src/ws2812" SRCSWS2812)


SET(SRCS ${SRCS} ${SRCSHW} ${SRCSUTIL} ${SRCSDCF} ${SRCSLEDCLOCK} ${SRCSWS2812} ${SRCSWS2812} ${LIBSRCS})

# define target
add_executable(${TARGET}.elf ${SRCS})
#target_link_libraries (${TARGET}.elf m c STM32F4_LIB_CMSIS STM32F4_LIB_PeriphLib FREERTOS_752)
target_link_libraries (${TARGET}.elf m c)

make_firmware(${TARGET}.elf)
