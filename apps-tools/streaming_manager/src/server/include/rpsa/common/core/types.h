/**
 * @file    types.h
 * @author  isnullxbh
 * @date    2019-03-19 13:14
 * @version 0.0.1
 */

#ifndef STREAMING_APPLICATION_COMMON_TYPES_H
#define STREAMING_APPLICATION_COMMON_TYPES_H

#ifdef OS_MACOS
using ulong = unsigned int;
#endif


#ifdef _WIN32
using ulong = unsigned long;
using u_int8_t = unsigned char;
using u_int16_t = unsigned short;
using u_int32_t = unsigned long int;
using u_int64_t = unsigned long long int;
#endif

#endif // STREAMING_APPLICATION_COMMON_TYPES_H
