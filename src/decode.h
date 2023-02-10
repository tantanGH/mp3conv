#ifndef __H_DECODE__
#define __H_DECODE__

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include "pcm.h"
#include "adpcm.h"
#include "mad.h"

typedef struct {
  uint8_t* start;
  size_t length;
} MAD_BUFFER;

typedef struct mad_stream MAD_STREAM;
typedef struct mad_header MAD_HEADER;
typedef struct mad_pcm MAD_PCM;
typedef struct mad_frame MAD_FRAME;
typedef struct mad_decoder MAD_DECODER;

typedef struct {

  int32_t mp3_bit_rate;
  int32_t mp3_sample_rate;
  int32_t mp3_num_channels;

  size_t num_samples;
  size_t consumed;

  int16_t out_format;
  FILE* out_fp;

  PCM_HANDLE* pcm;
  ADPCM_HANDLE* adpcm;

} MP3_DECODE_HANDLE;

int32_t decode_init(MP3_DECODE_HANDLE* decode, PCM_HANDLE* pcm, ADPCM_HANDLE* adpcm, int16_t out_format, FILE* fp);
void decode_close(MP3_DECODE_HANDLE* decode);
int32_t decode_mp3(MP3_DECODE_HANDLE* decode, void* mp3_data, size_t mp3_data_len);

#endif