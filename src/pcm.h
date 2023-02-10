#ifndef __H_PCM__
#define __H_PCM__

#include <stdint.h>
#include <stddef.h>

typedef struct {
  size_t num_samples;
  size_t buffer_len;
  size_t buffer_ofs;
  void* buffer;
} PCM_HANDLE;

int32_t pcm_init(PCM_HANDLE* pcm, size_t buffer_len);
void pcm_close(PCM_HANDLE* pcm);
int32_t pcm_write(PCM_HANDLE* pcm, FILE* fp);

#endif