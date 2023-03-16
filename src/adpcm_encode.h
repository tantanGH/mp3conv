#ifndef __H_ADPCM_ENCODE__
#define __H_ADPCM_ENCODE__

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#define ADPCM_ENCODE_SAMPLE_RATE  (15625)
#define ADPCM_ENCODE_BUFFER_LEN   (15625*8)
#define ADPCM_ENCODE_FWRITE_BUFFER_LEN (15625*8)

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
} ADPCM_ENCODE_HANDLE;

int32_t adpcm_encode_init(ADPCM_ENCODE_HANDLE* adpcm, FILE* fp, int16_t use_high_memory);
int32_t adpcm_encode_flush(ADPCM_ENCODE_HANDLE* adpcm);
void adpcm_encode_close(ADPCM_ENCODE_HANDLE* adpcm);
int32_t adpcm_encode_write(ADPCM_ENCODE_HANDLE* adpcm, int16_t* pcm_buffer, size_t pcm_len, int16_t pcm_channels);

#endif