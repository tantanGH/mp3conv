#ifndef __H_YM2608_ENCODE__
#define __H_YM2608_ENCODE__

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#define YM2608_ENCODE_BUFFER_LEN (48000*4)
#define YM2608_ENCODE_FWRITE_BUFFER_LEN (48000*4)

typedef struct {
  FILE* fp;
  int16_t use_high_memory;
  int16_t lib_initialized;
  size_t num_samples;
  size_t buffer_len;
  size_t buffer_ofs;
  uint8_t* buffer;
  uint8_t* fwrite_buffer;
} YM2608_ENCODE_HANDLE;

int32_t ym2608_encode_init(YM2608_ENCODE_HANDLE* nas, FILE* fp, int16_t use_high_memory);
int32_t ym2608_encode_flush(YM2608_ENCODE_HANDLE* nas);
void ym2608_encode_close(YM2608_ENCODE_HANDLE* nas);
int32_t ym2608_encode_write(YM2608_ENCODE_HANDLE* nas, int16_t* pcm_data, size_t pcm_len, int16_t channels);

#endif