/**
 * @file ptcc_params.h
 * @brief Mirrors the controller service state into parameters of the page.
 *
 * The service publishes every reading and every setting under its own name,
 * for instance PTCC_T_DET or PTCC_SETPOINT. Each of those becomes a parameter
 * with the same name, so the page reads a controller value exactly the way it
 * reads an oscilloscope value. Settings travel back as PTCC_SET_* commands.
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#pragma once

#include <string>
#include <string_view>

/** Creates the parameters. Call once, before the service client starts. */
auto ptccParamsInit() -> void;

/** Feed a value the service published into the parameter of the same name. */
auto ptccParamsOnDouble(std::string_view key, double value) -> void;
auto ptccParamsOnInt(std::string_view key, int value) -> void;
auto ptccParamsOnBool(std::string_view key, bool value) -> void;
auto ptccParamsOnString(std::string_view key, std::string_view value) -> void;

/** Sends settings the page has changed. Call from OnNewParams(). */
auto ptccParamsSendChanges() -> void;

/** Clears the readings, so a lost service does not leave stale numbers on the
 *  page. Settings keep their values: they describe the device, not the link. */
auto ptccParamsOnDisconnect() -> void;
