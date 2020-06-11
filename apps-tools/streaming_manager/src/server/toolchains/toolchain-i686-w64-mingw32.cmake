set (CMAKE_SYSTEM_NAME Windows)


# specify the cross compiler
set (CMAKE_C_COMPILER i686-w64-mingw32-gcc-posix)
set (CMAKE_CXX_COMPILER i686-w64-mingw32-g++-posix)
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ -lwsock32 -lws2_32 ${CMAKE_CSS_STANDARD_LIBRARIES}")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Bstatic,--whole-archive -Wl,--no-whole-archive -static-libgcc -static-libstdc++ -static -lwinpthread")
# where is the target environment
set (CMAKE_FIND_ROOT_PATH /usr/i686-w64-mingw32)

# search for programs in the build host directories
set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# set the resource compiler (RHBZ #652435)
set (CMAKE_RC_COMPILER i686-w64-mingw32-windres)
set (CMAKE_MC_COMPILER i686-w64-mingw32-windmc)

# override boost thread component suffix as mingw-w64-boost is compiled with threadapi=win32
set (Boost_THREADAPI win32)

# These are needed for compiling lapack (RHBZ #753906)
set (CMAKE_Fortran_COMPILER i686-w64-mingw32-gfortran)
set (CMAKE_AR:FILEPATH i686-w64-mingw32-ar)
set (CMAKE_RANLIB:FILEPATH i686-w64-mingw32-ranlib)
