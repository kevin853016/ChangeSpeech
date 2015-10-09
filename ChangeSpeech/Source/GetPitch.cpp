#include "GetPitch.h"
#include "ChangePitch.h"

//+---------------------------------------------------------------------------+
//+ 基频计算缓冲区大小（whwu）
//+---------------------------------------------------------------------------+
#define VPITCH_HEAP_SIZE		10240

//+---------------------------------------------------------------------------+
//+ 基频数据缓冲区大小（whwu）
//+---------------------------------------------------------------------------+
#define PITCH_DATA_SIZE			2048

//+---------------------------------------------------------------------------+
//+ 采样精度（whwu）
//+---------------------------------------------------------------------------+
#define AUDIO_SAMPLE_BITS		16	

//+---------------------------------------------------------------------------+
//+ 通道数（whwu）
//+---------------------------------------------------------------------------+
#define AUDIO_CHANNEL_NUM		1

//+---------------------------------------------------------------------------+
//+ 每次变声的处理数据块大小（whwu）
//+---------------------------------------------------------------------------+
#define VOICECHANGE_BUFF_SIZE	4096

#if !FLOAT_OPTIMIZE
//+---------------------------------------------------------------------------+
//+ 浮点处理参数（whwu）
//+---------------------------------------------------------------------------+
float g_fScale = 1.0 / 32768.0;
#endif

//#if PITCH_USE_FIX
//TEOTPitcher tPitch;
//char pPitchHeap[VPITCH_HEAP_SIZE];
//
////+---------------------------------------------------------------------------+
////+ 上次处理到的位置（whwu）
////+---------------------------------------------------------------------------+
//int nLastOne;
//#else
////+---------------------------------------------------------------------------+
////+ 基频提取核心部分
////+---------------------------------------------------------------------------+
//PPitcher::pitcher m_oPitcher;
//float pPitch[PPitcher::ONLINEF0BUFSIZE];
//#endif

int CAveFrequencyProc::Init()
{
	int nRet;
	nRet = m_oPitcher.online_reset();
	memset(m_pPitch, 0, PPitcher::ONLINEF0BUFSIZE * sizeof(float));
	return nRet;
}

int CAveFrequencyProc::UnInit()
{
	int nRet;
	nRet = m_oPitcher.online_reset();
	memset(m_pPitch, 0, PPitcher::ONLINEF0BUFSIZE * sizeof(float));
	return nRet;
}

bool CAveFrequencyProc::CheckSampleRate(EAuidoSampleRate eAuidoSampleRate)
{
	return ( eAuidoSampleRate == AudioSampleRate_8k || 
		eAuidoSampleRate == AudioSampleRate_16k || 
		eAuidoSampleRate == AudioSampleRate_32k || 
		eAuidoSampleRate == AudioSampleRate_48k || 
		eAuidoSampleRate == AudioSampleRate_11k || 
		eAuidoSampleRate == AudioSampleRate_22k || 
		eAuidoSampleRate == AudioSampleRate_44k );
}

short* CAveFrequencyProc::GetData(short* pAudioData, int& nSampleNum, EAuidoSampleRate eInSampleRate, EAuidoSampleRate eOutSampleRate)
{
	int nTempNum = (int)eInSampleRate / (int)eOutSampleRate;
	nSampleNum = nSampleNum / nTempNum;

	short* pAudioTempData = new short[nSampleNum];
	for ( int i = 0 ; i < nSampleNum; i++ )
	{
		pAudioTempData[i] = pAudioData[i * nTempNum];
	}

	return pAudioTempData;
}

//+---------------------------------------------------------------------------+
//+ 计算一段音频的频率（whwu）
//+---------------------------------------------------------------------------+
CGetF0Result CAveFrequencyProc::GetF0(char* pAudioData, int nLength, EAuidoSampleRate eAuidoSampleRate)
{
	CGetF0Result oGetF0Result;
	if ( !CheckSampleRate(eAuidoSampleRate) )
	{
		oGetF0Result.nResult = VOICE_CHANGE_SAMPLERATE_ERROR;
		return oGetF0Result;
	}

	bool bNewBuffer = false;
	if ( AudioSampleRate_32k == eAuidoSampleRate || AudioSampleRate_48k == eAuidoSampleRate )
	{
		nLength /= 2;
		pAudioData = (char*)GetData((short*)pAudioData, nLength, eAuidoSampleRate, AudioSampleRate_16k);
		nLength *= 2;
		eAuidoSampleRate = AudioSampleRate_16k;
		bNewBuffer = true;
	}
	else if ( AudioSampleRate_44k == eAuidoSampleRate )
	{
		nLength /= 2;
		pAudioData = (char*)GetData((short*)pAudioData, nLength, eAuidoSampleRate, AudioSampleRate_22k);
		nLength *= 2;
		eAuidoSampleRate = AudioSampleRate_22k;
		bNewBuffer = true;
	}

	int nSub = (int)eAuidoSampleRate / 50;
	if ( nLength < nSub )
	{
		if ( bNewBuffer )
		{
			delete[] pAudioData;
		}
		oGetF0Result.nResult = VOICE_CHANGE_SUCCESS;
		return oGetF0Result;
	}

#if PITCH_USE_FIX
	short pPitch[PITCH_DATA_SIZE] = {0};
	unsigned short nPitchCnt = 0;

	if ( bFirst )
	{
		memset(pPitchHeap, 0, VPITCH_HEAP_SIZE);
		long nStatus = EOTPitcherInit(&tPitch, 0, 0, pPitchHeap, VPITCH_HEAP_SIZE, (unsigned int)eAuidoSampleRate);
		if ( 0 != nStatus ) 
		{
			if ( bNewBuffer )
			{
				delete[] pAudioData;
			}
			oGetF0Result.nResult = VOICE_CHANGE_INIT_ERROR;
			return oGetF0Result;
		}

		//+---------------------------------------------------------------------------+
		//+ 这里1024要和上面pPitch定义时设置的一样（whwu）
		//+---------------------------------------------------------------------------+
		EOTPitcherReset(&tPitch, pPitch, PITCH_DATA_SIZE);

		nLastOne = 0;
	}

	nLength -= nSub;
	for ( int i = 0; i <= nLength; i += nSub )
	{
		//+---------------------------------------------------------------------------+
		//+ 针对每一帧进行基频提取（whwu）
		//+---------------------------------------------------------------------------+
		EOTPitchExtract(&tPitch, (short*)(pAudioData + i));
	}

	//+---------------------------------------------------------------------------+
	//+ 每一帧基频提取之后，通过本函数一次性获取整个wav的基频值（whwu）
	//+---------------------------------------------------------------------------+
	EOTGetPitch(&tPitch, &nPitchCnt);

	for ( ; nLastOne < nPitchCnt; ++nLastOne )
	{
		int nRealPitchNum = nLastOne % PITCH_DATA_SIZE;
		if ( pPitch[nRealPitchNum] > 50 && pPitch[nRealPitchNum] < 400 )
		{
			oGetF0Result.nFrameNum ++;
			oGetF0Result.nF0 += pPitch[nRealPitchNum];
			//printf("%d：%d\n", nLastOne, pPitch[nRealPitchNum]);
		}
	}
#else
	int nPitchCnt = 0;
	m_oPitcher.online_process((short*)pAudioData, nLength / sizeof(short), m_pPitch, nPitchCnt);

	for ( int i = 0; i < nPitchCnt; ++i )
	{
		if ( m_pPitch[i] > 100 && m_pPitch[i] < 400 )
		{
			oGetF0Result.nFrameNum ++;
			oGetF0Result.nF0 += m_pPitch[i];
			//printf("%d：%f\n", i, pPitch[i]);
			/*FILE * fp = fopen("1.txt","a+");
			fprintf(fp,"%f",m_pPitch[i]);
			fputc('\n',fp);
			fclose(fp);*/

		}
	}

#endif

	oGetF0Result.nResult = VOICE_CHANGE_SUCCESS;
	if ( bNewBuffer )
	{
		delete[] pAudioData;
	}
	return oGetF0Result;
}

//+---------------------------------------------------------------------------+
//+ 获取一段音频的平均频率（whwu）
//+---------------------------------------------------------------------------+
CGetF0Result CAveFrequencyProc::GetAveFrequency(char* pAudioData, int nAudioLength, EAuidoSampleRate eAuidoSampleRate)
{
	int nResult = 0;
	int nAveF0 = 0;
	int nFrameNum = 0;
	CGetF0Result oGetF0AveResult;
	CGetF0Result oGetF0Result;

	int nSampleRate = (int)eAuidoSampleRate / 60 * 6;
	bool bFirst = true;
	for ( int i = 0; i <= nAudioLength; i += nSampleRate )
	{
		oGetF0Result = GetF0(pAudioData + i, nAudioLength - i > nSampleRate ? nSampleRate : (nAudioLength - i), eAuidoSampleRate);
		bFirst = false;
		if ( VOICE_CHANGE_SUCCESS != oGetF0Result.nResult ) 
		{
			oGetF0AveResult.nResult = oGetF0Result.nResult;
			return oGetF0AveResult;
		}

		oGetF0AveResult.nFrameNum += oGetF0Result.nFrameNum;
		oGetF0AveResult.nF0 += oGetF0Result.nF0;
	}

	return oGetF0AveResult;
}
