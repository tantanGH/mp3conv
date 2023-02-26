#ifndef __H_NAS_ADPCM__
#define __H_NAS_ADPCM__

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#define NAS_ADPCM_BUFFER_LEN (48000*4)
#define NAS_ADPCM_FWRITE_BUFFER_LEN (48000*4)

typedef struct {
  FILE* fp;
  int16_t use_high_memory;
  int16_t lib_initialized;
  size_t num_samples;
  size_t buffer_len;
  size_t buffer_ofs;
  uint8_t* buffer;
  uint8_t* fwrite_buffer;
} NAS_ADPCM_WRITE_HANDLE;

int32_t nas_adpcm_init(NAS_ADPCM_WRITE_HANDLE* nas, FILE* fp, int16_t use_high_memory);
int32_t nas_adpcm_flush(NAS_ADPCM_WRITE_HANDLE* nas);
void nas_adpcm_close(NAS_ADPCM_WRITE_HANDLE* nas);
int32_t nas_adpcm_write(NAS_ADPCM_WRITE_HANDLE* nas, int16_t* pcm_data, size_t pcm_len, int16_t channels);

#endif