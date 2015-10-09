#ifndef __CHANGE_PITCH__
#define __CHANGE_PITCH__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

enum EChangeType
{
	CHANGE_TYPE_XIAOHUANGREN,
	CHANGE_TYPE_BABY,
	CHANGE_TYPE_TOMCAT,
	CHANGE_TYPE_SANTACLAUS,
	CHANGE_TYPE_TRANSFORMER,
	CHANGE_TYPE_ECHO,
	CHANGE_TYPE_ROBOT,
	CHANGE_TYPE_MAN,
	CHANGE_TYPE_WOMAN,
	CHANGE_TYPE_OLDER,
};

enum EAuidoSampleRate
{
	AudioSampleRate_8k		= 8000,
	AudioSampleRate_16k		= 16000,
	AudioSampleRate_32k		= 32000,
	AudioSampleRate_48k		= 48000,
	AudioSampleRate_11k		= 11025,
	AudioSampleRate_22k		= 22050,
	AudioSampleRate_44k		= 44100,
};

DLLEXPORT int InitChangePitch(void ** ppInst,EChangeType eChangeType,EAuidoSampleRate eAuidoSampleRate);

DLLEXPORT int Process(void * pInst, float * pfInput,int nSampleCount);

DLLEXPORT int UnInitChangePitch(void * pInst);

#ifdef __cplusplus
}
#endif

#endif //__CHANGE_PITCH__