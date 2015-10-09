#ifndef __CHANGE_PITCH_CONTEXT__
#define __CHANGE_PITCH_CONTEXT__

#include "ChangePitch.h"
#include "ChangePitchErrorcode.h"

class ChangePitchStrategyInterface
{
public:
	virtual int Process(float * pFInput,int nSamples) = 0;
	virtual int Init(EAuidoSampleRate eAuidoSampleRate) = 0;
	virtual int Uninit() = 0;
};

class ChangePitchContext
{
public:
	ChangePitchContext();
	int Init(EChangeType echangetype,EAuidoSampleRate eAuidoSampleRate);
	int Uninit();
	int Process(float * pFInput,int nSamples);

private: 
	bool _init;
	ChangePitchStrategyInterface * _pChangePitchStrategy;
};

class EffectEcho;
class CAveFrequencyProc;

class ChangePitchStrategy : public ChangePitchStrategyInterface
{
public:
	ChangePitchStrategy();
	virtual int Process(float * pFInput,int nSamples);
	int Init(EAuidoSampleRate eAuidoSampleRate);
	int Uninit();
	virtual void GetToPitch(float * pFInput,int nSamples);
	virtual void SetToPitch() = 0;
	float _fToPitch;
	float _fPitch;
	bool _bPitchAssigned;

	int _nFrameNum;
	int _nF0;
	int _nAveF0;
	float _fSize;
	float _fLatency;	
	EAuidoSampleRate _eEAuidoSampleRate;
	struct F0Result
	{
		int _nFrameNum;
		int _nF0;
	};
	void * _pInst;
	CAveFrequencyProc * _pFrequeceProc;
};

class StrategyToXiaoHuangren : public ChangePitchStrategy
{
public:
	StrategyToXiaoHuangren(){}
	void SetToPitch();
};

class StrategyToBaby : public ChangePitchStrategy
{
public:
	StrategyToBaby(){}
	void SetToPitch();
};

class StrategyToTomcat : public ChangePitchStrategy
{
public:
	StrategyToTomcat(){}
	void SetToPitch();
};

class StrategyToSantaclaus : public ChangePitchStrategy
{
public:
	StrategyToSantaclaus(){}
	void GetToPitch(float * pFInput,int nSamples);
	void SetToPitch();
};

class StrategyToTransformer : public ChangePitchStrategy
{
public:
	StrategyToTransformer(){}
	void GetToPitch(float * pFInput,int nSamples);
	void SetToPitch();
};

class StrategyToEcho : public ChangePitchStrategyInterface
{
public:
	StrategyToEcho();
	virtual int Process(float * pFInput,int nSamples);
	virtual int Init(EAuidoSampleRate eAuidoSampleRate);
	virtual int Uninit();
private:
	EffectEcho * _pEffectEcho;
};

class StrategyToRobot : public ChangePitchStrategyInterface
{
public:
	StrategyToRobot();
	int Process(float * pFInput,int nSamples);
	int Init(EAuidoSampleRate eAuidoSampleRate);
	int Uninit();
private:
	EffectEcho * _pEffectEcho;
};

class StrategyToMan : public ChangePitchStrategy
{
public:
	StrategyToMan(){}
	void GetToPitch(float * pFInput,int nSamples);
	void SetToPitch();
};

class StrategyToWoman : public ChangePitchStrategy
{
public:
	StrategyToWoman(){}
	void GetToPitch(float * pFInput,int nSamples);
	void SetToPitch();
};

class StrategyToOlder : public ChangePitchStrategy
{
public:
	StrategyToOlder(){}
	void GetToPitch(float * pFInput,int nSamples);
	void SetToPitch();
};

#endif // __CHANGE_PITCH_CONTEXT__
