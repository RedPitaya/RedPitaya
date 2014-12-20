/**
 * $Id: $
 *
 * @brief Red Pitaya library common module implementation
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>

#include "common.h"

static int fd = -1;

int cmn_Init()
{
	if (fd == -1) {
		if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) {
			return RP_EOMD;
		}
	}
	return RP_OK;
}

int cmn_Release()
{
	if (fd != -1) {
		 if(close(fd) < 0) {
			 return RP_ECMD;
		 }
		fd = -1;
	}

	return RP_OK;
}

int cmn_Map(size_t size, size_t offset, void** mapped)
{
	*mapped = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);

	if(mapped == (void *) -1) {
		return RP_EMMD;
	}

	return RP_OK;
}

int cmn_Unmap(size_t size, void** mapped)
{
	if(munmap(*mapped, size) < 0){
		return RP_EUMD;
	}
	*mapped = NULL;
	return RP_OK;
}

int cmn_SetShiftedValue(volatile uint32_t* field, uint32_t value, uint32_t mask, uint32_t bitsToSetShift)
{
	VALIDATE_BITS(value, mask);
	uint32_t currentValue;
	cmn_GetValue(field, &currentValue, 0xffffffff);
	currentValue &=  ~(mask << bitsToSetShift); // Clear all bits at specified location
	currentValue +=  (value << bitsToSetShift); // Set value at specified location
	SET_VALUE(*field, currentValue);
	return RP_OK;
}

int cmn_GetValue(volatile uint32_t* field, uint32_t* value, uint32_t mask)
{
	*value = *field & mask;
	return RP_OK;
}

int cmn_GetShiftedValue(volatile uint32_t* field, uint32_t* value, uint32_t mask, uint32_t bitsToSetShift)
{
	*value = (*field >> bitsToSetShift) & mask;
	return RP_OK;
}

int cmn_SetBits(volatile uint32_t* field, uint32_t bits, uint32_t mask)
{
	VALIDATE_BITS(bits, mask);
	SET_BITS(*field, bits);
	return RP_OK;
}

int cmn_UnsetBits(volatile uint32_t* field, uint32_t bits, uint32_t mask)
{
	VALIDATE_BITS(bits, mask);
	UNSET_BITS(*field, bits);
	return RP_OK;
}

int cmn_AreBitsSet(uint32_t field, uint32_t bits, uint32_t mask, bool* result)
{
	VALIDATE_BITS(bits, mask);
	*result = ARE_BITS_SET(field, bits);
	return RP_OK;
}
