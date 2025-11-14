# toolchain-aarch64-linux-gnu.cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Specify the cross-compiler
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

# Sysroot location (adjust based on your setup)
set(CMAKE_SYSROOT /usr/aarch64-linux-gnu /usr/lib/aarch64-linux-gnu /usr/include/aarch64-linux-gnu)
set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT})

# Search settings
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Optional: Architecture-specific flags
set(ARCH_FLAGS "-mcpu=cortex-a72")
set(CMAKE_C_FLAGS_INIT ${ARCH_FLAGS})
set(CMAKE_CXX_FLAGS_INIT ${ARCH_FLAGS})

# Optional: For Debian/Ubuntu multiarch support
set(CMAKE_LIBRARY_ARCHITECTURE aarch64-linux-gnu)