#ifndef STREAMING_LIB_STREAMING_NET_H
#define STREAMING_LIB_STREAMING_NET_H

#include <atomic>
#include <functional>
#include <mutex>
#include <thread>

#include "data_lib/buffers_pack.h"
#include "net_lib/asio_net.h"

#define TCP_BUFFER_LIMIT 32 * 1024

namespace streaming_lib {

class CStreamingNet
{
public:
	using Ptr = std::shared_ptr<CStreamingNet>;
	typedef std::function<DataLib::CDataBuffersPackDMA::Ptr()> getBufferFunc;
	typedef std::function<void()> unlockBufferFunc;

	static auto create(std::string &_host, uint16_t _port) -> Ptr;

	CStreamingNet(std::string &_host, uint16_t _port);
	~CStreamingNet();

	auto run() -> void;
	auto runNonThread() -> void;
	auto stop() -> void;
	auto sendBuffers(DataLib::CDataBuffersPackDMA::Ptr pack) -> void;

	getBufferFunc getBuffer;
	unlockBufferFunc unlockBufferF;

private:
	CStreamingNet(const CStreamingNet &) = delete;
	CStreamingNet(CStreamingNet &&) = delete;
	CStreamingNet &operator=(const CStreamingNet &) = delete;
	CStreamingNet &operator=(const CStreamingNet &&) = delete;

	std::string m_host;
	uint16_t m_port;

	net_lib::CAsioNet *m_asionet;

	uint64_t m_index_of_message;
	std::thread m_thread;
	std::atomic_bool m_threadRun;
	std::mutex m_mtx;

	auto startServer() -> void;
	auto stopServer() -> void;
	auto task() -> void;
};

} // namespace streaming_lib

#endif
