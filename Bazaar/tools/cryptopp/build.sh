export CXX=${CROSS_COMPILE}g++
export PREFIX=../build
make static
make install
