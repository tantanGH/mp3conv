#ifndef __H_ADPCM__
#define __H_ADPCM__

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <iocslib.h>

#define ADPCM_SAMPLE_RATE  (15625)
#define ADPCM_BUFFER_SIZE  (32768)
#define ADPCM_BUFFER_COUNT (8)
#define ADPCM_MODE         (4*256+3)

typedef struct {
  int16_t play_mode;
  int16_t step_index;
  int16_t last_estimate;
  size_t num_samples;
  int16_t current_buffer_id;
  size_t buffer_len;
  size_t buffer_ofs;
  uint8_t* buffers[ ADPCM_BUFFER_COUNT ];
  struct CHAIN2 chain_tables[ ADPCM_BUFFER_COUNT ];
} ADPCM_HANDLE;

int32_t adpcm_init(ADPCM_HANDLE* adpcm, int16_t play_mode);
void adpcm_close(ADPCM_HANDLE* adpcm);
int32_t adpcm_encode(ADPCM_HANDLE* adpcm, void* pcm_buffer, size_t pcm_buffer_len, int16_t pcm_bit_depth, int16_t pcm_channels);
int32_t adpcm_write_buffer(ADPCM_HANDLE* adpcm, FILE* fp, uint8_t* buffer, size_t len);
int32_t adpcm_write(ADPCM_HANDLE* adpcm, FILE* fp);

#endif