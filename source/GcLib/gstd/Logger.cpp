#include "Logger.hpp"

using namespace gstd;

/**********************************************************
//Logger
**********************************************************/
Logger* Logger::top_ = NULL;
Logger::Logger()
{
}
Logger::~Logger()
{
	listLogger_.clear();
	if (top_ == this)
		top_ = NULL;
}
void Logger::_WriteChild(SYSTEMTIME& time, std::wstring str)
{
	_Write(time, str);
	std::list<ref_count_ptr<Logger>>::iterator itr = listLogger_.begin();
	for (; itr != listLogger_.end(); itr++) {
		(*itr)->_Write(time, str);
	}
}

void Logger::Write(std::wstring str)
{
	SYSTEMTIME systemTime;
	GetLocalTime(&systemTime);
	this->_WriteChild(systemTime, str);
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
	return this->Initialize(L"", bEnable);
}
bool FileLogger::Initialize(std::wstring path, bool bEnable)
{
	bEnable_ = bEnable;
	if (path.size() == 0) {
		path = PathProperty::GetModuleDirectory() + PathProperty::GetModuleName() + std::wstring(L".log");
	}
	return this->SetPath(path);
}
bool FileLogger::SetPath(std::wstring path)
{
	if (!bEnable_)
		return false;

	path_ = path;
	File file(path);
	if (file.IsExists() == false) {
		file.CreateDirectory();
		_CreateFile(file);
	}

	path2_ = path_ + L"_";
	return true;
}
void FileLogger::_CreateFile(File& file)
{
	file.Create();

	//BOM（Byte Order Mark）
	file.WriteCharacter((unsigned char)0xFF);
	file.WriteCharacter((unsigned char)0xFE);
}
void FileLogger::_Write(SYSTEMTIME& time, std::wstring str)
{
	if (!bEnable_)
		return;

	{
		Lock lock(lock_);
		std::wstring strTime = StringUtility::Format(L"%.4d/%.2d/%.2d %.2d:%.2d:%.2d.%.3d ", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);

		File file(path_);
		if (!file.Open(File::WRITE))
			return;

		std::wstring out = strTime;
		out += str;
		out += L"\r\n";

		int pos = file.GetSize();
		file.Seek(pos);
		file.Write(&out[0], StringUtility::GetByteSize(out));

		bool bOverSize = file.GetSize() > sizeMax_;
		file.Close();

		if (bOverSize) {
			::MoveFileEx(path_.c_str(), path2_.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
			File file1(path_);
			_CreateFile(file1);
		}
	}
}
