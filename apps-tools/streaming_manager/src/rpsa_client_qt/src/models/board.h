#ifndef BOARD_H
#define BOARD_H

#include <QDateTime>
#include <QFile>
#include <QObject>
#include <QStorageInfo>
#include <QTimer>
#include <QVariantMap>
#include <atomic>
#include <memory>
#include <mutex>
#include <thread>

#include "broadcast_lib/asio_broadcast_socket.h"
#include "config_net_lib/client_net_config_manager.h"
#include "net_lib/asio_net.h"
#include "src/models/board_models.h"
#include "src/models/consolemodel.h"
#include "streaming_lib/streaming_file.h"
#include "uio_lib/memory_manager.h"

// the GPIO output file (reader_lib): not included here, its header does not go after Qt (emit)
class CGpioFileReader;

// One board: its settings connection, its ADC stream to a file on this computer and what the
// dashboard shows of it. Lives on the GUI thread; the network handlers post their work to it.
class CBoard : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString ip READ getIP CONSTANT)
    // the board as the user knows it: rp-xxxxxx from its MAC, else the address it was added by
    Q_PROPERTY(QString name READ getName NOTIFY modelChanged)
    // added by its address, not found by the broadcast
    Q_PROPERTY(bool manual READ isManual NOTIFY sourceChanged)
    Q_PROPERTY(bool isOnline READ isOnline NOTIFY isOnlineChanged)
    Q_PROPERTY(bool isConfigConnected READ getConfigManagerConnected NOTIFY configManagerConnectedChanged)
    // the settings connection: connecting, connected, taken (another client took the board),
    // refused (no streaming application on the board), unreachable (no answer)
    Q_PROPERTY(QString link READ getLink NOTIFY configManagerConnectedChanged)
    Q_PROPERTY(bool isMaster READ getIsMaster NOTIFY isMasterChanged)
    Q_PROPERTY(QString model READ getModel NOTIFY modelChanged)
    // what the board has: adcChannels, adcRate, adcBits, dacChannels, dacRate, adcRanges, adcACDC,
    // dacGain, adc, dac, gpio, adcBlockMode, dacTriggerOutputs, extTrigger, extTriggerLevel,
    // fixedRange, memorySize, memoryMaxBlock, headerSize, fromBoard (told by the board itself)
    Q_PROPERTY(QVariantMap caps READ getCaps NOTIFY modelChanged)
    // stopped, starting, streaming, sd (the board writes its SD card), armed, stopping
    Q_PROPERTY(QString state READ getState NOTIFY stateChanged)
    Q_PROPERTY(bool isADCStarted READ getIsADCStarted NOTIFY stateChanged)
    // received, speed (bytes a second), samples (per channel), lostBoard, lostFile, file,
    // fileRate (the write speed of the file, bytes a second, 0 before it is measured), fileQueue (0..1)
    Q_PROPERTY(QVariantMap stats READ getStats NOTIFY statsChanged)
    // the ADC trigger while streaming in the trigger mode: known (the board answered), armed,
    // count (triggers the FPGA has seen)
    Q_PROPERTY(QVariantMap trigger READ getTrigger NOTIFY triggerChanged)
    // The recording of the ADC (streaming_improvements 1.5): dest "sd" (the board writes its card:
    // its RecordStatus, asked every second) or "pc" (this computer writes the file); known,
    // running, limit, samples (per channel), rate (samples a second, 0 unknown), written, write
    // (bytes a second the disk takes), queue (0..1), lostBoard, lostQueue, free, total (bytes of
    // the disk of the file), file
    Q_PROPERTY(QVariantMap record READ getRecord NOTIFY recordChanged)
    // The last error of the streams of the board (streaming_improvements 2.2): text, time
    // (hh:mm:ss), count (errors since it was cleared); empty while there is none. Cleared by a
    // start that goes or by dismissError().
    Q_PROPERTY(QVariantMap error READ getError NOTIFY errorChanged)
    // The DAC (streaming_improvements 4.17): state stopped, connecting, playing, stopping, board
    // (the board stopped it while it played), error; source pc (a file of this computer sent to
    // the board) or sd (a file of its SD card); file, path, sent (bytes), rate (bytes a second),
    // pass, passes (0: until the stop), onePack (the file fits a block: the board repeats it),
    // done (the whole file is sent, the board plays the rest), seconds, text (why it stopped)
    Q_PROPERTY(QVariantMap dacState READ getDacState NOTIFY dacChanged)
    // The file of this computer the DAC plays (pcValue dacFile), read on a thread: path, reading,
    // ok, error, format, channels (1, 2: OUT1, OUT2; 3 both), bits, bytes (of samples, all channels),
    // samples (per channel), fileRate (WAV: its sample rate), size (of the file)
    Q_PROPERTY(QVariantMap dacFile READ getDacFile NOTIFY dacFileChanged)
    // The GPIO (streaming_improvements 4.17), as rpsa_client -G: in and out, each a map of
    // state (stopped, connecting, recording (in) or playing (out), sd (in: the board writes its
    // SD card), stopping, board (the board stopped it), error), source (pc, sd), file, path,
    // seconds, text (why it stopped), armed (waits for a trigger, as the board says).
    // in: packs, samples, words, bytes, lost, waited, rle, limitKind, limit (samples or seconds),
    // frac, rate (samples a second); sd: known, written, part. out: words, bytes, rate (bytes a
    // second), whole (the whole file is sent), waited (clocks the output waited for data, told by
    // the board at its stop), passes (0: until the stop)
    Q_PROPERTY(QVariantMap gpioState READ getGpioState NOTIFY gpioChanged)
    // The file of this computer the GPIO output plays (pcValue gpioOutFile), read on a thread:
    // path, reading, ok, error, kind (in: a recording of the input, out: output words), rle,
    // words, samples, clock, decimation, size
    Q_PROPERTY(QVariantMap gpioFile READ getGpioFile NOTIFY gpioFileChanged)
    // bumped when the settings of the board change: bindings that read value() depend on it
    Q_PROPERTY(int settingsRev READ settingsRev NOTIFY settingsChanged)
    Q_PROPERTY(bool settingsLoaded READ settingsLoaded NOTIFY settingsChanged)
    // the same for the settings of this board kept on this computer (pcValue)
    Q_PROPERTY(int pcRev READ pcRev NOTIFY pcSettingsChanged)
    // the preview shows the signal (else the console)
    Q_PROPERTY(bool chartEnable READ getChartEnable WRITE setChartEnable NOTIFY pcSettingsChanged)
    // the preview fits its Y range to the signal (else the full range of the inputs)
    Q_PROPERTY(bool previewFit READ previewFit WRITE setPreviewFit NOTIFY pcSettingsChanged)
    // samples of the stream per point of the preview (0: the whole block on the width)
    Q_PROPERTY(int previewDecimation READ previewDecimation WRITE setPreviewDecimation NOTIFY pcSettingsChanged)
    // the measurements of the preview (streaming_improvements 4.4)
    Q_PROPERTY(bool measure READ measure WRITE setMeasure NOTIFY pcSettingsChanged)
    // the preview shows the spectrum (streaming_improvements 4.5): its window (0 rectangular,
    // 1 Hann, 4 flat top) and the blocks averaged (1, 4, 16)
    Q_PROPERTY(bool spectrum READ spectrum WRITE setSpectrum NOTIFY pcSettingsChanged)
    Q_PROPERTY(int spectrumWindow READ spectrumWindow WRITE setSpectrumWindow NOTIFY pcSettingsChanged)
    Q_PROPERTY(int spectrumAverage READ spectrumAverage WRITE setSpectrumAverage NOTIFY pcSettingsChanged)

   public:
    using Ptr = std::shared_ptr<CBoard>;

    CBoard(QString ip, bool manual);
    ~CBoard();

    auto setOnline() -> void;
    auto getIP() -> QString;
    auto getName() -> QString;
    auto isManual() -> bool;
    auto setManual(bool manual) -> void;
    auto isOnline() -> bool;
    auto setMode(broadcast_lib::EMode mode) -> void;
    auto setModel(uint8_t model) -> void;
    auto setMac(const QString& mac) -> void;
    auto setGpio(bool gpio) -> void;
    auto getModel() -> QString;
    auto getIsMaster() -> bool;
    auto getConfigManagerConnected() -> bool;
    auto getLink() -> QString;
    auto getIsADCStarted() -> bool;
    auto getCaps() -> QVariantMap;
    auto getState() -> QString;
    auto getStats() -> QVariantMap;
    auto getTrigger() -> QVariantMap;
    auto getRecord() -> QVariantMap;
    auto getError() -> QVariantMap;
    auto getDacState() -> QVariantMap;
    auto getDacFile() -> QVariantMap;
    auto isDacRunning() -> bool;
    auto getGpioState() -> QVariantMap;
    auto getGpioFile() -> QVariantMap;
    auto isGpioInRunning() -> bool;
    auto isGpioOutRunning() -> bool;
    // the bytes a recording to this computer takes with the limit of the settings (0: no limit
    // or the board writes its own card) and the bytes a second of its data
    auto expectedBytes() -> double;
    // the disk of a folder that may not exist yet (its nearest existing parent): nothing is made
    static auto storageOf(const QString& dir) -> QStorageInfo;
    auto dataRate() -> double;
    auto settingsRev() -> int;
    auto settingsLoaded() -> bool;
    auto pcRev() -> int;
    auto getChartEnable() -> bool;
    auto setChartEnable(bool enable) -> void;
    auto previewFit() -> bool;
    auto setPreviewFit(bool fit) -> void;
    auto previewDecimation() -> int;
    auto setPreviewDecimation(int samplesPerPoint) -> void;
    auto measure() -> bool;
    auto setMeasure(bool on) -> void;
    auto spectrum() -> bool;
    auto setSpectrum(bool on) -> void;
    auto spectrumWindow() -> int;
    auto setSpectrumWindow(int window) -> void;
    auto spectrumAverage() -> int;
    auto setSpectrumAverage(int blocks) -> void;
    auto applySpectrum() -> void;

    Q_INVOKABLE QObject* getConsoleModel();

    // A setting of the board by its key (settings_lib), as the board last told it
    Q_INVOKABLE QString value(const QString& key);
    // Sets a setting on the board at once; false when the value is not valid for the key
    Q_INVOKABLE bool setValue(const QString& key, const QString& value);
    Q_INVOKABLE void readSettings();
    Q_INVOKABLE void saveDefaults();
    Q_INVOKABLE QString settingsJson();
    // false when the document is not one of settings
    // from: what the settings come from, for the log ("a file" when empty)
    Q_INVOKABLE bool loadSettingsJson(const QString& json, const QString& from = QString());

    // The settings of this board kept on this computer: recordDir ("" - the one of the
    // application), nameTemplate, openFolderOnStop, saveConsole, consoleLevel
    Q_INVOKABLE QVariant pcValue(const QString& key);
    Q_INVOKABLE void setPcValue(const QString& key, const QVariant& value);
    // the folder and the first file name a recording gets now
    Q_INVOKABLE QString recordDir();
    Q_INVOKABLE QString exampleFileName();
    Q_INVOKABLE void openRecordFolder();

    Q_INVOKABLE void startStreaming();
    Q_INVOKABLE void stopStreaming();
    Q_INVOKABLE void softwareTrigger();
    Q_INVOKABLE void reconnect();
    Q_INVOKABLE void dismissError();
    // The DAC: with dac_pass_mode DAC_NET the file of this computer (pcValue dacFile, dacRepeat
    // once / times / inf, dacTimes) is sent to the board; with DAC_FILE the board plays its own
    // file and only the start goes there
    Q_INVOKABLE void startDac();
    Q_INVOKABLE void stopDac();
    // The GPIO input: with gpio_in_pass_mode NET into a file of this computer (pcValue
    // gpioInFormat, gpioInLimitKind none / samples / time, gpioInLimit), with FILE the board
    // writes its SD card. The output: with gpio_out_pass_mode NET the file of this computer
    // (pcValue gpioOutFile, gpioOutRepeat once / times / inf, gpioOutTimes, gpioOutGaps), with
    // FILE the board plays gpio_out_file_sd. pcValue gpioInOff, gpioOutOff: Start all of the
    // board leaves them out
    Q_INVOKABLE void startGpioIn();
    Q_INVOKABLE void stopGpioIn();
    Q_INVOKABLE void startGpioOut();
    Q_INVOKABLE void stopGpioOut();

    auto startStreaming(QDateTime time) -> void;
    // a line from Start all / Stop all in the console of the board; error: also on the card,
    // unless the board already shows an error of its own (that one says more)
    auto note(const QString& msg, bool error) -> void;

   signals:
    void isOnlineChanged();
    void isMasterChanged();
    void modelChanged();
    void sourceChanged();
    void configManagerConnectedChanged();
    void stateChanged();
    void statsChanged();
    void triggerChanged();
    void recordChanged();
    void errorChanged();
    void dacChanged();
    void dacFileChanged();
    void gpioChanged();
    void gpioFileChanged();
    void settingsChanged();
    void pcSettingsChanged();
    // a stream lost data (for Stop all on loss)
    void dataLost();
    // a setting the board refused
    void settingRefused(QString key, QString value);

   private:
    struct SStat {
        uint64_t bytes = 0;
        uint64_t samples[4] = {0, 0, 0, 0};
        uint64_t lostBoard = 0;
        uint64_t lostFile = 0;
        double fileRate = 0;   // the measured write speed of the file, bytes per second
        double fileQueue = 0;  // what its queue in memory holds of its limit, 0..1
        int part = 0;          // the part of the file written now (streaming_improvements 4.11)
        uint64_t partBytes = 0;
        QString fileNow;       // its file
    };
    // the limit of a recording and its parts, as getRecord gives them: limitKind (none, samples,
    // time, size), frac, left (seconds), doneText, need (bytes the limit takes), part, partBytes,
    // parts (about how many), splitSize, splitTime
    auto fillLimit(QVariantMap& r, uint64_t samples, uint64_t written, double limitSamples, double limitTime, double limitSize, double splitSize,
                   double splitTime, int part, uint64_t partBytes) -> void;

    // Error and Warning show with "Errors only"; the console colours them
    enum class Log { Error = 0, Warning = 1, Normal = 2, Detail = 3 };

    auto post(std::function<void()> f) -> void;
    auto addLog(const QString& msg, Log level = Log::Normal) -> void;
    // an error of a stream: in the console and on the card until cleared
    auto setError(const QString& msg) -> void;
    auto setState(const QString& state) -> void;
    auto connectHandlers() -> void;
    auto connectConfig() -> void;
    auto updateOffline() -> void;
    auto applyBoardInfo(const BoardInfoMessage& info) -> void;
    auto settings() -> CStreamSettings*;
    auto recordedFiles() -> QString;
    auto adcStopped(const QString& why, bool error, bool broken = false) -> void;
    auto createStreaming() -> void;
    auto startADCFPGAStreaming() -> void;
    auto closeStreaming() -> void;
    auto updateSpeed() -> void;
    auto pollTrigger() -> void;
    // the name of a recording by the template, at `when` (else the start of the ADC)
    auto fileBaseName(const QDateTime& when = QDateTime()) -> QString;
    auto pcKey(const QString& key) -> QString;
    auto setLink(const QString& link) -> bool;
    auto adcSummary() -> QString;
    auto statSummary() -> QString;

    QString m_ip;
    bool m_manual;
    QTimer* m_offlineTimer;
    QTimer* m_reconnectTimer;
    QTimer* m_statTimer;
    QTimer* m_trigTimer;
    bool m_broadcastOnline = false;
    qint64 m_lastOnline = 0;
    broadcast_lib::EMode m_mode = broadcast_lib::AB_NONE;
    uint8_t m_model = 255;
    QString m_mac;
    bool m_gpio = false;
    bool m_infoFromBoard = false;
    BoardInfoMessage m_info;
    ClientNetConfigManager* m_configManager;
    bool m_connected = false;
    QString m_link = "connecting";
    // false once another client took the board: no fight over it, the user reconnects
    bool m_autoReconnect = true;
    qint64 m_lastLossLog = 0;
    uint64_t m_loggedLost = 0;
    bool m_settingsLoaded = false;
    int m_settingsRev = 0;
    int m_pcRev = 0;
    bool m_chartEnable = true;
    // the stream gives blocks to the preview: the card shows the signal or the list is shown
    // (the list has no console); read by the data thread
    std::atomic<bool> m_previewWanted{true};
    auto updatePreviewWanted() -> void;
    ConsoleModel m_consoleModel;
    QString m_state = "stopped";
    QString m_errorText;
    QString m_errorTime;
    int m_errorCount = 0;
    bool m_adcArmed = false;
    bool m_trigKnown = false;
    uint32_t m_adcTriggerCount = 0;
    bool m_stopRequested = false;
    QDateTime m_streamingDT;
    QString m_fileName;
    QFile m_consoleFile;

    streaming_lib::CStreamingFile::Ptr m_file_manager;
    net_lib::CAsioNet::Ptr m_asionet;
    DataLib::CBuffersCached::Ptr m_buffer;
    // owns the memory of the buffers of m_buffer (the client allocates it on the heap)
    std::shared_ptr<uio_lib::CMemoryManager> m_memory;
    adc_channels_t m_activeChannels;
    uint32_t m_blockSize = 0;

    std::mutex m_statMtx;
    SStat m_stat;
    uint64_t m_lastBytes = 0;
    qint64 m_lastBytesTime = 0;
    double m_speed = 0;
    uint64_t m_reportedLost = 0;

    // the recording on the SD card of the board (RecordStatus) and the samples a second seen
    StreamRecordStatus m_sdRecord;
    uint64_t m_sdFree = 0;
    uint64_t m_sdTotal = 0;
    bool m_recordKnown = false;
    uint64_t m_recSamples = 0;
    qint64 m_recTime = 0;
    double m_recRate = 0;
    // the disk of the files on this computer, read every second
    uint64_t m_pcFree = 0;
    uint64_t m_pcTotal = 0;
    auto pollRecord() -> void;

    // the DAC
    struct SDacRun {
        QString path;
        int format = 0;    // CStreamSettings::DataFormat
        int repeat = 0;    // 0 once, 1 N times, 2 until the stop
        int times = 1;
        bool gaps = false;
        uint32_t blockSize = 0;
        uint64_t fileBytes = 0;  // of samples, every channel
        int gen = 0;             // m_dacGen of the run: the end of an older sender does nothing
    };
    // what the sender thread ends with
    enum class DacEnd { Done, Stopped, NoConnection, Lost, FileError };
    auto connectDacHandlers() -> void;
    auto setDacState(const QString& state, const QString& text = QString()) -> void;
    auto dacStopped(const QString& why, bool error) -> void;
    auto startDacSender() -> void;
    auto dacSender(SDacRun run) -> void;
    auto dacSenderEnded(DacEnd end, const QString& text) -> void;
    auto joinDacSender() -> void;
    auto readDacFile() -> void;
    auto dacSummary() -> QString;

    QString m_dacState = "stopped";
    QString m_dacSource = "pc";
    QString m_dacPath;
    QString m_dacText;
    bool m_dacStopRequested = false;
    bool m_dacDone = false;
    int m_dacGen = 0;  // a start: the timeouts of an older one do nothing
    // a stop the board did not confirm (or not waited for): its STOPPED may still come while the
    // next start connects, and must not end it
    int m_dacStaleStops = 0;
    qint64 m_dacStart = 0;
    qint64 m_dacEnd = 0;  // the stop: the time and the rate of the run stay
    SDacRun m_dacRun;
    std::thread m_dacThread;
    std::atomic<bool> m_dacTerminate{false};
    std::atomic<bool> m_dacConnected{false};
    std::atomic<uint64_t> m_dacSent{0};
    std::mutex m_dacNetMtx;
    net_lib::CAsioNet::Ptr m_dacNet;  // the data connection of the sender, to close it from here

    QVariantMap m_dacFile;
    std::thread m_dacFileThread;
    bool m_dacFileBusy = false;
    bool m_dacFileAgain = false;  // the file changed while it was read: read it again

    // the GPIO: a direction runs on a thread of its own (as rpsa_client -G)
    enum class GpioEnd { None, Stopped, Done, Limit, TimeUp, FileError, DiskFull, NoConnection, Lost };
    struct SGpioIn {
        bool opened = false;
        uint64_t packs = 0, samples = 0, words = 0, bytes = 0, lost = 0, waited = 0;
        bool rle = false;
        QString file;  // the part written now
        int part = 0;
    };
    struct SGpioOut {
        bool connected = false;
        uint64_t words = 0, bytes = 0;
    };
    struct SGpioDir {
        QString state = "stopped";
        QString source = "pc";
        QString path;
        QString text;
        bool stopRequested = false;
        bool stopSent = false;
        bool boardDone = false;  // the board answered the stop or stopped it itself
        bool boardError = false;
        QString boardText;
        GpioEnd end = GpioEnd::None;  // what the thread ended with
        QString endText;
        int gen = 0;
        qint64 start = 0;
        qint64 stop = 0;
        std::thread thread;
        std::atomic<bool> terminate{false};
        std::atomic<bool> running{false};  // the thread
        // the data connection of the thread, to close it from here: a send blocked on a board
        // that does not read never sees terminate
        std::mutex netMtx;
        net_lib::CAsioNet::Ptr net;
        int staleStops = 0;  // as m_dacStaleStops
    };
    auto gpioDisconnect(SGpioDir& d) -> void;
    auto connectGpioHandlers() -> void;
    auto gpioRunning(const SGpioDir& d) -> bool;
    auto setGpioState(SGpioDir& d, const QString& state, const QString& text = QString()) -> void;
    // the board ended a direction (error: a refusal or a failure), or the thread did
    auto gpioBoardStopped(bool out, const QString& why, bool error) -> void;
    auto gpioThreadEnded(bool out) -> void;
    auto gpioFinish(bool out) -> void;
    auto gpioStopSent(bool out) -> void;
    auto gpioInRun(QString path, int format, uint32_t blockSize, uint64_t samplesLimit, double timeLimit, int gen) -> void;
    auto gpioClock() -> double;
    auto gpioOutRun(std::shared_ptr<CGpioFileReader> reader, uint32_t blockSize, int gen) -> void;
    auto readGpioFile() -> void;
    auto gpioInSummary() -> QString;
    auto gpioOutSummary() -> QString;

    SGpioDir m_gin;
    SGpioDir m_gout;
    // what the thread of a start of this computer gets once the board is ready
    int m_ginFormat = 0;  // CStreamSettings::GPIOFormat
    uint32_t m_gpioBlock = 0;
    std::shared_ptr<CGpioFileReader> m_goutReader;
    std::mutex m_gpioMtx;  // the counters of the threads
    SGpioIn m_ginStat;
    SGpioOut m_goutStat;
    bool m_goutWhole = false;   // the whole file is sent
    uint64_t m_goutWaited = 0;  // clocks, at the stop
    int m_goutPasses = 1;  // 0: until the stop
    uint64_t m_ginLimitSamples = 0;
    double m_ginLimitTime = 0;
    double m_ginRate = 0;  // samples a second of the lines
    // the board: the record of the input on its SD card, the trigger
    StreamRecordStatus m_sdGpioIn;
    bool m_gpioInArmed = false;
    bool m_gpioOutArmed = false;
    uint32_t m_gpioInTriggers = 0;
    uint64_t m_gpioOutWait = 0;

    QVariantMap m_gpioFile;
    std::thread m_gpioFileThread;
    bool m_gpioFileBusy = false;
    bool m_gpioFileAgain = false;
};

#endif  // BOARD_H
