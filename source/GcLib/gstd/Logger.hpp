#ifndef __GSTD_LOGGER__
#define __GSTD_LOGGER__

#include "File.hpp"
#include "GstdConstant.hpp"
#include "GstdUtility.hpp"
#include "Thread.hpp"

#include "Window.hpp"

namespace gstd {

/**********************************************************
//Logger
**********************************************************/
class Logger {
public:
	Logger();
	virtual ~Logger();
	virtual bool Initialize() { return true; }
	void AddLogger(ref_count_ptr<Logger> logger) { listLogger_.push_back(logger); }
	virtual void Write(std::wstring str);

	static void SetTop(Logger* logger) { top_ = logger; }
	static void WriteTop(std::wstring str)
	{
		if (top_ != NULL)
			top_->Write(str);
	} //トップのロガに出力します

protected:
	static Logger* top_;
	gstd::CriticalSection lock_;
	std::list<ref_count_ptr<Logger>> listLogger_; //子のロガ
	virtual void _WriteChild(SYSTEMTIME& time, std::wstring str);
	virtual void _Write(SYSTEMTIME& time, std::wstring str) = 0;
};

/**********************************************************
//FileLogger
**********************************************************/
class FileLogger : public Logger {
public:
	FileLogger();
	~FileLogger();
	void Clear();
	bool Initialize(bool bEnable = true);
	bool Initialize(std::wstring path, bool bEnable = true);
	bool SetPath(std::wstring path);
	void SetMaxFileSize(int size) { sizeMax_ = size; }

protected:
	bool bEnable_;
	std::wstring path_;
	std::wstring path2_;
	int sizeMax_;
	virtual void _Write(SYSTEMTIME& systemTime, std::wstring str);
	void _CreateFile(File& file);
};

} // namespace gstd;

#endif
