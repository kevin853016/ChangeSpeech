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
 * @par History��
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
	//+ һ֡�����ݵĴ�С //450;//640; //10�����4����whwu������
	//+---------------------------------------------------------------------------+
	const int NSAMP_WINDOW = 759/*550*/; 

	//+---------------------------------------------------------------------------+
	//+ ���ݻ�������С //20;//10�����20����whwu��
	//+---------------------------------------------------------------------------+
	const int AUDIO_BUFFER_LEN = NSAMP_WINDOW * 20;

	//+---------------------------------------------------------------------------+
	//+ ֡�� //10���룬�����޸ģ�whwu������
	//+---------------------------------------------------------------------------+
	const int NSAMP_SHIFT = 220/*160*/; 
	const int NSAMP_SHIFT_X = 0; //10����
	const int TAIL_ADD = NSAMP_WINDOW / NSAMP_SHIFT; //ĩβ��Ҫ����Ļ�Ƶ�����

	//+---------------------------------------------------------------------------+
	//+ ��ѡ��Ƶ�����������whwu��
	//+---------------------------------------------------------------------------+
	const int MAX_CANDIDATES = 15;
	const int ONLINEF0BUFSIZE = 1000;//���߻�Ƶ��ȡʱ���֡��
	const int ONDYNAMICDISIZE = 20;//Online��Ƶ��ȡʱ����ÿ֡���ݵ�ǰ���ݽ��ܵ���ǰ20֡���ݵ�Ӱ��

	//+---------------------------------------------------------------------------+
	//+ ���߻�Ƶ�洢����С��whwu��
	//+---------------------------------------------------------------------------+
	const int ONLINE_POOL = 100;

	typedef double PRAAT_DOUBLE;
	typedef float  PRAAT_FLOAT;
	
	//+---------------------------------------------------------------------------+
	//+ ��Ƶ��ѡ�ࣨwhwu��
	//+---------------------------------------------------------------------------+
	struct pitch_candidate
	{
	public:
		//+---------------------------------------------------------------------------+
		//+ ��Ƶ��ѡ��Ӧ��Ƶ�ʣ�whwu��
		//+---------------------------------------------------------------------------+
		PRAAT_DOUBLE frequency;

		//+---------------------------------------------------------------------------+
		//+ ��Ƶ��ѡ����غ�����ֵ��whwu��
		//+---------------------------------------------------------------------------+
		PRAAT_DOUBLE strength;

		void operator = (const pitch_candidate& cand)
		{
			frequency = cand.frequency;
			strength = cand.strength;
		}
	};

	//��Ƶ����
	struct pitch_frame
	{
	public:
		//+---------------------------------------------------------------------------+
		//+ ���ȱ�ֵ0~1��1Ϊ���ȱȽϴ�whwu��
		//+---------------------------------------------------------------------------+
		PRAAT_DOUBLE intensity;

		//+---------------------------------------------------------------------------+
		//+ ��ǰ�ĺ�ѡ����whwu��
		//+---------------------------------------------------------------------------+
		long nCandidates;

		//+---------------------------------------------------------------------------+
		//+ ��Ӧ�ĺ�ѡ��whwu��
		//+---------------------------------------------------------------------------+
		pitch_candidate candidate[MAX_CANDIDATES];
		long best;

		pitch_frame() : nCandidates(0), intensity(0), best(0)
		{

		}
	};

	/** 
	* @class	class pitcher
	* @brief   ��Ƶ��ȡ������(֧��online/offline)	
	* 
	* detail...
	* 
	* @author	zhyi
	* @date	2007-05-31
	* @see		
	* @par ��ע��ÿ����Ƶ�����10ms���������ݵĻ�Ƶ����֧��16k16bit��ʽ, һ���߳�ʹ��һ��pitcher����
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
 
		//�ٴ�ʹ��֮ǰ�����ڲ�״̬
		int reset();

		//׷���µ�audio���ݣ�size��Ҫ����1600��
		//processed_count�����Ѵ���Ļ�Ƶ����������ú�ѡ����ͨ������search_pathȷ�����Ż�Ƶֵ��
		int append_data(signed short* data, int size, int& processed_count);

		int append_finished();
		
		//ȷ�����Ż�Ƶ���У�count���ص�ǰȷ���Ļ�Ƶ������final_search�趨�Ƿ�ǿ��search
		int search_path(int& count, bool final_search = true);

		//��û�Ƶ���У�begΪ��Ҫ��ȡ��Ƶ����ʼ�㣨ʱ��Ϊbeg*10ms����countΪ��Ҫ��û�Ƶ�ĵ���
		int get_pitch(double* pitch, int beg, int count);
		
		//��û�Ƶ���У�begΪ��Ҫ��ȡ��Ƶ����ʼ�㣨ʱ��Ϊbeg*10ms����countΪ��Ҫ��û�Ƶ�ĵ���
		int get_pitch(float* pitch, int beg, int count);

		//���ߴ�����audio���ݣ�֮��ֱ�ӵ���get_pitch���ɻ�û�Ƶ����
		int offline_process(signed short* data, int size, int& count);
	
		//���߻�ȡ�������ݻ�Ƶ��ĺ�ѡ�㣬���Ҵ洢��һ���ļ���ȥ
		int offline_getAllCandicate(const int nBeginFrame,const int nFrameNumber,const char *szFilename);

		//////////////////////////////////////////////////////////////////////////
		int online_process(signed short *data,int size,float *pitch,int &frame);

		//OnLine��Ƶ��ȡʱ�ڴ����ֵ������
		int online_reset();

	private:

		//�趨������(�ݲ�֧��)
		void set_smp_rate(int smp_rate){}

		//�趨����bit��(�ݲ�֧��)
		void set_bit_size(int bit_size){}

		//��ʼ��
		int init();

		//���ʼ��
		int fini();

		//����һ֡����
		int process_frame(PRAAT_FLOAT* audio, int nx);

	private:
		//global param
		PRAAT_DOUBLE dt_;
		PRAAT_DOUBLE dx_;
		PRAAT_DOUBLE minimumPitch_;		//75.0
		PRAAT_DOUBLE periodsPerWindow_;	//3.0

		//+---------------------------------------------------------------------------+
		//+ �������� //0.03��whwu��
		//+---------------------------------------------------------------------------+
		PRAAT_DOUBLE silenceThreshold_;	

		//+---------------------------------------------------------------------------+
		//+ �ֲ����ֵ�жϷ�ֵ��whwu��
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
		//+ ��ȡ��0������ο�ֵ��whwu��
		//+---------------------------------------------------------------------------+
		PRAAT_DOUBLE globalPeak_;
		PRAAT_DOUBLE interpolation_depth_;
		PRAAT_DOUBLE brent_accuracy_;

		//+---------------------------------------------------------------------------+
		//+ ���ܵ����������ڵĲ���������whwu��
		//+---------------------------------------------------------------------------+
		long   nsamp_period_;
		long   halfnsamp_period_;
		long   brent_ixmax_;
		long   brent_depth_;

		//+---------------------------------------------------------------------------+
		//+ һ֡���ݵĴ�С��whwu��
		//+---------------------------------------------------------------------------+
		long   nsamp_window_;

		//+---------------------------------------------------------------------------+
		//+ ��֡���ݵĴ�С��whwu��
		//+---------------------------------------------------------------------------+
		long   halfnsamp_window_;
		long   minimumLag_;
		long   maximumLag_;

		//+---------------------------------------------------------------------------+
		//+ ��FFT�����ݴ�С��whwu��
		//+---------------------------------------------------------------------------+
		long   nsampFFT_;
		double Log2 ;
		double Log440 ;

		//local param

		//+---------------------------------------------------------------------------+
		//+ ��ȡ�˺�ѡ��֡����whwu��
		//+---------------------------------------------------------------------------+
		int    processed_count_;

		//+---------------------------------------------------------------------------+
		//+ ·����������֡����Ŀ��whwu��
		//+---------------------------------------------------------------------------+
		int    pathed_count_;

		bool   test_mode_;

		//+---------------------------------------------------------------------------+
		//+ ֡������������ȡ��ѡ���ȫ��֡��whwu��
		//+---------------------------------------------------------------------------+
		vector<pitch_frame*> pitch_frames_;

		//+---------------------------------------------------------------------------+
		//+ ��ȡһ֡��Ƶ�Ĵ洢����whwu��
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
		//+ ��Ƶ�洢����whwu��
		//+---------------------------------------------------------------------------+
		pitch_frame pPitch[ONLINE_POOL];


	private:
		//+---------------------------------------------------------------------------+
		//+ ���ݻ�������whwu��
		//+---------------------------------------------------------------------------+
		PRAAT_FLOAT buf_[AUDIO_BUFFER_LEN + NSAMP_WINDOW - NSAMP_SHIFT_X];
		int   buf_valid_head_; //1st window begin position
		int   buf_valid_tail_; //last window end position

		
		//+---------------------------------------------------------------------------+
		//+ ��ȡ���ݻ����������ݴ�С��whwu��
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
			if(buf_valid_tail_ < buf_valid_head_)//���tail��head�Ⱥ����ߵ�
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
			else//tail��head����, ֻ����ͷ��dulpi
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
		//+ �����ݴ��뵽�������У�whwu��
		//+---------------------------------------------------------------------------+
		int buf_append_data(signed short* data, int size)
		{
			int n = size + buf_valid_tail_ - AUDIO_BUFFER_LEN;
			if(n <= 0)//��Ӻ�û�г���AUDIO_BUFFER_LEN
			{
				audio_to_float(data, buf_+buf_valid_tail_, size);
				dulpi_tail(buf_valid_tail_, buf_valid_tail_ + size);
				buf_valid_tail_ += size;
			}
			else//��Ӻ��Ѿ�����AUDIO_BUFFER_LEN
			{
				int nn = n - (NSAMP_WINDOW - NSAMP_SHIFT_X);
				if(nn <= 0)//�������ֲ�����һ��window-shift
				{
					int dulp_len = buf_valid_tail_ + size - AUDIO_BUFFER_LEN;//��Ҫduplicate�ĳ���
					audio_to_float(data, buf_+buf_valid_tail_, size);//�������洢
					dulpi_tail(buf_valid_tail_, buf_valid_tail_ + size);//dulpi����AUDIO_BUFFER_LEN�Ĳ���
					buf_valid_tail_ = dulp_len;
				}
				else//�������ִ���һ��window-shift
				{
					int dulp_len = NSAMP_WINDOW - NSAMP_SHIFT_X;//��Ҫduplicate�ĳ���
					int len1 = AUDIO_BUFFER_LEN - buf_valid_tail_ + dulp_len;
					audio_to_float(data, buf_+buf_valid_tail_, len1);
					dulpi_tail(AUDIO_BUFFER_LEN, AUDIO_BUFFER_LEN + dulp_len);
					buf_valid_tail_ = dulp_len;

					//���µĸ���tail֮��
					audio_to_float(data + len1, buf_+buf_valid_tail_, size - len1);
					buf_valid_tail_ += (size - len1);
				}
			}
			return 0;
		}

		//+---------------------------------------------------------------------------+
		//+ ��ȡһ֡�����ݣ�whwu��
		//+---------------------------------------------------------------------------+
		PRAAT_FLOAT* cur_frame()
		{
			//+---------------------------------------------------------------------------+
			//+ ��ȡ���ݻ����������ݴ�С��whwu��
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
			//+ ����ͷ�ƶ�һ֡��whwu��
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

	//ȫ�ֲ��������ݣ������̹߳����̰߳�ȫ
	//����ģʽ
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
		//+ ����Ƶ�ʣ�whwu��
		//+---------------------------------------------------------------------------+
		PRAAT_DOUBLE dx_;

		//+---------------------------------------------------------------------------+
		//+ �������Ƶ�� //75.0��whwu��
		//+---------------------------------------------------------------------------+
		PRAAT_DOUBLE minimumPitch_;		
		PRAAT_DOUBLE periodsPerWindow_;	//3.0
		PRAAT_DOUBLE silenceThreshold_;	//0.03

		//+---------------------------------------------------------------------------+
		//+ �ֲ����ֵ�жϷ�ֵ��whwu��
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
		//+ ���ܵ����������ڵĲ���������whwu��
		//+---------------------------------------------------------------------------+
		long   nsamp_period_;
		long   halfnsamp_period_;
		long   brent_ixmax_;
		long   brent_depth_;

		//+---------------------------------------------------------------------------+
		//+ һ֡���ݵĴ�С��whwu��
		//+---------------------------------------------------------------------------+
		long   nsamp_window_;

		//+---------------------------------------------------------------------------+
		//+ ��֡���ݵĴ�С��whwu��
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