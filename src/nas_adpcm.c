#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "himem.h"
#include "nas_adpcm.h"

//
//  initialize nas adpcm write handle
//
int32_t nas_adpcm_init(NAS_ADPCM_WRITE_HANDLE* nas, FILE* fp, int16_t use_high_memory) {

  int32_t rc = -1;

  nas->fp = fp;
  nas->use_high_memory = use_high_memory;
  nas->num_samples = 0;
  nas->lib_initialized = 0;

  nas->buffer_len = NAS_ADPCM_BUFFER_LEN;
  nas->buffer_ofs = 0;
  nas->buffer = himem_malloc(nas->buffer_len, nas->use_high_memory);
  if (nas->buffer == NULL) goto exit;

  if (nas->use_high_memory) {
    nas->fwrite_buffer = himem_malloc(NAS_ADPCM_FWRITE_BUFFER_LEN * sizeof(uint8_t), 0);
    if (nas->fwrite_buffer == NULL) goto exit;
  }

  asm volatile (
    "jbsr  ptoa_make_buffer\n"
    :                   // output operand
    :                   // input operand
    :                   // clobbered register
  );

  rc = 0;

exit:
  return rc;
}

//
//  flush buffer data to disk
//
int32_t nas_adpcm_flush(NAS_ADPCM_WRITE_HANDLE* nas) {
  int32_t rc = 0;
  if (nas->fp != NULL && nas->buffer_ofs > 0) {
    size_t write_len = nas->buffer_ofs;
    size_t written_len = 0;
    if (nas->use_high_memory) {
      do {
        size_t cpy_len = ( write_len - written_len ) > NAS_ADPCM_FWRITE_BUFFER_LEN ? NAS_ADPCM_FWRITE_BUFFER_LEN : write_len - written_len;
        memcpy(nas->fwrite_buffer, &(nas->buffer[ written_len ]), cpy_len * sizeof(uint8_t));
        size_t len = fwrite(nas->fwrite_buffer, sizeof(uint8_t), cpy_len, nas->fp); 
        if (len == 0) break;
        written_len += len;        
      } while (written_len < write_len);
    } else {
      do {
        size_t len = fwrite(&(nas->buffer[ written_len ]), sizeof(uint8_t), write_len - written_len, nas->fp);
        if (len == 0) break;
        written_len += len;
      } while (written_len < write_len);    
    }
  if (write_len > written_len) rc = -1;     // disk full?
    nas->buffer_ofs = 0;
  }
  return rc;
}

//
//  close nas adpcm write handle
//
void nas_adpcm_close(NAS_ADPCM_WRITE_HANDLE* nas) {
  if (nas->buffer_ofs > 0) {
    nas_adpcm_flush(nas);
  }
  if (nas->buffer != NULL) {
    himem_free(nas->buffer, nas->use_high_memory);
    nas->buffer = NULL;
  }
  if (nas->fwrite_buffer != NULL) {
    himem_free(nas->fwrite_buffer, 0);
    nas->fwrite_buffer = NULL;
  }
}

//
//  write nas adpcm data with encoding
//
int32_t nas_adpcm_write(NAS_ADPCM_WRITE_HANDLE* nas, int16_t* pcm_data, size_t pcm_len, int16_t channels) {

  int32_t rc = -1;
  uint32_t pcm_data_bytes = pcm_len * sizeof(int16_t);    // do not need to multiply channels (already included)

  if (!nas->lib_initialized) {
    register uint32_t reg_d0 asm ("d0") = (uint32_t)(channels == 1 ? 0 : 1);
    asm volatile (
      "jbsr  ptoa_init\n"
      :                   // output operand
      : "r" (reg_d0)      // input operand
      :                   // clobbered register
    );
    nas->lib_initialized = 1;
  }

  if (nas->buffer_ofs + pcm_data_bytes > nas->buffer_len) {
    if (nas_adpcm_flush(nas) != 0) {
      goto exit;
    }
  }

  // NAS ADPCM encode
  register uint32_t reg_d0 asm ("d0") = (uint32_t)(pcm_data_bytes);
  register uint32_t reg_a0 asm ("a0") = (uint32_t)(pcm_data);
  register uint32_t reg_a1 asm ("a1") = (uint32_t)(&(nas->buffer[nas->buffer_ofs]));
  asm volatile (
    "jbsr  ptoa_exec\n"
    :                   // output operand
    : "r" (reg_d0),     // input operand
      "r" (reg_a0),     // input operand
      "r" (reg_a1)      // input operand
    :                   // clobbered register
  );

  nas->buffer_ofs += pcm_data_bytes / 4;
  nas->num_samples += pcm_len;

  rc = 0;

exit:
  return rc;
}
