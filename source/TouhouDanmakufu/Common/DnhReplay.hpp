#ifndef __TOUHOUDANMAKUFU_DNHREPLAY__
#define __TOUHOUDANMAKUFU_DNHREPLAY__

#include "DnhCommon.hpp"

/**********************************************************
//ReplayInformation
**********************************************************/
class ReplayInformation {
public:
	enum {
		INDEX_ACTIVE = 0,
		INDEX_DIGIT_MIN = 1,
		INDEX_DIGIT_MAX = 99,
		INDEX_USER = 100,
	};

	class StageData;

private:
	std::wstring path_;
	std::wstring playerScriptID_;
	std::wstring playerScriptFileName_;
	std::wstring playerScriptReplayName_;

	std::wstring comment_;
	std::wstring userName_;
	_int64 totalScore_;
	double fpsAvarage_;
	SYSTEMTIME date_;
	std::shared_ptr<ScriptCommonData> userData_;
	std::map<int, std::shared_ptr<StageData>> mapStageData_;

public:
	ReplayInformation();
	virtual ~ReplayInformation();

	std::wstring GetPath() { return path_; }
	std::wstring GetPlayerScriptID() { return playerScriptID_; }
	void SetPlayerScriptID(std::wstring id) { playerScriptID_ = id; }
	std::wstring GetPlayerScriptFileName() { return playerScriptFileName_; }
	void SetPlayerScriptFileName(std::wstring name) { playerScriptFileName_ = name; }
	std::wstring GetPlayerScriptReplayName() { return playerScriptReplayName_; }
	void SetPlayerScriptReplayName(std::wstring name) { playerScriptReplayName_ = name; }

	std::wstring GetComment() { return comment_; }
	void SetComment(std::wstring comment) { comment_ = comment; }
	std::wstring GetUserName() { return userName_; }
	void SetUserName(std::wstring name) { userName_ = name; }
	_int64 GetTotalScore() { return totalScore_; }
	void SetTotalScore(_int64 score) { totalScore_ = score; }
	double GetAvarageFps() { return fpsAvarage_; }
	void SetAvarageFps(double fps) { fpsAvarage_ = fps; }
	SYSTEMTIME GetDate() { return date_; }
	void SetDate(SYSTEMTIME date) { date_ = date; }
	std::wstring GetDateAsString();

	void SetUserData(std::string key, gstd::value val);
	gstd::value GetUserData(std::string key);
	bool IsUserDataExists(std::string key);

	std::shared_ptr<StageData> GetStageData(int stage) { return mapStageData_[stage]; }
	void SetStageData(int stage, std::shared_ptr<StageData> data) { mapStageData_[stage] = data; }
	std::vector<int> GetStageIndexList();

	bool SaveToFile(std::wstring scriptPath, int index);
	static std::shared_ptr<ReplayInformation> CreateFromFile(std::wstring scriptPath, std::wstring fileName);
	static std::shared_ptr<ReplayInformation> CreateFromFile(std::wstring path);
};

class ReplayInformation::StageData {
public:
	StageData()
	{
		recordKey_ = new gstd::RecordBuffer();
		scoreStart_ = 0;
		scoreLast_ = 0;
	}
	virtual ~StageData() {}

	std::wstring GetMainScriptID() { return mainScriptID_; }
	void SetMainScriptID(std::wstring id) { mainScriptID_ = id; }
	std::wstring GetMainScriptName() { return mainScriptName_; }
	void SetMainScriptName(std::wstring name) { mainScriptName_ = name; }
	std::wstring GetMainScriptRelativePath() { return mainScriptRelativePath_; }
	void SetMainScriptRelativePath(std::wstring path) { mainScriptRelativePath_ = path; }
	_int64 GetStartScore() { return scoreStart_; }
	void SetStartScore(_int64 score) { scoreStart_ = score; }
	_int64 GetLastScore() { return scoreLast_; }
	void SetLastScore(_int64 score) { scoreLast_ = score; }
	_int64 GetGraze() { return graze_; }
	void SetGraze(_int64 graze) { graze_ = graze; }
	_int64 GetPoint() { return point_; }
	void SetPoint(_int64 point) { point_ = point; }
	int GetEndFrame() { return frameEnd_; }
	void SetEndFrame(int frame) { frameEnd_ = frame; }
	int GetRandSeed() { return randSeed_; }
	void SetRandSeed(int seed) { randSeed_ = seed; }
	float GetFramePerSecond(int frame)
	{
		int index = frame / 60;
		int res = index < listFramePerSecond_.size() ? listFramePerSecond_[index] : 0;
		return res;
	}
	void AddFramePerSecond(float frame) { listFramePerSecond_.push_back(frame); }
	double GetFramePerSecondAvarage();
	std::shared_ptr<gstd::RecordBuffer> GetReplayKeyRecord() { return recordKey_; }
	void SetReplayKeyRecord(std::shared_ptr<gstd::RecordBuffer> rec) { recordKey_ = rec; }
	std::set<std::string> GetCommonDataAreaList();
	std::shared_ptr<ScriptCommonData> GetCommonData(std::string area);
	void SetCommonData(std::string area, std::shared_ptr<ScriptCommonData> commonData);

	std::wstring GetPlayerScriptID() { return playerScriptID_; }
	void SetPlayerScriptID(std::wstring id) { playerScriptID_ = id; }
	std::wstring GetPlayerScriptFileName() { return playerScriptFileName_; }
	void SetPlayerScriptFileName(std::wstring name) { playerScriptFileName_ = name; }
	std::wstring GetPlayerScriptReplayName() { return playerScriptReplayName_; }
	void SetPlayerScriptReplayName(std::wstring name) { playerScriptReplayName_ = name; }
	double GetPlayerLife() { return playerLife_; }
	void SetPlayerLife(double life) { playerLife_ = life; }
	double GetPlayerBombCount() { return playerBombCount_; }
	void SetPlayerBombCount(double bomb) { playerBombCount_ = bomb; }
	double GetPlayerPower() { return playerPower_; }
	void SetPlayerPower(double power) { playerPower_ = power; }
	int GetPlayerRebirthFrame() { return playerRebirthFrame_; }
	void SetPlayerRebirthFrame(int frame) { playerRebirthFrame_ = frame; }

	void ReadRecord(gstd::RecordBuffer& record);
	void WriteRecord(gstd::RecordBuffer& record);

private:
	//ステージ情報
	std::wstring mainScriptID_;
	std::wstring mainScriptName_;
	std::wstring mainScriptRelativePath_;

	_int64 scoreStart_;
	_int64 scoreLast_;
	_int64 graze_;
	_int64 point_;
	int frameEnd_;
	int randSeed_;
	std::vector<float> listFramePerSecond_;
	std::shared_ptr<gstd::RecordBuffer> recordKey_;
	std::map<std::string, std::shared_ptr<gstd::RecordBuffer>> mapCommonData_;

	//自機情報
	std::wstring playerScriptID_;
	std::wstring playerScriptFileName_;
	std::wstring playerScriptReplayName_;

	double playerLife_;
	double playerBombCount_;
	double playerPower_;
	int playerRebirthFrame_; //くらいボム有効フレーム
};

/**********************************************************
//ReplayInformationManager
**********************************************************/
class ReplayInformationManager {
public:
	ReplayInformationManager();
	virtual ~ReplayInformationManager();

	void UpdateInformationList(std::wstring pathScript);
	std::vector<int> GetIndexList();
	std::shared_ptr<ReplayInformation> GetInformation(int index);

protected:
	std::map<int, std::shared_ptr<ReplayInformation>> mapInfo_;
};

#endif
