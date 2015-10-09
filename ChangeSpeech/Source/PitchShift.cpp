#ifdef NDEBUG /* N.B. assert used with active statements so enable always. */
#undef NDEBUG /* Must undef above assert.h or other that might include it. */
#endif

#include "PitchShift.h"

#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include "config.h"
#endif

#ifdef ENABLE_NLS
#include <libintl.h>
#endif

#define         _ISOC9X_SOURCE  1
#define         _ISOC99_SOURCE  1
#define         __USE_ISOC99    1
#define         __USE_ISOC9X    1

#include <math.h>

#include "ladspa.h"

#ifndef M_PI
#define M_PI   3.1415926535897932384626433832795028841971693993751
#endif


#include <stdlib.h>
#include <math.h>
#include "ladspa-util.h"

/* Beware of dependcies if you change this */
#define DELAY_SIZE 8192

typedef struct {
	LADSPA_Data *pitch;
	LADSPA_Data *size;
	LADSPA_Data *input;
	LADSPA_Data *output;
	LADSPA_Data *latency;
	unsigned int count;
	LADSPA_Data *delay;
	unsigned int delay_mask;
	unsigned int delay_ofs;
	float        last_gain;
	float        last_inc;
	int          last_size;
	fixp16       rptr;
	unsigned int wptr;
	LADSPA_Data run_adding_gain;
} AmPitchshift;

void cleanupAmPitchshift(LADSPA_Handle instance) {
//#line 39 "am_pitchshift_1433.xml"
	AmPitchshift *plugin_data = (AmPitchshift *)instance;
	free(plugin_data->delay);
	free(instance);
}

void connectPortAmPitchshift(
 LADSPA_Handle instance,
 unsigned long port,
 LADSPA_Data *data) {
	AmPitchshift *plugin;

	plugin = (AmPitchshift *)instance;
	switch (port) {
	case AMPITCHSHIFT_PITCH:
		plugin->pitch = data;
		break;
	case AMPITCHSHIFT_SIZE:
		plugin->size = data;
		break;
	case AMPITCHSHIFT_INPUT:
		plugin->input = data;
		break;
	case AMPITCHSHIFT_OUTPUT:
		plugin->output = data;
		break;
	case AMPITCHSHIFT_LATENCY:
		plugin->latency = data;
		break;
	}
}

LADSPA_Handle instantiateAmPitchshift(
 unsigned long s_rate) {
	AmPitchshift *plugin_data = (AmPitchshift *)malloc(sizeof(AmPitchshift));
	unsigned int count;
	LADSPA_Data *delay = NULL;
	unsigned int delay_mask;
	unsigned int delay_ofs;
	float last_gain;
	float last_inc;
	int last_size;
	fixp16 rptr;
	unsigned int wptr;

//#line 27 "am_pitchshift_1433.xml"
	delay = (LADSPA_Data *)calloc(DELAY_SIZE, sizeof(LADSPA_Data));
	rptr.all = 0;
	wptr = 0;
	last_size = -1;
	delay_mask = 0xFF;
	delay_ofs = 0x80;
	last_gain = 0.5f;
	count = 0;
	last_inc = 0.0f;

	plugin_data->count = count;
	plugin_data->delay = delay;
	plugin_data->delay_mask = delay_mask;
	plugin_data->delay_ofs = delay_ofs;
	plugin_data->last_gain = last_gain;
	plugin_data->last_inc = last_inc;
	plugin_data->last_size = last_size;
	plugin_data->rptr = rptr;
	plugin_data->wptr = wptr;

	return (LADSPA_Handle)plugin_data;
}

#undef buffer_write
#undef RUN_ADDING
#undef RUN_REPLACING

#define buffer_write(b, v) (b = v)
#define RUN_ADDING    0
#define RUN_REPLACING 1

void runAmPitchshift(LADSPA_Handle instance, unsigned long sample_count) {
	AmPitchshift *plugin_data = (AmPitchshift *)instance;

	/* Pitch shift (float value) */
	const LADSPA_Data pitch = *(plugin_data->pitch);

	/* Buffer size (float value) */
	const LADSPA_Data size = *(plugin_data->size);

	/* Input (array of floats of length sample_count) */
	const LADSPA_Data * const input = plugin_data->input;

	/* Output (array of floats of length sample_count) */
	LADSPA_Data * const output = plugin_data->output;
	unsigned int count = plugin_data->count;
	LADSPA_Data * delay = plugin_data->delay;
	unsigned int delay_mask = plugin_data->delay_mask;
	unsigned int delay_ofs = plugin_data->delay_ofs;
	float last_gain = plugin_data->last_gain;
	float last_inc = plugin_data->last_inc;
	int last_size = plugin_data->last_size;
	fixp16 rptr = plugin_data->rptr;
	unsigned int wptr = plugin_data->wptr;

//#line 43 "am_pitchshift_1433.xml"
	unsigned long pos;
	fixp16 om;
	float gain = last_gain, gain_inc = last_inc;
	unsigned int i;

	om.all = f_round(pitch * 65536.0f);

	if (size != last_size) {
	  int size_tmp = f_round(size);

	  if (size_tmp > 7) {
	    size_tmp = 5;
	  } else if (size_tmp < 1) {
	    size_tmp = 1;
	  }
	  plugin_data->last_size = size;

	  /* Calculate the ringbuf parameters, the magick constants will need
	   * to be changed if you change DELAY_SIZE */
	  delay_mask = (1 << (size_tmp + 6)) - 1;
	  delay_ofs = 1 << (size_tmp + 5);
	}

	for (pos = 0; pos < sample_count; pos++) {
	  float out = 0.0f;

	  if (count++ > 14) {
	    float tmp;
	    count = 0;
	    tmp = 0.5f * (float)((rptr.part.in - wptr + delay_ofs/2) &
	          delay_mask) / (float)delay_ofs;
	    tmp = sinf(M_PI * 2.0f * tmp) * 0.5f + 0.5f;
	    gain_inc = (tmp - gain) / 15.0f;
	  }
	  gain += gain_inc;

	  delay[wptr] = input[pos];

	  /* Add contributions from the two readpointers, scaled by thier
	   * distance from the write pointer */
	  i = rptr.part.in;
	  out += cube_interp((float)rptr.part.fr * 0.0000152587f,
	                     delay[(i - 1) & delay_mask], delay[i],
	                     delay[(i + 1) & delay_mask],
	                     delay[(i + 2) & delay_mask]) * (1.0f - gain);
	  i += delay_ofs;
	  out += cube_interp((float)rptr.part.fr * 0.0000152587f,
	                     delay[(i - 1) & delay_mask], delay[i & delay_mask],
	                     delay[(i + 1) & delay_mask],
	                     delay[(i + 2) & delay_mask]) * gain;

	  buffer_write(output[pos], out);

	  /* Increment ringbuffer pointers */
	  wptr = (wptr + 1) & delay_mask;
	  rptr.all += om.all;
	  rptr.part.in &= delay_mask;
	}

    plugin_data->rptr.all = rptr.all;
    plugin_data->wptr = wptr;
    plugin_data->delay_mask = delay_mask;
    plugin_data->delay_ofs = delay_ofs;
    plugin_data->last_gain = gain;
    plugin_data->count = count;
    plugin_data->last_inc = gain_inc;

    *(plugin_data->latency) = delay_ofs/2;
}
#undef buffer_write
#undef RUN_ADDING
#undef RUN_REPLACING

#define buffer_write(b, v) (b += (v) * run_adding_gain)
#define RUN_ADDING    1
#define RUN_REPLACING 0

void setRunAddingGainAmPitchshift(LADSPA_Handle instance, LADSPA_Data gain) {
	((AmPitchshift *)instance)->run_adding_gain = gain;
}

void runAddingAmPitchshift(LADSPA_Handle instance, unsigned long sample_count) {
	AmPitchshift *plugin_data = (AmPitchshift *)instance;
	LADSPA_Data run_adding_gain = plugin_data->run_adding_gain;

	/* Pitch shift (float value) */
	const LADSPA_Data pitch = *(plugin_data->pitch);

	/* Buffer size (float value) */
	const LADSPA_Data size = *(plugin_data->size);

	/* Input (array of floats of length sample_count) */
	const LADSPA_Data * const input = plugin_data->input;

	/* Output (array of floats of length sample_count) */
	LADSPA_Data * const output = plugin_data->output;
	unsigned int count = plugin_data->count;
	LADSPA_Data * delay = plugin_data->delay;
	unsigned int delay_mask = plugin_data->delay_mask;
	unsigned int delay_ofs = plugin_data->delay_ofs;
	float last_gain = plugin_data->last_gain;
	float last_inc = plugin_data->last_inc;
	int last_size = plugin_data->last_size;
	fixp16 rptr = plugin_data->rptr;
	unsigned int wptr = plugin_data->wptr;

//#line 43 "am_pitchshift_1433.xml"
	unsigned long pos;
	fixp16 om;
	float gain = last_gain, gain_inc = last_inc;
	unsigned int i;

	om.all = f_round(pitch * 65536.0f);

	if (size != last_size) {
	  int size_tmp = f_round(size);

	  if (size_tmp > 7) {
	    size_tmp = 5;
	  } else if (size_tmp < 1) {
	    size_tmp = 1;
	  }
	  plugin_data->last_size = size;

	  /* Calculate the ringbuf parameters, the magick constants will need
	   * to be changed if you change DELAY_SIZE */
	  delay_mask = (1 << (size_tmp + 6)) - 1;
	  delay_ofs = 1 << (size_tmp + 5);
	}

	for (pos = 0; pos < sample_count; pos++) {
	  float out = 0.0f;

	  if (count++ > 14) {
	    float tmp;
	    count = 0;
	    tmp = 0.5f * (float)((rptr.part.in - wptr + delay_ofs/2) &
	          delay_mask) / (float)delay_ofs;
	    tmp = sinf(M_PI * 2.0f * tmp) * 0.5f + 0.5f;
	    gain_inc = (tmp - gain) / 15.0f;
	  }
	  gain += gain_inc;

	  delay[wptr] = input[pos];

	  /* Add contributions from the two readpointers, scaled by thier
	   * distance from the write pointer */
	  i = rptr.part.in;
	  out += cube_interp((float)rptr.part.fr * 0.0000152587f,
	                     delay[(i - 1) & delay_mask], delay[i],
	                     delay[(i + 1) & delay_mask],
	                     delay[(i + 2) & delay_mask]) * (1.0f - gain);
	  i += delay_ofs;
	  out += cube_interp((float)rptr.part.fr * 0.0000152587f,
	                     delay[(i - 1) & delay_mask], delay[i & delay_mask],
	                     delay[(i + 1) & delay_mask],
	                     delay[(i + 2) & delay_mask]) * gain;

	  buffer_write(output[pos], out);

	  /* Increment ringbuffer pointers */
	  wptr = (wptr + 1) & delay_mask;
	  rptr.all += om.all;
	  rptr.part.in &= delay_mask;
	}

    plugin_data->rptr.all = rptr.all;
    plugin_data->wptr = wptr;
    plugin_data->delay_mask = delay_mask;
    plugin_data->delay_ofs = delay_ofs;
    plugin_data->last_gain = gain;
    plugin_data->count = count;
    plugin_data->last_inc = gain_inc;

    *(plugin_data->latency) = delay_ofs/2;
}
