/**
 * @brief Red Pitaya simple version strings. To be embedded in binaries
 *        at build time for SW traceability.
 *
 * @author Ulrich Habel (DF4IAH) <espero7757@gmx.net>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef VERSION_H
#define VERSION_H

#define XSTR(s) STR(s)
#define STR(s) #s


/** @defgroup version_h Default version information
 * @{
 */

#ifndef VERSION
#define VERSION_STR "0.00-0000"
#else
#define VERSION_STR XSTR(VERSION)
#endif
 
#ifndef REVISION
#define REVISION_STR "unknown"
#else
#define REVISION_STR XSTR(REVISION)
#endif

/** @} */


#endif /* VERSION_H */
