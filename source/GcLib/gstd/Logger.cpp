#include "Logger.hpp"

using namespace gstd;

#include <ctime>

/**********************************************************
//Logger
**********************************************************/
Logger* Logger::top_ = nullptr;
Logger::Logger()
{
}
Logger::~Logger()
{
	listLogger_.clear();
	if (top_ == this)
		top_ = nullptr;
}
void Logger::_WriteChild(struct tm* time, std::string str)
{
	_Write(time, str);
	std::list<std::shared_ptr<Logger>>::iterator itr = listLogger_.begin();
	for (; itr != listLogger_.end(); itr++) {
		(*itr)->_Write(time, str);
	}
}

void Logger::Write(std::string str)
{
	time_t rawtime;
	struct tm* timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	this->_WriteChild(timeinfo, str);
}

/**********************************************************
//FileLogger
**********************************************************/
FileLogger::FileLogger()
{
	sizeMax_ = 10 * 1024 * 1024; // 10MB
}

FileLogger::~FileLogger()
{
}
void FileLogger::Clear()
{
	if (!bEnable_)
		return;

	File file1(path_);
	file1.Delete();
	File file2(path2_);
	file2.Delete();

	_CreateFile(file1);
}
bool FileLogger::Initialize(bool bEnable)
{
	return this->Initialize("", bEnable);
}
bool FileLogger::Initialize(std::string path, bool bEnable)
{
	bEnable_ = bEnable;
	if (path.size() == 0) {
		path = PathProperty::GetModuleDirectory() + PathProperty::GetModuleName() + std::string(".log");
	}
	return this->SetPath(path);
}
bool FileLogger::SetPath(std::string path)
{
	if (!bEnable_)
		return false;

	path_ = path;
	File file(path);
	if (file.IsExists() == false) {
		file.CreateDirectory();
		_CreateFile(file);
	}

	path2_ = path_ + "_";
	return true;
}
void FileLogger::_CreateFile(File& file)
{
	file.Create();

	//BOM（Byte Order Mark）
	file.WriteCharacter((unsigned char)0xFF);
	file.WriteCharacter((unsigned char)0xFE);
}
void FileLogger::_Write(struct tm* time, std::string str)
{
	if (!bEnable_)
		return;

	{
		Lock lock(lock_);
		std::string strTime = StringUtility::Format("%.4d/%.2d/%.2d %.2d:%.2d:%.2d ", time->tm_year + 1900, time->tm_mon + 1, time->tm_mday, time->tm_hour, time->tm_min, time->tm_sec);

		File file(path_);
		if (!file.Open(File::AccessType::Write))
			return;

		std::string out = strTime;
		out += str;
		out += "\r\n";

		int pos = file.GetSize();
		file.Seek(pos);
		file.Write(&out[0], out.size());

		bool bOverSize = file.GetSize() > sizeMax_;
		file.Close();

		if (bOverSize) {
#ifdef _WIN32
			::MoveFileEx(StringUtility::ConvertMultiToWide(path_, CP_UTF8).c_str(), StringUtility::ConvertMultiToWide(path2_, CP_UTF8).c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
#else
			// TODO: Linux only
#endif
			File file1(path_);
			_CreateFile(file1);
		}
	}
}
