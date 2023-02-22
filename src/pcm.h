#ifndef __H_PCM__
#define __H_PCM__

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#define PCM_BUFFER_LEN (48000*2)

typedef struct {
  FILE* fp;
  int16_t use_high_memory;
  size_t num_samples;
  size_t buffer_len;
  size_t buffer_ofs;
  int16_t* buffer;
} PCM_WRITE_HANDLE;

int32_t pcm_init(PCM_WRITE_HANDLE* pcm, FILE* fp, int16_t use_high_memory);
int32_t pcm_flush(PCM_WRITE_HANDLE* pcm);
void pcm_close(PCM_WRITE_HANDLE* pcm);
int32_t pcm_write(PCM_WRITE_HANDLE* pcm, int16_t* pcm_samples, size_t len);

#endif