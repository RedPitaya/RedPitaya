#include "profiles.h"
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QQmlEngine>
#include <QSettings>
#include <QUrl>
#include <QUuid>
#include "profile_logic.h"
#include "src/models/board.h"
#include "src/models/boardsmodel.h"
#include "settings_lib/stream_settings.h"

namespace {

auto settingsStore() -> QSettings {
    return QSettings("RedPitaya", "rpsa_client_qt");
}

auto parse(const QString& text) -> QJsonObject {
    return QJsonDocument::fromJson(text.toUtf8()).object();
}

auto localPath(const QString& url) -> QString {
    const QUrl u(url);
    return u.isLocalFile() ? u.toLocalFile() : url;
}

auto boardOf(QObject* o) -> CBoard* {
    return qobject_cast<CBoard*>(o);
}

}  // namespace

ProfilesManager* ProfilesManager::instance() {
    static ProfilesManager* s = new ProfilesManager();
    return s;
}

ProfilesManager::ProfilesManager(QObject* parent) : QObject(parent) {
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
    load();
}

auto ProfilesManager::load() -> void {
    m_profiles.clear();
    const QJsonArray a = QJsonDocument::fromJson(settingsStore().value("profiles/list").toByteArray()).array();
    for (const auto& v : a)
        if (v.isObject() && v.toObject().value("profile").isObject() && v.toObject().value("settings").isObject())
            m_profiles.append(v.toObject());
}

auto ProfilesManager::store() -> void {
    QJsonArray a;
    for (const auto& p : m_profiles)
        a.append(p);
    settingsStore().setValue("profiles/list", QJsonDocument(a).toJson(QJsonDocument::Compact));
    Q_EMIT changed();
}

auto ProfilesManager::find(const QString& id) const -> int {
    for (int i = 0; i < m_profiles.size(); i++)
        if (m_profiles[i].value("profile").toObject().value("id").toString() == id)
            return i;
    return -1;
}

auto ProfilesManager::freeName(const QString& name) const -> QString {
    auto taken = [this](const QString& n) {
        for (const auto& p : m_profiles)
            if (p.value("profile").toObject().value("name").toString() == n)
                return true;
        return false;
    };
    if (!taken(name))
        return name;
    for (int i = 2;; i++)
        if (!taken(name + " (" + QString::number(i) + ")"))
            return name + " (" + QString::number(i) + ")";
}

auto ProfilesManager::list() const -> QVariantList {
    QVariantList r;
    for (const auto& p : m_profiles) {
        const QJsonObject m = p.value("profile").toObject();
        QStringList labels;
        const QStringList g = m.value("groups").toVariant().toStringList();
        for (const auto& x : profiles::groups())
            if (g.contains(x))
                labels << profiles::groupLabel(x);
        r.append(QVariantMap{{"id", m.value("id").toString()},
                             {"name", m.value("name").toString()},
                             {"model", m.value("model").toString()},
                             {"board", m.value("board").toString()},
                             {"saved", m.value("saved").toDouble()},
                             {"groups", labels.join(" · ")},
                             {"groupList", g}});
    }
    return r;
}

QVariantList ProfilesManager::groupsOf(QObject* o) {
    QVariantList r;
    auto* b = boardOf(o);
    if (!b || !b->settingsLoaded())
        return r;
    const QJsonObject s = parse(b->settingsJson());
    const auto caps = profiles::Caps::fromMap(b->getCaps());
    for (const auto& g : profiles::groups()) {
        // what the board has not, it has no settings of
        if ((g == "dac" && !caps.dac) || (g == "gpio" && !caps.gpio))
            continue;
        r.append(QVariantMap{{"group", g}, {"label", profiles::groupLabel(g)}, {"summary", profiles::summary(g, s, caps)}});
    }
    return r;
}

QString ProfilesManager::idOf(const QString& name) {
    for (const auto& p : m_profiles)
        if (p.value("profile").toObject().value("name").toString() == name.trimmed())
            return p.value("profile").toObject().value("id").toString();
    return "";
}

QString ProfilesManager::save(QObject* o, const QString& nameIn, const QStringList& groups) {
    auto* b = boardOf(o);
    const QString name = nameIn.trimmed();
    if (name.isEmpty())
        return "Give the profile a name";
    if (!b || !b->settingsLoaded())
        return "The settings of the board are not read yet";
    if (groups.isEmpty())
        return "Choose what the profile holds";
    const QJsonObject settings = parse(b->settingsJson());
    if (settings.isEmpty())
        return "The settings of the board are not read yet";
    const QString old = idOf(name);
    QJsonObject meta{{"id", old.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces) : old},
                     {"name", name},
                     {"model", b->getModel()},
                     {"board", b->getName()},
                     {"saved", static_cast<double>(QDateTime::currentMSecsSinceEpoch())},
                     {"groups", QJsonArray::fromStringList(groups)},
                     {"caps", profiles::Caps::fromMap(b->getCaps()).toJson()}};
    const QJsonObject p{{"profile", meta}, {"settings", settings}};
    const int i = find(meta.value("id").toString());
    if (i >= 0)
        m_profiles[i] = p;
    else
        m_profiles.append(p);
    store();
    return "";
}

QVariantMap ProfilesManager::details(const QString& id) {
    const int i = find(id);
    if (i < 0)
        return {};
    const QJsonObject m = m_profiles[i].value("profile").toObject();
    const QJsonObject s = m_profiles[i].value("settings").toObject();
    const auto caps = profiles::Caps::fromJson(m.value("caps").toObject());
    const QStringList g = m.value("groups").toVariant().toStringList();
    QVariantList groups;
    QStringList missing;
    for (const auto& x : profiles::groups()) {
        if (!g.contains(x)) {
            missing << profiles::groupLabel(x);
            continue;
        }
        QVariantList rows;
        for (const auto& r : profiles::rows(x, s, caps))
            rows.append(QVariantMap{{"k", r.first}, {"v", r.second}});
        groups.append(QVariantMap{{"label", profiles::groupLabel(x)}, {"rows", rows}});
    }
    const QString board = m.value("board").toString();
    const QString model = m.value("model").toString();
    return QVariantMap{{"name", m.value("name").toString()},
                       {"from", (board.isEmpty() ? QString() : board + " · ") + (model.isEmpty() ? QString("a JSON file") : model)},
                       {"saved", m.value("saved").toDouble()},
                       {"groups", groups},
                       {"missing", missing.join(", ")}};
}

QVariantList ProfilesManager::preview(const QString& id, const QStringList& ips) {
    QVariantList r;
    const int i = find(id);
    if (i < 0)
        return r;
    const QJsonObject m = m_profiles[i].value("profile").toObject();
    const QJsonObject s = m_profiles[i].value("settings").toObject();
    const auto from = profiles::Caps::fromJson(m.value("caps").toObject());
    const QStringList g = m.value("groups").toVariant().toStringList();
    for (const auto& ip : ips) {
        auto* b = boardOf(BoardsModel::instance()->boardByIp(ip));
        if (!b)
            continue;
        QVariantMap one{{"ip", ip}, {"name", b->getName()}, {"model", b->getModel()}, {"streaming", b->getIsADCStarted()}};
        const bool ready = b->getConfigManagerConnected() && b->settingsLoaded();
        one["ready"] = ready;
        if (ready) {
            const auto res = profiles::adapt(s, g, from, parse(b->settingsJson()), profiles::Caps::fromMap(b->getCaps()));
            QVariantList changes;
            for (const auto& c : res.changes)
                changes.append(QVariantMap{{"label", c.label}, {"from", c.from}, {"to", c.to}});
            one["changes"] = changes;
            one["notes"] = res.notes;
        }
        r.append(one);
    }
    return r;
}

QString ProfilesManager::apply(const QString& id, const QStringList& ips) {
    const int i = find(id);
    if (i < 0)
        return "The profile is not there any more";
    const QJsonObject m = m_profiles[i].value("profile").toObject();
    const QJsonObject s = m_profiles[i].value("settings").toObject();
    const auto from = profiles::Caps::fromJson(m.value("caps").toObject());
    const QStringList g = m.value("groups").toVariant().toStringList();
    QStringList failed;
    for (const auto& ip : ips) {
        auto* b = boardOf(BoardsModel::instance()->boardByIp(ip));
        if (!b)
            continue;
        if (!b->getConfigManagerConnected() || !b->settingsLoaded()) {
            failed << b->getName();
            continue;
        }
        const auto res = profiles::adapt(s, g, from, parse(b->settingsJson()), profiles::Caps::fromMap(b->getCaps()));
        if (res.changes.isEmpty())
            continue;
        if (!b->loadSettingsJson(QString::fromUtf8(QJsonDocument(res.merged).toJson(QJsonDocument::Compact)), "the profile \"" + m.value("name").toString() + "\""))
            failed << b->getName();
    }
    return failed.isEmpty() ? QString() : "Not sent to " + failed.join(", ");
}

QString ProfilesManager::rename(const QString& id, const QString& nameIn) {
    const QString name = nameIn.trimmed();
    const int i = find(id);
    if (i < 0)
        return "The profile is not there any more";
    if (name.isEmpty())
        return "Give the profile a name";
    const QString other = idOf(name);
    if (!other.isEmpty() && other != id)
        return "A profile of this name is there";
    QJsonObject m = m_profiles[i].value("profile").toObject();
    m["name"] = name;
    m_profiles[i]["profile"] = m;
    store();
    return "";
}

QString ProfilesManager::duplicate(const QString& id) {
    const int i = find(id);
    if (i < 0)
        return "";
    QJsonObject p = m_profiles[i];
    QJsonObject m = p.value("profile").toObject();
    m["id"] = QUuid::createUuid().toString(QUuid::WithoutBraces);
    m["name"] = freeName(m.value("name").toString());
    p["profile"] = m;
    m_profiles.insert(i + 1, p);
    store();
    return m.value("id").toString();
}

void ProfilesManager::remove(const QString& id) {
    const int i = find(id);
    if (i < 0)
        return;
    m_profiles.removeAt(i);
    store();
}

QString ProfilesManager::exportJson(const QString& id, const QString& url) {
    const int i = find(id);
    if (i < 0)
        return "The profile is not there any more";
    // the settings as Export JSON of a board writes them (Import JSON of a board takes the file),
    // the profile beside them
    QJsonObject out = m_profiles[i].value("settings").toObject();
    out["profile"] = m_profiles[i].value("profile");
    QFile f(localPath(url));
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return "Not saved: " + f.errorString();
    f.write(QJsonDocument(out).toJson(QJsonDocument::Indented));
    return f.error() == QFile::NoError ? QString() : "Not saved: " + f.errorString();
}

QString ProfilesManager::importJson(const QString& url) {
    const QString path = localPath(url);
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return "Not read: " + f.errorString();
    QJsonObject root = QJsonDocument::fromJson(f.readAll()).object();
    QJsonObject meta = root.value("profile").toObject();
    root.remove("profile");
    CStreamSettings check;
    if (root.value("adc_streaming").toObject().isEmpty() || !check.parseJson(QString::fromUtf8(QJsonDocument(root).toJson()).toStdString()))
        return "Not a settings file of the streaming application";
    // a file of a board (Export JSON): all the groups, what the board had is not known
    if (meta.isEmpty()) {
        meta["name"] = QFileInfo(path).completeBaseName();
        meta["groups"] = QJsonArray::fromStringList(profiles::groups());
        profiles::Caps caps;
        caps.adcChannels = 4;
        meta["caps"] = caps.toJson();
    }
    meta["id"] = QUuid::createUuid().toString(QUuid::WithoutBraces);
    meta["name"] = freeName(meta.value("name").toString().trimmed().isEmpty() ? QString("Imported") : meta.value("name").toString().trimmed());
    if (!meta.contains("saved"))
        meta["saved"] = static_cast<double>(QDateTime::currentMSecsSinceEpoch());
    m_profiles.append(QJsonObject{{"profile", meta}, {"settings", root}});
    store();
    return "";
}
