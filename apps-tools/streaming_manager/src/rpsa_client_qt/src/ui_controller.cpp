#include "ui_controller.h"
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QUrl>
#include <QtQml>
#include "app_settings.h"
#include "models/boardsmodel.h"

#ifndef CLIENT_VERSION
#define CLIENT_VERSION "dev"
#endif

ui_controller::ui_controller(QObject* parent) : QObject{parent} {}

ui_controller* ui_controller::instance() {
    static ui_controller* s_instance = new ui_controller();
    return s_instance;
}

auto ui_controller::version() const -> QString {
    return CLIENT_VERSION;
}

QObject* ui_controller::getBoardsModel() {
    return BoardsModel::instance();
}

void ui_controller::runAll() {
    BoardsModel::instance()->runAll();
}

void ui_controller::stopAll() {
    BoardsModel::instance()->stopAll();
}

void ui_controller::openFolder(const QString& path) {
    const QString dir = AppSettings::expandHome(path);
    QDir().mkpath(dir);
    QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
}

QString ui_controller::readTextFile(const QString& url) {
    QFile f(QUrl(url).isLocalFile() ? QUrl(url).toLocalFile() : url);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = f.errorString();
        return "";
    }
    m_lastError.clear();
    return QString::fromUtf8(f.readAll());
}

bool ui_controller::writeTextFile(const QString& url, const QString& text) {
    QFile f(QUrl(url).isLocalFile() ? QUrl(url).toLocalFile() : url);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        m_lastError = f.errorString();
        return false;
    }
    f.write(text.toUtf8());
    m_lastError.clear();
    return true;
}

QString ui_controller::lastError() const {
    return m_lastError;
}
