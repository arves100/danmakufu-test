#ifndef __DIRECTX_DIRECTSOUND__
#define __DIRECTX_DIRECTSOUND__

#include "DxConstant.hpp"

namespace directx {

class DirectSoundManager;
class SoundInfoPanel;
class SoundInfo;
class SoundDivision;
class SoundPlayer;
class SoundStreamingPlayer;

class SoundPlayerWave;
class SoundStreamingPlayerWave;
class SoundStreamingPlayerMp3;
class SoundStreamingPlayerOgg;

class AcmBase;
class AcmMp3;
class AcmMp3Wave;

struct WAVEFILEHEADER { //WAVE構成フォーマット情報、"fmt "チャンクデータ
	char cRIFF[4];
	int iSizeRIFF;
	char cType[4];
	char cFmt[4];
	int iSizeFmt;
	WAVEFORMATEX WaveFmt;
	char cData[4];
	int iSizeData;
};

/**********************************************************
//DirectSoundManager
**********************************************************/
class DirectSoundManager {
public:
	class SoundManageThread;
	friend SoundManageThread;
	friend SoundInfoPanel;

public:
	enum {
		SD_VOLUME_MIN = DSBVOLUME_MIN,
		SD_VOLUME_MAX = DSBVOLUME_MAX,
	};
	enum FileFormat {
		SD_MIDI,
		SD_WAVE,
		SD_MP3,
		SD_OGG,
		SD_AWAVE, //圧縮wave waveヘッダmp3
		SD_UNKNOWN,
	};
	
	std::shared_ptr<SoundPlayer> _CreatePlayer(std::string path);

public:
	DirectSoundManager();
	virtual ~DirectSoundManager();
	static DirectSoundManager* GetBase() { return thisBase_; }
	virtual bool Initialize(SDL_Window* hWnd);
	void Clear();

	IDirectSound8* GetDirectSound() { return pDirectSound_; }
	gstd::CriticalSection& GetLock() { return lock_; }

	std::shared_ptr<SoundPlayer> GetPlayer(std::string path, bool bCreateAlways = false);
	SoundDivision& CreateSoundDivision(int index);
	SoundDivision GetSoundDivision(int index);
	SoundInfo GetSoundInfo(std::string path);

	bool AddSoundInfoFromFile(std::string path);
	//std::vector<SoundInfo> GetSoundInfoList();
	void SetFadeDeleteAll();

protected:
	IDirectSound8* pDirectSound_;
	IDirectSoundBuffer8* pDirectSoundBuffer_;
	gstd::CriticalSection lock_;
	SoundManageThread* threadManage_;
	std::map<std::string, std::list<std::shared_ptr<SoundPlayer>>> mapPlayer_;
	std::map<int, SoundDivision> mapDivision_;
	std::map<std::string, SoundInfo> mapInfo_;

	std::shared_ptr<SoundPlayer> _GetPlayer(std::string path);

private:
	static DirectSoundManager* thisBase_;
};

//フェードイン／フェードアウト制御
//必要なくなったバッファの開放など
class DirectSoundManager::SoundManageThread : public gstd::Thread, public gstd::InnerClass<DirectSoundManager> {
	friend DirectSoundManager;

protected:
	Uint32 timeCurrent_;
	Uint32 timePrevious_;

	SoundManageThread(DirectSoundManager* manager);
	void _Run();
	void _Arrange(); //必要なくなったデータを削除
	void _Fade(); //フェード処理
};

/**********************************************************
	//SoundDivision
	//音量などを共有するためのクラス
	**********************************************************/
class SoundDivision {
public:
	friend DirectSoundManager;
	enum {
		DIVISION_BGM = 0,
		DIVISION_SE,
		DIVISION_VOICE,
	};

public:
	SoundDivision();
	virtual ~SoundDivision();
	void SetVolumeRate(double rate) { rateVolume_ = rate; }
	double GetVolumeRate() { return rateVolume_; }
	bool IsValid() { return valid_; }

protected:
	double rateVolume_; //音量割合(0-100)
	bool valid_;
};

/**********************************************************
//SoundInfo
**********************************************************/
class SoundInfo {
	friend DirectSoundManager;

public:
	SoundInfo()
	{
		timeLoopStart_ = 0;
		timeLoopEnd_ = 0;
		valid_ = false;
	}
	virtual ~SoundInfo(){};
	std::string GetName() const { return name_; }
	std::string GetTitle() const { return title_; }
	double GetLoopStartTime() const  { return timeLoopStart_; }
	double GetLoopEndTime() const  { return timeLoopEnd_; }
	bool IsValid() const { return valid_; }

private:
	bool valid_;
	std::string name_;
	std::string title_;
	double timeLoopStart_;
	double timeLoopEnd_;
};

/**********************************************************
//SoundPlayer
**********************************************************/
class SoundPlayer {
	friend DirectSoundManager;
	friend DirectSoundManager::SoundManageThread;

public:
	class PlayStyle;
	enum {
		FADE_DEFAULT = 20,
	};

public:
	SoundPlayer();
	virtual ~SoundPlayer();
	std::string GetPath() { return path_; }
	gstd::CriticalSection& GetLock() { return lock_; }
	virtual void Restore() { pDirectSoundBuffer_->Restore(); }
	void SetSoundDivision(SoundDivision div);
	void SetSoundDivision(int index);

	virtual bool Play();
	virtual bool Play(PlayStyle& style);
	virtual bool Stop();
	virtual bool IsPlaying();
	virtual bool Seek(double time) = 0;
	virtual bool SetVolumeRate(double rateVolume);
	bool SetPanRate(double ratePan);
	double GetVolumeRate();
	void SetFade(double rateVolumeFadePerSec);
	void SetFadeDelete(double rateVolumeFadePerSec);
	void SetAutoDelete(bool bAuto = true) { bAutoDelete_ = bAuto; }
	double GetFadeVolumeRate();
	void Delete() { bDelete_ = true; }
	WAVEFORMATEX GetWaveFormat() { return formatWave_; }

protected:
	DirectSoundManager* manager_;
	std::string path_;
	gstd::CriticalSection lock_;
	IDirectSoundBuffer8* pDirectSoundBuffer_;
	std::shared_ptr<gstd::FileReader> reader_;
	SoundDivision division_;

	WAVEFORMATEX formatWave_;
	bool bLoop_; //ループ有無
	double timeLoopStart_; //ループ開始時間
	double timeLoopEnd_; //ループ終了時間
	bool bPause_;

	bool bDelete_; //削除フラグ
	bool bFadeDelete_; //フェードアウト後に削除
	bool bAutoDelete_; //自動削除
	double rateVolume_; //音量割合(0-100)
	double rateVolumeFadePerSec_; //フェード時の音量低下割合

	virtual bool _CreateBuffer(std::shared_ptr<gstd::FileReader> reader) = 0;
	virtual void _SetSoundInfo();
	static int _GetValumeAsDirectSoundDecibel(float rate);
};
class SoundPlayer::PlayStyle {
public:
	PlayStyle();
	virtual ~PlayStyle();
	void SetLoopEnable(bool bLoop) { bLoop_ = bLoop; }
	bool IsLoopEnable() { return bLoop_; }
	void SetLoopStartTime(double time) { timeLoopStart_ = time; }
	double GetLoopStartTime() { return timeLoopStart_; }
	void SetLoopEndTime(double time) { timeLoopEnd_ = time; }
	double GetLoopEndTime() { return timeLoopEnd_; }
	void SetStartTime(double time) { timeStart_ = time; }
	double GetStartTime() { return timeStart_; }
	bool IsRestart() { return bRestart_; }
	void SetRestart(bool b) { bRestart_ = b; }

private:
	bool bLoop_;
	double timeLoopStart_;
	double timeLoopEnd_;
	double timeStart_;
	bool bRestart_;
};

/**********************************************************
//SoundStreamPlayer
**********************************************************/
class SoundStreamingPlayer : public SoundPlayer {
	class StreamingThread;
	friend StreamingThread;

protected:
	HANDLE hEvent_[3];
	IDirectSoundNotify* pDirectSoundNotify_; //イベント
	int sizeCopy_;
	StreamingThread* thread_;
	bool bStreaming_;
	bool bRequestStop_; //ループ完了時のフラグ。すぐ停止すると最後のバッファが再生されないため。

	void _CreateSoundEvent(WAVEFORMATEX& formatWave);
	virtual void _CopyStream(int indexCopy);
	virtual void _CopyBuffer(LPVOID pMem, DWORD dwSize) = 0;
	void _RequestStop() { bRequestStop_ = true; }

public:
	SoundStreamingPlayer();
	virtual ~SoundStreamingPlayer();

	virtual bool Play(PlayStyle& style);
	virtual bool Stop();
	virtual bool IsPlaying();

};
class SoundStreamingPlayer::StreamingThread : public gstd::Thread, public gstd::InnerClass<SoundStreamingPlayer> {
protected:
	virtual void _Run();

public:
	StreamingThread(SoundStreamingPlayer* player) { _SetOuter(player); }
};

/**********************************************************
//SoundPlayerWave
**********************************************************/
class SoundPlayerWave : public SoundPlayer {
public:
	SoundPlayerWave();
	virtual ~SoundPlayerWave();

	virtual bool Play(PlayStyle& style);
	virtual bool Stop();
	virtual bool IsPlaying();
	virtual bool Seek(double time);

protected:
	virtual bool _CreateBuffer(std::shared_ptr<gstd::FileReader> reader);
};

/**********************************************************
//SoundStreamingPlayerWave
**********************************************************/
class SoundStreamingPlayerWave : public SoundStreamingPlayer {
public:
	SoundStreamingPlayerWave();
	virtual bool Seek(double time);

protected:
	int posWaveStart_;
	int posWaveEnd_;
	virtual bool _CreateBuffer(std::shared_ptr<gstd::FileReader> reader);
	virtual void _CopyBuffer(LPVOID pMem, DWORD dwSize);
};

/**********************************************************
//SoundStreamingPlayerOgg
**********************************************************/
class SoundStreamingPlayerOgg : public SoundStreamingPlayer {
public:
	SoundStreamingPlayerOgg();
	~SoundStreamingPlayerOgg();
	virtual bool Seek(double time);

protected:
	OggVorbis_File fileOgg_;
	ov_callbacks oggCallBacks_;

	virtual bool _CreateBuffer(std::shared_ptr<gstd::FileReader> reader);
	virtual void _CopyBuffer(LPVOID pMem, DWORD dwSize);

	static size_t _ReadOgg(void* ptr, size_t size, size_t nmemb, void* source);
	static int _SeekOgg(void* source, ogg_int64_t offset, int whence);
	static int _CloseOgg(void* source);
	static long _TellOgg(void* source);
};

/**********************************************************
//SoundStreamingPlayerMp3
**********************************************************/
class SoundStreamingPlayerMp3 : public SoundStreamingPlayer {
public:
	SoundStreamingPlayerMp3();
	~SoundStreamingPlayerMp3();
	virtual bool Seek(double time);

protected:
	MPEGLAYER3WAVEFORMAT formatMp3_;
	WAVEFORMATEX formatWave_;
	HACMSTREAM hAcmStream_;
	ACMSTREAMHEADER acmStreamHeader_;
	int posMp3DataStart_;
	int posMp3DataEnd_;
	DWORD waveDataSize_;
	double timeCurrent_;
	std::unique_ptr<gstd::ByteBuffer> bufDecode_;

	virtual bool _CreateBuffer(std::shared_ptr<gstd::FileReader> reader);
	virtual void _CopyBuffer(LPVOID pMem, DWORD dwSize);
	int _ReadAcmStream(char* pBuffer, int size);
};

} // namespace directx

#endif
