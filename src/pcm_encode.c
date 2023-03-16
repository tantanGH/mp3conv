#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "himem.h"
#include "pcm_encode.h"

//
//  initialize pcm encode handle
//
int32_t pcm_encode_init(PCM_ENCODE_HANDLE* pcm, FILE* fp, int16_t use_high_memory) {

  int32_t rc = -1;

  pcm->fp = fp;
  pcm->use_high_memory = use_high_memory;
  pcm->num_samples = 0;

  pcm->buffer_len = PCM_ENCODE_BUFFER_LEN;
  pcm->buffer_ofs = 0;
  pcm->buffer = himem_malloc(pcm->buffer_len * sizeof(int16_t), pcm->use_high_memory);
  if (pcm->buffer == NULL) goto exit;

  if (pcm->use_high_memory) {
    pcm->fwrite_buffer = himem_malloc(PCM_ENCODE_FWRITE_BUFFER_LEN * sizeof(int16_t), 0);
    if (pcm->fwrite_buffer == NULL) goto exit;
  }

  rc = 0;

exit: 
  return rc;
}

//
//  flush buffer data to disk
//
int32_t pcm_encode_flush(PCM_ENCODE_HANDLE* pcm) {
  int32_t rc = 0;
  if (pcm->fp != NULL && pcm->buffer_ofs > 0) {
    size_t write_len = pcm->buffer_ofs;
    size_t written_len = 0;
    if (pcm->use_high_memory) {
      do {
        size_t cpy_len = ( write_len - written_len ) > PCM_ENCODE_FWRITE_BUFFER_LEN ? PCM_ENCODE_FWRITE_BUFFER_LEN : write_len - written_len;
        memcpy(pcm->fwrite_buffer, &(pcm->buffer[ written_len ]), cpy_len * sizeof(int16_t));
        size_t len = fwrite(pcm->fwrite_buffer, sizeof(int16_t), cpy_len, pcm->fp); 
        if (len == 0) break;
        written_len += len;        
      } while (written_len < write_len);
    } else {
      do {
        size_t len = fwrite(&(pcm->buffer[ written_len ]), sizeof(int16_t), write_len - written_len, pcm->fp);
        if (len == 0) break;
        written_len += len;
      } while (written_len < write_len);
    }
    if (write_len > written_len) rc = -1;     // disk full?
    pcm->buffer_ofs = 0;
  }
  return rc;
}

//
//  close pcm encode handle
//
void pcm_encode_close(PCM_ENCODE_HANDLE* pcm) {
  if (pcm->buffer_ofs > 0) {
    pcm_encode_flush(pcm);
  }
  if (pcm->buffer != NULL) {
    himem_free(pcm->buffer, pcm->use_high_memory);
    pcm->buffer = NULL;
  }
  if (pcm->fwrite_buffer != NULL) {
    himem_free(pcm->fwrite_buffer, 0);
    pcm->fwrite_buffer = NULL;
  }
}

//
//  add pcm samples, if buffer is full, write to disk
//
int32_t pcm_encode_write(PCM_ENCODE_HANDLE* pcm, int16_t* pcm_samples, size_t len) {

  int32_t rc = -1;

  // if buffer is full, flush data to disk
  if ((pcm->buffer_ofs + len) > pcm->buffer_len) {
    if (pcm_encode_flush(pcm) != 0) {
      goto exit;
    }
  }

  // write data as much as possible
  size_t write_len = ((pcm->buffer_ofs + len) <= pcm->buffer_len) ? len : pcm->buffer_len - pcm->buffer_ofs;
  if (write_len > 0) {
    memcpy((void*)&(pcm->buffer[ pcm->buffer_ofs ]), (void*)pcm_samples, write_len * sizeof(int16_t));
    pcm->buffer_ofs += write_len;
    pcm->num_samples += write_len;
  }

  rc = 0;

exit:
  return rc;
}
