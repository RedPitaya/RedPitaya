#include "profile_logic.h"
#include <QJsonArray>
#include <QRegularExpression>
#include <QSet>
#include <algorithm>

namespace profiles {

namespace {

const QStringList kDestination = {"adc_pass_mode", "format_sd", "data_type_sd", "samples_limit_sd", "tdms_time_column"};
const QStringList kMemory = {"adc_size", "dac_size", "gpio_in_size", "gpio_out_size"};
// trigger source mask bits (uio_lib TRIG_SRC_*); on a 4-input board bits 0 and 1 are IN3, IN4
// (CStreamSettings::getADCTriggerFullScale, AdcTrigger.qml)
const QStringList kSourceBits = {"DAC A", "DAC B", "IN1", "IN2", "GPIO", "external"};
const QStringList kSourceBits4 = {"IN3", "IN4", "IN1", "IN2", "GPIO", "external"};

// the JSON keeps the names of the enums (CStreamSettings::toJson: ON, FILE, A_1_20, ...)
auto isOn(const QJsonValue& v) -> bool {
    return v.toString() == "ON";
}

auto isFile(const QJsonValue& v) -> bool {
    return v.toString() == "FILE" || v.toString() == "DAC_FILE";
}

auto enumText(const QString& e) -> QString {
    static const QHash<QString, QString> t = {
        {"ON", "on"},         {"OFF", "off"},        {"BIT_16", "16 bit"},     {"BIT_8", "8 bit"},          {"RISING", "rising"},
        {"FALLING", "falling"}, {"A_1_1", "±1 V"},   {"A_1_20", "±20 V"},      {"DAC_REP_OFF", "off"},      {"DAC_REP_ON", "on"},
        {"DAC_REP_INF", "forever"}, {"GPIO_REP_OFF", "off"}, {"GPIO_REP_ON", "on"}, {"GPIO_REP_INF", "forever"}, {"OWN", "own"},
        {"GEN_A", "DAC A"},   {"GEN_B", "DAC B"},    {"OSC1", "ADC 1"},        {"OSC2", "ADC 2"},           {"LA", "GPIO"},
        {"NET", "this PC"},   {"FILE", "SD card"},   {"DAC_NET", "this PC"},   {"DAC_FILE", "SD card"},     {"X1", "x1"},
        {"X5", "x5"},         {"RAW", "raw"},        {"VOLT", "volts"},        {"CODE", "codes"}};
    return t.value(e, e);
}

auto number(const QJsonValue& v) -> QString {
    return QString::number(v.toDouble(), 'g', 10);
}

auto maskText(int mask, const Caps& caps) -> QString {
    const QStringList& names = caps.adcChannels >= 4 ? kSourceBits4 : kSourceBits;
    QStringList s;
    for (int b = 0; b < names.size(); b++)
        if (mask & (1 << b))
            s << names[b];
    return s.isEmpty() ? QString("none") : s.join(", ");
}

auto passText(const QJsonValue& v) -> QString {
    return isFile(v) ? "SD card" : "this PC";
}

auto labelOf(const QString& section, const QString& key) -> QString {
    static const QHash<QString, QString> labels = {
        {"adc_decimation", "Decimation"},
        {"resolution", "Resolution"},
        {"use_calib", "Calibration"},
        {"adc_capture_time", "Capture time"},
        {"adc_trigger_mode", "Trigger"},
        {"adc_trigger_sources", "Trigger sources"},
        {"adc_trigger_level_v", "Trigger level"},
        {"adc_trigger_hysteresis_v", "Hysteresis"},
        {"adc_trigger_level_low", "Level window low (code)"},
        {"adc_trigger_level_high", "Level window high (code)"},
        {"adc_trigger_level_unit", "Level given in"},
        {"adc_trigger_edge", "Trigger edge"},
        {"adc_trigger_blocks", "Blocks a trigger"},
        {"adc_event_source", "ADC events from"},
        {"adc_pass_mode", "Data to"},
        {"format_sd", "SD file format"},
        {"data_type_sd", "SD values"},
        {"samples_limit_sd", "SD samples"},
        {"tdms_time_column", "TDMS time column"},
        {"block_size", "Block"},
        {"adc_size", "ADC memory"},
        {"dac_size", "DAC memory"},
        {"gpio_in_size", "GPIO input memory"},
        {"gpio_out_size", "GPIO output memory"},
        {"dac_rate", "DAC rate"},
        {"dac_pass_mode", "DAC data from"},
        {"file_type_sd", "DAC file type"},
        {"repeat", "DAC repeat"},
        {"repeatCount", "DAC repeats"},
        {"dac_play_gaps", "DAC plays the gaps"},
        {"dac_trigger_sources", "DAC trigger sources"},
        {"dac_event_source", "DAC events from"},
        {"gpio_in_pass_mode", "GPIO input to"},
        {"gpio_out_pass_mode", "GPIO output from"},
        {"gpio_decimation", "GPIO decimation"},
        {"gpio_trigger_sources", "GPIO trigger sources"},
        {"gpio_event_source", "GPIO events from"},
        {"ext_trig_edge", "External trigger edge"},
        {"ext_trig_debounce", "External trigger debounce"},
        {"ext_trig_conditioner", "External trigger conditioner"},
        {"ext_trig_dio0", "External trigger on DIO0"},
    };
    Q_UNUSED(section)
    if (labels.contains(key))
        return labels[key];
    QString s = key;
    s.replace('_', ' ');
    if (!s.isEmpty())
        s[0] = s[0].toUpper();
    return s;
}

auto valueText(const QString& key, const QJsonValue& v, const Caps& caps) -> QString {
    if (v.isUndefined() || v.isNull())
        return "-";
    if (key == "adc_decimation") {
        const double d = std::max(1.0, v.toDouble());
        return number(v) + (caps.adcRate > 0 ? " · " + rateText(caps.adcRate / d) : QString());
    }
    if (key.endsWith("_pass_mode"))
        return passText(v);
    if (key.endsWith("_trigger_sources"))
        return maskText(v.toInt(), caps);
    if (key.endsWith("_size"))
        return bytesText(v.toDouble());
    if (key.endsWith("_v"))
        return number(v) + " V";
    if (key == "dac_rate")
        return rateText(v.toDouble());
    if (key == "samples_limit_sd")
        return v.toDouble() == 0 ? QString("until the stop") : number(v);
    if (v.isString())
        return enumText(v.toString());
    if (v.isBool())
        return v.toBool() ? "yes" : "no";
    return number(v);
}

auto channelText(const QJsonObject& adc, int n, const Caps& caps) -> QString {
    const QString i = QString::number(n);
    if (!isOn(adc.value("channel_state_" + i)))
        return "off";
    QString s = "on · ";
    s += caps.adcRanges ? enumText(adc.value("channel_attenuator_" + i).toString()) : caps.fixedRange;
    if (caps.adcACDC)
        s += " · " + adc.value("channel_ac_dc_" + i).toString();
    return s;
}

auto addNote(QStringList& notes, const QString& note) -> void {
    if (!notes.contains(note))
        notes << note;
}

}  // namespace

auto Caps::fromMap(const QVariantMap& c) -> Caps {
    Caps r;
    r.adcChannels = c.value("adcChannels", 2).toInt();
    r.adcRate = c.value("adcRate").toDouble();
    r.dacRate = c.value("dacRate").toDouble();
    r.adcRanges = c.value("adcRanges", true).toBool();
    r.adcACDC = c.value("adcACDC").toBool();
    r.dac = c.value("dac", true).toBool();
    r.dacGain = c.value("dacGain").toBool();
    r.gpio = c.value("gpio", true).toBool();
    r.extTrigger = c.value("extTrigger", true).toBool();
    r.memorySize = c.value("memorySize").toDouble();
    r.memoryMaxBlock = c.value("memoryMaxBlock").toDouble();
    r.headerSize = c.value("headerSize").toDouble();
    r.fixedRange = c.value("fixedRange", "±1 V").toString();
    r.known = c.value("fromBoard").toBool();
    return r;
}

auto Caps::fromJson(const QJsonObject& o) -> Caps {
    return fromMap(o.toVariantMap());
}

auto Caps::toJson() const -> QJsonObject {
    return QJsonObject{{"adcChannels", adcChannels}, {"adcRate", adcRate}, {"dacRate", dacRate},       {"adcRanges", adcRanges},   {"adcACDC", adcACDC},
                       {"dac", dac},                 {"dacGain", dacGain},       {"gpio", gpio},             {"extTrigger", extTrigger},
                       {"memorySize", memorySize},   {"memoryMaxBlock", memoryMaxBlock}, {"headerSize", headerSize}, {"fixedRange", fixedRange},
                       {"fromBoard", known}};
}

auto groups() -> const QStringList& {
    static const QStringList g = {"inputs", "trigger", "dac", "gpio", "destination"};
    return g;
}

auto groupLabel(const QString& group) -> QString {
    if (group == "inputs")
        return "Inputs";
    if (group == "trigger")
        return "Trigger";
    if (group == "dac")
        return "DAC";
    if (group == "gpio")
        return "GPIO";
    if (group == "destination")
        return "Destination";
    return group;
}

auto groupOf(const QString& section, const QString& key) -> QString {
    if (section == "adc_streaming") {
        if (key.startsWith("adc_trigger_") || key == "adc_event_source")
            return "trigger";
        if (kDestination.contains(key))
            return "destination";
        return "inputs";
    }
    if (section == "ext_trigger")
        return "trigger";
    if (section == "dac_streaming")
        return key == "file_sd" ? QString() : "dac";
    if (section == "gpio_streaming")
        return key == "gpio_out_file_sd" ? QString() : "gpio";
    if (section == "memory_manager")
        return "destination";
    return {};
}

auto adapt(const QJsonObject& profile, const QStringList& groupList, const Caps& from, const QJsonObject& current, const Caps& to) -> Result {
    Result r;
    r.merged = current;
    static const QRegularExpression channelKey("^channel_(state|attenuator|ac_dc)_(\\d)$");
    QSet<int> notInProfile;
    for (auto s = profile.begin(); s != profile.end(); ++s) {
        if (!s.value().isObject())
            continue;
        const QString section = s.key();
        const QJsonObject keys = s.value().toObject();
        QJsonObject out = r.merged.value(section).toObject();
        for (auto k = keys.begin(); k != keys.end(); ++k) {
            const QString key = k.key();
            const QString group = groupOf(section, key);
            if (group.isEmpty() || !groupList.contains(group))
                continue;
            QJsonValue v = k.value();
            if (section == "adc_streaming") {
                const auto m = channelKey.match(key);
                if (m.hasMatch()) {
                    const int n = m.captured(2).toInt();
                    const QString kind = m.captured(1);
                    if (n > from.adcChannels) {
                        if (n <= to.adcChannels)
                            notInProfile.insert(n);
                        continue;
                    }
                    if (n > to.adcChannels) {
                        if (kind == "state" && isOn(v))
                            addNote(r.notes, QString("IN%1 is on in the profile: this board has no IN%1").arg(n));
                        continue;
                    }
                    if (kind == "attenuator" && !to.adcRanges) {
                        if (from.adcRanges)
                            addNote(r.notes, "Input range: this model has one (" + to.fixedRange + "), it keeps it");
                        continue;
                    }
                    if (kind == "ac_dc" && !to.adcACDC) {
                        if (from.adcACDC && v.toString() == "AC")
                            addNote(r.notes, "AC coupling: this model has none, its inputs stay DC");
                        continue;
                    }
                }
            }
            if (section == "dac_streaming" && !to.dac) {
                addNote(r.notes, "DAC: this board has none, the DAC settings are left out");
                continue;
            }
            if (section == "dac_streaming" && key.startsWith("channel_gain_") && !to.dacGain) {
                if (v.toString() == "X5")
                    addNote(r.notes, "DAC gain x5: this model has none, it keeps x1");
                continue;
            }
            if (section == "gpio_streaming" && !to.gpio) {
                addNote(r.notes, "GPIO: this board does not stream GPIO, the GPIO settings are left out");
                continue;
            }
            if (section == "ext_trigger" && to.known && !to.extTrigger) {
                addNote(r.notes, "External trigger: this board has no settings of it, they are left out");
                continue;
            }
            if (key.endsWith("_trigger_sources")) {
                int mask = v.toInt();
                int drop = 0;
                // bits 0, 1: DAC A, B on one board, IN3, IN4 on the other: they do not carry over
                // a profile of an unknown board (an imported file, no ADC rate): read as this board's
                const bool to4 = to.adcChannels >= 4, from4 = from.adcRate > 0 ? from.adcChannels >= 4 : to4;
                if (from4 != to4 || (!to4 && !to.dac))
                    drop |= 0b11;
                if (to.adcChannels < 2)
                    drop |= 0b1000;
                if (!to.gpio)
                    drop |= 0b10000;
                if (to.known && !to.extTrigger)
                    drop |= 0b100000;
                if (mask & drop) {
                    Caps names = from;
                    names.adcChannels = from4 ? 4 : 2;
                    addNote(r.notes, labelOf(section, key) + ": " + maskText(mask & drop, names) + " not on this board, left out");
                    mask &= ~drop;
                }
                v = mask;
            }
            if (key.endsWith("_event_source")) {
                const QString src = v.toString();
                if ((src.startsWith("GEN_") && !to.dac) || (src == "LA" && !to.gpio) || (src == "OSC2" && to.adcChannels < 2)) {
                    addNote(r.notes, labelOf(section, key) + " " + enumText(src) + ": not on this board, it keeps its own");
                    continue;
                }
            }
            if (key == "dac_rate" && to.dacRate > 0 && v.toDouble() > to.dacRate) {
                addNote(r.notes, "DAC rate " + rateText(v.toDouble()) + ": this board goes up to " + rateText(to.dacRate) + ", it keeps its own");
                continue;
            }
            // a level in codes is a voltage of the range of the board it was set on
            if (key == "adc_trigger_level_unit" && v.toString() == "CODE" && isOn(keys.value("adc_trigger_mode")) && from.adcRate > 0 &&
                (from.adcRanges != to.adcRanges || from.fixedRange != to.fixedRange || from.adcRate != to.adcRate))
                addNote(r.notes, "Trigger level in codes: on this model the same codes may be another voltage, check it");
            if (key == "adc_decimation" && from.adcRate > 0 && to.adcRate > 0 && from.adcRate != to.adcRate) {
                const double d = std::max(1.0, v.toDouble());
                addNote(r.notes, "ADC clock " + QString::number(to.adcRate / 1e6, 'g', 6) + " MHz: the same decimation is " + rateText(to.adcRate / d) +
                                     " here, not " + rateText(from.adcRate / d));
            }
            out[key] = v;
        }
        r.merged[section] = out;
    }
    if (!notInProfile.isEmpty()) {
        QStringList ins;
        QList<int> ns = notInProfile.values();
        std::sort(ns.begin(), ns.end());
        for (int n : ns)
            ins << "IN" + QString::number(n);
        addNote(r.notes, ins.join(", ") + ": not in the profile (saved on a " + QString::number(from.adcChannels) + "-input board), they stay as they are");
    }
    // the memory of the board: the sizes of the profile must fit it, the block its DMA
    if (groupList.contains("destination")) {
        QJsonObject mem = r.merged.value("memory_manager").toObject();
        const QJsonObject cur = current.value("memory_manager").toObject();
        double sum = 0;
        for (const auto& k : kMemory)
            sum += mem.value(k).toDouble();
        if (to.memorySize > 0 && sum > to.memorySize) {
            for (const auto& k : kMemory)
                mem[k] = cur.value(k);
            addNote(r.notes, "Memory: the profile takes " + bytesText(sum) + ", this board has " + bytesText(to.memorySize) + ": it keeps its own sizes");
        }
        if (to.memoryMaxBlock > 0 && mem.value("block_size").toDouble() + to.headerSize > to.memoryMaxBlock) {
            addNote(r.notes, "Block " + bytesText(mem.value("block_size").toDouble()) + ": larger than the DMA block of this board, it keeps its own");
            mem["block_size"] = cur.value("block_size");
        }
        r.merged["memory_manager"] = mem;
    }
    // what differs: an input in a row, the rest a key a row, in the order of the groups
    QList<QPair<int, Change>> changes;
    const QJsonObject curAdc = current.value("adc_streaming").toObject();
    const QJsonObject newAdc = r.merged.value("adc_streaming").toObject();
    for (int n = 1; n <= to.adcChannels; n++) {
        const QString a = channelText(curAdc, n, to), b = channelText(newAdc, n, to);
        if (a != b)
            changes.append({0, Change{"IN" + QString::number(n), a, b}});
    }
    for (auto s = r.merged.begin(); s != r.merged.end(); ++s) {
        const QJsonObject keys = s.value().toObject();
        const QJsonObject cur = current.value(s.key()).toObject();
        for (auto k = keys.begin(); k != keys.end(); ++k) {
            if (channelKey.match(k.key()).hasMatch() && s.key() == "adc_streaming")
                continue;
            if (cur.value(k.key()) == k.value())
                continue;
            const int order = static_cast<int>(groups().indexOf(groupOf(s.key(), k.key())));
            changes.append({order, Change{labelOf(s.key(), k.key()), valueText(k.key(), cur.value(k.key()), to), valueText(k.key(), k.value(), to)}});
        }
    }
    std::stable_sort(changes.begin(), changes.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
    for (const auto& c : changes)
        r.changes.append(c.second);
    return r;
}

auto rows(const QString& group, const QJsonObject& settings, const Caps& caps) -> QList<QPair<QString, QString>> {
    QList<QPair<QString, QString>> r;
    const QJsonObject adc = settings.value("adc_streaming").toObject();
    const QJsonObject dac = settings.value("dac_streaming").toObject();
    const QJsonObject gpio = settings.value("gpio_streaming").toObject();
    const QJsonObject mem = settings.value("memory_manager").toObject();
    if (group == "inputs") {
        for (int n = 1; n <= caps.adcChannels; n++)
            r.append({"IN" + QString::number(n), channelText(adc, n, caps)});
        r.append({"Decimation", valueText("adc_decimation", adc.value("adc_decimation"), caps)});
        r.append({"Resolution", valueText("resolution", adc.value("resolution"), caps) + (isOn(adc.value("use_calib")) ? " · calibrated" : "")});
    } else if (group == "trigger") {
        const bool on = isOn(adc.value("adc_trigger_mode"));
        r.append({"Mode", on ? QString("on") : QString("off (continuous)")});
        if (on) {
            r.append({"Sources", maskText(adc.value("adc_trigger_sources").toInt(), caps)});
            if (adc.value("adc_trigger_level_unit").toString() == "VOLT")
                r.append({"Level", number(adc.value("adc_trigger_level_v")) + " V ± " + number(adc.value("adc_trigger_hysteresis_v")) + " V"});
            else
                r.append({"Level window", number(adc.value("adc_trigger_level_low")) + " … " + number(adc.value("adc_trigger_level_high")) + " (codes)"});
            r.append({"Edge", enumText(adc.value("adc_trigger_edge").toString())});
            r.append({"Blocks a trigger", number(adc.value("adc_trigger_blocks"))});
        }
    } else if (group == "dac") {
        r.append({"Data from", passText(dac.value("dac_pass_mode"))});
        r.append({"Rate", rateText(dac.value("dac_rate").toDouble())});
        r.append({"Repeat", enumText(dac.value("repeat").toString())});
    } else if (group == "gpio") {
        r.append({"Input data to", passText(gpio.value("gpio_in_pass_mode"))});
        r.append({"Output data from", passText(gpio.value("gpio_out_pass_mode"))});
        r.append({"Decimation", number(gpio.value("gpio_decimation"))});
    } else if (group == "destination") {
        const bool sd = isFile(adc.value("adc_pass_mode"));
        r.append({"Data to", sd ? QString("SD card") : QString("this PC (network)")});
        if (sd) {
            r.append({"File", adc.value("format_sd").toString().toUpper()});
            r.append({"Samples", valueText("samples_limit_sd", adc.value("samples_limit_sd"), caps)});
        }
        r.append({"Block", bytesText(mem.value("block_size").toDouble())});
        r.append({"ADC memory", bytesText(mem.value("adc_size").toDouble())});
    }
    return r;
}

auto summary(const QString& group, const QJsonObject& settings, const Caps& caps) -> QString {
    QStringList s;
    for (const auto& row : rows(group, settings, caps))
        s << row.first + " " + row.second;
    return s.join("; ");
}

auto bytesText(double b) -> QString {
    if (b >= 1073741824)
        return QString::number(b / 1073741824, 'g', 4) + " GB";
    if (b >= 1048576)
        return QString::number(b / 1048576, 'g', 4) + " MB";
    if (b >= 1024)
        return QString::number(b / 1024, 'g', 4) + " kB";
    return QString::number(b, 'f', 0) + " B";
}

auto rateText(double r) -> QString {
    if (r >= 1e6)
        return QString::number(r / 1e6, 'g', 4) + " MS/s";
    if (r >= 1e3)
        return QString::number(r / 1e3, 'g', 4) + " kS/s";
    return QString::number(r, 'g', 4) + " S/s";
}

}  // namespace profiles
