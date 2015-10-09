#include <math.h>

#include "Echo.h"
#include <string.h>

#define MAX_SAMPLERATE 192000
#define MIN_SAMPLERATE 6000
#define MAX_DELAY 10
#define MIN_DELAY 0
#define MAX_DECAY 1
#define MIN_DECAY 0

EffectEcho::EffectEcho()
{
   _delay = float(1.0);
   _decay = float(0.5);
   _samplerate = 22050;
   _blockSize = 0;
   _len = 0;
   _init = false;
   _buffer0 = nullptr;
}

int EffectEcho::Init(float samplerate, float delay,float decay)
{
	if(_init)
	{
		return EFFECT_ECHO_INITED;
	}
	if(samplerate < MIN_SAMPLERATE ||samplerate > MAX_SAMPLERATE)
	{
		return EFFECT_ECHO_SAMPLERATE;
	}
	if(delay < MIN_DELAY || delay > MAX_DELAY)
	{
		return EFFECT_ECHO_DELAY;
	}
	if(decay < MIN_DECAY || decay >= MAX_DECAY)
	{
		return EFFECT_ECHO_DECAY;
	}
	_delay = delay;
	_decay = decay;
	_samplerate =  samplerate;
	_len = 0;
	_blockSize = (int) (_samplerate * _delay);
	_buffer0 = new float[_blockSize];
	memset(_buffer0,0,sizeof(float) * _blockSize);
	_init = true;
	return EFFECT_ECHO_SUCCESS;
}

int EffectEcho::Reset(float samplerate, float delay,float decay)
{
	if(!_init)
	{
		return EFFECT_ECHO_UNINIT;
	}
	if(samplerate < 6000 ||samplerate > 192000)
	{
		return EFFECT_ECHO_SAMPLERATE;
	}
	if(delay < 0 || delay > 10)
	{
		return EFFECT_ECHO_DELAY;
	}
	if(decay < 0 || decay >= 1)
	{
		return EFFECT_ECHO_DECAY;
	}
	_delay = delay;
	_decay = decay;
	_samplerate =  samplerate;
	_len = 0;
	_blockSize = (int) (_samplerate * _delay);
	if(_buffer0)
	{
		delete [] _buffer0;
	}
	_buffer0 = new float[_blockSize];
	memset(_buffer0,0,sizeof(float) * _blockSize);
	return EFFECT_ECHO_SUCCESS;
}

int EffectEcho::Uninit()
{
	if(!_init)
	{
		return EFFECT_ECHO_UNINIT;
	}
	if(_buffer0)
	{
		delete [] _buffer0;
	}
	_init = false;
	return EFFECT_ECHO_SUCCESS;
}

float EffectEcho::delayline(float x)
{
	_buffer0[_len] = x + _buffer0[_len] * _decay;         // write operation
	float  y = _buffer0[_len++];    // read operation 
	if (_len >= _blockSize) 
	{ 
		_len -= _blockSize; 
	} // wrap ptr 
	return y;
}

int EffectEcho::Process(float * pFInput,int nSamples)
{
	for(int i = 0;i < nSamples;i++)
	{
		pFInput[i] = delayline(pFInput[i]);
	}
	return EFFECT_ECHO_SUCCESS;
}