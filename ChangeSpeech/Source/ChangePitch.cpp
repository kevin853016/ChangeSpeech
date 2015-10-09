#include "ChangePitch.h"
#include "ChangePitchContext.h"

int InitChangePitch(void ** ppInst,EChangeType eChangeType,EAuidoSampleRate eAuidoSampleRate)
{
	int nRet = 0;
	ChangePitchContext * pChangePitchContext = new ChangePitchContext();
	nRet = pChangePitchContext->Init(eChangeType,eAuidoSampleRate);
	*ppInst = pChangePitchContext;
	return nRet;
}

int Process(void * pInst, float * pfInput,int nSampleCount)
{
	ChangePitchContext * pChangePitchContext = (ChangePitchContext *)pInst;
	if(!pChangePitchContext)
	{
		return CHANGEPITCH_NOEXIST_INST;
	}
	return pChangePitchContext->Process(pfInput,nSampleCount);
}

int UnInitChangePitch(void * pInst)
{
	ChangePitchContext * pChangePitchContext = (ChangePitchContext *)pInst;
	if(!pChangePitchContext)
	{
		return CHANGEPITCH_NOEXIST_INST;
	}
	int nRet = pChangePitchContext->Uninit();
	if(pChangePitchContext)
	{
		delete pChangePitchContext;
		pChangePitchContext = nullptr;
	}
	return nRet;
}