#ifndef PROFILE_LOGIC_H
#define PROFILE_LOGIC_H

#include <QJsonObject>
#include <QList>
#include <QPair>
#include <QString>
#include <QStringList>
#include <QVariantMap>

// The settings of a board as groups and their transfer to another board, maybe of another model
// (streaming_improvements 4.9). The settings are the JSON of CStreamSettings::toJson(): sections
// adc_streaming, dac_streaming, memory_manager, ext_trigger, gpio_streaming. Qt Core only: the
// tests build it alone.
namespace profiles {

// what a board has (CBoard::getCaps); known: told by the board, else from the model only
struct Caps {
    int adcChannels = 2;
    double adcRate = 0;
    double dacRate = 0;
    bool adcRanges = true;
    bool adcACDC = false;
    bool dac = true;
    bool dacGain = false;
    bool gpio = true;
    bool extTrigger = true;
    double memorySize = 0;
    double memoryMaxBlock = 0;
    double headerSize = 0;
    QString fixedRange = "±1 V";
    bool known = false;

    static auto fromMap(const QVariantMap& caps) -> Caps;
    static auto fromJson(const QJsonObject& o) -> Caps;
    auto toJson() const -> QJsonObject;
};

// inputs, trigger, dac, gpio, destination: in this order
auto groups() -> const QStringList&;
auto groupLabel(const QString& group) -> QString;
// the group of a key; empty: never copied (the files on the SD card of the board)
auto groupOf(const QString& section, const QString& key) -> QString;

struct Change {
    QString label;
    QString from;
    QString to;
};

struct Result {
    QJsonObject merged;     // the settings of the board with the profile on them
    QList<Change> changes;  // what differs then
    QStringList notes;      // what another model takes otherwise
};

// The groups of a profile (saved on a board with `from`) on the settings `current` of a board with `to`
auto adapt(const QJsonObject& profile, const QStringList& groups, const Caps& from, const QJsonObject& current, const Caps& to) -> Result;

// What a group holds, as rows (label, value), and in a line
auto rows(const QString& group, const QJsonObject& settings, const Caps& caps) -> QList<QPair<QString, QString>>;
auto summary(const QString& group, const QJsonObject& settings, const Caps& caps) -> QString;

auto bytesText(double b) -> QString;
auto rateText(double r) -> QString;

}  // namespace profiles

#endif
