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
	//+ ͨ�����⣺�ɹ�
	//+---------------------------------------------------------------------------+
	VOICE_CHANGE_SUCCESS								= 0,	

	//+---------------------------------------------------------------------------+
	//+ ͨ�����⣺�����ʲ�֧��
	//+---------------------------------------------------------------------------+
	VOICE_CHANGE_SAMPLERATE_ERROR						= 2000,	

	//+---------------------------------------------------------------------------+
	//+ ��Ƶ����ȡ����ʼ��ʧ�ܣ��������ڴ治��
	//+---------------------------------------------------------------------------+
	VOICE_CHANGE_INIT_ERROR								,	

	//+---------------------------------------------------------------------------+
	//+ ��Ч��û�ж�Ӧ����Ч
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