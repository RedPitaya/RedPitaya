#include <QApplication>
#include <QFont>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QTimer>

#include "app_settings.h"
#include "logic/chartdataholder.h"
#include "logic/device_logic.h"
#include "models/boardsmodel.h"
#include "ui_controller.h"

namespace {

// RPSA_QT_SNAPSHOT=<file.png>[:<ms>]: saves the window after ms (default 4000) and quits.
// RPSA_QT_UI=<script>: what the window opens with (see main.qml). For tests and screenshots.
auto snapshotHook(QQmlApplicationEngine& engine) -> void {
    const QString spec = qEnvironmentVariable("RPSA_QT_SNAPSHOT");
    if (spec.isEmpty())
        return;
    QString file = spec;
    int delay = 4000;
    const int colon = spec.lastIndexOf(':');
    if (colon > 0) {
        bool ok = false;
        const int ms = spec.mid(colon + 1).toInt(&ok);
        if (ok) {
            delay = ms;
            file = spec.left(colon);
        }
    }
    QTimer::singleShot(delay, &engine, [&engine, file]() {
        auto* window = qobject_cast<QQuickWindow*>(engine.rootObjects().value(0));
        if (!window) {
            QCoreApplication::exit(1);
            return;
        }
        window->grabWindow().save(file);
        window->close();  // as the user closes it: the window keeps its geometry
        QCoreApplication::exit(0);
    });
}

}  // namespace

int main(int argc, char* argv[]) {
    // the controls are styled by the application: the Basic style takes that
    QQuickStyle::setStyle("Basic");

    QApplication app(argc, argv);
    app.setOrganizationName("RedPitaya");
    app.setApplicationName("Red Pitaya streaming client");

    // the type of the web page of the board
    QFont font;
    font.setFamilies({"Helvetica Neue", "Helvetica", "Arial", "Liberation Sans", "DejaVu Sans"});
    font.setPixelSize(14);
    app.setFont(font);

    auto* settings = AppSettings::instance();
    auto* chart = ChartDataHolder::instance();
    chart->setRate(settings->previewRate());
    chart->setPoints(settings->previewPoints());
    QObject::connect(settings, &AppSettings::changed, chart, [settings, chart]() {
        chart->setRate(settings->previewRate());
        chart->setPoints(settings->previewPoints());
    });
    DeviceLogic::instance();

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("ui_controller", ui_controller::instance());
    engine.rootContext()->setContextProperty("cdh", chart);
    engine.rootContext()->setContextProperty("app", settings);
    engine.rootContext()->setContextProperty("boards", BoardsModel::instance());
    engine.rootContext()->setContextProperty("uiScript", qEnvironmentVariable("RPSA_QT_UI"));

    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject* obj, const QUrl& objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.addImportPath(QCoreApplication::applicationDirPath() + "/qml");
    engine.addImportPath(":/");
    engine.load(url);
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }
    snapshotHook(engine);
    return app.exec();
}
