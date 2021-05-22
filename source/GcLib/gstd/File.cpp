#include "File.hpp"
#include "GstdUtility.hpp"
#include "Logger.hpp"
#include <cassert>
#include <zlib.h>

using namespace gstd;

/**********************************************************
//ByteBuffer
**********************************************************/
ByteBuffer::ByteBuffer()
{
	data_ = nullptr;
	Clear();
}
ByteBuffer::ByteBuffer(ByteBuffer& buffer)
{
	data_ = nullptr;
	Clear();
	Copy(buffer);
}
ByteBuffer::~ByteBuffer()
{
	Clear();
	if (data_ != nullptr)
		delete[] data_;
}
int ByteBuffer::_GetReservedSize()
{
	return reserve_;
}
void ByteBuffer::_Resize(size_t size)
{
	char* oldData = data_;
	size_t oldSize = size_;

	data_ = new char[size];
	ZeroMemory(data_, size);

	//元のデータをコピー
	size_t sizeCopy = MIN(size, oldSize);
	if (oldData != nullptr) {
		memcpy(data_, oldData, sizeCopy);
		//古いデータを削除
		delete[] oldData;
	}
	reserve_ = size;
	size_ = size;
}
void ByteBuffer::Copy(ByteBuffer& src)
{
	if (data_ != nullptr && src.reserve_ != reserve_) {
		delete[] data_;
		data_ = new char[src.reserve_];
		ZeroMemory(data_, src.reserve_);
	}

	offset_ = src.offset_;
	reserve_ = src.reserve_;
	size_ = src.size_;

	memcpy(data_, src.data_, reserve_);
}
void ByteBuffer::Clear()
{
	if (data_ != nullptr)
		delete[] data_;

	data_ = new char[0];
	offset_ = 0;
	reserve_ = 0;
	size_ = 0;
}
void ByteBuffer::Seek(size_t pos)
{
	offset_ = pos;
	if (offset_ < 0)
		offset_ = 0;
	else if (offset_ > size_)
		offset_ = size_;
}
void ByteBuffer::SetSize(size_t size)
{
	_Resize(size);
}
size_t ByteBuffer::Write(const void* buf, size_t size)
{
	if (offset_ + size > reserve_) {
		size_t sizeNew = (offset_ + size) * 2;
		_Resize(sizeNew);
		size_ = 0; //あとで再計算
	}

	memcpy(&data_[offset_], buf, size);
	offset_ += size;
	size_ = MAX(size_, offset_);
	return size;
}
size_t ByteBuffer::Read(void* buf, size_t size)
{
	memcpy(buf, &data_[offset_], size);
	offset_ += size;
	return size;
}
char* ByteBuffer::GetPointer(size_t offset)
{
	if (offset > size_) {
		throw std::range_error(u8"ByteBuffer:インデックスエラー");
	}
	return &data_[offset];
}
size_t ByteBuffer::Decompress()
{
	size_t size = size_ * 1000;
	char* dest = new char[size];

	z_stream infstream;
	infstream.zalloc = Z_NULL;
	infstream.zfree = Z_NULL;
	infstream.opaque = Z_NULL;
	infstream.avail_in = (uInt)size_;
	infstream.next_in = (Bytef*)data_;
	infstream.avail_out = (uInt)size;
	infstream.next_out = (Bytef*)dest;

	inflateInit(&infstream);
	inflate(&infstream, Z_NO_FLUSH);
	inflateEnd(&infstream);
	delete[] data_;
	data_ = dest;
	return infstream.total_out;
}
size_t ByteBuffer::Decompress(size_t size)
{
	char* dest = new char[size];

	z_stream infstream;
	infstream.zalloc = Z_NULL;
	infstream.zfree = Z_NULL;
	infstream.opaque = Z_NULL;
	infstream.avail_in = (uInt)size_;
	infstream.next_in = (Bytef*)data_;
	infstream.avail_out = (uInt)size;
	infstream.next_out = (Bytef*)dest;

	inflateInit(&infstream);
	inflate(&infstream, Z_NO_FLUSH);
	inflateEnd(&infstream);
	delete[] data_;
	data_ = dest;
	size_ = size;
	reserve_ = size;
	return infstream.total_out;
}

/**********************************************************
//File
**********************************************************/
File::File()
{
	hFile_ = nullptr;
	path_ = "";
}
File::File(std::string path)
{
	hFile_ = nullptr;
	path_ = path;
}
File::~File()
{
	Close();
}
bool File::CreateDirectory()
{
	std::string dir = PathProperty::GetFileDirectory(path_);
	if (File::IsExists(dir))
		return true;

	std::vector<std::string> str = StringUtility::Split(dir, "\\/");
	std::string tPath = "";
	for (int iDir = 0; iDir < str.size(); iDir++) {
		tPath += str[iDir] + "\\";
#ifdef _WIN32
		WIN32_FIND_DATAW fData;
		std::wstring tPathW = StringUtility::ConvertMultiToWide(tPath, CP_UTF8);
		HANDLE hFile = ::FindFirstFileW(tPathW.c_str(), &fData);
		if (hFile == INVALID_HANDLE_VALUE) {
			SECURITY_ATTRIBUTES attr;
			attr.nLength = sizeof(SECURITY_ATTRIBUTES);
			attr.lpSecurityDescriptor = nullptr;
			attr.bInheritHandle = FALSE;
			::CreateDirectoryW(tPathW.c_str(), &attr);
		}
		FindClose(hFile);
#else
		struct stat statbuf;
		if (stat(tPath.c_str(), &statbuf) == -1)
		{
			mkdir(tPath.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
		}
#endif
	}
	return true;
}
void File::Delete()
{
	Close();

#ifdef _WIN32
	DeleteFileW(StringUtility::ConvertMultiToWide(path_, CP_UTF8).c_str());
#else
	remove(path_.c_str());
#endif
}
bool File::IsExists()
{
	if (hFile_ != nullptr)
		return true;

	return IsExists(path_);
}
bool File::IsExists(std::string path)
{
#ifdef _WIN32
	return PathFileExistsW(StringUtility::ConvertMultiToWide(path, CP_UTF8).c_str()) == TRUE;
#else
	return access(path.c_str(), F_OK) == 0;
#endif
}
bool File::IsDirectory()
{
#ifdef _WIN32
	WIN32_FIND_DATA fData;
	HANDLE hFile = FindFirstFileW(StringUtility::ConvertMultiToWide(path_, CP_UTF8).c_str(), &fData);
	bool res = hFile != INVALID_HANDLE_VALUE;
	if (res)
		res = (FILE_ATTRIBUTE_DIRECTORY & fData.dwFileAttributes) > 0;

	FindClose(hFile);
	return res;
#else
	struct stat statbuf;
	if (stat(path_.c_str(), &statbuf) == -1)
		return false;

	return S_ISDIR(statbuf.st_mode);
#endif
}
Sint64 File::GetSize()
{
	if (hFile_ != nullptr)
		return SDL_RWsize(hFile_);

#ifdef _WIN32
	int res = 0;
	WIN32_FIND_DATA fData;
	HANDLE hFile = FindFirstFileW(StringUtility::ConvertMultiToWide(path_, CP_UTF8).c_str(), &fData);
	res = hFile != INVALID_HANDLE_VALUE ? GetFileSize(hFile, nullptr) : 0;
	FindClose(hFile);
	return res;
#else
	struct stat statbuf;
	if (stat(path_.c_str(), &statbuf) == -1)
		return 0;

	return statbuf.st_size;
#endif
}

bool File::Open()
{
	return this->Open(AccessType::Read);
}
bool File::Open(AccessType typeAccess)
{
	if (hFile_ != nullptr)
		this->Close();

	hFile_ = SDL_RWFromFile(path_.c_str(), typeAccess == AccessType::Read ? "rb" : "wb");

	return hFile_ != nullptr;
}
bool File::Create()
{
	if (hFile_ != nullptr)
		this->Close();

	hFile_ = SDL_RWFromFile(path_.c_str(), "wb");
	return hFile_ != nullptr;
}
void File::Close()
{
	if (hFile_ != nullptr)
		SDL_RWclose(hFile_);
	hFile_ = nullptr;
}

size_t File::Read(void* buf, size_t size)
{
	return SDL_RWread(hFile_, buf, size, 1);
}
size_t File::Write(const void* buf, size_t size)
{
	return SDL_RWwrite(hFile_, buf, size, 1);
}
bool File::IsEqualsPath(std::string path1, std::string path2)
{
	path1 = PathProperty::GetUnique(path1);
	path2 = PathProperty::GetUnique(path2);
	bool res = (strcmp(path1.c_str(), path2.c_str()) == 0);
	return res;
}
std::vector<std::string> File::GetFilePathList(std::string dir)
{
	std::vector<std::string> res;

#ifdef _WIN32
	WIN32_FIND_DATAW data;
	HANDLE hFind;
	std::wstring dirW = StringUtility::ConvertMultiToWide(dir, CP_UTF8), findDir = dirW + L"*.*";
	hFind = FindFirstFileW(findDir.c_str(), &data);
	do {
		std::wstring name = data.cFileName;
		if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			&& (name != L".." && name != L".")) {
			//ディレクトリ
			std::wstring tDir = dirW + name;
			tDir += L"\\";

			continue;
		}
		if (wcscmp(data.cFileName, L"..") == 0 || wcscmp(data.cFileName, L".") == 0)
			continue;

		//ファイル
		std::wstring path = dirW + name;

		res.push_back(StringUtility::ConvertWideToMulti(path, CP_UTF8));

	} while (FindNextFileW(hFind, &data));
	FindClose(hFind);
#else

	// TODO: Linux
#endif

	return res;
}
std::vector<std::string> File::GetDirectoryPathList(std::string dir)
{
	std::vector<std::string> res;

#ifdef _WIN32
	WIN32_FIND_DATAW data;
	HANDLE hFind;
	std::wstring dirW = StringUtility::ConvertMultiToWide(dir, CP_UTF8), findDir = dirW + L"*.*";
	hFind = FindFirstFileW(findDir.c_str(), &data);
	do {
		std::wstring name = data.cFileName;
		if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (name != L".." && name != L".")) {
			//ディレクトリ
			std::wstring tDir = dirW + name;
			tDir += L"\\";
			res.push_back(StringUtility::ConvertWideToMulti(tDir, CP_UTF8));
			continue;
		}
		if (wcscmp(data.cFileName, L"..") == 0 || wcscmp(data.cFileName, L".") == 0)
			continue;

		//ファイル

	} while (FindNextFile(hFind, &data));
	FindClose(hFind);
#else
	// TODO: Linux
#endif

	return res;
}

/**********************************************************
//ArchiveFileEntry
**********************************************************/
ArchiveFileEntry::ArchiveFileEntry()
{
	typeCompress_ = CompressType::Non;
	sizeData_ = 0;
	sizeCompressed_ = 0;
	offset_ = 0;
}
ArchiveFileEntry::~ArchiveFileEntry()
{
}
int ArchiveFileEntry::_GetEntryRecordSize()
{
	int res = 0;

	res += sizeof(int);
	res += dir_.size();
	res += sizeof(int);
	res += name_.size();
	res += sizeof(CompressType);
	res += sizeof(int);
	res += sizeof(int);
	res += sizeof(int);

	return res;
}
void ArchiveFileEntry::_WriteEntryRecord(ByteBuffer& buf)
{
	buf.WriteInteger(dir_.size());
	buf.Write(&dir_[0], dir_.size());
	buf.WriteInteger(name_.size());
	buf.Write(&name_[0], name_.size());
	buf.Write(&typeCompress_, sizeof(CompressType));
	buf.WriteInteger(sizeData_);
	buf.WriteInteger(sizeCompressed_);
	buf.WriteInteger(offset_);
}
void ArchiveFileEntry::_ReadEntryRecord(ByteBuffer& buf)
{
	dir_.resize(buf.ReadInteger());
	buf.Read(&dir_[0], dir_.size());
	name_.resize(buf.ReadInteger());
	buf.Read(&name_[0], name_.size());
	buf.Read(&typeCompress_, sizeof(CompressType));
	sizeData_ = buf.ReadInteger();
	sizeCompressed_ = buf.ReadInteger();
	offset_ = buf.ReadInteger();
}
/**********************************************************
//FileArchiver
**********************************************************/
FileArchiver::FileArchiver()
{
}
FileArchiver::~FileArchiver()
{
}
bool FileArchiver::CreateArchiveFile(std::string path)
{
	bool res = true;
	File fileArchive(path);
	if (!fileArchive.Create())
		throw std::runtime_error(StringUtility::Format(u8"ファイル作成失敗[%s]", path.c_str()).c_str());

	fileArchive.Write((char*)&HEADER_ARCHIVEFILE[0], HEADER_ARCHIVEFILE.size());
	fileArchive.WriteInteger(listEntry_.size());

	int posArchiveEntryHeaderStart = fileArchive.GetFilePointer();
	fileArchive.WriteBoolean(false);
	fileArchive.WriteInteger(0);

	int posEntryStart = fileArchive.GetFilePointer();

	std::list<std::unique_ptr<ArchiveFileEntry>>::iterator itr;
	for (itr = listEntry_.begin(); itr != listEntry_.end(); itr++) {
		ArchiveFileEntry* entry = itr->get();

		std::string name = entry->GetName();
		entry->SetName(PathProperty::GetFileName(name));

		fileArchive.WriteInteger(entry->_GetEntryRecordSize());

		ByteBuffer buf;
		entry->_WriteEntryRecord(buf);
		fileArchive.Write(buf.GetPointer(), buf.GetSize());

		entry->SetName(name);
	}
	int posEntryEnd = fileArchive.GetFilePointer();

	for (itr = listEntry_.begin(); itr != listEntry_.end(); itr++) {
		ArchiveFileEntry* entry = itr->get();
		std::string path = entry->GetName();
		File file(path);
		if (!file.Open())
			throw std::runtime_error(StringUtility::Format(u8"ファイルオープン失敗[%s]", path.c_str()).c_str());

		entry->_SetDataSize(file.GetSize());
		entry->_SetOffset(fileArchive.GetFilePointer());
		ByteBuffer buf;
		buf.SetSize(file.GetSize());
		file.Read(buf.GetPointer(), buf.GetSize());

		ArchiveFileEntry::CompressType typeCompress = entry->GetCompressType();
		if (typeCompress == ArchiveFileEntry::CompressType::Non) {
			fileArchive.Write(buf.GetPointer(), buf.GetSize());
		} else if (typeCompress == ArchiveFileEntry::CompressType::Compress) {
			ByteBuffer bufComp;
			Compressor inflater;
			inflater.Compress(buf, bufComp);

			entry->_SetCompressedDataSize(bufComp.GetSize());
			fileArchive.Write(bufComp.GetPointer(), bufComp.GetSize());
		}
	}

	//とりあえず非圧縮で書き込む
	fileArchive.Seek(posArchiveEntryHeaderStart);
	fileArchive.WriteBoolean(false);
	fileArchive.WriteInteger(0);
	for (itr = listEntry_.begin(); itr != listEntry_.end(); itr++) {
		ArchiveFileEntry* entry = itr->get();

		std::string name = entry->GetName();
		entry->SetName(PathProperty::GetFileName(name));

		fileArchive.WriteInteger(entry->_GetEntryRecordSize());

		ByteBuffer buf;
		entry->_WriteEntryRecord(buf);
		fileArchive.Write(buf.GetPointer(), buf.GetSize());

		entry->SetName(name);
	}

	//圧縮可能か調べる
	fileArchive.Seek(posEntryStart);
	int sizeEntry = posEntryEnd - posEntryStart;
	ByteBuffer bufEntryIn;
	ByteBuffer bufEntryOut;
	bufEntryIn.SetSize(sizeEntry);
	fileArchive.Read(bufEntryIn.GetPointer(), sizeEntry);

	Compressor compEntry;
	compEntry.Compress(bufEntryIn, bufEntryOut);
	if (bufEntryOut.GetSize() < sizeEntry) {
		//エントリ圧縮
		fileArchive.Seek(posArchiveEntryHeaderStart);
		fileArchive.WriteBoolean(true);
		fileArchive.WriteInteger(bufEntryOut.GetSize());
		fileArchive.Write(bufEntryOut.GetPointer(), bufEntryOut.GetSize());

		int sizeGap = sizeEntry - bufEntryOut.GetSize();
		ByteBuffer bufGap;
		bufGap.SetSize(sizeGap);
		fileArchive.Write(bufGap.GetPointer(), sizeGap);
	}

	return res;
}

/**********************************************************
//ArchiveFile
**********************************************************/
ArchiveFile::ArchiveFile(std::string path)
{
	file_ = std::make_unique<File>(path);
}
ArchiveFile::~ArchiveFile()
{
	Close();
}
bool ArchiveFile::Open()
{
	if (!file_->Open())
		return false;
	if (mapEntry_.size() != 0)
		return true;

	bool res = true;
	try {
		std::string header;
		header.resize(HEADER_ARCHIVEFILE.size());
		file_->Read(&header[0], header.size());
		if (header != HEADER_ARCHIVEFILE)
			throw std::runtime_error("Invalid header");

		int countEntry = file_->ReadInteger();
		bool bCompress = file_->ReadBoolean();
		int sizeArchiveHeader = file_->ReadInteger();

		Reader* reader = nullptr;
		ByteBuffer* bufIn = new ByteBuffer;
		if (!bCompress)
			reader = file_.get();
		else {
			bufIn->SetSize(sizeArchiveHeader);
			file_->Read(bufIn->GetPointer(), sizeArchiveHeader);
			bufIn->Decompress();
			reader = bufIn;
		}

		for (int iEntry = 0; iEntry < countEntry; iEntry++) {
			std::unique_ptr<ArchiveFileEntry> entry = std::make_unique<ArchiveFileEntry>();
			int sizeEntry = reader->ReadInteger();
			ByteBuffer buf;
			// buf.Clear();
			buf.SetSize(sizeEntry);
			reader->Read(buf.GetPointer(), sizeEntry);
			entry->_ReadEntryRecord(buf);
			entry->_SetArchivePath(file_->GetPath());

			// std::string key = entry->GetDirectory() + entry->GetName();
			// mapEntry_[key] = entry;
			std::string key = entry->GetName();
			mapEntry_.insert(std::make_pair(key, std::move(entry)));
		}
		reader = nullptr;
		res = true;
	} catch (...) {
		res = false;
	}
	file_->Close();
	return res;
}
void ArchiveFile::Close()
{
	file_->Close();
}
std::set<std::string> ArchiveFile::GetKeyList()
{
	std::set<std::string> res;
	auto itr = mapEntry_.begin(), end = mapEntry_.end();
	for (; itr != end; itr++) {
		// std::wstring key = entry->GetDirectory() + entry->GetName();
		std::string key = itr->second->GetName();
		res.insert(key);
	}
	return res;
}
std::vector<std::shared_ptr<ArchiveFileEntry>> ArchiveFile::GetEntryList(std::string name)
{
	std::vector<std::shared_ptr<ArchiveFileEntry>> res;
	if (!IsExists(name))
		return res;

	auto itrPair = mapEntry_.equal_range(name);
	for (; itrPair.first != itrPair.second; itrPair.first++) {
		res.push_back((itrPair.first)->second);
	}

	return res;
}
bool ArchiveFile::IsExists(std::string name)
{
	return mapEntry_.find(name) != mapEntry_.end();
}
std::unique_ptr<ByteBuffer> ArchiveFile::CreateEntryBuffer(ArchiveFileEntry& entry)
{
	std::unique_ptr<ByteBuffer> res;
	File file(entry.GetArchivePath());
	if (file.Open()) {
		if (entry.GetCompressType() == ArchiveFileEntry::CompressType::Non) {
			file.Seek(entry.GetOffset());
			res = std::make_unique<ByteBuffer>();
			int size = entry.GetDataSize();
			res->SetSize(size);
			file.Read(res->GetPointer(), size);
		} else if (entry.GetCompressType() == ArchiveFileEntry::CompressType::Compress) {
			file.Seek(entry.GetOffset());

			ByteBuffer bufComp;
			bufComp.SetSize(entry.GetCompressedDataSize());
			file.Read(bufComp.GetPointer(), bufComp.GetSize());
			bufComp.Decompress(entry.GetDataSize());
			res = std::make_unique<ByteBuffer>(bufComp);
		}
	}
	return res;
}
/*
std::shared_ptr<ByteBuffer> ArchiveFile::GetBuffer(std::string name)
{
	if (!IsExists(name))
		return nullptr;

	if (!file_->Open())
		return nullptr;

	std::shared_ptr<ByteBuffer> res = new ByteBuffer();
	std::shared_ptr<ArchiveFileEntry> entry = mapEntry_[name];
	int offset = entry->GetOffset();
	int size = entry->GetDataSize();

	res->SetSize(size);
	file_->Seek(offset);
	file_->Read(res->GetPointer(), size);

	file_->Close();
	return res;
}
*/

/**********************************************************
//FileManager
**********************************************************/
FileManager* FileManager::thisBase_ = nullptr;
FileManager::FileManager()
{
}
FileManager::~FileManager()
{
	EndLoadThread();
}
bool FileManager::Initialize()
{
	if (thisBase_ != nullptr)
		return false;
	thisBase_ = this;
	threadLoad_ = std::make_unique<LoadThread>();
	threadLoad_->Start();
	return true;
}
void FileManager::EndLoadThread()
{
	{
		Lock lock(lock_);
		if (threadLoad_ == nullptr)
			return;
		threadLoad_->Stop();
		threadLoad_->Join();
		threadLoad_ = nullptr;
	}
}
bool FileManager::AddArchiveFile(std::string path)
{
	if (mapArchiveFile_.find(path) != mapArchiveFile_.end())
		return true;

	std::unique_ptr<ArchiveFile> file = std::make_unique<ArchiveFile>(path);
	if (!file->Open())
		return false;

	std::set<std::string> listKeyIn = file->GetKeyList();
	std::set<std::string> listKeyCurrent;
	std::map<std::string, std::unique_ptr<ArchiveFile>>::iterator itrFile;
	for (itrFile = mapArchiveFile_.begin(); itrFile != mapArchiveFile_.end(); itrFile++) {
		std::set<std::string> tList = itrFile->second->GetKeyList();
		std::set<std::string>::iterator itrList = tList.begin();
		for (; itrList != tList.end(); itrList++) {
			listKeyCurrent.insert(*itrList);
		}
	}

	std::set<std::string>::iterator itrKey = listKeyIn.begin();
	for (; itrKey != listKeyIn.end(); itrKey++) {
		std::string key = *itrKey;
		if (listKeyCurrent.find(key) == listKeyCurrent.end())
			continue;

		std::string log = StringUtility::Format("archive file entry already exists[%s]", key.c_str());
		Logger::WriteTop(log);
		throw std::runtime_error(log.c_str());
	}

	mapArchiveFile_.insert(std::make_pair(path, std::move(file)));
	return true;
}
bool FileManager::RemoveArchiveFile(std::string path)
{
	mapArchiveFile_.erase(path);
	return true;
}
std::shared_ptr<FileReader> FileManager::GetFileReader(std::string path)
{
	std::string orgPath = path;
	path = PathProperty::GetUnique(path);

	std::shared_ptr<FileReader> res;
	std::unique_ptr<File> fileRaw = std::make_unique<File>(path);
	if (fileRaw->IsExists()) {
		res = std::make_shared<ManagedFileReader>(fileRaw, std::shared_ptr<gstd::ArchiveFileEntry>());
	} else {
		std::vector<std::shared_ptr<ArchiveFileEntry>> listEntry;

		std::map<int, std::string> mapArchivePath;
		std::string key = PathProperty::GetFileName(path);
		std::map<std::string, std::unique_ptr<ArchiveFile>>::iterator itr;
		for (itr = mapArchiveFile_.begin(); itr != mapArchiveFile_.end(); itr++) {
			std::string pathArchive = itr->first;
			if (!itr->second->IsExists(key))
				continue;

			std::unique_ptr<File> file = std::make_unique<File>(pathArchive);
			auto list = itr->second->GetEntryList(key);
			listEntry.insert(listEntry.end(), list.begin(), list.end());
			for (int iEntry = 0; iEntry < list.size(); iEntry++) {
				ptrdiff_t addr = reinterpret_cast<ptrdiff_t>(list[iEntry].get());
				mapArchivePath[addr] = pathArchive;
			}
		}

		if (listEntry.size() == 1) {
			auto& entry = listEntry[0];
			ptrdiff_t addr = reinterpret_cast<ptrdiff_t>(entry.get());
			std::string pathArchive = mapArchivePath[addr];
			std::unique_ptr<File> file = std::make_unique<File>(pathArchive);
			res = std::make_shared<ManagedFileReader>(file, entry);
		} else {
			std::string module = PathProperty::GetModuleDirectory();
			module = PathProperty::GetUnique(module);

			std::string target = StringUtility::ReplaceAll(path, module, "");
			for (int iEntry = 0; iEntry < listEntry.size(); iEntry++) {
				std::shared_ptr<ArchiveFileEntry>& entry = listEntry[iEntry];
				std::string dir = entry->GetDirectory();
				if (target.find(dir) == std::string::npos)
					continue;

				ptrdiff_t addr = reinterpret_cast<ptrdiff_t>(entry.get());
				std::string pathArchive = mapArchivePath[addr];
				auto file = std::make_unique<File>(pathArchive);
				res = std::make_shared<ManagedFileReader>(file, entry);
				break;
			}
		}
	}
	if (res != nullptr)
		res->_SetOriginalPath(orgPath);
	return res;
}

std::shared_ptr<ByteBuffer> FileManager::_GetByteBuffer(ArchiveFileEntry& entry)
{
	std::shared_ptr<ByteBuffer> res;
	try {
		Lock lock(lock_);
		std::string key = entry.GetDirectory() + entry.GetName();
		if (mapByteBuffer_.find(key) != mapByteBuffer_.end()) {
			res = mapByteBuffer_[key];
		} else {
			res = ArchiveFile::CreateEntryBuffer(entry);
			if (res != nullptr)
				mapByteBuffer_[key] = res;
		}
	} catch (...) {
	}

	return res;
}
void FileManager::_ReleaseByteBuffer(ArchiveFileEntry& entry)
{
	{
		Lock lock(lock_);
		std::string key = entry.GetDirectory() + entry.GetName();
		if (mapByteBuffer_.find(key) == mapByteBuffer_.end())
			return;
		std::shared_ptr<ByteBuffer>& buffer = mapByteBuffer_[key];
		if (buffer.use_count() == 2) {
			mapByteBuffer_.erase(key);
		}
	}
}
void FileManager::AddLoadThreadEvent(std::unique_ptr<FileManager::LoadThreadEvent>& event)
{
	{
		Lock lock(lock_);
		if (threadLoad_ == nullptr)
			return;
		threadLoad_->AddEvent(event);
	}
}
void FileManager::AddLoadThreadListener(FileManager::LoadThreadListener* listener)
{
	{
		Lock lock(lock_);
		if (threadLoad_ == nullptr)
			return;
		threadLoad_->AddListener(listener);
	}
}
void FileManager::RemoveLoadThreadListener(FileManager::LoadThreadListener* listener)
{
	{
		Lock lock(lock_);
		if (threadLoad_ == nullptr)
			return;
		threadLoad_->RemoveListener(listener);
	}
}
void FileManager::WaitForThreadLoadComplete()
{
	while (!threadLoad_->IsThreadLoadComplete()) {
		Sleep(1);
	}
}

//FileManager::LoadThread
FileManager::LoadThread::LoadThread()
{
}
FileManager::LoadThread::~LoadThread()
{
}
void FileManager::LoadThread::_Run()
{
	while (this->GetStatus() == Thread::Status::Run) {
		signal_.Wait(10);

		while (this->GetStatus() == Thread::Status::Run) {
			// Logger::WriteTop(StringUtility::Format("ロードイベント取り出し開始"));

			Lock lock(lockEvent_);
			if (listEvent_.size() == 0)
				break;
			auto& event = listEvent_.front();
			//listPath_.erase(event->GetPath());
			listEvent_.pop_front();

			// Logger::WriteTop(StringUtility::Format("ロードイベント取り出し完了：%s", event->GetPath().c_str()));

			// Logger::WriteTop(StringUtility::Format("ロード開始：%s", event->GetPath().c_str()));
			{
				Lock lock(lockListener_);
				std::list<FileManager::LoadThreadListener*>::iterator itr;
				for (itr = listListener_.begin(); itr != listListener_.end(); itr++) {
					FileManager::LoadThreadListener* listener = (*itr);
					if (event->GetListener() == listener)
						listener->CallFromLoadThread(event);
				}
			}
			// Logger::WriteTop(StringUtility::Format("ロード完了：%s", event->GetPath().c_str()));
		}
		Sleep(1); //TODO なぜか待機入れると落ちづらい？
	}

	{
		Lock lock(lockListener_);
		listListener_.clear();
	}
}
void FileManager::LoadThread::Stop()
{
	Thread::Stop();
	signal_.SetSignal(false);
}
bool FileManager::LoadThread::IsThreadLoadComplete()
{
	bool res = false;
	{
		Lock lock(lockEvent_);
		res = listEvent_.size() == 0;
	}
	return res;
}
bool FileManager::LoadThread::IsThreadLoadExists(std::string path)
{
	bool res = false;
	{
		// Lock lock(lockEvent_);
		// res = listPath_.find(path) != listPath_.end();
	}
	return res;
}
void FileManager::LoadThread::AddEvent(std::unique_ptr<FileManager::LoadThreadEvent>& event)
{
	{
		Lock lock(lockEvent_);
		std::string path = event->GetPath();
		if (IsThreadLoadExists(path))
			return;
		listEvent_.push_back(std::move(event));
		// listPath_.insert(path);
		signal_.SetSignal(true);
		signal_.SetSignal(false);
	}
}
void FileManager::LoadThread::AddListener(FileManager::LoadThreadListener* listener)
{
	{
		Lock lock(lockListener_);
		std::list<FileManager::LoadThreadListener*>::iterator itr;
		for (itr = listListener_.begin(); itr != listListener_.end(); itr++) {
			if (*itr == listener)
				return;
		}

		listListener_.push_back(listener);
	}
}
void FileManager::LoadThread::RemoveListener(FileManager::LoadThreadListener* listener)
{
	{
		Lock lock(lockListener_);
		std::list<FileManager::LoadThreadListener*>::iterator itr;
		for (itr = listListener_.begin(); itr != listListener_.end(); itr++) {
			if (*itr != listener)
				continue;
			listListener_.erase(itr);
			break;
		}
	}
}

/**********************************************************
//ManagedFileReader
**********************************************************/
ManagedFileReader::ManagedFileReader(std::unique_ptr<File>& file, const std::shared_ptr<ArchiveFileEntry>& entry)
{
	offset_ = 0;
	file_ = std::move(file);
	entry_ = entry;

	if (!entry_.get()) {
		type_ = FileType::Normal;
	} else if (entry_->GetCompressType() == ArchiveFileEntry::CompressType::Non && entry_.get() != nullptr) {
		type_ = FileType::Archived;
	} else if (entry_->GetCompressType() != ArchiveFileEntry::CompressType::Non && entry_.get() != nullptr) {
		type_ = FileType::Archived_Compressed;
	}
}
ManagedFileReader::~ManagedFileReader()
{
	Close();
}
bool ManagedFileReader::Open()
{
	bool res = false;
	offset_ = 0;
	if (type_ == FileType::Normal) {
		res = file_->Open();
	} else if (type_ == FileType::Archived) {
		res = file_->Open();
		if (res) {
			file_->Seek(entry_->GetOffset());
		}
	} else if (type_ == FileType::Archived_Compressed) {
		buffer_ = FileManager::GetBase()->_GetByteBuffer(*entry_);
		res = buffer_.get() != nullptr;
	}
	return res;
}
void ManagedFileReader::Close()
{
	if (file_ != nullptr)
		file_->Close();
	if (buffer_.get() != nullptr) {
		buffer_.reset();
		FileManager::GetBase()->_ReleaseByteBuffer(*entry_);
	}
}
Sint64 ManagedFileReader::GetFileSize()
{
	int res = 0;
	if (type_ == FileType::Normal)
		res = file_->GetSize();
	else if (type_ == FileType::Archived)
		res = entry_->GetDataSize();
	else if (type_ == FileType::Archived_Compressed && buffer_ != nullptr)
		res = buffer_->GetSize();
	return res;
}
size_t ManagedFileReader::Read(void* buf, size_t size)
{
	size_t res = 0;
	if (type_ == FileType::Normal) {
		res = file_->Read(buf, size);
	} else if (type_ == FileType::Archived) {
		int read = size;
		if (entry_->GetDataSize() < offset_ + size) {
			read = entry_->GetDataSize() - offset_;
		}
		res = file_->Read(buf, read);
	} else if (type_ == FileType::Archived_Compressed) {
		int read = size;
		if (buffer_->GetSize() < offset_ + size) {
			read = buffer_->GetSize() - offset_;
		}
		memcpy(buf, &buffer_->GetPointer()[offset_], read);
		res = read;
	}
	offset_ += res;
	return res;
}
bool ManagedFileReader::SetFilePointerBegin()
{
	bool res = false;
	offset_ = 0;
	if (type_ == FileType::Normal) {
		res = file_->SetFilePointerBegin();
	} else if (type_ == FileType::Archived) {
		res = file_->Seek(entry_->GetOffset());
	} else if (type_ == FileType::Archived_Compressed) {
	}
	return res;
}
bool ManagedFileReader::SetFilePointerEnd()
{
	bool res = false;
	if (type_ == FileType::Normal) {
		res = file_->SetFilePointerEnd();
		offset_ = file_->GetSize();
	} else if (type_ == FileType::Archived) {
		res = file_->Seek(entry_->GetOffset() + entry_->GetDataSize());
		offset_ = entry_->GetDataSize();
	} else if (type_ == FileType::Archived_Compressed) {
		if (buffer_ != nullptr) {
			offset_ = buffer_->GetSize();
			res = true;
		}
	}
	return res;
}
bool ManagedFileReader::Seek(Sint64 offset)
{
	bool res = false;
	if (type_ == FileType::Normal) {
		res = file_->Seek(offset);
	} else if (type_ == FileType::Archived) {
		res = file_->Seek(entry_->GetOffset() + offset);
	} else if (type_ == FileType::Archived_Compressed) {
		if (buffer_ != nullptr) {
			res = true;
		}
	}
	if (res == true)
		offset_ = offset;
	return res;
}
Sint64 ManagedFileReader::GetFilePointer()
{
	Sint64 res = 0;
	if (type_ == FileType::Normal) {
		res = file_->GetFilePointer();
	} else if (type_ == FileType::Archived) {
		res = file_->GetFilePointer() - entry_->GetOffset();
	} else if (type_ == FileType::Archived_Compressed) {
		if (buffer_ != nullptr) {
			res = offset_;
		}
	}
	return res;
}
bool ManagedFileReader::IsArchived()
{
	return type_ != FileType::Normal;
}
bool ManagedFileReader::IsCompressed()
{
	return type_ == FileType::Archived_Compressed;
}

/**********************************************************
//RecordEntry
**********************************************************/
RecordEntry::RecordEntry()
{
	type_ = Type::Unknown;
}
RecordEntry::~RecordEntry()
{
}
int RecordEntry::_GetEntryRecordSize()
{
	int res = 0;
	res += sizeof(char);
	res += sizeof(int);
	res += key_.size();
	res += sizeof(int);
	res += buffer_.GetSize();
	return res;
}
void RecordEntry::_WriteEntryRecord(Writer& writer)
{
	writer.WriteCharacter(static_cast<char>(type_));
	writer.WriteInteger(key_.size());
	writer.Write(&key_[0], key_.size());

	writer.WriteInteger(buffer_.GetSize());
	writer.Write(buffer_.GetPointer(), buffer_.GetSize());
}
void RecordEntry::_ReadEntryRecord(Reader& reader)
{
	type_ = static_cast<Type>(reader.ReadCharacter());
	key_.resize(reader.ReadInteger());
	reader.Read(&key_[0], key_.size());

	buffer_.Clear();
	buffer_.SetSize(reader.ReadInteger());
	reader.Read(buffer_.GetPointer(), buffer_.GetSize());
}

/**********************************************************
//RecordBuffer
**********************************************************/
RecordBuffer::RecordBuffer()
{
}
RecordBuffer::~RecordBuffer()
{
	this->Clear();
}
void RecordBuffer::Clear()
{
	mapEntry_.clear();
}

bool RecordBuffer::IsExists(std::string key)
{
	return mapEntry_.find(key) != mapEntry_.end();
}
std::vector<std::string> RecordBuffer::GetKeyList()
{
	std::vector<std::string> res;
	std::map<std::string, std::unique_ptr<RecordEntry>>::iterator itr;
	for (itr = mapEntry_.begin(); itr != mapEntry_.end(); itr++) {
		std::string key = itr->first;
		res.push_back(key);
	}
	return res;
}

void RecordBuffer::Write(Writer& writer)
{
	int countEntry = mapEntry_.size();
	writer.WriteInteger(countEntry);

	std::map<std::string, std::unique_ptr<RecordEntry>>::iterator itr;
	for (itr = mapEntry_.begin(); itr != mapEntry_.end(); itr++) {
		itr->second->_WriteEntryRecord(writer);
	}
}
void RecordBuffer::Read(Reader& reader)
{
	this->Clear();
	int countEntry = reader.ReadInteger();
	for (int iEntry = 0; iEntry < countEntry; iEntry++) {
		std::unique_ptr<RecordEntry> entry = std::make_unique<RecordEntry>();
		entry->_ReadEntryRecord(reader);

		std::string key = entry->GetKey();
		mapEntry_.insert(std::make_pair(key, std::move(entry)));
	}
}
bool RecordBuffer::WriteToFile(std::string path, std::string header)
{
	File file(path);
	if (!file.Create())
		return false;

	file.Write((char*)&header[0], header.size());
	Write(file);
	file.Close();

	return true;
}
bool RecordBuffer::ReadFromFile(std::string path, std::string header)
{
	File file(path);
	if (!file.Open())
		return false;

	std::string fHead;
	fHead.resize(header.size());
	file.Read(&fHead[0], fHead.size());
	if (fHead != header)
		return false;

	Read(file);
	file.Close();

	return true;
}
RecordEntry::Type RecordBuffer::GetEntryType(std::string key)
{
	if (!IsExists(key))
		return RecordEntry::Type::NoEntry;
	return mapEntry_[key]->GetType();
}
size_t RecordBuffer::GetEntrySize(std::string key)
{
	if (!IsExists(key))
		return 0;
	ByteBuffer& buffer = mapEntry_[key]->GetBufferRef();
	return buffer.GetSize();
}
bool RecordBuffer::GetRecord(std::string key, void* buf, size_t size)
{
	if (!IsExists(key))
		return false;
	ByteBuffer& buffer = mapEntry_[key]->GetBufferRef();
	buffer.Seek(0);
	buffer.Read(buf, size);
	return true;
}
bool RecordBuffer::GetRecordAsBoolean(std::string key)
{
	bool res = 0;
	GetRecord(key, res);
	return res;
}
int RecordBuffer::GetRecordAsInteger(std::string key)
{
	int res = 0;
	GetRecord(key, res);
	return res;
}
float RecordBuffer::GetRecordAsFloat(std::string key)
{
	float res = 0;
	GetRecord(key, res);
	return res;
}
double RecordBuffer::GetRecordAsDouble(std::string key)
{
	double res = 0;
	GetRecord(key, res);
	return res;
}
std::string RecordBuffer::GetRecordAsString(std::string key)
{
	if (!IsExists(key))
		return "";

	std::string res;
	RecordEntry::Type type = mapEntry_[key]->GetType();
	ByteBuffer& buffer = mapEntry_[key]->GetBufferRef();
	buffer.Seek(0);
	if (type == RecordEntry::Type::String_A) {
		res.resize(buffer.GetSize());
		buffer.Read(&res[0], buffer.GetSize());
	} else if (type == RecordEntry::Type::String_W) {
		std::wstring wstr;
		wstr.resize(buffer.GetSize() / sizeof(wchar_t));
		buffer.Read(&wstr[0], buffer.GetSize());
		res = StringUtility::ConvertWideToMulti(wstr);
	}
	return res;
}

bool RecordBuffer::GetRecordAsRecordBuffer(std::string key, RecordBuffer& record)
{
	if (!IsExists(key))
		return false;
	ByteBuffer& buffer = mapEntry_[key]->GetBufferRef();
	buffer.Seek(0);
	record.Read(buffer);
	return true;
}
void RecordBuffer::SetRecord(RecordEntry::Type type, std::string key, const void* buf, size_t size)
{
	std::unique_ptr<RecordEntry> entry = std::make_unique<RecordEntry>();
	entry->SetType(type);
	entry->SetKey(key);
	ByteBuffer& buffer = entry->GetBufferRef();
	buffer.SetSize(size);
	buffer.Write(buf, size);
	mapEntry_.insert(std::make_pair(key, std::move(entry)));
}
void RecordBuffer::SetRecordAsRecordBuffer(std::string key, RecordBuffer& record)
{
	std::unique_ptr<RecordEntry> entry = std::make_unique<RecordEntry>();
	entry->SetType(RecordEntry::Type::Record);
	entry->SetKey(key);
	ByteBuffer& buffer = entry->GetBufferRef();
	record.Write(buffer);
	mapEntry_.insert(std::make_pair(key, std::move(entry)));
}

void RecordBuffer::Read(RecordBuffer& record)
{
}
void RecordBuffer::Write(RecordBuffer& record)
{
}

/**********************************************************
//PropertyFile
**********************************************************/
PropertyFile::PropertyFile()
{
}
PropertyFile::~PropertyFile()
{
}
bool PropertyFile::Load(std::string path)
{
	mapEntry_.clear();

	std::vector<char> text;
	FileManager* fileManager = FileManager::GetBase();
	if (fileManager != nullptr) {
		auto reader = fileManager->GetFileReader(path);

		if (reader == nullptr || !reader->Open()) {
			Logger::WriteTop(ErrorUtility::GetFileNotFoundErrorMessage(path));
			return false;
		}

		int size = reader->GetFileSize();
		text.resize(size + 1);
		reader->Read(&text[0], size);
		text[size] = '\0';
	} else {
		File file(path);
		if (!file.Open())
			return false;

		int size = file.GetSize();
		text.resize(size + 1);
		file.Read(&text[0], size);
		text[size] = '\0';
	}

	bool res = false;
	gstd::Scanner scanner(text);
	try {
		while (scanner.HasNext()) {
			gstd::Token& tok = scanner.Next();
			if (tok.GetType() != Token::Type::ID)
				continue;
			std::string key = tok.GetElement();
			while (true) {
				tok = scanner.Next();
				if (tok.GetType() == Token::Type::Equal)
					break;
				key += tok.GetElement();
			}

			std::string value;
			try {
				int posS = scanner.GetCurrentPointer();
				int posE = posS;
				while (true) {
					bool bEndLine = false;
					if (!scanner.HasNext()) {
						bEndLine = true;
					} else {
						tok = scanner.Next();
						bEndLine = tok.GetType() == Token::Type::Newline;
					}

					if (bEndLine) {
						posE = scanner.GetCurrentPointer();
						value = scanner.GetString(posS, posE);
						value = StringUtility::Trim(value);
						break;
					}
				}
			} catch (...) {
			}

			mapEntry_[key] = value;
		}

		res = true;
	} catch (std::exception& e) {
		mapEntry_.clear();
		Logger::WriteTop(
			ErrorUtility::GetParseErrorMessage(path, scanner.GetCurrentLine(), e.what()));
		res = false;
	}
	return res;
}
bool PropertyFile::HasProperty(std::string key)
{
	return mapEntry_.find(key) != mapEntry_.end();
}

std::string PropertyFile::GetString(std::string key, std::string def)
{
	if (!HasProperty(key))
		return def;

	return mapEntry_[key];
}

int PropertyFile::GetInteger(std::string key, int def)
{
	if (!HasProperty(key))
		return def;
	return StringUtility::ToInteger(mapEntry_[key]);
}
double PropertyFile::GetReal(std::string key, double def)
{
	if (!HasProperty(key))
		return def;
	return StringUtility::ToDouble(mapEntry_[key]);
}

/**********************************************************
//Compressor
**********************************************************/
Compressor::Compressor()
{
}
Compressor::~Compressor()
{
}
bool Compressor::Compress(ByteBuffer& bufIn, ByteBuffer& bufOut)
{
	//TODO 要独自の圧縮を実装
	//公開ソースでは、受け渡されたデータをそのまま返す
	bool res = true;

	int inputSize = bufIn.GetSize();
	bufOut.SetSize(inputSize);
	memcpy(bufOut.GetPointer(0), bufIn.GetPointer(0), inputSize);

	return res;
}

/**********************************************************
//DeCompressor
**********************************************************/
DeCompressor::DeCompressor()
{
}
DeCompressor::~DeCompressor()
{
}

bool DeCompressor::DeCompress(ByteBuffer& bufIn, ByteBuffer& bufOut)
{
	//TODO 要独自の圧縮を実装
	//公開ソースでは、受け渡されたデータをそのまま返す
	//why mkm why
	bool res = true;
	//zlib stuff
	/*
	if (!Compressed)
		return 0;*/

	return res;
}

bool DeCompressor::DeCompressHeader(ByteBuffer& bufIn, ByteBuffer& bufOut)
{
	//TODO 要独自の圧縮を実装
	//公開ソースでは、受け渡されたデータをそのまま返す
	//why mkm why
	bool res = true;
	int inputSize = bufIn.GetSize();
	bufOut.SetSize(inputSize);
	memcpy(bufOut.GetPointer(0), bufIn.GetPointer(0), inputSize);
	return res;
}

/**********************************************************
//SystemValueManager
**********************************************************/
const std::string SystemValueManager::RECORD_SYSTEM = "__RECORD_SYSTEM__";
const std::string SystemValueManager::RECORD_SYSTEM_GLOBAL = "__RECORD_SYSTEM_GLOBAL__";
SystemValueManager* SystemValueManager::thisBase_ = nullptr;
SystemValueManager::SystemValueManager()
{
}
SystemValueManager::~SystemValueManager()
{
}
bool SystemValueManager::Initialize()
{
	if (thisBase_ != nullptr)
		return false;

	mapRecord_.insert(std::make_pair(RECORD_SYSTEM, std::make_shared<RecordBuffer>()));
	mapRecord_.insert(std::make_pair(RECORD_SYSTEM_GLOBAL, std::make_shared<RecordBuffer>()));

	thisBase_ = this;
	return true;
}
void SystemValueManager::ClearRecordBuffer(std::string key)
{
	if (!IsExists(key))
		return;
	mapRecord_[key]->Clear();
}
bool SystemValueManager::IsExists(std::string key)
{
	return mapRecord_.find(key) != mapRecord_.end();
}
bool SystemValueManager::IsExists(std::string keyRecord, std::string keyValue)
{
	if (!IsExists(keyRecord))
		return false;
	auto record = GetRecordBuffer(keyRecord);
	return record.lock()->IsExists(keyValue);
}
std::weak_ptr<RecordBuffer> SystemValueManager::GetRecordBuffer(std::string key)
{
	return IsExists(key) ? mapRecord_[key] : std::weak_ptr<RecordBuffer>();
}
