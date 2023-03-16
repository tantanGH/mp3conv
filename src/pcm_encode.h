#ifndef __H_PCM_ENCODE__
#define __H_PCM_ENCODE__

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#define PCM_ENCODE_BUFFER_LEN (48000*4)
#define PCM_ENCODE_FWRITE_BUFFER_LEN (48000*4)

typedef struct {
  FILE* fp;
  int16_t use_high_memory;
  size_t num_samples;
  size_t buffer_len;
  size_t buffer_ofs;
  int16_t* buffer;
  int16_t* fwrite_buffer;
} PCM_ENCODE_HANDLE;

int32_t pcm_encode_init(PCM_ENCODE_HANDLE* pcm, FILE* fp, int16_t use_high_memory);
int32_t pcm_encode_flush(PCM_ENCODE_HANDLE* pcm);
void pcm_encode_close(PCM_ENCODE_HANDLE* pcm);
int32_t pcm_encode_write(PCM_ENCODE_HANDLE* pcm, int16_t* pcm_samples, size_t len);

#endif