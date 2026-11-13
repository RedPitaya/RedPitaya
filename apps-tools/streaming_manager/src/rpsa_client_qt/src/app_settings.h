#ifndef APP_SETTINGS_H
#define APP_SETTINGS_H

#include <QObject>
#include <QStringList>

// The settings of the application as a whole, kept on this computer (QSettings).
class AppSettings : public QObject {
    Q_OBJECT

    // the boards that answer the broadcast search (UDP 18902) are added by themselves
    Q_PROPERTY(bool discovery READ discovery WRITE setDiscovery NOTIFY changed)
    // the boards added by their address (another subnet does not hear the broadcast)
    Q_PROPERTY(QStringList manualHosts READ manualHosts NOTIFY changed)
    // the folder of the recordings of a board that has none of its own; {board} - its name
    Q_PROPERTY(QString recordDir READ recordDir WRITE setRecordDir NOTIFY changed)
    // 0, 2 or 4: pixels added to every text size
    Q_PROPERTY(int textSize READ textSize WRITE setTextSize NOTIFY changed)
    // "cards" or "list"
    Q_PROPERTY(QString view READ view WRITE setView NOTIFY changed)
    // the signal preview: blocks a second and points across
    Q_PROPERTY(int previewRate READ previewRate WRITE setPreviewRate NOTIFY changed)
    Q_PROPERTY(int previewPoints READ previewPoints WRITE setPreviewPoints NOTIFY changed)
    // what Start all starts, besides the ADC
    Q_PROPERTY(bool startAllAdc READ startAllAdc WRITE setStartAllAdc NOTIFY changed)
    // Stop all when a board loses data
    Q_PROPERTY(bool stopAllOnLoss READ stopAllOnLoss WRITE setStopAllOnLoss NOTIFY changed)

   public:
    static AppSettings* instance();

    auto discovery() const -> bool;
    auto setDiscovery(bool on) -> void;
    auto manualHosts() const -> QStringList;
    auto recordDir() const -> QString;
    auto setRecordDir(const QString& dir) -> void;
    auto textSize() const -> int;
    auto setTextSize(int add) -> void;
    auto view() const -> QString;
    auto setView(const QString& view) -> void;
    auto previewRate() const -> int;
    auto setPreviewRate(int rate) -> void;
    auto previewPoints() const -> int;
    auto setPreviewPoints(int points) -> void;
    auto startAllAdc() const -> bool;
    auto setStartAllAdc(bool on) -> void;
    auto stopAllOnLoss() const -> bool;
    auto setStopAllOnLoss(bool on) -> void;

    // The recording folder for a board name, "~" expanded.
    Q_INVOKABLE QString recordDirFor(const QString& board) const;

    // An empty string when the host is added, else why it is not.
    Q_INVOKABLE QString addManualHost(const QString& host);
    Q_INVOKABLE void removeManualHost(const QString& host);
    Q_INVOKABLE bool isManualHost(const QString& host) const;
    // A folder chosen in the dialog of the system, as a local path ("" when cancelled)
    Q_INVOKABLE QString urlToPath(const QString& url) const;
    Q_INVOKABLE QString pathToUrl(const QString& path) const;

    static auto expandHome(const QString& path) -> QString;

   signals:
    void changed();
    void manualHostAdded(QString host);
    void manualHostRemoved(QString host);

   private:
    AppSettings();
    Q_DISABLE_COPY_MOVE(AppSettings)

    auto put(const char* key, const QVariant& value) -> void;
};

#endif
