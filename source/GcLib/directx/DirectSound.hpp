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
gstd::ref_count_ptr<SoundPlayer> _CreatePlayer(std::wstring path);

public:
	DirectSoundManager();
	virtual ~DirectSoundManager();
	static DirectSoundManager* GetBase() { return thisBase_; }
	virtual bool Initialize(SDL_Window* hWnd);
	void Clear();

	IDirectSound8* GetDirectSound() { return pDirectSound_; }
	gstd::CriticalSection& GetLock() { return lock_; }

	gstd::ref_count_ptr<SoundPlayer> GetPlayer(std::wstring path, bool bCreateAlways = false);
	gstd::ref_count_ptr<SoundDivision> CreateSoundDivision(int index);
	gstd::ref_count_ptr<SoundDivision> GetSoundDivision(int index);
	gstd::ref_count_ptr<SoundInfo> GetSoundInfo(std::wstring path);

#if 0
	void SetInfoPanel(gstd::ref_count_ptr<SoundInfoPanel> panel)
	{
		gstd::Lock lock(lock_);
		panelInfo_ = panel;
	}
#endif

	bool AddSoundInfoFromFile(std::wstring path);
	std::vector<gstd::ref_count_ptr<SoundInfo>> GetSoundInfoList();
	void SetFadeDeleteAll();

protected:
	IDirectSound8* pDirectSound_;
	IDirectSoundBuffer8* pDirectSoundBuffer_;
	gstd::CriticalSection lock_;
	SoundManageThread* threadManage_;
	std::map<std::wstring, std::list<gstd::ref_count_ptr<SoundPlayer>>> mapPlayer_;
	std::map<int, gstd::ref_count_ptr<SoundDivision>> mapDivision_;
	std::map<std::wstring, gstd::ref_count_ptr<SoundInfo>> mapInfo_;

#if 0
	gstd::ref_count_ptr<SoundInfoPanel> panelInfo_;
#endif

	gstd::ref_count_ptr<SoundPlayer> _GetPlayer(std::wstring path);

private:
	static DirectSoundManager* thisBase_;
};

//フェードイン／フェードアウト制御
//必要なくなったバッファの開放など
class DirectSoundManager::SoundManageThread : public gstd::Thread, public gstd::InnerClass<DirectSoundManager> {
	friend DirectSoundManager;

protected:
	int timeCurrent_;
	int timePrevious_;

	SoundManageThread(DirectSoundManager* manager);
	void _Run();
	void _Arrange(); //必要なくなったデータを削除
	void _Fade(); //フェード処理
};

/**********************************************************
	//SoundInfoPanel
	**********************************************************/
#if 0
class SoundInfoPanel : public gstd::WindowLogger::Panel {
public:
	SoundInfoPanel();
	void SetUpdateInterval(int time) { timeUpdateInterval_ = time; }
	virtual void LocateParts();
	virtual void Update(DirectSoundManager* soundManager);

protected:
	struct Info {
		int address;
		std::wstring path;
		int countRef;
	};
	enum {
		ROW_ADDRESS,
		ROW_FILENAME,
		ROW_FULLPATH,
		ROW_COUNT_REFFRENCE,
	};
	gstd::WListView wndListView_;
	int timeLastUpdate_;
	int timeUpdateInterval_;

	virtual bool _AddedLogger(HWND hTab);
};
#endif

/**********************************************************
	//SoundDivision
	//音量などを共有するためのクラス
	**********************************************************/
class SoundDivision {
public:
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

protected:
	double rateVolume_; //音量割合(0-100)
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
	}
	virtual ~SoundInfo(){};
	std::wstring GetName() { return name_; }
	std::wstring GetTitle() { return title_; }
	double GetLoopStartTime() { return timeLoopStart_; }
	double GetLoopEndTime() { return timeLoopEnd_; }

private:
	std::wstring name_;
	std::wstring title_;
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
	std::wstring GetPath() { return path_; }
	gstd::CriticalSection& GetLock() { return lock_; }
	virtual void Restore() { pDirectSoundBuffer_->Restore(); }
	void SetSoundDivision(gstd::ref_count_ptr<SoundDivision> div);
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
	std::wstring path_;
	gstd::CriticalSection lock_;
	IDirectSoundBuffer8* pDirectSoundBuffer_;
	gstd::ref_count_ptr<gstd::FileReader> reader_;
	gstd::ref_count_ptr<SoundDivision> division_;

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

	virtual bool _CreateBuffer(gstd::ref_count_ptr<gstd::FileReader> reader) = 0;
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
	virtual bool _CreateBuffer(gstd::ref_count_ptr<gstd::FileReader> reader);
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
	virtual bool _CreateBuffer(gstd::ref_count_ptr<gstd::FileReader> reader);
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

	virtual bool _CreateBuffer(gstd::ref_count_ptr<gstd::FileReader> reader);
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
	gstd::ref_count_ptr<gstd::ByteBuffer> bufDecode_;

	virtual bool _CreateBuffer(gstd::ref_count_ptr<gstd::FileReader> reader);
	virtual void _CopyBuffer(LPVOID pMem, DWORD dwSize);
	int _ReadAcmStream(char* pBuffer, int size);
};

} // namespace directx

#endif
