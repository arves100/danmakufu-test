#ifndef __TOUHOUDANMAKUFU_DNHSTG_COMMON__
#define __TOUHOUDANMAKUFU_DNHSTG_COMMON__

#include "DnhCommon.hpp"
#include "DnhGcLibImpl.hpp"
#include "DnhReplay.hpp"
#include "DnhScript.hpp"

class StgSystemController;
class StgSystemInformation;
class StgStageController;
class StgPackageController;
class StgStageInformation;
class StgSystemInformation;
class StgMovePattern;

/**********************************************************
//StgMoveObject
**********************************************************/
class StgMoveObject {
	friend StgMovePattern;

public:
	StgMoveObject(StgStageController* stageController);
	virtual ~StgMoveObject();

	double GetPositionX() { return posX_; }
	void SetPositionX(double pos) { posX_ = pos; }
	double GetPositionY() { return posY_; }
	void SetPositionY(double pos) { posY_ = pos; }
	double cssn(double s, double ang);

	double GetSpeed();
	void SetSpeed(double speed);
	double GetDirectionAngle();
	void SetDirectionAngle(double angle);

	void SetSpeedX(double speedX);
	void SetSpeedY(double sppedY);

	std::shared_ptr<StgMovePattern>::unsync GetPattern() { return pattern_; }
	void SetPattern(std::shared_ptr<StgMovePattern>::unsync pattern) { pattern_ = pattern; }
	void AddPattern(int frameDelay, std::shared_ptr<StgMovePattern>::unsync pattern);

protected:
	double posX_;
	double posY_;
	std::shared_ptr<StgMovePattern>::unsync pattern_;

	int framePattern_;
	std::map<int, std::shared_ptr<StgMovePattern>::unsync> mapPattern_;
	virtual void _Move();
	void _AttachReservedPattern(std::shared_ptr<StgMovePattern>::unsync pattern);

private:
	StgStageController* stageController_;
};

/**********************************************************
//StgMovePattern
**********************************************************/
class StgMovePattern {
public:
	enum {
		TYPE_OTHER,
		TYPE_ANGLE,
		TYPE_XY,
		TYPE_LINE,

		NO_CHANGE = -256 * 256 * 256,
	};

public:
	StgMovePattern(StgMoveObject* target);
	virtual ~StgMovePattern() {}
	virtual void Move() = 0;
	double cssn(double c, double ang);

	int GetType() { return typeMove_; }

	virtual double GetSpeed() = 0;
	virtual double GetDirectionAngle() = 0;
	int GetShotDataID() { return idShotData_; }
	void SetShotDataID(int id) { idShotData_ = id; }

protected:
	int typeMove_;
	StgMoveObject* target_;

	int frameWork_; //アクティブになるフレーム。
	int idShotData_; //弾画像ID(弾オブジェクト専用)

	StgStageController* _GetStageController() { return target_->stageController_; }
	std::shared_ptr<StgMoveObject>::unsync _GetMoveObject(int id);
	virtual void _Activate() {}
};

class StgMovePattern_Angle : public StgMovePattern {
public:
	StgMovePattern_Angle(StgMoveObject* target);
	virtual void Move();

	virtual double GetSpeed() { return speed_; }
	virtual double GetDirectionAngle() { return angDirection_; }

	void SetSpeed(double speed) { speed_ = speed; }
	void SetDirectionAngle(double angle) { angDirection_ = angle; }
	void SetAcceleration(double accel) { acceleration_ = accel; }
	void SetMaxSpeed(double max) { maxSpeed_ = max; }
	void SetAngularVelocity(double av) { angularVelocity_ = av; }
	void SetRelativeObjectID(int id) { idRalativeID_ = id; }

protected:
	double speed_;
	double angDirection_;
	double acceleration_;
	double maxSpeed_;
	double angularVelocity_;
	int idRalativeID_;

	virtual void _Activate();
};

class StgMovePattern_XY : public StgMovePattern {
public:
	StgMovePattern_XY(StgMoveObject* target);
	virtual void Move();

	virtual double GetSpeed();
	virtual double GetDirectionAngle();

	double GetSpeedX() { return speedX_; }
	double GetSpeedY() { return speedY_; }
	void SetSpeedX(double value) { speedX_ = value; }
	void SetSpeedY(double value) { speedY_ = value; }
	void SetAccelerationX(double value) { accelerationX_ = value; }
	void SetAccelerationY(double value) { accelerationY_ = value; }
	void SetMaxSpeedX(double value) { maxSpeedX_ = value; }
	void SetMaxSpeedY(double value) { maxSpeedY_ = value; }

protected:
	double speedX_;
	double speedY_;
	double accelerationX_;
	double accelerationY_;
	double maxSpeedX_;
	double maxSpeedY_;
};

class StgMovePattern_Line : public StgMovePattern {
protected:
	enum {
		TYPE_SPEED,
		TYPE_FRAME,
		TYPE_WEIGHT,
		TYPE_NONE,
	};

public:
	StgMovePattern_Line(StgMoveObject* target);
	virtual void Move();
	virtual double GetSpeed() { return speed_; }
	virtual double GetDirectionAngle() { return angDirection_; }

	void SetAtSpeed(double tx, double ty, double speed);
	void SetAtFrame(double tx, double ty, double frame);
	void SetAtWait(double tx, double ty, double weight, double maxSpeed);

protected:
	int typeLine_;
	double speed_;
	double angDirection_;
	double weight_;
	double maxSpeed_;
	int frameStop_;
	double toX_;
	double toY_;
};

#endif
