#ifndef __H_MP3__
#define __H_MP3__

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include "pcm.h"
#include "adpcm.h"
#include "mad.h"

#define MP3_DECODE_BUFFER_LEN (48000*2)

typedef struct mad_stream MAD_STREAM;
typedef struct mad_synth MAD_SYNTH;
typedef struct mad_header MAD_HEADER;
typedef struct mad_pcm MAD_PCM;
typedef struct mad_frame MAD_FRAME;
typedef mad_timer_t MAD_TIMER;

typedef struct {

  int16_t use_high_memory;

  int32_t mp3_sample_rate;
  int32_t mp3_channels;

  MAD_STREAM mad_stream;
  MAD_FRAME mad_frame;
  MAD_SYNTH mad_synth;
  MAD_TIMER mad_timer;

  PCM_WRITE_HANDLE* pcm;
  ADPCM_WRITE_HANDLE* adpcm;

  size_t buffer_len;
  int16_t* buffer;

} MP3_DECODE_HANDLE;

int32_t mp3_init(MP3_DECODE_HANDLE* decode, PCM_WRITE_HANDLE* pcm, ADPCM_WRITE_HANDLE* adpcm, int16_t use_high_memory);
void mp3_close(MP3_DECODE_HANDLE* decode);
int32_t mp3_decode(MP3_DECODE_HANDLE* decode, uint8_t* mp3_data, size_t mp3_data_len);

#endif