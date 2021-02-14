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
	void AddLogger(std::shared_ptr<Logger> logger) { listLogger_.push_back(logger); }
	virtual void Write(std::string str);

	static void SetTop(Logger* logger) { top_ = logger; }
	static void WriteTop(std::string str)
	{
		if (top_ != NULL)
			top_->Write(str);
	} //トップのロガに出力します

protected:
	static Logger* top_;
	gstd::CriticalSection lock_;
	std::list<std::shared_ptr<Logger>> listLogger_; //子のロガ
	virtual void _WriteChild(struct tm* time, std::string str);
	virtual void _Write(struct tm* time, std::string str) = 0;
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
	bool Initialize(std::string path, bool bEnable = true);
	bool SetPath(std::string path);
	void SetMaxFileSize(int size) { sizeMax_ = size; }

protected:
	bool bEnable_;
	std::string path_;
	std::string path2_;
	int sizeMax_;
	virtual void _Write(struct tm* systemTime, std::string str);
	void _CreateFile(File& file);
};

} // namespace gstd;

#endif
