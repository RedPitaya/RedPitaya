#ifndef CONVERTER_LIB_CONVERTER_H
#define CONVERTER_LIB_CONVERTER_H

#include <atomic>
#include <memory>

namespace converter_lib {

class CConverter
{
public:
	using Ptr = std::shared_ptr<CConverter>;

	static auto create() -> Ptr;

	CConverter();
	~CConverter();

	bool convertToCSV(std::string _file_name, std::string _prefix);
	bool convertToCSV(std::string _file_name, int32_t start_seg, int32_t end_seg, std::string _prefix);
	void stopWriteToCSV();

private:
	CConverter(const CConverter &) = delete;
	CConverter(CConverter &&) = delete;
	CConverter &operator=(const CConverter &) = delete;
	CConverter &operator=(const CConverter &&) = delete;

	std::atomic_bool m_stopWriteCSV;
	std::mutex m_mtx;
};

} // namespace converter_lib

#endif
