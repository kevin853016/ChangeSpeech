/** 
 * @file	pitch.cpp
 * @brief	
 * 
 * detail...
 * 
 * @author	zhyi
 * @version	1.0
 * @date	2007-05-31
 * 
 * @see		
 * 
 * @par History：
 * <table border=0>
 *  <tr> <th>Version	<th>Date		<th>Author	<th>Remark </tr>
 *  <tr> <td>1.0		<td>2007-05-31	<td>zhyi	<td>create </tr>
 * </table>
 */
#include "pitch.h"

#include "math.h"
#include "stdio.h"
#include "assert.h"
//using namespace PPitcher;

//#define USE_IFLY_LOG

//#ifdef USE_IFLY_LOG
//#include "..\..\include\iFly_Log.h"
//#else
//#define IFLY_LOGINFO(x)
//#endif


#define PI 3.14159265358979323846
#define NUMlog2e  1.4426950408889634074
#define NUMlog2(x)  (log (x) * NUMlog2e)

using namespace PPitcher;

//global_param global_param::inst_;

#define GLOBAL_PITCHER_PARAM (global_param::get_instance())

pitcher::pitcher(void)
{
	init();
}

pitcher::~pitcher(void)
{
	fini();
}

void pitcher::SetAudioType(EAudioType eAudioType)
{

}

int pitcher::reset()
{
	//printf("pitcher::reset\n");
//	IFLY_LOGINFO("pitcher::reset");
	test_mode_ = false;
	pathed_count_ = 0;
	processed_count_ = 0;
	buf_valid_head_ = 0;
	buf_valid_tail_ = 0;

	/*int count = pitch_frames_.size();
	for(int i = 0; i < count; i++)
	{
		delete pitch_frames_[i];
	}*/
	nCount = 0;
	pitch_frames_.clear();

	return 0;
}

int pitcher::init()
{
	nCount = 0;
	//printf("pitcher::init\n");
	//IFLY_LOGINFO("pitcher::init");
	test_mode_ = false;
	pathed_count_ = 0;
	processed_count_ = 0;

	buf_valid_head_ = 0;
	buf_valid_tail_ = 0;
	memset(buf_, 0, sizeof(buf_));

	const global_param& param = GLOBAL_PITCHER_PARAM;

	minimumPitch_ = param.minimumPitch_;
	periodsPerWindow_ = param.periodsPerWindow_;
	silenceThreshold_ = param.silenceThreshold_;
	voicingThreshold_ = param.voicingThreshold_;
	octaveCost_ = param.octaveCost_;
	octaveJumpCost_ = param.octaveJumpCost_;
	voicedUnvoicedCost_ = param.voicedUnvoicedCost_;
	ceiling_ = param.ceiling_;
	pullFormants_ = param.pullFormants_;
	brent_depth_ = param.brent_depth_;
	brent_accuracy_ = param.brent_accuracy_;
	interpolation_depth_ = param.interpolation_depth_;
	dx_ = param.dx_;
	dt_ = param.dt_;
	nsamp_period_ = param.nsamp_period_;
	halfnsamp_period_ = param.halfnsamp_period_;
	ceiling_ = param.ceiling_;
	dt_window_ = param.dt_window_;
	nsamp_window_ = param.nsamp_window_;
	halfnsamp_window_ = param.halfnsamp_window_;
	window_ = param.window_;
	windowR_ = param.windowR_;
	minimumLag_ = param.minimumLag_;
	maximumLag_ = param.maximumLag_;
	nsampFFT_ = param.nsampFFT_;
	brent_ixmax_ = param.brent_ixmax_;
	globalPeak_ = param.globalPeak_;

	Log440 = log(440.0);
	Log2 = log(2.0);

	return 0;
}

int pitcher::fini()
{
	//printf("pitcher::fini\n");
	//IFLY_LOGINFO("pitcher::fini");
	int count = pitch_frames_.size();

	/*for(int i = 0; i < count; i++)
	{
		delete pitch_frames_[i];
	}*/
	pitch_frames_.clear();

	window_ = NULL;
	windowR_ = NULL;

	return 0;
}

int pitcher::offline_process(signed short* data, int nx, int& pitch_count)
{
	//IFLY_LOGINFO("pitcher::offline_process");
	reset();

	int step = NSAMP_WINDOW;
	int temp = 0;
	int i = 0;
	for( ; i < nx - step; i += step)
	{
		append_data(data + i, step, temp);
	}
	step = nx - i;
	if(step > 0)
	{
		append_data(data + i, step, temp);
	}

	search_path(pitch_count);

	return 0;
}

//+---------------------------------------------------------------------------+
//+ 处理音频数据，选出多候选（whwu）
//+---------------------------------------------------------------------------+
int pitcher::append_data(signed short* data, int nx, int& processed_count)
{
	if( data == NULL || nx <= 0 )
	{
		return -1;
	}
	//+---------------------------------------------------------------------------+
	//+ 将数据传入到缓冲区中（whwu）
	//+---------------------------------------------------------------------------+
	buf_append_data(data, nx);

	//+---------------------------------------------------------------------------+
	//+ 获取一帧的数据（whwu）
	//+---------------------------------------------------------------------------+
	PRAAT_FLOAT* frame = cur_frame();
	while( frame )
	{
		//+---------------------------------------------------------------------------+
		//+ 处理一帧的数据，选出多候选（whwu）
		//+---------------------------------------------------------------------------+
		process_frame(frame, NSAMP_WINDOW);
		frame = cur_frame();
	}
	processed_count = processed_count_;
	return 0;
}

int pitcher::append_finished()
{
	//IFLY_LOGINFO("pitcher::append_finished");
	for(int i = 0; i < TAIL_ADD; i++)
	{
		pitch_frame* frame = new pitch_frame;
		frame->nCandidates = 1;
		frame->intensity = 0.0;
		frame->best = 0;
		frame->candidate[0].frequency = 0.0;
		frame->candidate[0].strength = 1.0;
		pitch_frames_.push_back(frame);
		processed_count_++;
	}
	return 0;
}

//int kkk_debug = 0;
//int kkk_debug_ok = 0;

//+---------------------------------------------------------------------------+
//+ 进行路径搜索，获取基频值（whwu）
//+---------------------------------------------------------------------------+
//+ count					- 处理到的位置
//+---------------------------------------------------------------------------+
int pitcher::search_path(int& count, bool final_search)
{
	//printf("pitcher::search_path ...[%d]\n", ++kkk_debug);
	int i,j;
	int icand, icand1, icand2;

	int place;
	PRAAT_FLOAT maximum, value;

	if( final_search )
	{
		//append_finished();
	}

	//+---------------------------------------------------------------------------+
	//+ 获取全部的帧数（whwu）
	//+---------------------------------------------------------------------------+
	int all_count = pitch_frames_.size();
	if( all_count <= 0 )
	{
		count = pathed_count_;
		return 0;
	}
	
	//+---------------------------------------------------------------------------+
	//+ 到5帧才处理（whwu）
	//+---------------------------------------------------------------------------+
	if ( all_count - pathed_count_ < 5 )
	{
		count = pathed_count_;
		return 0;
	}

	//+---------------------------------------------------------------------------+
	//+ 需要保留前面处理的20帧（whwu）
	//+---------------------------------------------------------------------------+
	int start_pos = max(0, pathed_count_ - 20);

	//+---------------------------------------------------------------------------+
	//+ 一共要处理的帧数（whwu）
	//+---------------------------------------------------------------------------+
	count = all_count - start_pos;

	//+---------------------------------------------------------------------------+
	//+ 一共要大于20帧才处理（whwu）
	//+---------------------------------------------------------------------------+
	if( !final_search && count < 20 )
	{
		count = pathed_count_;
		return 0;
	}

	//+---------------------------------------------------------------------------+
	//+ 每帧候选对应的最佳上一候选的分数（whwu）
	//+---------------------------------------------------------------------------+
	PRAAT_FLOAT (*delta)[MAX_CANDIDATES] = new PRAAT_FLOAT[count][MAX_CANDIDATES];

	//+---------------------------------------------------------------------------+
	//+ 每帧候选对应的最佳上一候选（whwu）
	//+---------------------------------------------------------------------------+
	int	  (*psi)[MAX_CANDIDATES]   = new int[count][MAX_CANDIDATES];

	//+---------------------------------------------------------------------------+
	//+ 循环处理要处理的帧（whwu）
	//+---------------------------------------------------------------------------+
	for( i = start_pos; i < all_count; i++ )
	{
		pitch_frame* frame = pitch_frames_[i];
		PRAAT_DOUBLE unvoicedStrength = silenceThreshold_ <= 0 ? 0 :
			2 - frame->intensity / (silenceThreshold_ / (1 + voicingThreshold_));
		unvoicedStrength = voicingThreshold_ + (unvoicedStrength > 0 ? unvoicedStrength : 0);
		for ( j = 0; j < frame->nCandidates; j++ ) 
		{
			pitch_candidate* candidate = & frame->candidate [j];
			int voiceless = candidate->frequency == 0 || candidate->frequency > ceiling_;
			delta [i - start_pos] [j] = voiceless ? unvoicedStrength :
				candidate->strength - octaveCost_ * NUMlog2 (ceiling_ / candidate->frequency);
		}
	}

	/* Look for the most probable path through the maxima. */
	/* There is a cost for the voiced/unvoiced transition, */
	/* and a cost for a frequency jump. */
	for ( i = start_pos + 1; i < all_count; i++ ) 
	{
		//+---------------------------------------------------------------------------+
		//+ 获取两帧的数据（whwu）
		//+---------------------------------------------------------------------------+
		pitch_frame* prevFrame = pitch_frames_ [i - 1], *curFrame = pitch_frames_ [i];
		PRAAT_FLOAT *prevDelta = delta [i - 1 - start_pos], *curDelta = delta [i - start_pos];
		int *curPsi = psi [i - start_pos];

		//+---------------------------------------------------------------------------+
		//+ 前一帧候选的循环（whwu）
		//+---------------------------------------------------------------------------+
		for ( icand2 = 0; icand2 < curFrame -> nCandidates; icand2++ ) 
		{
			PRAAT_DOUBLE f2 = curFrame -> candidate [icand2]. frequency;
			maximum = -10;
			place = 0;

			//+---------------------------------------------------------------------------+
			//+ 后一帧候选的循环（whwu）
			//+---------------------------------------------------------------------------+
			for ( icand1 = 0; icand1 < prevFrame -> nCandidates; icand1++ ) 
			{
				PRAAT_DOUBLE f1 = prevFrame -> candidate [icand1]. frequency, transitionCost;
				int previousVoiceless = f1 <= 0 || f1 >= ceiling_;
				int currentVoiceless = f2 <= 0 || f2 >= ceiling_;

				//+---------------------------------------------------------------------------+
				//+ 计算代价值（whwu）
				//+---------------------------------------------------------------------------+
				if ( previousVoiceless != currentVoiceless )   /* Voice transition. */
					transitionCost = voicedUnvoicedCost_;
				else if ( currentVoiceless )   /* Both voiceless. */
					transitionCost = 0;
				else   /* Both voiced; frequency jump. */
					transitionCost = octaveJumpCost_ * fabs (NUMlog2 (f1 / f2));
				value = prevDelta [icand1] - transitionCost + curDelta [icand2];
				if ( value > maximum ) 
				{
					maximum = value;
					place = icand1;
				}
			}

			//+---------------------------------------------------------------------------+
			//+ 候选对应的最佳上一候选的分数（whwu）
			//+---------------------------------------------------------------------------+
			curDelta [icand2] = maximum;

			//+---------------------------------------------------------------------------+
			//+ 候选对应的最佳上一候选（whwu）
			//+---------------------------------------------------------------------------+
			curPsi [icand2] = place;
		}
	}

	//+---------------------------------------------------------------------------+
	//+ 回溯候选，选出最后一帧的最佳候选（whwu）
	//+---------------------------------------------------------------------------+
	maximum = delta [all_count - 1 - start_pos] [place = 0];
	for ( icand = 0; icand < pitch_frames_ [all_count - 1]-> nCandidates; icand++ )
	{
		if ( delta [all_count - 1 - start_pos] [icand] > maximum )
		{
			maximum = delta [all_count - 1 - start_pos] [place = icand];
		}
	}

	//+---------------------------------------------------------------------------+
	//+ 回溯候选，通过最后一帧的候选找到前面的最佳候选（whwu）
	//+---------------------------------------------------------------------------+
	for ( i = all_count - 1; i >= start_pos; i-- )
	{
		pitch_frame* frame = pitch_frames_ [i];
		frame->best = place;
		place = psi[i - start_pos][place];
	}

	pathed_count_ = processed_count_;
	count = pathed_count_;

	delete[] delta;
	delete[] psi;

	return 0;
}

int pitcher::get_pitch(double* pitch, int beg, int count)
{
	int valid_count = pitch_frames_.size() - beg;
	if(valid_count < 1)
	{
		return -1;
	}
	if(count > valid_count)
	{
		return -1;
	}
	for(int i = beg; i < beg + count; i++)
	{
		pitch_frame* frame = pitch_frames_ [i];
		pitch[i-beg] = (double)(frame -> candidate [frame->best].frequency);
		//printf("%f\n", pitch[i]);
	}
	return 0;
}

int pitcher::get_pitch(float* pitch, int beg, int count)
{
	int valid_count = pitch_frames_.size() - beg;
	if(valid_count < 1)
	{ 
		return -1;
	}
	if(count > valid_count)
	{
		return -1;
	}
	for(int i = beg; i < beg + count; i++)
	{
		pitch_frame* frame = pitch_frames_ [i];
		pitch[i-beg] = (float)(frame -> candidate [frame->best].frequency);
		//printf("%f\n", pitch[i]);
	}
	return 0;
}

//+---------------------------------------------------------------------------+
//+ 处理一帧的数据，选出多候选（whwu）
//+---------------------------------------------------------------------------+
int pitcher::process_frame(PRAAT_FLOAT* audio, int nx)
{
	processed_count_++;

	//+---------------------------------------------------------------------------+
	//+ 语音数据（whwu）
	//+---------------------------------------------------------------------------+
	PRAAT_FLOAT *amplitude;  
	long i, j;
	amplitude = audio;

	//+---------------------------------------------------------------------------+
	//+ 做fft的数据区（whwu）
	//+---------------------------------------------------------------------------+
	PRAAT_DOUBLE* frame = new PRAAT_DOUBLE[nsampFFT_];
	PRAAT_DOUBLE* rr = new PRAAT_DOUBLE[2*nsamp_window_+1];
	PRAAT_DOUBLE* r = rr + nsamp_window_;

	//+---------------------------------------------------------------------------+
	//+ 获取一帧基频的存储区（whwu）
	//+---------------------------------------------------------------------------+
	pitch_frame* pitchFrame = GetOnePitchFrame();
	PRAAT_DOUBLE localMean, localPeak;
	long leftSample = halfnsamp_window_, rightSample = leftSample + 1;
	long startSample, endSample;

	localMean = 0.0;

	//+---------------------------------------------------------------------------+
	//+ 获取两个边界点（whwu）
	//+---------------------------------------------------------------------------+
	startSample = rightSample - nsamp_period_;
	endSample = leftSample + nsamp_period_;

	//+---------------------------------------------------------------------------+
	//+ 整个有效基音周期内的平均数据（whwu）
	//+---------------------------------------------------------------------------+
	for ( i = startSample - 1; i < endSample; i ++ ) 
	{
		localMean += amplitude [i];
	}
	localMean /= 2 * nsamp_period_;

	startSample = rightSample - halfnsamp_window_;
	endSample = leftSample + halfnsamp_window_;

	//+---------------------------------------------------------------------------+
	//+ 去直流分量后加窗（whwu）
	//+---------------------------------------------------------------------------+
	for ( j = 0, i = startSample - 1; j < nsamp_window_; j ++ )
	{
		frame [j] = (amplitude [i ++] - localMean) * window_ [j];
	}

	//+---------------------------------------------------------------------------+
	//+ 补全fft数据，帧长取得不对吧，浪费了很多空间（whwu优化）
	//+---------------------------------------------------------------------------+
	for (j = nsamp_window_; j < nsampFFT_; j ++)
	{
		frame [j] = 0.0;
	}

	//+---------------------------------------------------------------------------+
	//+ 获取归0后的最大值（whwu）
	//+---------------------------------------------------------------------------+
	localPeak = 0;
	if ( (startSample = halfnsamp_window_ + 1 - halfnsamp_period_) < 0 ) startSample = 1;
	if ((endSample = halfnsamp_window_ + halfnsamp_period_) > nsamp_window_) endSample = nsamp_window_;
	for ( j = startSample - 1; j < endSample; j ++ )
	{
		if ( fabs (frame [j]) > localPeak ) 
			localPeak = fabs (frame [j]);
	}
	pitchFrame->intensity = localPeak > globalPeak_ ? 1 : localPeak / globalPeak_;

	pitchFrame->nCandidates = 1;
	pitchFrame->candidate[0].frequency = 0.0;  
	pitchFrame->candidate[0].strength = 0.0;

	//+---------------------------------------------------------------------------+
	//+ 全是静音（whwu）
	//+---------------------------------------------------------------------------+
	if (localPeak == 0) 
	{
		pitch_frames_.push_back(pitchFrame);

		delete[] frame;
		delete[] rr;

		return 0;
	}

	//+---------------------------------------------------------------------------+
	//+ 进行傅里叶变换（whwu）
	//+---------------------------------------------------------------------------+
	real_fft(frame, nsampFFT_, 1);   /* Complex spectrum. */

	//+---------------------------------------------------------------------------+
	//+ 求功率谱密度（whwu）
	//+---------------------------------------------------------------------------+
	frame [0] *= frame [0];  
	frame [1] *= frame [1]; 
	for (i = 2; i < nsampFFT_; i += 2) 
	{
		frame [i] = frame [i] * frame [i] + frame [i+1] * frame [i+1];
		frame [i + 1] = 0.0;  
	}

	//+---------------------------------------------------------------------------+
	//+ 进行傅里叶逆变换（whwu）
	//+---------------------------------------------------------------------------+
	real_fft (frame, nsampFFT_, -1);

	//+---------------------------------------------------------------------------+
	//+ 求全部的自相关值与最大值的比值（whwu）
	//+---------------------------------------------------------------------------+
	r [0] = 1.0;
	for ( i = 1; i <= brent_ixmax_; i ++ )
	{
		r [- (i)] = r [i] = frame [i + 1 - 1] / (frame [0] * windowR_ [i + 1 - 1]);
	}

	long imax[MAX_CANDIDATES];
	imax [0] = 0;
	for ( i = 1; i < maximumLag_ && i < brent_ixmax_; i ++ )
	{
		//+---------------------------------------------------------------------------+
		//+ 如果为局部最大值（whwu）
		//+---------------------------------------------------------------------------+
		if ( r [i] > 0.5 * voicingThreshold_ && r [i] > r [i-1] && r [i] >= r [i+1] )  
		{
			int place = -1;

			//+---------------------------------------------------------------------------+
			//+ 消除基频便宜误差（whwu）
			//+---------------------------------------------------------------------------+
			PRAAT_DOUBLE dr = 0.5 * (r [i+1] - r [i-1]), d2r = 2 * r [i] - r [i-1] - r [i+1];
			PRAAT_DOUBLE frequencyOfMaximum = 1 / dx_ / ( (i) + dr / d2r);
			long offset = - brent_ixmax_ - 1;
			PRAAT_DOUBLE strengthOfMaximum = 
				NUM_interpolate_sinc_d (r + offset + 1, brent_ixmax_ - offset, 1 / dx_ / frequencyOfMaximum - offset,
				30);
			if (strengthOfMaximum > 1.0) 
			{
				strengthOfMaximum = 1.0 / strengthOfMaximum;
			}

			if ( pitchFrame->nCandidates < MAX_CANDIDATES ) 
			{
				//+---------------------------------------------------------------------------+
				//+ 如果没有超过候选个数则直接加入（whwu）
				//+---------------------------------------------------------------------------+
				place = pitchFrame->nCandidates;
				pitchFrame->nCandidates++;
			} 
			else 
			{
				//+---------------------------------------------------------------------------+
				//+ 如果超过了候选最大个数（whwu）
				//+---------------------------------------------------------------------------+
				PRAAT_DOUBLE weakest = 2;
				int iweak;

				//+---------------------------------------------------------------------------+
				//+ 循环找到最小分数（whwu）
				//+---------------------------------------------------------------------------+
				for ( iweak = 1; iweak < MAX_CANDIDATES; iweak ++ ) 
				{
					PRAAT_DOUBLE localStrength = pitchFrame->candidate[iweak].strength - octaveCost_ *
						NUMlog2 (minimumPitch_ / pitchFrame->candidate[iweak].frequency);
					if ( localStrength < weakest ) 
					{ 
						weakest = localStrength; 
						place = iweak; 
					}
				}

				//+---------------------------------------------------------------------------+
				//+ 如果此候选的分数比最小的还小则不进行处理了（whwu）
				//+---------------------------------------------------------------------------+
				if ( strengthOfMaximum - octaveCost_ * NUMlog2 (minimumPitch_ / frequencyOfMaximum) <= weakest )
					place = -1;
			}

			//+---------------------------------------------------------------------------+
			//+ 找到了一个候选（whwu）
			//+---------------------------------------------------------------------------+
			if ( place >= 0 ) 
			{   
				pitchFrame->candidate[place].frequency = frequencyOfMaximum;
				pitchFrame->candidate[place].strength = strengthOfMaximum;
				imax [place] = i;
			}
		}
	}

	//+---------------------------------------------------------------------------+
	//+ 将基频候选保存到队列中（whwu）
	//+---------------------------------------------------------------------------+
	pitch_frames_.push_back(pitchFrame);

	delete[] frame;
	delete[] rr;

	return 0;
}


global_param::global_param()
{
	minimumPitch_ = 100.0;
	periodsPerWindow_ = 3.0;
	silenceThreshold_ = 0.03;
	voicingThreshold_ = 0.45;//0.45;
	octaveCost_ = 0.01;
	octaveJumpCost_ = 0.75;//0.35;
	voicedUnvoicedCost_ = 0.14;
	ceiling_ = 600;
	pullFormants_ = true;
	brent_depth_ = 3;
	brent_accuracy_ = 1e-7;
	interpolation_depth_ = 0.5;

	//+---------------------------------------------------------------------------+
	//+ 每个采样点的时间(秒)（whwu采样）
	//+---------------------------------------------------------------------------+
	dx_ = 1.0 / 22000/*16000*/; 
	globalPeak_ = 0.9;//

	dt_ = periodsPerWindow_ / minimumPitch_ / 4.0; 
	
	/*
	* Determine the number of samples in the longest period.
	* We need this to compute the local mean of the sound (looking one period in both directions),
	* and to compute the local peak of the sound (looking half a period in both directions).
	*/

	//+---------------------------------------------------------------------------+
	//+ 可能的最大基音周期的采样点数（whwu）
	//+---------------------------------------------------------------------------+
	nsamp_period_ = (long)floor (1 / dx_ / minimumPitch_); 
	halfnsamp_period_ = nsamp_period_ / 2 + 1;

	if (ceiling_ > 0.5 / dx_) ceiling_ = 0.5 / dx_;

	/*
	* Determine window_ length in seconds and in samples.
	*/
	dt_window_ = periodsPerWindow_ / minimumPitch_; //确定窗长(秒)
	nsamp_window_ = NSAMP_WINDOW; //floor (dt_window_ / dx_);       //确定窗长(采样点数)
	halfnsamp_window_ = nsamp_window_ / 2/* - 1*/;     //why?
	nsamp_window_ = halfnsamp_window_ * 2;

	window_ = NULL;
	window_ = new PRAAT_DOUBLE[nsamp_window_];
	//assert(window_);

	/*
	* Determine the minimum and maximum lags.
	*/
	minimumLag_ = (long)floor (1 / dx_ / ceiling_); //可能的最小周期(采样点数)
	if (minimumLag_ < 2) minimumLag_ = 2;
	maximumLag_ = (long)floor (nsamp_window_ / periodsPerWindow_) + 2;
	if (maximumLag_ > nsamp_window_) maximumLag_ = nsamp_window_;

	/*
	* Compute the number of samples needed for doing FFT.
	* To avoid edge effects, we have to append zeroes to the window_.
	* The maximum lag considered for maxima is maximumLag_.
	* The maximum lag used in interpolation is nsamp_window_ * interpolation_depth_.
	*/

	//+---------------------------------------------------------------------------+
	//+ 做FFT的数据大小（whwu）
	//+---------------------------------------------------------------------------+
	nsampFFT_ = 1; while (nsampFFT_ < nsamp_window_ * (1 + 0.5)) nsampFFT_ *= 2;

	int i = 0;

	/*
	* A Gaussian or Hanning window_ is applied against phase effects.
	* The Hanning window_ is 2 to 5 dB better for 3 periods/window_.
	* The Gaussian window_ is 25 to 29 dB better for 6 periods/window_.
	*/
	for (i = 0; i < nsamp_window_; i ++)
		window_ [i] = 0.5 - 0.5 * cos ((i+1) * 2 * PI / (nsamp_window_ + 1));

	windowR_ = NULL;
	windowR_ = new PRAAT_DOUBLE[nsampFFT_];
	//assert(windowR_);

	/*
	* Compute the normalized autocorrelation of the window_.
	*/
	for (i = 0; i < nsamp_window_; i ++) windowR_ [i] = window_ [i];
	for( ; i < nsampFFT_; i++) windowR_[i] = 0.0;
	real_fft (windowR_, nsampFFT_, 1);   /* Complex spectrum. */
	windowR_ [0] *= windowR_ [0];   /* DC component. */
	windowR_ [1] *= windowR_ [1];   /* Nyquist frequency. */
	for (i = 2; i < nsampFFT_; i += 2) {
		windowR_ [i] = windowR_ [i] * windowR_ [i] + windowR_ [i+1] * windowR_ [i+1];
		windowR_ [i + 1] = 0.0;   /* Power spectrum: square and zero. */
	}
	real_fft (windowR_, nsampFFT_, -1);   /* Autocorrelation. */
	for (i = 1; i < nsamp_window_; i ++) windowR_ [i] /= windowR_ [0];   /* Normalize. */
	windowR_ [0] = 1.0;   /* Normalize. */

	brent_ixmax_ = (long)(nsamp_window_ * interpolation_depth_);
}

global_param::~global_param()
{
	if (window_)
	{
		delete[] window_;
		window_ = NULL;
	}
	
	if (windowR_)
	{
		delete[] windowR_;
		windowR_ = NULL;
	}
}

void PPitcher::fft (PRAAT_DOUBLE *data, long nn, int isign) 
{ 
	long n = nn << 1, mmax = 2, m, j = 0, i; 
	for ( i = 0; i < n - 1; i += 2 ) 
	{ 
		if ( j > i )
		{ 
			PRAAT_DOUBLE dum; 
			dum = data [j], data [j] = data [i], data [i] = dum; 
			dum = data [j+1], data [j+1] = data [i+1], data [i+1] = dum; 
		} 
		m = n >> 1; 
		while (m >= 1 && j + 1 > m) { j -= m; m >>= 1; } 
		j += m; 
	} 

	//+---------------------------------------------------------------------------+
	//+ 总介数（whwu）
	//+---------------------------------------------------------------------------+
	while ( n > mmax ) 
	{ 
		//+---------------------------------------------------------------------------+
		//+ 当前介所有数据被分成多少份进行fft（whwu）
		//+---------------------------------------------------------------------------+
		long istep = 2 * mmax; 
		PRAAT_DOUBLE theta = 2 * PI / (isign * mmax); 
		PRAAT_DOUBLE wr, wi, wtemp, wpr, wpi; 
		wtemp = sin (0.5 * theta); 
		wpr = -2.0 * wtemp * wtemp; 
		wpi = sin (theta); 
		wr = 1.0, wi = 0.0; 

		//+---------------------------------------------------------------------------+
		//+ 内部循环（whwu）
		//+---------------------------------------------------------------------------+
		for ( m = 0; m < mmax - 1; m += 2 ) 
		{ 
			//+---------------------------------------------------------------------------+
			//+ 同时做fft的个数（whwu）
			//+---------------------------------------------------------------------------+
			for ( i = m; i < n; i += istep ) 
			{ 
				PRAAT_DOUBLE tempr, tempi; 
				j = i + mmax; 
				tempr = wr * data [j] - wi * data [j+1], tempi = wr * data [j+1] + wi * data [j]; 
				data [j] = data [i] - tempr, data [j+1] = data [i+1] - tempi; 
				data [i] += tempr, data [i+1] += tempi; 
			} 
			wtemp = wr, wr = wr * wpr - wi * wpi + wr, wi = wi * wpr + wtemp * wpi + wi; 
		} 
		mmax = istep; 
	} 
}

void PPitcher::real_fft (PRAAT_DOUBLE *data, long n, int isign)
{
	long i, i1, i2, i3, i4, np3; 
	PRAAT_DOUBLE c1 = 0.5, c2, h1r, h1i, h2r, h2i; 
	PRAAT_DOUBLE wr, wi, wpr, wpi, wtemp, theta; 
	theta = PI / (PRAAT_DOUBLE) (n >> 1); 
	if (isign == 1) 
	{ 
		c2 = -0.5; 
		fft(data, n >> 1, 1); 
	} 
	else 
	{ 
		c2 = 0.5; 
		theta = -theta; 
	} 
	wtemp = sin (0.5 * theta); 
	wpr = -2.0 * wtemp * wtemp; 
	wpi = sin (theta); 
	wr = 1.0 + wpr; 
	wi = wpi; 
	np3 = n + 1; 
	for ( i = 1; i < n >> 2; i++ ) 
	{ 
		i4 = 1 + (i3 = np3 - (i2 = 1 + (i1 = i + i ))); 
		h1r = c1 * (data [i1] + data [i3]); 
		h1i = c1 * (data [i2] - data [i4]); 
		h2r = - c2 * (data [i2] + data [i4]); 
		h2i = c2 * (data [i1] - data [i3]); 
		data [i1] = h1r + wr * h2r - wi * h2i; 
		data [i2] = h1i + wr * h2i + wi * h2r; 
		data [i3] = h1r - wr * h2r + wi * h2i; 
		data [i4] = - h1i + wr * h2i + wi * h2r; 
		wr = (wtemp = wr) * wpr - wi * wpi + wr; 
		wi = wi * wpr + wtemp * wpi + wi; 
	} 
	if ( isign == 1 ) 
	{ 
		data [0] = (h1r = data [0]) + data [1]; 
		data [1] = h1r - data [1]; 
	} 
	else 
	{ 
		data [0] = c1 * ((h1r = data [0]) + data [1]); 
		data [1] = c1 * (h1r - data [1]); 
		fft(data, n >> 1, -1); 
	} 
}

PRAAT_DOUBLE PPitcher::NUM_interpolate_sinc_d (PRAAT_DOUBLE y [], long nx, PRAAT_DOUBLE x, long maxDepth) 
{ 
	long ix, midleft = (long)floor (x), midright = midleft + 1, left, right; 
	PRAAT_DOUBLE result = 0.0, a, halfsina, aa, daa; 
	//NUM_interpolate_simple_cases {
	if (nx < 1) return -1; 
	if (x > nx) return y [nx]; 
	if (x < 1) return y [1]; 
	if (x == midleft) return y [midleft]; 
	/* 1 < x < nx && x not integer: interpolate. */ 
	if (maxDepth > midright - 1) maxDepth = midright - 1; 
	if (maxDepth > nx - midleft) maxDepth = nx - midleft; 
	if (maxDepth <= 0) return y [(long) floor (x + 0.5)]; 
	if (maxDepth == 1) return y [midleft] + (x - midleft) * (y [midright] - y [midleft]); 
	if (maxDepth == 2) { 
		PRAAT_DOUBLE yl = y [midleft], yr = y [midright]; 
		PRAAT_DOUBLE dyl = 0.5 * (yr - y [midleft - 1]), dyr = 0.5 * (y [midright + 1] - yl); 
		PRAAT_DOUBLE fil = x - midleft, fir = midright - x; 
		return yl * fir + yr * fil - fil * fir * (0.5 * (dyr - dyl) + (fil - 0.5) * (dyl + dyr - 2 * (yr - yl))); 
	}
	//NUM_interpolate_simple_cases }

	left = midright - maxDepth, right = midleft + maxDepth; 
	a = PI * (x - midleft); 
	halfsina = 0.5 * sin (a); 
	aa = a / (x - left + 1); 
	daa = PI / (x - left + 1); 
	for (ix = midleft - 1; ix >= left - 1; ix --) { 
		PRAAT_DOUBLE d = halfsina / a * (1.0 + cos (aa)); 
		result += y [ix] * d; 
		a += PI;	 
		aa += daa;	 
		halfsina = - halfsina; 
	} 
	a = PI * (midright - x); 
	halfsina = 0.5 * sin (a); 
	aa = a / (right - x + 1); 
	daa = PI / (right - x + 1); 
	for (ix = midright - 1; ix < right; ix ++) { 
		PRAAT_DOUBLE d = halfsina / a * (1.0 + cos (aa)); 
		result += y [ix] * d; 
		a += PI;	 
		aa += daa; 
		halfsina = - halfsina; 
	} 
	return result; 
}

PRAAT_DOUBLE PPitcher::NUMimproveMaximum_d(PRAAT_DOUBLE *corr_data, int data_num, PRAAT_FLOAT offset_time, int max_depth_, PRAAT_DOUBLE& xmid)
{
	long i, dwMidleft, dwMidright, dwLeft, dwRight; 
	PRAAT_FLOAT dResult, dHalfsina, dTemp1, dTemp2, dTemp3; 

	dwMidleft = (long)floor (offset_time);
	dwMidright = dwMidleft + 1;
	dResult = 0.0;

	if (data_num < 1) 
	{
		return 0; 
	}		
	if (offset_time < 1)
	{
		return corr_data[1]; 
	}
	if (offset_time == dwMidleft) 
	{
		return corr_data[dwMidleft]; 
	}
	if (max_depth_ > dwMidright - 1) 
	{
		max_depth_ = dwMidright - 1; 
	}
	if (max_depth_ > data_num - dwMidleft) 
	{
		max_depth_ = data_num - dwMidleft; 
	}
	if (max_depth_ <= 0) 
	{
		return corr_data [(long) floor (offset_time + 0.5)]; 
	}
	if (max_depth_ == 1) 
	{
		return corr_data[dwMidleft] + (offset_time - dwMidleft) * (corr_data[dwMidright] - corr_data[dwMidleft]); 
	}
	if (max_depth_ == 2) 
	{ 
		PRAAT_FLOAT yl = corr_data[dwMidleft], yr = corr_data[dwMidright]; 
		PRAAT_FLOAT dyl = 0.5f * (yr - corr_data[dwMidleft - 1]), dyr = 0.5f * (corr_data[dwMidright + 1] - yl); 
		PRAAT_FLOAT fil = offset_time - dwMidleft, fir = dwMidright - offset_time; 
		return yl * fir + yr * fil - fil * fir * (0.5f * (dyr - dyl) + (fil - 0.5f) * (dyl + dyr - 2.0f * (yr - yl))); 
	}

	dwLeft = dwMidright - max_depth_, dwRight = dwMidleft + max_depth_; 
	dTemp1 = PI * (offset_time - dwMidleft); 
	dHalfsina = 0.5f * sin (dTemp1); 
	dTemp2 = dTemp1 / (offset_time - dwLeft + 1); 
	dTemp3 = PI / (offset_time - dwLeft + 1); 
	for (i = dwMidleft; i >= dwLeft; i --) 
	{ 
		PRAAT_FLOAT dDTemp = dHalfsina / dTemp1 * (1.0f + cos (dTemp2)); 
		dResult += corr_data [i] * dDTemp; 
		dTemp1 += PI;	 
		dTemp2 += dTemp3;	 
		dHalfsina = - dHalfsina; 
	} 
	dTemp1 = PI * (dwMidright - offset_time); 
	dHalfsina = 0.5f * sin (dTemp1); 
	dTemp2 = dTemp1 / (dwRight - offset_time + 1); 
	dTemp3 = PI / (dwRight - offset_time + 1); 
	for (i = dwMidright; i <= dwRight; i ++) 
	{ 
		PRAAT_FLOAT dDTemp = dHalfsina / dTemp1 * (1.0f + cos (dTemp2)); 
		dResult += corr_data [i] * dDTemp; 
		dTemp1 += PI;	 
		dTemp2 += dTemp3; 
		dHalfsina = - dHalfsina; 
	} 
	return dResult; 
}

//+---------------------------------------------------------------------------+
//+ 获取在线基频提取（whwu）
//+---------------------------------------------------------------------------+
int pitcher::online_process(short *data, int size, float *pitch, int &frame)
{
	int nPreFrame = processed_count_;
	int nPresize = pitch_frames_.size();
	int nCount = 0;

	//+---------------------------------------------------------------------------+
	//+ 处理音频数据，选出多候选（whwu）
	//+---------------------------------------------------------------------------+
	append_data(data,size,nCount);

	//+---------------------------------------------------------------------------+
	//+ 一共处理的多少帧（whwu）
	//+---------------------------------------------------------------------------+
	int nframe = processed_count_ - nPreFrame;
	int nsize = pitch_frames_.size() - nPresize;
	int nPathframe = pathed_count_;
	
	//+---------------------------------------------------------------------------+
	//+ 至少3帧才做一次搜索（whwu）
	//+---------------------------------------------------------------------------+
	if ( processed_count_ - nPathframe < 3 )
	{
		nframe = 0;
		return true;
	}

	//+---------------------------------------------------------------------------+
	//+ 进行路径搜索，获取基频值（whwu）
	//+---------------------------------------------------------------------------+
	search_path(frame, false);

	//+---------------------------------------------------------------------------+
	//+ 本次搜索过的帧数（whwu）
	//+---------------------------------------------------------------------------+
	nframe = pathed_count_ - nPathframe;

	//+---------------------------------------------------------------------------+
	//+ 频率转音分（whwu）
	//+---------------------------------------------------------------------------+
	for ( int j = 0; j < nframe; j++ )
	{
		pitch_frame* frame = pitch_frames_ [j + nPathframe];
		pitch[j] = (double)(frame -> candidate [frame->best].frequency);
		/*if ( pitch[j] > 0.000001 )
		{
			pitch[j] = 12 * (log(pitch[j]) - Log440)/Log2 + 69;
		}*/
	}
	
	frame = nframe;
	return 1;
}

int pitcher::online_reset()
{
	//init();
	pathed_count_ = 0;
	processed_count_ = 0;
	buf_valid_head_ = 0;
	buf_valid_tail_ = 0;
	nCount = 0;

	memset(pPitch, 0, ONLINE_POOL * sizeof(pitch_frame));
	
	/*int count = pitch_frames_.size();
	for(int i = 0; i < count; i++)
	{
		delete pitch_frames_[i];
	}*/
	pitch_frames_.clear();
	return 1;
}

//int pitcher::offline_getAllCandicate(const int nBeginFrame,const int nFrameNumber,const char *szFilename)
//{
//	FILE *fpResult = fopen(szFilename,"w");
//	if (!fpResult)
//	{
//		std::cout << "Can\'t open file " << szFilename << std::endl;
//		return false;
//	}
//
//	//std::vector<float> vecCandicate;
//	std::vector<pitch_candidate> vecCandicate;
//	pitch_candidate pcandi;
//	for (int i = 0;i < nFrameNumber;i++)
//	{
//		vecCandicate.clear();
//		pitch_frame* frame = pitch_frames_ [i];
//		int nCanNum = frame->nCandidates;
//		for (int j = 0;j < nCanNum;j++)
//		{
//			pcandi = frame -> candidate[j];
//			//double dValue = (float)(frame -> candidate[j].frequency);
//			if (pcandi.frequency < 10.0)
//			{
//				vecCandicate.push_back(pcandi);
//			}
//			if (pcandi.frequency > ceiling_ || pcandi.frequency < minimumPitch_)
//			{
//				continue;
//			}
//			vecCandicate.push_back(pcandi);
//		}
//		for (int j = 0;j < vecCandicate.size();j++)
//		{
//			if (vecCandicate[j].frequency < 10.0)
//			{
//				continue;
//			}
//			pitch_candidate pca = vecCandicate[j];
//			//double dvalue = vecCandicate[j];
//			while (pca.frequency / 2 > minimumPitch_)
//			{
//				pca.frequency /= 2;
//				bool bFind = false;
//				for (int k = 0;k < vecCandicate.size();k++)
//				{
//					if (fabs(pca.frequency - vecCandicate[k].frequency) < 3.0)
//					{
//						bFind = true;
//						break;
//					}
//				}
//				if (!bFind)
//				{
//					vecCandicate.push_back(pca);
//				}
//			}
//			pca = vecCandicate[j];
//			while (pca.frequency * 2 < ceiling_)
//			{
//				pca.frequency *= 2;
//				bool bFind = false;
//				for (int k = 0;k < vecCandicate.size();k++)
//				{
//					if (fabs(pca.frequency - vecCandicate[k].frequency) < 3.0)
//					{
//						bFind = true;
//						break;
//					}
//				}
//				//dvalue *= 2;
//				if (!bFind)
//				{
//					vecCandicate.push_back(pca);
//				}
//			}
//		}
//		
//		std::sort(vecCandicate.begin(),vecCandicate.end(),SortCandi);
//
//		for (int j = 0;j < vecCandicate.size();j++)
//		{
//			//fprintf(fpResult,"%.2f\t",12 * log(vecCandicate[j].frequency / 440.0) / log(2.0) + 69);
//			if (vecCandicate[j].frequency > 10.0)
//			{
//				double dTone = vecCandicate[j].frequency;
//				fprintf(fpResult,"%.2f\t",12 * log(dTone / 440.0) / log(2.0) + 69);//\t%.2f ,vecCandicate[j].strength
//				/*while (dTone / 2 > minimumPitch_)
//				{
//					dTone /= 2;
//					fprintf(fpResult,"%.2f\t",12 * log(dTone / 440.0) / log(2.0) + 69);
//				}
//				dTone = vecCandicate[j].frequency;
//				while (dTone * 2 < ceiling_)
//				{
//					dTone *= 2;
//					fprintf(fpResult,"%.2f\t",12 * log(dTone / 440.0) / log(2.0) + 69);
//				}*/
//				//fprintf(fpResult,"0.0");
//				//fprintf(fpResult,"%.2f\t%.2f\t",12 * log(vecCandicate[j].frequency / 440.0) / log(2.0) + 69,vecCandicate[j].strength);
//			}
//			else
//			{
//				fprintf(fpResult,"0.0\t");//\t0.0
//			}
//			//break;
//		}
//		fprintf(fpResult,"\n");
//	}
//	
//	fprintf(fpResult,"\n---------------------------------------\n");
//	fprintf(fpResult,"Min = %.2f\t,Ceiling = %.2f\n",minimumPitch_,ceiling_);
//	fclose(fpResult);
//	return 1;
//}
//
//
//bool PPitcher::SortCandi(pitch_candidate &pifirst,pitch_candidate &pisecnd)
//{
//	return pifirst.strength > pisecnd.strength;
//}