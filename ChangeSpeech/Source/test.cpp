#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "Echo.h"
#include "PitchShift.h"
#include "GetPitch.h"
#include "ChangePitch.h"

int main(int argc, char * argv[])
{
	assert(argc == 3);

	void * hPitchshift;
	InitChangePitch(&hPitchshift,CHANGE_TYPE_WOMAN,AudioSampleRate_22k);

	/*EffectEcho effectEcho;
	effectEcho.Init(22050,0.01,0.8);*/

	FILE * fpIn = fopen(argv[1],"rb+");
	if(!fpIn)
		return -1;
	fseek(fpIn, 0L, SEEK_END);  
	size_t buffer_size = ftell(fpIn);  
	fseek(fpIn, 0, SEEK_SET);  

	FILE * fpOut = fopen(argv[2],"wb+");
	if(!fpOut)
	{
		fclose(fpIn);  
		return -1;
	}

#define MAX_SAMPLES (size_t)10240
	char * buffer = new char[buffer_size];
	size_t number_read;

	number_read = fread(buffer,1,buffer_size,fpIn);
	if(number_read != buffer_size)
	{
		return -1;
	}

	/*CGetF0Result oGetF0Result;
	oGetF0Result = GetAveFrequency(buffer, buffer_size, (EAuidoSampleRate)22050);
	if ( 0 != oGetF0Result.nResult )
	{
	printf("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
	}
	int nAveF0 = oGetF0Result.nF0 / oGetF0Result.nFrameNum;*/

	float * fInBuffer[2];
	buffer_size /= 2;
	fInBuffer[0] = new float[buffer_size];
	for(int i = 0;i < buffer_size;i++)
	{
		float fValue = ((short *)buffer)[i] / 32768.0;
		fInBuffer[0][i] = fValue;

	}

	Process(hPitchshift, fInBuffer[0],buffer_size);
	//for(int i =0;i < MAX_SAMPLES; )
	//{
	//	fwrite(fInBuffer[0]+nCount + i,sizeof(float),1,fpOut);
	//	i += 2;//对于小黄人音效需要间隔取采样点，对于其他音效只需按顺序取即可
	//}
		

	for(int i = 0;i < buffer_size; i++)
	{
		float fTemp = fInBuffer[0][i];// + fInBuffer[1][i]);///sqrt(2);
		fwrite(&fTemp,sizeof(float),1,fpOut);
	}

	delete [] buffer;
	delete [] fInBuffer[0];

	UnInitChangePitch(hPitchshift);

	return 0;
}
