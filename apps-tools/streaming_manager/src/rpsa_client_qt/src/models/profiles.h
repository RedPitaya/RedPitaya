#ifndef PROFILES_H
#define PROFILES_H

#include <QJsonObject>
#include <QObject>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

// Named profiles of the settings of a board, kept on this computer (streaming_improvements 4.9):
// saved from a board with the groups chosen, applied to boards with what changes shown first.
// A profile is the JSON of the settings of a board (Export JSON of a board) and a "profile" object
// beside it: name, groups, the board and what it had (caps).
class ProfilesManager : public QObject {
    Q_OBJECT
    // id, name, model, board, saved (ms), groups (the labels), groupList (the keys)
    Q_PROPERTY(QVariantList list READ list NOTIFY changed)

   public:
    static ProfilesManager* instance();

    auto list() const -> QVariantList;

    // the groups of a board now: group, label, summary
    Q_INVOKABLE QVariantList groupsOf(QObject* board);
    // the id of the profile of this name; "" none
    Q_INVOKABLE QString idOf(const QString& name);
    // the settings of the board as a profile (one of the same name is replaced); "" or why not
    Q_INVOKABLE QString save(QObject* board, const QString& name, const QStringList& groups);
    // name, from, saved, groups [{label, rows [{k, v}]}], missing (the groups not in it)
    Q_INVOKABLE QVariantMap details(const QString& id);
    // a board each: ip, name, model, ready, streaming, changes [{label, from, to}], notes
    Q_INVOKABLE QVariantList preview(const QString& id, const QStringList& ips);
    // "" or the boards it was not sent to
    Q_INVOKABLE QString apply(const QString& id, const QStringList& ips);
    Q_INVOKABLE QString rename(const QString& id, const QString& name);
    Q_INVOKABLE QString duplicate(const QString& id);
    Q_INVOKABLE void remove(const QString& id);
    // a file URL of a dialog; "" or why not
    Q_INVOKABLE QString exportJson(const QString& id, const QString& url);
    Q_INVOKABLE QString importJson(const QString& url);
    // a tab of a board asks for a dialog of main.qml: "save" or "apply"
    Q_INVOKABLE void ask(const QString& what, QObject* board) { Q_EMIT asked(what, board); }

   signals:
    void changed();
    void asked(QString what, QObject* board);

   private:
    explicit ProfilesManager(QObject* parent = nullptr);
    auto load() -> void;
    auto store() -> void;
    auto find(const QString& id) const -> int;
    auto freeName(const QString& name) const -> QString;

    QList<QJsonObject> m_profiles;  // {"profile": {...}, "settings": {...}}
};

#endif
