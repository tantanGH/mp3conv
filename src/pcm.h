#ifndef __H_PCM__
#define __H_PCM__

#include <stdint.h>
#include <stddef.h>

#define PCM_BUFFER_LEN (48000*2*2*4)

typedef struct {
  int16_t use_high_memory;
  size_t num_samples;
  size_t buffer_len;
  size_t buffer_ofs;
  void* buffer;
} PCM_HANDLE;

int32_t pcm_init(PCM_HANDLE* pcm, int16_t use_high_memory);
void pcm_close(PCM_HANDLE* pcm);
int32_t pcm_write(PCM_HANDLE* pcm, FILE* fp);

#endif