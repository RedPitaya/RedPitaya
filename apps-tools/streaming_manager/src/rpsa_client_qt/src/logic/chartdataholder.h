#ifndef CHARTDATAHOLDER_H
#define CHARTDATAHOLDER_H

#include <QObject>
#include <QMap>
#include <mutex>
#include <thread>
#include "data_lib/buffers_pack.h"

class ChartDataHolder : public QObject
{
	Q_OBJECT
public:
	static ChartDataHolder *instance();

	ChartDataHolder();
	~ChartDataHolder();

	auto regRP(const QString &ip) -> void;
	auto removeRP(const QString &ip) -> void;
	auto addBuffer(DataLib::CDataBuffersPackDMA::Ptr pack, uint64_t id, const QString &ip, bool skip) -> void;

	Q_INVOKABLE QVector<qreal> getChartSignal(QString ip, int index);
	Q_INVOKABLE bool getChartNeedUpdate(QString ip);
	Q_INVOKABLE void clearChartBuffer(QString ip);

public slots:

private:
	auto chartPackThread() -> void;

	std::mutex m_add_mutex;
	std::mutex m_chartGetPointMutex;
	QList<QString> m_rp;
	QMap<QString, DataLib::CDataBuffersPackDMA::Ptr> m_pack;
	QMap<QString, uint64_t> m_packId;

	std::thread *m_chartThread;
	bool m_IsWorkThread;
	uint32_t m_chartWidth;
	QMap<QString, QVector<qreal>> m_chartPoints[4];
	QList<QString> m_chartNeedDraw;
};

#endif // CONSOLEMODEL_H
