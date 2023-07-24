/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server dpin SCPI commands interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */


#ifndef DPIN_H_
#define DPIN_H_

scpi_result_t RP_DigitalPinReset(scpi_t * context);
scpi_result_t RP_DigitalPinStateQ(scpi_t * context);
scpi_result_t RP_DigitalPinState(scpi_t * context);
scpi_result_t RP_DigitalPinDirection(scpi_t * context);
scpi_result_t RP_DigitalPinDirectionQ(scpi_t *context);

#endif /* DPIN_H_ */
