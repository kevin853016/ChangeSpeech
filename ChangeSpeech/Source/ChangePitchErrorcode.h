#ifndef __CHANGE_PITCH_ERRORCODE__
#define __CHANGE_PITCH_ERRORCODE__

enum EEffectEcho_Error
{
	EFFECT_ECHO_SUCCESS = 0,
	EFFECT_ECHO_UNINIT = 1000,
	EFFECT_ECHO_INITED,
	EFFECT_ECHO_SAMPLERATE,
	EFFECT_ECHO_DELAY,
	EFFECT_ECHO_DECAY,
};

enum
{
	//+---------------------------------------------------------------------------+
	//+ 通用问题：成功
	//+---------------------------------------------------------------------------+
	VOICE_CHANGE_SUCCESS								= 0,	

	//+---------------------------------------------------------------------------+
	//+ 通用问题：采样率不支持
	//+---------------------------------------------------------------------------+
	VOICE_CHANGE_SAMPLERATE_ERROR						= 2000,	

	//+---------------------------------------------------------------------------+
	//+ 基频率提取：初始化失败，可能是内存不足
	//+---------------------------------------------------------------------------+
	VOICE_CHANGE_INIT_ERROR								,	

	//+---------------------------------------------------------------------------+
	//+ 音效：没有对应的音效
	//+---------------------------------------------------------------------------+
	VOICE_CHANGE_NO_EFFECT								,	
};

enum ChangePitchContext_Error
{
	CHANGEPITCHCONTEXT_SUCCESS = 0,
	CHANGEPITCHCONTEXT_UNINIT = 3000,
	CHANGEPITCHCONTEXT_INITED,
};

enum ChangePitch_Error
{
	CHANGEPITCH_SUCCESS = 0,
	CHANGEPITCH_NOEXIST_INST = 4000,
};


#endif //__CHANGE_PITCH_ERRORCODE__