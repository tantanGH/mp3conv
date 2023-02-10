#ifndef __H_ADPCM__
#define __H_ADPCM__

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#define ADPCM_SAMPLE_RATE (15625)

typedef struct {
  int16_t step_index;
  int16_t last_estimate;
  size_t num_samples;
  size_t buffer_len;
  size_t buffer_ofs;
  uint8_t* buffer;
} ADPCM_HANDLE;

int32_t adpcm_init(ADPCM_HANDLE* adpcm, size_t buffer_len);
void adpcm_close(ADPCM_HANDLE* adpcm);
int32_t adpcm_encode(ADPCM_HANDLE* adpcm, void* pcm_buffer, size_t pcm_buffer_len, int16_t pcm_bit_depth, int16_t pcm_channels);
int32_t adpcm_write(ADPCM_HANDLE* adpcm, FILE* fp, int16_t flush);

#endif