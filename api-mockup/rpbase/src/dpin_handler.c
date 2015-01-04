/**
 * $Id: $
 *
 * @brief Red Pitaya library Digital Pin handler implementation
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <stdio.h>
#include <stdint.h>

#include "version.h"
#include "common.h"
#include "housekeeping.h"
#include "dpin_handler.h"

int dpin_SetDirection(rp_dpin_t pin, rp_pinDirection_t direction)
{
    // LEDS
    if (pin < RP_DIO0_P)
    {
        // Direction for LEDS is always out
        if (direction == RP_IN)
        {
            return RP_ELID;
        }
    }
    // DIO_P
    else if (pin < RP_DIO0_N)
    {
        pin -= RP_DIO0_P;
        uint32_t bits = (1 << pin);
        return (direction == RP_OUT ? hk_SetExCdPBits(bits) : hk_UnsetExCdPBits(bits));
    }
    // DIO_N
    else
    {
        pin -= RP_DIO0_N;
        uint32_t bits = (1 << pin);
        return (direction == RP_OUT ? hk_SetExCdNBits(bits) : hk_UnsetExCdNBits(bits));
    }

    return RP_OK;
}

int dpin_GetDirection(rp_dpin_t pin, rp_pinDirection_t* direction)
{

    // LEDS
    if (pin < RP_DIO0_P)
    {
        *direction = RP_OUT;
    }
    // DIO_P
    else if (pin < RP_DIO0_N)
    {
        pin -= RP_DIO0_P;
        uint32_t bits = (1 << pin);
        bool dir;

        // Get direction
        ECHECK(hk_AreExCdPBitsSet(bits, &dir));
        *direction = (dir ? RP_OUT : RP_IN);
    }
    // DIO_N
    else
    {
        pin -= RP_DIO0_N;
        uint32_t bits = (1 << pin);
        bool dir;

        // Get direction
        ECHECK(hk_AreExCdNBitsSet(bits, &dir));
        *direction = (dir ? RP_OUT : RP_IN);
    }

    return RP_OK;
}


int dpin_SetState(rp_dpin_t pin, rp_pinState_t state)
{
    // LEDS
    if (pin < RP_DIO0_P)
    {
        uint32_t bits = (1 << pin);
        return (state == RP_HIGH ? hk_SetLedBits(bits) : hk_UnsetLedBits(bits));
    }
    // DIO_P
    else if (pin < RP_DIO0_N)
    {
        pin -= RP_DIO0_P;
        uint32_t bits = (1 << pin);
        bool direction;

        // Get direction
        ECHECK(hk_AreExCdPBitsSet(bits, &direction));
        if (direction) // Direction is OUT
        {
            return (state == RP_HIGH ? hk_SetExCoPBits(bits) : hk_UnsetExCoPBits(bits));
        }
        else
        {
            return RP_EWIP;
        }
    }
    // DIO_N
    else
    {
        pin -= RP_DIO0_N;
        uint32_t bits = (1 << pin);
        bool direction;

        // Get direction
        ECHECK(hk_AreExCdNBitsSet(bits, &direction));
        if (direction) // Direction is OUT
        {
            return (state == RP_HIGH ? hk_SetExCoNBits(bits) : hk_UnsetExCoNBits(bits));
        }
        else
        {
            return RP_EWIP;
        }
    }
}


int dpin_GetState(rp_dpin_t pin, rp_pinState_t* state)
{
    bool stateOn = false;

    // LEDS
    if (pin < RP_DIO0_P)
    {
        uint32_t bits = (1 << pin);
        ECHECK(hk_AreLedBitsSet(bits, &stateOn));
    }
    // DIO_P
    else if (pin < RP_DIO0_N)
    {
        pin -= RP_DIO0_P;
        uint32_t bits = (1 << pin);
        bool direction;

        // Get direction
        ECHECK(hk_AreExCdPBitsSet(bits, &direction));
        if (direction) // Direction is OUT
        {
            ECHECK(hk_AreExCoPBitsSet(bits, &stateOn));
        }
        else
        {
            ECHECK(hk_AreExCiPBitsSet(bits, &stateOn));
        }
    }
    // DIO_N
    else
    {
        pin -= RP_DIO0_N;
        uint32_t bits = (1 << pin);
        bool direction;

        // Get direction
        ECHECK(hk_AreExCdNBitsSet(bits, &direction));
        if (direction) // Direction is OUT
        {
            ECHECK(hk_AreExCoNBitsSet(bits, &stateOn));
        }
        else
        {
            ECHECK(hk_AreExCiNBitsSet(bits, &stateOn));
        }
    }

    *state = (stateOn ? RP_HIGH : RP_LOW);

    return RP_OK;
}

