#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include "pcm.h"
#include "memory.h"

//
//  initialize pcm handle
//
int32_t pcm_init(PCM_HANDLE* pcm, int16_t use_high_memory) {
  pcm->use_high_memory = use_high_memory;
  pcm->num_samples = 0;
  pcm->buffer_len = PCM_BUFFER_LEN;
  pcm->buffer_ofs = 0;
  pcm->buffer = malloc_himem(pcm->buffer_len, pcm->use_high_memory);
  return (pcm->buffer != NULL) ? 0 : -1;
}

//
//  close pcm handle
//
void pcm_close(PCM_HANDLE* pcm) {
  if (pcm->buffer != NULL) {
    free_himem(pcm->buffer, pcm->use_high_memory);
    pcm->buffer = 0;
  }
}

//
//  write pcm data to file
//
int32_t pcm_write(PCM_HANDLE* pcm, FILE* fp) {

  // default return code
  int32_t rc = -1;

  if (fp == NULL) {
    printf("error: no available output file handle.\n");
    goto exit;
  }

  size_t written = 0;
  do {
    written += fwrite(pcm->buffer + written, 1, pcm->buffer_ofs - written, fp);
  } while (written < pcm->buffer_ofs);
  pcm->buffer_ofs = 0;

  rc = 0;

exit:
  return rc;
}