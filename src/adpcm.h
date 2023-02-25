#ifndef __H_ADPCM__
#define __H_ADPCM__

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#define ADPCM_SAMPLE_RATE  (15625)
#define ADPCM_BUFFER_LEN   (15625)

#define ADPCM_FWRITE_BUFFER_LEN (65536)

typedef struct {
  FILE* fp;
  int16_t use_high_memory;
  int16_t step_index;
  int16_t last_estimate;
  size_t num_samples;
  size_t buffer_len;
  size_t buffer_ofs;
  uint8_t* buffer;
  uint8_t* fwrite_buffer;
} ADPCM_WRITE_HANDLE;

int32_t adpcm_init(ADPCM_WRITE_HANDLE* adpcm, FILE* fp, int16_t use_high_memory);
int32_t adpcm_flush(ADPCM_WRITE_HANDLE* adpcm);
void adpcm_close(ADPCM_WRITE_HANDLE* adpcm);
int32_t adpcm_write(ADPCM_WRITE_HANDLE* adpcm, int16_t* pcm_buffer, size_t pcm_len, int16_t pcm_channels);

#endif