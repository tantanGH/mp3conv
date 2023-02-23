#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "himem.h"
#include "pcm.h"

//
//  initialize pcm write handle
//
int32_t pcm_init(PCM_WRITE_HANDLE* pcm, FILE* fp, int16_t use_high_memory) {

  pcm->fp = fp;
  pcm->use_high_memory = use_high_memory;

  pcm->num_samples = 0;

  pcm->buffer_len = PCM_BUFFER_LEN;
  pcm->buffer_ofs = 0;
  pcm->buffer = himem_malloc(pcm->buffer_len * sizeof(int16_t), pcm->use_high_memory);

  return (pcm->buffer != NULL) ? 0 : -1;
}

//
//  flush buffer data to disk
//
int32_t pcm_flush(PCM_WRITE_HANDLE* pcm) {
  int32_t rc = 0;
  if (pcm->fp != NULL && pcm->buffer_ofs > 0) {
    size_t write_len = pcm->buffer_ofs;
    size_t written_len = 0;
    do {
      size_t len = fwrite(&(pcm->buffer[ written_len ]), sizeof(int16_t), write_len - written_len, pcm->fp);
      if (len == 0) break;
      written_len += len;
    } while (written_len < write_len);
    if (write_len < written_len) rc = -1;     // disk full?
    pcm->buffer_ofs = 0;
  }
  return rc;
}

//
//  close pcm write handle
//
void pcm_close(PCM_WRITE_HANDLE* pcm) {
  if (pcm->buffer_ofs > 0) {
    pcm_flush(pcm);
  }
  if (pcm->buffer != NULL) {
    himem_free(pcm->buffer, pcm->use_high_memory);
    pcm->buffer = NULL;
  }
}

//
//  add pcm samples, if buffer is full, write to disk
//
int32_t pcm_write(PCM_WRITE_HANDLE* pcm, int16_t* pcm_samples, size_t len) {

  int32_t rc = -1;

  // if buffer is full, flush data to disk
  if ((pcm->buffer_ofs + len) > pcm->buffer_len) {
//    printf("pcm->buffer_ofs=%d, len=%d\n",pcm->buffer_ofs,len);
    if (pcm_flush(pcm) != 0) {
      goto exit;
    }
//    printf("pcm->buffer_ofs=%d, len=%d\n",pcm->buffer_ofs,len);
  }

  // write data as much as possible
  size_t write_len = ((pcm->buffer_ofs + len) <= pcm->buffer_len) ? len : pcm->buffer_len - pcm->buffer_ofs;
//  printf("write_len=%d,len=%d\n",write_len,len);
  if (write_len > 0) {
    memcpy((void*)&(pcm->buffer[ pcm->buffer_ofs ]), (void*)pcm_samples, write_len * sizeof(int16_t));
    pcm->buffer_ofs += write_len;
    pcm->num_samples += write_len;
  }

  rc = 0;

exit:
  return rc;
}
