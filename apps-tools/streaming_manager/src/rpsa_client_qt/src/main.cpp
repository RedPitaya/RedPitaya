#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>

#include "ui_controller.h"
#include "logic/device_logic.h"
#include "logic/chartdataholder.h"

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#else
	QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
#endif

	QApplication app(argc, argv);

	DeviceLogic::instance();

	QQmlApplicationEngine engine;

	engine.rootContext()->setContextProperty("ui_controller", ui_controller::instance());
	engine.rootContext()->setContextProperty("cdh", ChartDataHolder::instance());

	const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
	QObject::connect(
		&engine,
		&QQmlApplicationEngine::objectCreated,
		&app,
		[url](QObject *obj, const QUrl &objUrl) {
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

	return app.exec();
}
