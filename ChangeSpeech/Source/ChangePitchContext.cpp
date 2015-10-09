#include "ChangePitchContext.h"
#include "Echo.h"
#include "PitchShift.h"
#include "GetPitch.h"

#define AVAILABLE_PITCH_COUNTS 30

ChangePitchContext::ChangePitchContext()
{
	_init = false;
	_pChangePitchStrategy = nullptr;
}

int ChangePitchContext::Init(EChangeType echangetype,EAuidoSampleRate eAuidoSampleRate)
{
	if(_init)
	{
		return CHANGEPITCHCONTEXT_INITED;
	}
	int nRet = 0;
	switch(echangetype)
	{
	case CHANGE_TYPE_XIAOHUANGREN:
		_pChangePitchStrategy = new StrategyToXiaoHuangren();
		break;
	case CHANGE_TYPE_BABY:
		_pChangePitchStrategy = new StrategyToBaby();
		break;
	case CHANGE_TYPE_TOMCAT:
		_pChangePitchStrategy = new StrategyToTomcat();
		break;
	case CHANGE_TYPE_SANTACLAUS:
		_pChangePitchStrategy = new StrategyToSantaclaus();
		break;
	case CHANGE_TYPE_TRANSFORMER:
		_pChangePitchStrategy = new StrategyToTransformer();
		break;
	case CHANGE_TYPE_ECHO:
		_pChangePitchStrategy = new StrategyToEcho();
		break;
	case CHANGE_TYPE_ROBOT:
		_pChangePitchStrategy = new StrategyToRobot();
		break;
	case CHANGE_TYPE_MAN:
		_pChangePitchStrategy = new StrategyToMan();
		break;
	case CHANGE_TYPE_WOMAN:
		_pChangePitchStrategy = new StrategyToWoman();
		break;
	case CHANGE_TYPE_OLDER:
		_pChangePitchStrategy = new StrategyToOlder();
		break;
	}
	nRet = _pChangePitchStrategy->Init(eAuidoSampleRate);
	_init = true;
	return nRet;
}

int ChangePitchContext::Uninit()
{
	if(!_init)
	{
		return CHANGEPITCHCONTEXT_UNINIT;
	}
	int nRet;
	nRet = _pChangePitchStrategy->Uninit();
	if(_pChangePitchStrategy)
	{
		delete _pChangePitchStrategy;
		_pChangePitchStrategy = nullptr;
	}
	_init = false;
	return nRet;
}

int ChangePitchContext::Process(float * pFInput,int nSamples)
{
	if(!_init)
	{
		return CHANGEPITCHCONTEXT_UNINIT;
	}

	return _pChangePitchStrategy->Process(pFInput,nSamples);
}

//StrategyToEcho
StrategyToEcho::StrategyToEcho()
{
	_pEffectEcho = nullptr;
}

int StrategyToEcho::Process(float * pFInput,int nSamples)
{
	return _pEffectEcho->Process(pFInput,nSamples);
}
int StrategyToEcho::Init(EAuidoSampleRate eAuidoSampleRate)
{
	_pEffectEcho = new EffectEcho();
	return _pEffectEcho->Init((float)eAuidoSampleRate,0.27,0.7);
}
int StrategyToEcho::Uninit()
{
	int nRet = _pEffectEcho->Uninit();
	if(_pEffectEcho)
	{
		delete _pEffectEcho;
		_pEffectEcho = nullptr;
	}
	return nRet;
}

//StrategyToRobot
StrategyToRobot::StrategyToRobot()
{
	_pEffectEcho = nullptr;
}

int StrategyToRobot::Process(float * pFInput,int nSamples)
{
	return _pEffectEcho->Process(pFInput,nSamples);
}
int StrategyToRobot::Init(EAuidoSampleRate eAuidoSampleRate)
{
	_pEffectEcho = new EffectEcho();
	return _pEffectEcho->Init((float)eAuidoSampleRate,0.01,0.8);
}
int StrategyToRobot::Uninit()
{
	int nRet = _pEffectEcho->Uninit();
	if(_pEffectEcho)
	{
		delete _pEffectEcho;
		_pEffectEcho = nullptr;
	}
	return nRet;
}

//ChangePitchStrategy
ChangePitchStrategy::ChangePitchStrategy()
{
	_nAveF0 = 0;
	_fPitch = 1;
	_fSize = 4;
	_fToPitch = 0;
	_nFrameNum = 0;
	_nF0 = 0;
	_bPitchAssigned = false;
	_pInst = nullptr;
	_pFrequeceProc = nullptr;
}

void ChangePitchStrategy::GetToPitch(float * pFInput,int nSamples)
{
	if(!_bPitchAssigned)
	{
		if(_nFrameNum < AVAILABLE_PITCH_COUNTS)
		{
			int nAudioLength = nSamples * 2;
			char * pAudioData = new char [nAudioLength];
			for(int i = 0 ;i < nSamples; i++)
			{
				((short *)pAudioData)[i] = pFInput[i] * 32768;
			}
			CGetF0Result f0Result = _pFrequeceProc->GetAveFrequency(pAudioData, nAudioLength, _eEAuidoSampleRate);
			delete [] pAudioData;
			if(f0Result.nFrameNum > 0)
			{
				_nF0 += f0Result.nF0;
				_nFrameNum += f0Result.nFrameNum;
			}
		}
		if(_nFrameNum >= AVAILABLE_PITCH_COUNTS)
		{
			_nAveF0 = _nF0 / _nFrameNum;
			if(_nAveF0 != 0)
			{
				_fPitch = _fToPitch / _nAveF0;
				if(_fPitch > 2)//2.3107
				{
					_fPitch = 2;
				}
				if(_fPitch < 0.6)//0.40849
				{
					_fPitch = 0.6;
				}
			}
			_bPitchAssigned = true;
		}
	}
}

int ChangePitchStrategy::Process(float * pFInput,int nSamples)
{
	GetToPitch(pFInput,nSamples);
	connectPortAmPitchshift(_pInst,AMPITCHSHIFT_PITCH,&_fPitch);
	connectPortAmPitchshift(_pInst,AMPITCHSHIFT_SIZE,&_fSize);
	connectPortAmPitchshift(_pInst,AMPITCHSHIFT_LATENCY,&_fLatency);
	connectPortAmPitchshift(_pInst,AMPITCHSHIFT_INPUT,pFInput);
	connectPortAmPitchshift(_pInst,AMPITCHSHIFT_OUTPUT,pFInput);
	runAmPitchshift(_pInst,nSamples);

	return CHANGEPITCHCONTEXT_SUCCESS;
}

int ChangePitchStrategy::Init(EAuidoSampleRate eAuidoSampleRate)
{
	_eEAuidoSampleRate = eAuidoSampleRate;
	_pInst = instantiateAmPitchshift((float)_eEAuidoSampleRate);
	_pFrequeceProc = new CAveFrequencyProc();
	int nRet = _pFrequeceProc->Init();
	if(!nRet)
	{
		return nRet;
	}
	SetToPitch();
	return CHANGEPITCHCONTEXT_SUCCESS;

}

int ChangePitchStrategy::Uninit()
{
	int nRet = _pFrequeceProc->UnInit();
	if(!nRet)
	{
		return nRet;
	}
	if(_pFrequeceProc)
	{
		delete _pFrequeceProc;
		_pFrequeceProc = nullptr;
	}
	cleanupAmPitchshift(_pInst);
	_nAveF0 = 0;
	_pInst = nullptr;
	_nFrameNum = 0;
	_nF0 = 0;
	return CHANGEPITCHCONTEXT_SUCCESS;
}

void StrategyToXiaoHuangren::SetToPitch()
{
	_bPitchAssigned = true;
	_fPitch = 0.7;
}

void StrategyToBaby::SetToPitch()
{
	_fToPitch = 500;
}

void StrategyToTomcat::SetToPitch()
{
	_bPitchAssigned = true;
	_fPitch = 1.5;
}

void StrategyToSantaclaus::GetToPitch(float * pFInput,int nSamples)
{
	if(!_bPitchAssigned)
	{
		if(_nFrameNum < AVAILABLE_PITCH_COUNTS)
		{
			int nAudioLength = nSamples * 2;
			char * pAudioData = new char [nAudioLength];
			for(int i = 0 ;i < nSamples; i++)
			{
				((short *)pAudioData)[i] = pFInput[i] * 32768;
			}
			CGetF0Result f0Result = _pFrequeceProc->GetAveFrequency(pAudioData, nAudioLength, _eEAuidoSampleRate);
			delete [] pAudioData;
			if(f0Result.nFrameNum > 0)
			{
				_nF0 += f0Result.nF0;
				_nFrameNum += f0Result.nFrameNum;
			}
		}
		if(_nFrameNum >= AVAILABLE_PITCH_COUNTS)
		{
			_nAveF0 = _nF0 / _nFrameNum;
			if(_nAveF0 != 0 && _nAveF0 > 95)
			{
				_fPitch = _fToPitch / _nAveF0;
				if(_fPitch < 0.65)//0.40849
				{
					_fPitch = 0.65;
				}
			}
			_bPitchAssigned = true;
		}
	}
}

void StrategyToSantaclaus::SetToPitch()
{
	_fToPitch = 95;
}

void StrategyToTransformer::GetToPitch(float * pFInput,int nSamples)
{
	if(!_bPitchAssigned)
	{
		if(_nFrameNum < AVAILABLE_PITCH_COUNTS)
		{
			int nAudioLength = nSamples * 2;
			char * pAudioData = new char [nAudioLength];
			for(int i = 0 ;i < nSamples; i++)
			{
				((short *)pAudioData)[i] = pFInput[i] * 32768;
			}
			CGetF0Result f0Result = _pFrequeceProc->GetAveFrequency(pAudioData, nAudioLength, _eEAuidoSampleRate);
			delete [] pAudioData;
			if(f0Result.nFrameNum > 0)
			{
				_nF0 += f0Result.nF0;
				_nFrameNum += f0Result.nFrameNum;
			}
		}
		if(_nFrameNum >= AVAILABLE_PITCH_COUNTS)
		{
			_nAveF0 = _nF0 / _nFrameNum;
			if(_nAveF0 != 0 && _nAveF0 > 80)
			{
				_fPitch = _fToPitch / _nAveF0;
				if(_fPitch < 0.6)//0.40849
				{
					_fPitch = 0.6;
				}
			}
			_bPitchAssigned = true;
		}
	}
}

void StrategyToTransformer::SetToPitch()
{
	_fToPitch = 80;
}

void StrategyToMan::SetToPitch()
{
	_fToPitch = 180;
}

void StrategyToMan::GetToPitch(float * pFInput,int nSamples)
{
	if(!_bPitchAssigned)
	{
		if(_nFrameNum < AVAILABLE_PITCH_COUNTS)
		{
			int nAudioLength = nSamples * 2;
			char * pAudioData = new char [nAudioLength];
			for(int i = 0 ;i < nSamples; i++)
			{
				((short *)pAudioData)[i] = pFInput[i] * 32768;
			}
			CGetF0Result f0Result = _pFrequeceProc->GetAveFrequency(pAudioData, nAudioLength, _eEAuidoSampleRate);
			delete [] pAudioData;
			if(f0Result.nFrameNum > 0)
			{
				_nF0 += f0Result.nF0;
				_nFrameNum += f0Result.nFrameNum;
			}
		}
		if(_nFrameNum >= AVAILABLE_PITCH_COUNTS)
		{
			_nAveF0 = _nF0 / _nFrameNum;
			if(_nAveF0 != 0 && _nAveF0 > 180)
			{
				_fPitch = _fToPitch / _nAveF0;
				if(_fPitch < 0.8)//0.40849
				{
					_fPitch = 0.8;
				}
			}
			_bPitchAssigned = true;
		}
	}
}

void StrategyToWoman::SetToPitch()
{
	_fToPitch = 230;
}

void StrategyToWoman::GetToPitch(float * pFInput,int nSamples)
{
	if(!_bPitchAssigned)
	{
		if(_nFrameNum < AVAILABLE_PITCH_COUNTS)
		{
			int nAudioLength = nSamples * 2;
			char * pAudioData = new char [nAudioLength];
			for(int i = 0 ;i < nSamples; i++)
			{
				((short *)pAudioData)[i] = pFInput[i] * 32768;
			}
			CGetF0Result f0Result = _pFrequeceProc->GetAveFrequency(pAudioData, nAudioLength, _eEAuidoSampleRate);
			delete [] pAudioData;
			if(f0Result.nFrameNum > 0)
			{
				_nF0 += f0Result.nF0;
				_nFrameNum += f0Result.nFrameNum;
			}
		}
		if(_nFrameNum >= AVAILABLE_PITCH_COUNTS)
		{
			_nAveF0 = _nF0 / _nFrameNum;
			if(_nAveF0 != 0 && _nAveF0 < 230)
			{
				_fPitch = _fToPitch / _nAveF0;
				if(_fPitch > 1.4)//2.3107
				{
					_fPitch = 1.4;
				}
			}
			_bPitchAssigned = true;
		}
	}
}

void StrategyToOlder::GetToPitch(float * pFInput,int nSamples)
{
	if(!_bPitchAssigned)
	{
		if(_nFrameNum < AVAILABLE_PITCH_COUNTS)
		{
			int nAudioLength = nSamples * 2;
			char * pAudioData = new char [nAudioLength];
			for(int i = 0 ;i < nSamples; i++)
			{
				((short *)pAudioData)[i] = pFInput[i] * 32768;
			}
			CGetF0Result f0Result = _pFrequeceProc->GetAveFrequency(pAudioData, nAudioLength, _eEAuidoSampleRate);
			delete [] pAudioData;
			if(f0Result.nFrameNum > 0)
			{
				_nF0 += f0Result.nF0;
				_nFrameNum += f0Result.nFrameNum;
			}
		}
		if(_nFrameNum >= AVAILABLE_PITCH_COUNTS)
		{
			_nAveF0 = _nF0 / _nFrameNum;
			if(_nAveF0 != 0 && _nAveF0 > 180)
			{
				_fPitch = _fToPitch / _nAveF0;
				if(_fPitch < 0.7)//0.40849
				{
					_fPitch = 0.7;
				}
			}
			_bPitchAssigned = true;
		}
	}
}

void StrategyToOlder::SetToPitch()
{
	_fToPitch = 180;
}