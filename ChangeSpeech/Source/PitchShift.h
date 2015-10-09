#ifndef __PITCH_SHIFT__
#define __PITCH_SHIFT__

//#ifdef __cplusplus
//extern "C" {
//#endif

#define AMPITCHSHIFT_PITCH             0
#define AMPITCHSHIFT_SIZE              1
#define AMPITCHSHIFT_INPUT             2
#define AMPITCHSHIFT_OUTPUT            3
#define AMPITCHSHIFT_LATENCY           4

void cleanupAmPitchshift(void * instance);

void connectPortAmPitchshift(
 void * instance,
 unsigned long port,
 float *data);

void * instantiateAmPitchshift(
 unsigned long s_rate);

void runAmPitchshift(void * instance, unsigned long sample_count);

//#ifdef __cplusplus
//}
//#endif

#endif //__PITCH_SHIFT__