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
	//+ �л�Ƶ��֡����whwu��
	//+---------------------------------------------------------------------------+
	int nFrameNum;

	//+---------------------------------------------------------------------------+
	//+ �л�Ƶ����Ƶ�ʣ�whwu��
	//+---------------------------------------------------------------------------+
	int nF0;

	//+---------------------------------------------------------------------------+
	//+ ���ش����루whwu��
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
	//+ ƽ��Ƶ�ʣ�whwu��
	//+---------------------------------------------------------------------------+
	short nAveFrequency;

	//+---------------------------------------------------------------------------+
	//+ ���ش����루whwu��
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
	//+ �ϴδ�����λ�ã�whwu��
	//+---------------------------------------------------------------------------+
	int nLastOne;
#else
	//+---------------------------------------------------------------------------+
	//+ ��Ƶ��ȡ���Ĳ���
	//+---------------------------------------------------------------------------+
	PPitcher::pitcher m_oPitcher;
	float m_pPitch[PPitcher::ONLINEF0BUFSIZE];
#endif
};



//#ifdef __cplusplus
//}
//#endif


#endif // __GET_PITCH__