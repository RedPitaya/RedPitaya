#include <QQmlEngine>
#include <QThread>
#include "chartdataholder.h"

ChartDataHolder *ChartDataHolder::instance()
{
	static ChartDataHolder *_instance = 0;
	if (_instance == 0) {
		_instance = new ChartDataHolder();
	}
	return _instance;
}

ChartDataHolder::ChartDataHolder()
	: QObject(nullptr)
{
	QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
	m_IsWorkThread = true;
	m_chartThread = new std::thread(&ChartDataHolder::chartPackThread, this);
}

ChartDataHolder::~ChartDataHolder()
{
	m_IsWorkThread = false;
	if (m_chartThread->joinable()) {
		m_chartThread->join();
	}
}

auto ChartDataHolder::regRP(const QString &ip) -> void
{
	std::lock_guard<std::mutex> lock(m_add_mutex);
	m_rp.append(ip);
	m_packId[ip] = 0;
	m_chartPoints[0][ip].clear();
	m_chartPoints[1][ip].clear();
	m_chartPoints[2][ip].clear();
	m_chartPoints[3][ip].clear();
}

auto ChartDataHolder::removeRP(const QString &ip) -> void
{
	std::lock_guard<std::mutex> lock(m_add_mutex);
	m_rp.removeAll(ip);
}

auto ChartDataHolder::addBuffer(DataLib::CDataBuffersPackDMA::Ptr pack, uint64_t id, const QString &ip, bool skip) -> void
{
	std::lock_guard<std::mutex> lock(m_add_mutex);

	// skip some
	if (m_packId[ip] != 0)
		return;

	uint64_t maxID = 0;
	for (auto i : m_packId.keys()) {
		if (i != ip) {
			maxID = maxID < m_packId[i] ? m_packId[i] : maxID;
		}
	}
	if (skip) {
		m_packId[ip] = maxID;
		m_pack[ip] = nullptr;
		//        qDebug() << "Skip " << ip;
		return;
	}
	//    qDebug() << ip << "Max" << maxID << " id " << id;
	if (maxID < id) {
		for (auto &i : m_rp) {
			m_packId[i] = 0;
			m_pack[i] = nullptr;
		}
		//        qDebug() << "Reset all";
	}
	//    qDebug() << "Set " << ip << id;
	m_pack[ip] = pack;
	m_packId[ip] = id;
}

auto ChartDataHolder::chartPackThread() -> void
{
	while (m_IsWorkThread) {
		QMap<QString, DataLib::CDataBuffersPackDMA::Ptr> packForWork;
		m_add_mutex.lock();
		auto chartWidth = 1000;
		bool canGet = m_rp.size();
		for (auto &i : m_rp) {
			canGet &= m_packId[i] != 0;
		}
		if (canGet) {
			packForWork = m_pack;
			for (auto i : m_packId.keys()) {
				m_packId[i] = 0;
			}
		}
		m_add_mutex.unlock();

		auto pack = [&](const QString ip, int index, DataLib::CDataBufferDMA::Ptr buf) {
			if (!buf)
				return;
			auto pointsCount = buf->getSamplesCount();
			auto bitSize = buf->getBitBySample();
			auto buffer = buf->getMappedDataMemory();
			int step = buf->getSamplesCount() / chartWidth;
			m_chartPoints[index][ip].resize(chartWidth);
			if (step <= 1) {
				for (auto i = 0u; i < pointsCount; i++) {
					if (bitSize == 8) {
						auto b = reinterpret_cast<int8_t *>(buffer);
						m_chartPoints[index][ip][i] = (float) b[i] / 128.0;
					}
					if (bitSize == 16) {
						auto b = reinterpret_cast<int16_t *>(buffer);
						m_chartPoints[index][ip][i] = (float) b[i] / 32768.0;
					}
				}
			} else {
				for (auto i = 1, w = 0; i < pointsCount - 1 && w < chartWidth; i += step, w++) {
					int64_t sum = 0;
					for (int j = 0u; j < step && i + j < pointsCount; j++) {
						if (bitSize == 8) {
							auto b = reinterpret_cast<int8_t *>(buffer);
							sum += b[i + j] << 8;
						}
						if (bitSize == 16) {
							auto b = reinterpret_cast<int16_t *>(buffer);
							sum += b[i + j];
						}
					}
					m_chartPoints[index][ip][w] = (float) (sum / step) / 32768.0;
				}
			}
		};

		if (canGet) {
			//            qDebug() << Q_FUNC_INFO;
			m_chartGetPointMutex.lock();
			if (m_chartNeedDraw.size() == 0) {
				for (auto i : packForWork.keys()) {
					if (packForWork[i]) {
						pack(i, 0, packForWork[i]->getBuffer(DataLib::CH1));
						pack(i, 1, packForWork[i]->getBuffer(DataLib::CH2));
						pack(i, 2, packForWork[i]->getBuffer(DataLib::CH3));
						pack(i, 3, packForWork[i]->getBuffer(DataLib::CH4));
						m_chartNeedDraw.append(i);
					}
				}
			}
			m_chartGetPointMutex.unlock();
		}
		QThread::usleep(1000 * 500);
	}
}

QVector<qreal> ChartDataHolder::getChartSignal(QString ip, int index)
{
	std::lock_guard<std::mutex> lock(m_chartGetPointMutex);
	return m_chartPoints[index][ip];
}

bool ChartDataHolder::getChartNeedUpdate(QString ip)
{
	std::lock_guard<std::mutex> lock(m_chartGetPointMutex);
	return m_chartNeedDraw.contains(ip);
}

void ChartDataHolder::clearChartBuffer(QString ip)
{
	std::lock_guard<std::mutex> lock(m_chartGetPointMutex);
	m_chartNeedDraw.removeAll(ip);
}
