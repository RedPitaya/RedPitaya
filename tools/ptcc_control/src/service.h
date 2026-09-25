/**
 * @file service.h
 * @brief Background service mode of ptcc_control.
 *
 * Keeps the controller open, polls it and exposes both directions over a
 * websocket, so a web application can drive the PTCC without linking the
 * driver (and its embedded interpreter) into the nginx worker.
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef PTCC_SERVICE_H
#define PTCC_SERVICE_H

#include <stdint.h>

#define PTCC_SERVICE_PORT 50001

/**
 * Runs until SIGINT/SIGTERM or a PTCC_STOP command. The controller has to be
 * open already.
 *
 * @param port websocket port to listen on.
 * @param period_ms monitor poll period, clamped to the command throttle.
 * @param device device node to reopen on a lost connection, may be NULL.
 * @param baudrate line rate used when reopening.
 */
int ptcc_service_run(uint16_t port, uint32_t period_ms, const char *device, uint32_t baudrate);

/** Asks the service loop to stop, safe to call from a signal handler. */
void ptcc_service_stop();

#endif  // PTCC_SERVICE_H
