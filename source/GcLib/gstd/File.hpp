#ifndef __GSTD_FILE__
#define __GSTD_FILE__

#include "GstdConstant.hpp"
#include "GstdUtility.hpp"
#include "Thread.hpp"

namespace gstd {

const std::string HEADER_ARCHIVEFILE = "ArchiveFile";
const std::string HEADER_RECORDFILE = "RecordBufferFile";

class ByteBuffer;
class FileManager;
/**********************************************************
//Writer
**********************************************************/
class Writer {
public:
	virtual ~Writer(){};
	virtual size_t Write(const void* buf, size_t size) = 0;
	template <typename T>
	size_t Write(T& data)
	{
		return Write(&data, sizeof(T));
	}
	void WriteBoolean(bool b) { Write(b); }
	void WriteCharacter(char ch) { Write(ch); }
	void WriteShort(int16_t num) { Write(num); }
	void WriteInteger(int32_t num) { Write(num); }
	void WriteInteger64(int64_t num) { Write(num); }
	void WriteFloat(float num) { Write(num); }
	void WriteDouble(double num) { Write(num); }
};

/**********************************************************
//Reader
**********************************************************/
class Reader {
public:
	virtual ~Reader(){};
	virtual size_t Read(void* buf, size_t size) = 0;
	template <typename T>
	size_t Read(T& data)
	{
		return Read(&data, sizeof(T));
	}
	bool ReadBoolean()
	{
		bool res;
		Read(res);
		return res;
	}
	char ReadCharacter()
	{
		char res;
		Read(res);
		return res;
	}
	int16_t ReadShort()
	{
		int16_t res;
		Read(res);
		return res;
	}
	int32_t ReadInteger()
	{
		int32_t num;
		Read(num);
		return num;
	}
	int64_t ReadInteger64()
	{
		int64_t num;
		Read(num);
		return num;
	}
	float ReadFloat()
	{
		float num;
		Read(num);
		return num;
	}
	double ReadDouble()
	{
		double num;
		Read(num);
		return num;
	}

	std::string ReadString(int size)
	{
		std::string res = "";
		res.resize(size);
		Read(&res[0], size);
		return res;
	}
};

/**********************************************************
//FileReader
**********************************************************/
class FileReader : public Reader {
	friend FileManager;

public:
	virtual bool Open() = 0;
	virtual void Close() = 0;
	virtual Sint64 GetFileSize() = 0;
	virtual bool SetFilePointerBegin() = 0;
	virtual bool SetFilePointerEnd() = 0;
	virtual bool Seek(Sint64 offset) = 0;
	virtual Sint64 GetFilePointer() = 0;
	virtual bool IsArchived() { return false; }
	virtual bool IsCompressed() { return false; }

	std::string GetOriginalPath() { return pathOriginal_; }
	std::string ReadAllString()
	{
		SetFilePointerBegin();
		return ReadString(GetFileSize());
	}

protected:
	std::string pathOriginal_;
	void _SetOriginalPath(std::string path) { pathOriginal_ = path; }
};

/**********************************************************
//ByteBuffer
**********************************************************/
class ByteBuffer : public Writer, public Reader {
public:
	ByteBuffer();
	ByteBuffer(ByteBuffer& buffer);
	virtual ~ByteBuffer();
	void Copy(ByteBuffer& src);
	void Clear();

	void Seek(size_t pos);
	void SetSize(size_t size);
	size_t GetSize() { return size_; }
	size_t GetOffset() { return offset_; }
	size_t Decompress();
	size_t Decompress(size_t size);

	virtual size_t Write(const void* buf, size_t size);
	virtual size_t Read(void* buf, size_t size);

	char* GetPointer(size_t offset = 0);

protected:
	size_t reserve_;
	size_t size_;
	size_t offset_;
	char* data_;

	int _GetReservedSize();
	void _Resize(size_t size);
};

/**********************************************************
//File
//ファイルは、x:\fffff.xxx
//ディレクトリはx:\ddddd\
**********************************************************/
class File : public Writer, public Reader {
public:
	enum class AccessType {
		Read,
		Write,
	};

public:
	File();
	File(std::string path);
	virtual ~File();
	bool CreateDirectory();
	void Delete();
	bool IsExists();
	static bool IsExists(std::string path);
	bool IsDirectory();

	Sint64 GetSize();
	std::string& GetPath() { return path_; }
	SDL_RWops* GetHandle() { return hFile_; }

	virtual bool Open();
	bool Open(AccessType typeAccess);
	bool Create();
	void Close();

	virtual size_t Write(const void* buf, size_t size);
	virtual size_t Read(void* buf, size_t size);

	bool SetFilePointerBegin() { return ( SDL_RWseek(hFile_, 0, RW_SEEK_SET) != 0xFFFFFFFF); }
	bool SetFilePointerEnd() { return ( SDL_RWseek(hFile_, 0, RW_SEEK_END) != 0xFFFFFFFF); }
	bool Seek(Sint64 offset, int seek = RW_SEEK_SET) { return ( SDL_RWseek(hFile_, offset, seek) != 0xFFFFFFFF); }
	Sint64 GetFilePointer() { return SDL_RWseek(hFile_, 0, RW_SEEK_CUR); }

	static bool IsEqualsPath(std::string path1, std::string path2);
	static std::vector<std::string> GetFilePathList(std::string dir);
	static std::vector<std::string> GetDirectoryPathList(std::string dir);

protected:
	SDL_RWops* hFile_;
	std::string path_;
};

/**********************************************************
//ArchiveFileEntry
**********************************************************/
class FileArchiver;
class ArchiveFile;

class ArchiveFileEntry {
	friend FileArchiver;
	friend ArchiveFile;

public:
	enum class CompressType {
		Non,
		Compress,
	};

public:
	ArchiveFileEntry();
	virtual ~ArchiveFileEntry();

	void SetDirectory(std::string dir) { dir_ = dir; }
	std::string& GetDirectory() { return dir_; }
	void SetName(std::string name) { name_ = name; }
	std::string& GetName() { return name_; }
	void SetCompressType(CompressType type) { typeCompress_ = type; }
	CompressType& GetCompressType() { return typeCompress_; }

	int GetOffset() { return offset_; }
	int GetDataSize() { return sizeData_; }
	int GetCompressedDataSize() { return sizeCompressed_; }

	std::string& GetArchivePath() { return pathArchive_; }

private:
	std::string dir_;
	std::string name_;
	CompressType typeCompress_;
	int sizeData_;
	int sizeCompressed_;
	int offset_;
	std::string pathArchive_;

	int _GetEntryRecordSize();
	void _WriteEntryRecord(ByteBuffer& buf);
	void _ReadEntryRecord(ByteBuffer& buf);

	void _SetOffset(int offset) { offset_ = offset; }
	void _SetDataSize(int size) { sizeData_ = size; }
	void _SetCompressedDataSize(int size) { sizeCompressed_ = size; }
	void _SetArchivePath(std::string path) { pathArchive_ = path; }
};

/**********************************************************
//FileArchiver
**********************************************************/
class FileArchiver {
public:
	FileArchiver();
	virtual ~FileArchiver();

	void AddEntry(std::unique_ptr<ArchiveFileEntry>&& entry) { listEntry_.push_back(std::move(entry)); }
	bool CreateArchiveFile(std::string path);

private:
	std::list<std::unique_ptr<ArchiveFileEntry>> listEntry_;
};

/**********************************************************
//ArchiveFile
**********************************************************/
class ArchiveFile {
public:
	ArchiveFile(std::string path);
	virtual ~ArchiveFile();
	bool Open();
	void Close();

	std::set<std::string> GetKeyList();
	std::multimap<std::string, std::shared_ptr<ArchiveFileEntry>>& GetEntryMap() { return mapEntry_; }
	std::vector<std::shared_ptr<ArchiveFileEntry>> GetEntryList(std::string name);
	bool IsExists(std::string name);
	static std::unique_ptr<ByteBuffer> CreateEntryBuffer(ArchiveFileEntry& entry);
	//std::shared_ptr<ByteBuffer> GetBuffer(std::string name);
private:
	std::unique_ptr<File> file_;
	std::multimap<std::string, std::shared_ptr<ArchiveFileEntry>> mapEntry_;
};

/**********************************************************
//FileManager
**********************************************************/
class ManagedFileReader;
class FileManager {
public:
	class LoadObject;
	class LoadThread;
	class LoadThreadListener;
	class LoadThreadEvent;
	friend ManagedFileReader;

public:
	static FileManager* GetBase() { return thisBase_; }
	FileManager();
	virtual ~FileManager();
	virtual bool Initialize();
	void EndLoadThread();
	bool AddArchiveFile(std::string path);
	bool RemoveArchiveFile(std::string path);
	std::shared_ptr<FileReader> GetFileReader(std::string path);

	void AddLoadThreadEvent(std::unique_ptr<LoadThreadEvent>& event);
	void AddLoadThreadListener(FileManager::LoadThreadListener* listener);
	void RemoveLoadThreadListener(FileManager::LoadThreadListener* listener);
	void WaitForThreadLoadComplete();

protected:
	gstd::CriticalSection lock_;
	std::unique_ptr<LoadThread> threadLoad_;
	std::map<std::string, std::unique_ptr<ArchiveFile>> mapArchiveFile_;
	std::map<std::string, std::shared_ptr<ByteBuffer>> mapByteBuffer_;

	std::shared_ptr<ByteBuffer> _GetByteBuffer(ArchiveFileEntry& entry);
	void _ReleaseByteBuffer(ArchiveFileEntry& entry);

private:
	static FileManager* thisBase_;
};

class FileManager::LoadObject {
public:
	virtual ~LoadObject(){};
};

class FileManager::LoadThread : public Thread {
public:
	LoadThread();
	virtual ~LoadThread();
	virtual void Stop();
	bool IsThreadLoadComplete();
	bool IsThreadLoadExists(std::string path);
	void AddEvent(std::unique_ptr<FileManager::LoadThreadEvent>& event);
	void AddListener(FileManager::LoadThreadListener* listener);
	void RemoveListener(FileManager::LoadThreadListener* listener);

protected:
	virtual void _Run();
	// std::set<std::string> listPath_;
	std::list<std::unique_ptr<FileManager::LoadThreadEvent>> listEvent_;
	std::list<FileManager::LoadThreadListener*> listListener_;

private:
	gstd::CriticalSection lockEvent_;
	gstd::CriticalSection lockListener_;
	gstd::ThreadSignal signal_;
};

class FileManager::LoadThreadListener {
public:
	virtual ~LoadThreadListener() {}
	virtual void CallFromLoadThread(std::unique_ptr<FileManager::LoadThreadEvent>& event) = 0;
};

class FileManager::LoadThreadEvent {
protected:
	FileManager::LoadThreadListener* listener_;
	std::string path_;
	std::shared_ptr<FileManager::LoadObject> source_;

public:
	LoadThreadEvent(FileManager::LoadThreadListener* listener, std::string path, std::shared_ptr<FileManager::LoadObject> source)
	{
		listener_ = listener;
		path_ = path;
		source_ = source;
	};
	virtual ~LoadThreadEvent() {}

	FileManager::LoadThreadListener* GetListener() { return listener_; }
	std::string GetPath() { return path_; }
	std::shared_ptr<FileManager::LoadObject> GetSource() { return source_; }
};

/**********************************************************
//ManagedFileReader
**********************************************************/
class ManagedFileReader : public FileReader {
public:
	ManagedFileReader(std::unique_ptr<File>& file, const std::shared_ptr<ArchiveFileEntry>& entry);
	~ManagedFileReader();

	virtual bool Open();
	virtual void Close();
	virtual Sint64 GetFileSize();
	virtual size_t Read(void* buf, size_t size);
	virtual bool SetFilePointerBegin();
	virtual bool SetFilePointerEnd();
	virtual bool Seek(Sint64 offset);
	virtual Sint64 GetFilePointer();

	virtual bool IsArchived();
	virtual bool IsCompressed();

private:
	enum class FileType {
		Normal,
		Archived,
		Archived_Compressed,
	};

	FileType type_;
	std::unique_ptr<File> file_;
	std::shared_ptr<ArchiveFileEntry> entry_;
	std::shared_ptr<ByteBuffer> buffer_;
	Sint64 offset_;
};

/**********************************************************
//Recordable
**********************************************************/
class Recordable;
class RecordEntry;
class RecordBuffer;
class Recordable {
public:
	virtual ~Recordable() {}
	virtual void Read(RecordBuffer& record) = 0;
	virtual void Write(RecordBuffer& record) = 0;
};

/**********************************************************
//RecordEntry
**********************************************************/
class RecordEntry {
	friend RecordBuffer;

public:
	enum class Type : char {
		NoEntry = -2,
		Unknown = -1,
		Boolean = 1,
		Integer = 2,
		Float = 3,
		Double = 4,
		String_A = 5,
		Record = 6,
		String_W = 7,
	};

public:
	RecordEntry();
	virtual ~RecordEntry();
	Type GetType() { return type_; }

	void SetKey(std::string key) { key_ = key; }
	void SetType(Type type) { type_ = type; }
	std::string& GetKey() { return key_; }
	ByteBuffer& GetBufferRef() { return buffer_; }

private:
	Type type_;
	std::string key_;
	ByteBuffer buffer_;

	int _GetEntryRecordSize();
	void _WriteEntryRecord(Writer& writer);
	void _ReadEntryRecord(Reader& reader);
};

/**********************************************************
//RecordBuffer
**********************************************************/
class RecordBuffer : public Recordable {
public:
	RecordBuffer();
	virtual ~RecordBuffer();
	void Clear(); //保持データクリア
	size_t GetEntryCount() const { return mapEntry_.size(); }
	bool IsExists(std::string key);
	std::vector<std::string> GetKeyList();

	void Write(Writer& writer);
	void Read(Reader& reader);
	bool WriteToFile(std::string path, std::string header = HEADER_RECORDFILE);
	bool ReadFromFile(std::string path, std::string header = HEADER_RECORDFILE);

	//エントリ
	RecordEntry::Type GetEntryType(std::string key);
	size_t GetEntrySize(std::string key);

	//エントリ取得(文字列キー)
	bool GetRecord(std::string key, void* buf, size_t size);
	template <typename T>
	bool GetRecord(std::string key, T& data)
	{
		return GetRecord(key, &data, sizeof(T));
	}
	bool GetRecordAsBoolean(std::string key);
	int GetRecordAsInteger(std::string key);
	float GetRecordAsFloat(std::string key);
	double GetRecordAsDouble(std::string key);
	std::string GetRecordAsString(std::string key);
	bool GetRecordAsRecordBuffer(std::string key, RecordBuffer& record);

	//エントリ取得(数値キー)
	bool GetRecord(RecordEntry::Type key, void* buf, size_t size) { return GetRecord(StringUtility::Format("%d", key), buf, size); }
	template <typename T>
	bool GetRecord(int key, T& data) { return GetRecord(StringUtility::Format("%d", key), data); }
	bool GetRecordAsBoolean(int key) { return GetRecordAsBoolean(StringUtility::Format("%d", key)); };
	int GetRecordAsInteger(int key) { return GetRecordAsInteger(StringUtility::Format("%d", key)); }
	float GetRecordAsFloat(int key) { return GetRecordAsFloat(StringUtility::Format("%d", key)); }
	double GetRecordAsDouble(int key) { return GetRecordAsDouble(StringUtility::Format("%d", key)); }
	std::string GetRecordAsString(int key) { return GetRecordAsString(StringUtility::Format("%d", key)); }
	bool GetRecordAsRecordBuffer(int key, RecordBuffer& record) { return GetRecordAsRecordBuffer(StringUtility::Format("%d", key), record); }

	//エントリ設定(文字列キー)
	void SetRecord(std::string key, const void* buf, size_t size) { SetRecord(RecordEntry::Type::Unknown, key, buf, size); }
	template <typename T>
	void SetRecord(std::string key, T& data)
	{
		SetRecord(RecordEntry::Type::Unknown, key, &data, sizeof(T));
	}
	void SetRecord(RecordEntry::Type type, std::string key, const void* buf, size_t size);
	template <typename T>
	void SetRecord(RecordEntry::Type type, std::string key, T& data)
	{
		SetRecord(type, key, &data, sizeof(T));
	}
	void SetRecordAsBoolean(std::string key, bool data) { SetRecord(RecordEntry::Type::Boolean, key, data); }
	void SetRecordAsInteger(std::string key, int data) { SetRecord(RecordEntry::Type::Integer, key, data); }
	void SetRecordAsFloat(std::string key, float data) { SetRecord(RecordEntry::Type::Float, key, data); }
	void SetRecordAsDouble(std::string key, double data) { SetRecord(RecordEntry::Type::Double, key, data); }
	void SetRecordAsString(std::string key, std::string data) { SetRecord(RecordEntry::Type::String_A, key, &data[0], data.size()); }

	// As we are switching to UTF-8, keeping std::wstring is pointless at this time, only Get will be kept for compatibility purpouse
	//void SetRecordAsStringW(std::string key, std::wstring data) { SetRecord(RecordEntry::Type::String_W, key, &data[0], data.size() * sizeof(wchar_t)); }

	void SetRecordAsRecordBuffer(std::string key, RecordBuffer& record);

	//エントリ設定(数値キー)
	void SetRecord(RecordEntry::Type key, const void* buf, size_t size) { SetRecord(StringUtility::Format("%d", key), buf, size); }
	template <typename T>
	void SetRecord(RecordEntry::Type key, T& data) { SetRecord(StringUtility::Format("%d", key), data); }
	void SetRecordAsBoolean(RecordEntry::Type key, bool data) { SetRecordAsInteger(StringUtility::Format("%d", key), data); }
	void SetRecordAsInteger(RecordEntry::Type key, int data) { SetRecordAsInteger(StringUtility::Format("%d", key), data); }
	void SetRecordAsFloat(RecordEntry::Type key, float data) { SetRecordAsFloat(StringUtility::Format("%d", key), data); }
	void SetRecordAsDouble(RecordEntry::Type key, double data) { SetRecordAsDouble(StringUtility::Format("%d", key), data); }
	void SetRecordAsString(RecordEntry::Type key, std::string data) { SetRecordAsString(StringUtility::Format("%d", key), data); }
	void SetRecordAsRecordBuffer(RecordEntry::Type key, RecordBuffer& record) { SetRecordAsRecordBuffer(StringUtility::Format("%d", key), record); }

	//Recoedable
	virtual void Read(RecordBuffer& record);
	virtual void Write(RecordBuffer& record);

private:
	std::map<std::string, std::unique_ptr<RecordEntry>> mapEntry_;
};

/**********************************************************
//PropertyFile
**********************************************************/
class PropertyFile {
public:
	PropertyFile();
	virtual ~PropertyFile();

	bool Load(std::string path);

	bool HasProperty(std::string key);
	std::string GetString(std::string key) { return GetString(key, ""); }
	std::string GetString(std::string key, std::string def);
	int GetInteger(std::string key) { return GetInteger(key, 0); }
	int GetInteger(std::string key, int def);
	double GetReal(std::string key) { return GetReal(key, 0.0); }
	double GetReal(std::string key, double def);

protected:
	std::map<std::string, std::string> mapEntry_;
};

/**********************************************************
//Compressor
**********************************************************/
class Compressor {
protected:
public:
	Compressor();
	virtual ~Compressor();
	bool Compress(ByteBuffer& bufIn, ByteBuffer& bufOut);
};

/**********************************************************
//DeCompressor
**********************************************************/
class DeCompressor {
public:
	DeCompressor();
	virtual ~DeCompressor();
	bool DeCompress(ByteBuffer& bufIn, ByteBuffer& bufOut);
	bool DeCompressHeader(ByteBuffer& bufIn, ByteBuffer& bufOut);

protected:
};

/**********************************************************
//SystemValueManager
**********************************************************/
class SystemValueManager {
public:
	const static std::string RECORD_SYSTEM;
	const static std::string RECORD_SYSTEM_GLOBAL;

public:
	SystemValueManager();
	virtual ~SystemValueManager();
	static SystemValueManager* GetBase() { return thisBase_; }
	virtual bool Initialize();

	virtual void ClearRecordBuffer(std::string key);
	bool IsExists(std::string key);
	bool IsExists(std::string keyRecord, std::string keyValue);
	std::weak_ptr<RecordBuffer> GetRecordBuffer(std::string key);

protected:
	std::map<std::string, std::shared_ptr<RecordBuffer>> mapRecord_;

private:
	static SystemValueManager* thisBase_;
};

} // namespace gstd

#endif
