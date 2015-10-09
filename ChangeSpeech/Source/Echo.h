#ifndef __EFFECT_ECHO__
#define __EFFECT_ECHO__

#include "ChangePitchErrorcode.h"

class EffectEcho
{

 public:

   EffectEcho();
   int Init(float samplerate,float delay,float decay);
   int Reset(float samplerate, float delay,float decay);
   int Uninit();
   int Process(float * pFInput,int nSamples);

 private: 
   float delayline(float x);
   float _delay;
   float _decay;
   float _samplerate;
   float * _buffer0;
   bool _init;
   int _blockSize;
   int _len;
};

#endif // __EFFECT_ECHO__
