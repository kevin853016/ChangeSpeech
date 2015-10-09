/** 
 * @file	pitch.h
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
#pragma once
#include <vector>
#include <math.h>
using namespace std;
//#define PRAAT_FLOAT signed short
//#define PRAAT_FLOAT PRAAT_FLOAT
namespace PPitcher
{
	//+---------------------------------------------------------------------------+
	//+ 一帧的数据的大小 //450;//640; //10毫秒的4倍（whwu采样）
	//+---------------------------------------------------------------------------+
	const int NSAMP_WINDOW = 759/*550*/; 

	//+---------------------------------------------------------------------------+
	//+ 数据缓冲区大小 //20;//10毫秒的20倍（whwu）
	//+---------------------------------------------------------------------------+
	const int AUDIO_BUFFER_LEN = NSAMP_WINDOW * 20;

	//+---------------------------------------------------------------------------+
	//+ 帧移 //10毫秒，采样修改（whwu采样）
	//+---------------------------------------------------------------------------+
	const int NSAMP_SHIFT = 220/*160*/; 
	const int NSAMP_SHIFT_X = 0; //10毫秒
	const int TAIL_ADD = NSAMP_WINDOW / NSAMP_SHIFT; //末尾需要不足的基频点个数

	//+---------------------------------------------------------------------------+
	//+ 候选基频点的最多个数（whwu）
	//+---------------------------------------------------------------------------+
	const int MAX_CANDIDATES = 15;
	const int ONLINEF0BUFSIZE = 1000;//在线基频提取时最大帧数
	const int ONDYNAMICDISIZE = 20;//Online基频提取时计算每帧数据当前数据仅受到此前20帧数据的影响

	//+---------------------------------------------------------------------------+
	//+ 在线基频存储区大小（whwu）
	//+---------------------------------------------------------------------------+
	const int ONLINE_POOL = 100;

	typedef double PRAAT_DOUBLE;
	typedef float  PRAAT_FLOAT;
	
	//+---------------------------------------------------------------------------+
	//+ 基频候选类（whwu）
	//+---------------------------------------------------------------------------+
	struct pitch_candidate
	{
	public:
		//+---------------------------------------------------------------------------+
		//+ 基频候选对应的频率（whwu）
		//+---------------------------------------------------------------------------+
		PRAAT_DOUBLE frequency;

		//+---------------------------------------------------------------------------+
		//+ 基频候选自相关函数的值（whwu）
		//+---------------------------------------------------------------------------+
		PRAAT_DOUBLE strength;

		void operator = (const pitch_candidate& cand)
		{
			frequency = cand.frequency;
			strength = cand.strength;
		}
	};

	//基频点类
	struct pitch_frame
	{
	public:
		//+---------------------------------------------------------------------------+
		//+ 幅度比值0~1，1为幅度比较大（whwu）
		//+---------------------------------------------------------------------------+
		PRAAT_DOUBLE intensity;

		//+---------------------------------------------------------------------------+
		//+ 当前的候选数（whwu）
		//+---------------------------------------------------------------------------+
		long nCandidates;

		//+---------------------------------------------------------------------------+
		//+ 对应的候选（whwu）
		//+---------------------------------------------------------------------------+
		pitch_candidate candidate[MAX_CANDIDATES];
		long best;

		pitch_frame() : nCandidates(0), intensity(0), best(0)
		{

		}
	};

	/** 
	* @class	class pitcher
	* @brief   基频提取处理类(支持online/offline)	
	* 
	* detail...
	* 
	* @author	zhyi
	* @date	2007-05-31
	* @see		
	* @par 备注：每个基频点代表10ms的语音数据的基频，仅支持16k16bit格式, 一个线程使用一个pitcher对象
	*/
	class pitcher
	{
	public:
		pitcher(void);
	public:
		~pitcher(void);

	public:
		enum EAudioType
		{
			eAudioType_8k16bit		= 100,
			eAudioType_16k16bit,
		};

	public:
		void SetAudioType(EAudioType eAudioType);
 
		//再次使用之前重置内部状态
		int reset();

		//追加新的audio数据，size不要超过1600，
		//processed_count返回已处理的基频点数（仅获得候选，需通过调用search_path确定最优基频值）
		int append_data(signed short* data, int size, int& processed_count);

		int append_finished();
		
		//确定最优基频序列，count返回当前确定的基频点数，final_search设定是否强制search
		int search_path(int& count, bool final_search = true);

		//获得基频序列，beg为需要获取基频的起始点（时间为beg*10ms），count为需要获得基频的点数
		int get_pitch(double* pitch, int beg, int count);
		
		//获得基频序列，beg为需要获取基频的起始点（时间为beg*10ms），count为需要获得基频的点数
		int get_pitch(float* pitch, int beg, int count);

		//离线处理大段audio数据，之后直接调用get_pitch即可获得基频序列
		int offline_process(signed short* data, int size, int& count);
	
		//离线获取所有数据基频点的候选点，并且存储到一个文件中去
		int offline_getAllCandicate(const int nBeginFrame,const int nFrameNumber,const char *szFilename);

		//////////////////////////////////////////////////////////////////////////
		int online_process(signed short *data,int size,float *pitch,int &frame);

		//OnLine基频提取时内存和数值的清理
		int online_reset();

	private:

		//设定采样率(暂不支持)
		void set_smp_rate(int smp_rate){}

		//设定量化bit数(暂不支持)
		void set_bit_size(int bit_size){}

		//初始化
		int init();

		//逆初始化
		int fini();

		//处理一帧数据
		int process_frame(PRAAT_FLOAT* audio, int nx);

	private:
		//global param
		PRAAT_DOUBLE dt_;
		PRAAT_DOUBLE dx_;
		PRAAT_DOUBLE minimumPitch_;		//75.0
		PRAAT_DOUBLE periodsPerWindow_;	//3.0

		//+---------------------------------------------------------------------------+
		//+ 静音门限 //0.03（whwu）
		//+---------------------------------------------------------------------------+
		PRAAT_DOUBLE silenceThreshold_;	

		//+---------------------------------------------------------------------------+
		//+ 局部最大值判断阀值（whwu）
		//+---------------------------------------------------------------------------+
		PRAAT_DOUBLE voicingThreshold_;   //0.45
		PRAAT_DOUBLE octaveCost_;         //0.01
		PRAAT_DOUBLE octaveJumpCost_;     //0.35
		PRAAT_DOUBLE voicedUnvoicedCost_; //0.14
		PRAAT_DOUBLE ceiling_;            //600.0
		bool   pullFormants_;       //true

		PRAAT_DOUBLE dt_window_;
		PRAAT_DOUBLE *window_;
		PRAAT_DOUBLE *windowR_;

		//+---------------------------------------------------------------------------+
		//+ 获取归0后的最大参考值（whwu）
		//+---------------------------------------------------------------------------+
		PRAAT_DOUBLE globalPeak_;
		PRAAT_DOUBLE interpolation_depth_;
		PRAAT_DOUBLE brent_accuracy_;

		//+---------------------------------------------------------------------------+
		//+ 可能的最大基音周期的采样点数（whwu）
		//+---------------------------------------------------------------------------+
		long   nsamp_period_;
		long   halfnsamp_period_;
		long   brent_ixmax_;
		long   brent_depth_;

		//+---------------------------------------------------------------------------+
		//+ 一帧数据的大小（whwu）
		//+---------------------------------------------------------------------------+
		long   nsamp_window_;

		//+---------------------------------------------------------------------------+
		//+ 半帧数据的大小（whwu）
		//+---------------------------------------------------------------------------+
		long   halfnsamp_window_;
		long   minimumLag_;
		long   maximumLag_;

		//+---------------------------------------------------------------------------+
		//+ 做FFT的数据大小（whwu）
		//+---------------------------------------------------------------------------+
		long   nsampFFT_;
		double Log2 ;
		double Log440 ;

		//local param

		//+---------------------------------------------------------------------------+
		//+ 获取了候选的帧数（whwu）
		//+---------------------------------------------------------------------------+
		int    processed_count_;

		//+---------------------------------------------------------------------------+
		//+ 路径搜索过的帧的数目（whwu）
		//+---------------------------------------------------------------------------+
		int    pathed_count_;

		bool   test_mode_;

		//+---------------------------------------------------------------------------+
		//+ 帧处理容器，获取候选后的全部帧（whwu）
		//+---------------------------------------------------------------------------+
		vector<pitch_frame*> pitch_frames_;

		//+---------------------------------------------------------------------------+
		//+ 获取一帧基频的存储区（whwu）
		//+---------------------------------------------------------------------------+
		pitch_frame* GetOnePitchFrame()
		{
			if ( nCount >= ONLINE_POOL - 1 )
			{
				nCount = 0;
			}
			return &pPitch[nCount++];
		}
		int nCount;

		//+---------------------------------------------------------------------------+
		//+ 基频存储区（whwu）
		//+---------------------------------------------------------------------------+
		pitch_frame pPitch[ONLINE_POOL];


	private:
		//+---------------------------------------------------------------------------+
		//+ 数据缓冲区（whwu）
		//+---------------------------------------------------------------------------+
		PRAAT_FLOAT buf_[AUDIO_BUFFER_LEN + NSAMP_WINDOW - NSAMP_SHIFT_X];
		int   buf_valid_head_; //1st window begin position
		int   buf_valid_tail_; //last window end position

		
		//+---------------------------------------------------------------------------+
		//+ 获取数据缓冲区的数据大小（whwu）
		//+---------------------------------------------------------------------------+
		int buf_valid_len()
		{
			if(buf_valid_tail_ >= buf_valid_head_)
				return buf_valid_tail_ - buf_valid_head_;
			else
				return buf_valid_tail_ - buf_valid_head_ + AUDIO_BUFFER_LEN;
		}
		int buf_idle_len()
		{
			return AUDIO_BUFFER_LEN - buf_valid_len();
		}
		int dulpi_tail(int beg, int end)
		{
			if(buf_valid_tail_ < buf_valid_head_)//如果tail和head先后次序颠倒
			{
				if(beg > NSAMP_WINDOW - NSAMP_SHIFT_X)
					return 0;
				int dulpi_len = end > NSAMP_WINDOW - NSAMP_SHIFT_X ? NSAMP_WINDOW - NSAMP_SHIFT_X - beg : end - beg;

#ifdef _DEBUG
				if (dulpi_len > (NSAMP_WINDOW - NSAMP_SHIFT_X))
				{
					printf("Memory access invalid!\n");
				}
#endif

				memcpy(buf_ + AUDIO_BUFFER_LEN + beg, buf_ + beg, sizeof(PRAAT_FLOAT)*dulpi_len);
				return 0;
			}
			else//tail和head正常, 只需向头部dulpi
			{
				if(end < AUDIO_BUFFER_LEN)
					return 0;
				if(end > AUDIO_BUFFER_LEN + NSAMP_WINDOW - NSAMP_SHIFT_X)
					return -1;
				int dulpi_pos = beg < AUDIO_BUFFER_LEN ? AUDIO_BUFFER_LEN : beg;
				int dulpi_len = end - dulpi_pos;

#ifdef _DEBUG
				if (dulpi_len > (NSAMP_WINDOW - NSAMP_SHIFT_X))
				{
					printf("Memory access invalid!\n");
				}
#endif
				memcpy(buf_+ dulpi_pos - AUDIO_BUFFER_LEN, buf_ + dulpi_pos, sizeof(PRAAT_FLOAT)*dulpi_len);
				return 0;
			}
		}

		//+---------------------------------------------------------------------------+
		//+ 将数据传入到缓冲区中（whwu）
		//+---------------------------------------------------------------------------+
		int buf_append_data(signed short* data, int size)
		{
			int n = size + buf_valid_tail_ - AUDIO_BUFFER_LEN;
			if(n <= 0)//添加后没有超出AUDIO_BUFFER_LEN
			{
				audio_to_float(data, buf_+buf_valid_tail_, size);
				dulpi_tail(buf_valid_tail_, buf_valid_tail_ + size);
				buf_valid_tail_ += size;
			}
			else//添加后已经超出AUDIO_BUFFER_LEN
			{
				int nn = n - (NSAMP_WINDOW - NSAMP_SHIFT_X);
				if(nn <= 0)//超出部分不大于一个window-shift
				{
					int dulp_len = buf_valid_tail_ + size - AUDIO_BUFFER_LEN;//需要duplicate的长度
					audio_to_float(data, buf_+buf_valid_tail_, size);//先正常存储
					dulpi_tail(buf_valid_tail_, buf_valid_tail_ + size);//dulpi超出AUDIO_BUFFER_LEN的部分
					buf_valid_tail_ = dulp_len;
				}
				else//超出部分大于一个window-shift
				{
					int dulp_len = NSAMP_WINDOW - NSAMP_SHIFT_X;//需要duplicate的长度
					int len1 = AUDIO_BUFFER_LEN - buf_valid_tail_ + dulp_len;
					audio_to_float(data, buf_+buf_valid_tail_, len1);
					dulpi_tail(AUDIO_BUFFER_LEN, AUDIO_BUFFER_LEN + dulp_len);
					buf_valid_tail_ = dulp_len;

					//上下的跟在tail之后
					audio_to_float(data + len1, buf_+buf_valid_tail_, size - len1);
					buf_valid_tail_ += (size - len1);
				}
			}
			return 0;
		}

		//+---------------------------------------------------------------------------+
		//+ 获取一帧的数据（whwu）
		//+---------------------------------------------------------------------------+
		PRAAT_FLOAT* cur_frame()
		{
			//+---------------------------------------------------------------------------+
			//+ 获取数据缓冲区的数据大小（whwu）
			//+---------------------------------------------------------------------------+
			if( buf_valid_len() < 2*NSAMP_WINDOW )
				return 0;

			PRAAT_FLOAT* frame = buf_ + buf_valid_head_;

#ifdef _DEBUG_NEW
			if(buf_valid_head_ < 0)
				printf("ERR");

			if ( buf_valid_head_ < AUDIO_BUFFER_LEN && (buf_valid_head_ + 450) > (AUDIO_BUFFER_LEN + NSAMP_WINDOW - NSAMP_SHIFT_X) )
			{
				printf("ERR\n");
			}
#endif
			//+---------------------------------------------------------------------------+
			//+ 数据头移动一帧（whwu）
			//+---------------------------------------------------------------------------+
			buf_valid_head_ += NSAMP_SHIFT;
			if( buf_valid_head_ >= AUDIO_BUFFER_LEN )
			//if(buf_valid_head_ >= AUDIO_BUFFER_LEN - NSAMP_SHIFT)
			{
				buf_valid_head_ = buf_valid_head_ - AUDIO_BUFFER_LEN;
				//buf_valid_head_ = buf_valid_head_ - AUDIO_BUFFER_LEN + NSAMP_SHIFT;

#ifdef _DEBUG_NEW
				if ( (buf_valid_head_ + 450) > (AUDIO_BUFFER_LEN + NSAMP_WINDOW - NSAMP_SHIFT_X) )
				{
					printf("ERR\n");
				}
#endif

			}

			return frame;
		}

		int audio_to_float(signed short* data, PRAAT_FLOAT* audio, int size)
		{
			if(test_mode_ == false)
			{
				///*
				for(int i = 0; i < size; i++)
				{
					audio[i] = data[i] * (1.0f / 32768);
				}
			}
			else
			{
				for(int i = 0; i < size; i++)
				{
					audio[i] = data[i];
				}
			}
			//*/
			return 0;
		}
	};

	//全局参数和数据，所有线程共享，线程安全
	//单体模式
	class global_param
	{
	public:
		static const global_param& get_instance() 
		{
			static global_param inst_;
			return inst_;
		}

	private:
		global_param();
		~global_param();

	public:
		//global param
		PRAAT_DOUBLE dt_;

		//+---------------------------------------------------------------------------+
		//+ 采样频率（whwu）
		//+---------------------------------------------------------------------------+
		PRAAT_DOUBLE dx_;

		//+---------------------------------------------------------------------------+
		//+ 人声最低频率 //75.0（whwu）
		//+---------------------------------------------------------------------------+
		PRAAT_DOUBLE minimumPitch_;		
		PRAAT_DOUBLE periodsPerWindow_;	//3.0
		PRAAT_DOUBLE silenceThreshold_;	//0.03

		//+---------------------------------------------------------------------------+
		//+ 局部最大值判断阀值（whwu）
		//+---------------------------------------------------------------------------+
		PRAAT_DOUBLE voicingThreshold_;   //0.45
		PRAAT_DOUBLE octaveCost_;         //0.01
		PRAAT_DOUBLE octaveJumpCost_;     //0.35
		PRAAT_DOUBLE voicedUnvoicedCost_; //0.14
		PRAAT_DOUBLE ceiling_;            //600.0
		bool   pullFormants_;       //true

		PRAAT_DOUBLE dt_window_;
		PRAAT_DOUBLE *window_;
		PRAAT_DOUBLE *windowR_;
		PRAAT_DOUBLE globalPeak_;
		PRAAT_DOUBLE interpolation_depth_;
		PRAAT_DOUBLE brent_accuracy_;

		//+---------------------------------------------------------------------------+
		//+ 可能的最大基音周期的采样点数（whwu）
		//+---------------------------------------------------------------------------+
		long   nsamp_period_;
		long   halfnsamp_period_;
		long   brent_ixmax_;
		long   brent_depth_;

		//+---------------------------------------------------------------------------+
		//+ 一帧数据的大小（whwu）
		//+---------------------------------------------------------------------------+
		long   nsamp_window_;

		//+---------------------------------------------------------------------------+
		//+ 半帧数据的大小（whwu）
		//+---------------------------------------------------------------------------+
		long   halfnsamp_window_;
		long   minimumLag_;
		long   maximumLag_;
		long   nsampFFT_;
	};

	PRAAT_DOUBLE NUM_interpolate_sinc_d (PRAAT_DOUBLE y [], long nx, PRAAT_DOUBLE x, long maxDepth);
	PRAAT_DOUBLE NUMimproveMaximum_d(PRAAT_DOUBLE *corr_data, int data_num, PRAAT_FLOAT offset_time, int max_depth_, PRAAT_DOUBLE& xmid);
	void fft (PRAAT_DOUBLE *data, long nn, int isign);
	void real_fft (PRAAT_DOUBLE *data, long n, int isign);
	bool SortCandi(pitch_candidate &pifirst,pitch_candidate &pisecnd);
};