/**
 * @file ptcc_params.cpp
 * @brief See ptcc_params.h.
 */

#include "ptcc_params.h"

#include <map>
#include <memory>
#include <vector>

#include "main.h"
#include "ptcc_service.h"

namespace {

/** Readings and limits: written by the service, read by the page. */
std::map<std::string, std::unique_ptr<CFloatParameter>, std::less<>> g_floats;
std::map<std::string, std::unique_ptr<CIntParameter>, std::less<>> g_ints;
std::map<std::string, std::unique_ptr<CBooleanParameter>, std::less<>> g_bools;
std::map<std::string, std::unique_ptr<CStringParameter>, std::less<>> g_strings;

/** A setting the page may change, and the command that carries it back.
 *  `scale` converts the protocol unit into the one shown on the page. */
struct FloatSetting {
    CFloatParameter *param;
    const char *command;
    float scale;
};

struct IntSetting {
    CIntParameter *param;
    const char *command;
};

std::vector<FloatSetting> g_float_settings;
std::vector<IntSetting> g_int_settings;

/** Page unit per protocol unit, for the few values where they differ. */
std::map<std::string, float, std::less<>> g_scales;

/** Names of the readings that stop meaning anything once the link is gone. */
std::vector<std::string> g_volatile_floats;

/** Wide enough for every quantity the controller reports; the device itself
 *  refuses values outside its USER_MIN/USER_MAX, and the page takes the real
 *  range from the PTCC_*_MIN / PTCC_*_MAX parameters. */
constexpr float FLOAT_MIN = -1000.0F;
constexpr float FLOAT_MAX = 1000.0F;

auto addFloat(const char *name, CBaseParameter::AccessMode access, bool volatile_reading) -> CFloatParameter * {
    auto param = std::make_unique<CFloatParameter>(name, access, 0.0F, 0, FLOAT_MIN, FLOAT_MAX);
    auto *raw = param.get();
    g_floats[name] = std::move(param);
    if (volatile_reading) {
        g_volatile_floats.emplace_back(name);
    }
    return raw;
}

auto addInt(const char *name, CBaseParameter::AccessMode access, int min, int max) -> CIntParameter * {
    auto param = std::make_unique<CIntParameter>(name, access, 0, 0, min, max);
    auto *raw = param.get();
    g_ints[name] = std::move(param);
    return raw;
}

auto addBool(const char *name) -> void {
    g_bools[name] = std::make_unique<CBooleanParameter>(name, CBaseParameter::RO, false, 0);
}

auto addString(const char *name) -> void {
    g_strings[name] = std::make_unique<CStringParameter>(name, CBaseParameter::RO, "", 0);
}

/** Readings, limits and identification, all read-only for the page. */
auto createReadings() -> void {
    // TEC monitor
    for (const char *name : {"PTCC_T_DET", "PTCC_T_INT", "PTCC_I_TEC", "PTCC_U_TEC",
                             "PTCC_I_SUP_P", "PTCC_I_SUP_N", "PTCC_U_SUP_P", "PTCC_U_SUP_N",
                             "PTCC_I_FAN", "PTCC_TH_RES"}) {
        addFloat(name, CBaseParameter::RO, true);
    }

    // Detection module monitor
    for (const char *name : {"PTCC_LABM_U_DET", "PTCC_LABM_U_1ST", "PTCC_LABM_U_OUT",
                             "PTCC_LABM_TEMP", "PTCC_LABM_U_SUP_P", "PTCC_LABM_U_SUP_N",
                             "PTCC_LABM_U_FAN", "PTCC_LABM_I_TEC_P", "PTCC_LABM_I_TEC_N",
                             "PTCC_LABM_TH1", "PTCC_LABM_TH2"}) {
        addFloat(name, CBaseParameter::RO, true);
    }

    // Ranges the attached module accepts. The page uses them to bound its
    // input fields, so they must survive a lost link.
    for (const char *name : {"PTCC_SETPOINT_MIN", "PTCC_SETPOINT_MAX", "PTCC_I_TEC_MAX_MIN",
                             "PTCC_I_TEC_MAX_MAX", "PTCC_SUP_U_P_MIN", "PTCC_SUP_U_P_MAX",
                             "PTCC_SUP_U_N_MIN", "PTCC_SUP_U_N_MAX", "PTCC_LABM_BIAS_U_MIN",
                             "PTCC_LABM_BIAS_U_MAX", "PTCC_LABM_BIAS_I_MIN", "PTCC_LABM_BIAS_I_MAX",
                             "PTCC_LABM_OFFSET_MIN", "PTCC_LABM_OFFSET_MAX"}) {
        addFloat(name, CBaseParameter::RO, false);
    }

    // PWM is a raw device value, so it is bounded by what the module reports
    // in PTCC_PWM_MIN / PTCC_PWM_MAX rather than by a percentage.
    addInt("PTCC_PWM", CBaseParameter::RO, 0, 65535);
    addInt("PTCC_PWM_MIN", CBaseParameter::RO, 0, 65535);
    addInt("PTCC_PWM_MAX", CBaseParameter::RO, 0, 65535);
    addInt("PTCC_STATUS", CBaseParameter::RO, 0, 255);
    addInt("PTCC_LAST_ERROR_CODE", CBaseParameter::RO, -100, 100);
    addInt("PTCC_MODULE_TYPE", CBaseParameter::RO, 0, 16);
    addInt("PTCC_LABM_GAIN_CODE", CBaseParameter::RO, 0, 255);
    addInt("PTCC_LABM_VARACTOR_MIN", CBaseParameter::RO, 0, 4095);
    addInt("PTCC_LABM_VARACTOR_MAX", CBaseParameter::RO, 0, 4095);
    addInt("PTCC_UPTIME", CBaseParameter::RO, 0, 1000000000);

    addBool("PTCC_CONNECTED");
    addBool("PTCC_SUPPLY_ON");
    addBool("PTCC_FAN_ON");
    addBool("PTCC_STATUS_IS_ERROR");
    addBool("PTCC_LABM_PRESENT");
    addBool("PTCC_LABM_GAIN_KNOWN");

    // Identification and free text. PTCC_LABM_GAINS is the list of gains the
    // module accepts, which the page turns into its gain selector.
    for (const char *name : {"PTCC_STATUS_TEXT", "PTCC_LAST_ERROR", "PTCC_DEV_NAME",
                             "PTCC_DEV_SERIAL", "PTCC_DEV_FIRMWARE", "PTCC_DEV_HARDWARE",
                             "PTCC_DEV_TYPE", "PTCC_MOD_NAME", "PTCC_MOD_SERIAL", "PTCC_MOD_TYPE",
                             "PTCC_DET_NAME", "PTCC_DET_SERIAL", "PTCC_LABM_GAINS", "PTCC_VERSION",
                             "PTCC_PROTOCOL", "PTCC_PORT"}) {
        addString(name);
    }
}

/** Settings: the page writes them, the service reports back what the device
 *  actually took, which may be a clamped or rounded value. */
auto createSettings() -> void {
    const struct {
        const char *name;
        const char *command;
        float scale;
    } floats[] = {
        {"PTCC_SETPOINT", "PTCC_SET_SETPOINT", 1.0F},
        {"PTCC_I_TEC_MAX", "PTCC_SET_I_TEC_MAX", 1.0F},
        {"PTCC_SUP_U_P", "PTCC_SET_SUP_U_P", 1.0F},
        {"PTCC_SUP_U_N", "PTCC_SET_SUP_U_N", 1.0F},
        {"PTCC_LABM_BIAS_U", "PTCC_SET_LABM_BIAS_U", 1.0F},
        // The protocol carries amperes, the panel shows milliamperes.
        {"PTCC_LABM_BIAS_I", "PTCC_SET_LABM_BIAS_I", 1000.0F},
        {"PTCC_LABM_OFFSET", "PTCC_SET_LABM_OFFSET", 1.0F},
        {"PTCC_LABM_GAIN", "PTCC_SET_LABM_GAIN", 1.0F},
    };

    for (const auto &entry : floats) {
        g_float_settings.push_back(
            {addFloat(entry.name, CBaseParameter::RW, false), entry.command, entry.scale});
        if (entry.scale != 1.0F) {
            g_scales[entry.name] = entry.scale;
            // The range of a converted setting is quoted in the same unit.
            g_scales[std::string(entry.name) + "_MIN"] = entry.scale;
            g_scales[std::string(entry.name) + "_MAX"] = entry.scale;
        }
    }

    const struct {
        const char *name;
        const char *command;
        int min;
        int max;
    } ints[] = {
        // AUTO / OFF / ON, as rp_ptcc_ctrl_t orders them
        {"PTCC_TEC_CTRL", "PTCC_SET_COOLER", 0, 2},
        {"PTCC_SUP_CTRL", "PTCC_SET_SUP_CTRL", 0, 2},
        {"PTCC_FAN_CTRL", "PTCC_SET_FAN", 0, 2},
        {"PTCC_PWM_SET", "PTCC_SET_PWM", 0, 65535},
        {"PTCC_LABM_VARACTOR", "PTCC_SET_LABM_VARACTOR", 0, 4095},
        {"PTCC_LABM_TRANS", "PTCC_SET_LABM_TRANS", 0, 1},
        {"PTCC_LABM_COUPLING", "PTCC_SET_LABM_COUPLING", 0, 1},
        {"PTCC_LABM_BW", "PTCC_SET_LABM_BW", 0, 2},
    };

    for (const auto &entry : ints) {
        g_int_settings.push_back(
            {addInt(entry.name, CBaseParameter::RW, entry.min, entry.max), entry.command});
    }
}

}  // namespace

auto ptccParamsInit() -> void {
    if (!g_floats.empty()) {
        return;
    }
    createReadings();
    createSettings();
}

auto ptccParamsOnDouble(std::string_view key, double value) -> void {
    const auto found = g_floats.find(key);
    if (found == g_floats.end()) {
        return;
    }

    const auto scale = g_scales.find(key);
    if (scale != g_scales.end()) {
        value *= scale->second;
    }

    // A setting the page is editing right now is left alone until its own
    // value has been sent, otherwise the field fights the person typing in it.
    if (found->second->GetAccessMode() == CBaseParameter::RW && found->second->IsNewValue()) {
        return;
    }
    found->second->SendValue(static_cast<float>(value));
}

auto ptccParamsOnInt(std::string_view key, int value) -> void {
    const auto found = g_ints.find(key);
    if (found == g_ints.end()) {
        // Whole numbers arrive as int even where the value is a float one,
        // such as a setpoint typed without decimals.
        ptccParamsOnDouble(key, static_cast<double>(value));
        return;
    }

    if (found->second->GetAccessMode() == CBaseParameter::RW && found->second->IsNewValue()) {
        return;
    }
    found->second->SendValue(value);
}

auto ptccParamsOnBool(std::string_view key, bool value) -> void {
    const auto found = g_bools.find(key);
    if (found != g_bools.end()) {
        found->second->SendValue(value);
    }
}

auto ptccParamsOnString(std::string_view key, std::string_view value) -> void {
    const auto found = g_strings.find(key);
    if (found != g_strings.end()) {
        found->second->SendValue(std::string(value));
    }
}

auto ptccParamsSendChanges() -> void {
    for (const auto &setting : g_float_settings) {
        if (setting.param->IsNewValue()) {
            ptccServiceSend(setting.command, setting.param->NewValue() / setting.scale);
            setting.param->Update();
        }
    }

    for (const auto &setting : g_int_settings) {
        if (setting.param->IsNewValue()) {
            ptccServiceSend(setting.command, setting.param->NewValue());
            setting.param->Update();
        }
    }
}

auto ptccParamsOnDisconnect() -> void {
    for (const auto &name : g_volatile_floats) {
        g_floats[name]->SendValue(0.0F);
    }

    const auto connected = g_bools.find("PTCC_CONNECTED");
    if (connected != g_bools.end()) {
        connected->second->SendValue(false);
    }

    const auto status = g_strings.find("PTCC_STATUS_TEXT");
    if (status != g_strings.end()) {
        status->second->SendValue("");
    }
}
