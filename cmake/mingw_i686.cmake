# run "i686-w64-mingw32-g++ -print-sysroot" to get the sysroot path
execute_process(COMMAND
    i686-w64-mingw32-g++ -print-sysroot
    OUTPUT_VARIABLE MINGW_SYSROOT
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE MINGW_SYSROOT_RESULT
)
if(NOT MINGW_SYSROOT_RESULT EQUAL 0)
    message(FATAL_ERROR "
        x86_64-w64-mingw32-g++ -print-sysroot failed!
        Did you install mingw, and is it in your PATH?
    ")
endif()

# if running on linux host, x86_64-w64-mingw32-g++ -print-sysroot may return an empty string
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND NOT MINGW_SYSROOT)
    set(MINGW_SYSROOT /usr/i686-w64-mingw32 )
endif()
message(STATUS "MINGW_SYSROOT: ${MINGW_SYSROOT}")


# Define the compilers to use for C and C++
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
set(CMAKE_RC_COMPILER i686-w64-mingw32-windres)

# Set the root path for finding MinGW libraries and headers
set(CMAKE_FIND_ROOT_PATH ${MINGW_SYSROOT})

# Configure CMake to only search in the specified root path for libraries and includes
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
