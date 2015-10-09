#ifndef __GET_PITCH__
#define __GET_PITCH__

//#ifdef __cplusplus
//extern "C" {
//#endif

#include "ChangePitch.h"
#include "ChangePitchErrorcode.h"
#include "pitch.h"

class CGetF0Result
{
public:
	CGetF0Result()
		: nFrameNum(0), nF0(0), nResult(VOICE_CHANGE_SUCCESS){}

public:
	//+---------------------------------------------------------------------------+
	//+ 有基频的帧数（whwu）
	//+---------------------------------------------------------------------------+
	int nFrameNum;

	//+---------------------------------------------------------------------------+
	//+ 有基频的总频率（whwu）
	//+---------------------------------------------------------------------------+
	int nF0;

	//+---------------------------------------------------------------------------+
	//+ 返回错误码（whwu）
	//+---------------------------------------------------------------------------+
	int nResult;
};

class CGetF0AveResult
{
public:
	CGetF0AveResult()
		: nAveFrequency(0), nResult(VOICE_CHANGE_SUCCESS){}

public:
	//+---------------------------------------------------------------------------+
	//+ 平均频率（whwu）
	//+---------------------------------------------------------------------------+
	short nAveFrequency;

	//+---------------------------------------------------------------------------+
	//+ 返回错误码（whwu）
	//+---------------------------------------------------------------------------+
	int nResult;
};

class CAveFrequencyProc
{
public:
	CAveFrequencyProc(){}
	~CAveFrequencyProc(){}
	int Init();
	CGetF0Result GetAveFrequency(char* pAudioData, int nAudioLength, EAuidoSampleRate eAuidoSampleRate);
	int UnInit();
private:
	bool CheckSampleRate(EAuidoSampleRate eAuidoSampleRate);
	short* GetData(short* pAudioData, int& nSampleNum, EAuidoSampleRate eInSampleRate, EAuidoSampleRate eOutSampleRate);
	CGetF0Result GetF0(char* pAudioData, int nLength, EAuidoSampleRate eAuidoSampleRate);

#if PITCH_USE_FIX
	TEOTPitcher tPitch;
	char pPitchHeap[VPITCH_HEAP_SIZE];

	//+---------------------------------------------------------------------------+
	//+ 上次处理到的位置（whwu）
	//+---------------------------------------------------------------------------+
	int nLastOne;
#else
	//+---------------------------------------------------------------------------+
	//+ 基频提取核心部分
	//+---------------------------------------------------------------------------+
	PPitcher::pitcher m_oPitcher;
	float m_pPitch[PPitcher::ONLINEF0BUFSIZE];
#endif
};



//#ifdef __cplusplus
//}
//#endif


#endif // __GET_PITCH__