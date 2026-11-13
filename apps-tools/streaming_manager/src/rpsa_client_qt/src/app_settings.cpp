#include "app_settings.h"
#include <QDir>
#include <QHostAddress>
#include <QQmlEngine>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QUrl>

namespace {

auto settings() -> QSettings {
    return QSettings("RedPitaya", "rpsa_client_qt");
}

}  // namespace

AppSettings* AppSettings::instance() {
    static AppSettings* s_instance = new AppSettings();
    return s_instance;
}

AppSettings::AppSettings() : QObject(nullptr) {
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
}

auto AppSettings::put(const char* key, const QVariant& value) -> void {
    auto s = settings();
    if (s.value(key) == value)
        return;
    s.setValue(key, value);
    Q_EMIT changed();
}

auto AppSettings::discovery() const -> bool {
    return settings().value("discovery", true).toBool();
}

auto AppSettings::setDiscovery(bool on) -> void {
    put("discovery", on);
}

auto AppSettings::manualHosts() const -> QStringList {
    return settings().value("manualHosts").toStringList();
}

auto AppSettings::recordDir() const -> QString {
    const auto docs = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    return settings().value("recordDir", QDir(docs).filePath("RedPitaya/streaming/{board}")).toString();
}

auto AppSettings::setRecordDir(const QString& dir) -> void {
    put("recordDir", dir.trimmed());
}

auto AppSettings::textSize() const -> int {
    const int v = settings().value("textSize", 0).toInt();
    return v == 2 || v == 4 ? v : 0;
}

auto AppSettings::setTextSize(int add) -> void {
    put("textSize", add == 2 || add == 4 ? add : 0);
}

auto AppSettings::view() const -> QString {
    return settings().value("view", "cards").toString() == "list" ? "list" : "cards";
}

auto AppSettings::setView(const QString& view) -> void {
    put("view", view == "list" ? "list" : "cards");
}

auto AppSettings::previewRate() const -> int {
    return qBound(1, settings().value("previewRate", 2).toInt(), 10);
}

auto AppSettings::setPreviewRate(int rate) -> void {
    put("previewRate", qBound(1, rate, 10));
}

auto AppSettings::previewPoints() const -> int {
    return qBound(200, settings().value("previewPoints", 1000).toInt(), 4000);
}

auto AppSettings::setPreviewPoints(int points) -> void {
    put("previewPoints", qBound(200, points, 4000));
}

auto AppSettings::startAllAdc() const -> bool {
    return settings().value("startAllAdc", true).toBool();
}

auto AppSettings::setStartAllAdc(bool on) -> void {
    put("startAllAdc", on);
}

auto AppSettings::stopAllOnLoss() const -> bool {
    return settings().value("stopAllOnLoss", false).toBool();
}

auto AppSettings::setStopAllOnLoss(bool on) -> void {
    put("stopAllOnLoss", on);
}

auto AppSettings::expandHome(const QString& path) -> QString {
    if (path == "~")
        return QDir::homePath();
    if (path.startsWith("~/") || path.startsWith("~\\"))
        return QDir::homePath() + path.mid(1);
    return path;
}

QString AppSettings::recordDirFor(const QString& board) const {
    QString dir = recordDir();
    dir.replace("{board}", board);
    return QDir::cleanPath(expandHome(dir));
}

QString AppSettings::addManualHost(const QString& host) {
    const QString h = host.trimmed();
    if (h.isEmpty())
        return "Type the address of the board.";
    QHostAddress address;
    static const QRegularExpression name("^[A-Za-z0-9]([A-Za-z0-9.-]{0,252})$");
    if (!address.setAddress(h) && !name.match(h).hasMatch())
        return "Not an IP address or a host name.";
    if (address.protocol() == QAbstractSocket::IPv6Protocol)
        return "IPv6 is not supported: use the IPv4 address of the board.";
    auto list = manualHosts();
    if (list.contains(h, Qt::CaseInsensitive))
        return "The board is in the list already.";
    list.append(h);
    put("manualHosts", list);
    Q_EMIT manualHostAdded(h);
    return "";
}

void AppSettings::removeManualHost(const QString& host) {
    auto list = manualHosts();
    if (list.removeAll(host) == 0)
        return;
    put("manualHosts", list);
    Q_EMIT manualHostRemoved(host);
}

bool AppSettings::isManualHost(const QString& host) const {
    return manualHosts().contains(host, Qt::CaseInsensitive);
}

QString AppSettings::urlToPath(const QString& url) const {
    const QUrl u(url);
    return u.isLocalFile() ? QDir::toNativeSeparators(u.toLocalFile()) : url;
}

QString AppSettings::pathToUrl(const QString& path) const {
    return QUrl::fromLocalFile(expandHome(path)).toString();
}
