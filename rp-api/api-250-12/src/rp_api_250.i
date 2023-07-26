%module rp_api_250

%include <typemaps.i>
%include <cstring.i>
%include <carrays.i>
%include <cpointer.i>

%apply unsigned char { uint8_t }
%apply unsigned short { uint16_t }
%apply unsigned int { uint32_t }
%apply unsigned long long { uint64_t }
%apply int { int32_t }


%apply unsigned char *OUTPUT { uint8_t *_out_value };

%apply int { mcp47x6_model }

%{
/* Includes the header in the wrapper code */
#include "rp-spi.h"
#include "rp-i2c.h"
#include "rp-spi.h"
#include "rp-i2c-mcp47x6.h"
#include "rp-i2c-max7311.h"
#include "rp-gpio-power.h"

%}

/* Parse the header file to generate wrappers */

%include "rp-spi.h"
%include "rp-i2c.h"
%include "rp-spi.h"
%include "rp-i2c-mcp47x6.h"
%include "rp-i2c-max7311.h"
%include "rp-gpio-power.h"