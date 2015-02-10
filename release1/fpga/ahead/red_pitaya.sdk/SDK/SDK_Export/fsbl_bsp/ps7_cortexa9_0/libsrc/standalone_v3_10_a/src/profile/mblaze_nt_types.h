//////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002-11 Xilinx, Inc.  All rights reserved.
// Xilinx, Inc.
//
// XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
// COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
// ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
// STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
// IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
// FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
// XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
// THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
// ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
// FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $Id: mblaze_nt_types.h,v 1.1.2.2 2011/05/30 06:46:18 svemula Exp $
//
//////////////////////////////////////////////////////////////////////

#ifndef _MBLAZE_NT_TYPES_H
#define _MBLAZE_NT_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef char            byte;
typedef short           half;
typedef int             word;
typedef unsigned char   ubyte;
typedef unsigned short  uhalf;
typedef unsigned int    uword;
typedef ubyte           boolean;

//typedef unsigned char   u_char;
//typedef unsigned short  u_short;
//typedef unsigned int    u_int;
//typedef unsigned long   u_long;

typedef short           int16_t;
typedef unsigned short  uint16_t;
typedef int             int32_t;
typedef unsigned int    uint32_t;

#ifdef __cplusplus
}
#endif

#endif
