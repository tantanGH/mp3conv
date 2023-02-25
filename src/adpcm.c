#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "himem.h"
#include "adpcm.h"

//
//  MSM6258V ADPCM constant tables
//
static const int16_t step_adjust[] = { -1, -1, -1, -1, 2, 4, 6, 8, -1, -1, -1, -1, 2, 4, 6, 8 };
static const int16_t step_size[] = { 
        16,  17,  19,  21,  23,  25,  28,  31,  34,  37,  41,  45,   50,   55,   60,   66,
        73,  80,  88,  97, 107, 118, 130, 143, 157, 173, 190, 209,  230,  253,  279,  307,
       337, 371, 408, 449, 494, 544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552 };

//
//  MSM6258V ADPCM decode
//
static int16_t decode(uint8_t code, int16_t* step_index, int16_t last_data) {

  int16_t si = *step_index;
  int16_t ss = step_size[ si ];

  int16_t delta = ( ss >> 3 );
  if (code & 0x01) {
    delta += (ss >> 2);
  }
  if (code & 0x02) {
    delta += (ss >> 1);
  }
  if (code & 0x04) {
    delta += ss;
  }
  if (code & 0x08) {
    delta = -delta;
  }
    
  int16_t estimate = last_data + delta;
  if (estimate > 2047) {
    estimate = 2047;
  }

  if (estimate < -2048) {
    estimate = -2048;
  }

  si += step_adjust[ code ];
  if (si < 0) {
    si = 0;
  }
  if (si > 48) {
    si = 48;
  }
  *step_index = si;

  return estimate;
}

//
//  MSM6258V ADPCM encode
//
static uint8_t encode(int16_t current_data, int16_t last_estimate, int16_t* step_index, int16_t* new_estimate) {

  int16_t ss = step_size[ *step_index ];

  int16_t delta = current_data - last_estimate;

  uint8_t code = 0x00;
  if (delta < 0) {
    code = 0x08;          // bit3 = 1
    delta = -delta;
  }
  if (delta >= ss) {
    code += 0x04;         // bit2 = 1
    delta -= ss;
  }
  if (delta >= (ss>>1)) {
    code += 0x02;         // bit1 = 1
    delta -= ss>>1;
  }
  if (delta >= (ss>>2)) {
    code += 0x01;         // bit0 = 1
  } 

  // need to use decoder to estimate
  *new_estimate = decode(code, step_index, last_estimate);

  return code;
}

//
//  initialize adpcm write handle
//
int32_t adpcm_init(ADPCM_WRITE_HANDLE* adpcm, FILE* fp, int16_t use_high_memory) {

  int32_t rc = -1;

  adpcm->fp = fp;
//  adpcm->use_high_memory = use_high_memory;
  adpcm->use_high_memory = 0;

  adpcm->step_index = 0;
  adpcm->last_estimate = 0;
  adpcm->num_samples = 0;

  adpcm->buffer_len = ADPCM_BUFFER_LEN;
  adpcm->buffer_ofs = 0;
  adpcm->buffer = himem_malloc(adpcm->buffer_len, adpcm->use_high_memory);
  if (adpcm->buffer == NULL) goto exit;
  
  if (adpcm->use_high_memory) {
    adpcm->fwrite_buffer = himem_malloc(ADPCM_FWRITE_BUFFER_LEN * sizeof(uint8_t), 0);
    if (adpcm->fwrite_buffer == NULL) goto exit;
  }

  rc = 0;

exit:
  return rc;
}

//
//  flush buffer data to disk
//
int32_t adpcm_flush(ADPCM_WRITE_HANDLE* adpcm) {
  int32_t rc = 0;
  if (adpcm->fp != NULL && adpcm->buffer_ofs > 0) {
    size_t write_len = adpcm->buffer_ofs;
    size_t written_len = 0;
    if (adpcm->use_high_memory) {
      do {
        size_t cpy_len = ( write_len - written_len ) > ADPCM_FWRITE_BUFFER_LEN ? ADPCM_FWRITE_BUFFER_LEN : write_len - written_len;
        memcpy(adpcm->fwrite_buffer, &(adpcm->buffer[ written_len ]), cpy_len * sizeof(uint8_t));
        size_t len = fwrite(adpcm->fwrite_buffer, sizeof(uint8_t), cpy_len, adpcm->fp); 
        if (len == 0) break;
        written_len += len;        
      } while (written_len < write_len);
    } else {
      do {
        size_t len = fwrite(&(adpcm->buffer[ written_len ]), sizeof(uint8_t), write_len - written_len, adpcm->fp);
        if (len == 0) break;
        written_len += len;
      } while (written_len < write_len);
    }
    if (write_len > written_len) rc = -1;     // disk full?
    adpcm->buffer_ofs = 0;
  }
  return rc;
}

//
//  close adpcm write handle
//
void adpcm_close(ADPCM_WRITE_HANDLE* adpcm) {
  if (adpcm->buffer_ofs > 0) {
    adpcm_flush(adpcm);
  }
  if (adpcm->buffer != NULL) {
    himem_free(adpcm->buffer, 0);
    adpcm->buffer = NULL;
  }
}

//
//  write pcm data with adpcm encoding
//
int32_t adpcm_write(ADPCM_WRITE_HANDLE* adpcm, int16_t* pcm_buffer, size_t pcm_len, int16_t pcm_channels) {

  int32_t rc = -1;
  size_t pcm_ofs = 0;

  while (pcm_ofs < pcm_len) {

    // get 12bit PCM mono data
    int16_t xx = 0;
    if (pcm_channels == 2) {
      // 16bit PCM LR to 12bit PCM mono
      xx = ((int32_t)(pcm_buffer[ pcm_ofs ]) + (int32_t)(pcm_buffer[ pcm_ofs + 1 ])) / 2 / 16;
      pcm_ofs += 2;
    } else {
      // 16bit PCM mono to 12bit PCM mono
      xx = pcm_buffer[ pcm_ofs ] / 16;
      pcm_ofs += 1;
   }

    // encode to 4bit ADPCM data
    int16_t new_estimate;
    uint8_t code = encode(xx, adpcm->last_estimate, &adpcm->step_index, &new_estimate);
    adpcm->last_estimate = new_estimate;

    // current buffer is full?
    if (adpcm->buffer_ofs >= adpcm->buffer_len) {
      if (adpcm_flush(adpcm) != 0) {
        goto exit;
      }
    }
    
    // fill a byte in this order: lower 4 bit -> upper 4 bit
    if ((adpcm->num_samples % 2) == 0) {
      adpcm->buffer[ adpcm->buffer_ofs ] = code;
    } else {
      adpcm->buffer[ adpcm->buffer_ofs ] |= code << 4;
      adpcm->buffer_ofs++;
    }
    adpcm->num_samples++;

  }

  rc = 0;

exit:
  return rc;
}
