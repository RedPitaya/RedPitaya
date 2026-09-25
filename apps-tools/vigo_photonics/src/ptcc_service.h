/**
 * @file ptcc_service.h
 * @brief Runs ptcc_control --service alongside the application.
 *
 * The PTCC driver embeds a Python interpreter, and this application is loaded
 * and unloaded by the nginx worker, which an embedded interpreter does not
 * survive. The controller therefore lives in its own process: started when the
 * application starts, stopped when it exits, and reached over a websocket.
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#pragma once

/** State published as PTCC_SERVICE_STATE. */
enum ptcc_service_state_t {
    PTCC_SERVICE_STOPPED = 0,    ///< Not running, and we could not start it
    PTCC_SERVICE_STARTING = 1,   ///< Process is up, no connection yet
    PTCC_SERVICE_CONNECTED = 2,  ///< Socket is open, no answer yet
    PTCC_SERVICE_ALIVE = 3       ///< Answers ping, so the service works
};

/** Starts the service, or attaches to one that is already listening. */
auto startPtccService() -> void;

/** Stops the service, but only the process this application started. */
auto stopPtccService() -> void;

/** Pings the service and refreshes the published state. Call periodically. */
auto updatePtccService() -> void;

/** Sends one command to the service. False when the link is down, in which
 *  case the value is dropped: the device state is republished on reconnect
 *  anyway, so a queued write would only fight it. */
auto ptccServiceSend(const char *key, float value) -> bool;
auto ptccServiceSend(const char *key, int value) -> bool;
